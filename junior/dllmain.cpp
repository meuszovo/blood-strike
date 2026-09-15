#include "pch.h"
using namespace sdk;
using namespace hooks;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND g_hWnd = nullptr;
WNDPROC g_oWndProc = nullptr;

static LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
        if (menuOpen) {
            ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
            return 0;
        }
        break;
    }

    LRESULT result = CallWindowProc(g_oWndProc, hWnd, msg, wParam, lParam);
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    return result;
}

static HMODULE g_hModule = nullptr;
static bool g_shouldUnload = false;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "global.h"
#include "imgui_settings.h"
#include "Fonts.h"
#include "vectors.h"
#include "drawer.h"
#include "saver_loader.h"
#include "smoke.h"
static void ZigzagLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness, float amplitude = 3.0f, int segments = 12) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float nx = -dy / len;
    float ny = dx / len;
    float time = (float)ImGui::GetTime() * 8.0f;
    for (int i = 0; i < segments; i++) {
        float t1 = (float)i / (float)segments;
        float t2 = (float)(i + 1) / (float)segments;
        float side = ((i % 2) == 0) ? 1.0f : -1.0f;
        float offset1 = sinf(time + t1 * 6.28f) * amplitude * side;
        float offset2 = sinf(time + t2 * 6.28f) * amplitude * ((i + 1) % 2 == 0 ? 1.0f : -1.0f);
        ImVec2 p1(a.x + dx * t1 + nx * offset1, a.y + dy * t1 + ny * offset1);
        ImVec2 p2(a.x + dx * t2 + nx * offset2, a.y + dy * t2 + ny * offset2);
        dl->AddLine(p1, p2, col, thickness);
    }
}

static void DottedLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness, float gap = 6.0f) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    for (float d = 0.0f; d < len; d += gap * 2.0f) {
        float s = d / len;
        float e = (d + gap) / len;
        if (e > 1.0f) e = 1.0f;
        dl->AddLine(ImVec2(a.x + dx * s, a.y + dy * s), ImVec2(a.x + dx * e, a.y + dy * e), col, thickness);
    }
}

static void GlowLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness) {
    ImU32 base = col & 0x00FFFFFF;
    ImU32 aVal = (col >> 24) & 0xFF;
    dl->AddLine(a, b, (ImU32)((ImU32)(aVal * 0.15f) << 24) | base, thickness + 6.0f);
    dl->AddLine(a, b, (ImU32)((ImU32)(aVal * 0.3f) << 24) | base, thickness + 3.0f);
    dl->AddLine(a, b, (ImU32)((ImU32)(aVal * 0.5f) << 24) | base, thickness + 1.0f);
    dl->AddLine(a, b, col, thickness);
}

static void ElectricLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness, int segments = 14) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float nx = -dy / len;
    float ny = dx / len;
    for (int i = 0; i < segments; i++) {
        float t1 = (float)i / (float)segments;
        float t2 = (float)(i + 1) / (float)segments;
        float rnd1 = ((float)(rand() % 100) / 100.0f - 0.5f) * 2.0f;
        float rnd2 = ((float)(rand() % 100) / 100.0f - 0.5f) * 2.0f;
        float amp = 5.0f * (1.0f - fabsf(t1 - 0.5f) * 2.0f);
        ImVec2 p1(a.x + dx * t1 + nx * rnd1 * amp, a.y + dy * t1 + ny * rnd1 * amp);
        ImVec2 p2(a.x + dx * t2 + nx * rnd2 * amp, a.y + dy * t2 + ny * rnd2 * amp);
        dl->AddLine(p1, p2, col, thickness);
    }
}

static void DashLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness, float dashLen = 10.0f, float gapLen = 5.0f) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float time = (float)ImGui::GetTime() * 5.0f;
    float offset = fmodf(time, dashLen + gapLen);
    for (float d = -offset; d < len; d += dashLen + gapLen) {
        float s = d / len;
        float e = (d + dashLen) / len;
        if (e < 0.0f) continue;
        if (s < 0.0f) s = 0.0f;
        if (e > 1.0f) e = 1.0f;
        if (s >= e) continue;
        dl->AddLine(ImVec2(a.x + dx * s, a.y + dy * s), ImVec2(a.x + dx * e, a.y + dy * e), col, thickness);
    }
}

static void ChainLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness, float chainSize = 6.0f, float gap = 3.0f) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float step = chainSize * 2.0f + gap;
    int count = (int)(len / step);
    for (int i = 0; i <= count; i++) {
        float d = (float)i * step;
        float t = d / len;
        if (t > 1.0f) break;
        ImVec2 center(a.x + dx * t, a.y + dy * t);
        dl->AddCircle(center, chainSize, col, 0, thickness);
    }
}

static void NeonLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness) {
    ImU32 base = col & 0x00FFFFFF;
    ImU32 aVal = (col >> 24) & 0xFF;
    dl->AddLine(a, b, (ImU32)((ImU32)(aVal * 0.1f) << 24) | base, thickness + 12.0f);
    dl->AddLine(a, b, (ImU32)((ImU32)(aVal * 0.2f) << 24) | base, thickness + 8.0f);
    dl->AddLine(a, b, (ImU32)((ImU32)(aVal * 0.4f) << 24) | base, thickness + 4.0f);
    dl->AddLine(a, b, (ImU32)((ImU32)(aVal * 0.7f) << 24) | base, thickness + 1.0f);
    dl->AddLine(a, b, IM_COL32(255, 255, 255, aVal), thickness);
}

static void PlasmaLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness, int segments = 20) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float nx = -dy / len;
    float ny = dx / len;
    float time = (float)ImGui::GetTime() * 4.0f;
    for (int i = 0; i < segments; i++) {
        float t1 = (float)i / (float)segments;
        float t2 = (float)(i + 1) / (float)segments;
        float wave = sinf(t1 * 12.0f + time) * 4.0f * sinf(t1 * 3.14f);
        float wave2 = sinf(t2 * 12.0f + time) * 4.0f * sinf(t2 * 3.14f);
        ImVec2 p1(a.x + dx * t1 + nx * wave, a.y + dy * t1 + ny * wave);
        ImVec2 p2(a.x + dx * t2 + nx * wave2, a.y + dy * t2 + ny * wave2);
        float pulse = 0.5f + 0.5f * sinf(time + t1 * 6.28f);
        ImU32 segCol = (col & 0x00FFFFFF) | ((ImU32)((ImU32)(((col >> 24) & 0xFF) * pulse) << 24));
        dl->AddLine(p1, p2, segCol, thickness + 3.0f);
        dl->AddLine(p1, p2, col, thickness);
    }
}

