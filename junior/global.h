#pragma once

namespace global
{
	namespace esp
	{
		bool enabled = false;
		bool box = false;
		bool skeleton = false;
		bool line = false;
		float boxColor[4] = { 1.f, 0.f, 1.f, 1.f };
		float skeletonColor[4] = { 1.f, 0.f, 1.f, 1.f };
		float lineColor[4] = { 1.f, 0.f, 1.f, 1.f };
	}

	namespace aim
	{
		bool enabled = false;
		bool drawFov = false;
		float fov = 175.f;
		float fovColor[4] = { 0.60f, 0.00f, 1.00f, 1.f };
		float smooth = 5.f;
		int aimKey = 1002;

	}

	namespace world_esp
	{
		bool show_uncommon = false;
		bool show_common = false;
		bool show_rare = false;
		bool show_purple = false;
		bool show_gold = false;
		bool show_mythi = false;
		float chest_max = 100;
		float vehicle_max = 100;
		float ammo_max = 100;
		float pickup_max = 100;
		float build_max = 100;
		bool render_chest = false;
		bool render_pickup = false;
		bool render_ammo = false;
		bool render_vehicle = false;
		bool render_wood_builds = false;
		bool render_brick_builds = false;
		bool render_metal_builds = false;

		bool show_3d_chest = false;
		bool show_3d_weapon = false;

		int max_rarity = 6;
		bool glow = false;

		float weapon_color_uncommon[4] = { 1.f, 0.f, 1.f, 1.f }; 
		float weapon_color_common[4] = { 1.f, 0.f, 1.f, 1.f }; 
		float weapon_color_rare[4] = { 0.8f, 0.f, 1.f, 1.f }; 
		float weapon_color_epic[4] = { 1.f, 0.f, 1.f, 1.f }; 
		float weapon_color_gold[4] = { 1.f, 0.2f, 1.f, 1.f }; 
		float weapon_color_mythic[4] = { 0.7f, 0.f, 1.f, 1.f };
		float chest_color[4] = { 1.f, 0.f, 1.f, 1.f };  
		float ammo_color[4] = { 0.8f, 0.f, 1.f, 1.f };  
		float vehicle_color[4] = { 0.6f, 0.f, 1.f, 1.f }; 
		float wood_color[4] = { 0.7f, 0.f, 0.9f, 1.f };  
		float brick_color[4] = { 0.9f, 0.f, 1.f, 1.f }; 
		float metal_color[4] = { 0.5f, 0.f, 0.8f, 1.f }; 
	}

	namespace menu
	{
		bool streamproof = false;
		float menuFadeIn = 0.f;
		bool menuFirstFrame = true;
		float bg_color[4] = { 0.02f, 0.02f, 0.02f, 1.f };
		float title_color[4] = { 0.71f, 0.f, 1.f, 1.f };
		float button_color[4] = { 0.71f, 0.f, 1.f, 1.f };
		float tabs_color[4] = { 0.06f, 0.02f, 0.12f, 0.9f };
		int particle_type = 0;
		float particle_speed = 1.f;
		bool show_particles = true;

		static const char* particle_type_names[] = {
			"Stars", "Snow", "Rain", "Firefly", "Ash"
		};
	}

}
