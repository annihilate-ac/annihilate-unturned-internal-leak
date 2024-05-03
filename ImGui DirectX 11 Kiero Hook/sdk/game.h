#pragma once

#include "../includes.h"

#include "../mono.h"

#include "math.h"

#define cast_this reinterpret_cast<uintptr_t>(this)
#define get_member(type, name, offset) type name() { return driver.read<type>(cast_this + offset); }
#define set_member(type, name, offset) void name( type val ) { driver.write<type>(cast_this + offset, val); }
#define member(type, name, offset) get_member(type, name, offset) set_member(type, name, offset)
#define offset(klass, varname, name) varname = klass->find_field(name)->offset(); std::cout << "[+] found offset: " << name << "\n";

namespace unity_classes
{
	inline mono_class_t* playermovement;
	inline mono_class_t* playerlife;
	inline mono_class_t* playerlook;
	inline mono_class_t* provider;
	inline mono_class_t* steamplayer;
	inline mono_class_t* player;
	inline mono_class_t* maincamera;
	inline mono_class_t* steamplayerid;
	inline mono_class_t* playerequipment;
	inline mono_class_t* itemasset;
	inline mono_class_t* itemmanager;
	inline mono_class_t* interactableitem;
	inline mono_class_t* playerclothing;
	inline mono_class_t* humanclothes;
	inline mono_class_t* itemregion;
	inline mono_class_t* itemdrop;
	inline mono_class_t* itemdata;
	inline mono_class_t* lightingmanager;
	inline mono_class_t* itemweaponasset;
	inline mono_class_t* itemgunasset;
	inline mono_class_t* playeranimator;
	inline mono_class_t* usablegun;
	inline mono_class_t* bulletinfo;
	inline mono_class_t* itembarrelasset;
	inline mono_class_t* level_lighting;
	inline mono_class_t* camera;
	inline mono_class_t* playerquests;
	inline mono_class_t* playerui;
	inline mono_class_t* crosshair;
	inline mono_class_t* item;
	inline mono_class_t* zombiemanager;
	inline mono_class_t* zombie_region;
	inline mono_class_t* zombie;
	inline mono_class_t* playerstance;
	inline mono_class_t* attachments;
	inline mono_class_t* mode_config_data;
	inline mono_class_t* gameplay_config_data;
	inline mono_class_t* item_data;
	inline mono_class_t* steam_channel;
	inline mono_class_t* player_input;
	inline mono_class_t* walking_player_input_packet;
	inline mono_class_t* raycast_info;
	inline mono_class_t* player_inventory_ui;
	inline mono_class_t* player_crafting_ui;
	inline mono_class_t* player_skills_ui;
	inline mono_class_t* player_information_ui;
	inline mono_class_t* preference_data;
	inline mono_class_t* viewmodel_data;

	static void init()
	{
#ifndef NO_SECURITY
		VIRTUALIZER_START;
#endif

		playermovement = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerMovement");
		playerlife = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerLife");
		playerlook = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerLook");
		provider = mono::find_class("Assembly-CSharp", "SDG.Unturned.Provider");
		steamplayer = mono::find_class("Assembly-CSharp", "SDG.Unturned.SteamPlayer");
		player = mono::find_class("Assembly-CSharp", "SDG.Unturned.Player");
		maincamera = mono::find_class("Assembly-CSharp", "SDG.Unturned.MainCamera");
		steamplayerid = mono::find_class("Assembly-CSharp", "SDG.Unturned.SteamPlayerID");
		playerequipment = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerEquipment");
		itemasset = mono::find_class("Assembly-CSharp", "SDG.Unturned.ItemAsset");
		itemmanager = mono::find_class("Assembly-CSharp", "SDG.Unturned.ItemManager");
		interactableitem = mono::find_class("Assembly-CSharp", "SDG.Unturned.InteractableItem");
		playerclothing = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerClothing");
		humanclothes = mono::find_class("Assembly-CSharp", "SDG.Unturned.HumanClothes");
		itemregion = mono::find_class("Assembly-CSharp", "SDG.Unturned.ItemRegion");
		itemdrop = mono::find_class("Assembly-CSharp", "SDG.Unturned.ItemDrop");
		itemdata = mono::find_class("Assembly-CSharp", "SDG.Unturned.ItemData");
		lightingmanager = mono::find_class("Assembly-CSharp", "SDG.Unturned.LightingManager");
		itemweaponasset = mono::find_class("Assembly-CSharp", "SDG.Unturned.ItemWeaponAsset");
		itemgunasset = mono::find_class("Assembly-CSharp", "SDG.Unturned.ItemGunAsset");
		playeranimator = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerAnimator");
		usablegun = mono::find_class("Assembly-CSharp", "SDG.Unturned.UseableGun");
		bulletinfo = mono::find_class("Assembly-CSharp", "SDG.Unturned.BulletInfo");
		itembarrelasset = mono::find_class("Assembly-CSharp", "SDG.Unturned.ItemBarrelAsset");
		level_lighting = mono::find_class("Assembly-CSharp", "SDG.Unturned.LevelLighting");
		camera = mono::find_class("Assembly-CSharp", "UnityEngine.Camera");
		playerquests = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerQuests");
		playerui = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerUI");
		crosshair = mono::find_class("Assembly-CSharp", "SDG.Unturned.Crosshair");
		item = mono::find_class("Assembly-CSharp", "SDG.Unturned.Item");
		zombiemanager = mono::find_class("Assembly-CSharp", "SDG.Unturned.ZombieManager");
		playerstance = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerStance");
		attachments = mono::find_class("Assembly-CSharp", "SDG.Unturned.Attachments");
		mode_config_data = mono::find_class("Assembly-CSharp", "SDG.Unturned.ModeConfigData");
		gameplay_config_data = mono::find_class("Assembly-CSharp", "SDG.Unturned.GameplayConfigData");
		item_data = mono::find_class("Assembly-CSharp", "SDG.Unturned.ItemData");
		steam_channel = mono::find_class("Assembly-CSharp", "SDG.Unturned.SteamChannel");
		player_input = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerInput");
		walking_player_input_packet = mono::find_class("Assembly-CSharp", "SDG.Unturned.WalkingPlayerInputPacket");
		zombie_region = mono::find_class("Assembly-CSharp", "SDG.Unturned.ZombieRegion");
		zombie = mono::find_class("Assembly-CSharp", "SDG.Unturned.Zombie");
		raycast_info = mono::find_class("Assembly-CSharp", "SDG.Unturned.RaycastInfo");

		player_inventory_ui = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerDashboardInventoryUI");
		player_crafting_ui = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerDashboardCraftingUI");
		player_skills_ui = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerDashboardSkillsUI");
		player_information_ui = mono::find_class("Assembly-CSharp", "SDG.Unturned.PlayerDashboardInformationUI");

		preference_data = mono::find_class("Assembly-CSharp", "SDG.Unturned.PreferenceData");
		viewmodel_data = mono::find_class("Assembly-CSharp", "SDG.Unturned.ViewmodelPreferenceData");

#ifndef NO_SECURITY
		VIRTUALIZER_END;
#endif
	}
}

namespace class_offsets
{
	inline int max_players;
	inline int clients;
	inline int player;
	inline int life;
	inline int health;
	inline int dead;
	inline int look;
	inline int movement;
	inline int input;
	inline int stamina;
	inline int water;
	inline int food;
	inline int _player;
	inline int orbit_yaw;
	inline int orbit_pitch;
	inline int yaw;
	inline int pitch;
	inline int character_height;
	inline int character_yaw;
	inline int is_connected;
	inline int character_camera;
	inline int camera_instance;
	inline int clothing;
	inline int lastupdatepos;
	inline int player_id;
	inline int model;
	inline int character_name;
	inline int velocity;
	inline int jump;
	inline int item_name;
	inline int asset;
	inline int first_model;
	inline int equipment;
	inline int manager;
	inline int clampeditems;
	inline int interactable_item_asset;
	inline int rarity;
	inline int interactable_item;
	inline int item_amount;
	inline int item_quality;
	inline int type;
	inline int item;
	inline int item_id;
	inline int characterclothes;
	inline int skull;
	inline int characters_mesh_renderers;
	inline int itemregions;
	inline int drops;
	inline int items;
	inline int itemmodel;
	inline int height;
	inline int is_grounded;
	inline int playerlook_aim;
	inline int is_orbiting;
	inline int is_tracking;
	inline int is_focusing;
	inline int is_locking;
	inline int orbit_speed;
	inline int orbit_position;
	inline int time;
	inline int range;
	inline int ballistic_drop;
	inline int ballistic_force;
	inline int ballistic_steps;
	inline int ballistic_travel;
	inline int projectile_lifespan;
	inline int isbusy;
	inline int equip_anim_length_frames;
	inline int last_equip;
	inline int aim_duration;
	inline int can_aim_during_sprint;
	inline int reload_time;
	inline int recoil_min_x;
	inline int recoil_max_x;
	inline int recoil_min_y;
	inline int recoil_max_y;
	inline int lightingmanager;
	inline int cycle;
	inline int base_spread;
	inline int sway;
	inline int last_item_position;
	inline int last_frame_had_item_position;
	inline int animator;
	inline int quests;
	inline int ammo_per_shot;
	inline int usable;
	inline int bullets;
	inline int bullet_origin;
	inline int bullet_pos;
	inline int bullet_velocity;
	inline int bullet_direction;
	inline int ballistic_drop_barrel;
	inline int barrel_asset;
	inline int shake_min_x;
	inline int bind_address;
	inline int viewmodel_camera_transform;
	inline int viewmodel_camera;
	inline int third_clothing;
	inline int first_clothes;
	inline int bullet_gravity_modifier;
	inline int muzzle_velocity;
	inline int gun_firerate;
	inline int penetrate_buildables;
	inline int cached_sky_color;
	inline int skybox_sky;
	inline int cached_ground_color;
	inline int skybox_needs_update;
	inline int camera_fov;
	inline int steam_id;
	inline int wants_window_enabled;
	inline int wants_center_dot_visible;
	inline int crosshair;
	inline int gun_crosshair_visible;
	inline int instance_item_name;
	inline int ticking_zombies;
	inline int zombies;
	inline int zombie_regions;
	inline int viewmodel_sway_multiplayer;
	inline int _gesture;
	inline int look_x;
	inline int look_y;
	inline int main_camera_zoom_factor;
	inline int scope_camera_zoom_factor;
	inline int allow_freecam;
	inline int perspective;
	inline int eyes;
	inline int first;
	inline int ammo;
	inline int ammo_min;
	inline int ammo_max;
	inline int gun_firemode;
	inline int third_attachments;
	inline int magazine_asset;
	inline int steps;
	inline int mode_config_data;
	inline int preference_data;
	inline int viewmodel_data;
	inline int viewmodel_hip_fov;
	inline int viewmodel_aim_fov;
	inline int viewmodel_x_offset;
	inline int viewmodel_y_offset;
	inline int viewmodel_z_offset;
	inline int gameplay;
	inline int timer_exit;
	inline int timer_respawn;
	inline int air_decel;
	inline int air_accel;
	inline int allow_shoulder_camera;
	inline int ballistics_enabled;
	inline int chart;
	inline int compass;
	inline int group_map;
	inline int satellite;
	inline int timer_home;
	inline int point;
	inline int item_asset;
 	inline int should_use_zoom_factor;
	inline int scope_camera;
	inline int is_admin;
	inline int channel_owner;
	inline int player_channel;
	inline int third;
	inline int client_input_history;
	inline int pending_input;
	inline int input_consumed;
	inline int input_buffer;
	inline int primary_pressed_between_frames;
	inline int primary_held_last_frame;
	inline int input_yaw;
	inline int input_pitch;
	inline int client_position;
	inline int provider_time;
	inline int camera_mode;
	inline int lean_obstructed;
	inline int _shoulder;
	inline int zombie_health;
	inline int zombie_specialty;
	inline int raycast_player;
	inline int raycast_zombie;
	inline int raycast_limb;
	inline int raycast_point;
	inline int raycast_direction;
	inline int raycast_material;
	inline int raycast_transform;
	inline int raycast_distance;