static void ArrowLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness, int arrowCount = 5) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float nx = -dy / len;
    float ny = dx / len;
    dl->AddLine(a, b, col, thickness);
    for (int i = 1; i <= arrowCount; i++) {
        float t = (float)i / (float)(arrowCount + 1);
        float arrowSize = 4.0f;
        ImVec2 center(a.x + dx * t, a.y + dy * t);
        float sign = (i % 2 == 0) ? 1.0f : -1.0f;
        ImVec2 tip(center.x + nx * arrowSize * sign, center.y + ny * arrowSize * sign);
        ImVec2 left(center.x - dx * 0.02f, center.y - dy * 0.02f);
        ImVec2 right(center.x + dx * 0.02f, center.y + dy * 0.02f);
        dl->AddTriangleFilled(tip, left, right, col);
    }
}

static void DrawStyledLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness, int mode) {
    switch (mode) {
        case 1: ZigzagLine(dl, a, b, col, thickness, 3.0f, 12); break;
        case 2: DottedLine(dl, a, b, col, thickness); break;
        case 3: GlowLine(dl, a, b, col, thickness); break;
        case 4: ElectricLine(dl, a, b, col, thickness); break;
        case 5: DashLine(dl, a, b, col, thickness); break;
        case 6: ChainLine(dl, a, b, col, thickness); break;
        case 7: NeonLine(dl, a, b, col, thickness); break;
        case 8: PlasmaLine(dl, a, b, col, thickness); break;
        case 9: ArrowLine(dl, a, b, col, thickness); break;
        default: dl->AddLine(a, b, col, thickness); break;
    }
}

void AimbotTick();

