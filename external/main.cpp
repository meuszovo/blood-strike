#pragma comment(lib, "d3d11.lib")

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "memory.hpp"
#include "game.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
static HWND g_hOverlayWnd = nullptr;
static HWND g_hGameWnd = nullptr;
static bool g_menuOpen = true;
static bool g_running = true;

static const char* GAME_CLASS = "UnityWndClass";
static const char* GAME_TITLE = "BloodStrike";

HWND FindGameWindow()
{
    HWND hwnd = nullptr;
    EnumWindows([](HWND h, LPARAM lp) -> BOOL {
        DWORD pid = 0;
        if (GetWindowThreadProcessId(h, &pid) && pid == mem::pid)
        {
            *reinterpret_cast<HWND*>(lp) = h;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&hwnd));
    return hwnd;
}

namespace settings
{
    bool aimbot = true;
    bool drawFov = true;
    float fov = 175.f;
    float smooth = 5.f;
    bool shooting = false;

    bool esp = true;
    bool espBox = true;
    bool espSkeleton = true;
    bool espLine = true;
    float boxColor[4] = { 1.f, 0.f, 1.f, 1.f };
    float skeletonColor[4] = { 1.f, 0.f, 1.f, 1.f };
    float lineColor[4] = { 1.f, 0.f, 1.f, 1.f };

    float panelColor[4] = { 0.51f, 0.08f, 0.71f, 1.f };
    bool streamproof = false;
    int menuKey = VK_INSERT;
}

void CleanupDeviceD3D()
{
    if (g_pRenderTargetView) { g_pRenderTargetView->Release(); g_pRenderTargetView = nullptr; }
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer)
    {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
        pBackBuffer->Release();
    }
}

static bool CreateDeviceD3D(HWND hWnd)
{
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL levels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2,
        D3D11_SDK_VERSION, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (FAILED(hr)) return false;

    IDXGIDevice* pDXGIDevice = nullptr;
    hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
    if (FAILED(hr)) return false;

    IDXGIAdapter* pAdapter = nullptr;
    hr = pDXGIDevice->GetAdapter(&pAdapter);
    pDXGIDevice->Release();
    if (FAILED(hr)) return false;

    IDXGIFactory* pFactory = nullptr;
    hr = pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pFactory);
    pAdapter->Release();
    if (FAILED(hr)) return false;

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    hr = pFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
    pFactory->Release();
    if (FAILED(hr)) return false;

    CreateRenderTarget();
    return true;
}

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void UpdateOverlayMouse()
{
    ImGuiIO& io = ImGui::GetIO();
    POINT pt;
    if (GetCursorPos(&pt) && ScreenToClient(g_hOverlayWnd, &pt))
    {
        io.MousePos = ImVec2((float)pt.x, (float)pt.y);
    }
    io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    io.MouseDown[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
}

void MoveOverlay()
{
    if (!g_hGameWnd) return;
    RECT rc;
    if (!GetClientRect(g_hGameWnd, &rc)) return;
    POINT topLeft = { rc.left, rc.top };
    ClientToScreen(g_hGameWnd, &topLeft);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    LONG exStyle = GetWindowLongA(g_hOverlayWnd, GWL_EXSTYLE);
    if (g_menuOpen)
        exStyle &= ~WS_EX_TRANSPARENT;
    else
        exStyle |= WS_EX_TRANSPARENT;
    SetWindowLongA(g_hOverlayWnd, GWL_EXSTYLE, exStyle);
    SetWindowPos(g_hOverlayWnd, HWND_TOPMOST, topLeft.x, topLeft.y, w, h, SWP_NOACTIVATE);

    static bool lastStreamproof = false;
    if (settings::streamproof != lastStreamproof)
    {
        SetWindowDisplayAffinity(g_hOverlayWnd, settings::streamproof ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
        lastStreamproof = settings::streamproof;
    }
}

void InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    if (!font) io.Fonts->AddFontDefault();

    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowPadding = ImVec2(0, 0);
    s.FramePadding = ImVec2(12, 12);
    s.ItemSpacing = ImVec2(10, 10);
    s.FrameRounding = 4.f;
    s.WindowRounding = 10.f;
    s.WindowBorderSize = 0.f;
    s.Colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.01f, 0.03f, 1.f);
    s.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.04f, 0.20f, 1.f);
    s.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.06f, 0.30f, 1.f);
    s.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.08f, 0.38f, 1.f);
    s.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.f, 1.f, 1.f);
    s.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.85f, 0.1f, 1.f, 1.f);
    s.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.04f, 0.25f, 1.f);
    s.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.06f, 0.40f, 1.f);
    s.Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.08f, 0.55f, 1.f);
    s.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.f);

    ImGui_ImplWin32_Init(g_hOverlayWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}

void DrawStyledLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness)
{
    dl->AddLine(a, b, col, thickness);
}

void RenderESP()
{
    if (!settings::esp) return;

    std::vector<game::EntityData> entities;
    if (!game::ReadEntityList(entities)) return;

    ImDrawList* bg = ImGui::GetBackgroundDrawList();

    for (auto& ent : entities)
    {
        if (!ent.valid) continue;

        float distM = ent.distance / 5.f;

        game::BoneData bones;
        if (settings::espSkeleton)
        {
            game::ReadBones(ent.actorInstance, ent.IEntity, bones);
        }

        if (settings::espSkeleton && bones.valid)
        {
            ImU32 skelCol = IM_COL32((int)(settings::skeletonColor[0] * 255), (int)(settings::skeletonColor[1] * 255), (int)(settings::skeletonColor[2] * 255), (int)(settings::skeletonColor[3] * 255));
            DrawStyledLine(bg, ImVec2(bones.neck.x, bones.neck.y), ImVec2(bones.spine1.x, bones.spine1.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.spine1.x, bones.spine1.y), ImVec2(bones.spine2.x, bones.spine2.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.spine2.x, bones.spine2.y), ImVec2(bones.spine3.x, bones.spine3.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.spine3.x, bones.spine3.y), ImVec2(bones.pelvis.x, bones.pelvis.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.spine1.x, bones.spine1.y), ImVec2(bones.sholL.x, bones.sholL.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.sholL.x, bones.sholL.y), ImVec2(bones.elbowL.x, bones.elbowL.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.elbowL.x, bones.elbowL.y), ImVec2(bones.wristL.x, bones.wristL.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.spine1.x, bones.spine1.y), ImVec2(bones.sholR.x, bones.sholR.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.sholR.x, bones.sholR.y), ImVec2(bones.elbowR.x, bones.elbowR.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.elbowR.x, bones.elbowR.y), ImVec2(bones.wristR.x, bones.wristR.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.pelvis.x, bones.pelvis.y), ImVec2(bones.buttCheekL.x, bones.buttCheekL.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.buttCheekL.x, bones.buttCheekL.y), ImVec2(bones.kneeL.x, bones.kneeL.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.kneeL.x, bones.kneeL.y), ImVec2(bones.footL.x, bones.footL.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.pelvis.x, bones.pelvis.y), ImVec2(bones.buttCheekR.x, bones.buttCheekR.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.buttCheekR.x, bones.buttCheekR.y), ImVec2(bones.kneeR.x, bones.kneeR.y), skelCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.kneeR.x, bones.kneeR.y), ImVec2(bones.footR.x, bones.footR.y), skelCol, 1.f);
        }

        if (settings::espBox && bones.valid)
        {
            float MIN_ESP = 15.f;
            float height = fabsf(bones.neck.y - ent.screenPos.y);
            if (height < MIN_ESP) height = MIN_ESP;
            float width = height * 0.4f;
            ImU32 boxCol = IM_COL32((int)(settings::boxColor[0] * 255), (int)(settings::boxColor[1] * 255), (int)(settings::boxColor[2] * 255), (int)(settings::boxColor[3] * 255));
            DrawStyledLine(bg, ImVec2(bones.neck.x - width, bones.neck.y), ImVec2(bones.neck.x + width, bones.neck.y), boxCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.neck.x + width, bones.neck.y), ImVec2(bones.neck.x + width, bones.neck.y + height), boxCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.neck.x + width, bones.neck.y + height), ImVec2(bones.neck.x - width, bones.neck.y + height), boxCol, 1.f);
            DrawStyledLine(bg, ImVec2(bones.neck.x - width, bones.neck.y + height), ImVec2(bones.neck.x - width, bones.neck.y), boxCol, 1.f);
        }

        if (settings::espLine)
        {
            ImU32 lineCol = IM_COL32((int)(settings::lineColor[0] * 255), (int)(settings::lineColor[1] * 255), (int)(settings::lineColor[2] * 255), (int)(settings::lineColor[3] * 255));
            ImVec2 ds = ImGui::GetIO().DisplaySize;
            DrawStyledLine(bg, ImVec2(ds.x * 0.5f, ds.y), ImVec2(ent.screenPos.x, ent.screenPos.y), lineCol, 1.f);
        }
    }
}