	inline int first_renderer_01;
	inline int third_renderer_01;
	inline int third_renderer_02;

	inline int item_asset_amount;
	inline int inventory_active;
	inline int crafting_active;
	inline int skills_active;
	inline int information_active;
	inline int skybox_material;

	static void init()
	{
#ifndef NO_SECURITY
		VIRTUALIZER_START;
#endif

		offset(unity_classes::provider, max_players, "_maxPlayers");
		offset(unity_classes::provider, is_connected, "_isConnected");
		offset(unity_classes::provider, clients, "_clients");
		offset(unity_classes::provider, provider_time, "_time");
		offset(unity_classes::steamplayer, player, "_player");
		offset(unity_classes::player, life, "_life");
		offset(unity_classes::player, look, "_look");
		offset(unity_classes::player, movement, "_movement");
		offset(unity_classes::playerlife, stamina, "_stamina");
		offset(unity_classes::playerlife, water, "_water");
		offset(unity_classes::playerlife, food, "_food");
		offset(unity_classes::player, _player, "_player");
		offset(unity_classes::playerlook, orbit_yaw, "_orbitYaw");
		offset(unity_classes::playerlook, orbit_pitch, "_orbitPitch");
		offset(unity_classes::playerlook, yaw, "_yaw");
		offset(unity_classes::playerlook, pitch, "_pitch");
		offset(unity_classes::playerlook, character_height, "characterHeight");
		offset(unity_classes::playerlook, character_yaw, "_characterYaw");
		offset(unity_classes::playerlook, character_camera, "_characterCamera");
		offset(unity_classes::maincamera, camera_instance, "_instance");
		offset(unity_classes::playermovement, lastupdatepos, "lastUpdatePos");
		offset(unity_classes::playermovement, is_grounded, "_isGrounded");
		offset(unity_classes::steamplayer, player_id, "_playerID");
		offset(unity_classes::steamplayerid, character_name, "_characterName");
		offset(unity_classes::playerlife, health, "lastHealth");
		offset(unity_classes::playermovement, velocity, "velocity");
		offset(unity_classes::playermovement, jump, "JUMP");
		offset(unity_classes::player, equipment, "_equipment");
		offset(unity_classes::playerequipment, asset, "_asset");
		offset(unity_classes::playerequipment, first_model, "_firstModel");
		offset(unity_classes::itemasset, item_name, "_itemName");
		offset(unity_classes::itemasset, rarity, "rarity");
		offset(unity_classes::itemasset, type, "type");
		offset(unity_classes::itemmanager, manager, "manager");
		offset(unity_classes::itemmanager, itemregions, "<regions>k__BackingField");
		offset(unity_classes::itemmanager, clampeditems, "clampedItems");
		offset(unity_classes::interactableitem, interactable_item_asset, "asset");
		offset(unity_classes::interactableitem, item, "_item");
		offset(unity_classes::player, clothing, "_clothing");
		offset(unity_classes::playerclothing, characterclothes, "<thirdClothes>k__BackingField");
		offset(unity_classes::humanclothes, skull, "skull");
		offset(unity_classes::humanclothes, characters_mesh_renderers, "characterMeshRenderers");
		offset(unity_classes::steamplayer, model, "_model");
		offset(unity_classes::itemregion, drops, "_drops");
		offset(unity_classes::itemregion, items, "items");
		offset(unity_classes::itemdrop, itemmodel, "_model");
		offset(unity_classes::playermovement, height, "height");
		offset(unity_classes::playerlook, playerlook_aim, "_aim");
		offset(unity_classes::lightingmanager, time, "_time");
		offset(unity_classes::itemweaponasset, range, "range");
		offset(unity_classes::playerequipment, isbusy, "isBusy");
		offset(unity_classes::itemgunasset, can_aim_during_sprint, "<canAimDuringSprint>k__BackingField");
		offset(unity_classes::itemgunasset, ballistic_drop, "ballisticDrop");
		offset(unity_classes::itemgunasset, ballistic_force, "ballisticForce");
		offset(unity_classes::itemgunasset, ballistic_steps, "ballisticSteps");
		offset(unity_classes::itemgunasset, ballistic_travel, "ballisticTravel");
		offset(unity_classes::itemgunasset, projectile_lifespan, "projectileLifespan");
		offset(unity_classes::itemgunasset, aim_duration, "<aimInDuration>k__BackingField");
		offset(unity_classes::itemgunasset, reload_time, "reloadTime");
		offset(unity_classes::itemgunasset, recoil_min_x, "recoilMin_x");
		offset(unity_classes::itemgunasset, recoil_min_y, "recoilMin_y");
		offset(unity_classes::itemgunasset, recoil_max_x, "recoilMax_x");
		offset(unity_classes::itemgunasset, recoil_max_y, "recoilMax_y");
		offset(unity_classes::lightingmanager, lightingmanager, "manager");
		offset(unity_classes::lightingmanager, cycle, "_cycle");
		offset(unity_classes::itemgunasset, base_spread, "<baseSpreadAngleRadians>k__BackingField");
		offset(unity_classes::playeranimator, sway, "scopeSway");
		offset(unity_classes::playeranimator, last_item_position, "lastFrameItemPosition");
		offset(unity_classes::playeranimator, last_frame_had_item_position, "lastFrameHadItemPosition");
		offset(unity_classes::player, animator, "_animator");
		offset(unity_classes::itemgunasset, ammo_per_shot, "<ammoPerShot>k__BackingField");
		offset(unity_classes::playerequipment, usable, "_useable");
		offset(unity_classes::usablegun, bullets, "bullets");
		offset(unity_classes::bulletinfo, bullet_origin, "origin");
		offset(unity_classes::bulletinfo, bullet_pos, "pos");
		offset(unity_classes::bulletinfo, bullet_direction, "dir");
		offset(unity_classes::itembarrelasset, ballistic_drop_barrel, "_ballisticDrop");
		offset(unity_classes::itemgunasset, barrel_asset, "barrelAsset");
		offset(unity_classes::provider, bind_address, "bindAddress");
		offset(unity_classes::itemgunasset, shake_min_x, "shakeMin_x");
		offset(unity_classes::playeranimator, viewmodel_camera_transform, "viewmodelCameraTransform");
		offset(unity_classes::playerclothing, third_clothing, "<thirdClothes>k__BackingField");
		offset(unity_classes::playerclothing, first_clothes, "<firstClothes>k__BackingField");
		offset(unity_classes::bulletinfo, bullet_velocity, "<velocity>k__BackingField");
		offset(unity_classes::itemgunasset, bullet_gravity_modifier, "<bulletGravityMultiplier>k__BackingField");
		offset(unity_classes::itemgunasset, muzzle_velocity, "<muzzleVelocity>k__BackingField");
		offset(unity_classes::itemgunasset, gun_firerate, "firerate");
		offset(unity_classes::level_lighting, cached_sky_color, "cachedSkyColor");
		offset(unity_classes::level_lighting, skybox_material, "skybox");
		offset(unity_classes::level_lighting, cached_ground_color, "cachedGroundColor");
		offset(unity_classes::level_lighting, skybox_needs_update, "skyboxNeedsColorUpdate");
		offset(unity_classes::playeranimator, viewmodel_camera, "viewmodelCamera");
		offset(unity_classes::camera, camera_fov, "fieldOfView");
		offset(unity_classes::player, quests, "_quests");
		offset(unity_classes::playerquests, steam_id, "_groupID");
		offset(unity_classes::itemgunasset, penetrate_buildables, "projectilePenetrateBuildables");
		offset(unity_classes::playerui, wants_window_enabled, "wantsWindowEnabled");
		offset(unity_classes::crosshair, wants_center_dot_visible, "gameWantsCenterDotVisible");
		offset(unity_classes::crosshair, gun_crosshair_visible, "isGunCrosshairVisible");
		offset(unity_classes::itemasset, instance_item_name, "<instantiatedItemName>k__BackingField");
		offset(unity_classes::item, item_id, "_id");
		offset(unity_classes::item, item_amount, "amount");
		offset(unity_classes::itemasset, item_asset_amount, "amount");
		offset(unity_classes::itemasset, item_quality, "quality");
		offset(unity_classes::itemdrop, interactable_item, "_interactableItem");
		offset(unity_classes::zombiemanager, ticking_zombies, "_tickingZombies");
		offset(unity_classes::zombie_region, zombies, "_zombies");
		offset(unity_classes::zombiemanager, zombie_regions, "_regions");
		offset(unity_classes::level_lighting, skybox_sky, "<skyboxSky>k__BackingField");
		offset(unity_classes::playeranimator, viewmodel_sway_multiplayer, "viewmodelSwayMultiplier");
		offset(unity_classes::playeranimator, _gesture, "_gesture");
		offset(unity_classes::playerlook, look_x, "look_x");
		offset(unity_classes::playerlook, perspective, "_perspective");
		offset(unity_classes::playerlook, look_y, "look_y");
		offset(unity_classes::playerlook, main_camera_zoom_factor, "mainCameraZoomFactor");
		offset(unity_classes::playerlook, scope_camera_zoom_factor, "scopeCameraZoomFactor")
		offset(unity_classes::playerequipment, equip_anim_length_frames, "equipAnimLengthFrames");
		offset(unity_classes::playerequipment, last_equip, "lastEquip");
		offset(unity_classes::playerlook, allow_freecam, "allowFreecamWithoutAdmin");
		offset(unity_classes::usablegun, ammo, "ammo");
		offset(unity_classes::itemgunasset, ammo_min, "ammoMin");
		offset(unity_classes::itemgunasset, ammo_max, "ammoMax");
		offset(unity_classes::usablegun, third_attachments, "thirdAttachments");
		offset(unity_classes::attachments, magazine_asset, "_magazineAsset");
		offset(unity_classes::playerlife, dead, "_isDead");
		offset(unity_classes::bulletinfo, steps, "steps");
		offset(unity_classes::provider, mode_config_data, "_modeConfigData");
		offset(unity_classes::provider, preference_data, "_preferenceData");
		offset(unity_classes::mode_config_data, gameplay, "Gameplay");
		offset(unity_classes::gameplay_config_data, timer_exit, "Timer_Exit");
		offset(unity_classes::gameplay_config_data, timer_respawn, "Timer_Respawn");
		offset(unity_classes::gameplay_config_data, air_decel, "AirStrafing_Deceleration_Multiplier");
		offset(unity_classes::gameplay_config_data, air_accel, "AirStrafing_Acceleration_Multiplier");
		offset(unity_classes::player, third, "thirdSpot");
		offset(unity_classes::player, first, "_first");
		offset(unity_classes::playerlook, is_orbiting, "isOrbiting");
		offset(unity_classes::playerlook, is_tracking, "isTracking");
		offset(unity_classes::playerlook, is_focusing, "isFocusing");
		offset(unity_classes::playerlook, is_locking, "isLocking");
		offset(unity_classes::playerlook, orbit_speed, "orbitSpeed");
		offset(unity_classes::playerlook, orbit_position, "orbitPosition");
		offset(unity_classes::player_input, pending_input, "clientPendingInput");
		offset(unity_classes::player, input, "_input");
		offset(unity_classes::player_input, client_input_history, "clientInputHistory");
		offset(unity_classes::player_input, pending_input, "clientPendingInput");
		offset(unity_classes::player_input, input_consumed, "consumed");
		offset(unity_classes::player_input, input_buffer, "buffer");
		offset(unity_classes::player_input, primary_held_last_frame, "localWasPrimaryHeldLastFrame");
		offset(unity_classes::player_input, primary_pressed_between_frames, "localWasPrimaryPressedBetweenSimulationFrames");
		offset(unity_classes::walking_player_input_packet, input_yaw, "yaw");
		offset(unity_classes::walking_player_input_packet, input_pitch, "pitch");
		offset(unity_classes::walking_player_input_packet, client_position, "clientPosition");

		offset(unity_classes::gameplay_config_data, allow_shoulder_camera, "Allow_Shoulder_Camera");
		offset(unity_classes::gameplay_config_data, ballistics_enabled, "Ballistics");
		offset(unity_classes::gameplay_config_data, chart, "Chart");
		offset(unity_classes::gameplay_config_data, compass, "Compass");
		offset(unity_classes::gameplay_config_data, crosshair, "Crosshair");
		offset(unity_classes::gameplay_config_data, group_map, "Group_Map");
		offset(unity_classes::gameplay_config_data, satellite, "Satellite");
		offset(unity_classes::gameplay_config_data, timer_home, "Timer_Home");

		offset(unity_classes::item_data, point, "_point");
		offset(unity_classes::interactableitem, item_asset, "item");

		offset(unity_classes::playerlook, scope_camera, "_scopeCamera");
		offset(unity_classes::playerlook, should_use_zoom_factor, "shouldUseZoomFactorForSensitivity");

		offset(unity_classes::steamplayer, is_admin, "_isAdmin");
		offset(unity_classes::steam_channel, channel_owner, "owner");
		offset(unity_classes::player, player_channel, "_channel");

		offset(unity_classes::playeranimator, lean_obstructed, "leanObstructed");
		offset(unity_classes::playeranimator, _shoulder, "_shoulder");
		
		offset(unity_classes::provider, camera_mode, "cameraMode");
		offset(unity_classes::zombie, zombie_health, "health");
		offset(unity_classes::zombie, zombie_specialty, "speciality");

		offset(unity_classes::raycast_info, raycast_player, "player");
		offset(unity_classes::raycast_info, raycast_zombie, "zombie");
		offset(unity_classes::raycast_info, raycast_limb, "limb");
		offset(unity_classes::raycast_info, raycast_point, "point");
		offset(unity_classes::raycast_info, raycast_direction, "direction");
		offset(unity_classes::raycast_info, raycast_material, "material");
		offset(unity_classes::raycast_info, raycast_transform, "transform");
		offset(unity_classes::raycast_info, raycast_distance, "distance");

		offset(unity_classes::playeranimator, first_renderer_01, "firstRenderer_0");
		offset(unity_classes::playeranimator, third_renderer_01, "thirdRenderer_0");
		offset(unity_classes::playeranimator, third_renderer_02, "thirdRenderer_1");

		offset(unity_classes::player_inventory_ui, inventory_active, "active");
		offset(unity_classes::player_crafting_ui, crafting_active, "active");
		offset(unity_classes::player_skills_ui, skills_active, "active");
		offset(unity_classes::player_information_ui, information_active, "active");

		offset(unity_classes::itemgunasset, gun_firemode, "firemode");

		offset(unity_classes::preference_data, viewmodel_data, "Viewmodel");

		offset(unity_classes::viewmodel_data, viewmodel_aim_fov, "Field_Of_View_Aim");
		offset(unity_classes::viewmodel_data, viewmodel_hip_fov, "Field_Of_View_Hip");

		offset(unity_classes::viewmodel_data, viewmodel_x_offset, "Offset_Horizontal");
		offset(unity_classes::viewmodel_data, viewmodel_y_offset, "Offset_Vertical");
		offset(unity_classes::viewmodel_data, viewmodel_z_offset, "Offset_Depth");

#ifndef NO_SECURITY
		VIRTUALIZER_END;
#endif
	}
}