void CheatTick( )
{
    uint64_t ClientEngine = *(uint64_t *)( base + bloodstrike::offsets::Messiah__ClientEngine );
    if ( !ClientEngine ) return;

    uint64_t IGameplay = *(uint64_t *)( ClientEngine + 0x58 );
    if ( !IGameplay ) return;

    uint64_t ClientPlayer = *(uint64_t *)( IGameplay + 0x58 );
    if ( !ClientPlayer ) return;

    bloodstrike::renderer::camera = *(uint64_t *)( ClientPlayer + 0x238 );
    bloodstrike::renderer::localActor = *(uint64_t *)( ClientPlayer + 0x288 );

    if ( !bloodstrike::renderer::camera || !bloodstrike::renderer::localActor ) return;

    glm::mat4x3 local_trans = *(glm::mat4x3 *)( bloodstrike::renderer::localActor + 0x58 );
    glm::vec3 local_pos = local_trans[3];

    ImVec2 ds = ImGui::GetIO( ).DisplaySize;
    ds.x /= 2.f;
    ds.y /= 2.f;

    glm::vec2 sc = { ds.x, ds.y };

    uint64_t entityListStart = *(uint64_t *)( base + bloodstrike::offsets::Messiah__EntityList );
    if ( !entityListStart ) return;

    uint64_t head = *(uint64_t *)( entityListStart + 0x8 );
    if ( !head ) return;

    uint64_t currentActor = *(uint64_t *)( head );

    int valid = 0;
    int skipped = 0;
    bool isMisc = false;
    if ( currentActor )
    {
        do
        {
            isMisc = false;
            targetAddr = 0x0;
            uint64_t actorInstance = *(uint64_t *)( currentActor + 0x18 );
            if ( !actorInstance )
            {
                currentActor = *(uint64_t *)( currentActor );
                skipped++;
                continue;
            }

            uint64_t actorProps = *(uint64_t *)( actorInstance + 0x278 );
            if ( !actorProps )
            {
                currentActor = *(uint64_t *)( currentActor );
                skipped++;
                continue;
            }

            uint64_t actorComponent = *(uint64_t *)( actorProps + 0x18 );
            if ( !actorComponent )
            {
                currentActor = *(uint64_t *)( currentActor );
                skipped++;
                continue;
            }

            uint64_t IEntity = *(uint64_t *)( actorComponent + 0x40 );
            if ( !IEntity )
            {
                currentActor = *(uint64_t *)( currentActor );
                skipped++;
                continue;
            }

            uint64_t entityMask = *(uint64_t *)( IEntity + 0x2e0 );
            if ( entityMask != 2 )
            {
                isMisc = true;
            }

            if ( IEntity == bloodstrike::renderer::localActor )
            {
                static bool debugStats = false;
                if ( debugStats )
                {
                    visuals::DrawLabel( std::format( "Local {:x}\nCamera {:x}\nEntity List {:x}\nSkipped: {} | Valid: {}", bloodstrike::renderer::localActor, bloodstrike::renderer::camera, head, skipped, valid ), glm::vec2( 150, 100 ), visuals::ColorToArray( ImColor( 255, 255, 255, 255 ) ), true );
                }
                currentActor = *(uint64_t *)( currentActor );
                skipped++;
                continue;
            }

            uint64_t IArea = *(uint64_t *)( IEntity + 0x88 );

            if ( IArea == 0x0 )
            {
                currentActor = *(uint64_t *)( currentActor );
                skipped++;
                continue;
            }

            uint64_t pose = *(uint64_t *)( actorInstance + 0x18 );
            if ( !pose )
            {
                currentActor = *(uint64_t *)( currentActor );
                skipped++;
                continue;
            }

            uint64_t BipedPose = *(uint64_t *)( pose + 0x90 );
            if ( !BipedPose )
            {
                currentActor = *(uint64_t *)( currentActor );
                skipped++;
                continue;
            }

            BipedPose += 0x8; // first bone ptr is garbage

            glm::vec2 result;
            glm::mat4x3 trans = *(glm::mat4x3 *)( IEntity + 0x58 );
            XMFLOAT3X4 dxTrans = *(XMFLOAT3X4 *)( IEntity + 0x58 );

            glm::vec3 coords = trans[3];
            float d = ( glm::distance( local_pos, coords ) );
            float d2 = (int)d;

            if ( w2s( bloodstrike::renderer::camera, coords, result ) )
            {
                ImVec2 pos = { (float)result[0], (float)result[1] };
                if ( ( pos.x > ImGui::GetIO( ).DisplaySize.x || pos.y > ImGui::GetIO( ).DisplaySize.y ) || ( pos.x < 1.0 || pos.y < 1.0 ) )
                {
                    currentActor = *(uint64_t *)( currentActor );
                    skipped++;
                    continue;
                }
                int dst_m = (int)( d2 / 5 );

                if ( !isMisc )
                {
                    float t = d2 / 10.f;

                    std::string address_txt = std::format( "[{:x}]", IEntity );
                    std::string dist_txt = std::format( "{:d}m", dst_m );

                    t = std::clamp( t, 0.f, 1.f );

                    float g_lin = powf( t, 2.2f );
                    float r_lin = powf( 1.f - t, 2.2f );

                    ImColor color( r_lin, g_lin, 0.f, 1.f );

                    {
                        glm::vec2 neck, spine1, spine2, spine3, pelvis, buttCheekL, buttCheekR, kneeL, kneeR, footL, footR, sholL, elbowL, wristL, sholR, elbowR, wristR; //  no head in bipedPose too lazy to find it in the other array
                        glm::vec3 _neck, _spine1, _spine2, _spine3, _pelvis, _buttCheekL, _buttCheekR, _kneeL, _kneeR, _footL, _footR, _sholL, _elbowL, _wristL, _sholR, _elbowR, _wristR;

                        // thanks tantem for matrix offset from bonestart
                        uint64_t boneStart = *(uint64_t *)( ( 7 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _neck );
                            w2s( bloodstrike::renderer::camera, _neck, neck );
                        }

                        boneStart = *(uint64_t *)( ( 6 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _spine1 );
                            w2s( bloodstrike::renderer::camera, _spine1, spine1 );
                        }

                        boneStart = *(uint64_t *)( ( 5 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _spine2 );
                            w2s( bloodstrike::renderer::camera, _spine2, spine2 );
                        }

                        boneStart = *(uint64_t *)( ( 4 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _spine3 );
                            w2s( bloodstrike::renderer::camera, _spine3, spine3 );
                        }

                        boneStart = *(uint64_t *)( ( 3 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _pelvis );
                            w2s( bloodstrike::renderer::camera, _pelvis, pelvis );
                        }

                        boneStart = *(uint64_t *)( ( 22 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _buttCheekL );
                            w2s( bloodstrike::renderer::camera, _buttCheekL, buttCheekL );
                        }

                        boneStart = *(uint64_t *)( ( 18 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _buttCheekR );
                            w2s( bloodstrike::renderer::camera, _buttCheekR, buttCheekR );
                        }

                        boneStart = *(uint64_t *)( ( 23 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _kneeL );
                            w2s( bloodstrike::renderer::camera, _kneeL, kneeL );
                        }

                        boneStart = *(uint64_t *)( ( 19 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _kneeR );
                            w2s( bloodstrike::renderer::camera, _kneeR, kneeR );
                        }

                        boneStart = *(uint64_t *)( ( 24 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _footL );
                            w2s( bloodstrike::renderer::camera, _footL, footL );
                        }

                        boneStart = *(uint64_t *)( ( 20 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _footR );
                            w2s( bloodstrike::renderer::camera, _footR, footR );
                        }

                        boneStart = *(uint64_t *)( ( 14 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _sholL );
                            w2s( bloodstrike::renderer::camera, _sholL, sholL );
                        }

                        boneStart = *(uint64_t *)( ( 9 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _sholR );
                            w2s( bloodstrike::renderer::camera, _sholR, sholR );
                        }

                        boneStart = *(uint64_t *)( ( 15 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _elbowL );
                            w2s( bloodstrike::renderer::camera, _elbowL, elbowL );
                        }

                        boneStart = *(uint64_t *)( ( 10 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _elbowR );
                            w2s( bloodstrike::renderer::camera, _elbowR, elbowR );
                        }

                        boneStart = *(uint64_t *)( ( 16 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _wristL );
                            w2s( bloodstrike::renderer::camera, _wristL, wristL );
                        }

                        boneStart = *(uint64_t *)( ( 11 * 0x8 ) + BipedPose );
                        if ( boneStart ) {
                            MessiahMatrixAdd( *(XMFLOAT3X4 *)( boneStart + 0x30 ), dxTrans, _wristR );
                            w2s( bloodstrike::renderer::camera, _wristR, wristR );
                        }

                        float MIN_ESP = 15.f;
                        float height = abs( neck.y - result.y );
                        if ( height < MIN_ESP ) height = MIN_ESP;
                        float width = height * 0.4;
                        ImVec2 topLeft( neck.x - width, neck.y );
                        ImVec2 topRight( neck.x + width, neck.y );
                        ImVec2 bottomLeft( neck.x - width, neck.y + height );
                        ImVec2 bottomRight( neck.x + width, neck.y + height );
                        ImVec2 centerTop( neck.x, neck.y );

                        glm::vec2* bones[] = { &neck, &spine1, &spine2, &spine3, &pelvis,
                            &buttCheekL, &buttCheekR, &kneeL, &kneeR, &footL, &footR,
                            &sholL, &elbowL, &wristL, &sholR, &elbowR, &wristR };
                        float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
                        int boneCount = 0;
                        for ( auto* b : bones ) {
                            if ( b->x != 0.f || b->y != 0.f ) {
                                if ( b->x < minX ) minX = b->x;
                                if ( b->x > maxX ) maxX = b->x;
                                if ( b->y < minY ) minY = b->y;
                                if ( b->y > maxY ) maxY = b->y;
                                boneCount++;
                            }
                        }
                        float spread = std::max( maxX - minX, maxY - minY );
                        if ( boneCount > 0 && spread > 0.f && spread < MIN_ESP ) {
                            float cx = ( minX + maxX ) * 0.5f;
                            float cy = ( minY + maxY ) * 0.5f;
                            float scale = MIN_ESP / spread;
                            for ( auto* b : bones )
                                *b = glm::vec2( cx + ( b->x - cx ) * scale, cy + ( b->y - cy ) * scale );
                            height = MIN_ESP;
                            width = height * 0.4f;
                            topLeft = ImVec2( neck.x - width, neck.y );
                            topRight = ImVec2( neck.x + width, neck.y );
                            bottomLeft = ImVec2( neck.x - width, neck.y + height );
                            bottomRight = ImVec2( neck.x + width, neck.y + height );
                        }

                        valid++;

                        if ( global::esp::enabled )
                        {
                            ImDrawList* bg = ImGui::GetBackgroundDrawList();

                            if ( global::esp::box )
                            {
                                ImU32 boxCol = IM_COL32((int)(global::esp::boxColor[0]*255), (int)(global::esp::boxColor[1]*255), (int)(global::esp::boxColor[2]*255), (int)(global::esp::boxColor[3]*255));
                                DrawStyledLine(bg, topLeft, topRight, boxCol, 1.f, 0);
                                DrawStyledLine(bg, topRight, bottomRight, boxCol, 1.f, 0);
                                DrawStyledLine(bg, bottomRight, bottomLeft, boxCol, 1.f, 0);
                                DrawStyledLine(bg, bottomLeft, topLeft, boxCol, 1.f, 0);
                            }

                            if ( global::esp::skeleton )
                            {
                                ImU32 skelCol = IM_COL32((int)(global::esp::skeletonColor[0]*255), (int)(global::esp::skeletonColor[1]*255), (int)(global::esp::skeletonColor[2]*255), (int)(global::esp::skeletonColor[3]*255));
                                ImVec2 v_neck(neck.x,neck.y), v_spine1(spine1.x,spine1.y), v_spine2(spine2.x,spine2.y), v_spine3(spine3.x,spine3.y);
                                ImVec2 v_pelvis(pelvis.x,pelvis.y), v_bCL(buttCheekL.x,buttCheekL.y), v_bCR(buttCheekR.x,buttCheekR.y);
                                ImVec2 v_kL(kneeL.x,kneeL.y), v_kR(kneeR.x,kneeR.y), v_fL(footL.x,footL.y), v_fR(footR.x,footR.y);
                                ImVec2 v_sL(sholL.x,sholL.y), v_eL(elbowL.x,elbowL.y), v_wL(wristL.x,wristL.y);
                                ImVec2 v_sR(sholR.x,sholR.y), v_eR(elbowR.x,elbowR.y), v_wR(wristR.x,wristR.y);
                                DrawStyledLine(bg, v_neck, v_spine1, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_spine1, v_spine2, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_spine2, v_spine3, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_spine3, v_pelvis, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_spine1, v_sL, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_sL, v_eL, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_eL, v_wL, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_spine1, v_sR, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_sR, v_eR, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_eR, v_wR, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_pelvis, v_bCL, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_bCL, v_kL, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_kL, v_fL, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_pelvis, v_bCR, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_bCR, v_kR, skelCol, 1.f, 0);
                                DrawStyledLine(bg, v_kR, v_fR, skelCol, 1.f, 0);
                            }

                            if ( global::esp::line )
                            {
                                ImU32 lineCol = IM_COL32((int)(global::esp::lineColor[0]*255), (int)(global::esp::lineColor[1]*255), (int)(global::esp::lineColor[2]*255), (int)(global::esp::lineColor[3]*255));
                                ImVec2 ds = ImGui::GetIO().DisplaySize;
                                ImVec2 lineStart = ImVec2( ds.x * 0.5f, ds.y );
                                ImVec2 lineEnd = ImVec2(neck.x, neck.y);
                                DrawStyledLine(bg, lineStart, lineEnd, lineCol, 1.f, 0);
                            }
                        }
                    }
                }
                else
                {
                    float t = d2 / 10.f;

                    std::string txt = std::format( "[{}m] object~{}", dst_m, entityMask );
                    visuals::DrawLabel( txt, glm::vec2( result.x, result.y ), visuals::ColorToArray( ImColor( 125, 125, 125, 255 ) ), true );
                }
            }

            currentActor = *(uint64_t *)( currentActor );
            valid++;
        } while ( currentActor != head );

        AimbotTick();
    }
}

