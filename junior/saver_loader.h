#pragma once
#include "global.h"
#include "config-setup.h"

void save_settings(std::string fileName) {

    char file[MAX_PATH];
    sprintf_s(file, "C:\\MicrosoftEddge\\%s.ini", fileName.c_str());
    CreateDirectoryA("C:\\MicrosoftEddge", NULL);

    WritePrivateProfileInt("menu", "streamproof", (int)global::menu::streamproof, file);

    for (int i = 0; i < 4; i++) {
        char key[64];
        sprintf_s(key, "bg_color_%d", i); WritePrivateProfileFloat("menu", key, global::menu::bg_color[i], file);
        sprintf_s(key, "title_color_%d", i); WritePrivateProfileFloat("menu", key, global::menu::title_color[i], file);
        sprintf_s(key, "button_color_%d", i); WritePrivateProfileFloat("menu", key, global::menu::button_color[i], file);
        sprintf_s(key, "tabs_color_%d", i); WritePrivateProfileFloat("menu", key, global::menu::tabs_color[i], file);
    }
    WritePrivateProfileInt("menu", "particle_type", global::menu::particle_type, file);
    WritePrivateProfileFloat("menu", "particle_speed", global::menu::particle_speed, file);
    WritePrivateProfileInt("menu", "show_particles", (int)global::menu::show_particles, file);

    WritePrivateProfileInt("esp", "enabled", (int)global::esp::enabled, file);
    WritePrivateProfileInt("esp", "box", (int)global::esp::box, file);
    WritePrivateProfileInt("esp", "skeleton", (int)global::esp::skeleton, file);
    WritePrivateProfileInt("esp", "line", (int)global::esp::line, file);

    WritePrivateProfileInt("aim", "enabled", (int)global::aim::enabled, file);
    WritePrivateProfileInt("aim", "drawFov", (int)global::aim::drawFov, file);
    WritePrivateProfileFloat("aim", "fov", global::aim::fov, file);
    WritePrivateProfileFloat("aim", "smooth", global::aim::smooth, file);
    WritePrivateProfileInt("aim", "aimKey", global::aim::aimKey, file);

    for (int i = 0; i < 4; i++) {
        char key[64];
        sprintf_s(key, "fov_color_%d", i); WritePrivateProfileFloat("aim", key, global::aim::fovColor[i], file);
    }

    WritePrivateProfileInt("world_esp", "show_uncommon", (int)global::world_esp::show_uncommon, file);
    WritePrivateProfileInt("world_esp", "show_common", (int)global::world_esp::show_common, file);
    WritePrivateProfileInt("world_esp", "show_rare", (int)global::world_esp::show_rare, file);
    WritePrivateProfileInt("world_esp", "show_purple", (int)global::world_esp::show_purple, file);
    WritePrivateProfileInt("world_esp", "show_gold", (int)global::world_esp::show_gold, file);
    WritePrivateProfileInt("world_esp", "show_mythi", (int)global::world_esp::show_mythi, file);
    WritePrivateProfileFloat("world_esp", "chest_max", global::world_esp::chest_max, file);
    WritePrivateProfileFloat("world_esp", "vehicle_max", global::world_esp::vehicle_max, file);
    WritePrivateProfileFloat("world_esp", "ammo_max", global::world_esp::ammo_max, file);
    WritePrivateProfileFloat("world_esp", "pickup_max", global::world_esp::pickup_max, file);
    WritePrivateProfileFloat("world_esp", "build_max", global::world_esp::build_max, file);
    WritePrivateProfileInt("world_esp", "render_chest", (int)global::world_esp::render_chest, file);
    WritePrivateProfileInt("world_esp", "render_pickup", (int)global::world_esp::render_pickup, file);
    WritePrivateProfileInt("world_esp", "render_ammo", (int)global::world_esp::render_ammo, file);
    WritePrivateProfileInt("world_esp", "render_vehicle", (int)global::world_esp::render_vehicle, file);
    WritePrivateProfileInt("world_esp", "render_wood_builds", (int)global::world_esp::render_wood_builds, file);
    WritePrivateProfileInt("world_esp", "render_brick_builds", (int)global::world_esp::render_brick_builds, file);
    WritePrivateProfileInt("world_esp", "render_metal_builds", (int)global::world_esp::render_metal_builds, file);
    WritePrivateProfileInt("world_esp", "show_3d_chest", (int)global::world_esp::show_3d_chest, file);
    WritePrivateProfileInt("world_esp", "show_3d_weapon", (int)global::world_esp::show_3d_weapon, file);
    WritePrivateProfileInt("world_esp", "max_rarity", global::world_esp::max_rarity, file);
    WritePrivateProfileInt("world_esp", "glow", (int)global::world_esp::glow, file);

    for (int i = 0; i < 4; i++) {
        char key[64];
        sprintf_s(key, "weapon_color_uncommon_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_uncommon[i], file);
        sprintf_s(key, "weapon_color_common_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_common[i], file);
        sprintf_s(key, "weapon_color_rare_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_rare[i], file);
        sprintf_s(key, "weapon_color_epic_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_epic[i], file);
        sprintf_s(key, "weapon_color_gold_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_gold[i], file);
        sprintf_s(key, "weapon_color_mythic_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_mythic[i], file);
        sprintf_s(key, "chest_color_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::chest_color[i], file);
        sprintf_s(key, "ammo_color_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::ammo_color[i], file);
        sprintf_s(key, "vehicle_color_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::vehicle_color[i], file);
        sprintf_s(key, "wood_color_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::wood_color[i], file);
        sprintf_s(key, "brick_color_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::brick_color[i], file);
        sprintf_s(key, "metal_color_%d", i); WritePrivateProfileFloat("world_esp", key, global::world_esp::metal_color[i], file);
    }
}

void load_settings(std::string fileName) {

    char file[MAX_PATH];
    sprintf_s(file, "C:\\MicrosoftEddge\\%s.ini", fileName.c_str());
    CreateDirectoryA("C:\\MicrosoftEddge", NULL);

    global::menu::streamproof = (bool)GetPrivateProfileInt("menu", "streamproof", (int)global::menu::streamproof, file);

    for (int i = 0; i < 4; i++) {
        char key[64];
        sprintf_s(key, "bg_color_%d", i); global::menu::bg_color[i] = GetPrivateProfileFloat("menu", key, global::menu::bg_color[i], file);
        sprintf_s(key, "title_color_%d", i); global::menu::title_color[i] = GetPrivateProfileFloat("menu", key, global::menu::title_color[i], file);
        sprintf_s(key, "button_color_%d", i); global::menu::button_color[i] = GetPrivateProfileFloat("menu", key, global::menu::button_color[i], file);
        sprintf_s(key, "tabs_color_%d", i); global::menu::tabs_color[i] = GetPrivateProfileFloat("menu", key, global::menu::tabs_color[i], file);
    }
    global::menu::particle_type = GetPrivateProfileInt("menu", "particle_type", global::menu::particle_type, file);
    global::menu::particle_speed = GetPrivateProfileFloat("menu", "particle_speed", global::menu::particle_speed, file);
    global::menu::show_particles = (bool)GetPrivateProfileInt("menu", "show_particles", (int)global::menu::show_particles, file);

    global::esp::enabled = (bool)GetPrivateProfileInt("esp", "enabled", (int)global::esp::enabled, file);
    global::esp::box = (bool)GetPrivateProfileInt("esp", "box", (int)global::esp::box, file);
    global::esp::skeleton = (bool)GetPrivateProfileInt("esp", "skeleton", (int)global::esp::skeleton, file);
    global::esp::line = (bool)GetPrivateProfileInt("esp", "line", (int)global::esp::line, file);

    global::aim::enabled = (bool)GetPrivateProfileInt("aim", "enabled", (int)global::aim::enabled, file);
    global::aim::drawFov = (bool)GetPrivateProfileInt("aim", "drawFov", (int)global::aim::drawFov, file);
    global::aim::fov = GetPrivateProfileFloat("aim", "fov", global::aim::fov, file);
    global::aim::smooth = GetPrivateProfileFloat("aim", "smooth", global::aim::smooth, file);
    global::aim::aimKey = GetPrivateProfileInt("aim", "aimKey", global::aim::aimKey, file);

    for (int i = 0; i < 4; i++) {
        char key[64];
        sprintf_s(key, "fov_color_%d", i); global::aim::fovColor[i] = GetPrivateProfileFloat("aim", key, global::aim::fovColor[i], file);
    }

    global::world_esp::show_uncommon = (bool)GetPrivateProfileInt("world_esp", "show_uncommon", (int)global::world_esp::show_uncommon, file);
    global::world_esp::show_common = (bool)GetPrivateProfileInt("world_esp", "show_common", (int)global::world_esp::show_common, file);
    global::world_esp::show_rare = (bool)GetPrivateProfileInt("world_esp", "show_rare", (int)global::world_esp::show_rare, file);
    global::world_esp::show_purple = (bool)GetPrivateProfileInt("world_esp", "show_purple", (int)global::world_esp::show_purple, file);
    global::world_esp::show_gold = (bool)GetPrivateProfileInt("world_esp", "show_gold", (int)global::world_esp::show_gold, file);
    global::world_esp::show_mythi = (bool)GetPrivateProfileInt("world_esp", "show_mythi", (int)global::world_esp::show_mythi, file);
    global::world_esp::chest_max = GetPrivateProfileFloat("world_esp", "chest_max", global::world_esp::chest_max, file);
    global::world_esp::vehicle_max = GetPrivateProfileFloat("world_esp", "vehicle_max", global::world_esp::vehicle_max, file);
    global::world_esp::ammo_max = GetPrivateProfileFloat("world_esp", "ammo_max", global::world_esp::ammo_max, file);
    global::world_esp::pickup_max = GetPrivateProfileFloat("world_esp", "pickup_max", global::world_esp::pickup_max, file);
    global::world_esp::build_max = GetPrivateProfileFloat("world_esp", "build_max", global::world_esp::build_max, file);
    global::world_esp::render_chest = (bool)GetPrivateProfileInt("world_esp", "render_chest", (int)global::world_esp::render_chest, file);
    global::world_esp::render_pickup = (bool)GetPrivateProfileInt("world_esp", "render_pickup", (int)global::world_esp::render_pickup, file);
    global::world_esp::render_ammo = (bool)GetPrivateProfileInt("world_esp", "render_ammo", (int)global::world_esp::render_ammo, file);
    global::world_esp::render_vehicle = (bool)GetPrivateProfileInt("world_esp", "render_vehicle", (int)global::world_esp::render_vehicle, file);
    global::world_esp::render_wood_builds = (bool)GetPrivateProfileInt("world_esp", "render_wood_builds", (int)global::world_esp::render_wood_builds, file);
    global::world_esp::render_brick_builds = (bool)GetPrivateProfileInt("world_esp", "render_brick_builds", (int)global::world_esp::render_brick_builds, file);
    global::world_esp::render_metal_builds = (bool)GetPrivateProfileInt("world_esp", "render_metal_builds", (int)global::world_esp::render_metal_builds, file);
    global::world_esp::show_3d_chest = (bool)GetPrivateProfileInt("world_esp", "show_3d_chest", (int)global::world_esp::show_3d_chest, file);
    global::world_esp::show_3d_weapon = (bool)GetPrivateProfileInt("world_esp", "show_3d_weapon", (int)global::world_esp::show_3d_weapon, file);
    global::world_esp::max_rarity = GetPrivateProfileInt("world_esp", "max_rarity", global::world_esp::max_rarity, file);
    global::world_esp::glow = (bool)GetPrivateProfileInt("world_esp", "glow", (int)global::world_esp::glow, file);

    for (int i = 0; i < 4; i++) {
        char key[64];
        sprintf_s(key, "weapon_color_uncommon_%d", i); global::world_esp::weapon_color_uncommon[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_uncommon[i], file);
        sprintf_s(key, "weapon_color_common_%d", i); global::world_esp::weapon_color_common[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_common[i], file);
        sprintf_s(key, "weapon_color_rare_%d", i); global::world_esp::weapon_color_rare[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_rare[i], file);
        sprintf_s(key, "weapon_color_epic_%d", i); global::world_esp::weapon_color_epic[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_epic[i], file);
        sprintf_s(key, "weapon_color_gold_%d", i); global::world_esp::weapon_color_gold[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_gold[i], file);
        sprintf_s(key, "weapon_color_mythic_%d", i); global::world_esp::weapon_color_mythic[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::weapon_color_mythic[i], file);
        sprintf_s(key, "chest_color_%d", i); global::world_esp::chest_color[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::chest_color[i], file);
        sprintf_s(key, "ammo_color_%d", i); global::world_esp::ammo_color[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::ammo_color[i], file);
        sprintf_s(key, "vehicle_color_%d", i); global::world_esp::vehicle_color[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::vehicle_color[i], file);
        sprintf_s(key, "wood_color_%d", i); global::world_esp::wood_color[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::wood_color[i], file);
        sprintf_s(key, "brick_color_%d", i); global::world_esp::brick_color[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::brick_color[i], file);
        sprintf_s(key, "metal_color_%d", i); global::world_esp::metal_color[i] = GetPrivateProfileFloat("world_esp", key, global::world_esp::metal_color[i], file);
    }
}