namespace game
{

	struct vec4
	{
		float x, y, z, w;
	};

	struct vmatrix_t
	{
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			float m[4][4];
		};
	};

	struct camera_t
	{
		vec3_t position;
		vmatrix_t matrix;
		uintptr_t object;
	}; inline camera_t camera;

	struct ray_t
	{

		vec3_t origin;
		vec3_t direction;

		ray_t(vec3_t origin, vec3_t direction)
		{

			this->origin = origin;
			this->direction = direction;

		}

		vec3_t get_point(float distance) { return origin + direction * distance; }

	};

	inline bool to_screen(const vec3_t& entity_position, ImVec2& screen_position)
	{

		vec3_t transform{ camera.matrix._14, camera.matrix._24, camera.matrix._34 };
		vec3_t right{ camera.matrix._11, camera.matrix._21, camera.matrix._31 };
		vec3_t up{ camera.matrix._12, camera.matrix._22, camera.matrix._32 };

		float w = transform.dot(entity_position) + camera.matrix._44;

		ImVec2 pos{ right.dot(entity_position) + camera.matrix._41, up.dot(entity_position) + camera.matrix._42 };

		bool ret = true;

		if (w < 0.098f)
			ret = false;

		static float screen_center_x = GetSystemMetrics(SM_CXSCREEN) / 2;
		static float screen_center_y = GetSystemMetrics(SM_CYSCREEN) / 2;

		screen_position = ImVec2(screen_center_x * (1 + pos.x / w), screen_center_y * (1 - pos.y / w));

		auto screen_size = ImGui::GetIO().DisplaySize;

		if (screen_position.x > screen_size.x || screen_position.y > screen_size.y)
			ret = false;

		return ret;
	}

	inline vec3_t calcangle(const vec3_t& src, const vec3_t& dst)
	{
#define M_RADPI 57.295779513082f
		vec3_t vecDiff = dst - src;
		return vec3_t(atan2(-vecDiff.z, vecDiff.length_2d()) * M_RADPI, atan2(vecDiff.y, vecDiff.x) * M_RADPI, 0.0f);
	}

	vec3_t calculate_angle(const vec3_t& origin, const vec3_t& dest, const vec3_t& sway)
	{
		vec3_t diff = origin - dest;
		vec3_t ret;

		float length = diff.length();
		ret.y = asinf(diff.y / length);
		ret.x = -atan2f(diff.x, -diff.z);

		return (ret * 57.29578f) - sway;
	}

}

enum EItemRarity : int
{
	COMMON,
	UNCOMMON,
	RARE,
	EPIC,
	LEGENDARY,
	MYTHICAL
};

enum EPlayerHeight
{
	STAND,
	CROUCH,
	PRONE
};

enum EItemType : int
{
	HAT,
	PANTS,
	SHIRT,
	MASK,
	BACKPACK,
	VEST,
	GLASSES,
	GUN,
	SIGHT,
	TACTICAL,
	GRIP,
	BARREL,
	MAGAZINE,
	FOOD,
	WATER,
	MEDICAL,
	MELEE,
	FUEL,
	TOOL,
	BARRICADE,
	STORAGE,
	BEACON,
	FARM,
	TRAP,
	STRUCTURE,
	SUPPLY,
	THROWABLE,
	GROWER,
	OPTIC,
	REFILL,
	FISHER,
	CLOUD,
	MAP,
	KEY,
	BOX,
	ARREST_START,
	ARREST_END,
	TANK,
	GENERATOR,
	DETONATOR,
	CHARGE,
	LIBRARY,
	FILTER,
	SENTRY,
	VEHICLE_REPAIR_TOOL,
	TIRE,
	COMPASS,
	OIL_PUMP
};

enum ELimb
{
	LEFT_FOOT,
	LEFT_LEG,
	RIGHT_FOOT,
	RIGHT_LEG,
	LEFT_HAND,
	LEFT_ARM,
	RIGHT_HAND,
	RIGHT_ARM,
	LEFT_BACK,
	RIGHT_BACK,
	LEFT_FRONT,
	RIGHT_FRONT,
	SPINE,
	SKULL,
	LIMB_MAX
};

enum EZombieSpeciality
{

	NONE,
	NORMAL,
	MEGA,
	CRAWLER,
	SPRINTER,
	FLANKER_FRIENDLY,
	FLANKER_STALK,
	BURNER,
	ACID,
	BOSS_ELECTRIC,
	BOSS_WIND,
	BOSS_FIRE,
	BOSS_ALL,
	BOSS_MAGMA,
	SPIRIT,
	BOSS_SPIRIT,
	BOSS_NUCLEAR,
	DL_RED_VOLATILE,
	DL_BLUE_VOLATILE,
	BOSS_ELVER_STOMPER,
	BOSS_KUWAIT,
	BOSS_BUAK_ELECTRIC,
	BOSS_BUAK_WIND,
	BOSS_BUAK_FIRE,
	BOSS_BUAK_FINAL

};

struct unity_list_t
{
	get_member(uint32_t, size, 0x18)
	get_member(uintptr_t, internal_list, 0x10)
	uintptr_t list()
	{
		return internal_list() + 0x20;
	}
};

template <typename T>
struct monoArray
{
	void* klass;
	void* monitor;
	void* bounds;
	int   max_length;
	T vector[65535];

	T& operator [] (int i)
	{
		return vector[i];
	}

	const T& operator [] (int i) const
	{
		return vector[i];
	}

	bool Contains(T item)
	{
		for (int i = 0; i < max_length; i++)
		{
			if (vector[i] == item) return true;
		}
		return false;
	}
};

template <class TYPE>
struct unity_list_2d {
	auto data() {
		std::vector<TYPE> results;
		if (!(uintptr_t)this)
			return results;
		results.resize(driver.read<int>((uintptr_t)this + 0x18));
		driver.read((uintptr_t)this + 0x20, (PBYTE)&results.at(0), (DWORD)results.size() * sizeof(TYPE));
		return results;
	}
};