static int AimGetBoneIndex(int bone) {
    switch (bone) {
        case 0: return 7;
        case 1: return 7;
        case 2: return 6;
        case 3: return 3;
        default: return 7;
    }
}

static bool AimGetBonePos(uint64_t BipedPose, XMFLOAT3X4& dxTrans, int boneIdx, glm::vec3& out) {
    uint64_t boneStart = *(uint64_t*)((boneIdx * 0x8) + BipedPose);
    if (!boneStart) return false;
    XMFLOAT3X4 boneMat = *(XMFLOAT3X4*)(boneStart + 0x30);
    MessiahMatrixAdd(boneMat, dxTrans, out);
    return true;
}

void AimbotTick()
{
    if (!bloodstrike::renderer::camera || !bloodstrike::renderer::localActor) return;

    should_change_mouse = false;
    dx = 0;
    dy = 0;

    ImVec2 screenCenter(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);
    g_screenCenterX = screenCenter.x;
    g_screenCenterY = screenCenter.y;

    static bool lastAimKeyState = false;
    bool aimKeyHeld = (GetAsyncKeyState(global::aim::aimKey) & 0x8000) != 0;
    if (aimKeyHeld && !lastAimKeyState) {
        global::aim::enabled = !global::aim::enabled;
    }
    lastAimKeyState = aimKeyHeld;

    if (global::aim::drawFov)
    {
        ImColor fovCol = ImColor(global::aim::fovColor[0], global::aim::fovColor[1], global::aim::fovColor[2], global::aim::fovColor[3]);
        ImGui::GetBackgroundDrawList()->AddCircle(screenCenter, global::aim::fov, fovCol, 64, 1.0f);
    }

    if (!global::aim::enabled || menuOpen) return;

    bool shooting = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (!shooting) return;

    uint64_t ClientEngine = *(uint64_t*)(base + bloodstrike::offsets::Messiah__ClientEngine);
    if (!ClientEngine) return;
    uint64_t IGameplay = *(uint64_t*)(ClientEngine + 0x58);
    if (!IGameplay) return;
    uint64_t ClientPlayer = *(uint64_t*)(IGameplay + 0x58);
    if (!ClientPlayer) return;
    uint64_t cam = *(uint64_t*)(ClientPlayer + 0x238);
    uint64_t actor = *(uint64_t*)(ClientPlayer + 0x288);
    if (!cam || !actor) return;

    glm::mat4x3 localTrans = *(glm::mat4x3*)(actor + 0x58);
    glm::vec3 localPos = localTrans[3];

    uint64_t entityListStart = *(uint64_t*)(base + bloodstrike::offsets::Messiah__EntityList);
    if (!entityListStart) return;
    uint64_t head = *(uint64_t*)(entityListStart + 0x8);
    if (!head) return;
    uint64_t currentActor = *(uint64_t*)(head);
    if (!currentActor) return;

    float closestAimDist = global::aim::fov;
    glm::vec2 bestAimTarget(0.0f, 0.0f);
    bool foundAimTarget = false;

    int aimBoneIdx = AimGetBoneIndex(0);
    int iterations = 0;
    ULONGLONG loopStart = GetTickCount64();

    do
    {
        if (iterations++ >= 1024 || GetTickCount64() - loopStart > 5) break;
        if (currentActor == head) break;

        uint64_t next = *(uint64_t*)(currentActor);

        uint64_t actorInstance = *(uint64_t*)(currentActor + 0x18);
        if (!actorInstance) { currentActor = next; continue; }
        uint64_t actorProps = *(uint64_t*)(actorInstance + 0x278);
        if (!actorProps) { currentActor = next; continue; }
        uint64_t actorComponent = *(uint64_t*)(actorProps + 0x18);
        if (!actorComponent) { currentActor = next; continue; }
        uint64_t IEntity = *(uint64_t*)(actorComponent + 0x40);
        if (!IEntity) { currentActor = next; continue; }
        uint64_t entityMask = *(uint64_t*)(IEntity + 0x2e0);
        if (entityMask != 2) { currentActor = next; continue; }
        if (IEntity == bloodstrike::renderer::localActor) { currentActor = next; continue; }
        uint64_t IArea = *(uint64_t*)(IEntity + 0x88);
        if (IArea == 0) { currentActor = next; continue; }

        uint64_t pose = *(uint64_t*)(actorInstance + 0x18);
        if (!pose) { currentActor = next; continue; }
        uint64_t BipedPose = *(uint64_t*)(pose + 0x90);
        if (!BipedPose) { currentActor = next; continue; }
        BipedPose += 0x8;

        XMFLOAT3X4 dxTrans = *(XMFLOAT3X4*)(IEntity + 0x58);

        glm::vec3 aimBonePos;
        if (AimGetBonePos(BipedPose, dxTrans, aimBoneIdx, aimBonePos)) {
            glm::vec2 bone2D;
            if (w2s(bloodstrike::renderer::camera, aimBonePos, bone2D)) {
                float ddx = bone2D.x - screenCenter.x;
                float ddy = bone2D.y - screenCenter.y;
                float distToCrosshair = sqrtf(ddx * ddx + ddy * ddy);
                if (distToCrosshair < closestAimDist) {
                    closestAimDist = distToCrosshair;
                    bestAimTarget = bone2D;
                    foundAimTarget = true;
                }
            }
        }

        currentActor = next;
    } while (currentActor != head);

    if (foundAimTarget) {
        float targetX = bestAimTarget.x - screenCenter.x;
        float targetY = bestAimTarget.y - screenCenter.y;

        float speed = 100.f / global::aim::smooth;
        float moveX = targetX;
        float moveY = targetY;
        if (fabsf(moveX) > speed) moveX = speed * (moveX > 0.f ? 1.f : -1.f);
        if (fabsf(moveY) > speed) moveY = speed * (moveY > 0.f ? 1.f : -1.f);

        dx = (int)moveX;
        dy = (int)moveY;
        should_change_mouse = true;
    }
}

