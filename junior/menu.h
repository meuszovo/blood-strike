#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "vectors.h"
#include "images.h"
#include "TextEditor.h"
#include "imgui_settings.h"
#include "Fonts.h"
#include "saver_loader.h"

static int iTabs = 0;
static int iSubTabs = 0;
static int last_iTabs = -1;
bool menu_o = true;

void menu()
{
    if (iTabs != last_iTabs)
    {
        last_iTabs = iTabs;
        switch (iTabs)
        {
        case 0: iSubTabs = 0; break;
        case 1: iSubTabs = 4; break;
        case 2: iSubTabs = 7; break;
        case 3: iSubTabs = 9; break;
        }
    }

    ImGui::SetNextWindowSize(ImVec2(760, 565));
    ImGui::Begin("General", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
    {
        ImGui::PushFont(default_font);
        anim_speed = ImGui::GetIO().DeltaTime * 8.f;

        auto draw = ImGui::GetWindowDrawList();
        const auto& p = ImGui::GetWindowPos();


        draw->AddRectFilled(p, p + ImVec2(760, 565), (ImU32)winbg_color, 10.f);

        draw->AddRectFilled(p, p + ImVec2(80, 540), (ImU32)child_color[0], 10.f, ImDrawFlags_RoundCornersTopLeft);

        draw->AddRectFilled(p + ImVec2(79, 0), p + ImVec2(80, 540), (ImU32)stroke_color);

        ImGui::SetCursorPos(ImVec2(10, 15));

        static bool sub_tabs_mode = false;

        ImGui::BeginChild("Tabs", ImVec2(60, 525), true);

        ImGui::Tab("Aim", &iTabs, 0);
        ImGui::Tab("Visuals", &iTabs, 1);
        ImGui::Tab("Misc", &iTabs, 2);
        ImGui::Tab("Settings", &iTabs, 3);

        ImGui::EndChild();

        if (iTabs == 0)
        {
            draw->AddRectFilled(p + ImVec2(80, 60), p + ImVec2(760, 62.f), (ImU32)stroke_color);
            ImGui::SetCursorPos(ImVec2(97, 12));
            ImGui::BeginGroup();
            {
                ImGui::SubTab("Aim", &iSubTabs, 0); ImGui::SameLine();
                ImGui::SubTab("Draw", &iSubTabs, 1); ImGui::SameLine();

            }
            ImGui::EndGroup();

            ImGui::SetCursorPos(ImVec2(85, 60));
            if (iSubTabs == 0) {
                ImGui::BeginChild("AimFrame", ImVec2(0, 0), true);
                {
                    ImGui::SetCursorPos(ImVec2(12, 12));
                    ImGui::BeginChild("AimGeneral", ImVec2(290, 470), false, 0);
                    {
                        ImGui::Checkbox("enable aim", &global::aim::enable);
                        ImGui::Checkbox("visible only", &global::aim::visible_only);
                        ImGui::Checkbox("prediction", &global::aim::prediction);
                        ImGui::Checkbox("enable deadzone", &global::aim::enable_deadzone);
                        ImGui::Checkbox("enable acceleration/deceleration", &global::aim::enable_deceleration);
                        ImGui::SliderFloat("smooth", &global::aim::smooth, 0, 100);
                        ImGui::Combo("hitbox", &global::aim::bone_type, global::aim::bone_char, IM_ARRAYSIZE(global::aim::bone_char));
                        ImGui::Combo("keybind", &global::aim::current_bind, global::aim::bind_char, IM_ARRAYSIZE(global::aim::bind_char));
                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos(ImVec2(314, 12));
                    ImGui::BeginChild("Adjustment", ImVec2(350, 470), false, 0);
                    {
                        ImGui::Checkbox("show fov", &global::aim::show_fov);
                        ImGui::SliderFloat("fov", &global::aim::fov_slider, 0, 500);
                        ImGui::SliderInt("max distance", &global::aim::max_distance, 0, 500);
                        ImGui::SliderFloat("dead zone", &global::aim::deadzone, 0, 500);
                        ImGui::SliderFloat("restriction", &global::aim::restriction, 0, 500);
                        ImGui::SliderFloat("aim speed", &global::aim::speed, 0, 500);
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
            else if (iSubTabs == 1)
            {
                ImGui::BeginChild("DrawFrame", ImVec2(0, 0), true);
                {
                    ImGui::SetCursorPos(ImVec2(12, 12));
                    ImGui::BeginChild("DrawGeneral", ImVec2(290, 470), false, 0);
                    {
                        ImGui::Checkbox("bullet tracer", &global::visual::bullet_tracer);
                        ImGui::Checkbox("target snapline", &global::aim::line_to_target);
                        ImGui::Checkbox("target circle", &global::aim::circle_to_tagert);
                        ImGui::Checkbox("reload bar", &global::visual::reload_bar);

                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos(ImVec2(314, 12));
                    ImGui::BeginChild("Adjustment", ImVec2(350, 470), false, 0);
                    {
                        ImGui::Checkbox("draw x tracer", &global::visual::draw_x_bullet);
                        ImGui::SliderFloat("bullet tracer time", &global::visual::time, 1, 15);
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
            else if (iSubTabs == 3)
            {
                ImGui::BeginChild("AimFrame2", ImVec2(0, 0), true);
                {
                    ImGui::SetCursorPos(ImVec2(12, 12));
                    ImGui::BeginChild("AimGeneral2", ImVec2(290, 470), false, 0);
                    {
                        ImGui::Checkbox("enable aim", &global::aim::enable);
                        ImGui::Checkbox("visible only", &global::aim::visible_only);
                        ImGui::Checkbox("prediction", &global::aim::prediction);
                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos(ImVec2(314, 12));
                    ImGui::BeginChild("AimAdjustment2", ImVec2(350, 470), false, 0);
                    {
                        ImGui::SliderFloat("smooth", &global::aim::smooth, 0, 100);
                        ImGui::Combo("hitbox", &global::aim::bone_type, global::aim::bone_char, IM_ARRAYSIZE(global::aim::bone_char));
                        ImGui::Combo("keybind", &global::aim::current_bind, global::aim::bind_char, IM_ARRAYSIZE(global::aim::bind_char));
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
        }
        else if (iTabs == 1)
        {
            draw->AddRectFilled(p + ImVec2(80, 60), p + ImVec2(760, 62.f), (ImU32)stroke_color);
            ImGui::SetCursorPos(ImVec2(97, 12));
            ImGui::BeginGroup();
            {
                ImGui::SubTab("Visual", &iSubTabs, 4); ImGui::SameLine();
                ImGui::SubTab("Colors", &iSubTabs, 6);
            }
            ImGui::EndGroup();

            ImGui::SetCursorPos(ImVec2(85, 60));
            if (iSubTabs == 4) {
                ImGui::BeginChild("VisualFrame", ImVec2(0, 0), true);
                {
                    ImGui::SetCursorPos(ImVec2(12, 12));
                    ImGui::BeginChild("VisualGeneral", ImVec2(290, 470), false, 0);
                    {
                        ImGui::Checkbox("enable", &global::visual::enable);
                        ImGui::Checkbox("draw skeleton", &global::visual::skeleton);
                        ImGui::Checkbox("draw box", &global::visual::box);
                        ImGui::Checkbox("draw distance", &global::visual::distance);
                        ImGui::Checkbox("draw platform", &global::visual::platform);
                        ImGui::Checkbox("draw rank", &global::visual::rank);
                        ImGui::Checkbox("draw weapon", &global::visual::weapon);
                        ImGui::Checkbox("draw snapline", &global::visual::snapline);
                        ImGui::Checkbox("draw chinese hat", &global::visual::chinese_hat);
                        ImGui::Checkbox("show visible", &global::visual::show_visible);
                        ImGui::Checkbox("show invisible", &global::visual::show_invisible);
                        ImGui::Checkbox("show knocked", &global::visual::show_knocked);
                        ImGui::Checkbox("show spectators", &global::visual::show_spectators);
                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos(ImVec2(314, 12));
                    ImGui::BeginChild("VisualAdjustment", ImVec2(350, 470), false, 0);
                    {
                        ImGui::Checkbox("outlined esp", &global::visual::outline);
                        ImGui::Combo("box type", &global::visual::box_type, global::visual::box_char, IM_ARRAYSIZE(global::visual::box_char));
                        ImGui::Combo("snapline position", &global::visual::snapline_position, global::visual::snapline_char, IM_ARRAYSIZE(global::visual::snapline_char));
                        ImGui::SliderInt("max distance", &global::visual::max_distance, 0, 500);
                        ImGui::SliderInt("box thickness", &global::visual::box_thickness, 1, 5);
                        ImGui::SliderInt("skeleton thickness", &global::visual::skeleton_thickness, 1, 5);
                        ImGui::SliderInt("snapline thickness", &global::visual::snapline_thickness, 1, 5);
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
            else if (iSubTabs == 6)
            {
                ImGui::BeginChild("ColorsFrame", ImVec2(0, 0), true);
                {
                    ImGui::SetCursorPos(ImVec2(12, 12));
                    ImGui::BeginChild("Visible", ImVec2(290, 470), false, 0);
                    {
                        ImGui::ColorEdit4("skeleton color", global::visual::skeleton_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("box color", global::visual::box_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("snapline color", global::visual::snapline_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("weapon color", global::visual::weapon_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("distance color", global::visual::distance_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("platform color", global::visual::platform_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("rank color", global::visual::rank_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("chinese hat color", global::visual::chinese_hat_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("knocked color", global::visual::knocked_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("spectators color", global::visual::spectators_color_visible,ImGuiColorEditFlags_NoTooltip| ImGuiColorEditFlags_NoInputs| ImGuiColorEditFlags_NoDragDrop| ImGuiColorEditFlags_AlphaPreview);
                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos(ImVec2(314, 12));
                    ImGui::BeginChild("Invisible", ImVec2(350, 470), false, 0);
                    {
                        ImGui::ColorEdit4("skeleton color", global::visual::skeleton_color_invisible, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("box color", global::visual::box_color_invisible, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("snapline color", global::visual::snapline_color_invisible, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("weapon color", global::visual::weapon_color_invisible, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                        ImGui::ColorEdit4("distance color", global::visual::distance_color_invisible, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
        }
        else if (iTabs == 2)
        {
            draw->AddRectFilled(p + ImVec2(80, 60), p + ImVec2(760, 62.f), (ImU32)stroke_color);
            ImGui::SetCursorPos(ImVec2(97, 12));
            ImGui::BeginGroup();
            {
                ImGui::SubTab("World", &iSubTabs, 7); ImGui::SameLine();
                ImGui::SubTab("Colors", &iSubTabs, 8);
            }
            ImGui::EndGroup();

            ImGui::SetCursorPos(ImVec2(85, 60));
            if (iSubTabs == 7) {
                ImGui::BeginChild("WorldFrame", ImVec2(0, 0), true);
                {
                    ImGui::SetCursorPos(ImVec2(12, 12));
                    ImGui::BeginChild("WorldGeneral", ImVec2(290, 470), false, 0);
                    {
                        ImGui::Checkbox("render pickup", &global::world_esp::render_pickup);
                        ImGui::Checkbox("render chest", &global::world_esp::render_chest);
                        ImGui::Checkbox("render ammobox", &global::world_esp::render_ammo);
                        ImGui::Checkbox("render vehicles", &global::world_esp::render_vehicle);
                        ImGui::Checkbox("render wood builds", &global::world_esp::render_wood_builds);
                        ImGui::Checkbox("render brick builds", &global::world_esp::render_brick_builds);
                        ImGui::Checkbox("render metal builds", &global::world_esp::render_metal_builds);
                        ImGui::Checkbox("render 3d box chest", &global::world_esp::show_3d_chest);
                        ImGui::Checkbox("render 3d box weapon", &global::world_esp::show_3d_weapon);
                        ImGui::Checkbox("add glow to world esp", &global::world_esp::glow);
                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos(ImVec2(314, 12));
                    ImGui::BeginChild("WorldAdjustment", ImVec2(350, 470), false, 0);
                    {

                        const char* rarity_labels[] = {
                            "uncommon",
                            "common",    
                            "rare",      
                            "epic",    
                            "legandary",      
                            "mythic",    
                            "all"       
                        };

                        ImGui::Combo("max rarity", &global::world_esp::max_rarity, rarity_labels, IM_ARRAYSIZE(rarity_labels));

                        ImGui::SliderFloat("max pickup", &global::world_esp::pickup_max, 1, 250);
                        ImGui::SliderFloat("max chest", &global::world_esp::chest_max, 1, 250);
                        ImGui::SliderFloat("max ammobox", &global::world_esp::ammo_max, 1, 250);
                        ImGui::SliderFloat("max vehicle", &global::world_esp::vehicle_max, 1, 250);
                        ImGui::SliderFloat("max builds", &global::world_esp::build_max, 1, 250);

                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
            else if (iSubTabs == 8)
            {
                ImGui::BeginChild("WorldColorsFrame", ImVec2(0, 0), true);
                {
                    ImGui::SetCursorPos(ImVec2(12, 12));
                    ImGui::BeginGroup(); 
                    {
                        ImGui::BeginChild("Weapons", ImVec2(290, 310), false, 0);
                        {
                            ImGui::ColorEdit4("uncommon", global::world_esp::weapon_color_uncommon, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                            ImGui::ColorEdit4("common", global::world_esp::weapon_color_common, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                            ImGui::ColorEdit4("rare", global::world_esp::weapon_color_rare, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                            ImGui::ColorEdit4("epic", global::world_esp::weapon_color_epic, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                            ImGui::ColorEdit4("legendary", global::world_esp::weapon_color_gold, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                            ImGui::ColorEdit4("mythic", global::world_esp::weapon_color_mythic, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                        }
                        ImGui::EndChild();
                    } ImGui::EndGroup();

                    ImGui::SetCursorPos(ImVec2(314, 12));
                    ImGui::BeginGroup(); 
                    {
                        ImGui::BeginChild("World", ImVec2(350, 180), false, 0);
                        {
                            ImGui::ColorEdit4("chest", global::world_esp::chest_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                            ImGui::ColorEdit4("vehicle", global::world_esp::vehicle_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                            ImGui::ColorEdit4("ammobox", global::world_esp::ammo_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                        }
                        ImGui::EndChild();

                        ImGui::BeginChild("Builds", ImVec2(350, 180), false, 0);
                        {
                            ImGui::ColorEdit4("wood", global::world_esp::wood_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                            ImGui::ColorEdit4("brick", global::world_esp::brick_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                            ImGui::ColorEdit4("metal", global::world_esp::metal_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview);
                        }
                        ImGui::EndChild();
                    }
                }
                ImGui::EndChild();
            }
        }
        else if (iTabs == 3)
        {
            draw->AddRectFilled(p + ImVec2(80, 60), p + ImVec2(760, 62.f), (ImU32)stroke_color);
            ImGui::SetCursorPos(ImVec2(97, 12));
            ImGui::BeginGroup();
            {
                ImGui::SubTab("Settings", &iSubTabs, 9);
            }
            ImGui::EndGroup();

            ImGui::SetCursorPos(ImVec2(85, 60));
            if (iSubTabs == 9) {
                ImGui::BeginChild("SettingsFrame", ImVec2(0, 0), true);
                {
                    ImGui::SetCursorPos(ImVec2(12, 12));
                    ImGui::BeginChild("SettingsGeneral", ImVec2(290, 470), false, 0);
                    {
                        ImGui::Checkbox("streamproof", &global::menu::streamproof);
                        float full_width = ImGui::GetContentRegionAvail().x;
                        float button_width = (full_width - ImGui::GetStyle().ItemSpacing.x);
                        if (ImGui::Button("exit menu", ImVec2(button_width, 25)))
                        {
                            exit(-1);
                        }
                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos(ImVec2(314, 12));
                    ImGui::BeginChild("Configs", ImVec2(350, 470), false, 0);
                    {
                        constexpr int max_cfgs = 10;
                        for (int i = 1; i <= max_cfgs; ++i)
                        {
                            float full_width = ImGui::GetContentRegionAvail().x;
                            float button_width = (full_width - ImGui::GetStyle().ItemSpacing.x) / 2;

                            std::string save_label = "SAVE CFG " + std::to_string(i);
                            std::string load_label = "LOAD CFG " + std::to_string(i);

                            if (ImGui::Button(save_label.c_str(), ImVec2(button_width, 25)))
                            {
                                std::string filename = "config" + std::to_string(i) + ".ini";
                                save_settings(filename);
                            }

                            ImGui::SameLine();

                            if (ImGui::Button(load_label.c_str(), ImVec2(button_width, 25)))
                            {
                                std::string filename = "config" + std::to_string(i) + ".ini";
                                load_settings(filename);
                            }
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
        }

        ImGui::PopFont();
    }
    ImGui::End();
}