void RenderAimbot()
{
    if (!settings::aimbot) return;
    if (g_menuOpen) return;

    bool shooting = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (!shooting) return;

    if (GetForegroundWindow() != g_hGameWnd) return;

    static ULONGLONG lastAimTick = 0;
    ULONGLONG now = GetTickCount64();
    if (now - lastAimTick < 16) return;
    lastAimTick = now;

    game::Vec2 target;
    float screenCX = ImGui::GetIO().DisplaySize.x * 0.5f;
    float screenCY = ImGui::GetIO().DisplaySize.y * 0.5f;
    if (!game::GetAimTarget(screenCX, screenCY, target, settings::fov)) return;

    float targetX = target.x - screenCX;
    float targetY = target.y - screenCY;

    float speed = 100.f / settings::smooth;
    float moveX = targetX;
    float moveY = targetY;
    if (fabsf(moveX) > speed) moveX = speed * (moveX > 0.f ? 1.f : -1.f);
    if (fabsf(moveY) > speed) moveY = speed * (moveY > 0.f ? 1.f : -1.f);

    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dx = (LONG)moveX;
    input.mi.dy = (LONG)moveY;
    input.mi.mouseData = 0;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}

static ImU32 pcolA(int alpha) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(settings::panelColor[0], settings::panelColor[1], settings::panelColor[2], alpha / 255.f));
}
static ImU32 pcolBright(float mul) {
    float r = settings::panelColor[0] * mul, g = settings::panelColor[1] * mul, b = settings::panelColor[2] * mul;
    if (r > 1.f) r = 1.f; if (g > 1.f) g = 1.f; if (b > 1.f) b = 1.f;
    return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.f));
}
static ImU32 pcolBase() {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(settings::panelColor[0], settings::panelColor[1], settings::panelColor[2], 1.f));
}

void RenderFovCircle()
{
    if (!settings::drawFov) return;
    float cx = ImGui::GetIO().DisplaySize.x * 0.5f;
    float cy = ImGui::GetIO().DisplaySize.y * 0.5f;
    ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(cx, cy), settings::fov, IM_COL32(150, 0, 255, 200), 64, 1.f);
}