HRESULT hkPresent( IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags )
{
    if ( !bloodstrike::renderer::hooked )
    {
        if ( FAILED( pSwapChain->GetDevice( __uuidof( ID3D11Device ), (void **)&bloodstrike::renderer::deviceInstance ) ) )
            return oPresent( pSwapChain, SyncInterval, Flags );

        bloodstrike::renderer::deviceInstance->GetImmediateContext( &bloodstrike::renderer::contextInstance );

        ID3D11Texture2D *backBuffer = nullptr;
        pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void **)&backBuffer );

        if ( !backBuffer ) return oPresent( pSwapChain, SyncInterval, Flags );

        bloodstrike::renderer::deviceInstance->CreateRenderTargetView( backBuffer, nullptr, &bloodstrike::renderer::rtv );
        backBuffer->Release( );

        IMGUI_CHECKVERSION( );
        ImGui::CreateContext( );
        ImGuiIO &io = ImGui::GetIO( ); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGuiStyle &s = ImGui::GetStyle( );

        s.FramePadding = ImVec2(16, 16);
        s.ItemSpacing = ImVec2(10, 10);
        s.FrameRounding = 4.f;
        s.WindowRounding = 10.f;
        s.WindowBorderSize = 0.f;
        s.PopupBorderSize = 0.f;
        s.WindowPadding = ImVec2(0, 0);
        s.ChildBorderSize = 1.f;
        s.Colors[ImGuiCol_WindowBg] = winbg_color;
        s.Colors[ImGuiCol_Border] = ImVec4(0.f, 0.f, 0.f, 0.f);
        s.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
        s.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.02f, 0.14f, 1.0f);
        s.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.04f, 0.20f, 1.0f);
        s.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.06f, 0.30f, 1.0f);
        s.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.08f, 0.38f, 1.0f);
        s.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.0f, 1.0f, 1.0f);
        s.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.85f, 0.1f, 1.0f, 1.0f);
        s.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.04f, 0.25f, 1.0f);
        s.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.06f, 0.40f, 1.0f);
        s.Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.08f, 0.55f, 1.0f);
        s.Colors[ImGuiCol_Header] = ImVec4(0.18f, 0.04f, 0.30f, 1.0f);
        s.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.06f, 0.40f, 1.0f);
        s.Colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.08f, 0.55f, 1.0f);
        s.Colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.05f, 0.50f, 0.5f);
        s.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
        s.PopupRounding = 5.f;
        s.PopupBorderSize = 1.3f;
        s.ScrollbarSize = 5.f;
        s.ScrollbarRounding = 10.f;

        font_esp = io.Fonts->AddFontFromFileTTF( "C:\\Windows\\Fonts\\verdanab.ttf", 10, nullptr, io.Fonts->GetGlyphRangesDefault( ) );

        default_font = io.Fonts->AddFontFromMemoryTTF(&PoppinsMedium, sizeof PoppinsMedium, 30, NULL, io.Fonts->GetGlyphRangesCyrillic());

        big_font = io.Fonts->AddFontFromMemoryTTF(&PoppinsMedium, sizeof PoppinsMedium, 42, NULL, io.Fonts->GetGlyphRangesCyrillic());

        small_font = io.Fonts->AddFontFromMemoryTTF(&PoppinsSemiBold, sizeof PoppinsSemiBold, 17, NULL, io.Fonts->GetGlyphRangesCyrillic());

        tabs_font = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 25, NULL, io.Fonts->GetGlyphRangesCyrillic());

        icon_font = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 20, NULL, io.Fonts->GetGlyphRangesCyrillic());

        logo_font = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 39, NULL, io.Fonts->GetGlyphRangesCyrillic());

        small_icon_font = io.Fonts->AddFontFromMemoryTTF(&icomoon, sizeof icomoon, 12, NULL, io.Fonts->GetGlyphRangesCyrillic());

        title_font = io.Fonts->AddFontFromMemoryTTF(&PoppinsBold, sizeof PoppinsBold, 24, NULL, io.Fonts->GetGlyphRangesCyrillic());

        bloodstrike::renderer::hWindow = *(HWND *)( base + bloodstrike::renderer::hwnd );

        ImGui_ImplWin32_Init( bloodstrike::renderer::hWindow );
        ImGui_ImplDX11_Init( bloodstrike::renderer::deviceInstance, bloodstrike::renderer::contextInstance );

        g_hWnd = bloodstrike::renderer::hWindow;
        g_oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