template <class TYPE>
struct List {
	auto data() {
		std::vector<TYPE> results; results.resize(0);
		results.resize(driver.read<int>((ULONGLONG)this + 0x18));
		if (results.size()) {
			driver.read(driver.read<ULONGLONG>((ULONGLONG)this + 0x10) + 0x20, (PBYTE)&results.at(0), (DWORD)results.size() * sizeof(TYPE));
		}
		return results;
	}
};
template <class TYPE>
struct Array {
	auto data() {
		std::vector<TYPE> results;
		if (!(ULONGLONG)this)
			return results;
		results.resize(driver.read<int>((ULONGLONG)this + 0x18));
		driver.read((ULONGLONG)this + 0x20, (PBYTE)&results.at(0), (DWORD)results.size() * sizeof(TYPE));
		return results;
	}
};

struct managed_string_t
{

	uintptr_t base() { return cast_this; }
	get_member(int, size, 0x10);
	std::string to_string() { return driver.read_unicode_str(base(), size()); }

};

struct vector4
{
	float x, y, z, w;
	inline vector4 operator*(float rhs) const { return { x * rhs, y * rhs, z * rhs, w * rhs }; }
	inline vector4 operator+(const vector4& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w }; }
	inline vector4 operator-(const vector4& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w }; }
	inline vector4 operator/(const vector4& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w }; }
	inline vector4 operator-() const { return { -x, -y, -z, -w }; }

	inline vector4 operator*(const vector4& rhs) const
	{
		return {
			w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
			w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
			w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x,
			w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
		};
	}

	inline vec3_t operator*(const vec3_t& rhs) const
	{
		float x = this->x * 2.0F;
		float y = this->y * 2.0F;
		float z = this->z * 2.0F;
		float xx = this->x * x;
		float yy = this->y * y;
		float zz = this->z * z;
		float xy = this->x * y;
		float xz = this->x * z;
		float yz = this->y * z;
		float wx = this->w * x;
		float wy = this->w * y;
		float wz = this->w * z;

		vec3_t res;
		res.x = (1.0f - (yy + zz)) * rhs.x + (xy - wz) * rhs.y + (xz + wy) * rhs.z;
		res.y = (xy + wz) * rhs.x + (1.0f - (xx + zz)) * rhs.y + (yz - wx) * rhs.z;
		res.z = (xz - wy) * rhs.x + (yz + wx) * rhs.y + (1.0f - (xx + yy)) * rhs.z;

		return res;
	}

	inline vector4 conjugate() const { return { -x, -y, -z, w }; }
};

struct TransformAccess
{
	uintptr_t hierarchyAddr;
	int index;
};

class trsX
{
public:
	vec3_t t;
private:
	char pad_000C[4];
public:
	vector4 q;
	vec3_t s;
private:
	char pad_000B[4];
};

class TransformInternal
{

	// no idea what the actual size is here, but 1000 should be enough
	static constexpr int BUFF_SIZE = 1000;

	static trsX* trsBuffer;
	static int* parentIndicesBuffer;

	uintptr_t address;

	TransformAccess transformAccess;
	uintptr_t localTransforms;
	uintptr_t parentIndices;

	// might be beneficial to call these yourself instead of in every function, if you happen to use multiple functions that call this?

public:
	TransformInternal(uintptr_t address)
	{

		if (!trsBuffer) trsBuffer = new trsX[BUFF_SIZE]; // 48 kb
		if (!parentIndicesBuffer) parentIndicesBuffer = new int[BUFF_SIZE]; // 4 kb

		this->address = address;
		this->transformAccess = driver.read<TransformAccess>(address + 0x38);
		this->localTransforms = driver.read<uintptr_t>(transformAccess.hierarchyAddr + 0x18);
		this->parentIndices = driver.read<uintptr_t>(transformAccess.hierarchyAddr + 0x20);

		updateParentIndicesBuffer();

		updateTrsXBuffer();

	}

	// ------------------------------
	// actual methods
	// ------------------------------

	inline vec3_t localPosition()
	{

		return trsBuffer[transformAccess.index].t;

	}

	inline vec3_t localScale()
	{

		return trsBuffer[transformAccess.index].s;

	}

	inline vector4 localRotation()
	{

		return trsBuffer[transformAccess.index].q;

	}

	inline vec3_t position()
	{

		vec3_t worldPos = trsBuffer[transformAccess.index].t;
		int index = parentIndicesBuffer[transformAccess.index];
		while (index >= 0)
		{
			auto parent = trsBuffer[index];

			worldPos = parent.q * worldPos;
			worldPos = worldPos * parent.s;
			worldPos = worldPos + parent.t;

			index = parentIndicesBuffer[index];
		}

		return worldPos;

	}

	inline vector4 rotation()
	{

		vector4 worldRot = trsBuffer[transformAccess.index].q;
		int index = parentIndicesBuffer[transformAccess.index];
		while (index >= 0)
		{
			auto parent = trsBuffer[index];

			worldRot = parent.q * worldRot;

			index = parentIndicesBuffer[index];
		}

		return worldRot;

	}

	inline vec3_t right()
	{

		static vec3_t right = { 1, 0, 0 };
		return rotation() * right;

	}

	inline vec3_t up()
	{

		static vec3_t up = { 0, 1, 0 };
		return rotation() * up;

	}

	inline vec3_t forward()
	{

		static vec3_t forward = { 0, 0, 1 };
		return rotation() * forward;

	}

	// local to world
	inline vec3_t TransformDirection(vec3_t localDirection)
	{

		return rotation() * localDirection;

	}

	// world to local
	inline vec3_t InverseTransformDirection(vec3_t worldDirection)
	{

		return rotation().conjugate() * worldDirection;

	}

	// local to world
	inline vec3_t TransformPoint(vec3_t localPoint)
	{

		vec3_t worldPos = localPoint;
		int index = transformAccess.index;
		while (index >= 0)
		{
			auto parent = trsBuffer[index];

			worldPos = worldPos * parent.s;
			worldPos = parent.q * worldPos;
			worldPos = worldPos + parent.t;

			index = parentIndicesBuffer[index];
		}

		return worldPos;

	}

	// world to local
	inline vec3_t InverseTransformPoint(vec3_t worldPoint)
	{

		vec3_t worldPos = trsBuffer[transformAccess.index].t;
		vector4 worldRot = trsBuffer[transformAccess.index].q;

		vec3_t localScale = trsBuffer[transformAccess.index].s;

		int index = parentIndicesBuffer[transformAccess.index];
		while (index >= 0)
		{
			auto parent = trsBuffer[index];

			worldPos = parent.q * worldPos;
			worldPos = worldPos * parent.s;
			worldPos = worldPos + parent.t;

			worldRot = parent.q * worldRot;

			index = parentIndicesBuffer[index];
		}

		vec3_t local = worldRot.conjugate() * (worldPoint - worldPos);
		return local / localScale;

		return worldPos;

	}

	inline void updateTrsXBuffer()
	{
		// i dont think this is needed, but just in case
		if (transformAccess.index >= BUFF_SIZE)
			return;

		driver.read(localTransforms, (void*)trsBuffer, sizeof(trsX) * (transformAccess.index + 1));
	}

	inline void updateParentIndicesBuffer()
	{
		if (transformAccess.index >= BUFF_SIZE)
			return;

		driver.read(parentIndices, (void*)parentIndicesBuffer, sizeof(int) * (transformAccess.index + 1));
	}

};

trsX* TransformInternal::trsBuffer = nullptr;
int* TransformInternal::parentIndicesBuffer = nullptr;

struct unity_transform_t
{

	vec3_t __fastcall position(int index = 0)
	{
		if (!cast_this)
			return {};
		struct TransformAccessReadOnly {
			uintptr_t pTransformData;
			int index;
		};
		struct TransformData {
			uintptr_t pTransformArray;
			uintptr_t pTransformIndices;
		};
		struct Matrix34 {
			vector4 vec0, vec1, vec2;
		};
		__m128 result = { 0 };
		const __m128 mulVec0 = { -2.00, 2.000, -2.00, 0.000 };
		const __m128 mulVec1 = { 2.000, -2.00, -2.00, 0.000 };
		const __m128 mulVec2 = { -2.00, -2.00, 2.000, 0.000 };

		TransformAccessReadOnly pTransformAccessReadOnly = driver.read<TransformAccessReadOnly>(cast_this + 0x38);
		if (!pTransformAccessReadOnly.pTransformData)
			return vec3_t{};
		TransformData transformData = driver.read<TransformData>((uintptr_t)pTransformAccessReadOnly.pTransformData + 0x18);

		if (!transformData.pTransformArray)
			return vec3_t{};
		SIZE_T sizeMatriciesBuf = sizeof(Matrix34) * pTransformAccessReadOnly.index + sizeof(Matrix34);
		SIZE_T sizeIndicesBuf = sizeof(int) * pTransformAccessReadOnly.index + sizeof(int);

		PVOID pMatriciesBuf = malloc(sizeMatriciesBuf);
		PVOID pIndicesBuf = malloc(sizeIndicesBuf);
		if (pMatriciesBuf && pIndicesBuf)
		{
			memset(pMatriciesBuf, 0, sizeMatriciesBuf);
			memset(pIndicesBuf, 0, sizeIndicesBuf);
			if (pMatriciesBuf && pIndicesBuf)
			{
				if (!driver.read((uintptr_t)transformData.pTransformArray, (PBYTE)pMatriciesBuf, (DWORD)sizeMatriciesBuf))
				{
					free(pMatriciesBuf);
					free(pIndicesBuf);
					return vec3_t{};
				}
				if (!driver.read((uintptr_t)transformData.pTransformIndices, (PBYTE)pIndicesBuf, (DWORD)sizeIndicesBuf))
				{
					free(pMatriciesBuf);
					free(pIndicesBuf);
					return vec3_t{};
				}

				result = *(__m128*)((uintptr_t)pMatriciesBuf + 0x30 * pTransformAccessReadOnly.index);
				int transformIndex = *(int*)((uintptr_t)pIndicesBuf + 0x4 * pTransformAccessReadOnly.index) + index;

				while (transformIndex >= 0)
				{
					if (0x30 * transformIndex >= sizeMatriciesBuf)
					{
						free(pMatriciesBuf);
						free(pIndicesBuf);
						return vec3_t{};
					}
					if (0x4 * transformIndex >= sizeIndicesBuf)
					{
						free(pMatriciesBuf);
						free(pIndicesBuf);
						return vec3_t{};
					}

					Matrix34 matrix34 = *(Matrix34*)((uintptr_t)pMatriciesBuf + 0x30 * transformIndex);
					__m128 xxxx = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x00));
					__m128 yyyy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x55));
					__m128 zwxy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x8E));
					__m128 wzyw = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xDB));
					__m128 zzzz = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xAA));
					__m128 yxwy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x71));
					__m128 tmp7 = _mm_mul_ps(*(__m128*)(&matrix34.vec2), result);

					result = _mm_add_ps(
						_mm_add_ps(
							_mm_add_ps(
								_mm_mul_ps(
									_mm_sub_ps(
										_mm_mul_ps(_mm_mul_ps(xxxx, mulVec1), zwxy),
										_mm_mul_ps(_mm_mul_ps(yyyy, mulVec2), wzyw)),
									_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0xAA))),
								_mm_mul_ps(
									_mm_sub_ps(
										_mm_mul_ps(_mm_mul_ps(zzzz, mulVec2), wzyw),
										_mm_mul_ps(_mm_mul_ps(xxxx, mulVec0), yxwy)),
									_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x55)))),
							_mm_add_ps(
								_mm_mul_ps(
									_mm_sub_ps(
										_mm_mul_ps(_mm_mul_ps(yyyy, mulVec0), yxwy),
										_mm_mul_ps(_mm_mul_ps(zzzz, mulVec1), zwxy)),
									_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x00))),
								tmp7)), *(__m128*)(&matrix34.vec0));
					transformIndex = *(int*)((uintptr_t)pIndicesBuf + 0x4 * transformIndex);
				}
				free(pMatriciesBuf);
				free(pIndicesBuf);
			}
		}
		return vec3_t(result.m128_f32[0], result.m128_f32[1], result.m128_f32[2]);
	}

	auto name() {
		if (!(uintptr_t)this)
			return std::string();
		uintptr_t addr = driver.read_chain<uintptr_t>((uintptr_t)this + 0x30, { 0x60 });
		return driver.read<std::string>(addr);
	}

	auto parent() {
		if (!(uintptr_t)this)
			return (unity_transform_t*)nullptr;
		return driver.read<unity_transform_t*>((uintptr_t)this + 0x90);
	}

	auto child(int index = 0) {
		if (!(uintptr_t)this)
			return (unity_transform_t*)nullptr;
		return driver.read_chain<unity_transform_t*>((uintptr_t)this + 0x70, { (uintptr_t)index * 0x8 });
	}

	auto child(const char* symbol) {
		for (int i = 0; i < 0xFF; i++)
		{
			auto results = ((unity_transform_t*)this)->child(i);
			auto txt = results->name();
			if (!txt.size())
				break;
			else if (txt._Equal(symbol))
				return results;
		}
		return (unity_transform_t*)nullptr;
	}

	auto root_rotation() {
		if (!(uintptr_t)this)
			return ImVec4();
		return driver.read_chain<ImVec4>((uintptr_t)this + 0x38, { 0x18, 0x10 });
	}

	void root_rotation(ImVec4 val)
	{

		auto chain_1 = driver.read<uintptr_t>(cast_this + 0x38);

		if (!chain_1)
			return;

		auto chain_2 = driver.read<uintptr_t>(chain_1 + 0x18);

		if (!chain_2)
			return;

		driver.write<ImVec4>(chain_2 + 0x10, val);

	}

	auto root_position() {
		if (!(uintptr_t)this)
			return vec3_t();
		auto o_root = driver.read_chain<uintptr_t>((uintptr_t)this + 0x38, { 0x18 });
		return driver.read<vec3_t>(o_root);
	}

	auto root_scaling() {
		if (!(uintptr_t)this)
			return vec3_t();
		return driver.read_chain<vec3_t>( (uintptr_t)this + 0x38, { 0x18, 0x20 });
	}

};

