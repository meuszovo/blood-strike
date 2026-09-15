#pragma once

#include "TextEditor.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <string>
#include <vector>
#include <string>
#include <chrono>

// Alles was vorher BLAU war jetzt LILA/MAGENTA!
// Nur Zeilen mit "Blau" ge�ndert, Rest bleibt 1:1!

inline ImColor active_color(255, 0, 255, 255);

inline ImColor active_color_half(219, 150, 150, 60); // UNVER�NDERT (ist kein blau!)

inline ImColor default_color(27, 27, 31, 255); // UNVER�NDERT

inline ImColor text_color[2] = { ImColor(245, 245, 245, 255), ImColor(225, 225, 225, 255) }; // UNVER�NDERT

inline ImColor background_color(230, 233, 238, 255); // UNVER�NDERT

inline ImColor second_color(250, 253, 258, 255); // UNVER�NDERT

inline ImColor stroke_color(245, 245, 245, 0); // UNVER�NDERT

inline ImColor stroke_color_active(245, 245, 245, 63); // UNVER�NDERT

inline ImColor child_color[2] = { ImColor(20, 20, 23, 255), ImColor(19, 19, 21, 255) };

inline ImColor scroll_bg_col(24, 24, 26, 255);

inline ImColor winbg_color(16, 16, 18, 255);

inline ImColor menu_title_color(180, 0, 255, 255);
inline ImColor menu_button_color(180, 0, 255, 255);
inline ImColor menu_tabs_color(15, 5, 30, 230);


inline ImFont* icon_font;

inline ImFont* logo_font;

inline ImFont* tabs_font;

inline ImFont* small_font;

inline ImFont* default_font;

inline ImFont* small_icon_font;

inline ImFont* title_font;

inline ImFont* arrow_icons;

inline ImVec2 frame_size(270, 16);

inline float anim_speed = 0.05f;


inline const TextEditor::Palette rE_palette = { {
                0xff7f7f7f,    // Default
                ImColor(187, 134, 192),    // Keyword    
                ImColor(181, 206, 155),    // Number
                ImColor(206, 145, 120),    // String
                ImColor(206, 145, 120), // Char literal
                0xffffffff, // Punctuation
                0xff408080,    // Preprocessor
                0xffaaaaaa, // Identifier
                ImColor(220, 205, 121), // Known identifier
                0xffc040a0, // Preproc identifier
                ImColor(106, 153, 62), // Comment (single line)
                ImColor(106, 153, 62), // Comment (multi line)
                child_color[1], // Background
                0xffe0e0e0, // Cursor
                0x80a06020, // Selection
                0x800020ff, // ErrorMarker
                0x40f08000, // Breakpoint
                ImColor(1.f, 1.f, 1.f, 0.2f), // Line number,
                0x40000000, // Current line fill
                0x40808080, // Current line fill (inactive)
                0x40a0a0a0, // Current line edge
            } };

inline float ImDegToRad(float degrees)
{
    static const float deg_to_rad = 0.01745329251994329576923690768489f;
    return degrees * deg_to_rad;
}

inline ImColor GetColorWithAlpha(ImColor color, float alpha)
{
    return ImColor(color.Value.x, color.Value.y, color.Value.z, alpha);
}

inline ImVec2 center_text(ImVec2 min, ImVec2 max, const char* text)
{
    return min + (max - min) / 2 - ImGui::CalcTextSize(text) / 2;
}

inline bool ColorPickerHQ(const char* label, float col[4])
{
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.03f, 0.16f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.18f, 0.05f, 0.28f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25f, 0.07f, 0.38f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.04f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.06f, 0.32f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.08f, 0.45f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 0.f, 1.f, 0.3f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
    bool ret = ImGui::ColorEdit4(label, col, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(7);
    return ret;
}

inline int rotation_start_index;
inline void ImRotateStart()
{
    rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
}

inline ImVec2 ImRotationCenter()
{
    ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

    const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

    return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2); // or use _ClipRectStack?
}

inline ImVec4 ImColorToImVec4(const ImColor& color)
{
    return ImVec4(color.Value.x, color.Value.y, color.Value.z, color.Value.w);
}

// Tab, SubTab, Keybind are now in imgui_widgets.cpp (not inline in header to avoid freeze)

inline void Pickerbox(std::string label, bool* v, float col[4])
{
    std::string picker_name = "##picker" + label;


    ImGui::SetCursorPos(ImGui::GetCursorPos() - ImVec2(30, 0));
    ImGui::ColorEdit4(picker_name.c_str(), (float*)col, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

    ImGui::SetCursorPos(ImGui::GetCursorPos() - ImVec2(0, 53));
    ImGui::Checkbox(label.c_str(), v);
}


inline void Keybindbox(std::string label, bool* v, int* key)
{
    std::string picker_name = "##keybind" + label;

    ImGui::SetCursorPos(ImGui::GetCursorPos() - ImVec2(30, 0));
    ImGui::Keybind(picker_name.c_str(), key);
    ImGui::SetCursorPos(ImGui::GetCursorPos() - ImVec2(0, 43));
    ImGui::Checkbox(label.c_str(), v);
}

inline void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
{
    float s = sin(rad), c = cos(rad);
    center = ImRotate(center, s, c) - center;

    auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
}

inline void rect_glow(ImDrawList* draw, ImVec2 start, ImVec2 end, ImColor col, float rounding, float intensity) {
    while (true) {
        if (col.Value.w < 0.0019f)
            break;

        draw->AddRectFilled(start, end, (ImU32)col, rounding);

        col.Value.w -= col.Value.w / intensity;
        start = ImVec2(start.x - 1, start.y - 1);
        end = ImVec2(end.x + 1, end.y + 1);
    }
}

