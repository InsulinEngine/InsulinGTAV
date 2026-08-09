#pragma once
// GENERATED FILE - do not edit by hand.
// Native wrappers for GTA V PS4 CUSA00411 v1.57, produced by tools/natives.
// The literal in each _i<> call is an RVA into eboot.bin, not a native hash;
// the invoker adds the runtime image base.
#include "invoker.h"
#include "rage/types/base_types.h"
#define _i rage::invoker::invoke

namespace native {

	static float absf(float value) { return _i<float>(0x9F87C0, value); }
	static int absi(int value) { return _i<int>(0x9F87A0, value); }
	static Void activate_damage_tracker_on_network_id(int netID, bool p1) { return _i<Void>(0xA533E0, netID, p1); }
	static Void activate_frontend_menu(uint32_t menuhash, bool Toggle_Pause, int component) { return _i<Void>(0x9E5160, menuhash, Toggle_Pause, component); }
	static Void activate_physics(Entity entity) { return _i<Void>(0xAA14F0, entity); }
	// [aligned] static Void activate_rockstar_editor() { return _i<Void>(0xAAE340); }
	static Void add_ammo_to_ped(Ped ped, uint32_t weaponHash, int ammo) { return _i<Void>(0xB38E30, ped, weaponHash, ammo); }
	static Void add_armour_to_ped(Ped ped, int amount) { return _i<Void>(0xA90850, ped, amount); }
	static Blip add_blip_for_coord(float x, float y, float z) { return _i<Blip>(0x9E22D0, x, y, z); }
	static Blip add_blip_for_entity(Entity entity) { return _i<Blip>(0x9E2290, entity); }
	static Blip add_blip_for_pickup(Pickup pickup) { return _i<Blip>(0x9E22B0, pickup); }
	static Blip add_blip_for_radius(float posX, float posY, float posZ, float radius) { return _i<Blip>(0x9E21A0, posX, posY, posZ, radius); }
	static Void add_cover_blocking_area(float playerX, float playerY, float playerZ, float radiusX, float radiusY, float radiusZ, bool p6, bool p7, bool p8, bool p9) { return _i<Void>(0xAF52F0, playerX, playerY, playerZ, radiusX, radiusY, radiusZ, p6, p7, p8, p9); }
	static ScrHandle add_cover_point(float p0, float p1, float p2, float p3, Any p4, Any p5, Any p6, bool p7) { return _i<ScrHandle>(0xAF4BA0, p0, p1, p2, p3, p4, p5, p6, p7); }
	static Void add_door_to_system(uint32_t doorHash, uint32_t modelHash, float x, float y, float z, bool p5, bool p6, bool p7) { return _i<Void>(0xA644B0, doorHash, modelHash, x, y, z, p5, p6, p7); }
	static Any add_entity_icon(Entity entity, const char* icon) { return _i<Any>(0x9CD330, entity, icon); }
	static Void add_explosion(float x, float y, float z, int explosionType, float damageScale, bool isAudible, bool isInvisible, float cameraShake, bool noDamage) { return _i<Void>(0x9BB3E0, x, y, z, explosionType, damageScale, isAudible, isInvisible, cameraShake, noDamage); }
	static Void add_explosion_with_user_vfx(float x, float y, float z, int explosionType, uint32_t explosionFx, float damageScale, bool isAudible, bool isInvisible, float cameraShake) { return _i<Void>(0x9BB460, x, y, z, explosionType, explosionFx, damageScale, isAudible, isInvisible, cameraShake); }
	static int add_hospital_restart(float x, float y, float z, float p3, Any p4) { return _i<int>(0x9F7870, x, y, z, p3, p4); }
	static Void add_line_to_conversation(int p0, const char* p1, const char* p2, int p3, int p4, bool p5, bool p6, bool p7, bool p8, int p9, bool p10, bool p11, bool p12) { return _i<Void>(0x982A00, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12); }
	static Any add_navmesh_blocking_object(float p0, float p1, float p2, float p3, float p4, float p5, float p6, bool p7, Any p8) { return _i<Any>(0xA70D10, p0, p1, p2, p3, p4, p5, p6, p7, p8); }
	static Void add_navmesh_required_region(float x, float y, float radius) { return _i<Void>(0xA70BD0, x, y, radius); }
	static Void add_next_message_to_previous_briefs(bool p0) { return _i<Void>(0x9E13A0, p0); }
	static void add_owned_explosion(Ped ped, float x, float y, float z, int explosionType, float damageScale, bool isAudible, bool isInvisible, float cameraShake) { _i<Void>(0x9BB420, ped, x, y, z, explosionType, damageScale, isAudible, isInvisible, cameraShake); }
	static Void add_patrol_route_link(Any p0, Any p1) { return _i<Void>(0xAF6300, p0, p1); }
	static Void add_patrol_route_node(int p0, const char* p1, float x1, float y1, float z1, float x2, float y2, float z2, int p8) { return _i<Void>(0xAF6270, p0, p1, x1, y1, z1, x2, y2, z2, p8); }
	static Void add_ped_to_conversation(Any p0, Any ped, const char* p2) { return _i<Void>(0x982AF0, p0, ped, p2); }
	static Any add_petrol_decal(float x, float y, float z, float groundLvl, float width, float transparency) { return _i<Any>(0x9CFF70, x, y, z, groundLvl, width, transparency); }
	static Void add_pickup_to_interior_room_by_name(Pickup pickup, const char* roomName) { return _i<Void>(0x9E88A0, pickup, roomName); }
	static Any add_police_restart(float p0, float p1, float p2, float p3, Any p4) { return _i<Any>(0x9F79F0, p0, p1, p2, p3, p4); }
	static Any add_relationship_group(const char* name, uint32_t* groupHash) { return _i<Any>(0xA93950, name, groupHash); }
	static Void add_script_to_random_ped(const char* name, uint32_t model, float p2, float p3) { return _i<Void>(0x987E90, name, model, p2, p3); }
	static ScrHandle add_shocking_event_at_position(int type, float x, float y, float z, float duration) { return _i<ScrHandle>(0x9B9470, type, x, y, z, duration); }
	static ScrHandle add_shocking_event_for_entity(int type, Entity entity, float duration) { return _i<ScrHandle>(0x9B9500, type, entity, duration); }
	static int add_stunt_jump(float p0, float p1, float p2, float p3, float p4, float p5, float p6, float p7, float p8, float p9, float p10, float p11, float p12, float p13, float p14, Any p15, Any p16, Any p17) { return _i<int>(0x9F9370, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17); }
	static int add_stunt_jump_angled(float p0, float p1, float p2, float p3, float p4, float p5, float p6, float p7, float p8, float p9, float p10, float p11, float p12, float p13, float p14, float p15, float p16, Any p17, Any p18, Any p19) { return _i<int>(0x9F9450, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19); }
	static Void add_text_component_float(float value, int decimalPlaces) { return _i<Void>(0x9E08B0, value, decimalPlaces); }
	static Void add_text_component_formatted_integer(int value, bool commaSeparated) { return _i<Void>(0x9E0910, value, commaSeparated); }
	static Void add_text_component_integer(int value) { return _i<Void>(0x9E08A0, value); }
	static Void add_text_component_substring_blip_name(Blip blip) { return _i<Void>(0x9E08E0, blip); }
	static Void add_text_component_substring_player_name(const char* text) { return _i<Void>(0x9E08F0, text); }
	static Void add_text_component_substring_text_label(const char* labelName) { return _i<Void>(0x9E08C0, labelName); }
	static Void add_text_component_substring_text_label_hash_key(uint32_t gxtEntryHash) { return _i<Void>(0x9E08D0, gxtEntryHash); }
	static Void add_text_component_substring_time(int timestamp, int flags) { return _i<Void>(0x9E0900, timestamp, flags); }
	static Void add_text_component_substring_website(const char* website) { return _i<Void>(0x9E09B0, website); }
	static Void add_to_clock_time(int hours, int minutes, int seconds) { return _i<Void>(0x996A20, hours, minutes, seconds); }
	static bool add_to_itemset(Any p0, Any p1) { return _i<bool>(0x9E9730, p0, p1); }
	static Void add_vehicle_stuck_check_with_warp(Any p0, float p1, Any p2, bool p3, bool p4, bool p5, Any p6) { return _i<Void>(0xB201D0, p0, p1, p2, p3, p4, p5, p6); }
	static Void add_vehicle_subtask_attack_coord(Ped ped, float x, float y, float z) { return _i<Void>(0xAF6510, ped, x, y, z); }
	static Void add_vehicle_subtask_attack_ped(Ped ped, Ped ped2) { return _i<Void>(0xAF6520, ped, ped2); }
	static Void add_vehicle_upsidedown_check(Vehicle vehicle) { return _i<Void>(0xB1BA30, vehicle); }
	static Void advance_clock_time_to(int hour, int minute, int second) { return _i<Void>(0x996A00, hour, minute, second); }
	static Void animated_shake_cam(Cam cam, const char* p1, const char* p2, const char* p3, float amplitude) { return _i<Void>(0x9936A0, cam, p1, p2, p3, amplitude); }
	static Void apply_damage_to_ped(Ped ped, int damageAmount, bool p2) { return _i<Void>(0xA94490, ped, damageAmount, p2); }
	static Void apply_force_to_entity(Entity entity, int forceFlags, float x, float y, float z, float offX, float offY, float offZ, int p8, bool isLocal, bool p10, bool isMassRel, bool p12, bool p13) { return _i<Void>(0x9B4C40, entity, forceFlags, x, y, z, offX, offY, offZ, p8, isLocal, p10, isMassRel, p12, p13); }
	static Void apply_impulse_to_cloth(float posX, float posY, float posZ, float vecX, float vecY, float vecZ, float impulse) { return _i<Void>(0xAA1320, posX, posY, posZ, vecX, vecY, vecZ, impulse); }
	static Void apply_ped_blood(Ped ped, int boneIndex, float xRot, float yRot, float zRot, const char* woundType) { return _i<Void>(0xA98C50, ped, boneIndex, xRot, yRot, zRot, woundType); }
	static Void apply_ped_blood_by_zone(Ped ped, Any p1, float p2, float p3, Any* p4) { return _i<Void>(0xA98D20, ped, p1, p2, p3, p4); }
	static Void apply_ped_blood_damage_by_zone(Ped ped, Any p1, float p2, float p3, Any p4) { return _i<Void>(0xA98B00, ped, p1, p2, p3, p4); }
	static Void apply_ped_blood_specific(Ped ped, Any p1, float p2, float p3, float p4, float p5, Any p6, float p7, Any* p8) { return _i<Void>(0xA98E00, ped, p1, p2, p3, p4, p5, p6, p7, p8); }
	static Void apply_ped_damage_decal(Ped ped, int p1, float p2, float p3, float p4, float p5, float p6, int p7, bool p8, const char* p9) { return _i<Void>(0xA98F10, ped, p1, p2, p3, p4, p5, p6, p7, p8, p9); }
	static Void apply_ped_damage_pack(Ped ped, const char* damagePack, float damage, float mult) { return _i<Void>(0xA99030, ped, damagePack, damage, mult); }
	static Void app_clear_block() { return _i<Void>(0x97ACC0); }
	static Void app_close_app() { return _i<Void>(0x97AD20); }
	static Void app_close_block() { return _i<Void>(0x97AD60); }
	static bool app_data_valid() { return _i<bool>(0x97A8D0); }
	static bool app_delete_app_data(const char* appName) { return _i<bool>(0x97AE60, appName); }
	static Any app_get_deleted_file_status() { return _i<Any>(0x97AE40); }
	static float app_get_float(const char* property) { return _i<float>(0x97A9B0, property); }
	static int app_get_int(const char* property) { return _i<int>(0x97A8F0, property); }
	static const char* app_get_string(const char* property) { return _i<const char*>(0x97AA80, property); }
	static bool app_has_linked_social_club_account() { return _i<bool>(0x97ADC0); }
	static bool app_has_synced_data(const char* appName) { return _i<bool>(0x97ADE0, appName); }
	static Void app_save_data() { return _i<Void>(0x97AE20); }
	static Void app_set_app(const char* appName) { return _i<Void>(0x97AB90, appName); }
	static Void app_set_block(const char* blockName) { return _i<Void>(0x97AC70, blockName); }
	static Void app_set_float(const char* property, float value) { return _i<Void>(0x97AB60, property, value); }
	static Void app_set_string(const char* property, const char* value) { return _i<Void>(0x97AB80, property, value); }
	static bool are_all_navmesh_regions_loaded() { return _i<bool>(0xA70C90); }
	static bool are_all_vehicle_windows_intact(Vehicle vehicle) { return _i<bool>(0xB23E40, vehicle); }
	static bool are_any_vehicle_seats_free(Vehicle vehicle) { return _i<bool>(0xB23EA0, vehicle); }
	static bool are_player_flashing_stars_about_to_drop(Player player) { return _i<bool>(0xAA9940, player); }
	static bool are_player_stars_greyed_out(Player player) { return _i<bool>(0xAA99B0, player); }
	// [aligned] static bool are_profile_settings_valid() { return _i<bool>(0x9FABC0); }
	static bool are_strings_equal(const char* string1, const char* string2) { return _i<bool>(0x9F8710, string1, string2); }
	static Void assisted_movement_close_route() { return _i<Void>(0xAAB8C0); }
	static Void assisted_movement_flush_route() { return _i<Void>(0xAAB910); }
	static bool assisted_movement_is_route_loaded(const char* route) { return _i<bool>(0xAF7430, route); }
	static Void assisted_movement_override_load_distance_this_frame(float dist) { return _i<Void>(0xAF74C0, dist); }
	static Void assisted_movement_remove_route(const char* route) { return _i<Void>(0xAF73F0, route); }
	static Void assisted_movement_request_route(const char* route) { return _i<Void>(0xAF73E0, route); }
	static Void assisted_movement_set_route_properties(const char* route, int props) { return _i<Void>(0xAF74B0, route, props); }
	static float atan(float p0) { return _i<float>(0x9F67F0, p0); }
	static float atan2(float p0, float p1) { return _i<float>(0x9F6830, p0, p1); }
	static Void attach_cam_to_ped_bone(Cam cam, Ped ped, int boneIndex, float x, float y, float z, bool heading) { return _i<Void>(0x992830, cam, ped, boneIndex, x, y, z, heading); }
	static Void attach_entity_to_entity(Entity entity1, Entity entity2, int boneIndex, float xPos, float yPos, float zPos, float xRot, float yRot, float zRot, bool p9, bool useSoftPinning, bool collision, bool isPed, int vertexIndex, bool fixedRot) { return _i<Void>(0x9B4CC0, entity1, entity2, boneIndex, xPos, yPos, zPos, xRot, yRot, zRot, p9, useSoftPinning, collision, isPed, vertexIndex, fixedRot); }
	static Void attach_entity_to_entity_physically(Entity entity1, Entity entity2, int boneIndex1, int boneIndex2, float xPos1, float yPos1, float zPos1, float xPos2, float yPos2, float zPos2, float xRot, float yRot, float zRot, float breakForce, bool fixedRot, bool p15, bool collision, bool p17, int p18) { return _i<Void>(0x9B4EC0, entity1, entity2, boneIndex1, boneIndex2, xPos1, yPos1, zPos1, xPos2, yPos2, zPos2, xRot, yRot, zRot, breakForce, fixedRot, p15, collision, p17, p18); }
	static Void attach_portable_pickup_to_ped(Ped ped, Any p1) { return _i<Void>(0xA65BC0, ped, p1); }
	static Void attach_rope_to_entity(Object rope, Entity entity, float x, float y, float z, bool p5) { return _i<Void>(0xAA0AD0, rope, entity, x, y, z, p5); }
	static Void attach_synchronized_scene_to_entity(int sceneID, Entity entity, int boneIndex) { return _i<Void>(0xA9CE70, sceneID, entity, boneIndex); }
	static Void attach_tv_audio_to_entity(Entity entity) { return _i<Void>(0x9CD920, entity); }
	static Any audio_is_scripted_music_playing() { return _i<Any>(0x9869C0); }
	static Void begin_replay_stats(Any p0, Any p1) { return _i<Void>(0x9F80A0, p0, p1); }
	static Void begin_srl() { return _i<Void>(0xADAD70); }
	static Void begin_text_command_clear_print(const char* text) { return _i<Void>(0x9E0860, text); }
	static Void begin_text_command_display_help(const char* inputType) { return _i<Void>(0x9E0790, inputType); }
	static Void begin_text_command_display_text(const char* text) { return _i<Void>(0x9E0690, text); }
	static Void begin_text_command_is_message_displayed(const char* text) { return _i<Void>(0x9E0660, text); }
	static Void begin_text_command_is_this_help_message_being_displayed(const char* labelName) { return _i<Void>(0x9E07D0, labelName); }
	static Void begin_text_command_print(const char* GxtEntry) { return _i<Void>(0x9E0630, GxtEntry); }
	static Void begin_text_command_scaleform_string(const char* componentType) { return _i<Void>(0x9D19E0, componentType); }
	static Void begin_text_command_set_blip_name(const char* gxtentry) { return _i<Void>(0x9E0810, gxtentry); }
	static Void begin_text_command_thefeed_post(const char* type) { return _i<Void>(0x9DFFC0, type); }
	static Void blip_siren(Vehicle vehicle) { return _i<Void>(0x985510, vehicle); }
	static Void block_decision_maker_event(uint32_t name, int type) { return _i<Void>(0x9B9400, name, type); }
	static float calculate_travel_distance_between_points(float x1, float y1, float z1, float x2, float y2, float z2) { return _i<float>(0xA70FD0, x1, y1, z1, x2, y2, z2); }
	static Void call_scaleform_movie_method(int scaleform, const char* method) { return _i<Void>(0x9D1320, scaleform, method); }
	static bool cancel_music_event(const char* eventName) { return _i<bool>(0x986A90, eventName); }
	static Void cancel_stunt_jump() { return _i<Void>(0x9F96B0); }
	static bool can_create_random_bike_rider() { return _i<bool>(0xA90C50); }
	static bool can_create_random_cops() { return _i<bool>(0xA95090); }
	static bool can_create_random_driver() { return _i<bool>(0xA90C00); }
	static bool can_create_random_ped(bool unk) { return _i<bool>(0xA90A90, unk); }
	static bool can_knock_ped_off_vehicle(Ped ped) { return _i<bool>(0xA94B80, ped); }
	static bool can_ped_hear_player(Player player, Ped ped) { return _i<bool>(0xAACF60, player, ped); }
	static bool can_ped_in_combat_see_target(Ped ped, Ped target) { return _i<bool>(0xA93080, ped, target); }
	static bool can_ped_ragdoll(Ped ped) { return _i<bool>(0xA97DB0, ped); }
	static bool can_phone_be_seen_on_screen() { return _i<bool>(0xAAF2F0); }
	static bool can_player_start_mission(Player player) { return _i<bool>(0xAAA4C0, player); }
	static bool can_register_mission_entities(int p0, Any p1, Any p2, Any p3) { return _i<bool>(0xA53920, p0, p1, p2, p3); }
	static bool can_register_mission_objects(int p0) { return _i<bool>(0xA536B0, p0); }
	static bool can_register_mission_peds(int p0) { return _i<bool>(0xA53730, p0); }
	static bool can_register_mission_pickups(int p0) { return _i<bool>(0xA53850, p0); }
	static bool can_register_mission_vehicles(int p0) { return _i<bool>(0xA537B0, p0); }
	static bool can_set_enter_state_for_registered_entity(const char* cutsceneEntName, uint32_t modelHash) { return _i<bool>(0x999920, cutsceneEntName, modelHash); }
	static bool can_set_exit_state_for_camera(bool p0) { return _i<bool>(0x999A20, p0); }
	static bool can_set_exit_state_for_registered_entity(const char* cutsceneEntName, uint32_t modelHash) { return _i<bool>(0x9999A0, cutsceneEntName, modelHash); }
	static bool can_shuffle_seat(Vehicle vehicle, Any p1) { return _i<bool>(0xB26720, vehicle, p1); }
	static bool can_use_weapon_on_parachute(uint32_t weaponHash) { return _i<bool>(0xB3B110, weaponHash); }
	static Void cap_interior(int interiorID, bool toggle) { return _i<Void>(0x9E9110, interiorID, toggle); }
	static Void cell_cam_activate(bool p0, bool p1) { return _i<Void>(0xAAF350, p0, p1); }
	static bool cell_cam_is_char_visible_no_face_check(Entity entity) { return _i<bool>(0xAAF430, entity); }
	static Void change_player_ped(Player player, Ped ped, bool b2, bool b3) { return _i<Void>(0xAA93B0, player, ped, b2, b3); }
	static Void clean_itemset(Any p0) { return _i<Void>(0x9E9920, p0); }
	static Void clear_additional_text(int p0, bool p1) { return _i<Void>(0x9E0D90, p0, p1); }
	static Void clear_all_broken_glass() { return _i<Void>(0x986BD0); }
	static Void clear_all_help_messages() { return _i<Void>(0x9E0B40); }
	static Void clear_all_ped_props(Ped ped) { return _i<Void>(0xA97580, ped); }
	static Void clear_ambient_zone_list_state(Any* p0, bool p1) { return _i<Void>(0x985290, p0, p1); }
	static Void clear_ambient_zone_state(const char* zoneName, bool p1) { return _i<Void>(0x9851C0, zoneName, p1); }
	static Void clear_angled_area_of_vehicles(float p0, float p1, float p2, float p3, float p4, float p5, float p6, bool p7, bool p8, bool p9, bool p10, bool p11, Any p12) { return _i<Void>(0x9F7550, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12); }
	static Void clear_area(float X, float Y, float Z, float radius, bool p4, bool ignoreCopCars, bool ignoreObjects, bool p7) { return _i<Void>(0x9F7450, X, Y, Z, radius, p4, ignoreCopCars, ignoreObjects, p7); }
	static Void clear_area_of_cops(float x, float y, float z, float radius, int flags) { return _i<Void>(0x9F7670, x, y, z, radius, flags); }
	static Void clear_area_of_objects(float x, float y, float z, float radius, int flags) { return _i<Void>(0x9F7630, x, y, z, radius, flags); }
	static Void clear_area_of_peds(float x, float y, float z, float radius, int flags) { return _i<Void>(0x9F7650, x, y, z, radius, flags); }
	static Void clear_area_of_projectiles(float x, float y, float z, float radius, int flags) { return _i<Void>(0x9F7690, x, y, z, radius, flags); }
	static Void clear_area_of_vehicles(float x, float y, float z, float radius, bool p4, bool p5, bool p6, bool p7, bool p8, Any p9) { return _i<Void>(0x9F74D0, x, y, z, radius, p4, p5, p6, p7, p8, p9); }
	static Void clear_bit(int* address, int offset) { return _i<Void>(0x9F6DC0, address, offset); }
	static Void clear_brief() { return _i<Void>(0x9E0B20); }
	static Void clear_decision_maker_event_response(uint32_t name, int type) { return _i<Void>(0x9B93C0, name, type); }
	static Void clear_draw_origin() { return _i<Void>(0x9CD580); }
	static Void clear_driveby_task_underneath_driving_task(Ped ped) { return _i<Void>(0xAF4520, ped); }
	static Void clear_entity_last_damage_entity(Entity entity) { return _i<Void>(0x9B5100, entity); }
	static Void clear_entity_last_weapon_damage(Entity entity) { return _i<Void>(0xB393A0, entity); }
	static Void clear_facial_idle_anim_override(Ped ped) { return _i<Void>(0xA9A140, ped); }
	static Void clear_floating_help(Any p0, bool p1) { return _i<Void>(0x9E4630, p0, p1); }
	static Void clear_focus() { return _i<Void>(0xAD9DD0); }
	static Void clear_gps_flags() { return _i<Void>(0x9E39B0); }
	static Void clear_gps_player_waypoint() { return _i<Void>(0x9E3B70); }
	static Void clear_gps_race_track() { return _i<Void>(0x9E39F0); }
	static Void clear_hd_area() { return _i<Void>(0xADAEB0); }
	static Void clear_help(bool toggle) { return _i<Void>(0x9E1DD0, toggle); }
	static Void clear_override_weather() { return _i<Void>(0x9F5F80); }
	static Void clear_ped_alternate_movement_anim(Ped ped, int stance, float p2) { return _i<Void>(0xA95A50, ped, stance, p2); }
	static Void clear_ped_alternate_walk_anim(Ped ped, float p1) { return _i<Void>(0xA95980, ped, p1); }
	static Void clear_ped_blood_damage(Ped ped) { return _i<Void>(0xA990E0, ped); }
	static Void clear_ped_blood_damage_by_zone(Ped ped, int p1) { return _i<Void>(0xA99130, ped, p1); }
	static Void clear_ped_damage_decal_by_zone(Ped ped, int p1, const char* p2) { return _i<Void>(0xA991F0, ped, p1, p2); }
	static Void clear_ped_decorations(Ped ped) { return _i<Void>(0xA99790, ped); }
	static Void clear_ped_drive_by_clipset_override(Ped ped) { return _i<Void>(0xA95750, ped); }
	static Void clear_ped_in_pause_menu() { return _i<Void>(0x9E5A20); }
	static Void clear_ped_last_damage_bone(Ped ped) { return _i<Void>(0xA91AC0, ped); }
	static Void clear_ped_last_weapon_damage(Ped ped) { return _i<Void>(0xB392C0, ped); }
	static Void clear_ped_non_creation_area() { return _i<Void>(0xA91030); }
	static Void clear_ped_prop(Ped ped, int propId) { return _i<Void>(0xA97520, ped, propId); }
	static Void clear_ped_secondary_task(Ped ped) { return _i<Void>(0xAF3CA0, ped); }
	static Void clear_ped_tasks(Ped ped) { return _i<Void>(0xAF3C20, ped); }
	static Void clear_ped_tasks_immediately(Ped ped) { return _i<Void>(0xAF49D0, ped); }
	static Void clear_ped_wetness(Ped ped) { return _i<Void>(0xA993C0, ped); }
	static Void clear_player_has_damaged_at_least_one_non_animal_ped(Player player) { return _i<Void>(0xAAB6C0, player); }
	static Void clear_player_has_damaged_at_least_one_ped(Player player) { return _i<Void>(0xAAB640, player); }
	static Void clear_player_parachute_model_override(Player player) { return _i<Void>(0xAAD870, player); }
	static Void clear_player_parachute_pack_model_override(Player player) { return _i<Void>(0xAAD930, player); }
	static Void clear_player_parachute_variation_override(Player player) { return _i<Void>(0xAAD740, player); }
	static Void clear_player_wanted_level(Player player) { return _i<Void>(0xAA9B40, player); }
	static Void clear_popschedule_override_vehicle_model(int scheduleId) { return _i<Void>(0xB3C730, scheduleId); }
	static Void clear_prints() { return _i<Void>(0x9E0B10); }
	static Void clear_relationship_between_groups(int relationship, uint32_t group1, uint32_t group2) { return _i<Void>(0xA93910, relationship, group1, group2); }
	static Void clear_reminder_message() { return _i<Void>(0x9E42D0); }
	static Void clear_room_for_entity(Entity entity) { return _i<Void>(0x9E82F0, entity); }
	static Void clear_sequence_task(Object* taskSequence) { return _i<Void>(0xAF3830, taskSequence); }
	static Void clear_small_prints() { return _i<Void>(0x9E0CD0); }
	static Void clear_this_print(const char* p0) { return _i<Void>(0x9E0B50, p0); }
	static Void clear_timecycle_modifier() { return _i<Void>(0x9D08D0); }
	static Void clear_vehicle_custom_primary_colour(Vehicle vehicle) { return _i<Void>(0xB1CBF0, vehicle); }
	static Void clear_vehicle_custom_secondary_colour(Vehicle vehicle) { return _i<Void>(0xB1CDF0, vehicle); }
	static Void clear_weather_type_persist() { return _i<Void>(0x9F5DC0); }
	static Void clone_ped_to_target(Ped ped, Ped targetPed) { return _i<Void>(0xA8FCF0, ped, targetPed); }
	static Void close_bomb_bay_doors(Vehicle vehicle) { return _i<Void>(0xB26340, vehicle); }
	static Void close_sequence_task(Object taskSequence) { return _i<Void>(0xAF37C0, taskSequence); }
	static int compare_strings(const char* str1, const char* str2, bool matchCase, int maxLength) { return _i<int>(0x9F8750, str1, str2, matchCase, maxLength); }
	static Void control_landing_gear(Vehicle vehicle, int state) { return _i<Void>(0xB24570, vehicle, state); }
	static bool control_mounted_weapon(Ped ped) { return _i<bool>(0xAF4640, ped); }
	static Pickup create_ambient_pickup(uint32_t pickupHash, float posX, float posY, float posZ, int p4, int value, uint32_t modelHash, bool p7, bool p8) { return _i<Pickup>(0xA65A40, pickupHash, posX, posY, posZ, p4, value, modelHash, p7, p8); }
	static Cam create_cam(const char* camName, bool p1) { return _i<Cam>(0x990F80, camName, p1); }
	static Cam create_camera(uint32_t camHash, bool p1) { return _i<Cam>(0x991170, camHash, p1); }
	static int create_checkpoint(int type, float posX1, float posY1, float posZ1, float posX2, float posY2, float posZ2, float radius, int red, int green, int blue, int alpha, int reserved) { return _i<int>(0x9CC460, type, posX1, posY1, posZ1, posX2, posY2, posZ2, radius, red, green, blue, alpha, reserved); }
	static Void create_forced_object(float x, float y, float z, Any p3, uint32_t modelHash, bool p5) { return _i<Void>(0x9B71C0, x, y, z, p3, modelHash, p5); }
	static int create_group(int unused) { return _i<int>(0xA92DC0, unused); }
	static bool create_incident(int incidentType, float x, float y, float z, int p5, float radius, int* outIncidentID, Any p7) { return _i<bool>(0x9F9C50, incidentType, x, y, z, p5, radius, outIncidentID, p7); }
	static bool create_incident_with_entity(int incidentType, Ped ped, int amountOfPeople, float radius, int* outIncidentID, Any p5) { return _i<bool>(0x9F9C90, incidentType, ped, amountOfPeople, radius, outIncidentID, p5); }
	static Any create_itemset(math::vector3<float>* distri) { return _i<Any>(0x9E9630, distri); }
	static Vehicle create_mission_train(int variation, float x, float y, float z, bool direction) { return _i<Vehicle>(0xB1F3D0, variation, x, y, z, direction); }
	static Void create_mobile_phone(int phoneType) { return _i<Void>(0xAAF040, phoneType); }
	static Void create_model_hide(float x, float y, float z, float radius, uint32_t model, bool p5) { return _i<Void>(0x9B6F90, x, y, z, radius, model, p5); }
	static Void create_model_hide_excluding_script_objects(float x, float y, float z, float radius, uint32_t model, bool p5) { return _i<Void>(0x9B7060, x, y, z, radius, model, p5); }
	static Void create_model_swap(float x, float y, float z, float radius, uint32_t originalModel, uint32_t newModel, bool p6) { return _i<Void>(0x9B6E20, x, y, z, radius, originalModel, newModel, p6); }
	static Void create_money_pickups(float x, float y, float z, int value, int amount, uint32_t model) { return _i<Void>(0xA66100, x, y, z, value, amount, model); }
	static Void create_new_scripted_conversation() { return _i<Void>(0x9829E0); }
	static Void create_nm_message(bool startImmediately, int messageId) { return _i<Void>(0xA99910, startImmediately, messageId); }
	static Object create_object_no_offset(uint32_t modelHash, float x, float y, float z, bool isNetwork, bool p5, bool dynamic) { return _i<Object>(0xA63A80, modelHash, x, y, z, isNetwork, p5, dynamic); }
	static Void create_patrol_route() { return _i<Void>(0xAF6320); }
	static Ped create_ped(int pedType, uint32_t modelHash, float x, float y, float z, float heading, bool isNetwork, bool p7) { return _i<Ped>(0xA8FBF0, pedType, modelHash, x, y, z, heading, isNetwork, p7); }
	static Ped create_ped_inside_vehicle(Vehicle vehicle, int pedType, uint32_t modelHash, int seat, bool isNetwork, bool p5) { return _i<Ped>(0xA90320, vehicle, pedType, modelHash, seat, isNetwork, p5); }
	static Pickup create_pickup_rotate(uint32_t pickupHash, float posX, float posY, float posZ, float rotX, float rotY, float rotZ, int flag, int amount, Any p9, bool p10, uint32_t modelHash) { return _i<Pickup>(0xA659A0, pickupHash, posX, posY, posZ, rotX, rotY, rotZ, flag, amount, p9, p10, modelHash); }
	static Void create_pick_up_rope_for_cargobob(Vehicle cargobob, int state) { return _i<Void>(0xB29BC0, cargobob, state); }
	static Ped create_random_ped(float posX, float posY, float posZ) { return _i<Ped>(0xA90B50, posX, posY, posZ); }
	static int create_script_vehicle_generator(float x, float y, float z, float heading, float p4, float p5, uint32_t modelHash, int p7, int p8, int p9, int p10, bool p11, bool p12, bool p13, bool p14, bool p15, int p16) { return _i<int>(0xB1B450, x, y, z, heading, p4, p5, modelHash, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16); }
	static int create_synchronized_scene(float x, float y, float z, float roll, float pitch, float yaw, int p6) { return _i<int>(0xA9CAE0, x, y, z, roll, pitch, yaw, p6); }
	static Object create_tracked_point() { return _i<Object>(0x9CDF00); }
	static Vehicle create_vehicle(uint32_t modelHash, float x, float y, float z, float heading, bool isNetwork, bool p6, Any p7) { return _i<Vehicle>(0xB1AFB0, modelHash, x, y, z, heading, isNetwork, p6, p7); }
	static Object create_weapon_object(uint32_t weaponHash, int ammoCount, float x, float y, float z, bool showWorldModel, float heading, Any p7, Any p8, Any p9) { return _i<Object>(0xB3A360, weaponHash, ammoCount, x, y, z, showWorldModel, heading, p7, p8, p9); }
	static Void datafile_create() { return _i<Void>(0x99E0D0); }
	static Void datafile_delete() { return _i<Void>(0x99E0E0); }
	static const char* datafile_get_file_dict() { return _i<const char*>(0x99E2C0); }
	static bool datafile_is_save_pending() { return _i<bool>(0x99E3D0); }
	static bool decor_exist_on(Entity entity, const char* propertyName) { return _i<bool>(0x9A0540, entity, propertyName); }
	static bool decor_get_bool(Entity entity, const char* propertyName) { return _i<bool>(0x9A02F0, entity, propertyName); }
	// [aligned] static float decor_get_float(Entity entity, const char* propertyName) { return _i<float>(0x9A03B0, entity, propertyName); }
	static int decor_get_int(Entity entity, const char* propertyName) { return _i<int>(0x9A0480, entity, propertyName); }
	static bool decor_is_registered_as_type(const char* propertyName, int type) { return _i<bool>(0x9A06C0, propertyName, type); }
	static Void decor_register(const char* propertyName, int type) { return _i<Void>(0x9A0640, propertyName, type); }
	static Void decor_register_lock() { return _i<Void>(0x9A0730); }
	static bool decor_remove(Entity entity, const char* propertyName) { return _i<bool>(0x9A05C0, entity, propertyName); }
	static bool decor_set_bool(Entity entity, const char* propertyName, bool value) { return _i<bool>(0x9A0060, entity, propertyName, value); }
	// [aligned] static bool decor_set_float(Entity entity, const char* propertyName, float value) { return _i<bool>(0x9A0140, entity, propertyName, value); }
	static bool decor_set_int(Entity entity, const char* propertyName, int value) { return _i<bool>(0x9A0220, entity, propertyName, value); }
	static bool decor_set_time(Entity entity, const char* propertyName, int timestamp) { return _i<bool>(0x9A0030, entity, propertyName, timestamp); }
	static Void delete_checkpoint(int checkpoint) { return _i<Void>(0x9CC8F0, checkpoint); }
	static Void delete_child_rope(Object rope) { return _i<Void>(0xAA07F0, rope); }
	static Void delete_entity(Entity* entity) { return _i<Void>(0x9B5160, entity); }
	static Void delete_incident(int test) { return _i<Void>(0x9F9CD0, test); }
	static Void delete_mission_train(Vehicle* train) { return _i<Void>(0xB205C0, train); }
	static Void delete_object(Object* object) { return _i<Void>(0xA63B20, object); }
	static Void delete_patrol_route(const char* patrolRoute) { return _i<Void>(0xAF6330, patrolRoute); }
	static Void delete_ped(Ped* ped) { return _i<Void>(0xA8FC40, ped); }
	static Void delete_script_vehicle_generator(int vehicleGenerator) { return _i<Void>(0xB1B620, vehicleGenerator); }
	static Void delete_stunt_jump(int p0) { return _i<Void>(0x9F9560, p0); }
	static Void delete_vehicle(Vehicle* vehicle) { return _i<Void>(0xB1B000, vehicle); }
	static Void destroy_all_cams(bool thisScriptCheck) { return _i<Void>(0x9913C0, thisScriptCheck); }
	static Void destroy_cam(Cam cam, bool thisScriptCheck) { return _i<Void>(0x991360, cam, thisScriptCheck); }
	static Void destroy_itemset(Any p0) { return _i<Void>(0x9E96B0, p0); }
	static Void destroy_mobile_phone() { return _i<Void>(0xAAF050); }
	static Void destroy_tracked_point(Object point) { return _i<Void>(0x9CE000, point); }
	static Void detach_cam(Cam cam) { return _i<Void>(0x992900, cam); }
	static Void detach_entity(Entity entity, bool p1, bool collision) { return _i<Void>(0x9B5170, entity, p1, collision); }
	static Void detach_portable_pickup_from_ped(Ped ped) { return _i<Void>(0xA65BD0, ped); }
	static Void detach_rope_from_entity(Object rope, Entity entity) { return _i<Void>(0xAA0B90, rope, entity); }
	static Void detach_synchronized_scene(int sceneID) { return _i<Void>(0xA9CEE0, sceneID); }
	static bool detach_vehicle_from_any_cargobob(Vehicle vehicle) { return _i<bool>(0xB29830, vehicle); }
	static bool detach_vehicle_from_any_tow_truck(Vehicle vehicle) { return _i<bool>(0xB24C20, vehicle); }
	static Void detach_vehicle_from_cargobob(Vehicle vehicle, Vehicle cargobob) { return _i<Void>(0xB29820, vehicle, cargobob); }
	static Void detach_vehicle_from_tow_truck(Vehicle towTruck, Vehicle vehicle) { return _i<Void>(0xB24C10, towTruck, vehicle); }
	static Void detach_vehicle_from_trailer(Vehicle vehicle) { return _i<Void>(0xB1EF00, vehicle); }
	static Void disable_aim_cam_this_update() { return _i<Void>(0x995190); }
	static Void disable_all_control_actions(int inputGroup) { return _i<Void>(0xA6B340, inputGroup); }
	static Void disable_control_action(int inputGroup, int control, bool disable) { return _i<Void>(0xA6B220, inputGroup, control, disable); }
	static Void disable_frontend_this_frame() { return _i<Void>(0x9E5240); }
	static Void disable_hospital_restart(int hospitalIndex, bool toggle) { return _i<Void>(0x9F79D0, hospitalIndex, toggle); }
	static Void disable_interior(int interiorID, bool toggle) { return _i<Void>(0x9E9020, interiorID, toggle); }
	static Void disable_navmesh_in_area(Any p0, Any p1, Any p2, Any p3, Any p4, Any p5, Any p6) { return _i<Void>(0xA70C70, p0, p1, p2, p3, p4, p5, p6); }
	static Void disable_ped_pain_audio(Ped ped, bool toggle) { return _i<Void>(0x985A70, ped, toggle); }
	static Void disable_plane_aileron(Vehicle vehicle, bool p1, bool p2) { return _i<Void>(0xB28490, vehicle, p1, p2); }
	static Void disable_player_firing(Player player, bool toggle) { return _i<Void>(0xAABA20, player, toggle); }
	static Void disable_player_vehicle_rewards(Player player) { return _i<Void>(0xAAD970, player); }
	static Void disable_police_restart(int policeIndex, bool toggle) { return _i<Void>(0x9F7B50, policeIndex, toggle); }
	static Void disable_script_brain_set(int brainSet) { return _i<Void>(0x9881E0, brainSet); }
	static Void disable_stunt_jump_set(int p0) { return _i<Void>(0x9F95A0, p0); }
	static Void disable_vehicle_distantlights(bool toggle) { return _i<Void>(0x9D0660, toggle); }
	static Void disable_vehicle_weapon(bool disabled, uint32_t weaponHash, Vehicle vehicle, Ped owner) { return _i<Void>(0xB2A380, disabled, weaponHash, vehicle, owner); }
	static Void display_ammo_this_frame(bool display) { return _i<Void>(0x9E36B0, display); }
	static Void display_area_name(bool toggle) { return _i<Void>(0x9E3640, toggle); }
	static Void display_cash(bool toggle) { return _i<Void>(0x9E3660, toggle); }
	static Void display_help_text_this_frame(const char* message, bool p1) { return _i<Void>(0x9E37F0, message, p1); }
	static Void display_hud(bool toggle) { return _i<Void>(0x9E10A0, toggle); }
	static Void display_onscreen_keyboard(int p0, const char* windowTitle, const char* p2, const char* defaultText, const char* defaultConcat1, const char* defaultConcat2, const char* defaultConcat3, int maxInputLength) { return _i<Void>(0x9FA910, p0, windowTitle, p2, defaultText, defaultConcat1, defaultConcat2, defaultConcat3, maxInputLength); }
	static Void display_radar(bool Toggle) { return _i<Void>(0x9E1110, Toggle); }
	static Void display_sniper_scope_this_frame() { return _i<Void>(0x9E36D0); }
	static Void display_system_signin_ui(bool unk) { return _i<Void>(0xAAB2F0, unk); }
	static bool does_anim_dict_exist(const char* animDict) { return _i<bool>(0xAD9770, animDict); }
	static bool does_blip_exist(Blip blip) { return _i<bool>(0x9E30F0, blip); }
	static bool does_cam_exist(Cam cam) { return _i<bool>(0x991400, cam); }
	static bool does_cargobob_have_pickup_magnet(Vehicle cargobob) { return _i<bool>(0xB29CE0, cargobob); }
	static bool does_cargobob_have_pick_up_rope(Vehicle cargobob) { return _i<bool>(0xB29B10, cargobob); }
	static bool does_cutscene_entity_exist(const char* cutsceneEntName, uint32_t modelHash) { return _i<bool>(0x999D00, cutsceneEntName, modelHash); }
	static bool does_entity_belong_to_this_script(Entity entity, bool p1) { return _i<bool>(0x9B22F0, entity, p1); }
	static bool does_entity_exist(Entity entity) { return _i<bool>(0x9B2290, entity); }
	static bool does_entity_have_drawable(Entity entity) { return _i<bool>(0x9B2370, entity); }
	static bool does_entity_have_physics(Entity entity) { return _i<bool>(0x9B23D0, entity); }
	static bool does_extra_exist(Vehicle vehicle, int extraId) { return _i<bool>(0xB22BF0, vehicle, extraId); }
	static bool does_group_exist(int groupId) { return _i<bool>(0xA94080, groupId); }
	static bool does_navmesh_blocking_object_exist(Any p0) { return _i<bool>(0xA70EC0, p0); }
	static bool does_particle_fx_looped_exist(int ptfxHandle) { return _i<bool>(0x9CF520, ptfxHandle); }
	static bool does_ped_have_ai_blip(Ped ped) { return _i<bool>(0x9E5D70, ped); }
	static bool does_pickup_exist(Pickup pickup) { return _i<bool>(0xA66120, pickup); }
	static bool does_pickup_object_exist(Object pickupObject) { return _i<bool>(0xA66170, pickupObject); }
	static bool does_rope_exist(Object* rope) { return _i<bool>(0xAA0810, rope); }
	static bool does_scenario_exist_in_area(float x, float y, float z, float radius, bool b) { return _i<bool>(0xAF56D0, x, y, z, radius, b); }
	static bool does_scenario_group_exist(const char* scenarioGroup) { return _i<bool>(0xAF58E0, scenarioGroup); }
	static bool does_scenario_of_type_exist_in_area(float p0, float p1, float p2, Any* p3, float p4, bool p5) { return _i<bool>(0xAF5770, p0, p1, p2, p3, p4, p5); }
	static bool does_scripted_cover_point_exist_at_coords(float x, float y, float z) { return _i<bool>(0xAF4C90, x, y, z); }
	static bool does_script_exist(const char* scriptName) { return _i<bool>(0xAB0F70, scriptName); }
	static bool does_script_vehicle_generator_exist(int vehicleGenerator) { return _i<bool>(0xB1B3F0, vehicleGenerator); }
	// [aligned] static bool does_script_with_name_hash_exist(uint32_t scriptHash) { return _i<bool>(0xAB11F0, scriptHash); }
	static bool does_text_block_exist(const char* gxt) { return _i<bool>(0x9E0CE0, gxt); }
	static bool does_text_label_exist(const char* gxt) { return _i<bool>(0x9E0E30, gxt); }
	static bool does_vehicle_exist_with_decorator(const char* decorator) { return _i<bool>(0xB2ACE0, decorator); }
	static bool does_vehicle_have_roof(Vehicle vehicle) { return _i<bool>(0xB21720, vehicle); }
	static bool does_vehicle_have_stuck_vehicle_check(Vehicle vehicle) { return _i<bool>(0xB1F620, vehicle); }
	static bool does_vehicle_have_weapons(Vehicle vehicle) { return _i<bool>(0xB2A2A0, vehicle); }
	static bool does_weapon_take_weapon_component(uint32_t weaponHash, uint32_t componentHash) { return _i<bool>(0xB3A530, weaponHash, componentHash); }
	static Void do_auto_save() { return _i<Void>(0x9F7FB0); }
	static Void do_screen_fade_in(int duration) { return _i<Void>(0x994070, duration); }
	static Void do_screen_fade_out(int duration) { return _i<Void>(0x994080, duration); }
	static Void draw_box(float x1, float y1, float z1, float x2, float y2, float z2, int red, int green, int blue, int alpha) { return _i<Void>(0x9CBA80, x1, y1, z1, x2, y2, z2, red, green, blue, alpha); }
	static Void draw_debug_box(float x1, float y1, float z1, float x2, float y2, float z2, int red, int green, int blue, int alpha) { return _i<Void>(0x9CB790, x1, y1, z1, x2, y2, z2, red, green, blue, alpha); }
	static Void draw_debug_cross(float x, float y, float z, float size, int red, int green, int blue, int alpha) { return _i<Void>(0x9CB7A0, x, y, z, size, red, green, blue, alpha); }
	static Void draw_debug_line_with_two_colours(float x1, float y1, float z1, float x2, float y2, float z2, int r1, int g1, int b1, int r2, int g2, int b2, int alpha1, int alpha2) { return _i<Void>(0x9CB770, x1, y1, z1, x2, y2, z2, r1, g1, b1, r2, g2, b2, alpha1, alpha2); }
	static Void draw_debug_sphere(float x, float y, float z, float radius, int red, int green, int blue, int alpha) { return _i<Void>(0x9CB780, x, y, z, radius, red, green, blue, alpha); }
	static Void draw_debug_text(const char* text, float x, float y, float z, int red, int green, int blue, int alpha) { return _i<Void>(0x9CB7B0, text, x, y, z, red, green, blue, alpha); }
	static Void draw_light_with_range(float posX, float posY, float posZ, int colorR, int colorG, int colorB, float range, float intensity) { return _i<Void>(0x9CBF70, posX, posY, posZ, colorR, colorG, colorB, range, intensity); }
	static Void draw_line(float x1, float y1, float z1, float x2, float y2, float z2, int red, int green, int blue, int alpha) { return _i<Void>(0x9CB7D0, x1, y1, z1, x2, y2, z2, red, green, blue, alpha); }
	static Void draw_marker(int type, float posX, float posY, float posZ, float dirX, float dirY, float dirZ, float rotX, float rotY, float rotZ, float scaleX, float scaleY, float scaleZ, int red, int green, int blue, int alpha, bool bobUpAndDown, bool faceCamera, int p19, bool rotate, const char* textureDict, const char* textureName, bool drawOnEnts) { return _i<Void>(0x9CC120, type, posX, posY, posZ, dirX, dirY, dirZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ, red, green, blue, alpha, bobUpAndDown, faceCamera, p19, rotate, textureDict, textureName, drawOnEnts); }
	static Void draw_poly(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, int red, int green, int blue, int alpha) { return _i<Void>(0x9CB8A0, x1, y1, z1, x2, y2, z2, x3, y3, z3, red, green, blue, alpha); }
	static Void draw_rect(float x, float y, float width, float height, int r, int g, int b, int a, Any p8) { return _i<Void>(0x9CCAC0, x, y, width, height, r, g, b, a, p8); }
	static Void draw_scaleform_movie(int scaleformHandle, float x, float y, float width, float height, int red, int green, int blue, int alpha, int unk) { return _i<Void>(0x9D11A0, scaleformHandle, x, y, width, height, red, green, blue, alpha, unk); }
	static Void draw_spot_light(float posX, float posY, float posZ, float dirX, float dirY, float dirZ, int colorR, int colorG, int colorB, float distance, float brightness, float roundness, float radius, float falloff) { return _i<Void>(0x9CBFA0, posX, posY, posZ, dirX, dirY, dirZ, colorR, colorG, colorB, distance, brightness, roundness, radius, falloff); }
	static Void draw_sprite(const char* textureDict, const char* textureName, float screenX, float screenY, float width, float height, float heading, int red, int green, int blue, int alpha, Any p11) { return _i<Void>(0x9CCCD0, textureDict, textureName, screenX, screenY, width, height, heading, red, green, blue, alpha, p11); }
	static Void draw_tv_channel(float xPos, float yPos, float xScale, float yScale, float rotation, int red, int green, int blue, int alpha) { return _i<Void>(0x9D1C00, xPos, yPos, xScale, yScale, rotation, red, green, blue, alpha); }
	static Void enable_alien_blood_vfx(bool Toggle) { return _i<Void>(0x9CF890, Toggle); }
	static Void enable_all_control_actions(int inputGroup) { return _i<Void>(0xA6B390, inputGroup); }
	static Void enable_clown_blood_vfx(bool toggle) { return _i<Void>(0x9CF850, toggle); }
	static Void enable_control_action(int inputGroup, int control, bool enable) { return _i<Void>(0xA6B2B0, inputGroup, control, enable); }
	static Void enable_dispatch_service(int dispatchService, bool toggle) { return _i<Void>(0x9F9BA0, dispatchService, toggle); }
	static Void enable_laser_sight_rendering(bool toggle) { return _i<Void>(0xB38580, toggle); }
	static Void enable_movie_subtitles(bool toggle) { return _i<Void>(0x9D1E90, toggle); }
	static Void enable_script_brain_set(int brainSet) { return _i<Void>(0x9881C0, brainSet); }
	static Void enable_special_ability(Player player, bool toggle) { return _i<Void>(0xAAC2B0, player, toggle); }
	static Void enable_stunt_jump_set(int p0) { return _i<Void>(0x9F9580, p0); }
	static Void enable_tennis_mode(Ped ped, bool toggle, bool p2) { return _i<Void>(0x9FA120, ped, toggle, p2); }
	static Void end_replay_stats() { return _i<Void>(0x9F80E0); }
	static Void end_srl() { return _i<Void>(0xADAD80); }
	static Void end_text_command_clear_print() { return _i<Void>(0x9E0870); }
	static Void end_text_command_display_help(Any p0, bool loop, bool beep, int shape) { return _i<Void>(0x9E07A0, p0, loop, beep, shape); }
	static Void end_text_command_display_text(float x, float y, Any p2) { return _i<Void>(0x9E06C0, x, y, p2); }
	static bool end_text_command_is_message_displayed() { return _i<bool>(0x9E0670); }
	static bool end_text_command_is_this_help_message_being_displayed(int p0) { return _i<bool>(0x9E07E0, p0); }
	static Void end_text_command_print(int duration, bool drawImmediately) { return _i<Void>(0x9E0640, duration, drawImmediately); }
	static Void end_text_command_scaleform_string() { return _i<Void>(0x9D19F0); }
	static Void end_text_command_set_blip_name(Blip blip) { return _i<Void>(0x9E0820, blip); }
	static int end_text_command_thefeed_post_crewtag_with_game_name(bool p0, bool p1, int64_t* p2, int p3, bool isLeader, bool unk0, int clanDesc, const char* playerName, int R, int G, int B) { return _i<int>(0x9E0420, p0, p1, p2, p3, isLeader, unk0, clanDesc, playerName, R, G, B); }
	static Void explode_ped_head(Ped ped, uint32_t weaponHash) { return _i<Void>(0xA90830, ped, weaponHash); }
	static Void explode_projectiles(Ped ped, uint32_t weaponHash, bool p2) { return _i<Void>(0xB39DF0, ped, weaponHash, p2); }
	static Void explode_vehicle(Vehicle vehicle, bool isAudible, bool isInvisible) { return _i<Void>(0xB1C3F0, vehicle, isAudible, isInvisible); }
	static Void explode_vehicle_in_cutscene(Vehicle vehicle, bool p1) { return _i<Void>(0xB20150, vehicle, p1); }
	static Void fade_decals_in_range(Any p0, Any p1, Any p2, Any p3, Any p4) { return _i<Void>(0x9CFD30, p0, p1, p2, p3, p4); }
	static Void fade_out_local_player(bool p0) { return _i<Void>(0xA53010, p0); }
	static bool fillout_pm_player_list(int* network_handle, Any p1, Any p2) { return _i<bool>(0xA4D810, network_handle, p1, p2); }
	static int find_radio_station_index(int station) { return _i<int>(0x984DD0, station); }
	static bool find_spawn_point_in_direction(float x1, float y1, float z1, float x2, float y2, float z2, float distance, math::vector3<float>* spawnPoint) { return _i<bool>(0x9F9E20, x1, y1, z1, x2, y2, z2, distance, spawnPoint); }
	static Void fix_vehicle_window(Vehicle vehicle, int index) { return _i<Void>(0xB1E7D0, vehicle, index); }
	static Void flash_ability_bar(bool toggle) { return _i<Void>(0x9E1750, toggle); }
	static Void flash_minimap_display() { return _i<Void>(0x9E3BC0); }
	static Void flash_wanted_display(bool p0) { return _i<Void>(0x9E1780, p0); }
	static Void force_cleanup(int cleanupFlags) { return _i<Void>(0xAAB0C0, cleanupFlags); }
	static Void force_cleanup_for_all_threads_with_this_name(const char* name, int cleanupFlags) { return _i<Void>(0xAAB100, name, cleanupFlags); }
	static Void force_cleanup_for_thread_with_this_id(int id, int cleanupFlags) { return _i<Void>(0xAAB130, id, cleanupFlags); }
	static Void force_entity_ai_and_animation_update(Entity entity) { return _i<Void>(0x9B2A20, entity); }
	static bool force_ped_motion_state(Ped ped, uint32_t motionStateHash, bool p2, bool p3, bool p4) { return _i<bool>(0xA9CF50, ped, motionStateHash, p2, p3, p4); }
	static Void force_ped_to_open_parachute(Ped ped) { return _i<Void>(0xA91E70, ped); }
	static Void force_room_for_entity(Entity entity, int interiorID, uint32_t roomHashKey) { return _i<Void>(0x9E8380, entity, interiorID, roomHashKey); }
	static Void freeze_entity_position(Entity entity, bool toggle) { return _i<Void>(0x9B5190, entity, toggle); }
	static Void freeze_radio_station(const char* radioStation) { return _i<Void>(0x984900, radioStation); }
	static int generate_directions_to_coord(float x, float y, float z, bool p3, float* direction, float* p5, float* distToNxJunction) { return _i<int>(0xA70640, x, y, z, p3, direction, p5, distToNxJunction); }
	static int get_active_vehicle_mission_type(Vehicle veh) { return _i<int>(0xAF3A70, veh); }
	static int get_allocated_stack_size() { return _i<int>(0x9F5740); }
	static int get_all_vehicles(Any* vehArray) { return _i<int>(0xB2CE30, vehArray); }
	static bool get_ammo_in_clip(Ped ped, uint32_t weaponHash, int* ammo) { return _i<bool>(0xB396D0, ped, weaponHash, ammo); }
	static int get_ammo_in_ped_weapon(Ped ped, uint32_t weaponhash) { return _i<int>(0xB38E00, ped, weaponhash); }
	// [aligned] static float get_anim_duration(const char* animDict, const char* animName) { return _i<float>(0x9B2AF0, animDict, animName); }
	static Any get_are_bomb_bay_doors_open(Any p0) { return _i<Any>(0xB263C0, p0); }
	// [aligned] static float get_aspect_ratio(bool b) { return _i<float>(0x9CDB80, b); }
	static int get_audible_music_track_text_id() { return _i<int>(0x984890); }
	static uint32_t get_best_ped_weapon(Ped ped, bool p1) { return _i<uint32_t>(0xB38AB0, ped, p1); }
	static int get_bits_in_range(int var, int rangeStart, int rangeEnd) { return _i<int>(0x9F9330, var, rangeStart, rangeEnd); }
	static int get_blip_alpha(Blip blip) { return _i<int>(0x9E26C0, blip); }
	static int get_blip_colour(Blip blip) { return _i<int>(0x9E28C0, blip); }
	static Blip get_blip_from_entity(Entity entity) { return _i<Blip>(0x9E2180, entity); }
	static int get_blip_hud_colour(Blip blip) { return _i<int>(0x9E2900, blip); }
	static int get_blip_info_id_display(Blip blip) { return _i<int>(0x9E2080, blip); }
	static Entity get_blip_info_id_entity_index(Blip blip) { return _i<Entity>(0x9E2100, blip); }
	static Pickup get_blip_info_id_pickup_index(Blip blip) { return _i<Pickup>(0x9E2170, blip); }
	static int get_blip_info_id_type(Blip blip) { return _i<int>(0x9E20C0, blip); }
	static int get_blip_sprite(Blip blip) { return _i<int>(0x9E2580, blip); }
	static float get_cam_anim_current_phase(Cam cam) { return _i<float>(0x993BB0, cam); }
	static float get_cam_far_clip(Cam cam) { return _i<float>(0x991860, cam); }
	static float get_cam_far_dof(Cam cam) { return _i<float>(0x991960, cam); }
	static float get_cam_fov(Cam cam) { return _i<float>(0x991760, cam); }
	static float get_cam_near_clip(Cam cam) { return _i<float>(0x9917E0, cam); }
	static bool get_cam_spline_node_index(Cam cam) { return _i<bool>(0x993190, cam); }
	static float get_cam_spline_node_phase(Cam cam) { return _i<float>(0x992FF0, cam); }
	static float get_cam_spline_phase(Cam cam) { return _i<float>(0x992F50, cam); }
	static int get_cause_of_most_recent_force_cleanup() { return _i<int>(0xAAB170); }
	static const char* get_clip_set_for_scripted_gun_task(int p0) { return _i<const char*>(0xAF3B30, p0); }
	static int get_clock_day_of_month() { return _i<int>(0x996AA0); }
	static int get_clock_day_of_week() { return _i<int>(0x996A90); }
	static int get_clock_hours() { return _i<int>(0x996A40); }
	static int get_clock_minutes() { return _i<int>(0x996A50); }
	static int get_clock_month() { return _i<int>(0x996AB0); }
	static int get_clock_seconds() { return _i<int>(0x996A60); }
	static int get_clock_year() { return _i<int>(0x996AC0); }
	static bool get_closest_fire_pos(math::vector3<float>* outPosition, float x, float y, float z) { return _i<bool>(0x9BB310, outPosition, x, y, z); }
	static bool get_closest_major_vehicle_node(float x, float y, float z, math::vector3<float>* outPosition, float unknown1, int unknown2) { return _i<bool>(0xA6F7D0, x, y, z, outPosition, unknown1, unknown2); }
	static Object get_closest_object_of_type(float x, float y, float z, float radius, uint32_t modelHash, bool isMission, bool p6, bool p7) { return _i<Object>(0xA63ED0, x, y, z, radius, modelHash, isMission, p6, p7); }
	static bool get_closest_ped(float x, float y, float z, float radius, bool p4, bool p5, Ped* outPed, bool p7, bool p8, int pedType) { return _i<bool>(0xA97A50, x, y, z, radius, p4, p5, outPed, p7, p8, pedType); }
	static Any get_closest_road(float x, float y, float z, float p3, int p4, math::vector3<float>* p5, math::vector3<float>* p6, Any* p7, Any* p8, float* p9, bool p10) { return _i<Any>(0xA6FFA0, x, y, z, p3, p4, p5, p6, p7, p8, p9, p10); }
	static Vehicle get_closest_vehicle(float x, float y, float z, float radius, uint32_t modelHash, int flags) { return _i<Vehicle>(0xB20420, x, y, z, radius, modelHash, flags); }
	static bool get_closest_vehicle_node(float x, float y, float z, math::vector3<float>* outPosition, int nodeType, float p5, float p6) { return _i<bool>(0xA6F6F0, x, y, z, outPosition, nodeType, p5, p6); }
	static bool get_closest_vehicle_node_with_heading(float x, float y, float z, math::vector3<float>* outPosition, float* outHeading, int nodeType, float p6, int p7) { return _i<bool>(0xA6F8A0, x, y, z, outPosition, outHeading, nodeType, p6, p7); }
	static float get_combat_float(Ped ped, int p1) { return _i<float>(0xA93F70, ped, p1); }
	static float get_control_normal(int inputGroup, int control) { return _i<float>(0xA6A720, inputGroup, control); }
	static int get_control_value(int inputGroup, int control) { return _i<int>(0xA6A6A0, inputGroup, control); }
	static int get_convertible_roof_state(Vehicle vehicle) { return _i<int>(0xB22E40, vehicle); }
	static bool get_current_ped_vehicle_weapon(Ped ped, uint32_t* weaponHash) { return _i<bool>(0xB38B50, ped, weaponHash); }
	static bool get_current_ped_weapon(Ped ped, uint32_t* weaponHash, bool p2) { return _i<bool>(0xB389A0, ped, weaponHash, p2); }
	static Entity get_current_ped_weapon_entity_index(Ped ped) { return _i<Entity>(0xB38A40, ped); }
	static Any get_current_playback_for_vehicle(Any p0) { return _i<Any>(0xB1FD90, p0); }
	static Any get_current_scripted_conversation_line() { return _i<Any>(0x982F90); }
	static int get_current_website_id() { return _i<int>(0x9E4BB0); }
	static int get_cutscene_section_playing() { return _i<int>(0x999660); }
	static int get_cutscene_time() { return _i<int>(0x999490); }
	static int get_cutscene_total_duration() { return _i<int>(0x9994F0); }
	static float get_decal_wash_level(int decal) { return _i<float>(0x9D0160, decal); }
	static int get_default_script_rendertarget_render_id() { return _i<int>(0x9E1AD0); }
	static float get_disabled_control_normal(int inputGroup, int control) { return _i<float>(0xA6AB00, inputGroup, control); }
	static const char* get_display_name_from_vehicle_model(uint32_t modelHash) { return _i<const char*>(0xB239B0, modelHash); }
	static float get_distance_between_coords(float x1, float y1, float z1, float x2, float y2, float z2, bool useZ) { return _i<float>(0x9F6870, x1, y1, z1, x2, y2, z2, useZ); }
	static bool get_dlc_vehicle_data(int dlcVehicleIndex, int* outData) { return _i<bool>(0x9A2C60, dlcVehicleIndex, outData); }
	static int get_dlc_vehicle_flags(int dlcVehicleIndex) { return _i<int>(0x9A2CB0, dlcVehicleIndex); }
	static uint32_t get_dlc_vehicle_model(int dlcVehicleIndex) { return _i<uint32_t>(0x9A2C30, dlcVehicleIndex); }
	static bool get_dlc_weapon_component_data(int dlcWeaponIndex, int dlcWeapCompIndex, int* ComponentDataPtr) { return _i<bool>(0x9A2E20, dlcWeaponIndex, dlcWeapCompIndex, ComponentDataPtr); }
	static bool get_dlc_weapon_data(int dlcWeaponIndex, int* outData) { return _i<bool>(0x9A2D40, dlcWeaponIndex, outData); }
	static int get_entity_alpha(Entity entity) { return _i<int>(0x9B6B10, entity); }
	static float get_entity_anim_current_time(Entity entity, const char* animDict, const char* animName) { return _i<float>(0x9B2A90, entity, animDict, animName); }
	static float get_entity_anim_total_time(Entity entity, const char* animDict, const char* animName) { return _i<float>(0x9B2AC0, entity, animDict, animName); }
	static Entity get_entity_attached_to(Entity entity) { return _i<Entity>(0x9B2B30, entity); }
	static Entity get_entity_attached_to_tow_truck(Vehicle towTruck) { return _i<Entity>(0xB24C80, towTruck); }
	static int get_entity_bone_index_by_name(Entity entity, const char* boneName) { return _i<int>(0x9B5030, entity, boneName); }
	static bool get_entity_collision_disabled(Entity entity) { return _i<bool>(0x9B5B10, entity); }
	static float get_entity_forward_x(Entity entity) { return _i<float>(0x9B2D10, entity); }
	static float get_entity_forward_y(Entity entity) { return _i<float>(0x9B2D80, entity); }
	static float get_entity_heading(Entity entity) { return _i<float>(0x9B2DF0, entity); }
	static int get_entity_health(Entity entity) { return _i<int>(0x9B3010, entity); }
	static float get_entity_height(Entity entity, float X, float Y, float Z, bool atTop, bool inWorldCoords) { return _i<float>(0x9B3120, entity, X, Y, Z, atTop, inWorldCoords); }
	static float get_entity_height_above_ground(Entity entity) { return _i<float>(0x9B3160, entity); }
	static Entity get_entity_index_of_cutscene_entity(const char* cutsceneEntName, uint32_t modelHash) { return _i<Entity>(0x999680, cutsceneEntName, modelHash); }
	static Entity get_entity_index_of_registered_entity(const char* cutsceneEntName, uint32_t modelHash) { return _i<Entity>(0x999830, cutsceneEntName, modelHash); }
	static int get_entity_lod_dist(Entity entity) { return _i<int>(0x9B6A50, entity); }
	static int get_entity_max_health(Entity entity) { return _i<int>(0x9B3060, entity); }
	static uint32_t get_entity_model(Entity entity) { return _i<uint32_t>(0x9B3330, entity); }
	static float get_entity_pitch(Entity entity) { return _i<float>(0x9B3530, entity); }
	static bool get_entity_player_is_free_aiming_at(Player player, Entity* entity) { return _i<bool>(0xAAA730, player, entity); }
	static int get_entity_population_type(Entity entity) { return _i<int>(0x9B3DD0, entity); }
	static Void get_entity_quaternion(Entity entity, float* x, float* y, float* z, float* w) { return _i<Void>(0x9B3560, entity, x, y, z, w); }
	static float get_entity_roll(Entity entity) { return _i<float>(0x9B36A0, entity); }
	static const char* get_entity_script(Entity entity, ScrHandle* script) { return _i<const char*>(0x9B3920, entity, script); }
	static float get_entity_speed(Entity entity) { return _i<float>(0x9B3980, entity); }
	static float get_entity_submerged_level(Entity entity) { return _i<float>(0x9B4630, entity); }
	static int get_entity_type(Entity entity) { return _i<int>(0x9B3DD0, entity); }
	static float get_entity_upright_value(Entity entity) { return _i<float>(0x9B3B70, entity); }
	static int get_event_at_index(bool p0, int eventNum) { return _i<int>(0xAB14B0, p0, eventNum); }
	static bool get_event_data(bool p0, int eventNum, int* argStruct, int argStructSize) { return _i<bool>(0xAB14F0, p0, eventNum, argStruct, argStructSize); }
	static bool get_event_exists(bool p0, int p1) { return _i<bool>(0xAB1480, p0, p1); }
	static int get_fake_wanted_level() { return _i<int>(0x9F85A0); }
	static Blip get_first_blip_info_id(int blipSprite) { return _i<Blip>(0x9E1F80, blipSprite); }
	static int get_follow_ped_cam_view_mode() { return _i<int>(0x994E30); }
	static int get_follow_vehicle_cam_view_mode() { return _i<int>(0x994FA0); }
	static int get_frame_count() { return _i<int>(0x9F6430); }
	static float get_frame_time() { return _i<float>(0x9F63F0); }
	static float get_gameplay_cam_fov() { return _i<float>(0x9941B0); }
	static float get_gameplay_cam_relative_heading() { return _i<float>(0x994270); }
	static float get_gameplay_cam_relative_pitch() { return _i<float>(0x9942B0); }
	static int get_game_timer() { return _i<int>(0x9F63E0); }
	static Void get_group_size(int groupID, Any* unknown, int* sizeInMembers) { return _i<Void>(0xA93FF0, groupID, unknown, sizeInMembers); }
	static uint32_t get_hash_key(const char* model) { return _i<uint32_t>(0x9F6DE0, model); }
	static uint32_t get_hash_of_map_area_at_coords(float x, float y, float z) { return _i<uint32_t>(0xB3C780, x, y, z); }
	static uint32_t get_hash_of_this_script_name() { return _i<uint32_t>(0xAB1410); }
	static float get_heading_from_vector_2d(float dx, float dy) { return _i<float>(0x9F6990, dx, dy); }
	// [aligned] static float get_heli_main_rotor_health(Vehicle vehicle) { return _i<float>(0xB24140, vehicle); }
	// [aligned] static float get_heli_tail_rotor_health(Vehicle vehicle) { return _i<float>(0xB241C0, vehicle); }
	static Void get_hud_colour(int hudColorIndex, int* r, int* g, int* b, int* a) { return _i<Void>(0x9E15F0, hudColorIndex, r, g, b, a); }
	static int get_ideal_player_switch_type(float x1, float y1, float z1, float x2, float y2, float z2) { return _i<int>(0xADA600, x1, y1, z1, x2, y2, z2); }
	static int get_id_of_this_thread() { return _i<int>(0xAB1380); }
	static Any get_indexed_item_in_itemset(Any p0, Any p1) { return _i<Any>(0x9E9850, p0, p1); }
	static int get_index_of_current_level() { return _i<int>(0x9F9800); }
	static int get_interior_at_coords(float x, float y, float z) { return _i<int>(0x9E8830, x, y, z); }
	static int get_interior_at_coords_with_type(float x, float y, float z, const char* interiorType) { return _i<int>(0x9E8A50, x, y, z, interiorType); }
	static int get_interior_from_collision(float x, float y, float z) { return _i<int>(0x9E8C00, x, y, z); }
	static int get_interior_from_entity(Entity entity) { return _i<int>(0x9E8450, entity); }
	static int get_interior_group_id(int interiorID) { return _i<int>(0x9E8190, interiorID); }
	static bool get_is_hidef() { return _i<bool>(0x9CDC00); }
	static bool get_is_left_vehicle_headlight_damaged(Vehicle vehicle) { return _i<bool>(0xB27BC0, vehicle); }
	static bool get_is_loading_screen_active() { return _i<bool>(0x9D2C00); }
	static bool get_is_ped_gadget_equipped(Ped ped, uint32_t gadgetHash) { return _i<bool>(0xB39D20, ped, gadgetHash); }
	static bool get_is_right_vehicle_headlight_damaged(Vehicle vehicle) { return _i<bool>(0xB27C40, vehicle); }
	static bool get_is_task_active(Ped ped, int taskNumber) { return _i<bool>(0xAF3960, ped, taskNumber); }
	static bool get_is_vehicle_engine_running(Vehicle vehicle) { return _i<bool>(0xB28510, vehicle); }
	static bool get_is_vehicle_primary_colour_custom(Vehicle vehicle) { return _i<bool>(0xB1CC40, vehicle); }
	static bool get_is_vehicle_secondary_colour_custom(Vehicle vehicle) { return _i<bool>(0xB1CE40, vehicle); }
	static bool get_is_waypoint_recording_loaded(const char* name) { return _i<bool>(0xAF6800, name); }
	static bool get_is_widescreen() { return _i<bool>(0x9CDBE0); }
	static Any get_itemset_size(Any p0) { return _i<Any>(0x9E9800, p0); }
	static Ped get_jack_target(Ped ped) { return _i<Ped>(0xA93270, ped); }
	static uint32_t get_key_for_entity_in_room(Entity entity) { return _i<uint32_t>(0x9E83C0, entity); }
	static int get_landing_gear_state(Vehicle vehicle) { return _i<int>(0xB245F0, vehicle); }
	static Vehicle get_last_driven_vehicle() { return _i<Vehicle>(0xB287F0); }
	static uint32_t get_last_material_hit_by_entity(Entity entity) { return _i<uint32_t>(0x9B2840, entity); }
	static Ped get_last_ped_in_vehicle_seat(Vehicle vehicle, int seatIndex) { return _i<Ped>(0xB1DA30, vehicle, seatIndex); }
	static int get_length_of_literal_string(const char* string) { return _i<int>(0x9E0F20, string); }
	static int get_length_of_string_with_this_text_label(const char* gxt) { return _i<int>(0x9E0ED0, gxt); }
	static const char* get_livery_name(Vehicle vehicle, int liveryIndex) { return _i<const char*>(0xB275E0, vehicle, liveryIndex); }
	static int get_local_player_aim_state() { return _i<int>(0xA6B130); }
	static Void get_local_time(int* year, int* month, int* day, int* hour, int* minute, int* second) { return _i<Void>(0x996B20, year, month, day, hour, minute, second); }
	static Blip get_main_player_blip_id() { return _i<Blip>(0x9E3430); }
	static int get_maximum_number_of_photos() { return _i<int>(0x9CBE60); }
	static bool get_max_ammo(Ped ped, uint32_t weaponHash, int* ammo) { return _i<bool>(0xB397E0, ped, weaponHash, ammo); }
	static bool get_max_ammo_by_type(Ped p0, int p1, int* p2) { return _i<Any>(0xB398A0, p0, p1, p2); }
	static int get_max_ammo_in_clip(Ped ped, uint32_t weaponHash, bool p2) { return _i<int>(0xB39430, ped, weaponHash, p2); }
	static float get_max_range_of_current_ped_weapon(Ped ped) { return _i<float>(0xB39ED0, ped); }
	static int get_max_wanted_level() { return _i<int>(0xAAA8E0); }
	static Ped get_melee_target_for_ped(Ped ped) { return _i<Ped>(0xA9BA50, ped); }
	static int get_milliseconds_per_game_minute() { return _i<int>(0x996AD0); }
	static bool get_mission_flag() { return _i<bool>(0x9F5850); }
	static Void get_mobile_phone_position(math::vector3<float>* position) { return _i<Void>(0xAAF260, position); }
	static Void get_mobile_phone_render_id(int* renderId) { return _i<Void>(0xAAF460, renderId); }
	static Void get_mobile_phone_rotation(math::vector3<float>* rotation, Any p1) { return _i<Void>(0xAAF0E0, rotation, p1); }
	static Void get_model_dimensions(uint32_t modelHash, math::vector3<float>* minimum, math::vector3<float>* maximum) { return _i<Void>(0x9F8480, modelHash, minimum, maximum); }
	static const char* get_mod_slot_name(Vehicle vehicle, int modType) { return _i<const char*>(0xB27540, vehicle, modType); }
	static const char* get_mod_text_label(Vehicle vehicle, int modType, int modValue) { return _i<const char*>(0xB27490, vehicle, modType, modValue); }
	static Ped get_mount(Ped ped) { return _i<Ped>(0xA91090, ped); }
	static Any get_music_playtime() { return _i<Any>(0x986B30); }
	static Any get_named_rendertarget_render_id(const char* p0) { return _i<Any>(0x9E1CD0, p0); }
	static const char* get_name_of_zone(float x, float y, float z) { return _i<const char*>(0xB3C620, x, y, z); }
	static Any get_navmesh_route_distance_remaining(Ped ped, Any* p1, Any* p2) { return _i<Any>(0xAF2DC0, ped, p1, p2); }
	static int get_navmesh_route_result(Ped ped) { return _i<int>(0xAF2EA0, ped); }
	static Player get_nearest_player_to_entity(Entity entity) { return _i<Player>(0x9B3D30, entity); }
	static Player get_nearest_player_to_entity_on_team(Entity entity, int team) { return _i<Player>(0x9B3D70, entity, team); }
	static int get_network_id_from_sound_id(int soundId) { return _i<int>(0x9836F0, soundId); }
	static int get_network_time() { return _i<int>(0xA540D0); }
	static Blip get_next_blip_info_id(int blipSprite) { return _i<Blip>(0x9E1F60, blipSprite); }
	static uint32_t get_next_weather_type_hash_name() { return _i<uint32_t>(0x9F5BA0); }
	// [aligned] static bool get_no_loading_screen() { return _i<bool>(0xAB1670); }
	static bool get_nth_closest_vehicle_node(float x, float y, float z, int nthClosest, math::vector3<float>* outPosition, Any unknown1, Any unknown2, Any unknown3) { return _i<bool>(0xA6F990, x, y, z, nthClosest, outPosition, unknown1, unknown2, unknown3); }
	static bool get_nth_closest_vehicle_node_favour_direction(float x, float y, float z, float desiredX, float desiredY, float desiredZ, int nthClosest, math::vector3<float>* outPosition, float* outHeading, int nodetype, Any p10, Any p11) { return _i<bool>(0xA6FCC0, x, y, z, desiredX, desiredY, desiredZ, nthClosest, outPosition, outHeading, nodetype, p10, p11); }
	static int get_nth_closest_vehicle_node_id(float x, float y, float z, int nth, int nodetype, float p5, float p6) { return _i<int>(0xA6FA70, x, y, z, nth, nodetype, p5, p6); }
	static Any get_nth_closest_vehicle_node_id_with_heading(float x, float y, float z, int nthClosest, math::vector3<float>* outPosition, float outHeading, Any p6, float p7, float p8) { return _i<Any>(0xA6FC10, x, y, z, nthClosest, outPosition, outHeading, p6, p7, p8); }
	static int get_number_of_active_blips() { return _i<int>(0x9E1EF0); }
	static int get_number_of_events(bool p0) { return _i<int>(0xAB1430, p0); }
	static int get_number_of_fires_in_range(float x, float y, float z, float radius) { return _i<int>(0x9BB260, x, y, z, radius); }
	static int get_number_of_ped_drawable_variations(Ped ped, int componentId) { return _i<int>(0xA95E20, ped, componentId); }
	static int get_number_of_ped_prop_drawable_variations(Ped ped, int propId) { return _i<int>(0xA95EF0, ped, propId); }
	static int get_number_of_ped_prop_texture_variations(Ped ped, int propId, int drawableId) { return _i<int>(0xA95F20, ped, propId, drawableId); }
	static int get_number_of_ped_texture_variations(Ped ped, int componentId, int drawableId) { return _i<int>(0xA95EC0, ped, componentId, drawableId); }
	static int get_number_of_players() { return _i<int>(0xAA9440); }
	static int get_number_of_streaming_requests() { return _i<int>(0xAD9CE0); }
	static int get_number_of_vehicle_colours(Vehicle vehicle) { return _i<int>(0xB218A0, vehicle); }
	static Any get_number_of_vehicle_doors(Vehicle vehicle) { return _i<Any>(0xB2B820, vehicle); }
	static int get_number_of_vehicle_number_plates() { return _i<int>(0xB1F2E0); }
	static int get_num_dlc_vehicles() { return _i<int>(0x9A2C10); }
	static int get_num_dlc_weapons() { return _i<int>(0x9A2CF0); }
	static int get_num_dlc_weapon_components(int dlcWeaponIndex) { return _i<int>(0x9A2DC0, dlcWeaponIndex); }
	static int get_num_mod_colors(int p0, bool p1) { return _i<int>(0xB26A70, p0, p1); }
	static int get_num_mod_kits(Vehicle vehicle) { return _i<int>(0xB267D0, vehicle); }
	static int get_num_reserved_mission_objects(bool p0, Any p1) { return _i<int>(0xA53A00, p0, p1); }
	static int get_num_reserved_mission_peds(bool p0, Any p1) { return _i<int>(0xA53AC0, p0, p1); }
	static int get_num_reserved_mission_vehicles(bool p0, Any p1) { return _i<int>(0xA53B80, p0, p1); }
	static int get_num_vehicle_mods(Vehicle vehicle, int modType) { return _i<int>(0xB27210, vehicle, modType); }
	static float get_object_fragment_damage_health(Any p0, bool p1) { return _i<float>(0xA65280, p0, p1); }
	static Object get_object_index_from_entity_index(Entity entity) { return _i<Object>(0x9B3C90, entity); }
	// [aligned] static const char* get_online_version() { return _i<const char*>(0xA49D50); }
	static const char* get_onscreen_keyboard_result() { return _i<const char*>(0x9FA990); }
	static int get_pause_menu_state() { return _i<int>(0x9E52D0); }
	static const char* get_pedheadshot_txd_string(int handle) { return _i<const char*>(0xA9D5F0, handle); }
	static Ped get_peds_jacker(Ped ped) { return _i<Ped>(0xA93250, ped); }
	static int get_ped_accuracy(Ped ped) { return _i<int>(0xA906D0, ped); }
	static int get_ped_alertness(Ped ped) { return _i<int>(0xA954C0, ped); }
	static int get_ped_ammo_by_type(Ped ped, Any ammoType) { return _i<int>(0xB39A40, ped, ammoType); }
	static int get_ped_armour(Ped ped) { return _i<int>(0xA91940, ped); }
	static Ped get_ped_as_group_leader(int groupID) { return _i<Ped>(0xA94D40, groupID); }
	static Ped get_ped_as_group_member(int groupID, int memberNumber) { return _i<Ped>(0xA94CC0, groupID, memberNumber); }
	static int get_ped_bone_index(Ped ped, int boneId) { return _i<int>(0xA9C4C0, ped, boneId); }
	static uint32_t get_ped_cause_of_death(Ped ped) { return _i<uint32_t>(0xA93640, ped); }
	static int get_ped_combat_movement(Ped ped) { return _i<int>(0xA9B620, ped); }
	static Any get_ped_combat_range(Ped ped) { return _i<Any>(0xA9B720, ped); }
	static bool get_ped_config_flag(Ped ped, int flagId, bool p2) { return _i<bool>(0xA9A920, ped, flagId, p2); }
	static Any get_ped_decorations_state(Ped ped) { return _i<Any>(0xA99290, ped); }
	static float get_ped_desired_move_blend_ratio(Ped ped) { return _i<float>(0xAF4A20, ped); }
	static int get_ped_drawable_variation(Ped ped, int componentId) { return _i<int>(0xA95DB0, ped, componentId); }
	static float get_ped_enveff_scale(Ped ped) { return _i<float>(0xA9C5A0, ped); }
	static int get_ped_group_index(Ped ped) { return _i<int>(0xA940C0, ped); }
	static bool get_ped_head_blend_data(Ped ped, Any* headBlendData) { return _i<bool>(0xA964B0, ped, headBlendData); }
	static Ped get_ped_index_from_entity_index(Entity entity) { return _i<Ped>(0x9B3CA0, entity); }
	static Ped get_ped_in_vehicle_seat(Vehicle vehicle, int index, Any p2) { return _i<Ped>(0xB1DA00, vehicle, index, p2); }
	static bool get_ped_last_damage_bone(Ped ped, int* outBone) { return _i<bool>(0xA91A50, ped, outBone); }
	static bool get_ped_last_weapon_impact_coord(Ped ped, math::vector3<float>* coords) { return _i<bool>(0xB39C00, ped, coords); }
	static int get_ped_max_health(Ped ped) { return _i<int>(0xA94900, ped); }
	static int get_ped_money(Ped ped) { return _i<int>(0xA91230, ped); }
	static int get_ped_nearby_peds(Ped ped, int* sizeAndPeds, int ignore) { return _i<int>(0xA9D250, ped, sizeAndPeds, ignore); }
	static int get_ped_nearby_vehicles(Ped ped, int* sizeAndVehs) { return _i<int>(0xA9D220, ped, sizeAndVehs); }
	static int get_ped_palette_variation(Ped ped, int componentId) { return _i<int>(0xA95F50, ped, componentId); }
	static int get_ped_parachute_landing_type(Ped ped) { return _i<int>(0xA923E0, ped); }
	static int get_ped_parachute_state(Ped ped) { return _i<int>(0xA92310, ped); }
	static Void get_ped_parachute_tint_index(Ped ped, int* outTintIndex) { return _i<Void>(0xA924A0, ped, outTintIndex); }
	static int get_ped_prop_index(Ped ped, int componentId) { return _i<int>(0xA973F0, ped, componentId); }
	static int get_ped_prop_texture_index(Ped ped, int componentId) { return _i<int>(0xA97660, ped, componentId); }
	static int get_ped_ragdoll_bone_index(Ped ped, int bone) { return _i<int>(0xA9C4F0, ped, bone); }
	static uint32_t get_ped_relationship_group_default_hash(Ped ped) { return _i<uint32_t>(0xA93AB0, ped); }
	static uint32_t get_ped_relationship_group_hash(Ped ped) { return _i<uint32_t>(0xA93B10, ped); }
	static bool get_ped_reset_flag(Ped ped, int flagId) { return _i<bool>(0xA9A950, ped, flagId); }
	static Entity get_ped_source_of_death(Ped ped) { return _i<Entity>(0xA935E0, ped); }
	static bool get_ped_stealth_movement(Ped ped) { return _i<bool>(0xA92D60, ped); }
	static int get_ped_texture_variation(Ped ped, int componentId) { return _i<int>(0xA95E50, ped, componentId); }
	static int get_ped_time_of_death(Ped ped) { return _i<int>(0xA93690, ped); }
	static int get_ped_type(Ped ped) { return _i<int>(0xA94770, ped); }
	// [aligned] static Ped get_ped_using_vehicle_door(Vehicle vehicle, int doorIndex) { return _i<Ped>(0xB21640, vehicle, doorIndex); }
	static float get_ped_waypoint_distance(Any p0) { return _i<float>(0xAF6CE0, p0); }
	static Any get_ped_waypoint_progress(Any p0) { return _i<Any>(0xAF6BE0, p0); }
	static uint32_t get_ped_weapontype_in_slot(Ped ped, uint32_t weaponSlot) { return _i<uint32_t>(0xB38D50, ped, weaponSlot); }
	static int get_ped_weapon_tint_index(Ped ped, uint32_t weaponHash) { return _i<int>(0xB3A600, ped, weaponHash); }
	static float get_phone_gesture_anim_current_time(Ped ped) { return _i<float>(0xAF35A0, ped); }
	static float get_phone_gesture_anim_total_time(Ped ped) { return _i<float>(0xAF3640, ped); }
	static Object get_pickup_object(Pickup pickup) { return _i<Object>(0xA661A0, pickup); }
	static Vehicle get_players_last_vehicle() { return _i<Vehicle>(0xAAAE00); }
	static float get_player_current_stealth_noise(Player player) { return _i<float>(0xAAC620, player); }
	static int get_player_group(Player player) { return _i<int>(0xAAABB0, player); }
	static bool get_player_has_reserve_parachute(Player player) { return _i<bool>(0xAACCD0, player); }
	static Player get_player_index() { return _i<Player>(0xAAAE30); }
	static bool get_player_invincible(Player player) { return _i<bool>(0xAAB3A0, player); }
	static int get_player_max_armour(Player player) { return _i<int>(0xAAABD0, player); }
	static const char* get_player_name(Player player) { return _i<const char*>(0xAA95B0, player); }
	static Void get_player_parachute_pack_tint_index(Player player, int* tintIndex) { return _i<Void>(0xAACC60, player, tintIndex); }
	static Void get_player_parachute_smoke_trail_color(Player player, int* r, int* g, int* b) { return _i<Void>(0xAACE10, player, r, g, b); }
	static Void get_player_parachute_tint_index(Player player, int* tintIndex) { return _i<Void>(0xAACB60, player, tintIndex); }
	static Ped get_player_ped(Player player) { return _i<Ped>(0xAA9310, player); }
	static Player get_player_ped_is_following(Ped ped) { return _i<Player>(0xA94140, ped); }
	static Entity get_player_ped_script_index(Player player) { return _i<Entity>(0xAA9360, player); }
	static Any get_player_radio_station_genre() { return _i<Any>(0x984490); }
	static int get_player_radio_station_index() { return _i<int>(0x9843E0); }
	static const char* get_player_radio_station_name() { return _i<const char*>(0x984420); }
	static Void get_player_reserve_parachute_tint_index(Player player, int* index) { return _i<Void>(0xAACBE0, player, index); }
	static Void get_player_rgb_colour(Player player, int* r, int* g, int* b) { return _i<Void>(0xAA93E0, player, r, g, b); }
	static int get_player_short_switch_state() { return _i<int>(0xADA680); }
	static float get_player_sprint_stamina_remaining(Player player) { return _i<float>(0xAAAA20, player); }
	static float get_player_sprint_time_remaining(Player player) { return _i<float>(0xAAAA70, player); }
	static int get_player_switch_state() { return _i<int>(0xADA660); }
	static int get_player_switch_type() { return _i<int>(0xADA5F0); }
	static bool get_player_target_entity(Player player, Entity* entity) { return _i<bool>(0xAAA5D0, player, entity); }
	static int get_player_team(Player player) { return _i<int>(0xAA9470, player); }
	static float get_player_underwater_time_remaining(Player player) { return _i<float>(0xAAAAE0, player); }
	static int get_player_wanted_level(Player player) { return _i<int>(0xAA9C80, player); }
	static float get_position_in_recording(Any p0) { return _i<float>(0xB1FAD0, p0); }
	static Void get_posix_time(int* year, int* month, int* day, int* hour, int* minute, int* second) { return _i<Void>(0x996AE0, year, month, day, hour, minute, second); }
	static uint32_t get_prev_weather_type_hash_name() { return _i<uint32_t>(0x9F5B80); }
	static int get_profile_setting(int profileSetting) { return _i<int>(0x9F86E0, profileSetting); }
	static const char* get_radio_station_name(int radioStation) { return _i<const char*>(0x984450, radioStation); }
	static Any get_rain_level() { return _i<Any>(0x9F6270); }
	static float get_random_float_in_range(float startRange, float endRange) { return _i<float>(0x9F6440, startRange, endRange); }
	static int get_random_int_in_range(int startRange, int endRange) { return _i<int>(0x9F64A0, startRange, endRange); }
	static Ped get_random_ped_at_coord(float x, float y, float z, float xRadius, float yRadius, float zRadius, int pedType) { return _i<Ped>(0xA97A20, x, y, z, xRadius, yRadius, zRadius, pedType); }
	static Vehicle get_random_vehicle_back_bumper_in_sphere(float p0, float p1, float p2, float p3, int p4, int p5, int p6) { return _i<Vehicle>(0xB203A0, p0, p1, p2, p3, p4, p5, p6); }
	static Vehicle get_random_vehicle_front_bumper_in_sphere(float p0, float p1, float p2, float p3, int p4, int p5, int p6) { return _i<Vehicle>(0xB20320, p0, p1, p2, p3, p4, p5, p6); }
	static Vehicle get_random_vehicle_in_sphere(float x, float y, float z, float radius, uint32_t modelHash, int flags) { return _i<Vehicle>(0xB202A0, x, y, z, radius, modelHash, flags); }
	static Void get_random_vehicle_model_in_memory(bool p0, uint32_t* modelHash, int* p2) { return _i<Void>(0xB21C60, p0, modelHash, p2); }
	static bool get_random_vehicle_node(float x, float y, float z, float radius, bool p4, bool p5, bool p6, math::vector3<float>* outPosition, int* nodeId) { return _i<bool>(0xA703C0, x, y, z, radius, p4, p5, p6, outPosition, nodeId); }
	static int get_relationship_between_groups(uint32_t group1, uint32_t group2) { return _i<int>(0xA93B70, group1, group2); }
	static int get_relationship_between_peds(Ped ped1, Ped ped2) { return _i<int>(0xA93A00, ped1, ped2); }
	static Cam get_rendering_cam() { return _i<Cam>(0x991550); }
	static uint32_t get_room_key_from_entity(Entity entity) { return _i<uint32_t>(0x9E83A0, entity); }
	static int get_rope_vertex_count(Object rope) { return _i<int>(0xAA0A20, rope); }
	static bool get_safe_coord_for_ped(float x, float y, float z, bool onGround, math::vector3<float>* outPosition, int flags) { return _i<bool>(0xA6F5A0, x, y, z, onGround, outPosition, flags); }
	static bool get_screen_coord_from_world_coord(float worldX, float worldY, float worldZ, float* screenX, float* screenY) { return _i<bool>(0x9CDD40, worldX, worldY, worldZ, screenX, screenY); }
	static Void get_screen_resolution(int* x, int* y) { return _i<Void>(0x9CDB30, x, y); }
	static int get_script_task_status(Ped targetPed, uint32_t taskHash) { return _i<int>(0xAF3A00, targetPed, taskHash); }
	static int get_seat_ped_is_trying_to_enter(Ped ped) { return _i<int>(0xA935A0, ped); }
	static uint32_t get_selected_ped_weapon(Ped ped) { return _i<uint32_t>(0xB39D90, ped); }
	static int get_sequence_progress(Ped ped) { return _i<int>(0xAF3940, ped); }
	static int get_shape_test_result(int rayHandle, bool* hit, math::vector3<float>* endCoords, math::vector3<float>* surfaceNormal, Entity* entityHit) { return _i<int>(0xAB34B0, rayHandle, hit, endCoords, surfaceNormal, entityHit); }
	static Void get_shop_ped_component(uint32_t p0, Any* p1) { return _i<Void>(0x9A2680, p0, p1); }
	static Void get_shop_ped_outfit(Any p0, Any* p1) { return _i<Void>(0x9A2AC0, p0, p1); }
	static Any get_shop_ped_outfit_locate(Any p0) { return _i<Any>(0x9A2AD0, p0); }
	static Void get_shop_ped_query_component(int componentId, int* outComponent) { return _i<Void>(0x9A2640, componentId, outComponent); }
	static Void get_shop_ped_query_outfit(Any p0, Any* outfit) { return _i<Void>(0x9A2AB0, p0, outfit); }
	static Void get_shop_ped_query_prop(Any p0, Any* p1) { return _i<Void>(0x9A2690, p0, p1); }
	static Any get_snow_level() { return _i<Any>(0x9F62B0); }
	static int get_sound_id() { return _i<int>(0x9833A0); }
	static int get_sound_id_from_network_id(int netId) { return _i<int>(0x983720, netId); }
	static int get_stream_play_time() { return _i<int>(0x985720); }
	static Void get_street_name_at_coord(float x, float y, float z, uint32_t* streetName, uint32_t* crossingRoad) { return _i<Void>(0xA705B0, x, y, z, streetName, crossingRoad); }
	static const char* get_street_name_from_hash_key(uint32_t hash) { return _i<const char*>(0x9E0F80, hash); }
	static float get_synchronized_scene_phase(int sceneID) { return _i<float>(0xA9CC70, sceneID); }
	static float get_synchronized_scene_rate(int sceneID) { return _i<float>(0xA9CD20, sceneID); }
	static const char* get_this_script_name() { return _i<const char*>(0xAB13E0); }
	static int get_timecycle_modifier_index() { return _i<int>(0x9D0910); }
	static const char* get_time_as_string(int time) { return _i<const char*>(0xA54270, time); }
	static int get_time_difference(int timeA, int timeB) { return _i<int>(0xA54260, timeA, timeB); }
	static int get_time_offset(int timeA, int timeB) { return _i<int>(0xA541F0, timeA, timeB); }
	static float get_time_position_in_recording(Any p0) { return _i<float>(0xB1FB20, p0); }
	static int get_time_since_last_arrest() { return _i<int>(0xAAB860); }
	static int get_time_since_last_death() { return _i<int>(0xAAB890); }
	static int get_time_since_player_drove_against_traffic(Player player) { return _i<int>(0xAAAF50, player); }
	static int get_time_since_player_drove_on_pavement(Player player) { return _i<int>(0xAAAF10, player); }
	static int get_time_since_player_hit_ped(Player player) { return _i<int>(0xAAAED0, player); }
	static int get_time_since_player_hit_vehicle(Player player) { return _i<int>(0xAAAE90, player); }
	static Any get_total_duration_of_vehicle_recording(Any p0, Any p1) { return _i<Any>(0xB1FA90, p0, p1); }
	static float get_total_duration_of_vehicle_recording_id(Any p0) { return _i<float>(0xB1FA60, p0); }
	static Entity get_train_carriage(Vehicle train, int trailerNumber) { return _i<Entity>(0xB20450, train, trailerNumber); }
	static int get_tv_channel() { return _i<int>(0x9D1BC0); }
	static float get_tv_volume() { return _i<float>(0x9D1BE0); }
	static Void get_utc_time(int* year, int* month, int* day, int* hour, int* minute, int* second) { return _i<Void>(0x996B00, year, month, day, hour, minute, second); }
	static float get_vehicle_acceleration(Vehicle vehicle) { return _i<float>(0xB25B00, vehicle); }
	static Vehicle get_vehicle_attached_to_cargobob(Vehicle cargobob) { return _i<Vehicle>(0xB298D0, cargobob); }
	static float get_vehicle_body_health(Vehicle vehicle) { return _i<float>(0xB2B610, vehicle); }
	static uint32_t get_vehicle_cause_of_destruction(Vehicle vehicle) { return _i<uint32_t>(0xB27A00, vehicle); }
	static int get_vehicle_class(Vehicle vehicle) { return _i<int>(0xB2A530, vehicle); }
	static int get_vehicle_class_from_name(uint32_t modelHash) { return _i<int>(0xB2A590, modelHash); }
	static float get_vehicle_class_max_acceleration(int vehicleClass) { return _i<float>(0xB26130, vehicleClass); }
	static float get_vehicle_class_max_agility(int vehicleClass) { return _i<float>(0xB260E0, vehicleClass); }
	static float get_vehicle_class_max_braking(int vehicleClass) { return _i<float>(0xB26180, vehicleClass); }
	static float get_vehicle_class_max_traction(int vehicleClass) { return _i<float>(0xB26090, vehicleClass); }
	static Void get_vehicle_color(Vehicle vehicle, int* r, int* g, int* b) { return _i<Void>(0xB279C0, vehicle, r, g, b); }
	static Void get_vehicle_colours(Vehicle vehicle, int* colorPrimary, int* colorSecondary) { return _i<Void>(0xB1D970, vehicle, colorPrimary, colorSecondary); }
	static int get_vehicle_colour_combination(Vehicle vehicle) { return _i<int>(0xB21920, vehicle); }
	static Void get_vehicle_custom_primary_colour(Vehicle vehicle, int* r, int* g, int* b) { return _i<Void>(0xB1CB70, vehicle, r, g, b); }
	static Void get_vehicle_custom_secondary_colour(Vehicle vehicle, int* r, int* g, int* b) { return _i<Void>(0xB1CD70, vehicle, r, g, b); }
	static uint32_t get_vehicle_default_horn(Vehicle vehicle) { return _i<uint32_t>(0x986D90, vehicle); }
	static float get_vehicle_dirt_level(Vehicle vehicle) { return _i<float>(0xB21200, vehicle); }
	static bool get_vehicle_doors_locked_for_player(Vehicle vehicle, Player player) { return _i<bool>(0xB1C0B0, vehicle, player); }
	static float get_vehicle_door_angle_ratio(Vehicle vehicle, int door) { return _i<float>(0xB215A0, vehicle, door); }
	static int get_vehicle_door_lock_status(Vehicle vehicle) { return _i<int>(0xB21C70, vehicle); }
	static float get_vehicle_engine_health(Vehicle vehicle) { return _i<float>(0xB23140, vehicle); }
	static float get_vehicle_enveff_scale(Vehicle vehicle) { return _i<float>(0xB1CF10, vehicle); }
	// [aligned] static float get_vehicle_estimated_max_speed(Vehicle vehicle) { return _i<float>(0xB25A70, vehicle); }
	static Void get_vehicle_extra_colours(Vehicle vehicle, int* pearlescentColor, int* wheelColor) { return _i<Void>(0xB22220, vehicle, pearlescentColor, wheelColor); }
	static Vehicle get_vehicle_index_from_entity_index(Entity entity) { return _i<Vehicle>(0x9B3CB0, entity); }
	static uint32_t get_vehicle_layout_hash(Vehicle vehicle) { return _i<uint32_t>(0xB22080, vehicle); }
	static bool get_vehicle_lights_state(Vehicle vehicle, bool* lightsOn, bool* highbeamsOn) { return _i<bool>(0xB1DAB0, vehicle, lightsOn, highbeamsOn); }
	static int get_vehicle_livery(Vehicle vehicle) { return _i<int>(0xB23BE0, vehicle); }
	static int get_vehicle_livery_count(Vehicle vehicle) { return _i<int>(0xB23C40, vehicle); }
	static float get_vehicle_max_braking(Vehicle vehicle) { return _i<float>(0xB25AA0, vehicle); }
	static int get_vehicle_max_number_of_passengers(Vehicle vehicle) { return _i<int>(0xB1BB90, vehicle); }
	static float get_vehicle_max_traction(Vehicle vehicle) { return _i<float>(0xB25AD0, vehicle); }
	static int get_vehicle_mod(Vehicle vehicle, int modType) { return _i<int>(0xB27060, vehicle, modType); }
	static float get_vehicle_model_acceleration(uint32_t modelHash) { return _i<float>(0xB25E70, modelHash); }
	static float get_vehicle_model_max_braking(uint32_t modelHash) { return _i<float>(0xB25BE0, modelHash); }
	static float get_vehicle_model_max_traction(uint32_t modelHash) { return _i<float>(0xB25D90, modelHash); }
	static int get_vehicle_model_number_of_seats(uint32_t modelHash) { return _i<int>(0xB1BBF0, modelHash); }
	static int get_vehicle_mod_kit(Vehicle vehicle) { return _i<int>(0xB268A0, vehicle); }
	static int get_vehicle_mod_kit_type(Vehicle vehicle) { return _i<int>(0xB26900, vehicle); }
	static Any get_vehicle_mod_modifier_value(Vehicle vehicle, int modType, int modIndex) { return _i<Any>(0xB27660, vehicle, modType, modIndex); }
	static bool get_vehicle_mod_variation(Vehicle vehicle, int modType) { return _i<bool>(0xB27140, vehicle, modType); }
	static bool get_vehicle_node_properties(float x, float y, float z, int* density, int* flags) { return _i<bool>(0xA6FDD0, x, y, z, density, flags); }
	static int get_vehicle_number_of_passengers(Vehicle vehicle) { return _i<int>(0xB1BB00, vehicle); }
	static const char* get_vehicle_number_plate_text(Vehicle vehicle) { return _i<const char*>(0xB1F280, vehicle); }
	static int get_vehicle_number_plate_text_index(Vehicle vehicle) { return _i<int>(0xB1F350, vehicle); }
	static Vehicle get_vehicle_ped_is_entering(Ped ped) { return _i<Vehicle>(0xA94380, ped); }
	static Vehicle get_vehicle_ped_is_in(Ped ped, bool lastVehicle) { return _i<Vehicle>(0xA90E80, ped, lastVehicle); }
	static Vehicle get_vehicle_ped_is_trying_to_enter(Ped ped) { return _i<Vehicle>(0xA935C0, ped); }
	static Vehicle get_vehicle_ped_is_using(Ped ped) { return _i<Vehicle>(0xA94300, ped); }
	static float get_vehicle_petrol_tank_health(Vehicle vehicle) { return _i<float>(0xB23340, vehicle); }
	static int get_vehicle_plate_type(Vehicle vehicle) { return _i<int>(0xB28E20, vehicle); }
	static Any get_vehicle_recording_id(int p0, const char* p1) { return _i<Any>(0xB1F6A0, p0, p1); }
	static bool get_vehicle_trailer_vehicle(Vehicle vehicle, Vehicle* trailer) { return _i<bool>(0xB256B0, vehicle, trailer); }
	static bool get_vehicle_tyres_can_burst(Vehicle vehicle) { return _i<bool>(0xB1E460, vehicle); }
	static Void get_vehicle_tyre_smoke_color(Vehicle vehicle, int* r, int* g, int* b) { return _i<Void>(0xB27870, vehicle, r, g, b); }
	static Any get_vehicle_waypoint_progress(Any p0) { return _i<Any>(0xAF75B0, p0); }
	static Any get_vehicle_waypoint_target_point(Any p0) { return _i<Any>(0xAF7630, p0); }
	static int get_vehicle_wheel_type(Vehicle vehicle) { return _i<int>(0xB26970, vehicle); }
	static int get_vehicle_window_tint(Vehicle vehicle) { return _i<int>(0xB27930, vehicle); }
	static float get_wanted_level_radius(Player player) { return _i<float>(0xAA9630, player); }
	static int get_wanted_level_threshold(int wantedLevel) { return _i<int>(0xAA9780, wantedLevel); }
	static float get_waypoint_distance_along_route(const char* p0, int p1) { return _i<float>(0xAF6E20, p0, p1); }
	static uint32_t get_weapontype_group(uint32_t weaponHash) { return _i<uint32_t>(0xB387A0, weaponHash); }
	static uint32_t get_weapontype_model(uint32_t weaponHash) { return _i<uint32_t>(0xB38660, weaponHash); }
	static uint32_t get_weapontype_slot(uint32_t weaponHash) { return _i<uint32_t>(0xB38730, weaponHash); }
	static int get_weapon_clip_size(uint32_t weaponHash) { return _i<int>(0xB3AAD0, weaponHash); }
	static bool get_weapon_component_hud_stats(uint32_t componentHash, int* outData) { return _i<bool>(0xB3AA00, componentHash, outData); }
	static uint32_t get_weapon_component_type_model(uint32_t componentHash) { return _i<uint32_t>(0xB385A0, componentHash); }
	static int get_weapon_damage_type(uint32_t weaponHash) { return _i<int>(0xB3B040, weaponHash); }
	static bool get_weapon_hud_stats(uint32_t weaponHash, int* outData) { return _i<bool>(0xB3A950, weaponHash, outData); }
	static Object get_weapon_object_from_ped(Ped ped, bool p1) { return _i<Object>(0xB3A560, ped, p1); }
	static int get_weapon_object_tint_index(Object weapon) { return _i<int>(0xB3A680, weapon); }
	static int get_weapon_tint_count(uint32_t weaponHash) { return _i<int>(0xB3A6E0, weaponHash); }
	static float get_wind_speed() { return _i<float>(0x9F61A0); }
	static int get_zone_at_coords(float x, float y, float z) { return _i<int>(0xB3C500, x, y, z); }
	static int get_zone_from_name_id(const char* zoneName) { return _i<int>(0xB3C590, zoneName); }
	static int get_zone_popschedule(int zoneId) { return _i<int>(0xB3C5D0, zoneId); }
	static int get_zone_scumminess(int zoneId) { return _i<int>(0xB3C6F0, zoneId); }
	static bool give_achievement_to_player(int achievement) { return _i<bool>(0xAAB1F0, achievement); }
	static Void give_delayed_weapon_to_ped(Ped ped, uint32_t weaponHash, int time, bool equipNow) { return _i<Void>(0xB39000, ped, weaponHash, time, equipNow); }
	static Void give_ped_helmet(Ped ped, bool cannotRemove, int helmetFlag, int textureIndex) { return _i<Void>(0xA9AD50, ped, cannotRemove, helmetFlag, textureIndex); }
	static Void give_ped_nm_message(Ped ped) { return _i<Void>(0xA99980, ped); }
	static Void give_ped_to_pause_menu(Ped ped, int p1) { return _i<Void>(0x9E5A60, ped, p1); }
	static Void give_player_ragdoll_control(Player player, bool toggle) { return _i<Void>(0xAAB500, player, toggle); }
	static Void give_weapon_component_to_ped(Ped ped, uint32_t weaponHash, uint32_t componentHash) { return _i<Void>(0xB39F30, ped, weaponHash, componentHash); }
	static Void give_weapon_component_to_weapon_object(Object weaponObject, uint32_t addonHash) { return _i<Void>(0xB3A3C0, weaponObject, addonHash); }
	static Void give_weapon_object_to_ped(Object weaponObject, Ped ped) { return _i<Void>(0xB3A520, weaponObject, ped); }
	static Void give_weapon_to_ped(Ped ped, uint32_t weaponHash, int ammoCount, bool isHidden, bool equipNow) { return _i<Void>(0xB38FD0, ped, weaponHash, ammoCount, isHidden, equipNow); }
	static bool has_achievement_been_passed(int achievement) { return _i<bool>(0xAAB280, achievement); }
	static bool has_action_mode_asset_loaded(const char* asset) { return _i<bool>(0xA9DB80, asset); }
	static bool has_additional_text_loaded(int slot) { return _i<bool>(0x9E0D60, slot); }
	static bool has_anim_dict_loaded(const char* animDict) { return _i<bool>(0xAD97E0, animDict); }
	static bool has_anim_event_fired(Entity entity, uint32_t actionHash) { return _i<bool>(0x9B5550, entity, actionHash); }
	static bool has_anim_set_loaded(const char* animSet) { return _i<bool>(0xAD98F0, animSet); }
	static bool has_closest_object_of_type_been_broken(float p0, float p1, float p2, float p3, uint32_t modelHash, Any p5) { return _i<bool>(0xA63FC0, p0, p1, p2, p3, modelHash, p5); }
	static bool has_collision_for_model_loaded(uint32_t model) { return _i<bool>(0xAD9690, model); }
	static bool has_collision_loaded_around_entity(Entity entity) { return _i<bool>(0x9B5FE0, entity); }
	static bool has_cutscene_finished() { return _i<bool>(0x9995D0); }
	static bool has_cutscene_loaded() { return _i<bool>(0x9990E0); }
	static bool has_entity_anim_finished(Entity entity, const char* animDict, const char* animName, int p3) { return _i<bool>(0x9B2520, entity, animDict, animName, p3); }
	static bool has_entity_been_damaged_by_any_object(Entity entity) { return _i<bool>(0x9B2550, entity); }
	static bool has_entity_been_damaged_by_any_ped(Entity entity) { return _i<bool>(0x9B25B0, entity); }
	static bool has_entity_been_damaged_by_any_vehicle(Entity entity) { return _i<bool>(0x9B2610, entity); }
	static bool has_entity_been_damaged_by_entity(Entity entity1, Entity entity2, bool p2) { return _i<bool>(0x9B2670, entity1, entity2, p2); }
	static bool has_entity_been_damaged_by_weapon(Entity entity, uint32_t weaponHash, int weaponType) { return _i<bool>(0xB39320, entity, weaponHash, weaponType); }
	static bool has_entity_clear_los_to_entity(Entity entity1, Entity entity2, int traceType) { return _i<bool>(0x9B26A0, entity1, entity2, traceType); }
	static bool has_entity_clear_los_to_entity_in_front(Entity entity1, Entity entity2) { return _i<bool>(0x9B2700, entity1, entity2); }
	static bool has_entity_collided_with_anything(Entity entity) { return _i<bool>(0x9B2730, entity); }
	static bool has_force_cleanup_occurred(int cleanupFlags) { return _i<bool>(0xAAB080, cleanupFlags); }
	static bool has_model_loaded(uint32_t model) { return _i<bool>(0xAD92C0, model); }
	static bool has_named_ptfx_asset_loaded(const char* fxName) { return _i<bool>(0xAD9D50, fxName); }
	static bool has_network_time_started() { return _i<bool>(0xA541C0); }
	static bool has_object_been_broken(Object object) { return _i<bool>(0xA63F20, object); }
	static bool has_ped_been_damaged_by_weapon(Ped ped, uint32_t weaponHash, int weaponType) { return _i<bool>(0xB39250, ped, weaponHash, weaponType); }
	static bool has_ped_got_weapon(Ped ped, uint32_t weaponHash, bool p2) { return _i<bool>(0xB38CB0, ped, weaponHash, p2); }
	static bool has_ped_got_weapon_component(Ped ped, uint32_t weaponHash, uint32_t componentHash) { return _i<bool>(0xB3A010, ped, weaponHash, componentHash); }
	static bool has_ped_head_blend_finished(Ped ped) { return _i<bool>(0xA96E80, ped); }
	static bool has_ped_received_event(Any p0, Any p1) { return _i<bool>(0xA9C360, p0, p1); }
	static bool has_pickup_been_collected(Pickup pickup) { return _i<bool>(0xA660C0, pickup); }
	static bool has_player_been_spotted_in_stolen_vehicle(Player player) { return _i<bool>(0xAAD3D0, player); }
	static bool has_player_damaged_at_least_one_non_animal_ped(Player player) { return _i<bool>(0xAAB6F0, player); }
	static bool has_player_damaged_at_least_one_ped(Player player) { return _i<bool>(0xAAB670, player); }
	static bool has_player_left_the_world(Player player) { return _i<bool>(0xAAD630, player); }
	static bool has_preload_mods_finished(Any p0) { return _i<bool>(0xB27760, p0); }
	static bool has_ptfx_asset_loaded() { return _i<bool>(0xAD9D10); }
	static bool has_scaleform_container_movie_loaded_into_parent(int scaleformHandle) { return _i<bool>(0x9D0FF0, scaleformHandle); }
	static bool has_scaleform_movie_loaded(int scaleformHandle) { return _i<bool>(0x9D0D80, scaleformHandle); }
	static bool has_script_loaded(const char* scriptName) { return _i<bool>(0xAB0F20, scriptName); }
	static bool has_script_with_name_hash_loaded(uint32_t scriptHash) { return _i<bool>(0xAB1150, scriptHash); }
	static bool has_sound_finished(int soundId) { return _i<bool>(0x9837D0, soundId); }
	static bool has_stealth_mode_asset_loaded(const char* asset) { return _i<bool>(0xA9DCC0, asset); }
	static bool has_streamed_texture_dict_loaded(const char* textureDict) { return _i<bool>(0x9CCA00, textureDict); }
	static bool has_this_additional_text_loaded(const char* gxt, int slot) { return _i<bool>(0x9E0DE0, gxt, slot); }
	static bool has_this_cutscene_loaded(const char* cutsceneName) { return _i<bool>(0x999110, cutsceneName); }
	static bool has_vehicle_asset_loaded(int vehicleAsset) { return _i<bool>(0xB24A90, vehicleAsset); }
	static bool has_vehicle_recording_been_loaded(Any p0, Any* p1) { return _i<bool>(0xB1F750, p0, p1); }
	static bool has_weapon_asset_loaded(uint32_t weaponHash) { return _i<bool>(0xB3A240, weaponHash); }
	static bool has_weapon_got_weapon_component(Object weapon, uint32_t addonHash) { return _i<bool>(0xB3A490, weapon, addonHash); }
	static Void hide_help_text_this_frame() { return _i<Void>(0x9E37D0); }
	static Void hide_hud_and_radar_this_frame() { return _i<Void>(0x9E36E0); }
	static Void hide_hud_component_this_frame(int id) { return _i<Void>(0x9E40F0, id); }
	static Void hide_loading_on_fade_this_frame() { return _i<Void>(0x9E3490); }
	static Void hide_number_on_blip(Blip blip) { return _i<Void>(0x9E2F30, blip); }
	static Void hide_ped_blood_damage_by_zone(Ped ped, Any p1, bool p2) { return _i<Void>(0xA99180, ped, p1, p2); }
	static Void hide_ped_weapon_for_scripted_cutscene(Ped ped, bool toggle) { return _i<Void>(0xB39120, ped, toggle); }
	static Void hide_scripted_hud_component_this_frame(int id) { return _i<Void>(0x9E4070, id); }
	static Any hint_ambient_audio_bank(Any p0, Any p1, Any p2) { return _i<Any>(0x9832D0, p0, p1, p2); }
	static Any hint_script_audio_bank(Any p0, Any p1, Any p2) { return _i<Any>(0x9832F0, p0, p1, p2); }
	static Void ignore_next_restart(bool toggle) { return _i<Void>(0x9F7CC0, toggle); }
	static Void init_shop_ped_component(int* outComponent) { return _i<Void>(0x9A24D0, outComponent); }
	static Void init_shop_ped_prop(int* outProp) { return _i<Void>(0x9A2510, outProp); }
	static Void interrupt_conversation(Any p0, Any* p1, Any* p2) { return _i<Void>(0x983050, p0, p1, p2); }
	static int int_to_participantindex(int value) { return _i<int>(0xAAAE80, value); }
	static Player int_to_playerindex(int value) { return _i<Player>(0xAAAE70, value); }
	static bool is_aim_cam_active() { return _i<bool>(0x9950C0); }
	static bool is_alarm_playing(const char* alarmName) { return _i<bool>(0x986D30, alarmName); }
	static bool is_ambient_speech_disabled(Ped ped) { return _i<bool>(0x985AD0, ped); }
	static bool is_ambient_speech_playing(Ped p0) { return _i<bool>(0x983E30, p0); }
	static bool is_ambient_zone_enabled(const char* ambientZone) { return _i<bool>(0x9853E0, ambientZone); }
	static bool is_any_object_near_point(float x, float y, float z, float range, bool p4) { return _i<bool>(0xA65350, x, y, z, range, p4); }
	static bool is_any_ped_near_point(float x, float y, float z, float radius) { return _i<bool>(0xA9BF50, x, y, z, radius); }
	static bool is_any_ped_shooting_in_area(float x1, float y1, float z1, float x2, float y2, float z2, bool p6, bool p7) { return _i<bool>(0xA905A0, x1, y1, z1, x2, y2, z2, p6, p7); }
	static bool is_any_speech_playing(Ped ped) { return _i<bool>(0x983F10, ped); }
	static bool is_any_vehicle_near_point(float x, float y, float z, float radius) { return _i<bool>(0xB24670, x, y, z, radius); }
	static bool is_an_entity(int handle) { return _i<bool>(0x9B3EB0, handle); }
	static bool is_audio_scene_active(const char* scene) { return _i<bool>(0x986830, scene); }
	static bool is_aussie_version() { return _i<bool>(0x9F9190); }
	static bool is_auto_save_in_progress() { return _i<bool>(0x9F8050); }
	static bool is_big_vehicle(Vehicle vehicle) { return _i<bool>(0xB21840, vehicle); }
	static bool is_blip_flashing(Blip blip) { return _i<bool>(0x9E2D00, blip); }
	static bool is_blip_on_minimap(Blip blip) { return _i<bool>(0x9E29E0, blip); }
	static bool is_blip_short_range(Blip blip) { return _i<bool>(0x9E29A0, blip); }
	static bool is_cam_active(Cam cam) { return _i<bool>(0x991470, cam); }
	static bool is_cam_interpolating(Cam cam) { return _i<bool>(0x9935D0, cam); }
	static bool is_cam_playing_anim(Cam cam, const char* animName, const char* animDictionary) { return _i<bool>(0x993A90, cam, animName, animDictionary); }
	static bool is_cam_rendering(Cam cam) { return _i<bool>(0x9914D0, cam); }
	static bool is_cam_shaking(Cam cam) { return _i<bool>(0x993750, cam); }
	static bool is_cam_spline_paused(Any p0) { return _i<bool>(0x9934C0, p0); }
	static bool is_cinematic_cam_shaking() { return _i<bool>(0x995F10); }
	static bool is_cinematic_shot_active(Any p0) { return _i<bool>(0x9960A0, p0); }
	static bool is_control_enabled(int inputGroup, int control) { return _i<bool>(0xA6A3F0, inputGroup, control); }
	static bool is_control_just_pressed(int inputGroup, int control) { return _i<bool>(0xA6A530, inputGroup, control); }
	static bool is_control_just_released(int inputGroup, int control) { return _i<bool>(0xA6A5A0, inputGroup, control); }
	static bool is_control_pressed(int inputGroup, int control) { return _i<bool>(0xA6A450, inputGroup, control); }
	static bool is_control_released(int inputGroup, int control) { return _i<bool>(0xA6A4C0, inputGroup, control); }
	static bool is_conversation_ped_dead(Ped ped) { return _i<bool>(0xA901C0, ped); }
	static bool is_cutscene_active() { return _i<bool>(0x999600); }
	static bool is_cutscene_playing() { return _i<bool>(0x999630); }
	static bool is_damage_tracker_active_on_network_id(int netID) { return _i<bool>(0xA533B0, netID); }
	static bool is_decal_alive(int decal) { return _i<bool>(0x9D0130, decal); }
	static bool is_disabled_control_just_pressed(int inputGroup, int control) { return _i<bool>(0xA6A9C0, inputGroup, control); }
	static bool is_disabled_control_just_released(int inputGroup, int control) { return _i<bool>(0xA6AA60, inputGroup, control); }
	static bool is_disabled_control_pressed(int inputGroup, int control) { return _i<bool>(0xA6A880, inputGroup, control); }
	static bool is_dlc_present(uint32_t dlcHash) { return _i<bool>(0x9D2B80, dlcHash); }
	static bool is_dlc_vehicle_mod(int modData) { return _i<bool>(0x9A2EE0, modData); }
	static bool is_door_closed(uint32_t door) { return _i<bool>(0xA64A50, door); }
	static bool is_driveby_task_underneath_driving_task(Ped ped) { return _i<bool>(0xAF45B0, ped); }
	static bool is_durango_version() { return _i<bool>(0x9F9130); }
	static bool is_entity_an_object(Entity entity) { return _i<bool>(0x9B4010, entity); }
	static bool is_entity_attached(Entity entity) { return _i<bool>(0x9B4110, entity); }
	static bool is_entity_attached_to_any_object(Entity entity) { return _i<bool>(0x9B41A0, entity); }
	static bool is_entity_attached_to_any_ped(Entity entity) { return _i<bool>(0x9B4230, entity); }
	static bool is_entity_attached_to_any_vehicle(Entity entity) { return _i<bool>(0x9B42C0, entity); }
	static bool is_entity_attached_to_entity(Entity from, Entity to) { return _i<bool>(0x9B4350, from, to); }
	static bool is_entity_at_entity(Entity entity1, Entity entity2, float xSize, float ySize, float zSize, bool p5, bool p6, int p7) { return _i<bool>(0x9B40C0, entity1, entity2, xSize, ySize, zSize, p5, p6, p7); }
	static bool is_entity_a_mission_entity(Entity entity) { return _i<bool>(0x9B3F50, entity); }
	static bool is_entity_a_ped(Entity entity) { return _i<bool>(0x9B3EF0, entity); }
	static bool is_entity_a_vehicle(Entity entity) { return _i<bool>(0x9B3FB0, entity); }
	static bool is_entity_dead(Entity entity, Any p1) { return _i<bool>(0x9B4380, entity, p1); }
	static bool is_entity_focus(Entity entity) { return _i<bool>(0xAD9ED0, entity); }
	static bool is_entity_in_air(Entity entity) { return _i<bool>(0x9B4440, entity); }
	static bool is_entity_in_water(Entity entity) { return _i<bool>(0x9B4600, entity); }
	static bool is_entity_in_zone(Entity entity, const char* zone) { return _i<bool>(0x9B4510, entity, zone); }
	static bool is_entity_occluded(Entity entity) { return _i<bool>(0x9B4A60, entity); }
	static bool is_entity_on_fire(Entity entity) { return _i<bool>(0x9BB1F0, entity); }
	static bool is_entity_on_screen(Entity entity) { return _i<bool>(0x9B4700, entity); }
	static bool is_entity_playing_anim(Entity entity, const char* animDict, const char* animName, int p4) { return _i<bool>(0x9B47A0, entity, animDict, animName, p4); }
	static bool is_entity_static(Entity entity) { return _i<bool>(0x9B47D0, entity); }
	static bool is_entity_touching_entity(Entity entity, Entity targetEntity) { return _i<bool>(0x9B4830, entity, targetEntity); }
	static bool is_entity_touching_model(Entity entity, uint32_t modelHash) { return _i<bool>(0x9B4860, entity, modelHash); }
	static bool is_entity_upright(Entity entity, float angle) { return _i<bool>(0x9B4890, entity, angle); }
	static bool is_entity_upsidedown(Entity entity) { return _i<bool>(0x9B4920, entity); }
	static bool is_entity_visible(Entity entity) { return _i<bool>(0x9B4980, entity); }
	static bool is_entity_visible_to_script(Entity entity) { return _i<bool>(0x9B49E0, entity); }
	static bool is_entity_waiting_for_world_collision(Entity entity) { return _i<bool>(0x9B4AD0, entity); }
	static bool is_explosion_in_angled_area(int explosionType, float x1, float y1, float z1, float x2, float y2, float z2, float angle) { return _i<bool>(0x9BB740, explosionType, x1, y1, z1, x2, y2, z2, angle); }
	static bool is_explosion_in_area(int explosionType, float x1, float y1, float z1, float x2, float y2, float z2) { return _i<bool>(0x9BB490, explosionType, x1, y1, z1, x2, y2, z2); }
	static bool is_explosion_in_sphere(int explosionType, float x, float y, float z, float radius) { return _i<bool>(0x9BB630, explosionType, x, y, z, radius); }
	static bool is_first_person_aim_cam_active() { return _i<bool>(0x995160); }
	static bool is_flash_light_on(Ped ped) { return _i<bool>(0xB3AEB0, ped); }
	static bool is_follow_ped_cam_active() { return _i<bool>(0x994A10); }
	static bool is_follow_vehicle_cam_active() { return _i<bool>(0x994E60); }
	static bool is_frontend_fading() { return _i<bool>(0x9F97E0); }
	static bool is_gameplay_cam_looking_behind() { return _i<bool>(0x9947C0); }
	static bool is_gameplay_cam_rendering() { return _i<bool>(0x9946C0); }
	static bool is_gameplay_cam_shaking() { return _i<bool>(0x9945C0); }
	static bool is_gameplay_hint_active() { return _i<bool>(0x995BC0); }
	static bool is_game_in_control_of_music() { return _i<bool>(0x9866D0); }
	static bool is_garage_empty(Any garage, bool p1, int p2) { return _i<bool>(0xA64BC0, garage, p1, p2); }
	static bool is_heli_part_broken(Vehicle vehicle, bool p1, bool p2, bool p3) { return _i<bool>(0xB23F90, vehicle, p1, p2, p3); }
	static bool is_help_message_being_displayed() { return _i<bool>(0x9E1E70); }
	static bool is_help_message_fading_out() { return _i<bool>(0x9E1E90); }
	static bool is_help_message_on_screen() { return _i<bool>(0x9E1DF0); }
	static bool is_horn_active(Vehicle vehicle) { return _i<bool>(0x985600, vehicle); }
	static bool is_hud_component_active(int id) { return _i<bool>(0x9E3FE0, id); }
	static bool is_hud_hidden() { return _i<bool>(0x9E1170); }
	static bool is_hud_preference_switched_on() { return _i<bool>(0x9E1040); }
	static bool is_interior_capped(int interiorID) { return _i<bool>(0x9E91B0, interiorID); }
	static bool is_interior_disabled(int interiorID) { return _i<bool>(0x9E90C0, interiorID); }
	static bool is_interior_ready(int interiorID) { return _i<bool>(0x9E89A0, interiorID); }
	static bool is_interior_scene() { return _i<bool>(0x9E82C0); }
	static bool is_in_itemset(Any p0, Any p1) { return _i<bool>(0x9E98B0, p0, p1); }
	static bool is_ipl_active(const char* iplName) { return _i<bool>(0xAD9BD0, iplName); }
	static bool is_itemset_valid(Any p0) { return _i<bool>(0x9E96F0, p0); }
	static bool is_look_inverted() { return _i<bool>(0xA6B100); }
	static bool is_memory_card_in_use() { return _i<bool>(0x9F8220); }
	static bool is_message_being_displayed() { return _i<bool>(0x9E0E10); }
	static bool is_minigame_in_progress() { return _i<bool>(0x9F8650); }
	static bool is_mission_complete_playing() { return _i<bool>(0x986740); }
	static bool is_mission_creator_blip(Blip blip) { return _i<bool>(0x9E2AC0, blip); }
	static bool is_mobile_phone_call_ongoing() { return _i<bool>(0x982930); }
	static bool is_mobile_phone_radio_active() { return _i<bool>(0x9843B0); }
	static bool is_model_a_ped(uint32_t modelHash) { return _i<bool>(0xAD9560, modelHash); }
	static bool is_model_a_vehicle(uint32_t model) { return _i<bool>(0xAD95D0, model); }
	static bool is_model_in_cdimage(uint32_t model) { return _i<bool>(0xAD9480, model); }
	static bool is_model_valid(uint32_t model) { return _i<bool>(0xAD94F0, model); }
	static bool is_mounted_weapon_task_underneath_driving_task(Ped ped) { return _i<bool>(0xAF4710, ped); }
	static bool is_move_blend_ratio_running(Ped ped) { return _i<bool>(0xAF8260, ped); }
	static bool is_move_blend_ratio_sprinting(Ped ped) { return _i<bool>(0xAF82A0, ped); }
	static bool is_move_blend_ratio_still(Ped ped) { return _i<bool>(0xAF8200, ped); }
	static bool is_move_blend_ratio_walking(Ped ped) { return _i<bool>(0xAF8220, ped); }
	static bool is_mp_gamer_tag_active(int gamerTagId) { return _i<bool>(0x9E48B0, gamerTagId); }
	static bool is_named_rendertarget_linked(uint32_t hash) { return _i<bool>(0x9E1D10, hash); }
	static bool is_named_rendertarget_registered(const char* p0) { return _i<bool>(0x9E1BA0, p0); }
	static bool is_navmesh_loaded_in_area(float x1, float y1, float z1, float x2, float y2, float z2) { return _i<bool>(0xA70CB0, x1, y1, z1, x2, y2, z2); }
	static bool is_network_loading_scene() { return _i<bool>(0xAD9240); }
	static bool is_new_load_scene_active() { return _i<bool>(0xADA430); }
	static bool is_new_load_scene_loaded() { return _i<bool>(0xADA450); }
	static bool is_next_weather_type(const char* weatherType) { return _i<bool>(0x9F5C10, weatherType); }
	static bool is_object_near_point(uint32_t objectHash, float x, float y, float z, float range) { return _i<bool>(0xA65450, objectHash, x, y, z, range); }
	static bool is_object_visible(Object object) { return _i<bool>(0xA655B0, object); }
	static bool is_object_within_brain_activation_range(Object object) { return _i<bool>(0x988040, object); }
	static bool is_pause_menu_active() { return _i<bool>(0x9E5290); }
	static bool is_pause_menu_restarting() { return _i<bool>(0x9E5370); }
	static bool is_pc_version() { return _i<bool>(0x9F9160); }
	static bool is_pedheadshot_ready(int handle) { return _i<bool>(0xA9D5C0, handle); }
	static bool is_pedheadshot_valid(int handle) { return _i<bool>(0xA9D580, handle); }
	static bool is_ped_active_in_scenario(Ped ped) { return _i<bool>(0xAF5C70, ped); }
	static bool is_ped_aiming_from_cover(Ped ped) { return _i<bool>(0xA90230, ped); }
	static bool is_ped_a_player(Ped ped) { return _i<bool>(0xA902C0, ped); }
	static bool is_ped_being_arrested(Ped ped) { return _i<bool>(0xAF87F0, ped); }
	static bool is_ped_being_jacked(Ped ped) { return _i<bool>(0xA931F0, ped); }
	static bool is_ped_being_stealth_killed(Ped ped) { return _i<bool>(0xA9B9E0, ped); }
	static bool is_ped_being_stunned(Ped ped, int p1) { return _i<bool>(0xA93220, ped, p1); }
	static bool is_ped_climbing(Ped ped) { return _i<bool>(0xA92130, ped); }
	static bool is_ped_component_variation_valid(Ped ped, int componentId, int drawableId, int textureId) { return _i<bool>(0xA96020, ped, componentId, drawableId, textureId); }
	static bool is_ped_cuffed(Ped ped) { return _i<bool>(0xAF8850, ped); }
	static bool is_ped_current_weapon_silenced(Ped ped) { return _i<bool>(0xB3AE40, ped); }
	static bool is_ped_dead_or_dying(Ped ped, bool p1) { return _i<bool>(0xA90140, ped, p1); }
	static bool is_ped_diving(Ped ped) { return _i<bool>(0xA921F0, ped); }
	static bool is_ped_doing_driveby(Ped ped) { return _i<bool>(0xA93130, ped); }
	static bool is_ped_ducking(Ped ped) { return _i<bool>(0xA92630, ped); }
	static bool is_ped_evasive_diving(Ped ped, Entity* evadingEntity) { return _i<bool>(0xA9AA80, ped, evadingEntity); }
	static bool is_ped_facing_ped(Ped ped, Ped otherPed, float angle) { return _i<bool>(0xA90460, ped, otherPed, angle); }
	static bool is_ped_falling(Ped ped) { return _i<bool>(0xA91F90, ped); }
	static bool is_ped_fatally_injured(Ped ped) { return _i<bool>(0xA900E0, ped); }
	static bool is_ped_fleeing(Ped ped) { return _i<bool>(0xA93290, ped); }
	static bool is_ped_getting_into_a_vehicle(Ped ped) { return _i<bool>(0xA951F0, ped); }
	static bool is_ped_getting_up(Ped ped) { return _i<bool>(0xAF6150, ped); }
	static bool is_ped_going_into_cover(Ped ped) { return _i<bool>(0xA93350, ped); }
	static bool is_ped_group_member(Ped ped, int groupId) { return _i<bool>(0xA92EA0, ped, groupId); }
	static bool is_ped_hanging_on_to_vehicle(Ped ped) { return _i<bool>(0xA92ED0, ped); }
	static bool is_ped_headtracking_entity(Ped ped, Entity entity) { return _i<bool>(0xA9A6C0, ped, entity); }
	static bool is_ped_headtracking_ped(Ped ped1, Ped ped2) { return _i<bool>(0xA9A690, ped1, ped2); }
	static bool is_ped_human(Ped ped) { return _i<bool>(0xA90E00, ped); }
	static bool is_ped_hurt(Ped ped) { return _i<bool>(0xA90080, ped); }
	static bool is_ped_injured(Ped ped) { return _i<bool>(0xA90020, ped); }
	static bool is_ped_in_any_boat(Ped ped) { return _i<bool>(0xA91600, ped); }
	static bool is_ped_in_any_heli(Ped ped) { return _i<bool>(0xA916E0, ped); }
	static bool is_ped_in_any_plane(Ped ped) { return _i<bool>(0xA91750, ped); }
	static bool is_ped_in_any_police_vehicle(Ped ped) { return _i<bool>(0xA91E00, ped); }
	static bool is_ped_in_any_sub(Ped ped) { return _i<bool>(0xA91670, ped); }
	static bool is_ped_in_any_taxi(Ped ped) { return _i<bool>(0xA92690, ped); }
	static bool is_ped_in_any_train(Ped ped) { return _i<bool>(0xA95170, ped); }
	static bool is_ped_in_any_vehicle(Ped ped, bool atGetIn) { return _i<bool>(0xA8FF40, ped, atGetIn); }
	static bool is_ped_in_combat(Ped ped, Ped target) { return _i<bool>(0xA93020, ped, target); }
	static bool is_ped_in_cover(Ped ped, bool p1) { return _i<bool>(0xA932C0, ped, p1); }
	static bool is_ped_in_cover_facing_left(Ped ped) { return _i<bool>(0xA932F0, ped); }
	static bool is_ped_in_current_conversation(Ped ped) { return _i<bool>(0x9840E0, ped); }
	static bool is_ped_in_flying_vehicle(Ped ped) { return _i<bool>(0xA917C0, ped); }
	static bool is_ped_in_group(Ped ped) { return _i<bool>(0xA940E0, ped); }
	static bool is_ped_in_melee_combat(Ped ped) { return _i<bool>(0xA90490, ped); }
	static bool is_ped_in_model(Ped ped, uint32_t modelHash) { return _i<bool>(0xA8FE60, ped, modelHash); }
	static bool is_ped_in_parachute_free_fall(Ped ped) { return _i<bool>(0xA91ED0, ped); }
	static bool is_ped_in_vehicle(Ped ped, Vehicle vehicle, bool atGetIn) { return _i<bool>(0xA8FE30, ped, vehicle, atGetIn); }
	static bool is_ped_in_writhe(Ped ped) { return _i<bool>(0xAF61E0, ped); }
	static bool is_ped_jacking(Ped ped) { return _i<bool>(0xA931C0, ped); }
	static bool is_ped_jumping(Ped ped) { return _i<bool>(0xA91FF0, ped); }
	static bool is_ped_jumping_out_of_vehicle(Ped ped) { return _i<bool>(0xA92250, ped); }
	static bool is_ped_male(Ped ped) { return _i<bool>(0xA90D90, ped); }
	static bool is_ped_model(Ped ped, uint32_t modelHash) { return _i<bool>(0xA90770, ped, modelHash); }
	static bool is_ped_on_any_bike(Ped ped) { return _i<bool>(0xA914B0, ped); }
	static bool is_ped_on_foot(Ped ped) { return _i<bool>(0xA91480, ped); }
	static bool is_ped_on_mount(Ped ped) { return _i<bool>(0xA91050, ped); }
	static bool is_ped_on_specific_vehicle(Ped ped, Vehicle vehicle) { return _i<bool>(0xA91130, ped, vehicle); }
	static bool is_ped_on_vehicle(Ped ped) { return _i<bool>(0xA910D0, ped); }
	static bool is_ped_performing_stealth_kill(Ped ped) { return _i<bool>(0xA9B8B0, ped); }
	static bool is_ped_planting_bomb(Ped ped) { return _i<bool>(0xA91520, ped); }
	static bool is_ped_prone(Ped ped) { return _i<bool>(0xA92FC0, ped); }
	static bool is_ped_ragdoll(Ped ped) { return _i<bool>(0xA97F00, ped); }
	static bool is_ped_reloading(Ped ped) { return _i<bool>(0xA90260, ped); }
	static bool is_ped_responding_to_event(Ped ped, Any event) { return _i<bool>(0xA93D60, ped, event); }
	static bool is_ped_ringtone_playing(Ped ped) { return _i<bool>(0x982870, ped); }
	static bool is_ped_running(Ped ped) { return _i<bool>(0xAF8420, ped); }
	static bool is_ped_running_arrest_task(Ped ped) { return _i<bool>(0xAF8760, ped); }
	static bool is_ped_running_mobile_phone_task(Ped ped) { return _i<bool>(0xA98170, ped); }
	static bool is_ped_running_ragdoll_task(Ped ped) { return _i<bool>(0xA97F70, ped); }
	static bool is_ped_shooting(Ped ped) { return _i<bool>(0xA905E0, ped); }
	static bool is_ped_sitting_in_any_vehicle(Ped ped) { return _i<bool>(0xA913F0, ped); }
	static bool is_ped_sitting_in_vehicle(Ped ped, Vehicle vehicle) { return _i<bool>(0xA913C0, ped, vehicle); }
	static bool is_ped_sprinting(Ped ped) { return _i<bool>(0xAF84D0, ped); }
	static bool is_ped_still(Ped ped) { return _i<bool>(0xAF82C0, ped); }
	static bool is_ped_stopped(Ped ped) { return _i<bool>(0xA90500, ped); }
	static bool is_ped_strafing(Ped ped) { return _i<bool>(0xAF8570, ped); }
	static bool is_ped_swimming(Ped ped) { return _i<bool>(0xA94E60, ped); }
	static bool is_ped_swimming_under_water(Ped ped) { return _i<bool>(0xA94EC0, ped); }
	static bool is_ped_tracked(Ped ped) { return _i<bool>(0xA9C300, ped); }
	static bool is_ped_trying_to_enter_a_locked_vehicle(Ped ped) { return _i<bool>(0xA95220, ped); }
	static bool is_ped_using_action_mode(Ped ped) { return _i<bool>(0xA9D2E0, ped); }
	static bool is_ped_using_any_scenario(Ped ped) { return _i<bool>(0xA99B70, ped); }
	static bool is_ped_using_scenario(Ped ped, const char* scenario) { return _i<bool>(0xA99B40, ped, scenario); }
	static bool is_ped_vaulting(Ped ped) { return _i<bool>(0xA92190, ped); }
	static bool is_ped_walking(Ped ped) { return _i<bool>(0xAF8370, ped); }
	static bool is_ped_weapon_component_active(Ped ped, uint32_t weaponHash, uint32_t componentHash) { return _i<bool>(0xB3A040, ped, weaponHash, componentHash); }
	static bool is_ped_weapon_ready_to_shoot(Ped ped) { return _i<bool>(0xB38CE0, ped); }
	static bool is_ped_wearing_helmet(Ped ped) { return _i<bool>(0xA9B090, ped); }
	static bool is_playback_going_on_for_vehicle(Any p0) { return _i<bool>(0xB1FCC0, p0); }
	static bool is_playback_using_ai_going_on_for_vehicle(Any p0) { return _i<bool>(0xB1FD20, p0); }
	static bool is_player_being_arrested(Player player, bool atArresting) { return _i<bool>(0xAAAD40, player, atArresting); }
	static bool is_player_climbing(Player player) { return _i<bool>(0xAAAD00, player); }
	static bool is_player_control_on(Player player) { return _i<bool>(0xAAAC10, player); }
	static bool is_player_dead(Player player) { return _i<bool>(0xAA9BE0, player); }
	static bool is_player_free_aiming(Player player) { return _i<bool>(0xAAA630, player); }
	static bool is_player_free_aiming_at_entity(Player player, Entity entity) { return _i<bool>(0xAAA680, player, entity); }
	static bool is_player_free_for_ambient_task(Player player) { return _i<bool>(0xAAAF90, player); }
	static bool is_player_in_cutscene(Player player) { return _i<bool>(0xA531D0, player); }
	static bool is_player_logging_in_np() { return _i<bool>(0xAAB2E0); }
	static bool is_player_online() { return _i<bool>(0xAAB2B0); }
	static bool is_player_playing(Player player) { return _i<bool>(0xAA9D80, player); }
	static bool is_player_pressing_horn(Player player) { return _i<bool>(0xAA9C20, player); }
	static bool is_player_ready_for_cutscene(Player player) { return _i<bool>(0xAAA500, player); }
	static bool is_player_riding_train(Player player) { return _i<bool>(0xAAD5D0, player); }
	static bool is_player_script_control_on(Player player) { return _i<bool>(0xAAACB0, player); }
	static bool is_player_switch_in_progress() { return _i<bool>(0xADA5C0); }
	static bool is_player_targetting_anything(Player player) { return _i<bool>(0xAAA8F0, player); }
	static bool is_player_targetting_entity(Player player, Entity entity) { return _i<bool>(0xAAA530, player, entity); }
	static bool is_player_teleport_active() { return _i<bool>(0xAAC600); }
	static bool is_player_wanted_level_greater(Player player, int wantedLevel) { return _i<bool>(0xAA9AF0, player, wantedLevel); }
	static bool is_playing_phone_gesture_anim(Ped ped) { return _i<bool>(0xAF3510, ped); }
	static bool is_point_in_angled_area(float p0, float p1, float p2, float p3, float p4, float p5, float p6, float p7, float p8, float p9, bool p10, bool p11) { return _i<bool>(0xA65110, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11); }
	static bool is_point_on_road(float x, float y, float z, Vehicle vehicle) { return _i<bool>(0xA70A00, x, y, z, vehicle); }
	static bool is_position_occupied(float x, float y, float z, float range, bool p4, bool p5, bool p6, bool p7, bool p8, Any p9, bool p10) { return _i<bool>(0x9F7230, x, y, z, range, p4, p5, p6, p7, p8, p9, p10); }
	static bool is_prev_weather_type(const char* weatherType) { return _i<bool>(0x9F5BC0, weatherType); }
	static bool is_projectile_in_area(float x1, float y1, float z1, float x2, float y2, float z2, bool ownedByPlayer) { return _i<bool>(0x9F88B0, x1, y1, z1, x2, y2, z2, ownedByPlayer); }
	static bool is_projectile_type_in_angled_area(float p0, float p1, float p2, float p3, float p4, float p5, float p6, Any p7, bool p8) { return _i<bool>(0x9F8A10, p0, p1, p2, p3, p4, p5, p6, p7, p8); }
	static bool is_projectile_type_in_area(float x1, float y1, float z1, float x2, float y2, float z2, int type, bool p7) { return _i<bool>(0x9F89B0, x1, y1, z1, x2, y2, z2, type, p7); }
	static bool is_ps3_version() { return _i<bool>(0x9F9150); }
	static bool is_radar_hidden() { return _i<bool>(0x9E1190); }
	static bool is_radar_preference_switched_on() { return _i<bool>(0x9E1060); }
	static bool is_radio_retuning() { return _i<bool>(0x9844C0); }
	static bool is_scenario_group_enabled(const char* scenarioGroup) { return _i<bool>(0xAF5960, scenarioGroup); }
	static bool is_scenario_occupied(float p0, float p1, float p2, float p3, bool p4) { return _i<bool>(0xAF5830, p0, p1, p2, p3, p4); }
	static bool is_scenario_type_enabled(const char* scenarioType) { return _i<bool>(0xAF5B40, scenarioType); }
	static bool is_screen_faded_in() { return _i<bool>(0x994010); }
	static bool is_screen_faded_out() { return _i<bool>(0x993FF0); }
	static bool is_screen_fading_in() { return _i<bool>(0x994050); }
	static bool is_screen_fading_out() { return _i<bool>(0x994030); }
	static bool is_scripted_conversation_loaded() { return _i<bool>(0x982F60); }
	static bool is_scripted_conversation_ongoing() { return _i<bool>(0x982F20); }
	static bool is_scripted_hud_component_active(int id) { return _i<bool>(0x9E4030, id); }
	static bool is_scripted_scenario_ped_using_conditional_anim(Ped ped, const char* animDict, const char* anim) { return _i<bool>(0xA95920, ped, animDict, anim); }
	static bool is_scripted_speech_playing(Any p0) { return _i<bool>(0x983EA0, p0); }
	static bool is_script_global_shaking() { return _i<bool>(0x993960); }
	static bool is_shocking_event_in_sphere(int type, float x, float y, float z, float radius) { return _i<bool>(0x9B95B0, type, x, y, z, radius); }
	static bool is_sniper_bullet_in_area(float x1, float y1, float z1, float x2, float y2, float z2) { return _i<bool>(0x9F87F0, x1, y1, z1, x2, y2, z2); }
	static bool is_sniper_inverted() { return _i<bool>(0x9F86B0); }
	static bool is_social_club_active() { return _i<bool>(0x9E5C30); }
	static bool is_special_ability_active(Player player) { return _i<bool>(0xAAC1D0, player); }
	static bool is_special_ability_enabled(Player player) { return _i<bool>(0xAAC320, player); }
	static bool is_special_ability_meter_full(Player player) { return _i<bool>(0xAAC240, player); }
	static bool is_special_ability_unlocked(uint32_t playerModel) { return _i<bool>(0xAAC1A0, playerModel); }
	static bool is_sphere_visible(float x, float y, float z, float radius) { return _i<bool>(0x9949A0, x, y, z, radius); }
	static bool is_sphere_visible_to_another_machine(float p0, float p1, float p2, float p3) { return _i<bool>(0xA534F0, p0, p1, p2, p3); }
	static bool is_sphere_visible_to_player(Any p0, float p1, float p2, float p3, float p4) { return _i<bool>(0xA53590, p0, p1, p2, p3, p4); }
	static bool is_srl_loaded() { return _i<bool>(0xADAD40); }
	static bool is_streaming_additional_text(int p0) { return _i<bool>(0x9E0DB0, p0); }
	static bool is_stream_playing() { return _i<bool>(0x9856F0); }
	static bool is_string_null(const char* string) { return _i<bool>(0x9F9200, string); }
	static bool is_string_null_or_empty(const char* string) { return _i<bool>(0x9F9220, string); }
	static bool is_stunt_jump_in_progress() { return _i<bool>(0x9F95F0); }
	static bool is_stunt_jump_message_showing() { return _i<bool>(0x9F9620); }
	static bool is_subtitle_preference_switched_on() { return _i<bool>(0x9E1080); }
	static bool is_synchronized_scene_looped(int sceneID) { return _i<bool>(0xA9CDA0, sceneID); }
	static bool is_synchronized_scene_running(int sceneId) { return _i<bool>(0xA9CB40, sceneId); }
	static bool is_system_ui_being_displayed() { return _i<bool>(0xAAB300); }
	static bool is_taxi_light_on(Vehicle vehicle) { return _i<bool>(0xB1C7D0, vehicle); }
	static bool is_tennis_mode(Ped ped) { return _i<bool>(0x9FA140, ped); }
	static bool is_this_a_minigame_script() { return _i<bool>(0x9F8670); }
	static Any is_this_model_an_amphibious_car(uint32_t model) { return _i<Any>(0xB20B00, model); }
	static bool is_this_model_a_bicycle(uint32_t model) { return _i<bool>(0xB20A00, model); }
	static bool is_this_model_a_bike(uint32_t model) { return _i<bool>(0xB20980, model); }
	static bool is_this_model_a_boat(uint32_t model) { return _i<bool>(0xB20680, model); }
	static bool is_this_model_a_car(uint32_t model) { return _i<bool>(0xB20880, model); }
	static bool is_this_model_a_heli(uint32_t model) { return _i<bool>(0xB20800, model); }
	static bool is_this_model_a_plane(uint32_t model) { return _i<bool>(0xB20780, model); }
	static bool is_this_model_a_quadbike(uint32_t model) { return _i<bool>(0xB20A80, model); }
	static bool is_this_model_a_train(uint32_t model) { return _i<bool>(0xB20900, model); }
	static bool is_thread_active(int threadId) { return _i<bool>(0xAB1290, threadId); }
	static bool is_time_equal_to(int timeA, int timeB) { return _i<bool>(0xA54240, timeA, timeB); }
	static bool is_time_less_than(int timeA, int timeB) { return _i<bool>(0xA54200, timeA, timeB); }
	static bool is_time_more_than(int timeA, int timeB) { return _i<bool>(0xA54220, timeA, timeB); }
	static bool is_toggle_mod_on(Vehicle vehicle, int modType) { return _i<bool>(0xB27410, vehicle, modType); }
	static bool is_tracked_ped_visible(Ped ped) { return _i<bool>(0xA9C240, ped); }
	static bool is_tracked_point_visible(Object point) { return _i<bool>(0x9CDFD0, point); }
	static bool is_valid_interior(int interiorID) { return _i<bool>(0x9E82D0, interiorID); }
	static bool is_vehicle_alarm_activated(Vehicle vehicle) { return _i<bool>(0xB1EBA0, vehicle); }
	static bool is_vehicle_attached_to_cargobob(Vehicle cargobob, Vehicle vehicleAttached) { return _i<bool>(0xB298A0, cargobob, vehicleAttached); }
	static bool is_vehicle_attached_to_tow_truck(Vehicle towTruck, Vehicle vehicle) { return _i<bool>(0xB24C50, towTruck, vehicle); }
	static bool is_vehicle_attached_to_trailer(Vehicle vehicle) { return _i<bool>(0xB1EF90, vehicle); }
	static bool is_vehicle_a_convertible(Vehicle vehicle, bool p1) { return _i<bool>(0xB22EA0, vehicle, p1); }
	static bool is_vehicle_bumper_broken_off(Vehicle vehicle, bool front) { return _i<bool>(0xB21F20, vehicle, front); }
	static bool is_vehicle_door_damaged(Vehicle veh, int doorID) { return _i<bool>(0xB21D40, veh, doorID); }
	static bool is_vehicle_door_fully_open(Vehicle vehicle, int doorIndex) { return _i<bool>(0xB21320, vehicle, doorIndex); }
	static bool is_vehicle_driveable(Vehicle vehicle, bool isOnFireCheck) { return _i<bool>(0xB234D0, vehicle, isOnFireCheck); }
	static bool is_vehicle_extra_turned_on(Vehicle vehicle, int extraId) { return _i<bool>(0xB22B60, vehicle, extraId); }
	static bool is_vehicle_high_detail(Vehicle vehicle) { return _i<bool>(0xB24980, vehicle); }
	static bool is_vehicle_in_burnout(Vehicle vehicle) { return _i<bool>(0xB251D0, vehicle); }
	static bool is_vehicle_in_garage_area(const char* garageName, Vehicle vehicle) { return _i<bool>(0xB1C840, garageName, vehicle); }
	static bool is_vehicle_model(Vehicle vehicle, uint32_t model) { return _i<bool>(0xB1B330, vehicle, model); }
	static bool is_vehicle_node_id_valid(int vehicleNodeId) { return _i<bool>(0xA6FE00, vehicleNodeId); }
	static bool is_vehicle_on_all_wheels(Vehicle vehicle) { return _i<bool>(0xB22000, vehicle); }
	static bool is_vehicle_searchlight_on(Vehicle vehicle) { return _i<bool>(0xB26450, vehicle); }
	static bool is_vehicle_seat_free(Vehicle vehicle, int seatIndex, Any p2) { return _i<bool>(0xB1D9D0, vehicle, seatIndex, p2); }
	static bool is_vehicle_siren_on(Vehicle vehicle) { return _i<bool>(0xB1D790, vehicle); }
	static bool is_vehicle_stolen(Vehicle vehicle) { return _i<bool>(0xB29610, vehicle); }
	static bool is_vehicle_stopped(Vehicle vehicle) { return _i<bool>(0xB1BAA0, vehicle); }
	static bool is_vehicle_stopped_at_traffic_lights(Vehicle vehicle) { return _i<bool>(0xB23020, vehicle); }
	static bool is_vehicle_stuck_on_roof(Vehicle vehicle) { return _i<bool>(0xB1B9D0, vehicle); }
	static bool is_vehicle_stuck_timer_up(Vehicle vehicle, int p1, int p2) { return _i<bool>(0xB233F0, vehicle, p1, p2); }
	static bool is_vehicle_tyre_burst(Vehicle vehicle, int wheelID, bool completely) { return _i<bool>(0xB1DB40, vehicle, wheelID, completely); }
	static bool is_vehicle_visible(Vehicle vehicle) { return _i<bool>(0xB28EC0, vehicle); }
	static bool is_vehicle_window_intact(Vehicle vehicle, int windowIndex) { return _i<bool>(0xB23DC0, vehicle, windowIndex); }
	static bool is_warning_message_active() { return _i<bool>(0x9E4FF0); }
	static bool is_waypoint_active() { return _i<bool>(0x9E3160); }
	static bool is_waypoint_playback_going_on_for_ped(Any p0) { return _i<bool>(0xAF6B60, p0); }
	static bool is_waypoint_playback_going_on_for_vehicle(Any p0) { return _i<bool>(0xAF7530, p0); }
	static bool is_weapon_valid(uint32_t weaponHash) { return _i<bool>(0xB38C40, weaponHash); }
	static bool is_world_point_within_brain_activation_range() { return _i<bool>(0x9881A0); }
	static bool is_xbox360_version() { return _i<bool>(0x9F9140); }
	static Void knock_off_ped_prop(Ped ped, bool p1, bool p2, bool p3, bool p4) { return _i<Void>(0xA97480, ped, p1, p2, p3, p4); }
	static Void knock_ped_off_vehicle(Ped ped) { return _i<Void>(0xA94C40, ped); }
	static bool leaderboards2_read_by_handle(Any* p0, Any* p1) { return _i<bool>(0xAD1570, p0, p1); }
	static bool leaderboards2_read_by_radius(Any* p0, Any p1, Any* p2) { return _i<bool>(0xAD15D0, p0, p1, p2); }
	static bool leaderboards2_read_by_rank(Any* p0, Any p1, Any p2) { return _i<bool>(0xAD15A0, p0, p1, p2); }
	static bool leaderboards2_read_by_score_float(Any* p0, float p1, Any p2) { return _i<bool>(0xAD1630, p0, p1, p2); }
	static bool leaderboards2_read_by_score_int(Any* p0, Any p1, Any p2) { return _i<bool>(0xAD1600, p0, p1, p2); }
	static bool leaderboards2_read_friends_by_row(Any* p0, Any* p1, Any p2, bool p3, Any p4, Any p5) { return _i<bool>(0xAD1530, p0, p1, p2, p3, p4, p5); }
	static bool leaderboards2_write_data(Any* p0) { return _i<bool>(0xAD18C0, p0); }
	static bool leaderboards_cache_data_row(Any* p0) { return _i<bool>(0xAD1930, p0); }
	static Void leaderboards_clear_cache_data() { return _i<Void>(0xAD1960); }
	static bool leaderboards_get_cache_data_row(Any p0, Any p1, Any* p2) { return _i<bool>(0xAD1AA0, p0, p1, p2); }
	static bool leaderboards_get_cache_exists(Any p0) { return _i<bool>(0xAD19A0, p0); }
	static Any leaderboards_get_cache_time(Any p0) { return _i<Any>(0xAD1A00, p0); }
	static Any leaderboards_get_column_id(Any p0, Any p1, Any p2) { return _i<Any>(0xAD12C0, p0, p1, p2); }
	static Any leaderboards_get_column_type(Any p0, Any p1, Any p2) { return _i<Any>(0xAD1310, p0, p1, p2); }
	static Any leaderboards_get_number_of_columns(Any p0, Any p1) { return _i<Any>(0xAD1280, p0, p1); }
	static Any leaderboards_read_clear(Any p0, Any p1, Any p2) { return _i<Any>(0xAD13C0, p0, p1, p2); }
	static Any leaderboards_read_clear_all() { return _i<Any>(0xAD1380); }
	static bool leaderboards_read_pending(Any p0, Any p1, Any p2) { return _i<bool>(0xAD1430, p0, p1, p2); }
	static bool leaderboards_read_successful(Any p0, Any p1, Any p2) { return _i<bool>(0xAD14B0, p0, p1, p2); }
	static Void link_named_rendertarget(uint32_t hash) { return _i<Void>(0x9E1C20, hash); }
	static Void load_all_objects_now() { return _i<Void>(0xAD9170); }
	static int load_movie_mesh_set(const char* movieMeshSetName) { return _i<int>(0x9CDA40, movieMeshSetName); }
	static Void load_rope_data(Object rope, const char* rope_preset) { return _i<Void>(0xAA0910, rope, rope_preset); }
	static Void load_scene(float x, float y, float z) { return _i<Void>(0xAD9190, x, y, z); }
	static bool load_stream(const char* streamName, const char* soundSet) { return _i<bool>(0x985750, streamName, soundSet); }
	static Void lock_minimap_angle(int angle) { return _i<Void>(0x9E3EE0, angle); }
	static Void lock_minimap_position(float x, float y) { return _i<Void>(0x9E3F10, x, y); }
	static Void lower_convertible_roof(Vehicle vehicle, bool instantlyLower) { return _i<Void>(0xB22DA0, vehicle, instantlyLower); }
	static bool make_ped_reload(Ped ped) { return _i<bool>(0xB3A0D0, ped); }
	static Void modify_water(float x, float y, float radius, float height) { return _i<Void>(0xB2FD00, x, y, radius, height); }
	static Void move_vehicle_decals(Any p0, Any p1) { return _i<Void>(0x9D02D0, p0, p1); }
	static bool network_accept_presence_invite(Any p0) { return _i<bool>(0xA4CFB0, p0); }
	static bool network_access_tunable_bool(const char* tunableContext, const char* tunableName) { return _i<bool>(0xA56320, tunableContext, tunableName); }
	static bool network_access_tunable_bool_hash(uint32_t tunableContext, uint32_t tunableName) { return _i<bool>(0xA56580, tunableContext, tunableName); }
	static bool network_access_tunable_float(const char* tunableContext, const char* tunableName, float* value) { return _i<bool>(0xA562A0, tunableContext, tunableName, value); }
	static bool network_access_tunable_float_hash(uint32_t tunableContext, uint32_t tunableName, float* value) { return _i<bool>(0xA564E0, tunableContext, tunableName, value); }
	static bool network_access_tunable_int(const char* tunableContext, const char* tunableName, int* value) { return _i<bool>(0xA56220, tunableContext, tunableName, value); }
	static bool network_access_tunable_int_hash(uint32_t tunableContext, uint32_t tunableName, int* value) { return _i<bool>(0xA56440, tunableContext, tunableName, value); }
	static Any network_action_follow_invite() { return _i<Any>(0xA4D240); }
	// [aligned] static Any network_add_entity_angled_area(float p0, float p1, float p2, float p3, float p4, float p5, float p6) { return _i<Any>(0xA55B20, p0, p1, p2, p3, p4, p5, p6); }
	static Any network_add_entity_area(float p0, float p1, float p2, float p3, float p4, float p5) { return _i<Any>(0xA55AF0, p0, p1, p2, p3, p4, p5); }
	static Void network_add_entity_to_synchronised_scene(Entity entity, int netScene, const char* animDict, const char* animName, float speed, float speedMulitiplier, int flag) { return _i<Void>(0xA55020, entity, netScene, animDict, animName, speed, speedMulitiplier, flag); }
	static Void network_add_followers(int* p0, int p1) { return _i<Void>(0xA4AEC0, p0, p1); }
	static bool network_add_friend(Any* p0, const char* p1) { return _i<bool>(0xA50A10, p0, p1); }
	static Void network_add_ped_to_synchronised_scene(Ped ped, int netScene, char* animDict, char* animName, float blendInSpeed, float blendOutSpeed, int duration, int flag, float playbackRate, Any p9) { return _i<Void>(0xA54E60, ped, netScene, animDict, animName, blendInSpeed, blendOutSpeed, duration, flag, playbackRate, p9); }
	static bool network_am_i_blocked_by_gamer(Any* p0) { return _i<bool>(0xA51390, p0); }
	static bool network_am_i_blocked_by_player(Player player) { return _i<bool>(0xA519E0, player); }
	static bool network_am_i_muted_by_gamer(Any* p0) { return _i<bool>(0xA51270, p0); }
	static bool network_am_i_muted_by_player(Player player) { return _i<bool>(0xA51920, player); }
	static Void network_apply_transition_parameter(Any p0, Any p1) { return _i<Void>(0xA4C420, p0, p1); }
	static bool network_are_handles_the_same(int* netHandle1, int* netHandle2) { return _i<bool>(0xA50190, netHandle1, netHandle2); }
	static bool network_are_transition_details_valid(Any p0) { return _i<bool>(0xA4CA90, p0); }
	static Void network_attach_synchronised_scene_to_entity(int netScene, Entity entity, int bone) { return _i<Void>(0xA551E0, netScene, entity, bone); }
	static Void network_bail(Any p0, Any p1, Any p2) { return _i<Void>(0xA4A480, p0, p1, p2); }
	static Void network_bail_transition(Any p0, Any p1, Any p2) { return _i<Void>(0xA4C280, p0, p1, p2); }
	static Void network_block_invites(bool toggle) { return _i<Void>(0xA4B4F0, toggle); }
	static Void network_buy_airstrike(int cost, bool p1, bool p2) { return _i<Void>(0xA1F590, cost, p1, p2); }
	static Void network_buy_bounty(int amount, Player victim, bool p2, bool p3) { return _i<Void>(0xA1F9A0, amount, victim, p2, p3); }
	static Void network_buy_fairground_ride(int amountSpent, Any p1, bool p2, bool p3) { return _i<Void>(0xA213A0, amountSpent, p1, p2, p3); }
	static Void network_buy_healthcare(int cost, bool p1, bool p2) { return _i<Void>(0xA1F4A0, cost, p1, p2); }
	static Void network_buy_heli_strike(int cost, bool p1, bool p2) { return _i<Void>(0xA1F7A0, cost, p1, p2); }
	static Void network_buy_item(Ped player, uint32_t item, Any p2, Any p3, bool p4, const char* item_name, Any p6, Any p7, Any p8, bool p9) { return _i<Void>(0xA1F0C0, player, item, p2, p3, p4, item_name, p6, p7, p8, p9); }
	static Void network_buy_property(float propertyCost, uint32_t propertyName, bool p2, bool p3) { return _i<Void>(0xA1F9D0, propertyCost, propertyName, p2, p3); }
	static bool network_can_access_multiplayer(int* loadingState) { return _i<bool>(0xA4A5B0, loadingState); }
	static bool network_can_bail() { return _i<bool>(0xA4A460); }
	static bool network_can_bet(Any p0) { return _i<bool>(0xA1D630, p0); }
	static bool network_can_communicate_with_gamer(int* player) { return _i<bool>(0xA51150, player); }
	static bool network_can_enter_multiplayer() { return _i<bool>(0xA4A6C0); }
	static bool network_can_receive_player_cash(Any p0, Any p1, Any p2, Any p3) { return _i<bool>(0xA272D0, p0, p1, p2, p3); }
	static bool network_can_session_end() { return _i<bool>(0xA4B6F0); }
	static bool network_can_set_waypoint() { return _i<bool>(0xA50BA0); }
	static bool network_can_spend_money(Any p0, bool p1, bool p2, bool p3, Any p4) { return _i<bool>(0xA1EFD0, p0, p1, p2, p3, p4); }
	static Void network_change_transition_slots(Any p0, Any p1) { return _i<Void>(0xA4C990, p0, p1); }
	static bool network_clan_download_membership(int* network_handle) { return _i<bool>(0xA52450, network_handle); }
	static bool network_clan_download_membership_pending(Any* p0) { return _i<bool>(0xA52480, p0); }
	static bool network_clan_get_membership(int* p0, int* clanMembership, int p2) { return _i<bool>(0xA52590, p0, clanMembership, p2); }
	static int network_clan_get_membership_count(int* p0) { return _i<int>(0xA52530, p0); }
	static bool network_clan_get_membership_desc(int* memberDesc, int p1) { return _i<bool>(0xA52420, memberDesc, p1); }
	static bool network_clan_get_membership_valid(int* p0, Any p1) { return _i<bool>(0xA52560, p0, p1); }
	static bool network_clan_join(int clanDesc) { return _i<bool>(0xA525C0, clanDesc); }
	static bool network_clan_player_get_desc(int* clanDesc, int bufferSize, int* network_handle) { return _i<bool>(0xA52300, clanDesc, bufferSize, network_handle); }
	static bool network_clan_player_is_active(int* network_handle) { return _i<bool>(0xA52250, network_handle); }
	static bool network_clan_remote_memberships_are_in_cache(int* p0) { return _i<bool>(0xA52500, p0); }
	static Void network_clear_character_wallet(Any p0) { return _i<Void>(0xA1D4C0, p0); }
	static Void network_clear_clock_time_override() { return _i<Void>(0xA55A50); }
	static Void network_clear_followers() { return _i<Void>(0xA4AED0); }
	static Any network_clear_follow_invite() { return _i<Any>(0xA4D270); }
	static Void network_clear_found_gamers() { return _i<Void>(0xA4B160); }
	static Void network_clear_transition_creator_handle() { return _i<Void>(0xA4C0B0); }
	static Void network_clear_voice_channel() { return _i<Void>(0xA51DB0); }
	static Void network_close_transition_matchmaking() { return _i<Void>(0xA4C890); }
	static int network_create_synchronised_scene(float x, float y, float z, float xRot, float yRot, float zRot, int p6, int p7, int p8, float p9, Any p10, float p11) { return _i<int>(0xA54E10, x, y, z, xRot, yRot, zRot, p6, p7, p8, p9, p10, p11); }
	static Void network_delete_character(int characterIndex, bool p1, bool p2) { return _i<Void>(0xA1D3F0, characterIndex, p1, p2); }
	static Void network_disable_invincible_flashing(int player, bool p1) { return _i<Void>(0xA4F190, player, p1); }
	static bool network_does_entity_exist_with_network_id(Entity entity) { return _i<bool>(0xA4F600, entity); }
	static bool network_does_network_id_exist(int netID) { return _i<bool>(0xA4F5C0, netID); }
	static bool network_does_tunable_exist(const char* tunableContext, const char* tunableName) { return _i<bool>(0xA561B0, tunableContext, tunableName); }
	// [aligned] static bool network_does_tunable_exist_hash(uint32_t tunbaleContext, uint32_t tunableName) { return _i<bool>(0xA563C0, tunbaleContext, tunableName); }
	static bool network_do_transition_quickmatch(Any p0, Any p1, Any p2, Any p3, Any p4, Any p5) { return _i<bool>(0xA4BD00, p0, p1, p2, p3, p4, p5); }
	static bool network_do_transition_quickmatch_async(Any p0, Any p1, Any p2, Any p3, Any p4, Any p5) { return _i<bool>(0xA4BDA0, p0, p1, p2, p3, p4, p5); }
	static bool network_do_transition_quickmatch_with_group(Any p0, Any p1, Any p2, Any p3, Any* p4, Any p5, Any p6, Any p7) { return _i<bool>(0xA4BE40, p0, p1, p2, p3, p4, p5, p6, p7); }
	static bool network_do_transition_to_freemode(Any* p0, Any p1, bool p2, int players, bool p4) { return _i<bool>(0xA4C320, p0, p1, p2, players, p4); }
	static bool network_do_transition_to_game(bool p0, int maxPlayers) { return _i<bool>(0xA4C2B0, p0, maxPlayers); }
	static bool network_do_transition_to_new_freemode(Any* p0, Any p1, int players, bool p3, bool p4, bool p5) { return _i<bool>(0xA4C360, p0, p1, players, p3, p4, p5); }
	static bool network_do_transition_to_new_game(bool p0, int maxPlayers, bool p2) { return _i<bool>(0xA4C2E0, p0, maxPlayers, p2); }
	static Void network_earn_from_ai_target_kill(Any p0, Any p1) { return _i<Void>(0xA1DF10, p0, p1); }
	// [aligned] static Void network_earn_from_ambient_job(int p0, const char* p1, Any* p2) { return _i<Void>(0xA1E160, p0, p1, p2); }
	// [aligned] static Void network_earn_from_bend_job(int amount, const char* heistHash) { return _i<Void>(0xA1DD10, amount, heistHash); }
	static Void network_earn_from_bounty(int amount, int* network_handle, Any* p2, Any p3) { return _i<Void>(0xA1DD50, amount, network_handle, p2, p3); }
	static Void network_earn_from_challenge_win(Any p0, Any* p1, bool p2) { return _i<Void>(0xA1DD30, p0, p1, p2); }
	static Void network_earn_from_crate_drop(int amount) { return _i<Void>(0xA1DC20, amount); }
	// [aligned] static Void network_earn_from_daily_objectives(int p0, const char* p1, int p2) { return _i<Void>(0xA1E140, p0, p1, p2); }
	static Void network_earn_from_gangattack_pickup(int amount) { return _i<Void>(0xA1D9E0, amount); }
	static Void network_earn_from_holdups(int amount) { return _i<Void>(0xA1DD80, amount); }
	static Void network_earn_from_import_export(Any p0, Any p1) { return _i<Void>(0xA1DD70, p0, p1); }
	static Void network_earn_from_job(int amount, const char* p1) { return _i<Void>(0xA1DCF0, amount, p1); }
	// [aligned] static Void network_earn_from_job_bonus(Any p0, Any* p1, Any* p2) { return _i<Void>(0xA1E180, p0, p1, p2); }
	static Void network_earn_from_personal_vehicle(Any p0, Any p1, Any p2, Any p3, Any p4, Any p5, Any p6, Any p7, Any p8) { return _i<Void>(0xA1E0E0, p0, p1, p2, p3, p4, p5, p6, p7, p8); }
	static Void network_earn_from_pickup(int amount) { return _i<Void>(0xA1D860, amount); }
	static Void network_earn_from_property(int amount, uint32_t propertyName) { return _i<Void>(0xA1DE40, amount, propertyName); }
	static Void network_earn_from_rockstar(int amount) { return _i<Void>(0xA1DFD0, amount); }
	static Void network_earn_from_vehicle(Any p0, Any p1, Any p2, Any p3, Any p4, Any p5, Any p6, Any p7) { return _i<Void>(0xA1E090, p0, p1, p2, p3, p4, p5, p6, p7); }
	static Void network_end_tutorial_session() { return _i<Void>(0xA556D0); }
	static Any network_explode_vehicle(Vehicle vehicle, bool isAudible, bool isInvisible, bool p3) { return _i<Any>(0xA56A00, vehicle, isAudible, isInvisible, p3); }
	static Void network_fade_in_entity(Entity entity, bool state, Any p2) { return _i<Void>(0xA530B0, entity, state, p2); }
	static Void network_fade_out_entity(Entity entity, bool normal, bool slow) { return _i<Void>(0xA53090, entity, normal, slow); }
	static Void network_force_local_use_of_synced_scene_camera(int netScene, const char* animDict, const char* animName) { return _i<Void>(0xA553C0, netScene, animDict, animName); }
	static bool network_gamertag_from_handle_pending() { return _i<bool>(0xA4FEB0); }
	static bool network_gamertag_from_handle_start(int* network_handle) { return _i<bool>(0xA4FEA0, network_handle); }
	static bool network_gamertag_from_handle_succeeded() { return _i<bool>(0xA4FEC0); }
	static bool network_gamer_has_headset(Any* p0) { return _i<bool>(0xA50F60, p0); }
	static int network_get_activity_player_num(bool p0) { return _i<int>(0xA4BB60, p0); }
	static bool network_get_currently_selected_gamer_handle_from_invite_menu(Any* p0) { return _i<bool>(0xA4D540, p0); }
	static int network_get_destroyer_of_network_id(int netId, uint32_t* weaponHash) { return _i<int>(0xA4EEE0, netId, weaponHash); }
	static Entity network_get_entity_from_network_id(int netId) { return _i<Entity>(0xA4F340, netId); }
	static bool network_get_entity_is_local(Entity entity) { return _i<bool>(0xA4F3E0, entity); }
	static bool network_get_entity_is_networked(Entity entity) { return _i<bool>(0xA4F380, entity); }
	static Entity network_get_entity_killer_of_player(Player player, uint32_t* weaponHash) { return _i<Entity>(0xA4EFA0, player, weaponHash); }
	static bool network_get_found_gamer(Any* p0, Any p1) { return _i<bool>(0xA4B0A0, p0, p1); }
	static int network_get_friend_count() { return _i<int>(0xA50750); }
	static const char* network_get_friend_name(Player player) { return _i<const char*>(0xA50760, player); }
	static const char* network_get_gamertag_from_handle(int* network_handle) { return _i<const char*>(0xA4FED0, network_handle); }
	static int network_get_host_of_script(const char* scriptName, int p1, int p2) { return _i<int>(0xA4E8B0, scriptName, p1, p2); }
	static Any network_get_host_of_this_script() { return _i<Any>(0xA4E870); }
	static Void network_get_local_handle(int* network_handle, int bufferSize) { return _i<Void>(0xA4FB70, network_handle, bufferSize); }
	static int network_get_max_friends() { return _i<int>(0xA50740); }
	static int network_get_network_id_from_entity(Entity entity) { return _i<int>(0xA4F2E0, entity); }
	static int network_get_num_connected_players() { return _i<int>(0xA4E6F0); }
	static int network_get_num_found_gamers() { return _i<int>(0xA4B080); }
	static int network_get_num_participants() { return _i<int>(0xA4E110); }
	static int network_get_num_presence_invites() { return _i<int>(0xA4CF80); }
	static int network_get_num_script_participants(Any* p0, Any p1, Any p2) { return _i<int>(0xA4EBD0, p0, p1, p2); }
	static int network_get_participant_index(int index) { return _i<int>(0xA4E5E0, index); }
	static Player network_get_player_from_gamer_handle(int* network_handle) { return _i<Player>(0xA50270, network_handle); }
	static int network_get_player_index(Player player) { return _i<int>(0xA4E510, player); }
	static Player network_get_player_index_from_ped(Ped ped) { return _i<Player>(0xA4E660, ped); }
	static float network_get_player_loudness(Any p0) { return _i<float>(0xA51A40, p0); }
	static bool network_get_presence_invite_handle(Any p0, Any* p1) { return _i<bool>(0xA4D070, p0, p1); }
	static Any network_get_presence_invite_id(Any p0) { return _i<Any>(0xA4D010, p0); }
	static Any network_get_presence_invite_inviter(Any p0) { return _i<Any>(0xA4D040, p0); }
	static Any network_get_presence_invite_session_id(Any p0) { return _i<Any>(0xA4D100, p0); }
	static Void network_get_primary_clan_data_cancel() { return _i<Void>(0xA528A0); }
	static Any network_get_primary_clan_data_clear() { return _i<Any>(0xA52880); }
	static bool network_get_primary_clan_data_new(Any* p0, Any* p1) { return _i<bool>(0xA52930, p0, p1); }
	static Any network_get_primary_clan_data_pending() { return _i<Any>(0xA528F0); }
	static bool network_get_primary_clan_data_start(Any* p0, Any p1) { return _i<bool>(0xA528C0, p0, p1); }
	static Any network_get_primary_clan_data_success() { return _i<Any>(0xA52910); }
	static int network_get_random_int() { return _i<int>(0xA4DB30); }
	static int network_get_script_status() { return _i<int>(0xA4E150); }
	static Any network_get_talker_proximity() { return _i<Any>(0xA51AC0); }
	static bool network_get_this_script_is_network_script() { return _i<bool>(0xA4E0A0); }
	static int network_get_timeout_time() { return _i<int>(0xA52070); }
	static bool network_get_transition_host(int* network_handle) { return _i<bool>(0xA4C6C0, network_handle); }
	static Any network_get_transition_members(Any* p0, Any p1) { return _i<Any>(0xA4C3F0, p0, p1); }
	static int64_t network_get_vc_balance() { return _i<int>(0xA27020); }
	static int64_t network_get_vc_bank_balance() { return _i<int>(0xA26FC0); }
	static int64_t network_get_vc_wallet_balance(int character) { return _i<int>(0xA26FF0, character); }
	static Void network_give_player_jobshare_cash(int amount, int* network_handle) { return _i<Void>(0xA1D510, amount, network_handle); }
	static Void network_handle_from_friend(Player friendIndex, int* network_handle, int bufferSize) { return _i<Void>(0xA4FDF0, friendIndex, network_handle, bufferSize); }
	static Void network_handle_from_member_id(const char* memberId, int* network_handle, int bufferSize) { return _i<Void>(0xA4FC30, memberId, network_handle, bufferSize); }
	static Void network_handle_from_player(Player player, int* network_handle, int bufferSize) { return _i<Void>(0xA4FCB0, player, network_handle, bufferSize); }
	static Void network_handle_from_user_id(const char* userId, int* network_handle, int bufferSize) { return _i<Void>(0xA4FBB0, userId, network_handle, bufferSize); }
	static uint32_t network_hash_from_gamer_handle(int* network_handle) { return _i<uint32_t>(0xA4FD40, network_handle); }
	static uint32_t network_hash_from_player_handle(Player player) { return _i<uint32_t>(0xA4FD20, player); }
	static bool network_has_control_of_door(int doorID) { return _i<bool>(0xA4F8C0, doorID); }
	static bool network_has_control_of_entity(Entity entity) { return _i<bool>(0xA4F820, entity); }
	static bool network_has_control_of_network_id(int netId) { return _i<bool>(0xA4F6B0, netId); }
	static bool network_has_control_of_pickup(Pickup pickup) { return _i<bool>(0xA4F890, pickup); }
	static bool network_has_follow_invite() { return _i<bool>(0xA4D210); }
	static bool network_has_headset() { return _i<bool>(0xA50EE0); }
	static bool network_has_invited_gamer(Any* p0) { return _i<bool>(0xA4D380, p0); }
	static bool network_has_invited_gamer_to_transition(Any* p0) { return _i<bool>(0xA4CB50, p0); }
	static bool network_has_pending_invite() { return _i<bool>(0xA4B380); }
	static bool network_has_player_started_transition(Player player) { return _i<bool>(0xA4CA40, player); }
	static bool network_has_ros_privilege(int p0) { return _i<bool>(0xA59530, p0); }
	static bool network_have_online_privileges() { return _i<bool>(0xA4A030); }
	static Any network_host_transition(Any p0, Any p1, Any p2, Any p3, Any p4, Any p5, Any p6, Any p7, Any p8, Any p9) { return _i<Any>(0xA4BC30, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9); }
	static Void network_initialize_cash(int p0, int p1) { return _i<Void>(0xA1D3E0, p0, p1); }
	static bool network_invite_gamers(Any* p0, Any p1, Any* p2, Any* p3) { return _i<bool>(0xA4D350, p0, p1, p2, p3); }
	static bool network_invite_gamers_to_transition(Any* p0, Any p1) { return _i<bool>(0xA4C110, p0, p1); }
	static bool network_is_activity_session() { return _i<bool>(0xA4CC70); }
	static Any network_is_activity_spectator() { return _i<Any>(0xA4BB00); }
	static bool network_is_activity_spectator_from_handle(int* network_handle) { return _i<bool>(0xA4BBA0, network_handle); }
	static Any network_is_adding_friend() { return _i<Any>(0xA509F0); }
	static bool network_is_cable_connected() { return _i<bool>(0xA592F0); }
	static bool network_is_chatting_in_platform_party(int* network_handle) { return _i<bool>(0xA4D9E0, network_handle); }
	static bool network_is_clock_time_overridden() { return _i<bool>(0xA55AD0); }
	static bool network_is_cloud_available() { return _i<bool>(0xA49F40); }
	static bool network_is_finding_gamers() { return _i<bool>(0xA4B040); }
	static bool network_is_friend(int* network_handle) { return _i<bool>(0xA50950, network_handle); }
	static bool network_is_friend_index_online(int friendIndex) { return _i<bool>(0xA50AA0, friendIndex); }
	static bool network_is_friend_in_multiplayer(const char* friendName) { return _i<bool>(0xA50900, friendName); }
	static bool network_is_friend_in_same_title(const char* friendName) { return _i<bool>(0xA50880, friendName); }
	static bool network_is_friend_online(const char* name) { return _i<bool>(0xA50800, name); }
	static bool network_is_gamer_blocked_by_me(Any* p0) { return _i<bool>(0xA51300, p0); }
	static bool network_is_gamer_in_my_session(int* network_handle) { return _i<bool>(0xA50330, network_handle); }
	static bool network_is_gamer_muted_by_me(int* p0) { return _i<bool>(0xA511E0, p0); }
	static bool network_is_gamer_talking(int* p0) { return _i<bool>(0xA50FF0, p0); }
	static bool network_is_game_in_progress() { return _i<bool>(0xA4B650); }
	static bool network_is_handle_valid(int* network_handle, int bufferSize) { return _i<bool>(0xA50240, network_handle, bufferSize); }
	static bool network_is_host() { return _i<bool>(0xA49FD0); }
	static bool network_is_host_of_this_script() { return _i<bool>(0xA4E840); }
	static bool network_is_inactive_profile(Any* p0) { return _i<bool>(0xA50680, p0); }
	static bool network_is_in_mp_cutscene() { return _i<bool>(0xA54650); }
	static bool network_is_in_session() { return _i<bool>(0xA4B680); }
	static bool network_is_in_spectator_mode() { return _i<bool>(0xA54600); }
	static bool network_is_in_transition() { return _i<bool>(0xA4C750); }
	static Any network_is_in_tutorial_session() { return _i<Any>(0xA55700); }
	static bool network_is_local_player_invincible() { return _i<bool>(0xA4F150); }
	static bool network_is_multiplayer_disabled() { return _i<bool>(0xA4A6A0); }
	static bool network_is_participant_active(int p0) { return _i<bool>(0xA4E790, p0); }
	static Any network_is_pending_friend(Any p0) { return _i<Any>(0xA509E0, p0); }
	static bool network_is_player_active(Player player) { return _i<bool>(0xA4E7D0, player); }
	static bool network_is_player_a_participant(Any p0) { return _i<bool>(0xA4E800, p0); }
	static bool network_is_player_blocked_by_me(Player player) { return _i<bool>(0xA51980, player); }
	static bool network_is_player_connected(Player player) { return _i<bool>(0xA4E710, player); }
	static bool network_is_player_in_mp_cutscene(Player player) { return _i<bool>(0xA54660, player); }
	static bool network_is_player_muted_by_me(Player player) { return _i<bool>(0xA518C0, player); }
	static bool network_is_player_talking(Player player) { return _i<bool>(0xA51800, player); }
	static bool network_is_script_active(const char* scriptName, Any p1, bool p2, Any p3) { return _i<bool>(0xA4E9B0, scriptName, p1, p2, p3); }
	static bool network_is_session_active() { return _i<bool>(0xA4B660); }
	static bool network_is_session_busy() { return _i<bool>(0xA4B6D0); }
	static bool network_is_session_started() { return _i<bool>(0xA4B6B0); }
	static bool network_is_signed_in() { return _i<bool>(0xA49D50); }
	static bool network_is_signed_online() { return _i<bool>(0xA49D70); }
	static Any network_is_transition_busy() { return _i<Any>(0xA4C790); }
	static Any network_is_transition_host() { return _i<Any>(0xA4C5E0); }
	static bool network_is_transition_host_from_handle(int* network_handle) { return _i<bool>(0xA4C610, network_handle); }
	static Any network_is_transition_matchmaking() { return _i<Any>(0xA4C7B0); }
	static bool network_is_transition_started() { return _i<bool>(0xA4C770); }
	static Any network_is_transition_to_game() { return _i<Any>(0xA4C3B0); }
	static Any network_join_group_activity() { return _i<Any>(0xA4BE90); }
	static bool network_join_transition(Player player) { return _i<bool>(0xA4CAF0, player); }
	static Any network_launch_transition() { return _i<Any>(0xA4C210); }
	static Any network_leave_transition() { return _i<Any>(0xA4C1D0); }
	static bool network_mark_transition_gamer_as_fully_joined(Any* p0) { return _i<bool>(0xA4C540, p0); }
	static const char* network_member_id_from_gamer_handle(int* network_handle) { return _i<const char*>(0xA502A0, network_handle); }
	static bool network_money_can_bet(Any p0, bool p1, bool p2) { return _i<bool>(0xA1D5F0, p0, p1, p2); }
	static Void network_open_transition_matchmaking() { return _i<Void>(0xA4C870); }
	static Void network_override_chat_restrictions(Player player, bool toggle) { return _i<Void>(0xA51C60, player, toggle); }
	static Void network_override_receive_restrictions(Player player, bool toggle) { return _i<Void>(0xA51D10, player, toggle); }
	// [aligned] static Void network_override_send_restrictions(Player player, bool toggle) { return _i<Void>(0xA51CC0, player, toggle); }
	static Void network_override_transition_chat(bool p0) { return _i<Void>(0xA51B30, p0); }
	static Void network_pay_employee_wage(Any p0, bool p1, bool p2) { return _i<Void>(0xA1F230, p0, p1, p2); }
	static Void network_pay_match_entry_fee(int value, int* p1, bool p2, bool p3) { return _i<Void>(0xA1F320, value, p1, p2, p3); }
	static const char* network_player_get_name(Player player) { return _i<const char*>(0xA50430, player); }
	static const char* network_player_get_userid(Player player, char* buffer) { return _i<const char*>(0xA504B0, player, buffer); }
	static bool network_player_has_headset(Player player) { return _i<bool>(0xA51860, player); }
	static int network_player_id_to_int() { return _i<int>(0xAAB040); }
	static bool network_player_is_rockstar_dev(Player player) { return _i<bool>(0xA504E0, player); }
	static Void network_receive_player_jobshare_cash(int value, int* network_handle) { return _i<Void>(0xA1D520, value, network_handle); }
	static Void network_refund_cash(int index, const char* context, const char* reason, bool unk) { return _i<Void>(0xA1D590, index, context, reason, unk); }
	static Void network_register_entity_as_networked(Entity entity) { return _i<Void>(0xA4F450, entity); }
	static Void network_register_host_broadcast_variables(int* vars, int numVars) { return _i<Void>(0xA4E170, vars, numVars); }
	static Void network_register_player_broadcast_variables(int* vars, int numVars) { return _i<Void>(0xA4E190, vars, numVars); }
	static bool network_remove_entity_area(Any p0) { return _i<bool>(0xA55BB0, p0); }
	static bool network_remove_presence_invite(Any p0) { return _i<bool>(0xA4CFE0, p0); }
	static Void network_remove_transition_invite(Any* p0) { return _i<Void>(0xA4D2B0, p0); }
	// [aligned] static bool network_request_cloud_background_scripts() { return _i<bool>(0xA55F20); }
	static Void network_request_cloud_tunables() { return _i<Void>(0xA55EC0); }
	static bool network_request_control_of_door(int doorID) { return _i<bool>(0xA4F7B0, doorID); }
	static bool network_request_control_of_entity(Entity entity) { return _i<bool>(0xA4F780, entity); }
	static bool network_request_control_of_network_id(int netId) { return _i<bool>(0xA4F640, netId); }
	static Void network_reset_body_tracker() { return _i<Void>(0xA56700); }
	static Void network_resurrect_local_player(float x, float y, float z, float heading, bool unk, bool changetime, Any p6) { return _i<Void>(0xA4F0E0, x, y, z, heading, unk, changetime, p6); }
	static bool network_send_text_message(const char* message, int* network_handle) { return _i<bool>(0xA4BA50, message, network_handle); }
	static bool network_send_transition_gamer_instruction(int* network_handle, const char* p1, int p2, int p3, bool p4) { return _i<bool>(0xA4C470, network_handle, p1, p2, p3, p4); }
	static Void network_session_block_join_requests(bool p0) { return _i<Void>(0xA4B7E0, p0); }
	static Void network_session_cancel_invite() { return _i<Void>(0xA4B320); }
	static Void network_session_change_slots(int p0, bool p1) { return _i<Void>(0xA4B830, p0, p1); }
	static bool network_session_end(bool p0, bool p1) { return _i<bool>(0xA4AAF0, p0, p1); }
	static Void network_session_force_cancel_invite() { return _i<Void>(0xA4B350); }
	static Void network_session_get_inviter(int* network_handle) { return _i<Void>(0xA4B440, network_handle); }
	static bool network_session_host(int p0, int maxPlayers, bool p2) { return _i<bool>(0xA4A910, p0, maxPlayers, p2); }
	static bool network_session_host_closed(int p0, int maxPlayers) { return _i<bool>(0xA4A970, p0, maxPlayers); }
	static bool network_session_host_friends_only(int p0, int maxPlayers) { return _i<bool>(0xA4A9D0, p0, maxPlayers); }
	static Void network_session_host_single_player(int p0) { return _i<Void>(0xA4B610, p0); }
	static bool network_session_is_closed_crew() { return _i<bool>(0xA4AA60); }
	static bool network_session_is_closed_friends() { return _i<bool>(0xA4AA30); }
	static bool network_session_is_private() { return _i<bool>(0xA4AAC0); }
	static bool network_session_is_solo() { return _i<bool>(0xA4AA90); }
	static Any network_session_is_visible() { return _i<Any>(0xA4B7A0); }
	static Void network_session_kick_player(Player player) { return _i<Void>(0xA4AB80, player); }
	static Void network_session_leave_single_player() { return _i<Void>(0xA4B630); }
	static Void network_session_mark_visible(bool p0) { return _i<Void>(0xA4B750, p0); }
	static Void network_session_voice_host() { return _i<Void>(0xA4B8A0); }
	static Void network_session_voice_leave() { return _i<Void>(0xA4B8D0); }
	static bool network_session_was_invited() { return _i<bool>(0xA4B420); }
	static Void network_set_activity_spectator(bool toggle) { return _i<Void>(0xA4BAE0, toggle); }
	static Void network_set_activity_spectator_max(int maxSpectators) { return _i<Void>(0xA4BB40, maxSpectators); }
	static bool network_set_currently_selected_gamer_handle_from_invite_menu(Any* p0) { return _i<bool>(0xA4D5A0, p0); }
	static Void network_set_entity_can_blend(Entity entity, bool toggle) { return _i<Void>(0xA529E0, entity, toggle); }
	static Void network_set_friendly_fire_option(bool toggle) { return _i<Void>(0xA52030, toggle); }
	static Void network_set_gamer_invited_to_transition(int* network_handle) { return _i<Void>(0xA4C140, network_handle); }
	static Void network_set_in_mp_cutscene(bool p0, bool p1) { return _i<Void>(0xA54620, p0, p1); }
	static Void network_set_in_spectator_mode(bool toggle, Ped playerPed) { return _i<Void>(0xA54480, toggle, playerPed); }
	static Void network_set_local_player_invincible_time(int time) { return _i<Void>(0xA4F110, time); }
	static Void network_set_mission_finished() { return _i<Void>(0xA4E980); }
	static Void network_set_override_spectator_mode(bool toggle) { return _i<Void>(0xA51BC0, toggle); }
	static Void network_set_property_id(Any p0) { return _i<Void>(0xA56C20, p0); }
	static Void network_set_rich_presence(Any p0, Any p1, Any p2, Any p3) { return _i<Void>(0xA52050, p0, p1, p2, p3); }
	static Void network_set_script_is_safe_for_network_game() { return _i<Void>(0x9F7850); }
	static Void network_set_talker_proximity(float p0) { return _i<Void>(0xA51AA0, p0); }
	static Void network_set_team_only_chat(bool toggle) { return _i<Void>(0xA51B50, toggle); }
	static Void network_set_this_script_is_network_script(int lobbySize, bool p1, int playerId) { return _i<Void>(0xA4DFA0, lobbySize, p1, playerId); }
	static Void network_set_transition_activity_id(Any p0) { return _i<Void>(0xA4C940, p0); }
	static Void network_set_transition_creator_handle(Any* p0) { return _i<Void>(0xA4C030, p0); }
	static Void network_set_voice_active(bool toggle) { return _i<Void>(0xA51AF0, toggle); }
	static Void network_set_voice_channel(Any p0) { return _i<Void>(0xA51D60, p0); }
	static Void network_show_profile_ui(int* network_handle) { return _i<Void>(0xA503C0, network_handle); }
	static Void network_spent_ammo_drop(Any p0, bool p1, bool p2) { return _i<Void>(0xA1F8A0, p0, p1, p2); }
	static Void network_spent_arrest_bail(Any p0, bool p1, bool p2) { return _i<Void>(0xA20AE0, p0, p1, p2); }
	static Any network_spent_bank_interest() { return _i<Any>(0xA20900); }
	static Void network_spent_betting(Any p0, Any p1, Any* p2, bool p3, bool p4) { return _i<Void>(0xA1F350, p0, p1, p2, p3, p4); }
	static Void network_spent_boat_pickup(Any p0, bool p1, bool p2) { return _i<Void>(0xA1FBF0, p0, p1, p2); }
	static Void network_spent_bounty(Any p0, bool p1, bool p2) { return _i<Void>(0xA20C30, p0, p1, p2); }
	static Void network_spent_bull_shark(Any p0, bool p1, bool p2) { return _i<Void>(0xA1FCF0, p0, p1, p2); }
	static Void network_spent_buy_offtheradar(Any p0, bool p1, bool p2) { return _i<Void>(0xA20210, p0, p1, p2); }
	static Void network_spent_buy_passive_mode(Any p0, bool p1, bool p2) { return _i<Void>(0xA20800, p0, p1, p2); }
	static Void network_spent_buy_reveal_players(Any p0, bool p1, bool p2) { return _i<Void>(0xA20310, p0, p1, p2); }
	static Void network_spent_buy_wantedlevel(Any p0, Any* p1, bool p2, bool p3) { return _i<Void>(0xA201E0, p0, p1, p2, p3); }
	static Void network_spent_call_player(Any p0, Any* p1, bool p2, bool p3) { return _i<Void>(0xA20C00, p0, p1, p2, p3); }
	static Void network_spent_carwash(Any p0, Any p1, Any p2, bool p3, bool p4) { return _i<Void>(0xA20410, p0, p1, p2, p3, p4); }
	static Void network_spent_cash_drop(int amount, bool p1, bool p2) { return _i<Void>(0xA1FDF0, amount, p1, p2); }
	static Void network_spent_cinema(Any p0, Any p1, bool p2, bool p3) { return _i<Void>(0xA20520, p0, p1, p2, p3); }
	static Void network_spent_from_rockstar(int bank, bool p1, bool p2) { return _i<Void>(0xA20D20, bank, p1, p2); }
	static Void network_spent_heli_pickup(Any p0, bool p1, bool p2) { return _i<Void>(0xA1FAF0, p0, p1, p2); }
	static Void network_spent_hire_mercenary(Any p0, bool p1, bool p2) { return _i<Void>(0xA200E0, p0, p1, p2); }
	static Void network_spent_hire_mugger(Any p0, bool p1, bool p2) { return _i<Void>(0xA1FEE0, p0, p1, p2); }
	static Void network_spent_holdups(Any p0, bool p1, bool p2) { return _i<Void>(0xA20710, p0, p1, p2); }
	static Void network_spent_in_stripclub(Any p0, bool p1, Any p2, bool p3) { return _i<Void>(0xA1F3A0, p0, p1, p2, p3); }
	static Void network_spent_no_cops(Any p0, bool p1, bool p2) { return _i<Void>(0xA20F70, p0, p1, p2); }
	static Void network_spent_pay_vehicle_insurance_premium(int amount, uint32_t vehicleModel, int* network_handle, bool notBankrupt, bool hasTheMoney) { return _i<Void>(0xA20BD0, amount, vehicleModel, network_handle, notBankrupt, hasTheMoney); }
	static Void network_spent_player_healthcare(Any p0, Any p1, bool p2, bool p3) { return _i<Void>(0xA20F40, p0, p1, p2, p3); }
	static Void network_spent_prostitutes(Any p0, bool p1, bool p2) { return _i<Void>(0xA209F0, p0, p1, p2); }
	static Void network_spent_request_heist(Any p0, bool p1, bool p2) { return _i<Void>(0xA212A0, p0, p1, p2); }
	static Void network_spent_request_job(Any p0, bool p1, bool p2) { return _i<Void>(0xA211A0, p0, p1, p2); }
	static Void network_spent_robbed_by_mugger(int amount, bool p1, bool p2) { return _i<Void>(0xA1FFE0, amount, p1, p2); }
	static Void network_spent_taxi(int amount, bool p1, bool p2) { return _i<Void>(0xA1F120, amount, p1, p2); }
	static Void network_spent_telescope(Any p0, bool p1, bool p2) { return _i<Void>(0xA20620, p0, p1, p2); }
	static Void network_start_synchronised_scene(int netScene) { return _i<Void>(0xA552A0, netScene); }
	static Void network_stop_synchronised_scene(int netScene) { return _i<Void>(0xA55310, netScene); }
	static Void network_suppress_invite(bool toggle) { return _i<Void>(0xA4B4D0, toggle); }
	static Void network_unregister_networked_entity(Entity entity) { return _i<Void>(0xA4F520, entity); }
	static Any network_update_load_scene() { return _i<Any>(0xAD9210); }
	static Entity net_to_ent(int netHandle) { return _i<Entity>(0xA4FB30, netHandle); }
	static Object net_to_obj(int netHandle) { return _i<Object>(0xA4FAF0, netHandle); }
	static Ped net_to_ped(int netHandle) { return _i<Ped>(0xA4FAB0, netHandle); }
	static Vehicle net_to_veh(int netHandle) { return _i<Vehicle>(0xA4FA70, netHandle); }
	static bool new_load_scene_start(float p0, float p1, float p2, float p3, float p4, float p5, float p6, Any p7) { return _i<bool>(0xADA2E0, p0, p1, p2, p3, p4, p5, p6, p7); }
	static bool new_load_scene_start_sphere(float x, float y, float z, float radius, Any p4) { return _i<bool>(0xADA390, x, y, z, radius, p4); }
	static Void new_load_scene_stop() { return _i<Void>(0xADA420); }
	static int obj_to_net(Object object) { return _i<int>(0xA4FA10, object); }
	static Void open_bomb_bay_doors(Vehicle vehicle) { return _i<Void>(0xB262C0, vehicle); }
	static Void open_patrol_route(const char* patrolRoute) { return _i<Void>(0xAF6240, patrolRoute); }
	static Void open_sequence_task(Object* taskSequence) { return _i<Void>(0xAF3750, taskSequence); }
	static Void override_cam_spline_motion_blur(Cam cam, int p1, float p2, float p3) { return _i<Void>(0x9933D0, cam, p1, p2, p3); }
	static Void override_cam_spline_velocity(Cam cam, int p1, float p2, float p3) { return _i<Void>(0x993300, cam, p1, p2, p3); }
	static Void override_popschedule_vehicle_model(int scheduleId, uint32_t vehicleHash) { return _i<Void>(0xB3C720, scheduleId, vehicleHash); }
	static bool override_save_house(bool p0, float p1, float p2, float p3, float p4, bool p5, float p6, float p7) { return _i<bool>(0x9F7E00, p0, p1, p2, p3, p4, p5, p6, p7); }
	static Void override_trevor_rage(Any* p0) { return _i<Void>(0x983900, p0); }
	static Void override_underwater_stream(Any* p0, bool p1) { return _i<Void>(0x983790, p0, p1); }
	static Void override_veh_horn(Vehicle vehicle, bool mute, int p2) { return _i<Void>(0x985560, vehicle, mute, p2); }
	static Player participant_id() { return _i<Player>(0xA4EE10); }
	static int participant_id_to_int() { return _i<int>(0xA4EE40); }
	static Void pause_clock(bool toggle) { return _i<Void>(0x9969E0, toggle); }
	static Void pause_playback_recorded_vehicle(Any p0) { return _i<Void>(0xB1FC20, p0); }
	static Void pause_scripted_conversation(bool p0) { return _i<Void>(0x982FC0, p0); }
	static bool ped_has_use_scenario_task(Ped ped) { return _i<bool>(0xAF5860, ped); }
	static int ped_to_net(Ped ped) { return _i<int>(0xA4F9B0, ped); }
	static Void pin_rope_vertex(Object rope, int vertex, float x, float y, float z) { return _i<Void>(0xAA0960, rope, vertex, x, y, z); }
	static bool place_object_on_ground_properly(Object object) { return _i<bool>(0xA63B30, object); }
	static Void player_attach_virtual_bound(float p0, float p1, float p2, float p3, float p4, float p5, float p6, float p7) { return _i<Void>(0xAAD350, p0, p1, p2, p3, p4, p5, p6, p7); }
	static Void player_detach_virtual_bound() { return _i<Void>(0xAAD370); }
	static Player player_id() { return _i<Player>(0xAAAFC0); }
	static Ped player_ped_id() { return _i<Ped>(0xAAB000); }
	static Void playstats_award_xp(Any p0, Any p1, Any p2) { return _i<Void>(0xAD03B0, p0, p1, p2); }
	static Void playstats_cheat_applied(const char* cheat) { return _i<Void>(0xAD0CC0, cheat); }
	static Void playstats_cloth_change(Any p0, Any p1, Any p2, Any p3, Any p4) { return _i<Void>(0xAD0B80, p0, p1, p2, p3, p4); }
	static Void playstats_friend_activity(Any p0, Any p1) { return _i<Void>(0xAD0AD0, p0, p1); }
	static Void playstats_match_started(Any p0, Any p1, Any p2) { return _i<Void>(0xAD0810, p0, p1, p2); }
	static Void playstats_mission_checkpoint(Any* p0, Any p1, Any p2, Any p3) { return _i<Void>(0xAD06A0, p0, p1, p2, p3); }
	static Void playstats_mission_over(Any* p0, Any p1, Any p2, bool p3, bool p4, bool p5) { return _i<Void>(0xAD05F0, p0, p1, p2, p3, p4, p5); }
	static Void playstats_mission_started(Any* p0, Any p1, Any p2, bool p3) { return _i<Void>(0xAD05D0, p0, p1, p2, p3); }
	static Void playstats_npc_invite(Any* p0) { return _i<Void>(0xAD03A0, p0); }
	static Void playstats_oddjob_done(Any p0, Any p1, Any p2) { return _i<Void>(0xAD0AE0, p0, p1, p2); }
	static Void playstats_prop_change(Any p0, Any p1, Any p2, Any p3) { return _i<Void>(0xAD0AF0, p0, p1, p2, p3); }
	static Void playstats_race_checkpoint(Any p0, Any p1, Any p2, Any p3, Any p4) { return _i<Void>(0xAD07B0, p0, p1, p2, p3, p4); }
	static Void playstats_rank_up(Any p0) { return _i<Void>(0xAD03D0, p0); }
	static Void playstats_shop_item(Any p0, Any p1, Any p2, Any p3, Any p4) { return _i<Void>(0xAD0830, p0, p1, p2, p3, p4); }
	static Void playstats_website_visited(uint32_t scaleformHash, int p1) { return _i<Void>(0xAD0AC0, scaleformHash, p1); }
	static Void play_anim_on_running_scenario(Ped ped, const char* animDict, const char* animName) { return _i<Void>(0xAF58C0, ped, animDict, animName); }
	static Void play_end_credits_music(bool play) { return _i<Void>(0x9848C0, play); }
	static bool play_entity_anim(Entity entity, const char* animName, const char* animDict, float p3, bool loop, bool stayInAnim, bool p6, float delta, Any bitset) { return _i<bool>(0x9B52A0, entity, animName, animDict, p3, loop, stayInAnim, p6, delta, bitset); }
	static Void play_entity_scripted_anim(Any p0, Any* p1, Any* p2, Any* p3, float p4, float p5) { return _i<Void>(0xAF31D0, p0, p1, p2, p3, p4, p5); }
	static Void play_facial_anim(Ped ped, const char* animName, const char* animDict) { return _i<Void>(0xA9A050, ped, animName, animDict); }
	static Void play_mission_complete_audio(const char* audioName) { return _i<Void>(0x986720, audioName); }
	static Void play_pain(Ped ped, int painID, int p1, Any p3) { return _i<Void>(0x983990, ped, painID, p1, p3); }
	static Void play_ped_ringtone(const char* ringtoneName, Ped ped, bool p2) { return _i<Void>(0x982850, ringtoneName, ped, p2); }
	static Any play_police_report(const char* name, float p1) { return _i<Any>(0x9854D0, name, p1); }
	static Void play_sound_from_coord(int soundId, const char* audioName, float x, float y, float z, const char* audioRef, bool p6, int range, bool p8) { return _i<Void>(0x9835D0, soundId, audioName, x, y, z, audioRef, p6, range, p8); }
	static Void play_sound_from_entity(int soundId, const char* audioName, Entity entity, const char* audioRef, bool p4, Any p5) { return _i<Void>(0x9834B0, soundId, audioName, entity, audioRef, p4, p5); }
	static Void play_stream_from_object(Object object) { return _i<Void>(0x985870, object); }
	static Void play_stream_from_ped(Ped ped) { return _i<Void>(0x9857D0, ped); }
	// [aligned] static Void play_stream_from_position(float x, float y, float z) { return _i<Void>(0x9858D0, x, y, z); }
	static Void play_stream_from_vehicle(Vehicle vehicle) { return _i<Void>(0x985820, vehicle); }
	static Void play_stream_frontend() { return _i<Void>(0x9858C0); }
	static bool play_synchronized_audio_event(Any p0) { return _i<bool>(0x9873E0, p0); }
	static bool play_synchronized_cam_anim(Any p0, Any p1, const char* animName, const char* animDictionary) { return _i<bool>(0x993C00, p0, p1, animName, animDictionary); }
	static bool play_synchronized_entity_anim(Entity entity, int syncedScene, const char* animation, const char* propName, float p4, float p5, Any p6, float p7) { return _i<bool>(0x9B5310, entity, syncedScene, animation, propName, p4, p5, p6, p7); }
	static Void play_vehicle_door_close_sound(Vehicle vehicle, int p1) { return _i<Void>(0x986560, vehicle, p1); }
	static Void play_vehicle_door_open_sound(Vehicle vehicle, int p1) { return _i<Void>(0x9864D0, vehicle, p1); }
	static Void point_cam_at_coord(Cam cam, float x, float y, float z) { return _i<Void>(0x992990, cam, x, y, z); }
	static Void point_cam_at_entity(Cam cam, Entity entity, float p2, float p3, float p4, bool p5) { return _i<Void>(0x992A50, cam, entity, p2, p3, p4, p5); }
	static Void point_cam_at_ped_bone(Cam cam, int ped, int boneIndex, float x, float y, float z, bool p6) { return _i<Void>(0x992A70, cam, ped, boneIndex, x, y, z, p6); }
	static Void populate_now() { return _i<Void>(0x9F97F0); }
	static Void pop_timecycle_modifier() { return _i<Void>(0x9D0980); }
	static Void prefetch_srl(const char* srl) { return _i<Void>(0xADAD20, srl); }
	static Void preload_script_phone_conversation(bool p0, bool p1) { return _i<Void>(0x982DC0, p0, p1); }
	static Void preload_vehicle_mod(Any p0, Any p1, Any p2) { return _i<Void>(0xB276C0, p0, p1, p2); }
	static bool prepare_alarm(const char* alarmName) { return _i<bool>(0x986CA0, alarmName); }
	static bool prepare_music_event(const char* eventName) { return _i<bool>(0x986A20, eventName); }
	static Any prepare_synchronized_audio_event(const char* p0, Any p1) { return _i<Any>(0x987360, p0, p1); }
	static bool prepare_synchronized_audio_event_for_scene(Any p0, Any* p1) { return _i<bool>(0x987390, p0, p1); }
	static Void process_entity_attachments(Entity entity) { return _i<Void>(0x9B5020, entity); }
	static Void pulse_blip(Blip blip) { return _i<Void>(0x9E2EE0, blip); }
	static Void push_timecycle_modifier() { return _i<Void>(0x9D0970); }
	static Void raise_convertible_roof(Vehicle vehicle, bool instantlyRaise) { return _i<Void>(0xB22DF0, vehicle, instantlyRaise); }
	static Void refresh_interior(int interiorID) { return _i<Void>(0x9E8F50, interiorID); }
	static Void refresh_waypoint() { return _i<Void>(0x9E3150); }
	static Void register_bool_to_save(Any* p0, const char* name) { return _i<Void>(0x9F99A0, p0, name); }
	static Void register_enum_to_save(Any* p0, const char* name) { return _i<Void>(0x9F9940, p0, name); }
	static Void register_float_to_save(Any* p0, const char* name) { return _i<Void>(0x9F9970, p0, name); }
	static Void register_hated_targets_around_ped(Ped ped, float radius) { return _i<Void>(0xA97980, ped, radius); }
	static Void register_int_to_save(Any* p0, const char* name) { return _i<Void>(0x9F98E0, p0, name); }
	static bool register_named_rendertarget(const char* p0, bool p1) { return _i<bool>(0x9E1AE0, p0, p1); }
	static Void register_object_script_brain(const char* scriptName, uint32_t objectName, int p2, float p3, int p4, int p5) { return _i<Void>(0x987F70, scriptName, objectName, p2, p3, p4, p5); }
	static int register_pedheadshot(Ped ped) { return _i<int>(0xA9D410, ped); }
	static Any register_save_house(float p0, float p1, float p2, float p3, Any* p4, Any p5, Any p6) { return _i<Any>(0x9F7D60, p0, p1, p2, p3, p4, p5, p6); }
	static Void register_script_with_audio(Any p0) { return _i<Void>(0x9831D0, p0); }
	static Void register_target(Ped ped, Ped target) { return _i<Void>(0xA978D0, ped, target); }
	static Void register_text_label_to_save(Any* p0, const char* name) { return _i<Void>(0x9F99D0, p0, name); }
	static Void register_world_point_script_brain(Any* p0, float p1, Any p2) { return _i<Void>(0x9880F0, p0, p1, p2); }
	static Void release_ambient_audio_bank() { return _i<Void>(0x983320); }
	static Void release_mission_audio_bank() { return _i<Void>(0x983310); }
	static Void release_movie_mesh_set(int movieMeshSet) { return _i<Void>(0x9CDAB0, movieMeshSet); }
	static bool release_named_rendertarget(Any* p0) { return _i<bool>(0x9E1BE0, p0); }
	static Void release_named_script_audio_bank(const char* audioBank) { return _i<Void>(0x983330, audioBank); }
	static Void release_preload_mods(Vehicle vehicle) { return _i<Void>(0xB277C0, vehicle); }
	static Void release_script_audio_bank() { return _i<Void>(0x983350); }
	static Void release_sound_id(int soundId) { return _i<Void>(0x9833D0, soundId); }
	static Void remove_action_mode_asset(const char* asset) { return _i<Void>(0xA9DBB0, asset); }
	static Void remove_all_cover_blocking_areas() { return _i<Void>(0xAF53A0); }
	static Void remove_all_ped_weapons(Ped ped, bool p1) { return _i<Void>(0xB39020, ped, p1); }
	static Void remove_all_pickups_of_type(uint32_t pickupHash) { return _i<Void>(0xA660A0, pickupHash); }
	static Void remove_all_projectiles_of_type(uint32_t weaponHash, bool p1) { return _i<Void>(0xB39E10, weaponHash, p1); }
	static Void remove_all_shocking_events(bool p0) { return _i<Void>(0x9B9610, p0); }
	static Void remove_anim_dict(const char* animDict) { return _i<Void>(0xAD9860, animDict); }
	static Void remove_anim_set(const char* animSet) { return _i<Void>(0xAD9980, animSet); }
	static Void remove_blip(Blip* blip) { return _i<Void>(0x9E2E50, blip); }
	static Void remove_clip_set(const char* clipSet) { return _i<Void>(0xAD9AB0, clipSet); }
	static Void remove_cover_point(ScrHandle coverpoint) { return _i<Void>(0xAF4C50, coverpoint); }
	static Void remove_cutscene() { return _i<Void>(0x9990B0); }
	static Void remove_decal(int decal) { return _i<Void>(0x9D0110, decal); }
	static Void remove_decals_from_object(Object obj) { return _i<Void>(0x9CFDB0, obj); }
	static Void remove_decals_from_object_facing(Object obj, float x, float y, float z) { return _i<Void>(0x9CFE10, obj, x, y, z); }
	static Void remove_decals_from_vehicle(Vehicle vehicle) { return _i<Void>(0x9CFE80, vehicle); }
	static Void remove_decals_in_range(float x, float y, float z, float range) { return _i<Void>(0x9CFD70, x, y, z, range); }
	static Void remove_dispatch_spawn_blocking_area(Any p0) { return _i<Void>(0x9FA790, p0); }
	static Void remove_door_from_system(uint32_t doorHash) { return _i<Void>(0xA64500, doorHash); }
	static Void remove_forced_object(Any p0, Any p1, Any p2, Any p3, Any p4) { return _i<Void>(0x9B7290, p0, p1, p2, p3, p4); }
	static Void remove_from_itemset(Any p0, Any p1) { return _i<Void>(0x9E97A0, p0, p1); }
	static Void remove_group(int groupId) { return _i<Void>(0xA92E80, groupId); }
	static Void remove_ipl(const char* iplName) { return _i<Void>(0xAD9BC0, iplName); }
	static Void remove_model_hide(Any p0, Any p1, Any p2, Any p3, Any p4, Any p5) { return _i<Void>(0x9B7130, p0, p1, p2, p3, p4, p5); }
	static Void remove_model_swap(float x, float y, float z, float radius, uint32_t originalModel, uint32_t newModel, bool p6) { return _i<Void>(0x9B6F00, x, y, z, radius, originalModel, newModel, p6); }
	static Void remove_mp_gamer_tag(int gamerTagId) { return _i<Void>(0x9E4890, gamerTagId); }
	static Void remove_multiplayer_bank_cash() { return _i<Void>(0x9E3770); }
	static Void remove_multiplayer_hud_cash() { return _i<Void>(0x9E37A0); }
	static Void remove_named_ptfx_asset(const char* fxName) { return _i<Void>(0xAD9D80, fxName); }
	static Void remove_navmesh_blocking_object(Any p0) { return _i<Void>(0xA70E90, p0); }
	static Void remove_navmesh_required_regions() { return _i<Void>(0xA70C20); }
	static Void remove_particle_fx(int ptfxHandle, bool p1) { return _i<Void>(0x9CF460, ptfxHandle, p1); }
	static Void remove_particle_fx_from_entity(Entity entity) { return _i<Void>(0x9CF480, entity); }
	static Void remove_particle_fx_in_range(float X, float Y, float Z, float radius) { return _i<Void>(0x9CF4D0, X, Y, Z, radius); }
	static Void remove_ped_defensive_area(Ped ped, bool toggle) { return _i<Void>(0xA98550, ped, toggle); }
	static Void remove_ped_elegantly(Ped* ped) { return _i<Void>(0xA90840, ped); }
	static Void remove_ped_from_group(Ped ped) { return _i<Void>(0xA92E90, ped); }
	static Void remove_ped_helmet(Ped ped, bool instantly) { return _i<Void>(0xA9AD70, ped, instantly); }
	static Void remove_ped_preferred_cover_set(Ped ped) { return _i<Void>(0xA986F0, ped); }
	static Void remove_pickup(Pickup pickup) { return _i<Void>(0xA660F0, pickup); }
	static Void remove_pick_up_rope_for_cargobob(Vehicle cargobob) { return _i<Void>(0xB29BD0, cargobob); }
	static Void remove_player_helmet(Player player, bool p2) { return _i<Void>(0xAAB4B0, player, p2); }
	static Void remove_ptfx_asset() { return _i<Void>(0xAD9D30); }
	static Void remove_relationship_group(uint32_t groupHash) { return _i<Void>(0xA939C0, groupHash); }
	static Void remove_scenario_blocking_area(Any p0, bool p1) { return _i<Void>(0xA99A10, p0, p1); }
	static Void remove_scenario_blocking_areas() { return _i<Void>(0xA999F0); }
	static bool remove_shocking_event(ScrHandle event) { return _i<bool>(0x9B95E0, event); }
	static Void remove_shocking_event_spawn_blocking_areas() { return _i<Void>(0x9B9630); }
	static Void remove_stealth_mode_asset(const char* asset) { return _i<Void>(0xA9DCF0, asset); }
	static Void remove_vehicles_from_generators_in_area(float x1, float y1, float z1, float x2, float y2, float z2, Any unk) { return _i<Void>(0xB22AF0, x1, y1, z1, x2, y2, z2, unk); }
	static Void remove_vehicle_asset(int vehicleAsset) { return _i<Void>(0xB24AC0, vehicleAsset); }
	static Void remove_vehicle_high_detail_model(Vehicle vehicle) { return _i<Void>(0xB24940, vehicle); }
	static Void remove_vehicle_mod(Vehicle vehicle, int modType) { return _i<Void>(0xB272B0, vehicle, modType); }
	static Void remove_vehicle_recording(Any p0, Any* p1) { return _i<Void>(0xB1F790, p0, p1); }
	static Void remove_vehicle_stuck_check(Vehicle vehicle) { return _i<Void>(0xB1D950, vehicle); }
	static Void remove_vehicle_upsidedown_check(Vehicle vehicle) { return _i<Void>(0xB1BA80, vehicle); }
	static Void remove_vehicle_window(Vehicle vehicle, int windowIndex) { return _i<Void>(0xB1E650, vehicle, windowIndex); }
	static Void remove_waypoint_recording(const char* name) { return _i<Void>(0xAF6840, name); }
	static Void remove_weapon_asset(uint32_t weaponHash) { return _i<Void>(0xB3A270, weaponHash); }
	static Void remove_weapon_component_from_ped(Ped ped, uint32_t weaponHash, uint32_t componentHash) { return _i<Void>(0xB39FA0, ped, weaponHash, componentHash); }
	static Void remove_weapon_from_ped(Ped ped, uint32_t weaponHash) { return _i<Void>(0xB390A0, ped, weaponHash); }
	static Void render_script_cams(bool render, bool ease, int easeTime, bool p3, bool p4, Any p5) { return _i<Void>(0x990E50, render, ease, easeTime, p3, p4, p5); }
	static Void report_crime(Player player, int crimeType, int wantedLvlThresh) { return _i<Void>(0xAAA1A0, player, crimeType, wantedLvlThresh); }
	static Void request_action_mode_asset(const char* asset) { return _i<Void>(0xA9DB00, asset); }
	static Void request_additional_collision_at_coord(float p0, float p1, float p2) { return _i<Void>(0xAD9730, p0, p1, p2); }
	static Void request_additional_text(const char* gxt, int slot) { return _i<Void>(0x9E0D20, gxt, slot); }
	static bool request_ambient_audio_bank(const char* p0, bool p1, Any p2) { return _i<bool>(0x983230, p0, p1, p2); }
	static Void request_anim_dict(const char* animDict) { return _i<Void>(0xAD97D0, animDict); }
	static Void request_anim_set(const char* animSet) { return _i<Void>(0xAD98E0, animSet); }
	static Void request_collision_at_coord(float x, float y, float z) { return _i<Void>(0xAD9640, x, y, z); }
	static Void request_collision_for_model(uint32_t model) { return _i<Void>(0xAD9680, model); }
	static Void request_cutscene(const char* cutsceneName, int p1) { return _i<Void>(0x998FD0, cutsceneName, p1); }
	static Void request_ipl(const char* iplName) { return _i<Void>(0xAD9B40, iplName); }
	static Void request_menu_ped_model(uint32_t model) { return _i<Void>(0xAD92A0, model); }
	static bool request_mission_audio_bank(const char* p0, bool p1, Any p2) { return _i<bool>(0x9831F0, p0, p1, p2); }
	static Void request_model(uint32_t model) { return _i<Void>(0xAD9290, model); }
	static Void request_named_ptfx_asset(const char* fxName) { return _i<Void>(0xAD9D40, fxName); }
	static Void request_ptfx_asset() { return _i<Void>(0xAD9D00); }
	static int request_scaleform_movie(const char* scaleformName) { return _i<int>(0x9D0CC0, scaleformName); }
	static int request_scaleform_movie_instance(const char* scaleformName) { return _i<int>(0x9D0D20, scaleformName); }
	static Void request_script(const char* scriptName) { return _i<Void>(0xAB0E40, scriptName); }
	static bool request_script_audio_bank(const char* p0, bool p1, Any p2) { return _i<bool>(0x983270, p0, p1, p2); }
	static Void request_script_with_name_hash(uint32_t scriptHash) { return _i<Void>(0xAB0FB0, scriptHash); }
	static Void request_stealth_mode_asset(const char* asset) { return _i<Void>(0xA9DC30, asset); }
	static Void request_streamed_texture_dict(const char* textureDict, bool p1) { return _i<Void>(0x9CC970, textureDict, p1); }
	static Void request_vehicle_asset(uint32_t vehicleHash, int vehicleAsset) { return _i<Void>(0xB249E0, vehicleHash, vehicleAsset); }
	static Void request_vehicle_high_detail_model(Vehicle vehicle) { return _i<Void>(0xB24770, vehicle); }
	static Void request_vehicle_recording(Any p0, Any* p1) { return _i<Void>(0xB1F6D0, p0, p1); }
	static Void request_waypoint_recording(const char* name) { return _i<Void>(0xAF67C0, name); }
	static Void request_weapon_asset(uint32_t weaponHash, int p1, int p2) { return _i<Void>(0xB3A180, weaponHash, p1, p2); }
	static Void request_weapon_high_detail_model(Entity weaponObject) { return _i<Void>(0xB3AC70, weaponObject); }
	static Void reserve_network_mission_objects(int p0) { return _i<Void>(0xA53650, p0); }
	static Void reserve_network_mission_peds(int p0) { return _i<Void>(0xA53660, p0); }
	static Void reserve_network_mission_vehicles(int p0) { return _i<Void>(0xA53670, p0); }
	static Void reset_ai_melee_weapon_damage_modifier() { return _i<Void>(0xA91B70); }
	static Void reset_ai_weapon_damage_modifier() { return _i<Void>(0xA91B30); }
	static Void reset_dispatch_ideal_spawn_distance() { return _i<Void>(0x9FA5C0); }
	static Void reset_dispatch_spawn_blocking_areas() { return _i<Void>(0x9FA7C0); }
	static Void reset_entity_alpha(Entity entity) { return _i<Void>(0x9B6B70, entity); }
	static Void reset_exclusive_scenario_group() { return _i<Void>(0xAF5B20); }
	static Void reset_group_formation_default_spacing(int groupHandle) { return _i<Void>(0xA94280, groupHandle); }
	static Void reset_hud_component_values(int id) { return _i<Void>(0x9E4190, id); }
	static Void reset_ped_audio_flags(Any p0) { return _i<Void>(0x986E70, p0); }
	static Void reset_ped_in_vehicle_context(Ped ped) { return _i<Void>(0xA958C0, ped); }
	static Void reset_ped_last_vehicle(Ped ped) { return _i<Void>(0xA90EB0, ped); }
	static Void reset_ped_movement_clipset(Ped ped, float p1) { return _i<Void>(0xA955F0, ped, p1); }
	static Void reset_ped_ragdoll_timer(Ped ped) { return _i<Void>(0xA98080, ped); }
	static Void reset_ped_strafe_clipset(Ped ped) { return _i<Void>(0xA95670, ped); }
	static Void reset_ped_visible_damage(Ped ped) { return _i<Void>(0xA98AB0, ped); }
	static Void reset_ped_weapon_movement_clipset(Ped ped) { return _i<Void>(0xA956E0, ped); }
	static Void reset_player_arrest_state(Player player) { return _i<Void>(0xAAADD0, player); }
	static Void reset_player_input_gait(Player player) { return _i<Void>(0xAAD0C0, player); }
	static Void reset_player_stamina(Player player) { return _i<Void>(0xAAA990, player); }
	static Void reset_reticule_values() { return _i<Void>(0x9E4180); }
	static Void reset_scenario_groups_enabled() { return _i<Void>(0xAF5A90); }
	static Void reset_scenario_types_enabled() { return _i<Void>(0xAF5C60); }
	static Void reset_trevor_rage() { return _i<Void>(0x983920); }
	static Void reset_vehicle_stuck_timer(Vehicle vehicle, int nullAttributes) { return _i<Void>(0xB23480, vehicle, nullAttributes); }
	static Void reset_vehicle_wheels(Vehicle vehicle, bool toggle) { return _i<Void>(0xB23F10, vehicle, toggle); }
	static Void reset_wanted_level_difficulty(Player player) { return _i<Void>(0xAAA050, player); }
	static Void restart_frontend_menu(uint32_t menuHash, int p1) { return _i<Void>(0x9E51B0, menuHash, p1); }
	static Void restart_scripted_conversation() { return _i<Void>(0x982FF0); }
	static Void restore_player_stamina(Player player, float p1) { return _i<Void>(0xAAA9C0, player, p1); }
	static Void resurrect_ped(Ped ped) { return _i<Void>(0xA987D0, ped); }
	static Void revive_injured_ped(Ped ped) { return _i<Void>(0xA98740, ped); }
	static Void roll_down_window(Vehicle vehicle, int windowIndex) { return _i<Void>(0xB1E6E0, vehicle, windowIndex); }
	static Void roll_down_windows(Vehicle vehicle) { return _i<Void>(0xB1E6D0, vehicle); }
	static Void roll_up_window(Vehicle vehicle, int windowIndex) { return _i<Void>(0xB1E750, vehicle, windowIndex); }
	static bool rope_are_textures_loaded() { return _i<bool>(0xAA0FF0); }
	static Void rope_convert_to_simple(Object rope) { return _i<Void>(0xAA0F70, rope); }
	static Void rope_draw_shadow_enabled(Object* rope, bool toggle) { return _i<Void>(0xAA08B0, rope, toggle); }
	static Void rope_force_length(Object rope, float length) { return _i<Void>(0xAA1270, rope, length); }
	static Void rope_reset_length(Object rope, bool length) { return _i<Void>(0xAA12C0, rope, length); }
	static Void rope_set_update_pinverts(Object rope) { return _i<Void>(0xAA0C10, rope); }
	static Void rope_unload_textures() { return _i<Void>(0xAA1020); }
	static Void script_is_moving_mobile_phone_offscreen(bool toggle) { return _i<Void>(0xAAF2D0, toggle); }
	static Void sc_email_message_clear_recip_list() { return _i<Void>(0xAB5DB0); }
	static Void sc_email_message_push_gamer_to_recip_list(Player* player) { return _i<Void>(0xAB5D10, player); }
	static bool sc_inbox_message_get_data_bool(int p0, const char* p1) { return _i<bool>(0xAB5940, p0, p1); }
	static bool sc_inbox_message_get_data_int(int p0, const char* context, int* out) { return _i<bool>(0xAB5900, p0, context, out); }
	static bool sc_inbox_message_get_data_string(int p0, const char* context, const char* out) { return _i<bool>(0xAB59C0, p0, context, out); }
	static bool sc_inbox_message_get_ugcdata(Any p0, Any* p1) { return _i<bool>(0xAB5B90, p0, p1); }
	static bool sc_profanity_check_string(const char* string, int* taskHandle) { return _i<bool>(0xAB6090, string, taskHandle); }
	static int sc_profanity_get_check_is_pending(int taskHandle) { return _i<int>(0xAB61E0, taskHandle); }
	static int sc_profanity_get_string_status(int p0) { return _i<int>(0xAB62A0, p0); }
	static Void set_ability_bar_value(float p0, float p1) { return _i<Void>(0x9E1760, p0, p1); }
	static Void set_activate_object_physics_as_soon_as_it_is_unfrozen(Object object, bool toggle) { return _i<Void>(0xA652F0, object, toggle); }
	static Void set_aggressive_horns(bool toggle) { return _i<Void>(0x985660, toggle); }
	static Void set_air_drag_multiplier_for_players_vehicle(Player player, float multiplier) { return _i<Void>(0xAAB740, player, multiplier); }
	static Void set_ai_melee_weapon_damage_modifier(float modifier) { return _i<Void>(0xA91B50, modifier); }
	static Void set_ai_weapon_damage_modifier(float value) { return _i<Void>(0xA91B10, value); }
	static Void set_all_low_priority_vehicle_generators_active(bool active) { return _i<Void>(0xB1B770, active); }
	static Void set_all_random_peds_flee(Player player, bool toggle) { return _i<Void>(0xAA9E30, player, toggle); }
	static Void set_all_random_peds_flee_this_frame(Player player) { return _i<Void>(0xAA9E80, player); }
	static Void set_all_vehicle_generators_active() { return _i<Void>(0xB1B760); }
	static Void set_all_vehicle_generators_active_in_area(float x1, float y1, float z1, float x2, float y2, float z2, bool p6, bool p7) { return _i<Void>(0xB1B730, x1, y1, z1, x2, y2, z2, p6, p7); }
	static Void set_ambient_voice_name(Ped ped, const char* name) { return _i<Void>(0x983A40, ped, name); }
	static Void set_ambient_zone_list_state(Any* p0, bool p1, bool p2) { return _i<Void>(0x985200, p0, p1, p2); }
	static Void set_ambient_zone_list_state_persistent(const char* ambientZone, bool p1, bool p2) { return _i<Void>(0x985350, ambientZone, p1, p2); }
	static Void set_ambient_zone_state(Any* p0, bool p1, bool p2) { return _i<Void>(0x985180, p0, p1, p2); }
	static Void set_ambient_zone_state_persistent(const char* ambientZone, bool p1, bool p2) { return _i<Void>(0x985310, ambientZone, p1, p2); }
	static bool set_ammo_in_clip(Ped ped, uint32_t weaponHash, int ammo) { return _i<bool>(0xB397B0, ped, weaponHash, ammo); }
	static Void set_animal_mood(Ped animal, int mood) { return _i<Void>(0x9842F0, animal, mood); }
	static Void set_anim_looped(Any p0, bool p1, Any p2, bool p3) { return _i<Void>(0xAF3420, p0, p1, p2, p3); }
	static Void set_anim_rate(Ped p0, float p1, float p2, float p3) { return _i<Void>(0xAF3370, p0, p1, p2, p3); }
	static Void set_audio_flag(const char* flagName, bool toggle) { return _i<Void>(0x987300, flagName, toggle); }
	static Void set_audio_scene_variable(const char* scene, const char* variable, float value) { return _i<Void>(0x986870, scene, variable, value); }
	static Void set_audio_vehicle_priority(Vehicle vehicle, Any p1) { return _i<Void>(0x985DE0, vehicle, p1); }
	static Void set_auto_give_parachute_when_enter_plane(Player player, bool toggle) { return _i<Void>(0xAAD100, player, toggle); }
	static Void set_bit(int* address, int offset) { return _i<Void>(0x9F6DA0, address, offset); }
	static Void set_bits_in_range(int* var, int rangeStart, int rangeEnd, int p3) { return _i<Void>(0x9F92E0, var, rangeStart, rangeEnd, p3); }
	static Void set_blip_alpha(Blip blip, int alpha) { return _i<Void>(0x9E2680, blip, alpha); }
	static Void set_blip_as_friendly(Blip blip, bool toggle) { return _i<Void>(0x9E2EC0, blip, toggle); }
	static Void set_blip_as_mission_creator_blip(Blip blip, bool toggle) { return _i<Void>(0x9E2AA0, blip, toggle); }
	static Void set_blip_as_short_range(Blip blip, bool toggle) { return _i<Void>(0x9E2D40, blip, toggle); }
	static Void set_blip_bright(Blip blip, bool toggle) { return _i<Void>(0x9E3250, blip, toggle); }
	static Void set_blip_category(Blip blip, int index) { return _i<Void>(0x9E2E30, blip, index); }
	static Void set_blip_colour(Blip blip, int color) { return _i<Void>(0x9E2820, blip, color); }
	static Void set_blip_display(Blip blip, int displayId) { return _i<Void>(0x9E2E10, blip, displayId); }
	static Void set_blip_fade(Blip blip, int opacity, int duration) { return _i<Void>(0x9E2700, blip, opacity, duration); }
	static Void set_blip_flashes(Blip blip, bool toggle) { return _i<Void>(0x9E2CC0, blip, toggle); }
	static Void set_blip_flashes_alternate(Blip blip, bool toggle) { return _i<Void>(0x9E2CE0, blip, toggle); }
	static Void set_blip_flash_interval(Blip blip, Any p1) { return _i<Void>(0x9E2800, blip, p1); }
	static Void set_blip_flash_timer(Blip blip, int duration) { return _i<Void>(0x9E27E0, blip, duration); }
	static Void set_blip_high_detail(Blip blip, bool toggle) { return _i<Void>(0x9E2A80, blip, toggle); }
	static Void set_blip_name_from_text_file(Blip blip, const char* gxtEntry) { return _i<Void>(0x9E2600, blip, gxtEntry); }
	static Void set_blip_name_to_player_name(Blip blip, Player player) { return _i<Void>(0x9E2640, blip, player); }
	static Void set_blip_priority(Blip blip, int priority) { return _i<Void>(0x9E2DF0, blip, priority); }
	static Void set_blip_rotation(Blip blip, int rotation) { return _i<Void>(0x9E2760, blip, rotation); }
	static Void set_blip_route(Blip blip, bool enabled) { return _i<Void>(0x9E12F0, blip, enabled); }
	static Void set_blip_route_colour(Blip blip, int colour) { return _i<Void>(0x9E1320, blip, colour); }
	static Void set_blip_scale(Blip blip, float scale) { return _i<Void>(0x9E2D60, blip, scale); }
	static Void set_blip_secondary_colour(Blip blip, float r, float g, float b) { return _i<Void>(0x9E2840, blip, r, g, b); }
	static Void set_blip_show_cone(Blip blip, bool toggle) { return _i<Void>(0x9E3270, blip, toggle); }
	static Void set_blip_sprite(Blip blip, int spriteId) { return _i<Void>(0x9E2560, blip, spriteId); }
	static Void set_boat_anchor(Vehicle vehicle, bool toggle) { return _i<Void>(0xB1D410, vehicle, toggle); }
	static Void set_cam_active(Cam cam, bool active) { return _i<Void>(0x991450, cam, active); }
	static Void set_cam_active_with_interp(Cam camTo, Cam camFrom, int duration, int easeLocation, int easeRotation) { return _i<Void>(0x9935B0, camTo, camFrom, duration, easeLocation, easeRotation); }
	static Void set_cam_anim_current_phase(Cam cam, float phase) { return _i<Void>(0x993B40, cam, phase); }
	static Void set_cam_coord(Cam cam, float posX, float posY, float posZ) { return _i<Void>(0x991AA0, cam, posX, posY, posZ); }
	static Void set_cam_debug_name(Cam camera, const char* name) { return _i<Void>(0x992D80, camera, name); }
	// [aligned] static Void set_cam_dof_fnumber_of_lens(Cam camera, float p1) { return _i<Void>(0x992360, camera, p1); }
	// [aligned] static Void set_cam_dof_focus_distance_bias(Cam camera, float p1) { return _i<Void>(0x992500, camera, p1); }
	// [aligned] static Void set_cam_dof_max_near_in_focus_distance(Cam camera, float p1) { return _i<Void>(0x9925D0, camera, p1); }
	// [aligned] static Void set_cam_dof_max_near_in_focus_distance_blend_level(Cam camera, float p1) { return _i<Void>(0x9926A0, camera, p1); }
	static Void set_cam_dof_strength(Cam cam, float dofStrength) { return _i<Void>(0x991F90, cam, dofStrength); }
	static Void set_cam_far_clip(Cam cam, float farClip) { return _i<Void>(0x991CB0, cam, farClip); }
	static Void set_cam_far_dof(Cam cam, float farDOF) { return _i<Void>(0x991F10, cam, farDOF); }
	static Void set_cam_fov(Cam cam, float fieldOfView) { return _i<Void>(0x991B70, cam, fieldOfView); }
	static Void set_cam_inherit_roll_vehicle(Cam cam, bool p1) { return _i<Void>(0x992980, cam, p1); }
	static Void set_cam_motion_blur_strength(Cam cam, float strength) { return _i<Void>(0x991E00, cam, strength); }
	static Void set_cam_near_clip(Cam cam, float nearClip) { return _i<Void>(0x991C30, cam, nearClip); }
	static Void set_cam_near_dof(Cam cam, float nearDOF) { return _i<Void>(0x991E90, cam, nearDOF); }
	static Void set_cam_rot(Cam cam, float rotX, float rotY, float rotZ, int rotationOrder) { return _i<Void>(0x991B50, cam, rotX, rotY, rotZ, rotationOrder); }
	static Void set_cam_shake_amplitude(Cam cam, float amplitude) { return _i<Void>(0x9937D0, cam, amplitude); }
	static Void set_cam_spline_duration(int cam, int timeDuration) { return _i<Void>(0x993090, cam, timeDuration); }
	static Void set_cam_spline_phase(Cam cam, float p1) { return _i<Void>(0x992EB0, cam, p1); }
	static Void set_cam_use_shallow_dof_mode(Cam cam, bool toggle) { return _i<Void>(0x9920E0, cam, toggle); }
	static Void set_can_attack_friendly(Ped ped, bool toggle, bool p2) { return _i<Void>(0xA95440, ped, toggle, p2); }
	static Void set_can_respray_vehicle(Vehicle vehicle, bool state) { return _i<Void>(0xB1CF90, vehicle, state); }
	// [aligned] static Void set_cargobob_pickup_magnet_active(Vehicle cargobob, bool isActive) { return _i<Void>(0xB29D90, cargobob, isActive); }
	// [aligned] static Void set_cargobob_pickup_magnet_strength(Vehicle cargobob, float strength) { return _i<Void>(0xB29DB0, cargobob, strength); }
	static Void set_car_high_speed_bump_severity_multiplier(float multiplier) { return _i<Void>(0xB2B790, multiplier); }
	static Void set_cg_at_boundcenter(Object rope) { return _i<Void>(0xAA15F0, rope); }
	static Void set_checkpoint_rgba(int checkpoint, int red, int green, int blue, int alpha) { return _i<Void>(0x9CC650, checkpoint, red, green, blue, alpha); }
	static Void set_cinematic_button_active(bool p0) { return _i<Void>(0x995E40, p0); }
	static Void set_cinematic_cam_shake_amplitude(float p0) { return _i<Void>(0x995F40, p0); }
	static Void set_cinematic_mode_active(bool p0) { return _i<Void>(0x9961F0, p0); }
	static Void set_clock_date(int day, int month, int year) { return _i<Void>(0x996A70, day, month, year); }
	static Void set_convertible_roof(Vehicle vehicle, bool p1) { return _i<Void>(0xB22D30, vehicle, p1); }
	static Void set_create_random_cops(bool toggle) { return _i<Void>(0xA95020, toggle); }
	static Void set_create_random_cops_not_on_scenarios(bool toggle) { return _i<Void>(0xA95050, toggle); }
	static Void set_create_random_cops_on_scenarios(bool toggle) { return _i<Void>(0xA95070, toggle); }
	static Void set_credits_active(bool toggle) { return _i<Void>(0x9F77B0, toggle); }
	static bool set_current_ped_vehicle_weapon(Ped ped, uint32_t weaponHash) { return _i<bool>(0xB38B20, ped, weaponHash); }
	static Void set_cutscene_audio_override(const char* p0) { return _i<Void>(0x985460, p0); }
	static Void set_cutscene_fade_values(bool p0, bool p1, bool p2, bool p3) { return _i<Void>(0x999AA0, p0, p1, p2, p3); }
	static Void set_cutscene_origin(float x, float y, float z, float p3, int p4) { return _i<Void>(0x999460, x, y, z, p3, p4); }
	static Void set_cutscene_ped_component_variation(const char* cutsceneEntName, int p1, int p2, int p3, uint32_t modelHash) { return _i<Void>(0x999CC0, cutsceneEntName, p1, p2, p3, modelHash); }
	static Void set_cutscene_ped_prop_variation(const char* cutsceneEntName, int p1, int p2, int p3, uint32_t modelHash) { return _i<Void>(0x999DA0, cutsceneEntName, p1, p2, p3, modelHash); }
	static Void set_cutscene_trigger_area(float p0, float p1, float p2, float p3, float p4, float p5) { return _i<Void>(0x999880, p0, p1, p2, p3, p4, p5); }
	static Void set_damping(Object rope, int vertex, float value) { return _i<Void>(0xAA1370, rope, vertex, value); }
	static Void set_debug_lines_and_spheres_drawing_active(bool enabled) { return _i<Void>(0x9CB750, enabled); }
	static Void set_decision_maker(Ped ped, uint32_t name) { return _i<Void>(0x9B9360, ped, name); }
	static Void set_disable_ambient_melee_move(Player player, bool toggle) { return _i<Void>(0xAABAD0, player, toggle); }
	static Void set_disable_frag_damage(Object object, bool toggle) { return _i<Void>(0xAA1810, object, toggle); }
	static Void set_disable_vehicle_petrol_tank_damage(Vehicle vehicle, bool toggle) { return _i<Void>(0xB22790, vehicle, toggle); }
	static Void set_disable_vehicle_petrol_tank_fires(Vehicle vehicle, bool toggle) { return _i<Void>(0xB226E0, vehicle, toggle); }
	static Void set_dispatch_cops_for_player(Player player, bool toggle) { return _i<Void>(0xAA9A80, player, toggle); }
	static Void set_dispatch_ideal_spawn_distance(float p0) { return _i<Void>(0x9FA5E0, p0); }
	static Void set_dispatch_time_between_spawn_attempts(Any p0, float p1) { return _i<Void>(0x9FA680, p0, p1); }
	static Void set_dispatch_time_between_spawn_attempts_multiplier(Any p0, float p1) { return _i<Void>(0x9FA690, p0, p1); }
	static Void set_ditch_police_models(bool toggle) { return _i<Void>(0xAD9CD0, toggle); }
	static Void set_draw_origin(float x, float y, float z, bool screen) { return _i<Void>(0x9CD4C0, x, y, z, screen); }
	static Void set_driver_ability(Ped driver, float ability) { return _i<Void>(0xA97CD0, driver, ability); }
	static Void set_driver_aggressiveness(Ped driver, float aggressiveness) { return _i<Void>(0xA97D40, driver, aggressiveness); }
	static Void set_drive_task_cruise_speed(Ped driver, float cruiseSpeed) { return _i<Void>(0xAF5230, driver, cruiseSpeed); }
	static Void set_drive_task_driving_style(Ped ped, int drivingStyle) { return _i<Void>(0xAF5260, ped, drivingStyle); }
	static Void set_drive_task_max_cruise_speed(Any p0, float p1) { return _i<Void>(0xAF5240, p0, p1); }
	static Void set_emitter_radio_station(const char* emitterName, const char* radioStation) { return _i<Void>(0x984730, emitterName, radioStation); }
	static Void set_enable_bound_ankles(Ped ped, bool toggle) { return _i<Void>(0xA95360, ped, toggle); }
	static Void set_enable_handcuffs(Ped ped, bool toggle) { return _i<Void>(0xA952E0, ped, toggle); }
	static Void set_enable_ped_enveff_scale(Ped ped, bool toggle) { return _i<Void>(0xA9C630, ped, toggle); }
	static Void set_enable_scuba(Ped ped, bool toggle) { return _i<Void>(0xA953E0, ped, toggle); }
	static Void set_entity_alpha(Entity entity, int alphaLevel, bool skin) { return _i<Void>(0x9B6AB0, entity, alphaLevel, skin); }
	static Void set_entity_always_prerender(Entity entity, bool toggle) { return _i<Void>(0x9B6C80, entity, toggle); }
	static Void set_entity_anim_current_time(Entity entity, const char* animDictionary, const char* animName, float time) { return _i<Void>(0x9B55C0, entity, animDictionary, animName, time); }
	static Void set_entity_anim_speed(Entity entity, const char* animDictionary, const char* animName, float speedMultiplier) { return _i<Void>(0x9B55E0, entity, animDictionary, animName, speedMultiplier); }
	static Void set_entity_as_mission_entity(Entity entity, bool p1, bool p2) { return _i<Void>(0x9B5600, entity, p1, p2); }
	static Void set_entity_as_no_longer_needed(Entity* entity) { return _i<Void>(0x9B5620, entity); }
	static Void set_entity_can_be_damaged(Entity entity, bool toggle) { return _i<Void>(0x9B57A0, entity, toggle); }
	static Void set_entity_can_be_damaged_by_relationship_group(Entity entity, bool bCanBeDamaged, int relGroup) { return _i<Void>(0x9B5890, entity, bCanBeDamaged, relGroup); }
	static Void set_entity_can_be_targeted_without_los(Entity entity, bool toggle) { return _i<Void>(0x9B59E0, entity, toggle); }
	static Void set_entity_collision(Entity entity, bool toggle, bool keepPhysics) { return _i<Void>(0x9B5AE0, entity, toggle, keepPhysics); }
	static Void set_entity_coords(Entity entity, float xPos, float yPos, float zPos, bool xAxis, bool yAxis, bool zAxis, bool clearArea) { return _i<Void>(0x9B5BA0, entity, xPos, yPos, zPos, xAxis, yAxis, zAxis, clearArea); }
	static Void set_entity_coords_no_offset(Entity entity, float xPos, float yPos, float zPos, bool xAxis, bool yAxis, bool zAxis) { return _i<Void>(0x9B5C40, entity, xPos, yPos, zPos, xAxis, yAxis, zAxis); }
	static Void set_entity_dynamic(Entity entity, bool toggle) { return _i<Void>(0x9B5C70, entity, toggle); }
	static Void set_entity_has_gravity(Entity entity, bool toggle) { return _i<Void>(0x9B68E0, entity, toggle); }
	static Void set_entity_heading(Entity entity, float heading) { return _i<Void>(0x9B5CF0, entity, heading); }
	static Void set_entity_health(Entity entity, int health) { return _i<Void>(0x9B5D00, entity, health); }
	static Void set_entity_icon_color(Entity entity, int red, int green, int blue, int alpha) { return _i<Void>(0x9CD420, entity, red, green, blue, alpha); }
	static Void set_entity_icon_visibility(Entity entity, bool toggle) { return _i<Void>(0x9CD3C0, entity, toggle); }
	static Void set_entity_invincible(Entity entity, bool toggle) { return _i<Void>(0x9B5D20, entity, toggle); }
	static Void set_entity_lights(Entity entity, bool toggle) { return _i<Void>(0x9B5F20, entity, toggle); }
	static Void set_entity_load_collision_flag(Entity entity, bool toggle, Any p2) { return _i<Void>(0x9B5F90, entity, toggle, p2); }
	static Void set_entity_locally_invisible(Entity entity) { return _i<Void>(0xA532E0, entity); }
	static Void set_entity_locally_visible(Entity entity) { return _i<Void>(0xA53340, entity); }
	static Void set_entity_lod_dist(Entity entity, int value) { return _i<Void>(0x9B6A00, entity, value); }
	static Void set_entity_max_health(Entity entity, int value) { return _i<Void>(0x9B30B0, entity, value); }
	static Void set_entity_max_speed(Entity entity, float speed) { return _i<Void>(0x9B6050, entity, speed); }
	static Void set_entity_motion_blur(Entity entity, bool toggle) { return _i<Void>(0x9B7340, entity, toggle); }
	static Void set_entity_no_collision_entity(Entity entity1, Entity entity2, bool unknown) { return _i<Void>(0x9B7310, entity1, entity2, unknown); }
	static Void set_entity_only_damaged_by_player(Entity entity, bool toggle) { return _i<Void>(0x9B60C0, entity, toggle); }
	static Void set_entity_only_damaged_by_relationship_group(Entity entity, bool p1, Any p2) { return _i<Void>(0x9B6140, entity, p1, p2); }
	static Void set_entity_proofs(Entity entity, bool bulletProof, bool fireProof, bool explosionProof, bool collisionProof, bool meleeProof, bool p6, bool p7, bool drownProof) { return _i<Void>(0x9B6210, entity, bulletProof, fireProof, explosionProof, collisionProof, meleeProof, p6, p7, drownProof); }
	static Void set_entity_records_collisions(Entity entity, bool toggle) { return _i<Void>(0x9B6650, entity, toggle); }
	static Void set_entity_render_scorched(Entity entity, bool toggle) { return _i<Void>(0x9B6CE0, entity, toggle); }
	static Void set_entity_rotation(Entity entity, float pitch, float roll, float yaw, int rotationOrder, bool p5) { return _i<Void>(0x9B66A0, entity, pitch, roll, yaw, rotationOrder, p5); }
	static Void set_entity_trafficlight_override(Entity entity, int state) { return _i<Void>(0x9B6D60, entity, state); }
	static Void set_entity_velocity(Entity entity, float x, float y, float z) { return _i<Void>(0x9B67C0, entity, x, y, z); }
	static Void set_entity_visible(Entity entity, bool toggle, bool unk) { return _i<Void>(0x9B66C0, entity, toggle, unk); }
	static Void set_entity_visible_in_cutscene(Any p0, bool p1, bool p2) { return _i<Void>(0xA53230, p0, p1, p2); }
	static Void set_everyone_ignore_player(Player player, bool toggle) { return _i<Void>(0xAA9DC0, player, toggle); }
	static Void set_exclusive_scenario_group(const char* scenarioGroup) { return _i<Void>(0xAF5AA0, scenarioGroup); }
	static Void set_explosive_ammo_this_frame(Player player) { return _i<Void>(0x9FAA50, player); }
	static Void set_explosive_melee_this_frame(Player player) { return _i<Void>(0x9FAAE0, player); }
	static Void set_facial_idle_anim_override(Ped ped, const char* animName, const char* animDict) { return _i<Void>(0xA9A120, ped, animName, animDict); }
	static Void set_fade_in_after_death_arrest(bool toggle) { return _i<Void>(0x9F7D20, toggle); }
	static Void set_fade_in_after_load(bool toggle) { return _i<Void>(0x9F7D40, toggle); }
	static Void set_fade_out_after_arrest(bool toggle) { return _i<Void>(0x9F7D00, toggle); }
	static Void set_fade_out_after_death(bool toggle) { return _i<Void>(0x9F7CE0, toggle); }
	static Void set_fake_wanted_level(int fakeWantedLevel) { return _i<Void>(0x9F8500, fakeWantedLevel); }
	static Void set_far_draw_vehicles(bool toggle) { return _i<Void>(0xB1BEE0, toggle); }
	static Void set_fire_ammo_this_frame(Player player) { return _i<Void>(0x9FAA90, player); }
	static Void set_flash(float p0, float p1, float fadeIn, float duration, float fadeOut) { return _i<Void>(0x9CDE70, p0, p1, fadeIn, duration, fadeOut); }
	static Any set_flash_light_fade_distance(float distance) { return _i<Any>(0xB3AF30, distance); }
	static Void set_focus_entity(Entity entity) { return _i<Void>(0xAD9E50, entity); }
	static Void set_follow_ped_cam_view_mode(int viewMode) { return _i<Void>(0x994E50, viewMode); }
	static Void set_follow_vehicle_cam_view_mode(int viewMode) { return _i<Void>(0x994FC0, viewMode); }
	static Void set_force_hd_vehicle(Vehicle vehicle, bool toggle) { return _i<Void>(0xB28D20, vehicle, toggle); }
	static Void set_force_object_this_frame(Any p0, Any p1, Any p2, Any p3) { return _i<Void>(0xA67430, p0, p1, p2, p3); }
	static Void set_frontend_active(bool active) { return _i<Void>(0x9E5270, active); }
	static Void set_frontend_radio_active(bool active) { return _i<Void>(0x984810, active); }
	static Void set_gameplay_cam_relative_heading(float heading) { return _i<Void>(0x9942A0, heading); }
	static Void set_gameplay_cam_relative_pitch(float x, float Value2) { return _i<Void>(0x9942E0, x, Value2); }
	static Void set_gameplay_cam_shake_amplitude(float amplitude) { return _i<Void>(0x9945F0, amplitude); }
	static Void set_gameplay_coord_hint(float p0, float p1, float p2, Any p3, Any p4, Any p5, Any p6) { return _i<Void>(0x9957F0, p0, p1, p2, p3, p4, p5, p6); }
	static Void set_gameplay_entity_hint(Entity entity, float xOffset, float yOffset, float zOffset, bool p4, int p5, int p6, int p7, Any p8) { return _i<Void>(0x995AF0, entity, xOffset, yOffset, zOffset, p4, p5, p6, p7, p8); }
	static Void set_gameplay_hint_fov(float FOV) { return _i<Void>(0x995CC0, FOV); }
	static Void set_gameplay_object_hint(Any p0, float p1, float p2, float p3, bool p4, Any p5, Any p6, Any p7) { return _i<Void>(0x995A20, p0, p1, p2, p3, p4, p5, p6, p7); }
	static Void set_gameplay_ped_hint(Ped p0, float x1, float y1, float z1, bool p4, Any p5, Any p6, Any p7) { return _i<Void>(0x995880, p0, x1, y1, z1, p4, p5, p6, p7); }
	static Void set_gameplay_vehicle_hint(Any p0, float p1, float p2, float p3, bool p4, Any p5, Any p6, Any p7) { return _i<Void>(0x995950, p0, p1, p2, p3, p4, p5, p6, p7); }
	static Void set_game_paused(bool toggle) { return _i<Void>(0x9F96D0, toggle); }
	static Void set_game_pauses_for_streaming(bool toggle) { return _i<Void>(0xAD9C70, toggle); }
	static Void set_garbage_trucks(bool toggle) { return _i<Void>(0xB1F600, toggle); }
	static Void set_global_min_bird_flight_height(float height) { return _i<Void>(0xAF2D90, height); }
	static Void set_gps_active(bool active) { return _i<Void>(0x986700, active); }
	static Void set_gps_disabled_zone(Any p0, Any p1, Any p2, Any p3, Any p4, Any p5) { return _i<Void>(0xA706E0, p0, p1, p2, p3, p4, p5); }
	static Void set_gps_flags(int p0, float p1) { return _i<Void>(0x9E3980, p0, p1); }
	static Void set_gps_flashes(bool toggle) { return _i<Void>(0x9E3B90, toggle); }
	static Void set_gravity_level(int level) { return _i<Void>(0x9F9830, level); }
	static Void set_group_formation(int groupId, int formationType) { return _i<Void>(0xA94160, groupId, formationType); }
	static Void set_group_separation_range(int groupHandle, float separationRange) { return _i<Void>(0xA92F70, groupHandle, separationRange); }
	static Void set_hd_area(float x, float y, float z, float ground) { return _i<Void>(0xADAE60, x, y, z, ground); }
	static Void set_heli_blades_full_speed(Vehicle vehicle) { return _i<Void>(0xB20C00, vehicle); }
	static Void set_heli_blades_speed(Vehicle vehicle, float speed) { return _i<Void>(0xB20CC0, vehicle, speed); }
	static Void set_horn_enabled(Vehicle vehicle, bool toggle) { return _i<Void>(0x985D50, vehicle, toggle); }
	static Void set_hud_component_position(int id, float x, float y) { return _i<Void>(0x9E41B0, id, x, y); }
	static Void set_ignore_low_priority_shocking_events(Player player, bool toggle) { return _i<Void>(0xAA9F70, player, toggle); }
	static Void set_ignore_no_gps_flag(bool ignore) { return _i<Void>(0xA706A0, ignore); }
	static Void set_ik_target(Ped ped, int p1, Ped targetPed, int boneLookAt, float x, float y, float z, Any p7, int duration, int duration1) { return _i<Void>(0xA9DA60, ped, p1, targetPed, boneLookAt, x, y, z, p7, duration, duration1); }
	static Void set_initial_player_station(const char* radioStation) { return _i<Void>(0x984980, radioStation); }
	static Void set_input_exclusive(int inputGroup, int control) { return _i<Void>(0xA6B1D0, inputGroup, control); }
	static Void set_interior_active(int interiorID, bool toggle) { return _i<Void>(0xAD9260, interiorID, toggle); }
	static Void set_last_driven_vehicle(Vehicle vehicle) { return _i<Void>(0xB287A0, vehicle); }
	static Void set_local_player_invisible_locally(bool p0) { return _i<Void>(0xA52D80, p0); }
	static Void set_local_player_visible_in_cutscene(bool p0, bool p1) { return _i<Void>(0xA52D00, p0, p1); }
	static Void set_local_player_visible_locally(bool p0) { return _i<Void>(0xA52E20, p0); }
	static Void set_max_wanted_level(int maxWantedLevel) { return _i<Void>(0xAA9CD0, maxWantedLevel); }
	static Void set_microphone_position(bool p0, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) { return _i<Void>(0x982C40, p0, x1, y1, z1, x2, y2, z2, x3, y3, z3); }
	static Void set_minigame_in_progress(bool toggle) { return _i<Void>(0x9F8630, toggle); }
	static Void set_minimap_block_waypoint(bool toggle) { return _i<Void>(0x9E3D50, toggle); }
	static Any set_minimap_component(int p0, bool p1, int p2) { return _i<Any>(0x9E33C0, p0, p1, p2); }
	static Void set_mission_flag(bool toggle) { return _i<Void>(0x9F5800, toggle); }
	static Void set_mission_name(bool p0, const char* name) { return _i<Void>(0x9E3C70, p0, name); }
	static Void set_mission_train_as_no_longer_needed(Vehicle* train, bool p1) { return _i<Void>(0xB205D0, train, p1); }
	static Void set_mission_train_coords(Vehicle train, float x, float y, float z) { return _i<Void>(0xB205E0, train, x, y, z); }
	static Void set_mobile_phone_position(float posX, float posY, float posZ) { return _i<Void>(0xAAF1D0, posX, posY, posZ); }
	static Void set_mobile_phone_radio_state(bool state) { return _i<Void>(0x9843C0, state); }
	static Void set_mobile_phone_rotation(float rotX, float rotY, float rotZ, Any p3) { return _i<Void>(0xAAF0A0, rotX, rotY, rotZ, p3); }
	static Void set_mobile_phone_scale(float scale) { return _i<Void>(0xAAF080, scale); }
	static Void set_mobile_radio_enabled_during_gameplay(bool Toggle) { return _i<Void>(0x984BC0, Toggle); }
	static Void set_model_as_no_longer_needed(uint32_t model) { return _i<Void>(0xAD9400, model); }
	static Void set_mounted_weapon_target(Ped shootingPed, Ped targetPed, Vehicle targetVehicle, float x, float y, float z, Any p6) { return _i<Void>(0xAF46E0, shootingPed, targetPed, targetVehicle, x, y, z, p6); }
	static Void set_mp_gamer_tag_alpha(int gamerTagId, int component, int alpha) { return _i<Void>(0x9E4A70, gamerTagId, component, alpha); }
	static Void set_mp_gamer_tag_colour(int gamerTagId, int flag, int color) { return _i<Void>(0x9E4A10, gamerTagId, flag, color); }
	static Void set_mp_gamer_tag_health_bar_colour(int headDisplayId, int color) { return _i<Void>(0x9E4A40, headDisplayId, color); }
	static Void set_mp_gamer_tag_name(int gamerTagId, const char* string) { return _i<Void>(0x9E4B00, gamerTagId, string); }
	static Void set_mp_gamer_tag_visibility(int gamerTagId, int component, bool toggle, Any p3) { return _i<Void>(0x9E4910, gamerTagId, component, toggle, p3); }
	static Void set_mp_gamer_tag_wanted_level(int gamerTagId, int wantedlvl) { return _i<Void>(0x9E4AA0, gamerTagId, wantedlvl); }
	static Void set_multiplayer_bank_cash() { return _i<Void>(0x9E3750); }
	static Void set_multiplayer_hud_cash(int p0, int p1) { return _i<Void>(0x9E3790, p0, p1); }
	static Void set_network_id_can_migrate(int netId, bool toggle) { return _i<Void>(0xA52960, netId, toggle); }
	static Void set_network_id_exists_on_all_machines(int netId, bool toggle) { return _i<Void>(0xA52980, netId, toggle); }
	static Void set_network_id_visible_in_cutscene(int netId, bool p1, bool p2) { return _i<Void>(0xA52B30, netId, p1, p2); }
	static Void set_network_vehicle_respot_timer(int netId, int time, Any p2) { return _i<Void>(0xA546C0, netId, time, p2); }
	static Void set_new_waypoint(float x, float y) { return _i<Void>(0x9E3190, x, y); }
	static Void set_next_desired_move_state(float p0) { return _i<Void>(0xAF4A00, p0); }
	static Void set_nightvision(bool toggle) { return _i<Void>(0x9CDC30, toggle); }
	static Void set_noiseoveride(bool toggle) { return _i<Void>(0x9CDD00, toggle); }
	static Void set_noisinessoveride(float value) { return _i<Void>(0x9CDD20, value); }
	static Void set_no_loading_screen(bool toggle) { return _i<Void>(0xAB1570, toggle); }
	static Void set_number_of_parked_vehicles(int value) { return _i<Void>(0xB1BF00, value); }
	static Void set_object_as_no_longer_needed(Object* object) { return _i<Void>(0x9B5740, object); }
	static Void set_object_physics_params(Object object, float weight, float p2, float p3, float p4, float p5, float gravity, float p7, float p8, float p9, float p10, float buoyancy) { return _i<Void>(0xA65250, object, weight, p2, p3, p4, p5, gravity, p7, p8, p9, p10, buoyancy); }
	static Void set_object_targettable(Object object, bool targettable) { return _i<Void>(0xA63CF0, object, targettable); }
	static Void set_override_weather(const char* weatherType) { return _i<Void>(0x9F5EE0, weatherType); }
	static Void set_parachute_task_target(Ped ped, float x, float y, float z) { return _i<Void>(0xAF2430, ped, x, y, z); }
	static Void set_parachute_task_thrust(Ped ped, float thrust) { return _i<Void>(0xAF24B0, ped, thrust); }
	static Void set_parked_vehicle_density_multiplier_this_frame(float multiplier) { return _i<Void>(0xB1BE60, multiplier); }
	static Void set_particle_fx_blood_scale(bool p0) { return _i<Void>(0x9CF7E0, p0); }
	static Void set_particle_fx_cam_inside_nonplayer_vehicle(Any p0, bool p1) { return _i<Void>(0x9CF650, p0, p1); }
	static Void set_particle_fx_cam_inside_vehicle(bool p0) { return _i<Void>(0x9CF630, p0); }
	static Void set_particle_fx_looped_alpha(int ptfxHandle, float alpha) { return _i<Void>(0x9CF5F0, ptfxHandle, alpha); }
	static Void set_particle_fx_looped_colour(int ptfxHandle, float r, float g, float b, bool p4) { return _i<Void>(0x9CF5C0, ptfxHandle, r, g, b, p4); }
	static Void set_particle_fx_looped_evolution(int ptfxHandle, const char* propertyName, float amount, bool Id) { return _i<Void>(0x9CF5A0, ptfxHandle, propertyName, amount, Id); }
	static Void set_particle_fx_looped_offsets(int ptfxHandle, float x, float y, float z, float rotX, float rotY, float rotZ) { return _i<Void>(0x9CF550, ptfxHandle, x, y, z, rotX, rotY, rotZ); }
	static Void set_particle_fx_looped_scale(int ptfxHandle, float scale) { return _i<Void>(0x9CF600, ptfxHandle, scale); }
	static Void set_particle_fx_shootout_boat(Any p0) { return _i<Void>(0x9CF6D0, p0); }
	static Void set_pause_menu_active(bool toggle) { return _i<Void>(0x9E5220, toggle); }
	static Void set_ped_accuracy(Ped ped, int accuracy) { return _i<Void>(0xA90640, ped, accuracy); }
	static Void set_ped_alertness(Ped ped, int value) { return _i<Void>(0xA95520, ped, value); }
	static Void set_ped_allowed_to_duck(Ped ped, bool toggle) { return _i<Void>(0xA946B0, ped, toggle); }
	static Void set_ped_allow_vehicles_override(Ped ped, bool toggle) { return _i<Void>(0xA90A30, ped, toggle); }
	static Void set_ped_alternate_walk_anim(Ped ped, const char* animDict, const char* animName, float p3, bool p4) { return _i<Void>(0xA95950, ped, animDict, animName, p3, p4); }
	static Void set_ped_ammo(Ped ped, uint32_t weaponHash, int ammo, Any p3) { return _i<Void>(0xB38EA0, ped, weaponHash, ammo, p3); }
	static Void set_ped_ammo_by_type(Ped ped, Any ammoType, int ammo) { return _i<Void>(0xB399D0, ped, ammoType, ammo); }
	static Void set_ped_ammo_to_drop(Any p0, Any p1) { return _i<Void>(0xB39AB0, p0, p1); }
	static Void set_ped_angled_defensive_area(Ped ped, float p1, float p2, float p3, float p4, float p5, float p6, float p7, bool p8, bool p9) { return _i<Void>(0xA982D0, ped, p1, p2, p3, p4, p5, p6, p7, p8, p9); }
	static Void set_ped_armour(Ped ped, int amount) { return _i<Void>(0xA90930, ped, amount); }
	static Void set_ped_as_cop(Ped ped, bool toggle) { return _i<Void>(0xA947D0, ped, toggle); }
	static Void set_ped_as_enemy(Ped ped, bool toggle) { return _i<Void>(0xA950B0, ped, toggle); }
	static Void set_ped_as_group_leader(Ped ped, int groupId) { return _i<Void>(0xA92E40, ped, groupId); }
	static Void set_ped_as_group_member(Ped ped, int groupId) { return _i<Void>(0xA92E50, ped, groupId); }
	static Void set_ped_as_no_longer_needed(Ped* ped) { return _i<Void>(0x9B5680, ped); }
	static Void set_ped_blend_from_parents(Ped ped, Any p1, Any p2, float p3, float p4) { return _i<Void>(0xA96330, ped, p1, p2, p3, p4); }
	static Void set_ped_bounds_orientation(Ped ped, float p1, float p2, float p3, float p4, float p5) { return _i<Void>(0xA97830, ped, p1, p2, p3, p4, p5); }
	static Void set_ped_can_arm_ik(Ped ped, bool toggle) { return _i<Void>(0xA9A3B0, ped, toggle); }
	static Void set_ped_can_be_dragged_out(Ped ped, bool toggle) { return _i<Void>(0xA90D10, ped, toggle); }
	static Void set_ped_can_be_knocked_off_vehicle(Ped ped, int state) { return _i<Void>(0xA94B00, ped, state); }
	static Void set_ped_can_be_shot_in_vehicle(Ped ped, bool toggle) { return _i<Void>(0xA919F0, ped, toggle); }
	static Void set_ped_can_be_targeted_when_injured(Ped ped, bool toggle) { return _i<Void>(0xA9AB60, ped, toggle); }
	static Void set_ped_can_be_targeted_without_los(Ped ped, bool toggle) { return _i<Void>(0xA93C80, ped, toggle); }
	static Void set_ped_can_be_targetted(Ped ped, bool toggle) { return _i<Void>(0xA91BE0, ped, toggle); }
	static Void set_ped_can_be_targetted_by_player(Ped ped, Player player, bool toggle) { return _i<Void>(0xA91CC0, ped, player, toggle); }
	static Void set_ped_can_be_targetted_by_team(Ped ped, int team, bool toggle) { return _i<Void>(0xA91C40, ped, team, toggle); }
	static Void set_ped_can_cower_in_cover(Ped ped, bool toggle) { return _i<Void>(0xA9B2A0, ped, toggle); }
	static Void set_ped_can_evasive_dive(Ped ped, bool toggle) { return _i<Void>(0xA9AA20, ped, toggle); }
	static Void set_ped_can_head_ik(Ped ped, bool toggle) { return _i<Void>(0xA9A460, ped, toggle); }
	static Void set_ped_can_leg_ik(Ped ped, bool toggle) { return _i<Void>(0xA9A4C0, ped, toggle); }
	static Void set_ped_can_peek_in_cover(Ped ped, bool toggle) { return _i<Void>(0xA9B300, ped, toggle); }
	static Void set_ped_can_play_ambient_anims(Ped ped, bool toggle) { return _i<Void>(0xA9A2B0, ped, toggle); }
	static Void set_ped_can_play_ambient_base_anims(Ped ped, bool toggle) { return _i<Void>(0xA9A300, ped, toggle); }
	static Void set_ped_can_play_gesture_anims(Ped ped, bool toggle) { return _i<Void>(0xA9A150, ped, toggle); }
	static Void set_ped_can_play_viseme_anims(Ped ped, bool toggle, bool p2) { return _i<Void>(0xA9A1C0, ped, toggle, p2); }
	static Void set_ped_can_ragdoll(Ped ped, bool toggle) { return _i<Void>(0xA980F0, ped, toggle); }
	static Void set_ped_can_ragdoll_from_player_impact(Ped ped, bool toggle) { return _i<Void>(0xA9ACF0, ped, toggle); }
	static Void set_ped_can_smash_glass(Ped ped, bool p1, bool p2) { return _i<Void>(0xA95100, ped, p1, p2); }
	static Void set_ped_can_switch_weapon(Ped ped, bool toggle) { return _i<Void>(0xA9B430, ped, toggle); }
	static Void set_ped_can_teleport_to_group_leader(Ped pedHandle, int groupHandle, bool toggle) { return _i<Void>(0xA92E60, pedHandle, groupHandle, toggle); }
	static Void set_ped_can_torso_ik(Ped ped, bool toggle) { return _i<Void>(0xA9A520, ped, toggle); }
	static Void set_ped_can_use_auto_conversation_lookat(Ped ped, bool toggle) { return _i<Void>(0xA9A640, ped, toggle); }
	static Void set_ped_capsule(Ped ped, float value) { return _i<Void>(0xA9D3C0, ped, value); }
	static Void set_ped_chance_of_firing_blanks(Ped ped, float xBias, float yBias) { return _i<Void>(0xB3ABD0, ped, xBias, yBias); }
	static Void set_ped_cloth_prone(Any p0, Any p1) { return _i<Void>(0xA9A750, p0, p1); }
	static Void set_ped_combat_ability(Ped ped, int p1) { return _i<Void>(0xA9B680, ped, p1); }
	static Void set_ped_combat_attributes(Ped ped, int attributeIndex, bool enabled) { return _i<Void>(0xA9B780, ped, attributeIndex, enabled); }
	static Void set_ped_combat_movement(Ped ped, int combatMovement) { return _i<Void>(0xA9B5D0, ped, combatMovement); }
	static Void set_ped_combat_range(Ped ped, int p1) { return _i<Void>(0xA9B6D0, ped, p1); }
	static Void set_ped_component_variation(Ped ped, int componentId, int drawableId, int textureId, int paletteId) { return _i<Void>(0xA960A0, ped, componentId, drawableId, textureId, paletteId); }
	static Void set_ped_config_flag(Ped ped, int flagId, bool value) { return _i<Void>(0xA9A7F0, ped, flagId, value); }
	static Void set_ped_coords_keep_vehicle(Ped ped, float posX, float posY, float posZ) { return _i<Void>(0xA94F30, ped, posX, posY, posZ); }
	static Void set_ped_coords_no_gang(Ped ped, float posX, float posY, float posZ) { return _i<Void>(0xA94C50, ped, posX, posY, posZ); }
	static Void set_ped_cower_hash(Ped ped, const char* p1) { return _i<Void>(0xA9BC40, ped, p1); }
	static Void set_ped_current_weapon_visible(Ped ped, bool visible, bool deselectWeapon, bool p3, bool p4) { return _i<Void>(0xB391B0, ped, visible, deselectWeapon, p3, p4); }
	static Void set_ped_default_component_variation(Ped ped) { return _i<Void>(0xA962B0, ped); }
	static Void set_ped_defensive_area_attached_to_ped(Ped ped, Ped attachPed, float p2, float p3, float p4, float p5, float p6, float p7, float p8, bool p9, bool p10) { return _i<Void>(0xA98450, ped, attachPed, p2, p3, p4, p5, p6, p7, p8, p9, p10); }
	static Void set_ped_defensive_area_direction(Ped ped, float p1, float p2, float p3, bool p4) { return _i<Void>(0xA98490, ped, p1, p2, p3, p4); }
	static Void set_ped_defensive_sphere_attached_to_ped(Ped ped, Ped target, float xOffset, float yOffset, float zOffset, float radius, bool p6) { return _i<Void>(0xA983F0, ped, target, xOffset, yOffset, zOffset, radius, p6); }
	static Void set_ped_density_multiplier_this_frame(float multiplier) { return _i<Void>(0xA90F10, multiplier); }
	static Void set_ped_desired_heading(Ped ped, float heading) { return _i<Void>(0xA90360, ped, heading); }
	static Void set_ped_desired_move_blend_ratio(Ped ped, float p1) { return _i<Void>(0xAF4A10, ped, p1); }
	static Void set_ped_dies_instantly_in_water(Ped ped, bool toggle) { return _i<Void>(0xA9B490, ped, toggle); }
	static Void set_ped_dies_in_sinking_vehicle(Ped ped, bool toggle) { return _i<Void>(0xA918F0, ped, toggle); }
	static Void set_ped_dies_in_vehicle(Ped ped, bool toggle) { return _i<Void>(0xA94FC0, ped, toggle); }
	static Void set_ped_dies_in_water(Ped ped, bool toggle) { return _i<Void>(0xA91840, ped, toggle); }
	static Void set_ped_dies_when_injured(Ped ped, bool toggle) { return _i<Void>(0xA98960, ped, toggle); }
	static Void set_ped_drive_by_clipset_override(Ped ped, const char* clipset) { return _i<Void>(0xA95740, ped, clipset); }
	static Void set_ped_drops_inventory_weapon(Ped ped, uint32_t weaponHash, float xOffset, float yOffset, float zOffset, Any p5) { return _i<Void>(0xB39410, ped, weaponHash, xOffset, yOffset, zOffset, p5); }
	static Void set_ped_drops_weapon(Ped ped) { return _i<Void>(0xB39400, ped); }
	static Void set_ped_drops_weapons_when_dead(Ped ped, bool toggle) { return _i<Void>(0xB391F0, ped, toggle); }
	static Void set_ped_ducking(Ped ped, bool toggle) { return _i<Void>(0xA925D0, ped, toggle); }
	static Void set_ped_enable_weapon_blocking(Ped ped, bool toggle) { return _i<Void>(0xA989D0, ped, toggle); }
	static Void set_ped_enveff_scale(Ped ped, float value) { return _i<Void>(0xA9C500, ped, value); }
	static Void set_ped_firing_pattern(Ped ped, uint32_t patternHash) { return _i<Void>(0xA93E40, ped, patternHash); }
	static Void set_ped_flee_attributes(Ped ped, int attributes, bool p2) { return _i<Void>(0xA9BBD0, ped, attributes, p2); }
	static Void set_ped_gadget(Ped ped, uint32_t gadgetHash, bool p2) { return _i<Void>(0xB39D00, ped, gadgetHash, p2); }
	static Void set_ped_generates_dead_body_events(Ped ped, bool toggle) { return _i<Void>(0xA9AC20, ped, toggle); }
	static Void set_ped_gesture_group(Ped ped, const char* animGroupGesture) { return _i<Void>(0xA95B00, ped, animGroupGesture); }
	static Void set_ped_get_out_upside_down_vehicle(Ped ped, bool toggle) { return _i<Void>(0xA95570, ped, toggle); }
	static Void set_ped_gravity(Ped ped, bool toggle) { return _i<Void>(0xA943A0, ped, toggle); }
	static Void set_ped_group_member_passenger_index(Ped ped, int index) { return _i<Void>(0xA9A9C0, ped, index); }
	static Void set_ped_head_blend_data(Ped ped, int shapeFirstID, int shapeSecondID, int shapeThirdID, int skinFirstID, int skinSecondID, int skinThirdID, float shapeMix, float skinMix, float thirdMix, bool isParent) { return _i<Void>(0xA96350, ped, shapeFirstID, shapeSecondID, shapeThirdID, skinFirstID, skinSecondID, skinThirdID, shapeMix, skinMix, thirdMix, isParent); }
	static Void set_ped_head_overlay(Ped ped, int overlayID, int index, float opacity) { return _i<Void>(0xA966D0, ped, overlayID, index, opacity); }
	static Void set_ped_hearing_range(Ped ped, float value) { return _i<Void>(0xA929A0, ped, value); }
	static Void set_ped_helmet(Ped ped, bool canWearHelmet) { return _i<Void>(0xA9ADF0, ped, canWearHelmet); }
	static Void set_ped_helmet_flag(Ped ped, int helmetFlag) { return _i<Void>(0xA9AE40, ped, helmetFlag); }
	static Void set_ped_helmet_prop_index(Ped ped, int propIndex, Any p2) { return _i<Void>(0xA9AE90, ped, propIndex, p2); }
	static Void set_ped_helmet_texture_index(Ped ped, int textureIndex) { return _i<Void>(0xA9B030, ped, textureIndex); }
	static Void set_ped_id_range(Ped ped, float value) { return _i<Void>(0xA92750, ped, value); }
	static Void set_ped_infinite_ammo(Ped ped, bool toggle, uint32_t weaponHash) { return _i<Void>(0xB38EC0, ped, toggle, weaponHash); }
	static Void set_ped_infinite_ammo_clip(Ped ped, bool toggle) { return _i<Void>(0xB38EE0, ped, toggle); }
	static Void set_ped_into_vehicle(Ped ped, Vehicle vehicle, int seatIndex) { return _i<Void>(0xA90A10, ped, vehicle, seatIndex); }
	static Void set_ped_in_vehicle_context(Ped ped, uint32_t context) { return _i<Void>(0xA95850, ped, context); }
	static Void set_ped_keep_task(Ped ped, bool toggle) { return _i<Void>(0xA94DB0, ped, toggle); }
	static Void set_ped_leg_ik_mode(Ped ped, int mode) { return _i<Void>(0xA9B370, ped, mode); }
	static Void set_ped_lod_multiplier(Ped ped, float multiplier) { return _i<Void>(0xA9DD80, ped, multiplier); }
	static Void set_ped_max_health(Ped ped, int value) { return _i<Void>(0xA94890, ped, value); }
	static Void set_ped_max_move_blend_ratio(Ped ped, float value) { return _i<Void>(0xA9D010, ped, value); }
	static Void set_ped_max_time_in_water(Ped ped, float value) { return _i<Void>(0xA94950, ped, value); }
	static Void set_ped_max_time_underwater(Ped ped, float value) { return _i<Void>(0xA949D0, ped, value); }
	static Void set_ped_min_ground_time_for_stungun(Ped ped, int ms) { return _i<Void>(0xA92F80, ped, ms); }
	static Void set_ped_min_move_blend_ratio(Ped ped, float value) { return _i<Void>(0xA9D060, ped, value); }
	static Void set_ped_model_is_suppressed(Ped ped, bool toggle) { return _i<Void>(0xA9AB30, ped, toggle); }
	static Void set_ped_money(Ped ped, int amount) { return _i<Void>(0xA911D0, ped, amount); }
	static Void set_ped_motion_blur(Ped ped, bool toggle) { return _i<Void>(0xA9B3D0, ped, toggle); }
	static Void set_ped_movement_clipset(Ped ped, const char* clipSet, float p2) { return _i<Void>(0xA955D0, ped, clipSet, p2); }
	static Void set_ped_move_anims_blend_out(Ped ped) { return _i<Void>(0xA90CB0, ped); }
	static Void set_ped_move_rate_override(Ped ped, float value) { return _i<Void>(0xA9D0C0, ped, value); }
	static Void set_ped_name_debug(Ped ped, const char* name) { return _i<Void>(0xA98870, ped, name); }
	static Void set_ped_never_leaves_group(Ped ped, bool toggle) { return _i<Void>(0xA94710, ped, toggle); }
	static Void set_ped_non_creation_area(float x1, float y1, float z1, float x2, float y2, float z2) { return _i<Void>(0xA90FB0, x1, y1, z1, x2, y2, z2); }
	static Void set_ped_parachute_tint_index(Ped ped, int tintIndex) { return _i<Void>(0xA92450, ped, tintIndex); }
	static Void set_ped_paths_in_area(float x1, float y1, float z1, float x2, float y2, float z2, bool unknown, Any p7) { return _i<Void>(0xA6F570, x1, y1, z1, x2, y2, z2, unknown, p7); }
	static Void set_ped_path_avoid_fire(Ped ped, bool avoidFire) { return _i<Void>(0xAF2D30, ped, avoidFire); }
	static Void set_ped_path_can_drop_from_height(Ped ped, bool Toggle) { return _i<Void>(0xAF2BA0, ped, Toggle); }
	static Void set_ped_path_can_use_climbovers(Ped ped, bool Toggle) { return _i<Void>(0xAF2AE0, ped, Toggle); }
	static Void set_ped_path_can_use_ladders(Ped ped, bool Toggle) { return _i<Void>(0xAF2B40, ped, Toggle); }
	static Void set_ped_path_may_enter_water(Ped ped, bool mayEnterWater) { return _i<Void>(0xAF2C70, ped, mayEnterWater); }
	static Void set_ped_path_prefer_to_avoid_water(Ped ped, bool avoidWater) { return _i<Void>(0xAF2CD0, ped, avoidWater); }
	static Any set_ped_pinned_down(Ped ped, bool pinned, int i) { return _i<Any>(0xA93410, ped, pinned, i); }
	static Void set_ped_population_budget(int p0) { return _i<Void>(0xAD9DB0, p0); }
	static Void set_ped_preferred_cover_set(Ped ped, Any itemSet) { return _i<Void>(0xA986A0, ped, itemSet); }
	static Void set_ped_primary_lookat(Ped ped, Ped lookAt) { return _i<Void>(0xA9A6F0, ped, lookAt); }
	static Void set_ped_prop_index(Ped ped, int componentId, int drawableId, int TextureId, bool attach) { return _i<Void>(0xA97450, ped, componentId, drawableId, TextureId, attach); }
	static Void set_ped_ragdoll_force_fall(Ped ped) { return _i<Void>(0xA98000, ped); }
	static Void set_ped_ragdoll_on_collision(Ped ped, bool toggle) { return _i<Void>(0xA97EE0, ped, toggle); }
	static Void set_ped_random_component_variation(Ped ped, bool p1) { return _i<Void>(0xA96160, ped, p1); }
	static Void set_ped_random_props(Ped ped) { return _i<Void>(0xA96200, ped); }
	static Void set_ped_relationship_group_default_hash(Ped ped, uint32_t hash) { return _i<Void>(0xA93820, ped, hash); }
	static Void set_ped_relationship_group_hash(Ped ped, uint32_t hash) { return _i<Void>(0xA93880, ped, hash); }
	static Void set_ped_reserve_parachute_tint_index(Ped ped, Any p1) { return _i<Void>(0xA924F0, ped, p1); }
	static Void set_ped_reset_flag(Ped ped, int flagId, bool doReset) { return _i<Void>(0xA9A8A0, ped, flagId, doReset); }
	static Void set_ped_seeing_range(Ped ped, float value) { return _i<Void>(0xA92940, ped, value); }
	static Void set_ped_shoots_at_coord(Ped ped, float x, float y, float z, bool toggle) { return _i<Void>(0xA9AB10, ped, x, y, z, toggle); }
	static Void set_ped_shoot_rate(Ped ped, int shootRate) { return _i<Void>(0xA93E50, ped, shootRate); }
	static Void set_ped_sphere_defensive_area(Ped ped, float x, float y, float z, float radius, bool p5, bool p6) { return _i<Void>(0xA98300, ped, x, y, z, radius, p5, p6); }
	static Void set_ped_stay_in_vehicle_when_jacked(Ped ped, bool toggle) { return _i<Void>(0xA91990, ped, toggle); }
	static Void set_ped_stealth_movement(Ped ped, bool p1, const char* action) { return _i<Void>(0xA92CA0, ped, p1, action); }
	static Void set_ped_steers_around_objects(Ped ped, bool toggle) { return _i<Void>(0xA9BD50, ped, toggle); }
	static Void set_ped_steers_around_peds(Ped ped, bool toggle) { return _i<Void>(0xA9BD00, ped, toggle); }
	static Void set_ped_steers_around_vehicles(Ped ped, bool toggle) { return _i<Void>(0xA9BDB0, ped, toggle); }
	static Void set_ped_strafe_clipset(Ped ped, const char* clipSet) { return _i<Void>(0xA95660, ped, clipSet); }
	static Void set_ped_suffers_critical_hits(Ped ped, bool toggle) { return _i<Void>(0xA91310, ped, toggle); }
	static Void set_ped_sweat(Ped ped, float sweat) { return _i<Void>(0xA99580, ped, sweat); }
	static Void set_ped_target_loss_response(Ped ped, int responseType) { return _i<Void>(0xA9B7F0, ped, responseType); }
	static Void set_ped_to_inform_respected_friends(Ped ped, float radius, int maxFriends) { return _i<Void>(0xA93CE0, ped, radius, maxFriends); }
	static bool set_ped_to_ragdoll(Ped ped, int time1, int time2, int ragdollType, bool p4, bool p5, bool p6) { return _i<bool>(0xA97E30, ped, time1, time2, ragdollType, p4, p5, p6); }
	static Void set_ped_using_action_mode(Ped ped, bool p1, Any p2, const char* action) { return _i<Void>(0xA9D340, ped, p1, p2, action); }
	static Void set_ped_visual_field_center_angle(Ped ped, float angle) { return _i<Void>(0xA92BE0, ped, angle); }
	static Void set_ped_visual_field_max_angle(Ped ped, float value) { return _i<Void>(0xA92A60, ped, value); }
	static Void set_ped_visual_field_max_elevation_angle(Ped ped, float angle) { return _i<Void>(0xA92B20, ped, angle); }
	static Void set_ped_visual_field_min_angle(Ped ped, float value) { return _i<Void>(0xA92A00, ped, value); }
	static Void set_ped_visual_field_min_elevation_angle(Ped ped, float angle) { return _i<Void>(0xA92AC0, ped, angle); }
	static Void set_ped_visual_field_peripheral_range(Ped ped, float range) { return _i<Void>(0xA92B80, ped, range); }
	static Any set_ped_waypoint_route_offset(Any p0, Any p1, Any p2, Any p3) { return _i<Any>(0xAF6D70, p0, p1, p2, p3); }
	static Void set_ped_weapon_movement_clipset(Ped ped, const char* clipSet) { return _i<Void>(0xA956D0, ped, clipSet); }
	static Void set_ped_weapon_tint_index(Ped ped, uint32_t weaponHash, int tintIndex) { return _i<Void>(0xB3A5E0, ped, weaponHash, tintIndex); }
	static Void set_ped_wetness_enabled_this_frame(Ped ped) { return _i<Void>(0xA99490, ped); }
	static Void set_ped_wetness_height(Ped ped, float height) { return _i<Void>(0xA99420, ped, height); }
	static Void set_pickup_regeneration_time(Pickup pickup, int time) { return _i<Void>(0xA663A0, pickup, time); }
	static Void set_playback_speed(Vehicle vehicle, float speed) { return _i<Void>(0xB1FE30, vehicle, speed); }
	static Void set_playback_to_use_ai(Vehicle vehicle, int flag) { return _i<Void>(0xB1FF10, vehicle, flag); }
	static Void set_playback_to_use_ai_try_to_revert_back_later(Any p0, Any p1, Any p2, bool p3) { return _i<Void>(0xB1FF60, p0, p1, p2, p3); }
	static Void set_playerpad_shakes_when_controller_disabled(bool toggle) { return _i<Void>(0xA6B1B0, toggle); }
	static Void set_players_last_vehicle(Vehicle vehicle) { return _i<Void>(0xB2A600, vehicle); }
	static Void set_player_angry(Ped playerPed, bool disabled) { return _i<Void>(0x983930, playerPed, disabled); }
	static Void set_player_can_be_hassled_by_gangs(Player player, bool toggle) { return _i<Void>(0xAAA840, player, toggle); }
	static Void set_player_can_do_drive_by(Player player, bool toggle) { return _i<Void>(0xAAA7F0, player, toggle); }
	static Void set_player_can_leave_parachute_smoke_trail(Player player, bool enabled) { return _i<Void>(0xAACD10, player, enabled); }
	static Void set_player_can_use_cover(Player player, bool toggle) { return _i<Void>(0xAAA890, player, toggle); }
	static Void set_player_cloth_lock_counter(int value) { return _i<Void>(0xAAD320, value); }
	static Void set_player_cloth_package_index(int index) { return _i<Void>(0xAAD2F0, index); }
	static Void set_player_cloth_pin_frames(Player player, bool toggle) { return _i<Void>(0xAAD2B0, player, toggle); }
	static Void set_player_control(Player player, bool toggle, int possiblyFlags) { return _i<Void>(0xAA9C60, player, toggle, possiblyFlags); }
	static Void set_player_forced_aim(Player player, bool toggle) { return _i<Void>(0xAAB940, player, toggle); }
	static Void set_player_forced_zoom(Player player, bool toggle) { return _i<Void>(0xAAB980, player, toggle); }
	static Void set_player_force_skip_aim_intro(Player player, bool toggle) { return _i<Void>(0xAAB9D0, player, toggle); }
	static Void set_player_has_reserve_parachute(Player player) { return _i<Void>(0xAACCA0, player); }
	static Void set_player_health_recharge_multiplier(Player player, float regenRate) { return _i<Void>(0xAAC670, player, regenRate); }
	static Void set_player_invincible(Player player, bool toggle) { return _i<Void>(0xAAB320, player, toggle); }
	static Void set_player_invisible_locally(Player player, bool toggle) { return _i<Void>(0xA52EC0, player, toggle); }
	static Void set_player_lockon(Player player, bool toggle) { return _i<Void>(0xAAB560, player, toggle); }
	static Void set_player_lockon_range_override(Player player, float range) { return _i<Void>(0xAAA7A0, player, range); }
	static Void set_player_max_armour(Player player, int value) { return _i<Void>(0xAABB90, player, value); }
	static Void set_player_may_not_enter_any_vehicle(Player player) { return _i<Void>(0xAAB1B0, player); }
	static Void set_player_may_only_enter_this_vehicle(Player player, Vehicle vehicle) { return _i<Void>(0xAAB1A0, player, vehicle); }
	static Void set_player_melee_weapon_damage_modifier(Player player, float modifier, Any p2) { return _i<Void>(0xAAC8E0, player, modifier, p2); }
	static Void set_player_melee_weapon_defense_modifier(Player player, float modifier) { return _i<Void>(0xAAC940, player, modifier); }
	static Void set_player_model(Player player, uint32_t model) { return _i<Void>(0xAA93A0, player, model); }
	static Void set_player_noise_multiplier(Player player, float multiplier) { return _i<Void>(0xAACEC0, player, multiplier); }
	static Void set_player_parachute_model_override(Player player, uint32_t model) { return _i<Void>(0xAAD770, player, model); }
	static Void set_player_parachute_pack_model_override(Player player, uint32_t model) { return _i<Void>(0xAAD8F0, player, model); }
	static Void set_player_parachute_pack_tint_index(Player player, int tintIndex) { return _i<Void>(0xAACC20, player, tintIndex); }
	static Void set_player_parachute_smoke_trail_color(Player player, int r, int g, int b) { return _i<Void>(0xAACD60, player, r, g, b); }
	static Void set_player_parachute_tint_index(Player player, int tintIndex) { return _i<Void>(0xAACB20, player, tintIndex); }
	static Void set_player_parachute_variation_override(Player player, int p1, Any p2, Any p3, bool p4) { return _i<Void>(0xAAD6D0, player, p1, p2, p3, p4); }
	static Void set_player_reserve_parachute_tint_index(Player player, int index) { return _i<Void>(0xAACBA0, player, index); }
	static Void set_player_simulate_aiming(Player player, bool toggle) { return _i<Void>(0xAAD270, player, toggle); }
	static Void set_player_sneaking_noise_multiplier(Player player, float multiplier) { return _i<Void>(0xAACF10, player, multiplier); }
	static Void set_player_sprint(Player player, bool toggle) { return _i<Void>(0xAAA940, player, toggle); }
	static Void set_player_stealth_perception_modifier(Player player, float value) { return _i<Void>(0xAAD1A0, player, value); }
	static Void set_player_switch_outro(float p0, float p1, float p2, float p3, float p4, float p5, float p6, float p7, Any p8) { return _i<Void>(0xADA6E0, p0, p1, p2, p3, p4, p5, p6, p7, p8); }
	static Void set_player_targeting_mode(int targetMode) { return _i<Void>(0xAAB5A0, targetMode); }
	static Void set_player_team(Player player, int team) { return _i<Void>(0xAA94B0, player, team); }
	static Void set_player_vehicle_damage_modifier(Player player, float damageAmount) { return _i<Void>(0xAAC990, player, damageAmount); }
	static Void set_player_vehicle_defense_modifier(Player player, float modifier) { return _i<Void>(0xAAC9E0, player, modifier); }
	static Void set_player_visible_locally(Player player, bool toggle) { return _i<Void>(0xA52F60, player, toggle); }
	static Void set_player_wanted_centre_position(Player player, float x, float y, float z) { return _i<Void>(0xAA9700, player, x, y, z); }
	static Void set_player_wanted_level(Player player, int wantedLevel, bool disableNoMission) { return _i<Void>(0xAA97A0, player, wantedLevel, disableNoMission); }
	static Void set_player_wanted_level_now(Player player, bool p1) { return _i<Void>(0xAA9880, player, p1); }
	static Void set_player_wanted_level_no_drop(Player player, int wantedLevel, bool p2) { return _i<Void>(0xAA97C0, player, wantedLevel, p2); }
	static Void set_player_weapon_damage_modifier(Player player, float damageAmount) { return _i<Void>(0xAAC7F0, player, damageAmount); }
	static Void set_player_weapon_defense_modifier(Player player, float modifier) { return _i<Void>(0xAAC840, player, modifier); }
	static Void set_police_ignore_player(Player player, bool toggle) { return _i<Void>(0xAA9D10, player, toggle); }
	static Void set_police_radar_blips(bool toggle) { return _i<Void>(0xAA9CF0, toggle); }
	static Void set_radar_as_exterior_this_frame() { return _i<Void>(0x9E3530); }
	static Void set_radar_as_interior_this_frame(uint32_t interior, float x, float y, int z, int zoom) { return _i<Void>(0x9E34A0, interior, x, y, z, zoom); }
	static Void set_radar_zoom(int zoomLevel) { return _i<Void>(0x9E1420, zoomLevel); }
	static Void set_radio_auto_unfreeze(bool toggle) { return _i<Void>(0x984960, toggle); }
	static Void set_radio_to_station_index(int radioStation) { return _i<Void>(0x9847F0, radioStation); }
	static Void set_radio_to_station_name(const char* stationName) { return _i<Void>(0x984560, stationName); }
	static Void set_radio_track(const char* radioStation, const char* radioTrack) { return _i<Void>(0x984A00, radioStation, radioTrack); }
	static Void set_random_boats(bool toggle) { return _i<Void>(0xB1F5C0, toggle); }
	static Void set_random_event_flag(bool p0) { return _i<Void>(0x9F5870, p0); }
	static Void set_random_trains(bool toggle) { return _i<Void>(0xB1F3B0, toggle); }
	static Void set_random_vehicle_density_multiplier_this_frame(float multiplier) { return _i<Void>(0xB1BE40, multiplier); }
	static Void set_reduce_ped_model_budget(bool toggle) { return _i<Void>(0xAD9C90, toggle); }
	static Void set_reduce_vehicle_model_budget(bool toggle) { return _i<Void>(0xAD9CB0, toggle); }
	static Void set_relationship_between_groups(int relationship, uint32_t group1, uint32_t group2) { return _i<Void>(0xA93890, relationship, group1, group2); }
	static Void set_render_train_as_derailed(Vehicle train, bool toggle) { return _i<Void>(0xB22160, train, toggle); }
	static Void set_roads_back_to_original(float p0, float p1, float p2, float p3, float p4, float p5, Any p6) { return _i<Void>(0xA70220, p0, p1, p2, p3, p4, p5, p6); }
	static Void set_roads_back_to_original_in_angled_area(float x1, float y1, float z1, float x2, float y2, float z2, float p6, Any p7) { return _i<Void>(0xA70240, x1, y1, z1, x2, y2, z2, p6, p7); }
	static Void set_roads_in_angled_area(float x1, float y1, float z1, float x2, float y2, float z2, float angle, bool unknown1, bool unknown2, bool unknown3) { return _i<Void>(0xA6F530, x1, y1, z1, x2, y2, z2, angle, unknown1, unknown2, unknown3); }
	static Void set_roads_in_area(float x1, float y1, float z1, float x2, float y2, float z2, bool unknown1, bool unknown2) { return _i<Void>(0xA6F500, x1, y1, z1, x2, y2, z2, unknown1, unknown2); }
	static Void set_run_sprint_multiplier_for_player(Player player, float multiplier) { return _i<Void>(0xAAB800, player, multiplier); }
	static Void set_save_house(Any p0, bool p1, bool p2) { return _i<Void>(0x9F7DE0, p0, p1, p2); }
	static Void set_save_menu_active(bool unk) { return _i<Void>(0x9F76C0, unk); }
	static Void set_scaleform_movie_as_no_longer_needed(int* scaleformHandle) { return _i<Void>(0x9D1020, scaleformHandle); }
	static Void set_scaleform_movie_to_use_system_time(int scaleform, bool toggle) { return _i<Void>(0x9D10A0, scaleform, toggle); }
	static Void set_scenario_group_enabled(const char* scenarioGroup, bool p1) { return _i<Void>(0xAF5A00, scenarioGroup, p1); }
	static Void set_scenario_peds_spawn_in_sphere_area(Any p0, Any p1, Any p2, Any p3, Any p4) { return _i<Void>(0xA99A30, p0, p1, p2, p3, p4); }
	static Void set_scenario_peds_to_be_returned_by_next_command(bool value) { return _i<Void>(0xA97B50, value); }
	static Void set_scenario_ped_density_multiplier_this_frame(float p0, float p1) { return _i<Void>(0xA90F20, p0, p1); }
	static Void set_scenario_type_enabled(const char* scenarioType, bool toggle) { return _i<Void>(0xAF5BD0, scenarioType, toggle); }
	static Void set_scripted_anim_seat_offset(Ped ped, float p1) { return _i<Void>(0xA9B580, ped, p1); }
	static Void set_scripted_conversion_coord_this_frame(float x, float y, float z) { return _i<Void>(0xA90F50, x, y, z); }
	static Void set_script_as_no_longer_needed(const char* scriptName) { return _i<Void>(0xAB0EC0, scriptName); }
	static Void set_script_vehicle_generator(Any vehicleGenerator, bool enabled) { return _i<Void>(0xB1B6A0, vehicleGenerator, enabled); }
	static Void set_script_with_name_hash_as_no_longer_needed(uint32_t scriptHash) { return _i<Void>(0xAB10A0, scriptHash); }
	static Void set_seethrough(bool toggle) { return _i<Void>(0x9CE690, toggle); }
	static Void set_siren_with_no_driver(Vehicle vehicle, bool toggle) { return _i<Void>(0x985BA0, vehicle, toggle); }
	static Void set_special_ability_multiplier(float multiplier) { return _i<Void>(0xAAC390, multiplier); }
	static Void set_srl_time(float p0) { return _i<Void>(0xADAD90, p0); }
	static Void set_state_of_closest_door_of_type(uint32_t type, float x, float y, float z, bool locked, float heading, bool p6) { return _i<Void>(0xA64220, type, x, y, z, locked, heading, p6); }
	static Void set_static_emitter_enabled(const char* emitterName, bool toggle) { return _i<Void>(0x984760, emitterName, toggle); }
	static Void set_store_enabled(bool toggle) { return _i<Void>(0xA570C0, toggle); }
	static Void set_streamed_texture_dict_as_no_longer_needed(const char* textureDict) { return _i<Void>(0x9CCA60, textureDict); }
	static Void set_streaming(bool toggle) { return _i<Void>(0xAD9C20, toggle); }
	static Void set_super_jump_this_frame(Player player) { return _i<Void>(0x9FAB20, player); }
	static Void set_swim_multiplier_for_player(Player player, float multiplier) { return _i<Void>(0xAAB7A0, player, multiplier); }
	static Void set_synchronized_scene_looped(int sceneID, bool toggle) { return _i<Void>(0xA9CD60, sceneID, toggle); }
	static Void set_synchronized_scene_phase(int sceneID, float phase) { return _i<Void>(0xA9CC30, sceneID, phase); }
	static Void set_synchronized_scene_rate(int sceneID, float rate) { return _i<Void>(0xA9CCB0, sceneID, rate); }
	static Void set_task_vehicle_chase_behavior_flag(Ped ped, int flag, bool set) { return _i<Void>(0xAF4060, ped, flag, set); }
	static Void set_task_vehicle_chase_ideal_pursuit_distance(Ped ped, float distance) { return _i<Void>(0xAF40F0, ped, distance); }
	static Void set_taxi_lights(Vehicle vehicle, bool state) { return _i<Void>(0xB1C740, vehicle, state); }
	static Void set_team_pickup_object(Object object, Any p1, bool p2) { return _i<Void>(0xA664F0, object, p1, p2); }
	static Void set_text_centre(bool align) { return _i<Void>(0x9E1930, align); }
	static Void set_text_colour(int red, int green, int blue, int alpha) { return _i<Void>(0x9E18F0, red, green, blue, alpha); }
	static Void set_text_dropshadow(int distance, int r, int g, int b, int a) { return _i<Void>(0x9E1A20, distance, r, g, b, a); }
	static Void set_text_drop_shadow() { return _i<Void>(0x9E1A10); }
	static Void set_text_edge(int p0, int r, int g, int b, int a) { return _i<Void>(0x9E1A50, p0, r, g, b, a); }
	static Void set_text_font(int fontType) { return _i<Void>(0x9E19F0, fontType); }
	static Void set_text_justification(int justifyType) { return _i<Void>(0x9E1970, justifyType); }
	static Void set_text_leading(bool p0) { return _i<Void>(0x9E19C0, p0); }
	static Void set_text_outline() { return _i<Void>(0x9E1A40); }
	static Void set_text_proportional(bool p0) { return _i<Void>(0x9E19E0, p0); }
	static Void set_text_render_id(int renderId) { return _i<Void>(0x9E1A60, renderId); }
	static Void set_text_right_justify(bool toggle) { return _i<Void>(0x9E1950, toggle); }
	static Void set_text_scale(float p0, float size) { return _i<Void>(0x9E18D0, p0, size); }
	static Void set_text_wrap(float start, float end) { return _i<Void>(0x9E19A0, start, end); }
	static Void set_this_script_can_be_paused(bool toggle) { return _i<Void>(0x9F9700, toggle); }
	static Void set_this_script_can_remove_blips_created_by_any_script(bool toggle) { return _i<Void>(0x9F9730, toggle); }
	static Void set_timecycle_modifier(const char* modifierName) { return _i<Void>(0x9D07B0, modifierName); }
	static Void set_timecycle_modifier_strength(float strength) { return _i<Void>(0x9D0830, strength); }
	static Void set_time_scale(float time) { return _i<Void>(0x9F57C0, time); }
	static Void set_tracked_point_info(Object point, float x, float y, float z, float radius) { return _i<Void>(0x9CDF20, point, x, y, z, radius); }
	static Void set_train_cruise_speed(Vehicle train, float speed) { return _i<Void>(0xB1F550, train, speed); }
	static Void set_transition_timecycle_modifier(const char* modifierName, float transition) { return _i<Void>(0x9D0850, modifierName, transition); }
	static Void set_tv_audio_frontend(bool toggle) { return _i<Void>(0x9CD9D0, toggle); }
	static Void set_tv_channel(int channel) { return _i<Void>(0x9D1BA0, channel); }
	static Void set_tv_volume(float volume) { return _i<Void>(0x9D1BD0, volume); }
	static Void set_user_radio_control_enabled(bool toggle) { return _i<Void>(0x9849B0, toggle); }
	static Void set_use_hi_dof() { return _i<Void>(0x992160); }
	static Void set_variable_on_sound(int soundId, Any* p1, float p2) { return _i<Void>(0x983750, soundId, p1, p2); }
	static Void set_variable_on_stream(const char* p0, float p1) { return _i<Void>(0x983770, p0, p1); }
	static Void set_vehicle_alarm(Vehicle vehicle, bool state) { return _i<Void>(0xB1EB00, vehicle, state); }
	static Void set_vehicle_allow_no_passengers_lockon(Vehicle veh, bool toggle) { return _i<Void>(0xB1B190, veh, toggle); }
	static Void set_vehicle_as_no_longer_needed(Vehicle* vehicle) { return _i<Void>(0x9B56E0, vehicle); }
	static Any set_vehicle_automatically_attaches(Vehicle vehicle, Any p1, Any p2) { return _i<Any>(0xB24D30, vehicle, p1, p2); }
	static Void set_vehicle_boost_active(Vehicle vehicle, bool Toggle) { return _i<Void>(0x986380, vehicle, Toggle); }
	static Void set_vehicle_brake_lights(Vehicle vehicle, bool toggle) { return _i<Void>(0xB25430, vehicle, toggle); }
	static Void set_vehicle_burnout(Vehicle vehicle, bool toggle) { return _i<Void>(0xB25170, vehicle, toggle); }
	static Void set_vehicle_can_be_targetted(Vehicle vehicle, bool state) { return _i<Void>(0xB20E30, vehicle, state); }
	static Void set_vehicle_can_be_used_by_fleeing_peds(Vehicle vehicle, bool toggle) { return _i<Void>(0xB2A650, vehicle, toggle); }
	static Void set_vehicle_can_be_visibly_damaged(Vehicle vehicle, bool state) { return _i<Void>(0xB20FD0, vehicle, state); }
	static Void set_vehicle_can_break(Vehicle vehicle, bool toggle) { return _i<Void>(0xB216B0, vehicle, toggle); }
	static Void set_vehicle_ceiling_height(Vehicle vehicle, float p1) { return _i<Void>(0xB2ABD0, vehicle, p1); }
	static Void set_vehicle_colours(Vehicle vehicle, int colorPrimary, int colorSecondary) { return _i<Void>(0xB1C8B0, vehicle, colorPrimary, colorSecondary); }
	static Void set_vehicle_colour_combination(Vehicle vehicle, int colorCombination) { return _i<Void>(0xB21910, vehicle, colorCombination); }
	static Void set_vehicle_custom_primary_colour(Vehicle vehicle, int r, int g, int b) { return _i<Void>(0xB1CAA0, vehicle, r, g, b); }
	static Void set_vehicle_custom_secondary_colour(Vehicle vehicle, int r, int g, int b) { return _i<Void>(0xB1CCA0, vehicle, r, g, b); }
	static Void set_vehicle_damage(Vehicle vehicle, float xOffset, float yOffset, float zOffset, float damage, float radius, bool p6) { return _i<Void>(0xB230C0, vehicle, xOffset, yOffset, zOffset, damage, radius, p6); }
	static Void set_vehicle_deformation_fixed(Vehicle vehicle) { return _i<Void>(0xB22480, vehicle); }
	static Void set_vehicle_density_multiplier_this_frame(float multiplier) { return _i<Void>(0xB1BE10, multiplier); }
	static Void set_vehicle_dirt_level(Vehicle vehicle, float dirtLevel) { return _i<Void>(0xB21250, vehicle, dirtLevel); }
	static Void set_vehicle_doors_locked(Vehicle vehicle, int doorLockStatus) { return _i<Void>(0xB1BF10, vehicle, doorLockStatus); }
	static Void set_vehicle_doors_locked_for_all_players(Vehicle vehicle, bool toggle) { return _i<Void>(0xB1C120, vehicle, toggle); }
	static Void set_vehicle_doors_locked_for_player(Vehicle vehicle, Player player, bool toggle) { return _i<Void>(0xB1C090, vehicle, player, toggle); }
	static Void set_vehicle_doors_locked_for_team(Vehicle vehicle, int team, bool toggle) { return _i<Void>(0xB1C230, vehicle, team, toggle); }
	static Void set_vehicle_doors_shut(Vehicle vehicle, bool closeInstantly) { return _i<Void>(0xB1E3E0, vehicle, closeInstantly); }
	static Void set_vehicle_door_broken(Vehicle vehicle, int doorIndex, bool deleteDoor) { return _i<Void>(0xB21690, vehicle, doorIndex, deleteDoor); }
	static Void set_vehicle_door_control(Vehicle vehicle, int doorIndex, int speed, float angle) { return _i<Void>(0xB21550, vehicle, doorIndex, speed, angle); }
	static Void set_vehicle_door_latched(Vehicle vehicle, int doorIndex, bool p2, bool p3, bool p4) { return _i<Void>(0xB21570, vehicle, doorIndex, p2, p3, p4); }
	static Void set_vehicle_door_open(Vehicle vehicle, int doorIndex, bool loose, bool openInstantly) { return _i<Void>(0xB1E560, vehicle, doorIndex, loose, openInstantly); }
	static Void set_vehicle_door_shut(Vehicle vehicle, int doorIndex, bool closeInstantly) { return _i<Void>(0xB21670, vehicle, doorIndex, closeInstantly); }
	static Void set_vehicle_engine_can_degrade(Vehicle vehicle, bool toggle) { return _i<Void>(0xB29290, vehicle, toggle); }
	static Void set_vehicle_engine_health(Vehicle vehicle, float health) { return _i<Void>(0xB231A0, vehicle, health); }
	static Void set_vehicle_engine_on(Vehicle vehicle, bool value, bool instantly, bool otherwise) { return _i<Void>(0xB213B0, vehicle, value, instantly, otherwise); }
	static Void set_vehicle_enveff_scale(Vehicle vehicle, float fade) { return _i<Void>(0xB1CEA0, vehicle, fade); }
	static Void set_vehicle_exclusive_driver(Vehicle vehicle, bool p1) { return _i<Void>(0xB2AD10, vehicle, p1); }
	static Void set_vehicle_explodes_on_high_explosion_damage(Vehicle vehicle, bool toggle) { return _i<Void>(0xB242E0, vehicle, toggle); }
	static Void set_vehicle_extra(Vehicle vehicle, int extraId, bool toggle) { return _i<Void>(0xB22BD0, vehicle, extraId, toggle); }
	static Void set_vehicle_extra_colours(Vehicle vehicle, int pearlescentColor, int wheelColor) { return _i<Void>(0xB22180, vehicle, pearlescentColor, wheelColor); }
	static Void set_vehicle_fixed(Vehicle vehicle) { return _i<Void>(0xB22410, vehicle); }
	static Void set_vehicle_forward_speed(Vehicle vehicle, float speed) { return _i<Void>(0xB1DC20, vehicle, speed); }
	static Void set_vehicle_friction_override(Vehicle vehicle, float friction) { return _i<Void>(0xB2AA70, vehicle, friction); }
	static Void set_vehicle_fullbeam(Vehicle vehicle, bool toggle) { return _i<Void>(0xB1C950, vehicle, toggle); }
	static Void set_vehicle_gravity(Vehicle vehicle, bool toggle) { return _i<Void>(0xB28F30, vehicle, toggle); }
	static Void set_vehicle_handbrake(Vehicle vehicle, bool toggle) { return _i<Void>(0xB25530, vehicle, toggle); }
	static Void set_vehicle_has_been_owned_by_player(Vehicle vehicle, bool owned) { return _i<Void>(0xB23560, vehicle, owned); }
	static Void set_vehicle_has_strong_axles(Vehicle vehicle, bool toggle) { return _i<Void>(0xB23910, vehicle, toggle); }
	static Void set_vehicle_indicator_lights(Vehicle vehicle, int turnSignal, bool toggle) { return _i<Void>(0xB25320, vehicle, turnSignal, toggle); }
	static Void set_vehicle_interiorlight(Vehicle vehicle, bool toggle) { return _i<Void>(0xB1EC10, vehicle, toggle); }
	static Void set_vehicle_is_considered_by_player(Vehicle vehicle, bool toggle) { return _i<Void>(0xB219F0, vehicle, toggle); }
	static Void set_vehicle_is_stolen(Vehicle vehicle, bool isStolen) { return _i<Void>(0xB29670, vehicle, isStolen); }
	static Void set_vehicle_is_wanted(Vehicle vehicle, bool state) { return _i<Void>(0xB28170, vehicle, state); }
	static Void set_vehicle_lights(Vehicle vehicle, int state) { return _i<Void>(0xB1E950, vehicle, state); }
	static Void set_vehicle_light_multiplier(Vehicle vehicle, float multiplier) { return _i<Void>(0xB1ED00, vehicle, multiplier); }
	static Void set_vehicle_livery(Vehicle vehicle, int livery) { return _i<Void>(0xB23B80, vehicle, livery); }
	static Void set_vehicle_lod_multiplier(Vehicle vehicle, float multiplier) { return _i<Void>(0xB288E0, vehicle, multiplier); }
	static Void set_vehicle_mod(Vehicle vehicle, int modType, int modIndex, bool customTires) { return _i<Void>(0xB26F80, vehicle, modType, modIndex, customTires); }
	static Void set_vehicle_model_is_suppressed(uint32_t model, bool suppressed) { return _i<Void>(0xB20280, model, suppressed); }
	static Void set_vehicle_mod_kit(Vehicle vehicle, int modKit) { return _i<Void>(0xB26830, vehicle, modKit); }
	static Void set_vehicle_name_debug(Vehicle vehicle, const char* name) { return _i<Void>(0xB242D0, vehicle, name); }
	static Void set_vehicle_needs_to_be_hotwired(Vehicle vehicle, bool toggle) { return _i<Void>(0xB23600, vehicle, toggle); }
	static Void set_vehicle_number_plate_text(Vehicle vehicle, const char* plateText) { return _i<Void>(0xB1F230, vehicle, plateText); }
	static Void set_vehicle_number_plate_text_index(Vehicle vehicle, int plateIndex) { return _i<Void>(0xB1F300, vehicle, plateIndex); }
	static bool set_vehicle_on_ground_properly(Vehicle vehicle, Any p1) { return _i<bool>(0xB1B7E0, vehicle, p1); }
	static Void set_vehicle_out_of_control(Vehicle vehicle, bool killDriver, bool explodeOnImpact) { return _i<Void>(0xB1C4A0, vehicle, killDriver, explodeOnImpact); }
	static Void set_vehicle_petrol_tank_health(Vehicle vehicle, float health) { return _i<Void>(0xB233A0, vehicle, health); }
	static Void set_vehicle_population_budget(int p0) { return _i<Void>(0xAD9D90, p0); }
	static Void set_vehicle_provides_cover(Vehicle vehicle, bool toggle) { return _i<Void>(0xB214B0, vehicle, toggle); }
	static Void set_vehicle_radio_enabled(Vehicle vehicle, bool toggle) { return _i<Void>(0x984C90, vehicle, toggle); }
	static Void set_vehicle_radio_loud(Vehicle vehicle, bool toggle) { return _i<Void>(0x984AF0, vehicle, toggle); }
	static Void set_vehicle_reduce_grip(Vehicle vehicle, bool toggle) { return _i<Void>(0xB25240, vehicle, toggle); }
	static Void set_vehicle_rudder_broken(Vehicle vehicle, bool p1) { return _i<Void>(0xB25890, vehicle, p1); }
	static Void set_vehicle_searchlight(Vehicle heli, bool toggle, bool canBeUsedByAI) { return _i<Void>(0xB26520, heli, toggle, canBeUsedByAI); }
	static Void set_vehicle_siren(Vehicle vehicle, bool toggle) { return _i<Void>(0xB1D730, vehicle, toggle); }
	static Void set_vehicle_steer_bias(Vehicle vehicle, float value) { return _i<Void>(0xB22B10, vehicle, value); }
	static Void set_vehicle_strong(Vehicle vehicle, bool toggle) { return _i<Void>(0xB1D8B0, vehicle, toggle); }
	static Void set_vehicle_timed_explosion(Vehicle vehicle, Ped ped, bool toggle) { return _i<Void>(0xB1C580, vehicle, ped, toggle); }
	static Void set_vehicle_tyres_can_burst(Vehicle vehicle, bool toggle) { return _i<Void>(0xB1E400, vehicle, toggle); }
	static Void set_vehicle_tyre_burst(Vehicle vehicle, int index, bool onRim, float p3) { return _i<Void>(0xB1E3C0, vehicle, index, onRim, p3); }
	static Void set_vehicle_tyre_fixed(Vehicle vehicle, int tyreIndex) { return _i<Void>(0xB1F1A0, vehicle, tyreIndex); }
	static Void set_vehicle_tyre_smoke_color(Vehicle vehicle, int r, int g, int b) { return _i<Void>(0xB27810, vehicle, r, g, b); }
	static Void set_vehicle_undriveable(Vehicle vehicle, bool toggle) { return _i<Void>(0xB213E0, vehicle, toggle); }
	static Void set_vehicle_wheels_can_break(Vehicle vehicle, bool enabled) { return _i<Void>(0xB1E4C0, vehicle, enabled); }
	static Void set_vehicle_wheels_can_break_off_when_blow_up(Vehicle vehicle, bool toggle) { return _i<Void>(0xB2AAC0, vehicle, toggle); }
	static Void set_vehicle_wheel_type(Vehicle vehicle, int WheelType) { return _i<Void>(0xB269D0, vehicle, WheelType); }
	static Void set_vehicle_window_tint(Vehicle vehicle, int tint) { return _i<Void>(0xB278E0, vehicle, tint); }
	static Void set_wanted_level_multiplier(float multiplier) { return _i<Void>(0xAA9FC0, multiplier); }
	static Void set_warning_message(const char* entryLine1, int instructionalKey, const char* entryLine2, bool p3, Any p4, Any* p5, Any* p6, bool background) { return _i<Void>(0x9E4C60, entryLine1, instructionalKey, entryLine2, p3, p4, p5, p6, background); }
	static Void set_waypoint_off() { return _i<Void>(0x9E3120); }
	static Void set_weapon_animation_override(Ped ped, uint32_t animStyle) { return _i<Void>(0xB3AFB0, ped, animStyle); }
	static Void set_weapon_object_tint_index(Object weapon, int tintIndex) { return _i<Void>(0xB3A630, weapon, tintIndex); }
	static Void set_weather_type_now(const char* weatherType) { return _i<Void>(0x9F5D00, weatherType); }
	static Void set_weather_type_now_persist(const char* weatherType) { return _i<Void>(0x9F5CB0, weatherType); }
	static Void set_weather_type_persist(const char* weatherType) { return _i<Void>(0x9F5C60, weatherType); }
	static Void set_widescreen_borders(bool p0, int p1) { return _i<Void>(0x994090, p0, p1); }
	static Void set_widescreen_format(Any p0) { return _i<Void>(0x9E3620, p0); }
	static Void set_wind(float speed) { return _i<Void>(0x9F6150, speed); }
	static Void set_wind_direction(float direction) { return _i<Void>(0x9F61C0, direction); }
	static Void set_wind_speed(float speed) { return _i<Void>(0x9F6170, speed); }
	static Void set_zone_enabled(int zoneId, bool toggle) { return _i<Void>(0xB3C6D0, zoneId, toggle); }
	static Void shake_cam(Cam cam, const char* type, float amplitude) { return _i<Void>(0x993600, cam, type, amplitude); }
	static Void shake_cinematic_cam(const char* p0, float p1) { return _i<Void>(0x995ED0, p0, p1); }
	static Void shoot_single_bullet_between_coords(float x1, float y1, float z1, float x2, float y2, float z2, int damage, bool p7, uint32_t weaponHash, Ped ownerPed, bool isAudible, bool isInvisible, float speed) { return _i<Void>(0x9F8240, x1, y1, z1, x2, y2, z2, damage, p7, weaponHash, ownerPed, isAudible, isInvisible, speed); }
	// [aligned] static Void shoot_single_bullet_between_coords_ignore_entity(float x1, float y1, float z1, float x2, float y2, float z2, int damage, bool p7, uint32_t weaponHash, Ped ownerPed, bool isAudible, bool isInvisible, float speed, Entity entity, Any p14) { return _i<Void>(0x9F8240, x1, y1, z1, x2, y2, z2, damage, p7, weaponHash, ownerPed, isAudible, isInvisible, speed, entity, p14); }
	static Void show_heading_indicator_on_blip(Blip blip, bool toggle) { return _i<Void>(0x9E2FD0, blip, toggle); }
	static Void show_hud_component_this_frame(int id) { return _i<Void>(0x9E4150, id); }
	static Void show_number_on_blip(Blip blip, int number) { return _i<Void>(0x9E2F00, blip, number); }
	static Void shutdown_and_launch_single_player_game() { return _i<Void>(0xA51FD0); }
	static Void shutdown_creator_budget() { return _i<Void>(0xADAED0); }
	static Void shutdown_loading_screen() { return _i<Void>(0xAB1560); }
	static Void simulate_player_input_gait(Player player, float amount, int gaitType, float speed, bool p4, bool p5) { return _i<Void>(0xAAD010, player, amount, gaitType, speed, p4, p5); }
	static Void skip_radio_forward() { return _i<Void>(0x9848F0); }
	static Void skip_time_in_playback_recorded_vehicle(Any p0, float p1) { return _i<Void>(0xB1FEB0, p0, p1); }
	static Void skip_to_end_and_stop_playback_recorded_vehicle(Any p0) { return _i<Void>(0xB1FDE0, p0); }
	static Void skip_to_next_scripted_conversation_line() { return _i<Void>(0x983040); }
	static bool slide_object(Object object, float toX, float toY, float toZ, float speedX, float speedY, float speedZ, bool collision) { return _i<bool>(0xA63CB0, object, toX, toY, toZ, speedX, speedY, speedZ, collision); }
	static Void smash_vehicle_window(Vehicle vehicle, int index) { return _i<Void>(0xB1E7C0, vehicle, index); }
	static Void special_ability_charge_absolute(Player player, int p1, bool p2) { return _i<Void>(0xAABF50, player, p1, p2); }
	static Void special_ability_charge_continuous(Player player, Ped p2) { return _i<Void>(0xAABED0, player, p2); }
	static Void special_ability_charge_large(Player player, bool p1, bool p2) { return _i<Void>(0xAABEA0, player, p1, p2); }
	static Void special_ability_charge_medium(Player player, bool p1, bool p2) { return _i<Void>(0xAABE70, player, p1, p2); }
	static Void special_ability_charge_normalized(Player player, float normalizedValue, bool p2) { return _i<Void>(0xAABFD0, player, normalizedValue, p2); }
	static Void special_ability_charge_small(Player player, bool p1, bool p2) { return _i<Void>(0xAABE40, player, p1, p2); }
	static Void special_ability_deactivate(Player player) { return _i<Void>(0xAABCB0, player); }
	static Void special_ability_deactivate_fast(Player player) { return _i<Void>(0xAABD10, player); }
	static Void special_ability_deplete_meter(Player player, bool p1) { return _i<Void>(0xAAC0E0, player, p1); }
	static Void special_ability_fill_meter(Player player, bool p1) { return _i<Void>(0xAAC060, player, p1); }
	static Void special_ability_lock(uint32_t playerModel) { return _i<Void>(0xAAC160, playerModel); }
	static Void special_ability_reset(Player player) { return _i<Void>(0xAABD80, player); }
	static Void special_ability_unlock(uint32_t playerModel) { return _i<Void>(0xAAC180, playerModel); }
	static Void start_alarm(const char* alarmName, bool p2) { return _i<Void>(0x986CD0, alarmName, p2); }
	static bool start_audio_scene(const char* scene) { return _i<bool>(0x9867B0, scene); }
	static Void start_cutscene(int p0) { return _i<Void>(0x9993B0, p0); }
	static Void start_cutscene_at_coords(float x, float y, float z, int p3) { return _i<Void>(0x9993E0, x, y, z, p3); }
	static Any start_entity_fire(Entity entity) { return _i<Any>(0x9BB100, entity); }
	static Void start_firing_amnesty(int duration) { return _i<Void>(0xAAA140, duration); }
	static int start_particle_fx_looped_at_coord(const char* effectName, float x, float y, float z, float xRot, float yRot, float zRot, float scale, bool xAxis, bool yAxis, bool zAxis, bool p11) { return _i<int>(0x9CF140, effectName, x, y, z, xRot, yRot, zRot, scale, xAxis, yAxis, zAxis, p11); }
	static int start_particle_fx_looped_on_entity(const char* effectName, Entity entity, float xOffset, float yOffset, float zOffset, float xRot, float yRot, float zRot, float scale, bool xAxis, bool yAxis, bool zAxis) { return _i<int>(0x9CF200, effectName, entity, xOffset, yOffset, zOffset, xRot, yRot, zRot, scale, xAxis, yAxis, zAxis); }
	// [aligned] static int start_particle_fx_looped_on_entity_bone(const char* effectName, Entity entity, float xOffset, float yOffset, float zOffset, float xRot, float yRot, float zRot, int boneIndex, float scale, bool xAxis, bool yAxis, bool zAxis) { return _i<int>(0x9CF120, effectName, entity, xOffset, yOffset, zOffset, xRot, yRot, zRot, boneIndex, scale, xAxis, yAxis, zAxis); }
	static int start_particle_fx_looped_on_ped_bone(const char* effectName, Ped ped, float xOffset, float yOffset, float zOffset, float xRot, float yRot, float zRot, int boneIndex, float scale, bool xAxis, bool yAxis, bool zAxis) { return _i<int>(0x9CF1A0, effectName, ped, xOffset, yOffset, zOffset, xRot, yRot, zRot, boneIndex, scale, xAxis, yAxis, zAxis); }
	static Void start_playback_recorded_vehicle(Vehicle vehicle, int p1, const char* playback, bool p3) { return _i<Void>(0xB1FB70, vehicle, p1, playback, p3); }
	static Void start_playback_recorded_vehicle_using_ai(Any p0, Any p1, Any* p2, float p3, Any p4) { return _i<Void>(0xB1FE90, p0, p1, p2, p3, p4); }
	static Void start_playback_recorded_vehicle_with_flags(Vehicle vehicle, Any p1, const char* playback, Any p3, Any p4, Any p5) { return _i<Void>(0xB1FB90, vehicle, p1, playback, p3, p4, p5); }
	static Void start_player_switch(Ped from, Ped to, int flags, int switchType) { return _i<Void>(0xADA4D0, from, to, flags, switchType); }
	static Void start_player_teleport(Player player, float x, float y, float z, float heading, bool p5, bool p6, bool p7) { return _i<Void>(0xAAC560, player, x, y, z, heading, p5, p6, p7); }
	static Void start_preloaded_conversation() { return _i<Void>(0x982ED0); }
	static Void start_rope_unwinding_front(Object rope) { return _i<Void>(0xAA0F00, rope); }
	static Void start_rope_winding(Object rope) { return _i<Void>(0xAA0E90, rope); }
	static Void start_save_data(Any* p0, Any p1, bool p2) { return _i<Void>(0x9F9850, p0, p1, p2); }
	static Void start_script_phone_conversation(bool p0, bool p1) { return _i<Void>(0x982D70, p0, p1); }
	static int start_shape_test_bound(Entity entity, int flags1, int flags2) { return _i<int>(0xAB3360, entity, flags1, flags2); }
	static int start_shape_test_bounding_box(Entity entity, int flags1, int flags2) { return _i<int>(0xAB32F0, entity, flags1, flags2); }
	static int start_shape_test_box(float x, float y, float z, float x1, float y2, float z2, float rotX, float rotY, float rotZ, Any p9, Any p10, Any entity, Any p12) { return _i<int>(0xAB3320, x, y, z, x1, y2, z2, rotX, rotY, rotZ, p9, p10, entity, p12); }
	static int start_shape_test_capsule(float x1, float y1, float z1, float x2, float y2, float z2, float radius, int flags, Entity entity, int p9) { return _i<int>(0xAB3390, x1, y1, z1, x2, y2, z2, radius, flags, entity, p9); }
	static int start_shape_test_los_probe(float x1, float y1, float z1, float x2, float y2, float z2, int flags, Entity entity, int p8) { return _i<int>(0xAB3290, x1, y1, z1, x2, y2, z2, flags, entity, p8); }
	static Void start_vehicle_alarm(Vehicle vehicle) { return _i<Void>(0xB1EB50, vehicle); }
	static Void start_vehicle_horn(Vehicle vehicle, int duration, uint32_t mode, bool forever) { return _i<Void>(0xB237E0, vehicle, duration, mode, forever); }
	static Any stat_clear_slot_for_reload(int statSlot) { return _i<Any>(0xACF000, statSlot); }
	static Any stat_delete_slot(Any p0) { return _i<Any>(0xACF1E0, p0); }
	static bool stat_get_bool(uint32_t statHash, bool* outValue, int p2) { return _i<bool>(0xACF820, statHash, outValue, p2); }
	static bool stat_get_date(uint32_t statHash, Any* p1, Any p2, Any p3) { return _i<bool>(0xACF850, statHash, p1, p2, p3); }
	static bool stat_get_float(uint32_t statHash, float* outValue, int p2) { return _i<bool>(0xACF7F0, statHash, outValue, p2); }
	static int stat_get_int(uint32_t statHash, int* outValue, int p2) { return _i<bool>(0xACF710, statHash, outValue, p2); }
	static const char* stat_get_license_plate(uint32_t statName) { return _i<const char*>(0xACF9F0, statName); }
	static bool stat_get_masked_int(Any p0, Any* p1, Any p2, Any p3, Any p4) { return _i<bool>(0xACF8F0, p0, p1, p2, p3, p4); }
	static int stat_get_number_of_days(uint32_t statName) { return _i<int>(0xACFBC0, statName); }
	static int stat_get_number_of_hours(uint32_t statName) { return _i<int>(0xACFBE0, statName); }
	static int stat_get_number_of_minutes(uint32_t statName) { return _i<int>(0xACFC00, statName); }
	static int stat_get_number_of_seconds(uint32_t statName) { return _i<int>(0xACFC20, statName); }
	static bool stat_get_pos(Any p0, Any* p1, Any* p2, Any* p3, Any p4) { return _i<bool>(0xACF8B0, p0, p1, p2, p3, p4); }
	static const char* stat_get_string(uint32_t statHash, int p1) { return _i<const char*>(0xACF880, statHash, p1); }
	static const char* stat_get_user_id(Any p0) { return _i<const char*>(0xACF9C0, p0); }
	static Void stat_increment(uint32_t statName, float value) { return _i<Void>(0xACFA50, statName, value); }
	static bool stat_load(int p0) { return _i<bool>(0xACF0B0, p0); }
	static bool stat_load_pending(Any p0) { return _i<bool>(0xACF150, p0); }
	static Any stat_save_pending() { return _i<Any>(0xACF180); }
	static Any stat_save_pending_or_requested() { return _i<Any>(0xACF1A0); }
	static bool stat_set_bool(uint32_t statName, bool value, bool save) { return _i<bool>(0xACF520, statName, value, save); }
	static bool stat_set_current_posix_time(uint32_t statName, bool p1) { return _i<bool>(0xACF6E0, statName, p1); }
	static bool stat_set_date(uint32_t statName, Any* value, int numFields, bool save) { return _i<bool>(0xACF5A0, statName, value, numFields, save); }
	static bool stat_set_float(uint32_t statName, float value, bool save) { return _i<bool>(0xACF4E0, statName, value, save); }
	static bool stat_set_gxt_label(uint32_t statName, const char* value, bool save) { return _i<bool>(0xACF560, statName, value, save); }
	static bool stat_set_int(uint32_t statName, int value, bool save) { return _i<bool>(0xACF4B0, statName, value, save); }
	static bool stat_set_license_plate(uint32_t statName, const char* str) { return _i<bool>(0xACFA20, statName, str); }
	static bool stat_set_masked_int(uint32_t statName, Any p1, Any p2, int p3, bool save) { return _i<bool>(0xACF660, statName, p1, p2, p3, save); }
	static bool stat_set_pos(uint32_t statName, float x, float y, float z, bool save) { return _i<bool>(0xACF620, statName, x, y, z, save); }
	static bool stat_set_string(uint32_t statName, const char* value, bool save) { return _i<bool>(0xACF5E0, statName, value, save); }
	static bool stat_slot_is_loaded(Any p0) { return _i<bool>(0xACF250, p0); }
	static Void stop_alarm(const char* alarmName, bool toggle) { return _i<Void>(0x986CF0, alarmName, toggle); }
	static Void stop_all_alarms(bool stop) { return _i<Void>(0x986D10, stop); }
	static Void stop_all_garage_activity() { return _i<Void>(0xB22400); }
	static Void stop_anim_playback(Ped ped, Any p1, bool p2) { return _i<Void>(0xAF3200, ped, p1, p2); }
	static Void stop_anim_task(Ped ped, const char* animDictionary, const char* animationName, float p3) { return _i<Void>(0xAF3180, ped, animDictionary, animationName, p3); }
	static Void stop_any_ped_model_being_suppressed() { return _i<Void>(0xA9AB50); }
	static Void stop_audio_scene(const char* scene) { return _i<Void>(0x9867F0, scene); }
	static Void stop_audio_scenes() { return _i<Void>(0x986820); }
	static Void stop_cam_pointing(Cam cam) { return _i<Void>(0x992A90, cam); }
	static Void stop_cam_shaking(Cam cam, bool p1) { return _i<Void>(0x993850, cam, p1); }
	static Void stop_cinematic_cam_shaking(bool p0) { return _i<Void>(0x995F70, p0); }
	static Void stop_cinematic_shot(Any p0) { return _i<Void>(0x996110, p0); }
	static Void stop_current_playing_ambient_speech(Ped ped) { return _i<Void>(0x983DE0, ped); }
	static Void stop_cutscene(bool p0) { return _i<Void>(0x9993F0, p0); }
	static Void stop_cutscene_cam_shaking() { return _i<Void>(0x996330); }
	static Void stop_cutscene_immediately() { return _i<Void>(0x999430); }
	static Any stop_entity_anim(Entity entity, const char* animation, const char* animGroup, float p3) { return _i<Any>(0x9B54F0, entity, animation, animGroup, p3); }
	static Void stop_entity_fire(Entity entity) { return _i<Void>(0x9BB180, entity); }
	static Void stop_fire_in_range(float x, float y, float z, float radius) { return _i<Void>(0x9BB2E0, x, y, z, radius); }
	static Void stop_gameplay_cam_shaking(bool p0) { return _i<Void>(0x994620, p0); }
	static Void stop_gameplay_hint(bool p0) { return _i<Void>(0x995C00, p0); }
	static Void stop_particle_fx_looped(int ptfxHandle, bool p1) { return _i<Void>(0x9CF440, ptfxHandle, p1); }
	static Void stop_ped_ringtone(Ped ped) { return _i<Void>(0x9828E0, ped); }
	static Void stop_ped_speaking(Ped ped, bool shaking) { return _i<Void>(0x985940, ped, shaking); }
	static Void stop_ped_weapon_firing_when_dropped(Ped ped) { return _i<Void>(0xA9B540, ped); }
	static Void stop_playback_recorded_vehicle(Any p0) { return _i<Void>(0xB1FBD0, p0); }
	static Void stop_player_switch() { return _i<Void>(0xADA590); }
	static Void stop_player_teleport() { return _i<Void>(0xAAC5F0); }
	static Void stop_rope_unwinding_front(Object rope) { return _i<Void>(0xAA0F40, rope); }
	static Void stop_rope_winding(Object rope) { return _i<Void>(0xAA0ED0, rope); }
	static Void stop_save_array() { return _i<Void>(0x9F9B60); }
	static Void stop_save_data() { return _i<Void>(0x9F9890); }
	static Void stop_save_struct() { return _i<Void>(0x9F9B00); }
	static Any stop_scripted_conversation(bool p0) { return _i<Any>(0x983010, p0); }
	static Void stop_script_global_shaking(bool p0) { return _i<Void>(0x993990, p0); }
	static Void stop_sound(int soundId) { return _i<Void>(0x9836D0, soundId); }
	static Void stop_stream() { return _i<Void>(0x985930); }
	static bool stop_synchronized_audio_event(Any p0) { return _i<bool>(0x987420, p0); }
	static bool stop_synchronized_entity_anim(Entity entity, float p1, bool p2) { return _i<bool>(0x9B5510, entity, p1, p2); }
	static bool string_to_int(const char* string, int* outInteger) { return _i<bool>(0x9F9240, string, outInteger); }
	static Void suppress_agitation_events_next_frame() { return _i<Void>(0x9B9670); }
	static Void suppress_shocking_events_next_frame() { return _i<Void>(0x9B9640); }
	static Void suppress_shocking_event_type_next_frame(int type) { return _i<Void>(0x9B9650, type); }
	static Void switch_train_track(int intersectionId, bool state) { return _i<Void>(0xB1F420, intersectionId, state); }
	static float tan(float p0) { return _i<float>(0x9F67C0, p0); }
	static Void task_achieve_heading(Ped ped, float heading, int timeout) { return _i<Void>(0xAF2790, ped, heading, timeout); }
	static Void task_aim_gun_at_coord(Ped ped, float x, float y, float z, int time, bool p5, bool p6) { return _i<Void>(0xAF3BB0, ped, x, y, z, time, p5, p6); }
	static Void task_aim_gun_at_entity(Ped ped, Entity entity, int duration, bool p3) { return _i<Void>(0xAF3B70, ped, entity, duration, p3); }
	static Void task_aim_gun_scripted(Ped ped, uint32_t scriptTask, bool p2, bool p3) { return _i<Void>(0xAF3AB0, ped, scriptTask, p2, p3); }
	static Void task_aim_gun_scripted_with_target(Any p0, Any p1, float p2, float p3, float p4, Any p5, bool p6, bool p7) { return _i<Void>(0xAF3AE0, p0, p1, p2, p3, p4, p5, p6, p7); }
	static Void task_arrest_ped(Ped ped, Ped target) { return _i<Void>(0xAF8750, ped, target); }
	static Void task_boat_mission(Ped pedDriver, Vehicle boat, Any p2, Any p3, float x, float y, float z, Any p7, float maxSpeed, int drivingStyle, float p10, Any p11) { return _i<Void>(0xAF4470, pedDriver, boat, p2, p3, x, y, z, p7, maxSpeed, drivingStyle, p10, p11); }
	static Void task_chat_to_ped(Ped ped, Ped target, Any p2, float p3, float p4, float p5, float p6, float p7) { return _i<Void>(0xAF47B0, ped, target, p2, p3, p4, p5, p6, p7); }
	static Void task_clear_defensive_area(Any p0) { return _i<Void>(0xAF4B50, p0); }
	static Void task_clear_look_at(Ped ped) { return _i<Void>(0xAF3740, ped); }
	static Void task_climb(Ped ped, bool unused) { return _i<Void>(0xAF4810, ped, unused); }
	static Void task_climb_ladder(Ped ped, int p1) { return _i<Void>(0xAF4870, ped, p1); }
	static Void task_combat_hated_targets_around_ped(Ped ped, float radius, int p2) { return _i<Void>(0xAF5EA0, ped, radius, p2); }
	static Void task_combat_hated_targets_around_ped_timed(Any p0, float p1, Any p2, Any p3) { return _i<Void>(0xAF5F90, p0, p1, p2, p3); }
	static Void task_combat_hated_targets_in_area(Ped ped, float x, float y, float z, float radius, Any p5) { return _i<Void>(0xAF5DA0, ped, x, y, z, radius, p5); }
	static Void task_cower(Ped ped, int duration) { return _i<Void>(0xAF2060, ped, duration); }
	static Void task_enter_vehicle(Ped ped, Vehicle vehicle, int timeout, int seat, float speed, int p5, Any p6) { return _i<Void>(0xAF2230, ped, vehicle, timeout, seat, speed, p5, p6); }
	static Void task_everyone_leave_vehicle(Vehicle vehicle) { return _i<Void>(0xAF3D30, vehicle); }
	static Void task_exit_cover(Any p0, Any p1, float p2, float p3, float p4) { return _i<Void>(0xAF50D0, p0, p1, p2, p3, p4); }
	static Void task_extend_route(float x, float y, float z) { return _i<Void>(0xAF27C0, x, y, z); }
	static Void task_flush_route() { return _i<Void>(0xAF27B0); }
	static Void task_follow_nav_mesh_to_coord(Ped ped, float x, float y, float z, float speed, int timeout, float stoppingRange, bool persistFollowing, float unk) { return _i<Void>(0xAF2AB0, ped, x, y, z, speed, timeout, stoppingRange, persistFollowing, unk); }
	static Void task_follow_point_route(Ped ped, float speed, int unknown) { return _i<Void>(0xAF2800, ped, speed, unknown); }
	static Void task_follow_to_offset_of_entity(Ped ped, Entity entity, float offsetX, float offsetY, float offsetZ, float movementSpeed, int timeout, float stoppingRange, bool persistFollowing) { return _i<Void>(0xAF2710, ped, entity, offsetX, offsetY, offsetZ, movementSpeed, timeout, stoppingRange, persistFollowing); }
	static Void task_follow_waypoint_recording(Any p0, Any p1, Any p2, Any p3, Any p4) { return _i<Void>(0xAF6AA0, p0, p1, p2, p3, p4); }
	static Void task_get_off_boat(Ped ped, Vehicle boat) { return _i<Void>(0xAF2280, ped, boat); }
	static Void task_goto_entity_offset_xy(Any p0, Any p1, Any p2, float p3, float p4, float p5, float p6, Any p7) { return _i<Void>(0xAF3D70, p0, p1, p2, p3, p4, p5, p6, p7); }
	static Void task_go_straight_to_coord(Ped ped, float x, float y, float z, float speed, int timeout, float targetHeading, float distanceToSlide) { return _i<Void>(0xAF2740, ped, x, y, z, speed, timeout, targetHeading, distanceToSlide); }
	static Void task_go_straight_to_coord_relative_to_entity(Entity entity1, Entity entity2, float p2, float p3, float p4, float p5, Any p6) { return _i<Void>(0xAF2770, entity1, entity2, p2, p3, p4, p5, p6); }
	static Void task_go_to_coord_any_means(Ped ped, float x, float y, float z, float speed, Any p5, bool p6, int walkingStyle, float p8) { return _i<Void>(0xAF2F40, ped, x, y, z, speed, p5, p6, walkingStyle, p8); }
	static Void task_go_to_coord_any_means_extra_params(Ped ped, float x, float y, float z, float speed, Any p5, bool p6, int walkingStyle, float p8, Any p9, Any p10, Any p11, Any p12) { return _i<Void>(0xAF2F70, ped, x, y, z, speed, p5, p6, walkingStyle, p8, p9, p10, p11, p12); }
	static Void task_go_to_coord_any_means_extra_params_with_cruise_speed(Ped ped, float x, float y, float z, float speed, Any p5, bool p6, int walkingStyle, float p8, Any p9, Any p10, Any p11, Any p12, Any p13) { return _i<Void>(0xAF2FB0, ped, x, y, z, speed, p5, p6, walkingStyle, p8, p9, p10, p11, p12, p13); }
	static Void task_go_to_coord_while_aiming_at_coord(Ped ped, float x, float y, float z, float aimAtX, float aimAtY, float aimAtZ, float moveSpeed, bool p8, float p9, float p10, bool p11, Any flags, bool p13, uint32_t firingPattern) { return _i<Void>(0xAF65B0, ped, x, y, z, aimAtX, aimAtY, aimAtZ, moveSpeed, p8, p9, p10, p11, flags, p13, firingPattern); }
	static Void task_go_to_coord_while_aiming_at_entity(Any p0, float p1, float p2, float p3, Any p4, float p5, bool p6, float p7, float p8, bool p9, Any p10, bool p11, Any p12, Any p13) { return _i<Void>(0xAF6610, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13); }
	static Void task_go_to_entity(Entity entity, Entity target, int duration, float distance, float speed, float p5, int p6) { return _i<Void>(0xAF2820, entity, target, duration, distance, speed, p5, p6); }
	static Void task_go_to_entity_while_aiming_at_entity(Ped ped, Entity entityToWalkTo, Entity entityToAimAt, float speed, bool shootatEntity, float p5, float p6, bool p7, bool p8, uint32_t firingPattern) { return _i<Void>(0xAF6740, ped, entityToWalkTo, entityToAimAt, speed, shootatEntity, p5, p6, p7, p8, firingPattern); }
	static Void task_guard_assigned_defensive_area(Any p0, float p1, float p2, float p3, float p4, float p5, Any p6) { return _i<Void>(0xAF51C0, p0, p1, p2, p3, p4, p5, p6); }
	static Void task_guard_current_position(Ped p0, float p1, float p2, bool p3) { return _i<Void>(0xAF51A0, p0, p1, p2, p3); }
	static Void task_guard_sphere_defensive_area(Ped p0, float p1, float p2, float p3, float p4, float p5, Any p6, float p7, float p8, float p9, float p10) { return _i<Void>(0xAF51E0, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); }
	static Void task_hands_up(Ped ped, int duration, Ped facingPed, int p3, bool p4) { return _i<Void>(0xAF20E0, ped, duration, facingPed, p3, p4); }
	static Void task_heli_chase(Ped pilot, Entity entityToFollow, float x, float y, float z) { return _i<Void>(0xAF4160, pilot, entityToFollow, x, y, z); }
	static Void task_heli_mission(Ped pilot, Vehicle vehicle, Vehicle vehicleToFollow, Ped pedToFollow, float posX, float posY, float posZ, int mode, float speed, float radius, float angle, int p11, int height, float p13, int p14) { return _i<Void>(0xAF4370, pilot, vehicle, vehicleToFollow, pedToFollow, posX, posY, posZ, mode, speed, radius, angle, p11, height, p13, p14); }
	static Void task_jump(Ped ped, bool unused, Any p2, Any p3) { return _i<Void>(0xAF1FE0, ped, unused, p2, p3); }
	static Void task_leave_any_vehicle(Ped ped, int p1, int p2) { return _i<Void>(0xAF3A90, ped, p1, p2); }
	static Void task_leave_vehicle(Ped ped, Vehicle vehicle, int flags) { return _i<Void>(0xAF2260, ped, vehicle, flags); }
	static Void task_look_at_coord(Entity entity, float x, float y, float z, float duration, Any p5, Any p6) { return _i<Void>(0xAF3700, entity, x, y, z, duration, p5, p6); }
	static Void task_look_at_entity(Ped ped, Entity lookAt, int duration, int unknown1, int unknown2) { return _i<Void>(0xAF3720, ped, lookAt, duration, unknown1, unknown2); }
	static Void task_open_vehicle_door(Ped ped, Vehicle vehicle, int timeOut, int doorIndex, float speed) { return _i<Void>(0xAF2210, ped, vehicle, timeOut, doorIndex, speed); }
	static Void task_parachute(Ped ped, bool p1, Any p2) { return _i<Void>(0xAF2320, ped, p1, p2); }
	static Void task_parachute_to_target(Ped ped, float x, float y, float z) { return _i<Void>(0xAF23A0, ped, x, y, z); }
	static Void task_patrol(Ped ped, const char* p1, Any p2, bool p3, bool p4) { return _i<Void>(0xAF6410, ped, p1, p2, p3, p4); }
	static Void task_pause(Ped ped, int ms) { return _i<Void>(0xAF1EE0, ped, ms); }
	static Void task_ped_slide_to_coord(Ped ped, float x, float y, float z, float heading, float p5) { return _i<Void>(0xAF4B60, ped, x, y, z, heading, p5); }
	static Void task_ped_slide_to_coord_hdg_rate(Ped ped, float x, float y, float z, float heading, float p5, float p6) { return _i<Void>(0xAF4B80, ped, x, y, z, heading, p5, p6); }
	static Void task_perform_sequence(Ped ped, Object taskSequence) { return _i<Void>(0xAF3810, ped, taskSequence); }
	static Void task_plane_chase(Ped pilot, Entity entityToFollow, float x, float y, float z) { return _i<Void>(0xAF4180, pilot, entityToFollow, x, y, z); }
	static Void task_plane_mission(Ped pilot, Vehicle plane, Vehicle targetVehicle, Ped targetPed, float destinationX, float destinationY, float destinationZ, int missionType, float vehicleSpeed, float p9, float heading, float maxAltitude, float minAltitude, Any p13) { return _i<Void>(0xAF43F0, pilot, plane, targetVehicle, targetPed, destinationX, destinationY, destinationZ, missionType, vehicleSpeed, p9, heading, maxAltitude, minAltitude, p13); }
	static Void task_plant_bomb(Ped ped, float x, float y, float z, float heading) { return _i<Void>(0xAF2A70, ped, x, y, z, heading); }
	static Void task_play_anim(Ped ped, const char* animDictionary, const char* animationName, float speed, float speedMultiplier, int duration, int flag, float playbackRate, bool lockX, bool lockY, bool lockZ) { return _i<Void>(0xAF3000, ped, animDictionary, animationName, speed, speedMultiplier, duration, flag, playbackRate, lockX, lockY, lockZ); }
	static Void task_play_phone_gesture_animation(Ped ped, const char* animDict, const char* animation, const char* boneMaskType, float p4, float p5, bool p6, bool p7) { return _i<Void>(0xAF3450, ped, animDict, animation, boneMaskType, p4, p5, p6, p7); }
	static Void task_put_ped_directly_into_cover(Ped ped, float x, float y, float z, Any timeout, bool p5, float p6, bool p7, bool p8, Any p9, bool p10) { return _i<Void>(0xAF4F80, ped, x, y, z, timeout, p5, p6, p7, p8, p9, p10); }
	static Void task_put_ped_directly_into_melee(Ped ped, Ped meleeTarget, float p2, float p3, float p4, bool p5) { return _i<Void>(0xAF50F0, ped, meleeTarget, p2, p3, p4, p5); }
	static Void task_rappel_from_heli(Ped ped, int unused) { return _i<Void>(0xAF2520, ped, unused); }
	static Void task_react_and_flee_ped(Ped ped, Ped fleeTarget) { return _i<Void>(0xAF28B0, ped, fleeTarget); }
	static Void task_reload_weapon(Ped ped, bool unused) { return _i<Void>(0xAF6130, ped, unused); }
	static Void task_scripted_animation(Ped ped, Any* p1, Any* p2, Any* p3, float p4, float p5) { return _i<Void>(0xAF31A0, ped, p1, p2, p3, p4, p5); }
	static Void task_seek_cover_from_ped(Ped ped, Ped target, int duration, bool p3) { return _i<Void>(0xAF4F10, ped, target, duration, p3); }
	static Void task_seek_cover_from_pos(Ped ped, float x, float y, float z, int duration, bool p5) { return _i<Void>(0xAF4EF0, ped, x, y, z, duration, p5); }
	static Void task_seek_cover_to_coords(Ped ped, float x1, float y1, float z1, float x2, float y2, float z2, Any p7, bool p8) { return _i<Void>(0xAF4F50, ped, x1, y1, z1, x2, y2, z2, p7, p8); }
	static Void task_seek_cover_to_cover_point(Any p0, Any p1, float p2, float p3, float p4, Any p5, bool p6) { return _i<Void>(0xAF4F30, p0, p1, p2, p3, p4, p5, p6); }
	static Void task_set_blocking_of_non_temporary_events(Ped ped, bool toggle) { return _i<Void>(0xAF7920, ped, toggle); }
	static Void task_set_decision_maker(Ped p0, uint32_t p1) { return _i<Void>(0xAF4B20, p0, p1); }
	static Void task_set_sphere_defensive_area(Any p0, float p1, float p2, float p3, float p4) { return _i<Void>(0xAF4B30, p0, p1, p2, p3, p4); }
	static Void task_shocking_event_react(Ped ped, int eventHandle) { return _i<Void>(0xAF28C0, ped, eventHandle); }
	static Void task_shoot_at_coord(Ped ped, float x, float y, float z, int duration, uint32_t firingPattern) { return _i<Void>(0xAF3BE0, ped, x, y, z, duration, firingPattern); }
	static Void task_shuffle_to_next_vehicle_seat(Ped ped, Vehicle vehicle, Any p2) { return _i<Void>(0xAF3C00, ped, vehicle, p2); }
	static Void task_sky_dive(Ped ped, Any p1) { return _i<Void>(0xAF2300, ped, p1); }
	static Void task_smart_flee_coord(Ped ped, float x, float y, float z, float distance, int time, bool p6, bool p7) { return _i<Void>(0xAF2850, ped, x, y, z, distance, time, p6, p7); }
	static Void task_smart_flee_ped(Ped ped, Ped fleeTarget, float distance, Any fleeTime, bool p4, bool p5) { return _i<Void>(0xAF2880, ped, fleeTarget, distance, fleeTime, p4, p5); }
	static Void task_stand_guard(Ped ped, float x, float y, float z, float heading, const char* scenarioName) { return _i<Void>(0xAF5210, ped, x, y, z, heading, scenarioName); }
	static Void task_stand_still(Ped ped, int time) { return _i<Void>(0xAF1F50, ped, time); }
	static Void task_stay_in_cover(Ped ped) { return _i<Void>(0xAF64B0, ped); }
	static Void task_stealth_kill(Ped killer, Ped target, uint32_t killType, float p3, bool p4) { return _i<Void>(0xAF2A50, killer, target, killType, p3, p4); }
	// [aligned] static Void task_stop_phone_gesture_animation(Ped ped, Any p1) { return _i<Void>(0xAF3490, ped, p1); }
	static Void task_swap_weapon(Ped ped, bool p1) { return _i<Void>(0xAF60B0, ped, p1); }
	static Void task_sweep_aim_entity(Ped ped, const char* anim, const char* p2, const char* p3, const char* p4, int p5, Vehicle vehicle, float p7, float p8) { return _i<Void>(0xAF8630, ped, anim, p2, p3, p4, p5, vehicle, p7, p8); }
	static Void task_synchronized_scene(Ped ped, int scene, const char* animDictionary, const char* animationName, float speed, float speedMultiplier, int duration, int flag, float playbackRate, Any p9) { return _i<Void>(0xAF85D0, ped, scene, animDictionary, animationName, speed, speedMultiplier, duration, flag, playbackRate, p9); }
	static Void task_throw_projectile(int ped, float x, float y, float z, Any p4, Any p5) { return _i<Void>(0xAF6090, ped, x, y, z, p4, p5); }
	static Void task_toggle_duck(bool p0, bool p1) { return _i<Void>(0xAF5110, p0, p1); }
	static Void task_turn_ped_to_face_coord(Ped ped, float x, float y, float z, int duration) { return _i<Void>(0xAF3DA0, ped, x, y, z, duration); }
	static Void task_turn_ped_to_face_entity(Ped ped, Entity entity, int duration) { return _i<Void>(0xAF3B90, ped, entity, duration); }
	static Void task_use_mobile_phone(Ped ped, int p1, Any p2) { return _i<Void>(0xAF4780, ped, p1, p2); }
	static Void task_use_mobile_phone_timed(Ped ped, int duration) { return _i<Void>(0xAF47A0, ped, duration); }
	static Void task_use_nearest_scenario_chain_to_coord(Any p0, float p1, float p2, float p3, float p4, Any p5) { return _i<Void>(0xAF5690, p0, p1, p2, p3, p4, p5); }
	static Void task_use_nearest_scenario_chain_to_coord_warp(Any p0, float p1, float p2, float p3, float p4, Any p5) { return _i<Void>(0xAF56B0, p0, p1, p2, p3, p4, p5); }
	static Void task_use_nearest_scenario_to_coord_warp(Ped ped, float x, float y, float z, float radius, Any p5) { return _i<Void>(0xAF55E0, ped, x, y, z, radius, p5); }
	static Void task_vehicle_aim_at_coord(Ped ped, float x, float y, float z) { return _i<Void>(0xAF6580, ped, x, y, z); }
	static Void task_vehicle_aim_at_ped(Ped ped, Ped target) { return _i<Void>(0xAF6550, ped, target); }
	static Void task_vehicle_chase(Ped driver, Entity targetEnt) { return _i<Void>(0xAF4020, driver, targetEnt); }
	static Void task_vehicle_drive_to_coord(Ped ped, Vehicle vehicle, float x, float y, float z, float speed, Any p6, uint32_t vehicleModel, int drivingMode, float stopRange, float p10) { return _i<Void>(0xAF25A0, ped, vehicle, x, y, z, speed, p6, vehicleModel, drivingMode, stopRange, p10); }
	static Void task_vehicle_drive_to_coord_longrange(Ped ped, Vehicle vehicle, float x, float y, float z, float speed, int driveMode, float stopRange) { return _i<Void>(0xAF2620, ped, vehicle, x, y, z, speed, driveMode, stopRange); }
	static Void task_vehicle_drive_wander(Ped ped, Vehicle vehicle, float speed, int drivingStyle) { return _i<Void>(0xAF2640, ped, vehicle, speed, drivingStyle); }
	static Void task_vehicle_escort(Ped ped, Vehicle vehicle, Vehicle targetVehicle, int mode, float speed, int drivingStyle, float minDistance, int p7, float noRoadsDistance) { return _i<Void>(0xAF3FD0, ped, vehicle, targetVehicle, mode, speed, drivingStyle, minDistance, p7, noRoadsDistance); }
	// [aligned] static Void task_vehicle_follow(Ped driver, Vehicle vehicle, Entity targetEntity, int drivingStyle, float speed, int minDistance) { return _i<Void>(0xAF4000, driver, vehicle, targetEntity, drivingStyle, speed, minDistance); }
	static Void task_vehicle_follow_waypoint_recording(Ped ped, Vehicle vehicle, const char* WPRecording, int p3, int p4, int p5, int p6, float p7, bool p8, float p9) { return _i<Void>(0xAF74E0, ped, vehicle, WPRecording, p3, p4, p5, p6, p7, p8, p9); }
	static Void task_vehicle_heli_protect(Ped pilot, Vehicle vehicle, Entity entityToFollow, float targetSpeed, int p4, float radius, int altitude, int p7) { return _i<Void>(0xAF4030, pilot, vehicle, entityToFollow, targetSpeed, p4, radius, altitude, p7); }
	static Void task_vehicle_mission(Any p0, Any p1, Any p2, Any p3, float p4, Any p5, float p6, float p7, bool p8) { return _i<Void>(0xAF3EB0, p0, p1, p2, p3, p4, p5, p6, p7, p8); }
	static Void task_vehicle_mission_coors_target(Ped ped, Vehicle vehicle, float x, float y, float z, int p5, int p6, int p7, float p8, float p9, bool p10) { return _i<Void>(0xAF3F40, ped, vehicle, x, y, z, p5, p6, p7, p8, p9, p10); }
	static Void task_vehicle_mission_ped_target(Ped ped, Vehicle vehicle, Ped pedTarget, int mode, float maxSpeed, int drivingStyle, float minDistance, float p7, bool p8) { return _i<Void>(0xAF3F00, ped, vehicle, pedTarget, mode, maxSpeed, drivingStyle, minDistance, p7, p8); }
	static Void task_vehicle_park(Ped ped, Vehicle vehicle, float x, float y, float z, float heading, int mode, float radius, bool keepEngineOn) { return _i<Void>(0xAF2A20, ped, vehicle, x, y, z, heading, mode, radius, keepEngineOn); }
	static Void task_vehicle_play_anim(Vehicle vehicle, const char* animation_set, const char* animation_name) { return _i<Void>(0xAF36E0, vehicle, animation_set, animation_name); }
	static Void task_vehicle_shoot_at_coord(Ped ped, float x, float y, float z, float p4) { return _i<Void>(0xAF6560, ped, x, y, z, p4); }
	static Void task_vehicle_shoot_at_ped(Ped ped, Ped target, float p2) { return _i<Void>(0xAF6530, ped, target, p2); }
	static Void task_vehicle_temp_action(Ped driver, Vehicle vehicle, int action, int time) { return _i<Void>(0xAF3E90, driver, vehicle, action, time); }
	static Void task_wander_in_area(Ped ped, float x, float y, float z, float radius, float minimalLength, float timeBetweenWalks) { return _i<Void>(0xAF28D0, ped, x, y, z, radius, minimalLength, timeBetweenWalks); }
	static Void task_wander_standard(Ped ped, float p1, int p2) { return _i<Void>(0xAF29E0, ped, p1, p2); }
	static Void task_warp_ped_into_vehicle(Ped ped, Vehicle vehicle, int seat) { return _i<Void>(0xAF47D0, ped, vehicle, seat); }
	static Void task_writhe(Ped ped, Ped target, int time, int p3, Any p4, Any p5) { return _i<Void>(0xAF61B0, ped, target, time, p3, p4, p5); }
	static Void terminate_all_scripts_with_this_name(const char* scriptName) { return _i<Void>(0x9F7810, scriptName); }
	static Void terminate_this_thread() { return _i<Void>(0xAB13A0); }
	static bool test_probe_against_all_water(Any p0, Any p1, Any p2, Any p3, Any p4, Any p5, Any p6, Any p7) { return _i<bool>(0xB2FB60, p0, p1, p2, p3, p4, p5, p6, p7); }
	static bool test_probe_against_water(float x1, float y1, float z1, float x2, float y2, float z2, math::vector3<float>* result) { return _i<bool>(0xB2FAF0, x1, y1, z1, x2, y2, z2, result); }
	static const char* texture_download_get_name(int p0) { return _i<const char*>(0xA591F0, p0); }
	static bool texture_download_has_failed(int p0) { return _i<bool>(0xA591C0, p0); }
	static Void texture_download_release(int p0) { return _i<Void>(0xA591A0, p0); }
	static int texture_download_request(int* PlayerHandle, const char* FilePath, const char* Name, bool p3) { return _i<int>(0xA58EB0, PlayerHandle, FilePath, Name, p3); }
	static Void toggle_stealth_radar(bool toggle) { return _i<Void>(0x9E3BE0, toggle); }
	static Void toggle_vehicle_mod(Vehicle vehicle, int modType, bool toggle) { return _i<Void>(0xB27380, vehicle, modType, toggle); }
	static Void track_object_visibility(Any p0) { return _i<Void>(0xA65570, p0); }
	static Void track_vehicle_visibility(Vehicle vehicle) { return _i<Void>(0xB28E80, vehicle); }
	static bool trigger_music_event(const char* eventName) { return _i<bool>(0x986AD0, eventName); }
	static Void trigger_script_event(bool p0, int64_t* args, int argCount, int bit) { return _i<Void>(0xAB1540, p0, args, argCount, bit); }
	// [aligned] static bool ui3dscene_is_available() { return _i<bool>(0x9D1EB0); }
	static Void unblock_decision_maker_event(uint32_t name, int type) { return _i<Void>(0x9B9430, name, type); }
	static Void uncuff_ped(Ped ped) { return _i<Void>(0xAF8800, ped); }
	static Void unfreeze_radio_station(const char* radioStation) { return _i<Void>(0x984940, radioStation); }
	static Void unlock_minimap_angle() { return _i<Void>(0x9E3F00); }
	static Void unlock_minimap_position() { return _i<Void>(0x9E3F30); }
	static Void unlock_mission_news_story(int newsStory) { return _i<Void>(0x984860, newsStory); }
	static Void unpause_playback_recorded_vehicle(Any p0) { return _i<Void>(0xB1FC70, p0); }
	static Void unpin_interior(int interiorID) { return _i<Void>(0x9E8960, interiorID); }
	static Void unpin_rope_vertex(Object rope, int vertex) { return _i<Void>(0xAA09D0, rope, vertex); }
	static Void unregister_pedheadshot(int handle) { return _i<Void>(0xA9D560, handle); }
	static Void unregister_script_with_audio() { return _i<Void>(0x9831E0); }
	static Void update_navmesh_blocking_object(Any p0, float p1, float p2, float p3, float p4, float p5, float p6, float p7, Any p8) { return _i<Void>(0xA70DD0, p0, p1, p2, p3, p4, p5, p6, p7, p8); }
	static int update_onscreen_keyboard() { return _i<int>(0x9FA970); }
	static Void update_task_aim_gun_scripted_target(Ped p0, Ped p1, float p2, float p3, float p4, bool p5) { return _i<Void>(0xAF3B10, p0, p1, p2, p3, p4, p5); }
	static Void update_task_hands_up_duration(Ped ped, int duration) { return _i<Void>(0xAF21B0, ped, duration); }
	static Void update_task_sweep_aim_entity(Ped ped, Entity entity) { return _i<Void>(0xAF8670, ped, entity); }
	static Void update_task_sweep_aim_position(Any p0, float p1, float p2, float p3) { return _i<Void>(0xAF86C0, p0, p1, p2, p3); }
	static Void use_player_colour_instead_of_team_colour(bool toggle, Any p1) { return _i<Void>(0xA54970, toggle, p1); }
	static Void use_siren_as_horn(Vehicle vehicle, bool toggle) { return _i<Void>(0x985EC0, vehicle, toggle); }
	static Void use_waypoint_recording_as_assisted_movement_route(Any* p0, bool p1, float p2, float p3) { return _i<Void>(0xAF7180, p0, p1, p2, p3); }
	static Void using_mission_creator(bool toggle) { return _i<Void>(0x9F85B0, toggle); }
	static Void vehicle_waypoint_playback_override_speed(Any p0, float p1) { return _i<Void>(0xAF78A0, p0, p1); }
	static Void vehicle_waypoint_playback_pause(Any p0) { return _i<Void>(0xAF76C0, p0); }
	static Void vehicle_waypoint_playback_resume(Any p0) { return _i<Void>(0xAF77C0, p0); }
	static Void vehicle_waypoint_playback_use_default_speed(Any p0) { return _i<Void>(0xAF7830, p0); }
	static int veh_to_net(Vehicle vehicle) { return _i<int>(0xA4F950, vehicle); }
	static Void wash_decals_from_vehicle(Vehicle vehicle, float p1) { return _i<Void>(0x9CFCD0, vehicle, p1); }
	static Void wash_decals_in_range(Any p0, Any p1, Any p2, Any p3, Any p4) { return _i<Void>(0x9CFC90, p0, p1, p2, p3, p4); }
	static bool was_cutscene_skipped() { return _i<bool>(0x9995A0); }
	static bool was_ped_killed_by_stealth(Ped ped) { return _i<bool>(0xA9BAE0, ped); }
	static bool was_ped_killed_by_takedown(Ped ped) { return _i<bool>(0xA9BB30, ped); }
	static bool was_ped_skeleton_updated(Ped ped) { return _i<bool>(0xA99860, ped); }
	static bool waypoint_playback_get_is_paused(Any p0) { return _i<bool>(0xAF6E80, p0); }
	static Void waypoint_playback_override_speed(Any p0, float p1, bool p2) { return _i<Void>(0xAF7050, p0, p1, p2); }
	static Void waypoint_playback_pause(Any p0, bool p1, bool p2) { return _i<Void>(0xAF6F10, p0, p1, p2); }
	static Void waypoint_playback_resume(Any p0, bool p1, Any p2, Any p3) { return _i<Void>(0xAF6FB0, p0, p1, p2, p3); }
	static Void waypoint_playback_start_aiming_at_coord(Any p0, float p1, float p2, float p3, bool p4) { return _i<Void>(0xAF7260, p0, p1, p2, p3, p4); }
	static Void waypoint_playback_start_aiming_at_ped(Any p0, Any p1, bool p2) { return _i<Void>(0xAF71A0, p0, p1, p2); }
	static Void waypoint_playback_start_shooting_at_coord(Any p0, float p1, float p2, float p3, bool p4, Any p5) { return _i<Void>(0xAF7350, p0, p1, p2, p3, p4, p5); }
	// [aligned] static Void waypoint_playback_start_shooting_at_ped(Any p0, Any p1, bool p2, Any p3) { return _i<Void>(0xAF7350, p0, p1, p2, p3); }
	static Void waypoint_playback_stop_aiming_or_shooting(Any p0) { return _i<Void>(0xAF7370, p0); }
	static Void waypoint_playback_use_default_speed(Any p0) { return _i<Void>(0xAF7110, p0); }
	static bool waypoint_recording_get_closest_waypoint(const char* name, float x, float y, float z, int* point) { return _i<bool>(0xAF69F0, name, x, y, z, point); }
	static bool waypoint_recording_get_coord(const char* name, int point, math::vector3<float>* coord) { return _i<bool>(0xAF68E0, name, point, coord); }
	static bool waypoint_recording_get_num_points(const char* name, int* points) { return _i<bool>(0xAF6880, name, points); }
	static float waypoint_recording_get_speed_at_point(const char* name, int point) { return _i<float>(0xAF6950, name, point); }
	static bool would_entity_be_occluded(uint32_t entityModelHash, float x, float y, float z, bool p4) { return _i<bool>(0x9B4A90, entityModelHash, x, y, z, p4); }
}