struct transform_ptr_t {
	auto m_CachedPtr() { return driver.read<unity_transform_t*>((uintptr_t)this + 0x10); }
};

struct internal_transform_t
{
	get_member(vec3_t, position, 0x90)
};

struct component_t
{

	std::string name;
	uintptr_t ptr;

};

struct quaternion_t
{

	float x;
	float y;
	float z;
	float w;

	vec3_t operator*(const vec3_t& point)
	{

		float num = this->x * 2.f;
		float num2 = this->y * 2.f;
		float num3 = this->z * 2.f;
		float num4 = this->x * num;
		float num5 = this->y * num2;
		float num6 = this->z * num3;
		float num7 = this->x * num2;
		float num8 = this->x * num3;
		float num9 = this->y * num3;
		float num10 = this->w * num;
		float num11 = this->w * num2;
		float num12 = this->w * num3;

		vec3_t result;
		result.x = (1.f - (num5 + num6)) * point.x + (num7 - num12) * point.y + (num8 + num11) * point.z;
		result.y = (num7 + num12) * point.x + (1.f - (num4 + num6)) * point.y + (num9 - num10) * point.z;
		result.z = (num8 - num11) * point.x + (num9 + num10) * point.y + (1.f - (num4 + num5)) * point.z;

		return result;

	}

};

struct transform_t
{

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::core_module_path, "UnityEngine", "Transform");
	}

	transform_t* find(NaOrganization::MidTerm::VmGeneralType::String name)
	{

		if (!this)
			return nullptr;

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Transform", "Find", { "System.String" });
		return NaOrganization::MidTerm::NaMethodInvoker<transform_t*, transform_t*, NaOrganization::MidTerm::VmGeneralType::String>(invoker.method.GetInvokeAddress()).Invoke(this, name);

	}

	transform_t* get_parent()
	{

		if (!this)
			return nullptr;

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Transform", "get_parent", { });
		return NaOrganization::MidTerm::NaMethodInvoker<transform_t*, transform_t*>(invoker.method.GetInvokeAddress()).Invoke(this);

	}

	vec3_t get_position()
	{

		if (!this)
			return {};

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Vector3", "get_position", { });
		return NaOrganization::MidTerm::NaMethodInvoker<vec3_t, transform_t*>(invoker.method.GetInvokeAddress()).Invoke(this);

	}

	quaternion_t get_rotation()
	{

		if (!this)
			return {};

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Quaternion", "get_rotation", { });
		return NaOrganization::MidTerm::NaMethodInvoker<quaternion_t, transform_t*>(invoker.method.GetInvokeAddress()).Invoke(this);

	}

	vec3_t get_forward()
	{

		if (!this)
			return {};

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Vector3", "get_forward", { });
		return NaOrganization::MidTerm::NaMethodInvoker<vec3_t, transform_t*>(invoker.method.GetInvokeAddress()).Invoke(this);

	}

};

struct game_object_t
{

	vec3_t position()
	{

		auto component_array = driver.read<uintptr_t>(cast_this + 0x30);
		auto transform_component = driver.read<uintptr_t>(component_array + 0x8);
		auto internal_transform = driver.read<uintptr_t>(transform_component + 0x38);
		auto transform_position = driver.read<vec3_t>(internal_transform + 0x90);

		return transform_position;

	}

	internal_transform_t* transform()
	{

		auto component_array = driver.read<uintptr_t>(cast_this + 0x30);
		auto transform_component = driver.read<uintptr_t>(component_array + 0x8);
		auto internal_transform = driver.read<internal_transform_t*>(transform_component + 0x38);

		return internal_transform;

	}

	uintptr_t get_transform_component()
	{

		auto component_array = driver.read<uintptr_t>(cast_this + 0x30);
		auto transform_component = driver.read<uintptr_t>(component_array + 0x8);

		return transform_component;

	}

	std::vector<component_t> get_components()
	{
		std::vector<component_t> ret_list{};
		auto comp_list = driver.read<uintptr_t>(cast_this + 0x30);

		for (unsigned int i = 0x8; i < 0x100; i = i + 0x10)
		{
			auto component = driver.read<uintptr_t>(comp_list + i);

			if (!component)
				continue;

			auto component_base = driver.read<uintptr_t>(component + 0x28);
			if (!component_base)
				continue;

			auto name_ptr = driver.read<uintptr_t>(component_base + 0x10);
			if (!name_ptr)
				continue;

			auto wide_name = driver.read_str(name_ptr);

			ret_list.emplace_back<component_t>({ std::string(wide_name.begin(), wide_name.end()), component});
		}

		return ret_list;
	}

};

struct mono_behaviour_t
{

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::core_module_path, "UnityEngine", "Component");
	}

	transform_t* get_transform()
	{

		if (!this)
			return nullptr;

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Transform", "get_transform", { });
		return NaOrganization::MidTerm::NaMethodInvoker<transform_t*, void*>(invoker.method.GetInvokeAddress()).Invoke(this);

	}

	mono_behaviour_t* get_component(void* type)
	{

		if (!this)
			return nullptr;

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Component", "GetComponent", { "System.Type" });
		return NaOrganization::MidTerm::NaMethodInvoker<mono_behaviour_t*, mono_behaviour_t*, void*>(invoker.method.GetInvokeAddress()).Invoke(this, type);

	}

	game_object_t* game_object()
	{ 
		
		auto internal_ptr = driver.read<uintptr_t>(cast_this + 0x10);

		if (!internal_ptr)
			return nullptr;

		return driver.read<game_object_t*>(internal_ptr + 0x30); 

	}

	auto class_name()
	{

		char name_buf[256]{ 0 };

		auto name_ptr = driver.read_chain<uintptr_t>(cast_this, { 0x0, 0x0, 0x48 });

		if (!name_ptr)
			return std::string("null");

		driver.read(name_ptr, (ULONG64)&name_buf, sizeof(name_buf));

		return std::string(name_buf);

	}

};

struct player_bounds_t
{

	ImVec2 top_left{};
	ImVec2 top_middle{};
	ImVec2 bottom_right{};
	ImVec2 bottom_middle{};

	bool valid{ false };

	void verify(vec3_t mins, vec3_t maxs)
	{

		top_left.y = top_left.x = FLT_MAX;
		bottom_right.y = bottom_right.x = -FLT_MAX;

		for (size_t i = 0; i < 8; ++i)
		{

			const vec3_t point(i & 1 ? maxs.x : mins.x, i & 2 ? maxs.y : mins.y, i & 4 ? maxs.z : mins.z);

			ImVec2 screen;

			if (!game::to_screen(point, screen))
				return;

			top_left = ImVec2(min(top_left.x, screen.x), min(top_left.y, screen.y));
			bottom_right = ImVec2(max(bottom_right.x, screen.x), max(bottom_right.y, screen.y));

		}

		valid = true;
		top_middle = top_left + ImVec2((bottom_right.x - top_left.x) * 0.5f, 0);
		bottom_middle = ImVec2(top_middle.x, top_middle.y + bottom_right.y - top_left.y);

	}

};

struct player_movement_t
{
	static auto get_instance() -> player_movement_t*
	{
		static auto instance = (player_movement_t*)unity_classes::playermovement->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}
	member(vec3_t, last_update_pos, class_offsets::lastupdatepos);
	member(vec3_t, velocity, class_offsets::velocity);
	member(float, jump, class_offsets::jump);
	get_member(EPlayerHeight, height, class_offsets::height);
	get_member(bool, is_grounded, class_offsets::is_grounded);
	float get_height()
	{
		switch (height())
		{
		case EPlayerHeight::STAND:
			return 2.f;
			break;
		case EPlayerHeight::CROUCH:
			return 1.2f;
			break;
		case EPlayerHeight::PRONE:
			return 0.8f;
			break;
		default:
			return 2.f;
			break;
		}
	}
};

enum EPlayerPerspective : int
{
	FIRST,
	THIRD
};

struct player_look_t : mono_behaviour_t
{