#ifdef DEBUG
        printf( "[+] imgui init\n" );
#endif

        int w, h, channels = 0;
        unsigned char *pixels = stbi_load_from_memory(
            (const stbi_uc *)kk,
            sizeof(kk),
            &w,
            &h,
            &channels,
            4
        );

        if ( pixels )
        {
            D3D11_TEXTURE2D_DESC desc{};
            desc.Width = w;
            desc.Height = h;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA sub{};
            sub.pSysMem = pixels;
            sub.SysMemPitch = w * 4;

            ID3D11Texture2D *texture = nullptr;
            bloodstrike::renderer::deviceInstance->CreateTexture2D( &desc, &sub, &texture );

            bloodstrike::renderer::deviceInstance->CreateShaderResourceView( texture, nullptr, &bloodstrike::renderer::srv );
            texture->Release( );
            stbi_image_free( pixels );

            bloodstrike::renderer::hooked = true;
        }

        {
            int jw, jh, jch = 0;
            unsigned char *jpixels = stbi_load("F:\\CARRERA CHEAT BLOODSTRIKE\\junior\\ESP JUNIOR.png", &jw, &jh, &jch, 4);
            if (jpixels)
            {
                D3D11_TEXTURE2D_DESC jdesc{};
                jdesc.Width = jw;
                jdesc.Height = jh;
                jdesc.MipLevels = 1;
                jdesc.ArraySize = 1;
                jdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                jdesc.SampleDesc.Count = 1;
                jdesc.Usage = D3D11_USAGE_DEFAULT;
                jdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                D3D11_SUBRESOURCE_DATA jsub{};
                jsub.pSysMem = jpixels;
                jsub.SysMemPitch = jw * 4;

                ID3D11Texture2D *jtexture = nullptr;
                bloodstrike::renderer::deviceInstance->CreateTexture2D( &jdesc, &jsub, &jtexture );
                bloodstrike::renderer::deviceInstance->CreateShaderResourceView( jtexture, nullptr, &bloodstrike::renderer::junior_srv );
                jtexture->Release( );
                stbi_image_free( jpixels );
            }
        }

        {
            int aw, ah, ach = 0;
            unsigned char *apixels = stbi_load("F:\\CARRERA CHEAT BLOODSTRIKE\\junior\\aim icon.png", &aw, &ah, &ach, 4);
            if (apixels)
            {
                D3D11_TEXTURE2D_DESC adesc{};
                adesc.Width = aw;
                adesc.Height = ah;
                adesc.MipLevels = 1;
                adesc.ArraySize = 1;
                adesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                adesc.SampleDesc.Count = 1;
                adesc.Usage = D3D11_USAGE_DEFAULT;
                adesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                D3D11_SUBRESOURCE_DATA asub{};
                asub.pSysMem = apixels;
                asub.SysMemPitch = aw * 4;

                ID3D11Texture2D *atexture = nullptr;
                bloodstrike::renderer::deviceInstance->CreateTexture2D( &adesc, &asub, &atexture );
                bloodstrike::renderer::deviceInstance->CreateShaderResourceView( atexture, nullptr, &bloodstrike::renderer::aim_icon_srv );
                atexture->Release( );
                stbi_image_free( apixels );
            }
            ImGui::SetAimIconTexture((ImTextureID)bloodstrike::renderer::aim_icon_srv);
        }

        {
            int vw, vh, vch = 0;
            unsigned char *vpixels = stbi_load("F:\\CARRERA CHEAT BLOODSTRIKE\\junior\\visuals icon.png", &vw, &vh, &vch, 4);
            if (vpixels)
            {
                D3D11_TEXTURE2D_DESC vdesc{};
                vdesc.Width = vw;
                vdesc.Height = vh;
                vdesc.MipLevels = 1;
                vdesc.ArraySize = 1;
                vdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                vdesc.SampleDesc.Count = 1;
                vdesc.Usage = D3D11_USAGE_DEFAULT;
                vdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                D3D11_SUBRESOURCE_DATA vsub{};
                vsub.pSysMem = vpixels;
                vsub.SysMemPitch = vw * 4;

                ID3D11Texture2D *vtexture = nullptr;
                bloodstrike::renderer::deviceInstance->CreateTexture2D( &vdesc, &vsub, &vtexture );
                bloodstrike::renderer::deviceInstance->CreateShaderResourceView( vtexture, nullptr, &bloodstrike::renderer::visuals_icon_srv );
                vtexture->Release( );
                stbi_image_free( vpixels );
            }
            ImGui::SetVisualsIconTexture((ImTextureID)bloodstrike::renderer::visuals_icon_srv);
        }

        {
            int xw, xh, xch = 0;
            unsigned char *xpixels = stbi_load("F:\\CARRERA CHEAT BLOODSTRIKE\\junior\\misc icon.png", &xw, &xh, &xch, 4);
            if (xpixels)
            {
                D3D11_TEXTURE2D_DESC xdesc{};
                xdesc.Width = xw;
                xdesc.Height = xh;
                xdesc.MipLevels = 1;
                xdesc.ArraySize = 1;
                xdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                xdesc.SampleDesc.Count = 1;
                xdesc.Usage = D3D11_USAGE_DEFAULT;
                xdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                D3D11_SUBRESOURCE_DATA xsub{};
                xsub.pSysMem = xpixels;
                xsub.SysMemPitch = xw * 4;

                ID3D11Texture2D *xtexture = nullptr;
                bloodstrike::renderer::deviceInstance->CreateTexture2D( &xdesc, &xsub, &xtexture );
                bloodstrike::renderer::deviceInstance->CreateShaderResourceView( xtexture, nullptr, &bloodstrike::renderer::misc_icon_srv );
                xtexture->Release( );
                stbi_image_free( xpixels );
            }
            ImGui::SetMiscIconTexture((ImTextureID)bloodstrike::renderer::misc_icon_srv);
        }

        {
            int sw, sh, sch = 0;
            unsigned char *spixels = stbi_load("F:\\CARRERA CHEAT BLOODSTRIKE\\junior\\settings icon.png", &sw, &sh, &sch, 4);
            if (spixels)
            {
                D3D11_TEXTURE2D_DESC sdesc{};
                sdesc.Width = sw;
                sdesc.Height = sh;
                sdesc.MipLevels = 1;
                sdesc.ArraySize = 1;
                sdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                sdesc.SampleDesc.Count = 1;
                sdesc.Usage = D3D11_USAGE_DEFAULT;
                sdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                D3D11_SUBRESOURCE_DATA ssub{};
                ssub.pSysMem = spixels;
                ssub.SysMemPitch = sw * 4;

                ID3D11Texture2D *stexture = nullptr;
                bloodstrike::renderer::deviceInstance->CreateTexture2D( &sdesc, &ssub, &stexture );
                bloodstrike::renderer::deviceInstance->CreateShaderResourceView( stexture, nullptr, &bloodstrike::renderer::settings_icon_srv );
                stexture->Release( );
                stbi_image_free( spixels );
            }
            ImGui::SetSettingsIconTexture((ImTextureID)bloodstrike::renderer::settings_icon_srv);
        }

        {
            int mw, mh, mch = 0;
            unsigned char *mpixels = stbi_load("F:\\CARRERA CHEAT BLOODSTRIKE\\junior\\ESP MOHA.png", &mw, &mh, &mch, 4);
            if (mpixels)
            {
                D3D11_TEXTURE2D_DESC mdesc{};
                mdesc.Width = mw;
                mdesc.Height = mh;
                mdesc.MipLevels = 1;
                mdesc.ArraySize = 1;
                mdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                mdesc.SampleDesc.Count = 1;
                mdesc.Usage = D3D11_USAGE_DEFAULT;
                mdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                D3D11_SUBRESOURCE_DATA msub{};
                msub.pSysMem = mpixels;
                msub.SysMemPitch = mw * 4;

                ID3D11Texture2D *mtexture = nullptr;
                bloodstrike::renderer::deviceInstance->CreateTexture2D( &mdesc, &msub, &mtexture );
                bloodstrike::renderer::deviceInstance->CreateShaderResourceView( mtexture, nullptr, &bloodstrike::renderer::moha_srv );
                mtexture->Release( );
                stbi_image_free( mpixels );
            }
        }

        return oPresent( pSwapChain, SyncInterval, Flags );
    }

    ImGuiStyle &style = ImGui::GetStyle( );
    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_TitleBg] = ImVec4( 0.18f, 0.12f, 0.20f, 1.00f );


    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc( &desc );

    ImGuiIO &io = ImGui::GetIO( );

    if ( hooks::g_needsResize )
    {
        ID3D11Texture2D *backBuffer = nullptr;
        pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void **)&backBuffer );
        if ( backBuffer )
        {
            bloodstrike::renderer::deviceInstance->CreateRenderTargetView( backBuffer, nullptr, &bloodstrike::renderer::rtv );
            backBuffer->Release( );
            ImGui_ImplDX11_InvalidateDeviceObjects( );
            ImGui_ImplDX11_CreateDeviceObjects( );
        }

        D3D11_VIEWPORT vp;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = static_cast<FLOAT>( hooks::g_newWidth );
        vp.Height = static_cast<FLOAT>( hooks::g_newHeight );
        vp.MinDepth = 0.f;
        vp.MaxDepth = 1.f;
        bloodstrike::renderer::contextInstance->RSSetViewports( 1, &vp );

        ImGuiIO &io = ImGui::GetIO( );
        io.DisplaySize.x = static_cast<float>( hooks::g_newWidth );
        io.DisplaySize.y = static_cast<float>( hooks::g_newHeight );

        hooks::g_needsResize = false;
    }


    ImGui_ImplDX11_NewFrame( );
    ImGui_ImplWin32_NewFrame( );
    ImGui::NewFrame( );

    CheatTick();

    if ( GetAsyncKeyState( VK_INSERT ) & 1 )
    {
		if (global::menu::streamproof) global::menu::streamproof = false;
		menuOpen = !menuOpen;
		if (menuOpen) {
			global::menu::menuFadeIn = 0.f;
			global::menu::menuFirstFrame = true;
		}
    }

    if ( ( GetAsyncKeyState( VK_DELETE ) & 0x8000 ) != 0 )
    {
        g_shouldUnload = true;
    }

    if ( menuOpen )
    {
        static int iTabs = 0;
        static ImVec2 winPos = ImVec2(0, 0);
        static bool dragging = false;
        static ImVec2 dragOff;

        if (global::menu::menuFirstFrame) {
            ImVec2 ds2 = ImGui::GetIO().DisplaySize;
            winPos = ImVec2(ds2.x * 0.5f - 175.f, ds2.y * 0.5f - 125.f);
            global::menu::menuFirstFrame = false;
        }

        float dt2 = ImGui::GetIO().DeltaTime;
        if (global::menu::menuFadeIn < 1.f) global::menu::menuFadeIn = ImMin(global::menu::menuFadeIn + dt2 * 3.f, 1.f);
        float me = 1.f - powf(1.f - global::menu::menuFadeIn, 4.f);

        ImGui::SetNextWindowSize(ImVec2(420, 380));
        ImGui::SetNextWindowPos(winPos);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, me);
        ImGui::Begin("BLOODSTRIKE INTERNAL", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

        auto draw = ImGui::GetWindowDrawList();
        const auto& p = ImGui::GetWindowPos();
        winPos = p;

        ImVec2 mouse = ImGui::GetMousePos();
        bool inTitle = mouse.x >= p.x && mouse.x <= p.x + 420 && mouse.y >= p.y && mouse.y <= p.y + 30;
        bool inClose = mouse.x >= p.x + 392 && mouse.x <= p.x + 416 && mouse.y >= p.y + 4 && mouse.y <= p.y + 26;

        if (ImGui::IsMouseReleased(0) && inClose) exit(-1);

        if (inTitle && !inClose && ImGui::IsMouseClicked(0)) { dragging = true; dragOff = ImVec2(mouse.x - p.x, mouse.y - p.y); }
        if (dragging) {
            if (ImGui::IsMouseDown(0)) winPos = ImVec2(mouse.x - dragOff.x, mouse.y - dragOff.y);
            else dragging = false;
        }

        draw->AddRectFilled(p, p + ImVec2(420, 380), IM_COL32(0, 0, 0, 245), 8.f);
        draw->AddRect(p, p + ImVec2(420, 380), IM_COL32(150, 0, 220, 80), 8.f, 0, 1.5f);

        {
            struct RainDrop { float x, y, speed, len; };
            static RainDrop rain[60];
            static bool init = false;
            if (!init) {
                for (int i = 0; i < 60; i++) {
                    rain[i].x = (float)(rand() % 420);
                    rain[i].y = (float)(rand() % 380);
                    rain[i].speed = 150.f + (float)(rand() % 250);
                    rain[i].len = 6.f + (float)(rand() % 10);
                }
                init = true;
            }
            float dt = ImGui::GetIO().DeltaTime;
            for (int i = 0; i < 60; i++) {
                rain[i].y += rain[i].speed * dt;
                if (rain[i].y > 380.f) {
                    rain[i].y = -rain[i].len;
                    rain[i].x = (float)(rand() % 420);
                }
                float rx = p.x + rain[i].x;
                float ry = p.y + rain[i].y;
                draw->AddLine(ImVec2(rx, ry), ImVec2(rx - 0.5f, ry + rain[i].len), IM_COL32(100, 140, 255, 90), 1.f);
            }
        }

        draw->AddRectFilled(p + ImVec2(0, 0), p + ImVec2(420, 30), IM_COL32(100, 0, 160, 60), 8.f, ImDrawFlags_RoundCornersTop);
        draw->AddRectFilled(p + ImVec2(0, 26), p + ImVec2(420, 28), IM_COL32(150, 0, 220, 60), 0.f);
        float gt = ImGui::GetTime();
        ImU32 titleGlow = IM_COL32(180, 0, 255, (int)(80 + sinf(gt * 2.f) * 40));
        draw->AddText(ImVec2(p.x + 12, p.y + 6), titleGlow, "CARRERA");

        draw->AddRectFilled(p + ImVec2(394, 6), p + ImVec2(414, 24), IM_COL32(200, 30, 30, 180), 3.f);
        draw->AddText(ImVec2(p.x + 399, p.y + 5), IM_COL32(255, 255, 255, 200), "X");

        const char* tabNames[] = { "AIM", "ESP", "SET" };
        for (int i = 0; i < 3; i++) {
            float tx = p.x + 8 + i * 136;
            bool active = iTabs == i;
            ImU32 tabBg = active ? IM_COL32(150, 0, 220, 100) : IM_COL32(30, 15, 45, 60);
            draw->AddRectFilled(ImVec2(tx, p.y + 34), ImVec2(tx + 132, p.y + 54), tabBg, 4.f);
            if (active) draw->AddRectFilled(ImVec2(tx, p.y + 52), ImVec2(tx + 132, p.y + 53), IM_COL32(200, 50, 255, 200), 0.f);
            ImU32 textCol = active ? IM_COL32(220, 180, 255, 255) : IM_COL32(150, 120, 180, 200);
            float tw = ImGui::CalcTextSize(tabNames[i]).x;
            draw->AddText(ImVec2(tx + (132 - tw) * 0.5f, p.y + 36), textCol, tabNames[i]);
            if (ImGui::IsMouseReleased(0) && mouse.x >= tx && mouse.x <= tx + 132 && mouse.y >= p.y + 34 && mouse.y <= p.y + 54)
                iTabs = i;
        }

        ImGui::SetCursorPos(ImVec2(18, 65));
        {
            float cx = 18.f;
            if (iTabs == 0)
            {
                ImGui::SetCursorPosX(cx); ImGui::Checkbox("Aimbot On", &global::aim::enabled);
                ImGui::SetCursorPosX(cx); ImGui::Checkbox("Draw Fov", &global::aim::drawFov);
                ImGui::SetCursorPosX(cx); ImGui::SliderFloat("Fov", &global::aim::fov, 10, 1000, "%.0f");
                ImGui::SetCursorPosX(cx); ImGui::SliderFloat("Smooth", &global::aim::smooth, 1, 100, "%.0f");
            }
            else if (iTabs == 1)
            {
                ImGui::SetCursorPosX(cx); ImGui::Checkbox("ESP Enabled", &global::esp::enabled);
                ImGui::SetCursorPosX(cx); ImGui::Checkbox("ESP Box", &global::esp::box);
                ImGui::SameLine(); ImGui::SetCursorPosX(cx + 200); ImGui::ColorEdit4("##boxcol", global::esp::boxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview);
                ImGui::SetCursorPosX(cx); ImGui::Checkbox("ESP Skeleton", &global::esp::skeleton);
                ImGui::SameLine(); ImGui::SetCursorPosX(cx + 200); ImGui::ColorEdit4("##skelcol", global::esp::skeletonColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview);
                ImGui::SetCursorPosX(cx); ImGui::Checkbox("ESP Line", &global::esp::line);
                ImGui::SameLine(); ImGui::SetCursorPosX(cx + 200); ImGui::ColorEdit4("##linecol", global::esp::lineColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview);
            }
            else if (iTabs == 2)
            {
                ImGui::SetCursorPosX(cx); ImGui::Checkbox("streamproof", &global::menu::streamproof);
                ImGui::SetCursorPosX(cx);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
                if (ImGui::Button("exit", ImVec2(70, 22))) exit(-1);
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    ImGui::Render( );
    bloodstrike::renderer::contextInstance->OMSetRenderTargets( 1, &bloodstrike::renderer::rtv, nullptr );
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );

    return oPresent( pSwapChain, SyncInterval, Flags );
}


void Thread( HMODULE hModule )
{
    g_hModule = hModule;
    if ( !findPresent( ) || MH_Initialize() != MH_OK )
    {
#ifdef DEBUG
        printf( "[-] failed to initalize d3d11..." );
#endif
		cleanup( hModule );
        return;
    }

#ifdef DEBUG
    printf( "[+] present %llX\n", aPresent );
#endif

    MH_STATUS status = MH_CreateHook(
        (LPVOID)aPresent,
        &hkPresent,
        (void **)&oPresent
    );
    status = MH_EnableHook( (LPVOID)aPresent );

#ifdef DEBUG
    printf( "hooked Present -> %d\n", status );
#endif

    status = MH_CreateHook(
        (LPVOID)aResizeBuffers,
        &hkResizeBuffers,
        (void **)&oResizeBuffers
    );
    status = MH_EnableHook( (LPVOID)aResizeBuffers );

    HMODULE hUser32 = GetModuleHandleW( L"user32.dll" );
    if ( !hUser32 ) hUser32 = LoadLibraryW( L"user32.dll" );
    if ( !hUser32 )
    {
        cleanup( hModule );
        return;
    }

    aGetRawInputData = (uint64_t)GetProcAddress( hUser32, "GetRawInputData" );

    status = MH_CreateHook(
        (LPVOID)aGetRawInputData,
        &hkGetRawInputData,
        (void **)&oGetRawInputData
    );
    status = MH_EnableHook( (LPVOID)aGetRawInputData );

    while ( !g_shouldUnload )
    {
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    }

    cleanup( hModule );
}


BOOL APIENTRY DllMain( HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        base = (uint64_t)GetModuleHandleA( NULL );
        srand( static_cast<unsigned int>( time( nullptr ) ) );

#ifdef DEBUG
        AllocConsole( );
        freopen_s( &f, "CONOUT$", "w", stdout );
        freopen_s( &f, "CONIN$", "r", stdin );
        printf( "[+] init\n" );
#endif

        CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)Thread, hModule, NULL, NULL );

        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
