#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <iostream>
#include <Windows.h>
#include <d3d11.h>
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <dwmapi.h>
#include "vectors.h"

inline ImFont* font_esp;
extern inline ImFont* small_font;
inline ImFont* big_font;

namespace draw
{
    void box(int x, int y, int w, int h, ImColor color, float thickness, bool outline)
    {
        ImDrawList* draw = ImGui::GetBackgroundDrawList();

        if (outline)
        {
            float outline_thick = thickness + 2.0f;
            draw->AddLine(ImVec2(x, y), ImVec2(x + w, y), (ImU32)ImColor(0, 0, 0), outline_thick);
            draw->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + h), (ImU32)ImColor(0, 0, 0), outline_thick);
            draw->AddLine(ImVec2(x + w, y + h), ImVec2(x, y + h), (ImU32)ImColor(0, 0, 0), outline_thick);
            draw->AddLine(ImVec2(x, y + h), ImVec2(x, y), (ImU32)ImColor(0, 0, 0), outline_thick);
        }

        draw->AddLine(ImVec2(x, y), ImVec2(x + w, y), color, thickness);
        draw->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + h), color, thickness);
        draw->AddLine(ImVec2(x + w, y + h), ImVec2(x, y + h), color, thickness);
        draw->AddLine(ImVec2(x, y + h), ImVec2(x, y), color, thickness);
    }

	void corner_box( int x, int y, int w, int h, ImColor color, float thickness, bool outline ) {

		if (outline)
		{
			ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x, y ), ImVec2( x, y + (h / 3) ), (ImU32)ImColor( 0, 0, 0 ), thickness + 2 );
			ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x, y ), ImVec2( x + (w / 3), y ), (ImU32)ImColor( 0, 0, 0 ), thickness + 2 );
			ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x + w - (w / 3), y ), ImVec2( x + w, y ), (ImU32)ImColor( 0, 0, 0 ), thickness + 2 );
			ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x + w, y ), ImVec2( x + w, y + (h / 3) ), (ImU32)ImColor( 0, 0, 0 ), thickness + 2 );
			ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x, y + h - (h / 3) ), ImVec2( x, y + h ), (ImU32)ImColor( 0, 0, 0 ), thickness + 2 );
			ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x, y + h ), ImVec2( x + (w / 3), y + h ), (ImU32)ImColor( 0, 0, 0 ), thickness + 2 );
			ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x + w - (w / 3), y + h ), ImVec2( x + w, y + h ), (ImU32)ImColor( 0, 0, 0 ), thickness + 2 );
			ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x + w, y + h - (h / 3) ), ImVec2( x + w, y + h ), (ImU32)ImColor( 0, 0, 0 ), thickness + 2 );
		}

		ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x, y ), ImVec2( x, y + (h / 3) ), color, thickness );
		ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x, y ), ImVec2( x + (w / 3), y ), color, thickness );
		ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x + w - (w / 3), y ), ImVec2( x + w, y ), color, thickness );
		ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x + w, y ), ImVec2( x + w, y + (h / 3) ), color, thickness );
		ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x, y + h - (h / 3) ), ImVec2( x, y + h ), color, thickness );
		ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x, y + h ), ImVec2( x + (w / 3), y + h ), color, thickness );
		ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x + w - (w / 3), y + h ), ImVec2( x + w, y + h ), color, thickness );
		ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( x + w, y + h - (h / 3) ), ImVec2( x + w, y + h ), color, thickness );
	}

    void draw_text_outlined( const ImVec2& pos, const ImColor& color, const char* text, bool outline )
    {
        if (outline) {
            ImGui::GetBackgroundDrawList( )->AddText( font_esp, ImGui::GetFontSize( ), ImVec2( pos.x - 1.0f, pos.y - 1.0f ), (ImU32)ImColor( 0, 0, 0, 200 ), text );
            ImGui::GetBackgroundDrawList( )->AddText( font_esp, ImGui::GetFontSize( ), ImVec2( pos.x + 1.0f, pos.y - 1.0f ), (ImU32)ImColor( 0, 0, 0, 200 ), text );
            ImGui::GetBackgroundDrawList( )->AddText( font_esp, ImGui::GetFontSize( ), ImVec2( pos.x - 1.0f, pos.y + 1.0f ), (ImU32)ImColor( 0, 0, 0, 200 ), text );
            ImGui::GetBackgroundDrawList( )->AddText( font_esp, ImGui::GetFontSize( ), ImVec2( pos.x + 1.0f, pos.y + 1.0f ), (ImU32)ImColor( 0, 0, 0, 200 ), text );
            ImGui::GetBackgroundDrawList( )->AddText( font_esp, ImGui::GetFontSize( ), ImVec2( pos.x - 1.0f, pos.y ), (ImU32)ImColor( 0, 0, 0, 200 ), text );
            ImGui::GetBackgroundDrawList( )->AddText( font_esp, ImGui::GetFontSize( ), ImVec2( pos.x + 1.0f, pos.y ), (ImU32)ImColor( 0, 0, 0, 200 ), text );
            ImGui::GetBackgroundDrawList( )->AddText( font_esp, ImGui::GetFontSize( ), ImVec2( pos.x, pos.y - 1.0f ), (ImU32)ImColor( 0, 0, 0, 200 ), text );
            ImGui::GetBackgroundDrawList( )->AddText( font_esp, ImGui::GetFontSize( ), ImVec2( pos.x, pos.y + 1.0f ), (ImU32)ImColor( 0, 0, 0, 200 ), text );
        }

        ImGui::GetBackgroundDrawList( )->AddText( font_esp, ImGui::GetFontSize( ), pos, color, text );
    }

    void draw_line( const fvector2d& a, const fvector2d& b, ImColor color, float thickness )
    {
        ImGui::GetBackgroundDrawList( )->AddLine( ImVec2( a.x, a.y ), ImVec2( b.x, b.y ), (ImU32)color, thickness );
    }

    void draw_watermark(int32_t spectator_count, bool show_spectator)
    {
        ImVec2 labelSize = ImGui::CalcTextSize("Ryvex External");

        // Hintergrund-Box
        ImGui::GetBackgroundDrawList()->AddRectFilled(
            { 15, 15 },
            { 15 + labelSize.x + 37, 15 + labelSize.y + 10 },
            IM_COL32(10, 10, 10, 210), 6.f
        );

        // Linker �Balken� lila statt blau
        ImGui::GetBackgroundDrawList()->AddRectFilled(
            { 15, 18 },
            { 18, 15 + labelSize.y + 7 },
            IM_COL32(128, 0, 255, 255), // Kr�ftiges Lila
            4.f, ImDrawFlags_RoundCornersRight
        );

        ImGui::PushFont(small_font);

        // Schrift bleibt wei�!
        ImGui::GetBackgroundDrawList()->AddText(
            { 23, 17 },
            IM_COL32(255, 255, 255, 255), // Wei�
            "Ryvex External"
        );

        // FPS-Box + Text
        std::string fpsText = "Fps: " + std::to_string((int)ImGui::GetIO().Framerate);
        ImVec2 fpsSize = ImGui::CalcTextSize(fpsText.c_str());
        float fpsX = 15 + labelSize.x + 45;

        ImGui::GetBackgroundDrawList()->AddRectFilled(
            { fpsX, 15 },
            { fpsX + fpsSize.x + 13, 15 + fpsSize.y + 3 },
            IM_COL32(10, 10, 10, 210), 6.f
        );
        ImGui::GetBackgroundDrawList()->AddText(
            { fpsX + 5, 17 },
            IM_COL32(255, 255, 255, 255),
            fpsText.c_str()
        );

        // Spectator-Box + Text (falls aktiv)
        if (show_spectator) {
            std::string specText = "Spectators: " + std::to_string(spectator_count);
            ImVec2 specSize = ImGui::CalcTextSize(specText.c_str());
            float spectatorsX = fpsX + fpsSize.x + 20;

            ImGui::GetBackgroundDrawList()->AddRectFilled(
                { spectatorsX, 15 },
                { spectatorsX + specSize.x + 13, 15 + specSize.y + 3 },
                IM_COL32(10, 10, 10, 210), 6.f
            );
            ImGui::GetBackgroundDrawList()->AddText(
                { spectatorsX + 5, 17 },
                IM_COL32(255, 255, 255, 255),
                specText.c_str()
            );
        }

        ImGui::PopFont();
    }

}