	static auto get_instance() -> player_look_t*
	{
		static auto instance = (player_look_t*)unity_classes::playerlook->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::assembly_path, "SDG.Unturned", "PlayerLook");
	}

	void teleport_yaw(float new_yaw)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "System.Void", "TeleportYaw", { "System.Single" });
		return NaOrganization::MidTerm::NaMethodInvoker<void, player_look_t*, float>(invoker.method.GetInvokeAddress()).Invoke(this, new_yaw);

	}

	member(float, orbit_yaw, class_offsets::orbit_yaw)
	member(float, orbit_pitch, class_offsets::orbit_pitch)
	member(float, yaw, class_offsets::yaw)
	member(ImVec2, viewangles, class_offsets::pitch)
	member(float, character_height, class_offsets::character_height)
	member(float, character_yaw, class_offsets::character_yaw)
	member(float, look_x, class_offsets::look_x);
	member(float, look_y, class_offsets::look_y);
	member(float, main_camera_zoom_factor, class_offsets::main_camera_zoom_factor);
	member(float, scope_camera_zoom_factor, class_offsets::scope_camera_zoom_factor);
	member(float, allow_freecam, class_offsets::allow_freecam);
	member(EPlayerPerspective, perspective, class_offsets::perspective);

	get_member(float, eyes, class_offsets::eyes);
	get_member(bool, should_use_zoom_factor, class_offsets::should_use_zoom_factor);
	get_member(uintptr_t, character_camera, class_offsets::character_camera);
	get_member(uintptr_t, scope_camera, class_offsets::scope_camera);
	get_member(uintptr_t, aim, class_offsets::playerlook_aim);

	member(bool, is_orbiting, class_offsets::is_orbiting);
	member(bool, is_tracking, class_offsets::is_tracking);
	member(bool, is_focusing, class_offsets::is_focusing);
	member(bool, is_locking, class_offsets::is_locking);

	member(float, orbit_speed, class_offsets::orbit_speed);
	member(vec3_t, orbit_position, class_offsets::orbit_position);

};

struct crosshair_t
{

	member(bool, wants_center_dot, class_offsets::wants_center_dot_visible);
	member(bool, wants_gun_crosshair, class_offsets::gun_crosshair_visible);

};

struct player_ui_t
{

	static auto get_instance() -> player_ui_t*
	{
		static auto instance = (player_ui_t*)unity_classes::playerui->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}

	get_member(bool, wants_window, class_offsets::wants_window_enabled);
	get_member(crosshair_t*, crosshair, class_offsets::crosshair);

};

struct player_inventory_ui_t
{

	static auto get_instance() -> player_inventory_ui_t*
	{
		static auto instance = (player_inventory_ui_t*)unity_classes::player_inventory_ui->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}

	get_member(bool, active, class_offsets::inventory_active);

};

struct player_crafting_ui_t
{

	static auto get_instance() -> player_crafting_ui_t*
	{
		static auto instance = (player_crafting_ui_t*)unity_classes::player_crafting_ui->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}

	get_member(bool, active, class_offsets::crafting_active);

};

struct player_skills_ui_t
{

	static auto get_instance() -> player_skills_ui_t*
	{
		static auto instance = (player_skills_ui_t*)unity_classes::player_skills_ui->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}

	get_member(bool, active, class_offsets::skills_active);

};

struct information_ui_t
{

	static auto get_instance() -> information_ui_t*
	{
		static auto instance = (information_ui_t*)unity_classes::player_information_ui->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}

	get_member(bool, active, class_offsets::information_active);

};

struct lighting_manager_t
{
	static auto get_instance() -> lighting_manager_t*
	{
		static auto instance = (lighting_manager_t*)unity_classes::lightingmanager->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}
	get_member(lighting_manager_t*, manager, class_offsets::lightingmanager);
	member(UINT, time, class_offsets::time);
	member(UINT, cycle, class_offsets::cycle);
};

struct item_t
{

	get_member(unsigned short, id, class_offsets::item_id);
	get_member(BYTE, amount, class_offsets::item_amount);

};

std::string get_string_from_wcs(const wchar_t* pcs)
{

	int res;
	char buf[0x400];
	char* pbuf = buf;
	std::shared_ptr<char[]> shared_pbuf;

	res = WideCharToMultiByte(CP_UTF8, 0, pcs, -1, buf, sizeof(buf), NULL, NULL);

	if (0 == res && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		res = WideCharToMultiByte(CP_UTF8, 0, pcs, -1, NULL, 0, NULL, NULL);

		shared_pbuf = std::shared_ptr<char[]>(new char[res]);

		pbuf = shared_pbuf.get();

		res = WideCharToMultiByte(CP_UTF8, 0, pcs, -1, pbuf, res, NULL, NULL);
	}

	return std::string(pbuf);

}

struct item_asset_t
{

	std::string item_name()
	{

		if (!this)
			return "Hands";

		uintptr_t managed_string = driver.read<uintptr_t>(cast_this + class_offsets::item_name);

		if (!managed_string)
			return "None";

		auto wide = driver.read_wstr(managed_string + 0x14);

		if (wide.empty())
			return "None";
		
		return get_string_from_wcs(wide.c_str());

	}

	std::string instance_item_name()
	{

		if (!this)
			return "None";

		uintptr_t managed_string = driver.read<uintptr_t>(cast_this + class_offsets::instance_item_name);

		if (!managed_string)
			return "None";

		auto wide = driver.read_wstr(managed_string + 0x14);

		if (wide.empty())
			return "None";

		return get_string_from_wcs(wide.c_str());

	}

	std::wstring item_name_wide()
	{
		if (!this)
			return L"Hands";
		uintptr_t managed_string = driver.read<uintptr_t>(cast_this + class_offsets::item_name);
		auto wide = driver.read_wstr(managed_string + 0x14);
		return wide;
	}

	get_member(EItemRarity, rarity, class_offsets::rarity)
	get_member(EItemType, type, class_offsets::type)
	get_member(item_t*, item, class_offsets::item)
	get_member(BYTE, amount, class_offsets::item_asset_amount);

};

struct item_weapon_asset_t : item_asset_t
{
	member(float, range, class_offsets::range);
};

struct weapon_ballistic_information_t
{

	float force;
	BYTE steps;
	float travel;

	bool valid()
	{
		return travel > 0 && force > 0;
	}

};

struct item_barrel_asset_t : item_asset_t
{
	member(float, ballistic_drop, class_offsets::ballistic_drop_barrel);
};

struct gun_shake_data_t
{
	float min_x, min_y, min_z, max_x, max_y, max_z;
};

struct item_gun_asset_t : item_weapon_asset_t
{
	member(float, ballistic_drop, class_offsets::ballistic_drop);
	member(float, ballistic_force, class_offsets::ballistic_force);
	member(float, ballistic_steps, class_offsets::ballistic_steps);
	member(float, ballistic_travel, class_offsets::ballistic_travel);
	member(float, projectile_lifespan, class_offsets::projectile_lifespan);
	member(float, aim_duration, class_offsets::aim_duration);
	member(bool, can_aim_during_sprint, class_offsets::can_aim_during_sprint);
	member(float, reload_time, class_offsets::reload_time);
	member(float, recoil_min_x, class_offsets::recoil_min_x);
	member(float, recoil_max_x, class_offsets::recoil_max_x);
	member(float, recoil_min_y, class_offsets::recoil_min_y);
	member(float, recoil_max_xy, class_offsets::recoil_max_y);
	member(game::vec4, recoil, class_offsets::recoil_min_x);
	get_member(weapon_ballistic_information_t, ballistic_information, class_offsets::ballistic_force);
	member(float, base_spread, class_offsets::base_spread);
	member(BYTE, ammo_per_shot, class_offsets::ammo_per_shot);
	member(item_barrel_asset_t*, barrel, class_offsets::barrel_asset);
	member(gun_shake_data_t, shake_data, class_offsets::shake_min_x);
	member(float, bullet_gravity_modifier, class_offsets::bullet_gravity_modifier);
	member(float, muzzle_velocity, class_offsets::muzzle_velocity);
	member(BYTE, fire_rate, class_offsets::gun_firerate);
	member(bool, penetrate_buildables, class_offsets::penetrate_buildables);
	get_member(BYTE, ammo_min, class_offsets::ammo_min);
	get_member(BYTE, ammo_max, class_offsets::ammo_max);
	get_member(int, firemode, class_offsets::gun_firemode);
};

struct bullet_info_t
{

	member(vec3_t, origin, class_offsets::bullet_origin);
	member(vec3_t, pos, class_offsets::bullet_pos);
	member(vec3_t, direction, class_offsets::bullet_direction);
	member(vec3_t, velocity, class_offsets::bullet_velocity);
	member(BYTE, steps, class_offsets::steps);

};

struct attachments_t
{

	get_member(item_asset_t*, magazine_asset, class_offsets::magazine_asset);

};

struct usable_t
{

};

struct usable_gun_t : usable_t
{

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::assembly_path, "SDG.Unturned", "UseableGun");
	}

	void fire()
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "System.Void", "fire", { });
		return NaOrganization::MidTerm::NaMethodInvoker<void, usable_gun_t*>(invoker.method.GetInvokeAddress()).Invoke(this);

	}

	get_member(unity_list_t*, bullets, class_offsets::bullets)
	get_member(BYTE, ammo, class_offsets::ammo);
	get_member(attachments_t*, third_attachments, class_offsets::third_attachments);

};

struct player_equipment_t
{
	get_member(item_asset_t*, asset, class_offsets::asset);
	get_member(uintptr_t, first_model, class_offsets::first_model);
	member(bool, is_busy, class_offsets::isbusy);
	member(UINT, equip_anim_length_frames, class_offsets::equip_anim_length_frames);
	member(float, last_equip, class_offsets::last_equip);
	get_member(usable_t*, usable, class_offsets::usable)
};

struct player_life_t
{
	get_member(BYTE, food, class_offsets::food);
	get_member(BYTE, water, class_offsets::water);
	get_member(BYTE, stamina, class_offsets::stamina);
	get_member(BYTE, health, class_offsets::health)
	get_member(bool, dead, class_offsets::dead);
};

struct interactable_item_t : mono_behaviour_t
{

	get_member(item_asset_t*, asset, class_offsets::interactable_item_asset)
	get_member(item_t*, item, class_offsets::item_asset);

};

struct item_drop_t
{

	get_member(transform_ptr_t*, model, class_offsets::itemmodel)
	get_member(interactable_item_t*, interactable, class_offsets::interactable_item);

};

struct item_data_t
{

	get_member(vec3_t, position, class_offsets::point);
	get_member(item_t*, item, class_offsets::item_asset);

};

struct item_region_t
{

	get_member(List<item_drop_t*>*, drops, class_offsets::drops)
	get_member(List<item_t*>*, items, class_offsets::items);

};

struct item_manager_t
{

	static auto get_instance() -> item_manager_t*
	{
		static auto instance = (item_manager_t*)unity_classes::itemmanager->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}
	get_member(item_manager_t*, manager, class_offsets::manager)
	get_member(unity_list_t*, clamped_items, class_offsets::clampeditems)
	get_member(Array<item_region_t*>*, item_regions, class_offsets::itemregions)

};