void RenderMenu()
{
    if (GetAsyncKeyState(VK_INSERT) & 1)
        g_menuOpen = !g_menuOpen;

    if (!g_menuOpen) return;

    ImGuiIO& io = ImGui::GetIO();

    auto CustomSlider = [](const char* label, float* value, float min, float max, const char* fmt = "%.1f") -> bool
    {
        float w = 384.f;
        float h = 22.f;

        ImVec2 pos = ImGui::GetCursorScreenPos();
        float t = (*value - min) / (max - min);
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImU32 barBg = IM_COL32(25, 8, 35, 255);
        ImU32 barFill = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::panelColor[0], settings::panelColor[1], settings::panelColor[2], 1.f));
        ImU32 textCol = IM_COL32(220, 220, 220, 255);
        ImU32 valCol = IM_COL32(180, 180, 180, 255);

        ImVec2 barMin(pos.x, pos.y + 16.f);
        ImVec2 barMax(pos.x + w, pos.y + 16.f + h);

        dl->AddRectFilled(barMin, barMax, barBg, 4.f);
        dl->AddRectFilled(barMin, ImVec2(barMin.x + t * w, barMax.y), barFill, 4.f);

        dl->AddText(ImVec2(pos.x, pos.y), textCol, label);

        char valBuf[32];
        snprintf(valBuf, sizeof(valBuf), fmt, *value);
        float valW = ImGui::CalcTextSize(valBuf).x;
        dl->AddText(ImVec2(barMax.x - valW, pos.y), valCol, valBuf);

        ImGui::SetCursorScreenPos(barMin);
        ImGui::InvisibleButton(label, ImVec2(w, h));
        bool changed = false;
        if (ImGui::IsItemActive())
        {
            float mouse_x = ImGui::GetIO().MousePos.x;
            float new_t = (mouse_x - pos.x) / w;
            if (new_t < 0.f) new_t = 0.f;
            if (new_t > 1.f) new_t = 1.f;
            float newVal = min + new_t * (max - min);
            if (newVal != *value)
            {
                *value = newVal;
                changed = true;
            }
        }
        ImGui::SetCursorScreenPos(ImVec2(pos.x, barMax.y + 8.f));
        ImGui::Dummy(ImVec2(0, 0));
        return changed;
    };

    auto CustomCheckbox = [](const char* label, bool* v) -> bool
    {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float boxSize = 24.f;
        float spacing = 10.f;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 boxMin(pos.x, pos.y + 2.f);
        ImVec2 boxMax(pos.x + boxSize, pos.y + 2.f + boxSize);

        ImU32 boxBg = *v ? ImGui::ColorConvertFloat4ToU32(ImVec4(settings::panelColor[0] * 0.8f, settings::panelColor[1] * 0.8f, settings::panelColor[2] * 0.8f, 1.f)) : IM_COL32(30, 15, 40, 255);
        ImU32 border = *v ? pcolBase() : IM_COL32(80, 50, 100, 150);
        dl->AddRectFilled(boxMin, boxMax, boxBg, 5.f);
        dl->AddRect(boxMin, boxMax, border, 5.f, 0, 1.5f);

        if (*v)
        {
            ImVec2 p1(pos.x + 6.f,  pos.y + 14.f);
            ImVec2 p2(pos.x + 10.f, pos.y + 19.f);
            ImVec2 p3(pos.x + 18.f, pos.y + 8.f);
            dl->AddLine(p1, p2, IM_COL32(255, 255, 255, 255), 2.5f);
            dl->AddLine(p2, p3, IM_COL32(255, 255, 255, 255), 2.5f);
        }

        float textH = ImGui::CalcTextSize(label).y;
        dl->AddText(ImVec2(boxMax.x + spacing, pos.y + (boxSize - textH) * 0.5f + 2.f), IM_COL32(220, 220, 220, 255), label);

        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton(label, ImVec2(boxSize + spacing + ImGui::CalcTextSize(label).x, boxSize + 4.f));
        bool changed = false;
        if (ImGui::IsItemClicked())
        {
            *v = !*v;
            changed = true;
        }
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + boxSize + 2.f));
        ImGui::Dummy(ImVec2(0, 0));
        return changed;
    };

    auto DrawColorSwatch = [](const char* id, ImVec2 textEndPos, float* col) -> bool
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float sz = 14.f;
        float gap = 8.f;
        ImVec2 a(textEndPos.x + gap, textEndPos.y);
        ImVec2 b(a.x + sz, a.y + sz);
        dl->AddRectFilled(a, b, ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], col[3])), 3.f);
        dl->AddRect(a, b, IM_COL32(200, 200, 200, 100), 3.f, 0, 1.f);

        bool clicked = false;
        ImVec2 mp = ImGui::GetIO().MousePos;
        if (ImGui::GetIO().MouseClicked[0] && mp.x >= a.x && mp.x <= b.x && mp.y >= a.y && mp.y <= b.y)
        {
            ImVec2 cp = ImGui::GetIO().MousePos;
            ImGui::SetNextWindowPos(ImVec2(cp.x - 90.f, cp.y));
            ImGui::OpenPopup(id);
            clicked = true;
        }

        ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(20, 10, 30, 255));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
        if (ImGui::BeginPopup(id))
        {
            ImGui::ColorPicker4("##cp", col, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview);
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
        return clicked;
    };
    ImVec2 winPos(io.DisplaySize.x * 0.5f - 210.f, io.DisplaySize.y * 0.5f - 190.f);

    ImGui::SetNextWindowSize(ImVec2(420, 380));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
    ImGui::Begin("BLOODSTRIKE INTERNAL", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

    auto draw = ImGui::GetWindowDrawList();
    const auto& p = ImGui::GetWindowPos();

    draw->AddRectFilled(p, p + ImVec2(420, 380), IM_COL32(5, 2, 8, 255), 8.f);
    draw->AddRect(p, p + ImVec2(420, 380), pcolA(200), 8.f, 0, 1.5f);

    {
        struct RainDrop { float x, y, speed, len; };
        static RainDrop rain[60];
        static bool init = false;
        if (!init) {
            srand(42);
            for (int i = 0; i < 60; i++) {
                rain[i].x = (float)(rand() % 420);
                rain[i].y = (float)(rand() % 380);
                rain[i].speed = 150.f + (float)(rand() % 250);
                rain[i].len = 6.f + (float)(rand() % 10);
            }
            init = true;
        }
        float dt = io.DeltaTime;
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

    draw->AddRectFilled(p + ImVec2(0, 0), p + ImVec2(420, 30), pcolA(60), 8.f, ImDrawFlags_RoundCornersTop);
    draw->AddRectFilled(p + ImVec2(0, 26), p + ImVec2(420, 28), pcolA(60), 0.f);
    float gt = ImGui::GetTime();
    ImU32 titleGlow = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::panelColor[0], settings::panelColor[1], settings::panelColor[2], (80 + sinf(gt * 2.f) * 40) / 255.f));
    draw->AddText(ImVec2(p.x + 12, p.y + 6), titleGlow, "CARRERA");

    draw->AddRectFilled(p + ImVec2(394, 6), p + ImVec2(414, 24), IM_COL32(200, 30, 30, 180), 3.f);
    draw->AddText(ImVec2(p.x + 399, p.y + 5), IM_COL32(255, 255, 255, 200), "X");

    static int iTab = 0;
    for (int i = 0; i < 3; i++) {
        float tx = p.x + 4;
        float tabW = 48.f;
        float tabH = 48.f;
        float tabY = p.y + 34 + i * 54;
        bool active = iTab == i;
        ImU32 tabBg = active ? pcolA(100) : IM_COL32(30, 15, 45, 60);
        draw->AddRectFilled(ImVec2(tx, tabY), ImVec2(tx + tabW, tabY + tabH), tabBg, 6.f);
        if (active) draw->AddRectFilled(ImVec2(tx + tabW - 2.f, tabY), ImVec2(tx + tabW - 1.f, tabY + tabH), pcolA(200), 0.f);
        ImU32 iconCol = active ? pcolBright(1.3f) : IM_COL32(150, 120, 180, 200);
        float iconCx = tx + tabW * 0.5f;
        float iconCy = tabY + tabH * 0.5f;

        if (i == 0)
        {
            float r = 8.f;
            draw->AddCircle(ImVec2(iconCx, iconCy), r, iconCol, 0, 1.5f);
            draw->AddCircleFilled(ImVec2(iconCx, iconCy), 1.5f, iconCol, 0);
            float g = 3.f, l = r - 1.f;
            draw->AddLine(ImVec2(iconCx, iconCy - g), ImVec2(iconCx, iconCy - l), iconCol, 1.5f);
            draw->AddLine(ImVec2(iconCx, iconCy + g), ImVec2(iconCx, iconCy + l), iconCol, 1.5f);
            draw->AddLine(ImVec2(iconCx - g, iconCy), ImVec2(iconCx - l, iconCy), iconCol, 1.5f);
            draw->AddLine(ImVec2(iconCx + g, iconCy), ImVec2(iconCx + l, iconCy), iconCol, 1.5f);
        }
        else if (i == 1)
        {
            float ew = 10.f, eh = 6.f;
            ImVec2 left(iconCx - ew, iconCy), right(iconCx + ew, iconCy);
            draw->AddBezierQuadratic(left, ImVec2(iconCx - ew * 0.4f, iconCy - eh), ImVec2(iconCx + ew * 0.4f, iconCy - eh), iconCol, 1.5f);
            draw->AddBezierQuadratic(ImVec2(iconCx + ew * 0.4f, iconCy - eh), right, ImVec2(iconCx + ew * 0.4f, iconCy + eh), iconCol, 1.5f);
            draw->AddBezierQuadratic(ImVec2(iconCx + ew * 0.4f, iconCy + eh), ImVec2(iconCx - ew * 0.4f, iconCy + eh), left, iconCol, 1.5f);
            draw->AddBezierQuadratic(ImVec2(iconCx - ew * 0.4f, iconCy + eh), left, ImVec2(iconCx - ew * 0.4f, iconCy - eh), iconCol, 1.5f);
            draw->AddCircleFilled(ImVec2(iconCx, iconCy), 3.5f, iconCol, 16);
            draw->AddCircleFilled(ImVec2(iconCx, iconCy), 1.5f, IM_COL32(5, 2, 8, 255), 12);
        }
        else if (i == 2)
        {
            float r = 7.f, ri = 4.5f, th = 2.5f;
            draw->AddCircleFilled(ImVec2(iconCx, iconCy), ri, iconCol, 20);
            draw->AddCircleFilled(ImVec2(iconCx, iconCy), ri - 2.f, active ? pcolA(100) : IM_COL32(30, 15, 45, 60), 20);
            for (int t = 0; t < 6; t++)
            {
                float a1 = (3.14159f * 2.f * t) / 6 - 0.25f;
                float a2 = (3.14159f * 2.f * t) / 6 + 0.25f;
                float or_ = r + th;
                ImVec2 p1(iconCx + cosf(a1) * ri, iconCy + sinf(a1) * ri);
                ImVec2 p2(iconCx + cosf(a1) * or_, iconCy + sinf(a1) * or_);
                ImVec2 p3(iconCx + cosf(a2) * or_, iconCy + sinf(a2) * or_);
                ImVec2 p4(iconCx + cosf(a2) * ri, iconCy + sinf(a2) * ri);
                draw->AddQuadFilled(p1, p2, p3, p4, iconCol);
            }
        }

        ImVec2 mouse = io.MousePos;
        if (io.MouseReleased[0] && mouse.x >= tx && mouse.x <= tx + tabW && mouse.y >= tabY && mouse.y <= tabY + tabH)
            iTab = i;
    }

    draw->AddLine(ImVec2(p.x + 56, p.y + 30), ImVec2(p.x + 56, p.y + 370), IM_COL32(100, 50, 150, 80), 1.f);

    ImVec2 closeMin(p.x + 394, p.y + 6);
    ImVec2 closeMax(p.x + 414, p.y + 24);
    if (io.MouseReleased[0] && io.MousePos.x >= closeMin.x && io.MousePos.x <= closeMax.x &&
        io.MousePos.y >= closeMin.y && io.MousePos.y <= closeMax.y)
        g_running = false;

    ImGui::SetCursorPos(ImVec2(68, 34));
    float cx = 68.f;
    float contentW = 340.f;

    if (iTab == 0)
    {
        ImGui::SetCursorPosX(cx); CustomCheckbox("Aimbot On", &settings::aimbot);
        ImGui::SetCursorPosX(cx); CustomCheckbox("Draw Fov", &settings::drawFov);

        ImGui::SetCursorPosX(cx); CustomSlider("Aimbot Fov", &settings::fov, 10.f, 1000.f, "%.0f");
        ImGui::SetCursorPosX(cx); CustomSlider("Aimbot Smoothness", &settings::smooth, 1.f, 100.f, "%.1f");
    }
    else if (iTab == 1)
    {
        ImGui::SetCursorPosX(cx); CustomCheckbox("ESP Enabled", &settings::esp);
        DrawColorSwatch("##enbsw", ImVec2(p.x + 392.f, ImGui::GetCursorScreenPos().y - 26.f), settings::boxColor);

        ImGui::SetCursorPosX(cx); CustomCheckbox("ESP Box", &settings::espBox);
        DrawColorSwatch("##boxsw", ImVec2(p.x + 392.f, ImGui::GetCursorScreenPos().y - 26.f), settings::boxColor);

        ImGui::SetCursorPosX(cx); CustomCheckbox("ESP Skeleton", &settings::espSkeleton);
        DrawColorSwatch("##skelsw", ImVec2(p.x + 392.f, ImGui::GetCursorScreenPos().y - 26.f), settings::skeletonColor);

        ImGui::SetCursorPosX(cx); CustomCheckbox("ESP Line", &settings::espLine);
        DrawColorSwatch("##linesw", ImVec2(p.x + 392.f, ImGui::GetCursorScreenPos().y - 26.f), settings::lineColor);
    }
    else if (iTab == 2)
    {
        ImGui::SetCursorPosX(cx); CustomCheckbox("Streamproof", &settings::streamproof);
        ImGui::SetCursorPosX(cx);
        ImGui::Text("Panel Color");
        ImGui::SetCursorPosX(cx); ImGui::ColorEdit4("##panelcolor", settings::panelColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaPreview);
        ImGui::SetCursorPosX(cx);
        if (ImGui::Button("Exit", ImVec2(120, 30)))
            g_running = false;
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    while (!mem::Attach(L"BloodStrike.exe"))
        Sleep(1000);

    g_hGameWnd = nullptr;
    while (!g_hGameWnd)
    {
        g_hGameWnd = FindGameWindow();
        if (!g_hGameWnd) Sleep(500);
    }

    RECT rc;
    GetClientRect(g_hGameWnd, &rc);
    POINT topLeft = { rc.left, rc.top };
    ClientToScreen(g_hGameWnd, &topLeft);

    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "OverlayClass";
    RegisterClassExA(&wc);

    g_hOverlayWnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        wc.lpszClassName, "Overlay",
        WS_POPUP | WS_VISIBLE,
        topLeft.x, topLeft.y, rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);

    SetLayeredWindowAttributes(g_hOverlayWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    if (!CreateDeviceD3D(g_hOverlayWnd))
    {
        CleanupDeviceD3D();
        MessageBoxA(nullptr, "D3D11 creation failed!", "Error", MB_OK | MB_ICONERROR);
        Sleep(3000);
        return 1;
    }

    InitImGui();

    MSG msg{};
    while (g_running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) g_running = false;
        }
        if (!g_running) break;

        MoveOverlay();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        UpdateOverlayMouse();

        ImGui::GetBackgroundDrawList()->AddText(ImVec2(10, 10), IM_COL32(255, 255, 0, 255), "OVERLAY OK");

        RenderESP();
        RenderFovCircle();
        RenderAimbot();
        RenderMenu();

        ImGui::Render();
        const float clear_color[4] = { 0.f, 0.f, 0.f, 0.f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(0, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(g_hOverlayWnd);

    return 0;
}