struct steam_id_t
{

	get_member(unsigned long, id, 0x4);

};

struct player_quests_t
{

	get_member(steam_id_t*, group_id, class_offsets::steam_id)

};

struct player_input_packet_t
{



};

struct player_walking_input_t : player_input_packet_t
{

	member(float, yaw, class_offsets::input_yaw);
	member(float, pitch, class_offsets::input_pitch);
	member(vec3_t, client_position, class_offsets::client_position);

};

struct player_input_t
{

	member(UINT, consumed, class_offsets::input_consumed);
	member(UINT, buffer, class_offsets::input_buffer);

	member(bool, primary_between_frames, class_offsets::primary_pressed_between_frames);
	member(bool, primary_held_last_frame, class_offsets::primary_held_last_frame);

	get_member(unity_list_t*, client_input_history, class_offsets::client_input_history);
	get_member(player_input_packet_t*, pending_input, class_offsets::pending_input);

};

struct player_t : mono_behaviour_t
{

	static auto get_instance() -> player_t*
	{

		static auto instance = (player_t*)unity_classes::player->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;

	}

	static auto get_local() { return driver.read<player_t*>((uintptr_t)get_instance() + class_offsets::_player); }

	get_member(player_movement_t*, movement, class_offsets::movement);
	get_member(player_input_t*, input, class_offsets::input);
	get_member(player_life_t*, life, class_offsets::life)
	get_member(player_look_t*, look, class_offsets::look)
	get_member(player_equipment_t*, equipment, class_offsets::equipment)
	get_member(struct player_clothing_t*, clothing, class_offsets::clothing)
	get_member(struct player_animator_t*, animator, class_offsets::animator)
	get_member(player_quests_t*, quests, class_offsets::quests)
	get_member(struct steam_channel_t*, channel, class_offsets::player_channel);
	get_member(uintptr_t, third, class_offsets::third);
	get_member(uintptr_t, first, class_offsets::first);

};

struct steam_player_id_t
{

	std::string character_name()
	{

		uintptr_t managed_string = driver.read<uintptr_t>(cast_this + class_offsets::character_name);
		auto wide = driver.read_wstr(managed_string + 0x14);

		return get_string_from_wcs(wide.c_str());

	}

};

struct unity_color_fields_t
{

	float r, g, b, a;

};

struct unity_color_t
{

	unity_color_fields_t fields{};

};

struct UnityEngine_Color_Fields {
	float r;
	float g;
	float b;
	float a;
};

struct UnityEngine_Color_o {
	UnityEngine_Color_Fields fields;
};

struct level_lighting_t
{

	static auto get_instance() -> level_lighting_t*
	{

		static auto vtable = (level_lighting_t*)unity_classes::provider->get_vtable(mono::get_root_domain())->get_static_field_data();
		return vtable;

	}

	get_member(struct material_t*, material, class_offsets::skybox_material);

	member(UnityEngine_Color_o, skybox_color, class_offsets::skybox_sky);
	member(UnityEngine_Color_o, cached_sky_color, class_offsets::cached_sky_color);
	member(UnityEngine_Color_o, cached_ground_color, class_offsets::cached_ground_color);
	member(bool, skybox_needs_update, class_offsets::skybox_needs_update);

};

struct steam_player_t
{
	get_member(player_t*, player, class_offsets::player)
	get_member(steam_player_id_t*, player_id, class_offsets::player_id)
	get_member(transform_ptr_t*, model, class_offsets::model)
	member(bool, is_admin, class_offsets::is_admin);

	unity_transform_t* skeleton()
	{

		auto _model = model();

		if (!_model)
			return nullptr;

		auto transform = _model->m_CachedPtr();

		if (!transform)
			return nullptr;

		auto child1 = transform->child(0);

		if (!child1)
			return nullptr;

		return child1->child(1);

	}

};

struct steam_channel_t
{

	get_member(steam_player_t*, owner, class_offsets::channel_owner);

};

struct gameplay_config_data_t
{

	member(unsigned int, exit_timer, class_offsets::timer_exit);
	member(unsigned int, respawn_timer, class_offsets::timer_respawn);
	member(unsigned int, home_timer, class_offsets::timer_home);

	member(float, air_accel, class_offsets::air_accel);
	member(float, air_decel, class_offsets::air_decel);

	member(bool, allow_shoulder_camera, class_offsets::allow_shoulder_camera);
	member(bool, ballistics_enabled, class_offsets::ballistics_enabled);
	member(bool, chart, class_offsets::chart);
	member(bool, compass, class_offsets::compass);
	member(bool, crosshair, class_offsets::crosshair);
	member(bool, group_map, class_offsets::group_map);
	member(bool, satellite, class_offsets::satellite);

};

struct mode_config_data_t
{

	get_member(gameplay_config_data_t*, config_data, class_offsets::gameplay);

};

struct viewmodel_preference_data_t
{

	member(float, aim_fov, class_offsets::viewmodel_aim_fov);
	member(float, hip_fov, class_offsets::viewmodel_hip_fov);
	member(float, x_offset, class_offsets::viewmodel_x_offset);
	member(float, y_offset, class_offsets::viewmodel_y_offset);
	member(float, z_offset, class_offsets::viewmodel_z_offset);

};

struct preference_data_t
{

	get_member(viewmodel_preference_data_t*, viewmodel_data, class_offsets::viewmodel_data);

};

struct provider_t
{
	static auto get_instance() -> provider_t*
	{

		static auto vtable = (provider_t*)unity_classes::provider->get_vtable(mono::get_root_domain())->get_static_field_data();
		return vtable;

	}

	auto bind_address()
	{

		uintptr_t managed_string = driver.read<uintptr_t>(cast_this + class_offsets::bind_address);
		auto wide = driver.read_wstr(managed_string + 0x14);
		return std::string(wide.begin(), wide.end());

	}

	member(UINT, time, class_offsets::provider_time);
	member(int, camera_mode, class_offsets::camera_mode);

	get_member(unity_list_t*, clients, class_offsets::clients)
	get_member(BYTE, max_clients, class_offsets::max_players)
	get_member(bool, is_connected, class_offsets::is_connected)
	get_member(mode_config_data_t*, config_data, class_offsets::mode_config_data);
	get_member(preference_data_t*, preference_data, class_offsets::preference_data);

};

struct main_camera_t
{
	static auto get_instance() -> main_camera_t*
	{
		static auto instance = (main_camera_t*)unity_classes::maincamera->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}
	get_member(uintptr_t, instance, class_offsets::camera_instance)
};

struct base_object_t
{
	uint64_t previousObjectLink; //0x0000
	uint64_t nextObjectLink; //0x0008
	uint64_t object; //0x0010
};

struct game_object_manager_t
{
	uint64_t lastTaggedObject; //0x0
	uint64_t taggedObjects; //0x8
	uint64_t lastMainCameraTagged; // 0x10
	uint64_t MainCameraTagged; // 0x18
	uint64_t lastActiveObject; //0x20
	uint64_t activeObjects; // 0x28
};

template <class type>
std::vector<type> get_active_objects(const char* object_name)
{
	// NULL = GOM
	auto camera_objects = driver.read<std::array<uintptr_t, 2>>(NULL + offsetof(game_object_manager_t, game_object_manager_t::lastTaggedObject));
	std::vector<type> tmp_return{};

	char name[256];
	uintptr_t class_name_ptr = 0x00;

	base_object_t activeObject = driver.read<base_object_t>(camera_objects[1]);
	base_object_t lastObject = driver.read<base_object_t>(camera_objects[0]);

	if (activeObject.object)
	{
		while (activeObject.object != 0 && activeObject.object != lastObject.object)
		{
			class_name_ptr = driver.read<uintptr_t>(activeObject.object + 0x60);
			driver.read(class_name_ptr + 0x0, &name, sizeof(name));

			if (strcmp(name, object_name) == 0)
			{
				auto unk1 = driver.read<uintptr_t>(activeObject.object + 0x30);
				auto unk2 = driver.read<uintptr_t>(unk1 + 0x18);
				tmp_return.emplace_back(driver.read<type>(unk2 + 0x28));
			}

			activeObject = driver.read<base_object_t>(activeObject.nextObjectLink);
		}
		return tmp_return;
	}

	return std::vector<type>{};

}

struct shader_t
{

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::core_module_path, "UnityEngine", "Shader");
	}

	static shader_t* find(NaOrganization::MidTerm::VmGeneralType::String name)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Shader", "Find", { "System.String" });
		return NaOrganization::MidTerm::NaMethodInvoker<shader_t*, NaOrganization::MidTerm::VmGeneralType::String>(invoker.method.GetInvokeAddress()).Invoke(name);

	}

};

struct asset_bundle_t
{

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::asset_bundle_path, "UnityEngine", "AssetBundle");
	}

	static asset_bundle_t* load_from_file(NaOrganization::MidTerm::VmGeneralType::String path)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.AssetBundle", "LoadFromFile", { "System.String" });
		return NaOrganization::MidTerm::NaMethodInvoker<asset_bundle_t*, NaOrganization::MidTerm::VmGeneralType::String>(invoker.method.GetInvokeAddress()).Invoke(path);

	}

	shader_t* load_asset(NaOrganization::MidTerm::VmGeneralType::String name)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.AssetBundle", "LoadAsset", { "System.String", "System.Type" });

		//auto type = mono::GetMonoReflectionType(reinterpret_cast<MonoClass*>(shader_t::ThisClass().klass.klass));

		//return NaOrganization::MidTerm::NaMethodInvoker<shader_t*, NaOrganization::MidTerm::VmGeneralType::String, MonoReflectionType*>(invoker.method.GetInvokeAddress()).Invoke(name, type);

		return nullptr;

	}

};

struct material_t
{

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::core_module_path, "UnityEngine", "Material");
	}

	void set_shader(shader_t* shader)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "System.Void", "set_shader", { "UnityEngine.Shader" });
		return NaOrganization::MidTerm::NaMethodInvoker<void, material_t*, shader_t*>(invoker.method.GetInvokeAddress()).Invoke(this, shader);

	}

	shader_t* get_shader()
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Shader", "get_shader", { });
		return NaOrganization::MidTerm::NaMethodInvoker<shader_t*, material_t*>(invoker.method.GetInvokeAddress()).Invoke(this);

	}

	void set_int(NaOrganization::MidTerm::VmGeneralType::String str, int value)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "System.Void", "SetInt", { "System.String", "System.Int32" });
		return NaOrganization::MidTerm::NaMethodInvoker<void, material_t*, NaOrganization::MidTerm::VmGeneralType::String, int>(invoker.method.GetInvokeAddress()).Invoke(this, str, value);

	}

	void set_color(NaOrganization::MidTerm::VmGeneralType::String str, UnityEngine_Color_o color)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "System.Void", "SetColor", { "System.String", "UnityEngine.Color" });
		return NaOrganization::MidTerm::NaMethodInvoker<void, material_t*, NaOrganization::MidTerm::VmGeneralType::String, UnityEngine_Color_o>(invoker.method.GetInvokeAddress()).Invoke(this, str, color);

	}

	void enable_keyword(NaOrganization::MidTerm::VmGeneralType::String str)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "System.Void", "EnableKeyword", { "System.String" });
		return NaOrganization::MidTerm::NaMethodInvoker<void, material_t*, NaOrganization::MidTerm::VmGeneralType::String>(invoker.method.GetInvokeAddress()).Invoke(this, str);

	}

	void disable_keyword(NaOrganization::MidTerm::VmGeneralType::String str)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "System.Void", "DisableKeyword", { "System.String" });
		return NaOrganization::MidTerm::NaMethodInvoker<void, material_t*, NaOrganization::MidTerm::VmGeneralType::String>(invoker.method.GetInvokeAddress()).Invoke(this, str);

	}

};

struct renderer_t
{
	
	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::core_module_path, "UnityEngine", "Renderer");
	}

	material_t* get_material()
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Material", "get_material", { });
		return NaOrganization::MidTerm::NaMethodInvoker<material_t*, renderer_t*>(invoker.method.GetInvokeAddress()).Invoke(this);

	}

};

struct skinned_mesh_renderer_t : renderer_t
{

};

struct raycast_hit_t
{

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::physics_module, "UnityEngine", "RaycastHit");
	}

	uintptr_t get_transform()
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "UnityEngine.Transform", "get_transform", { });
		return NaOrganization::MidTerm::NaMethodInvoker<uintptr_t, raycast_hit_t*>(invoker.method.GetInvokeAddress()).Invoke(this);

	}

	vec3_t point{};
	vec3_t normal{};
	unsigned int face_id{};
	float distance{};
	vec2_t uv{};
	int collider{};

};

enum QueryTriggerInteraction
{

	UseGlobal,
	Ignore,
	Collide

};

struct physics_t
{

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::physics_module, "UnityEngine", "Physics");
	}

	static bool raycast(vec3_t origin, vec3_t direction, float max_distance, int layer_mask = 479970304, QueryTriggerInteraction trigger = QueryTriggerInteraction::Ignore)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "System.Boolean", "Raycast", { "UnityEngine.Vector3", "UnityEngine.Vector3", "System.Single", "System.Int32", "UnityEngine.QueryTriggerInteraction" });
		return NaOrganization::MidTerm::NaMethodInvoker<bool, vec3_t, vec3_t, float, int, QueryTriggerInteraction>(invoker.method.GetInvokeAddress()).Invoke(origin, direction, max_distance, layer_mask, trigger);

	}

	static bool linecast(vec3_t from, vec3_t to, raycast_hit_t* hit, int mask = 479970304, QueryTriggerInteraction interaction = QueryTriggerInteraction::Ignore)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "System.Boolean", "Linecast", { "UnityEngine.Vector3", "UnityEngine.Vector3", "UnityEngine.RaycastHit", "System.Int32", "UnityEngine.QueryTriggerInteraction" });
		return NaOrganization::MidTerm::NaMethodInvoker<bool, vec3_t, vec3_t, raycast_hit_t**, int, QueryTriggerInteraction>(invoker.method.GetInvokeAddress()).Invoke(from, to, &hit, mask, interaction);

	}

};

struct human_clothes_t
{

	get_member(TransformInternal*, skull, class_offsets::skull);
	get_member(unity_list_2d<skinned_mesh_renderer_t*>*, renderers, class_offsets::characters_mesh_renderers);

};

struct player_clothing_t
{

	get_member(human_clothes_t*, human_clothes, class_offsets::characterclothes);
	get_member(human_clothes_t*, third_clothes, class_offsets::third_clothing);
	get_member(human_clothes_t*, first_clothes, class_offsets::first_clothes);

};

struct raycast_info_t
{

	get_member(uintptr_t, zombie, class_offsets::raycast_zombie);
	member(uintptr_t, player, class_offsets::raycast_player);
	member(int, material, class_offsets::raycast_material);
	member(int, limb, class_offsets::raycast_limb);
	member(vec3_t, point, class_offsets::raycast_point);
	member(vec3_t, direction, class_offsets::raycast_direction);
	member(uintptr_t, transform, class_offsets::raycast_transform);
	member(float, distance, class_offsets::raycast_distance);

};

struct damage_tool_t
{

	static NaOrganization::MidTerm::NaResolver::Class ThisClass() {
		return NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::assembly_path, "SDG.Unturned", "DamageTool");
	}

	static raycast_info_t* raycast(game::ray_t* ray, float range, int mask = 479970304, player_t* ignore_player = NULL)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "SDG.Unturned.RaycastInfo", "raycast", { "UnityEngine.Ray", "System.Single", "System.Int32", "SDG.Unturned.Player" });
		return NaOrganization::MidTerm::NaMethodInvoker<raycast_info_t*, game::ray_t*, float, int, player_t*>(invoker.method.GetInvokeAddress()).Invoke(ray, range, mask, ignore_player);

	}

	static player_t* get_player(uintptr_t transform)
	{

		static auto invoker = NaOrganization::MidTerm::UnityResolver.GetMethod(ThisClass(), "SDG.Unturned.Player", "getPlayer", { });
		return NaOrganization::MidTerm::NaMethodInvoker<player_t*, uintptr_t>(invoker.method.GetInvokeAddress()).Invoke(transform);

	}

};

struct player_animator_t
{

	member(vec3_t, scope_sway, class_offsets::sway);
	member(vec3_t, last_item_position, class_offsets::last_item_position);
	member(float, viewmodel_sway_multiplier, class_offsets::viewmodel_sway_multiplayer);
	member(int, gesture, class_offsets::_gesture);

	member(bool, lean_obstructed, class_offsets::lean_obstructed);
	member(bool, last_frame_had_item_position, class_offsets::last_frame_had_item_position);
	member(float, shoulder, class_offsets::_shoulder);

	get_member(TransformInternal*, camera_transform, class_offsets::viewmodel_camera_transform);
	get_member(uintptr_t, viewmodel_camera, class_offsets::viewmodel_camera);

	get_member(skinned_mesh_renderer_t*, first_renderer, class_offsets::first_renderer_01);
	get_member(skinned_mesh_renderer_t*, third_renderer_1, class_offsets::third_renderer_01);
	get_member(skinned_mesh_renderer_t*, third_renderer_2, class_offsets::third_renderer_02);

};

struct zombie_t : mono_behaviour_t
{

	get_member(unsigned short, health, class_offsets::zombie_health);
	get_member(EZombieSpeciality, specialty, class_offsets::zombie_specialty);

};

struct zombie_region_t
{

	get_member(unity_list_t*, zombies, class_offsets::zombies);

};

struct zombie_manager_t
{

	static auto get_instance() -> zombie_manager_t*
	{
		static auto instance = (zombie_manager_t*)unity_classes::zombiemanager->get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;
	}

	get_member(unity_list_t*, ticking_zombies, class_offsets::ticking_zombies);
	get_member(monoArray<zombie_region_t*>*, zombie_regions, class_offsets::zombie_regions);

};

struct player_visibility_data_t
{

	player_t* ptr{ nullptr };
	ULONGLONG vis_time{ NULL };
	bool visible{ false };

};

std::vector<player_visibility_data_t> g_vis_data{};
std::mutex g_vis_mtx{};

struct bounding_box_t
{

	float x, y, w, h;
	bool on_screen;

	void calculate(ImVec2 base_screen, ImVec2 head_screen)
	{

		float height = head_screen.y - base_screen.y;
		float width = height / 4;

		float Entity_x = base_screen.x - width;
		float Entity_y = base_screen.y;
		float Entity_w = height / 2;
		x = floor(Entity_x);
		y = floor(Entity_y);
		w = floor(Entity_w);
		h = floor(height);

		on_screen = true;

	}

};

struct cached_player_t
{

	steam_player_t* steamplayer{ nullptr };
	player_t* player{ nullptr };
	player_life_t* playerlife{ nullptr };
	player_look_t* playerlook{ nullptr };
	human_clothes_t* clothes{ nullptr };
	player_movement_t* movement{ nullptr };
	internal_transform_t* transform{ nullptr };
	player_equipment_t* equipment{ nullptr };
	std::string name{ "none" };
	std::string weapon_name{ "none" };
	bounding_box_t bounds{};
	player_animator_t* animator{ nullptr };
	player_quests_t* quests{ nullptr };
	vec3_t origin{};
	int ref_count{ 1 };

	static cached_player_t* ref_factory() { return new cached_player_t(); }
	
	void add_ref() { ++ref_count; }

	void release() { if (--ref_count == 0) delete this; }

	uintptr_t get_player() { return reinterpret_cast<uintptr_t>(player); }

	uintptr_t get_steam_player() { return reinterpret_cast<uintptr_t>(steamplayer); }

	uintptr_t get_player_look() { return reinterpret_cast<uintptr_t>(playerlook); }

	uintptr_t get_player_movement() { return reinterpret_cast<uintptr_t>(movement); }

	player_visibility_data_t get_vis_data()
	{

		player_visibility_data_t ret{};

		g_vis_mtx.lock();

		for (const auto& data : g_vis_data)
			if (data.ptr == player)
				ret = data;

		g_vis_mtx.unlock();

		return ret;

	}

	cached_player_t& operator=(const cached_player_t& other)
	{

		steamplayer = other.steamplayer;
		player = other.player;
		playerlife = other.playerlife;
		playerlook = other.playerlook;
		clothes = other.clothes;
		movement = other.movement;
		transform = other.transform;
		equipment = other.equipment;
		name = other.name;
		weapon_name = other.weapon_name;
		bounds = other.bounds;
		animator = other.animator;
		quests = other.quests;

		return *this;

	}

};

struct cached_item_t
{

	vec3_t position{};
	std::string name{};
	BYTE count{};
	EItemRarity rarity{};
	EItemType type{};
	int ref_count{ 1 };

	static cached_item_t* ref_factory() { return new cached_item_t(); }

	void add_ref() { ++ref_count; }

	void release() { if (--ref_count == 0) delete this; }

	cached_item_t& operator=(const cached_item_t& other)
	{

		position = other.position;

		return *this;

	}

};

struct cached_zombie_t
{

	vec3_t pos;
	unsigned short health;
	EZombieSpeciality specialty;
	bounding_box_t bounds{};

};

struct snapshot_info_t
{

	vec3_t position{};
	vec3_t head_pos{};
	ULONGLONG time{};

};

struct player_snapshot_t
{

	cached_player_t player{ nullptr };
	std::vector<snapshot_info_t> positions{};
	bool dormant{ false };

};

inline std::vector<player_snapshot_t> g_snapshots{};

namespace cheat
{

	inline std::vector<cached_player_t> players{};
	inline std::vector<cached_item_t> items{};
	inline std::vector<cached_zombie_t> zombies{};

	inline cached_player_t local_player;
	inline std::mutex sync{};
	inline item_gun_asset_t* local_gun{};

}

namespace game
{

	inline bool is_visible(vec3_t from, vec3_t pos)
	{

		auto direction = pos - from;
		return !physics_t::raycast(from, direction, direction.magnitude());

	}

}