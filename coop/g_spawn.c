
#include "g_local.h"

#define LEG_WAIT_TIME 1 /* FS: Coop: Rogue specific */
#define MAX_LEGSFRAME 23 /* FS: Coop: Rogue specific */

#define SPAWNGROW_LIFESPAN 0.3 /* FS: Coop: Rogue specific */
#define STEPSIZE 18 /* FS: Coop: Rogue specific */

void Z_SpawnDMItems(); /* FS: Zaero specific game dll changes */

typedef struct
{
	char *name;
	void (*spawn)(edict_t *ent);
} spawn_t;

void SP_item_health(edict_t *self);
void SP_item_health_small(edict_t *self);
void SP_item_health_large(edict_t *self);
void SP_item_health_mega(edict_t *self);

void SP_info_player_start(edict_t *ent);
void SP_info_player_deathmatch(edict_t *ent);
void SP_info_player_coop(edict_t *ent);
void SP_info_player_intermission(edict_t *ent);

void SP_func_plat(edict_t *ent);
void SP_func_rotating(edict_t *ent);
void SP_func_button(edict_t *ent);
void SP_func_door(edict_t *ent);
void SP_func_door_secret(edict_t *ent);
void SP_func_door_rotating(edict_t *ent);
void SP_func_water(edict_t *ent);
void SP_func_train(edict_t *ent);
void SP_func_conveyor(edict_t *self);
void SP_func_wall(edict_t *self);
void SP_func_object(edict_t *self);
void SP_func_explosive(edict_t *self);
void SP_func_timer(edict_t *self);
void SP_func_areaportal(edict_t *ent);
void SP_func_clock(edict_t *ent);
void SP_func_killbox(edict_t *ent);

void SP_trigger_always(edict_t *ent);
void SP_trigger_once(edict_t *ent);
void SP_trigger_multiple(edict_t *ent);
void SP_trigger_relay(edict_t *ent);
void SP_trigger_push(edict_t *ent);
void SP_trigger_hurt(edict_t *ent);
void SP_trigger_key(edict_t *ent);
void SP_trigger_counter(edict_t *ent);
void SP_trigger_elevator(edict_t *ent);
void SP_trigger_gravity(edict_t *ent);
void SP_trigger_monsterjump(edict_t *ent);

void SP_target_temp_entity(edict_t *ent);
void SP_target_speaker(edict_t *ent);
void SP_target_explosion(edict_t *ent);
void SP_target_changelevel(edict_t *ent);
void SP_target_secret(edict_t *ent);
void SP_target_goal(edict_t *ent);
void SP_target_splash(edict_t *ent);
void SP_target_spawner(edict_t *ent);
void SP_target_blaster(edict_t *ent);
void SP_target_crosslevel_trigger(edict_t *ent);
void SP_target_crosslevel_target(edict_t *ent);
void SP_target_laser(edict_t *self);
void SP_target_help(edict_t *ent);
void SP_target_actor(edict_t *ent);
void SP_target_lightramp(edict_t *self);
void SP_target_earthquake(edict_t *ent);
void SP_target_character(edict_t *ent);
void SP_target_string(edict_t *ent);

void SP_worldspawn(edict_t *ent);
void SP_viewthing(edict_t *ent);

void SP_light(edict_t *self);
void SP_light_mine1(edict_t *ent);
void SP_light_mine2(edict_t *ent);
void SP_info_null(edict_t *self);
void SP_info_notnull(edict_t *self);
void SP_path_corner(edict_t *self);
void SP_point_combat(edict_t *self);

void SP_misc_explobox(edict_t *self);
void SP_misc_banner(edict_t *self);
void SP_misc_satellite_dish(edict_t *self);
void SP_misc_actor(edict_t *self);
void SP_misc_gib_arm(edict_t *self);
void SP_misc_gib_leg(edict_t *self);
void SP_misc_gib_head(edict_t *self);
void SP_misc_insane(edict_t *self);
void SP_misc_deadsoldier(edict_t *self);
void SP_misc_viper(edict_t *self);
void SP_misc_viper_bomb(edict_t *self);
void SP_misc_bigviper(edict_t *self);
void SP_misc_strogg_ship(edict_t *self);
void SP_misc_teleporter(edict_t *self);
void SP_misc_teleporter_dest(edict_t *self);
void SP_misc_blackhole(edict_t *self);
void SP_misc_eastertank(edict_t *self);
void SP_misc_easterchick(edict_t *self);
void SP_misc_easterchick2(edict_t *self);

void SP_monster_berserk(edict_t *self);
void SP_monster_gladiator(edict_t *self);
void SP_monster_gunner(edict_t *self);
void SP_monster_infantry(edict_t *self);
void SP_monster_soldier_light(edict_t *self);
void SP_monster_soldier(edict_t *self);
void SP_monster_soldier_ss(edict_t *self);
void SP_monster_tank(edict_t *self);
void SP_monster_medic(edict_t *self);
void SP_monster_flipper(edict_t *self);
void SP_monster_chick(edict_t *self);
void SP_monster_parasite(edict_t *self);
void SP_monster_flyer(edict_t *self);
void SP_monster_brain(edict_t *self);
void SP_monster_floater(edict_t *self);
void SP_monster_hover(edict_t *self);
void SP_monster_mutant(edict_t *self);
void SP_monster_supertank(edict_t *self);
void SP_monster_boss2(edict_t *self);
void SP_monster_jorg(edict_t *self);
void SP_monster_boss3_stand(edict_t *self);

void SP_monster_commander_body(edict_t *self);

void SP_turret_breach(edict_t *self);
void SP_turret_base(edict_t *self);
void SP_turret_driver(edict_t *self);

void SP_func_plat2(edict_t *ent); /* FS: Coop: Rogue specific */
void SP_func_door_secret2(edict_t *ent); /* FS: Coop: Rogue specific */
void SP_func_force_wall(edict_t *ent); /* FS: Coop: Rogue specific */
void SP_info_player_coop_lava(edict_t *self); /* FS: Coop: Rogue specific */
void SP_info_teleport_destination(edict_t *self); /* FS: Coop: Rogue specific */
void SP_trigger_teleport(edict_t *self); /* FS: Coop: Rogue specific */
void SP_trigger_disguise(edict_t *self); /* FS: Coop: Rogue specific */
void SP_monster_stalker(edict_t *self); /* FS: Coop: Rogue specific */
void SP_monster_turret(edict_t *self); /* FS: Coop: Rogue specific */
void SP_target_steam(edict_t *self); /* FS: Coop: Rogue specific */
void SP_target_anger(edict_t *self); /* FS: Coop: Rogue specific */
void SP_target_killplayers(edict_t *self); /* FS: Coop: Rogue specific */

void SP_target_blacklight(edict_t *self); /* FS: Coop: Rogue specific */
void SP_target_orb(edict_t *self); /* FS: Coop: Rogue specific */

void SP_hint_path(edict_t *self); /* FS: Coop: Rogue specific */
void SP_monster_carrier(edict_t *self); /* FS: Coop: Rogue specific */
void SP_monster_widow(edict_t *self); /* FS: Coop: Rogue specific */
void SP_monster_widow2(edict_t *self); /* FS: Coop: Rogue specific */
void SP_dm_tag_token(edict_t *self); /* FS: Coop: Rogue specific */
void SP_dm_dball_goal(edict_t *self); /* FS: Coop: Rogue specific */
void SP_dm_dball_ball(edict_t *self); /* FS: Coop: Rogue specific */
void SP_dm_dball_team1_start(edict_t *self); /* FS: Coop: Rogue specific */
void SP_dm_dball_team2_start(edict_t *self); /* FS: Coop: Rogue specific */
void SP_dm_dball_ball_start(edict_t *self); /* FS: Coop: Rogue specific */
void SP_dm_dball_speed_change(edict_t *self); /* FS: Coop: Rogue specific */
void SP_monster_kamikaze(edict_t *self); /* FS: Coop: Rogue specific */
void SP_turret_invisible_brain(edict_t *self); /* FS: Coop: Rogue specific */
void SP_xatrix_item(edict_t *self); /* FS: Coop: Rogue specific */
void SP_misc_nuke_core(edict_t *self); /* FS: Coop: Rogue specific */

void ThrowMoreStuff(edict_t *self, vec3_t point); /* FS: Coop: Rogue specific */
void ThrowSmallStuff(edict_t *self, vec3_t point); /* FS: Coop: Rogue specific */
void ThrowWidowGibLoc(edict_t *self, char *gibname, int damage, /* FS: Coop: Rogue specific */
		int type, vec3_t startpos, qboolean fade); /* FS: Coop: Rogue specific */
void ThrowWidowGibSized(edict_t *self, char *gibname, int damage, int type, /* FS: Coop: Rogue specific */
		vec3_t startpos, int hitsound, qboolean fade);

void SP_monster_soldier_hypergun (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_monster_soldier_lasergun (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_monster_soldier_ripper (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_monster_fixbot (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_monster_gekk (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_monster_chick_heat (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_monster_gladb (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_monster_boss5 (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_rotating_light (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_object_repair (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_misc_crashviper (edict_t *ent); /* FS: Coop: Xatrix specific */
void SP_misc_viper_missile (edict_t *self); /* FS: Coop: Xatrix specific */
void SP_misc_amb4 (edict_t *ent); /* FS: Coop: Xatrix specific */
void SP_target_mal_laser (edict_t *ent); /* FS: Coop: Xatrix specific */
void SP_misc_transport (edict_t *ent); /* FS: Coop: Xatrix specific */

void SP_misc_nuke (edict_t *ent); /* FS: Coop: Xatrix specific */

void SP_sound_echo (edict_t *self); /* FS: Zaero specific game dll changes */

void SP_misc_lasertripbomb (edict_t *self); /* FS: Zaero specific game dll changes */
void SP_load_mirrorlevel (edict_t *self); /* FS: Zaero specific game dll changes */
void SP_trigger_laser(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_monster_autocannon(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_monster_autocannon_floor(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_monster_sentien(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_misc_securitycamera(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_monster_hound (edict_t *self); /* FS: Zaero specific game dll changes */
void SP_monster_handler (edict_t *self); /* FS: Zaero specific game dll changes */
void SP_misc_commdish (edict_t *self); /* FS: Zaero specific game dll changes */
void SP_misc_crate(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_misc_crate_medium(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_misc_crate_small(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_monster_zboss (edict_t *self); /* FS: Zaero specific game dll changes */
void SP_target_zboss_target(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_func_barrier(edict_t *self); /* FS: Zaero specific game dll changes */
void SP_misc_seat(edict_t *self); /* FS: Zaero specific game dll changes */

void SP_SetCDTrack(int track); /* FS: Coop: Added */
void SP_info_coop_checkpoint (edict_t * self ); /* FS: Coop: Added */

qboolean Spawn_CheckCoop_MapHacks (edict_t *ent); /* FS: Coop: Check if we have to modify some stuff for coop so we don't have to rely on distributing ent files. */
int G_SpawnCheckpoints (edict_t *ent); /* FS: Coop: Add checkpoints if we got them */
extern void SP_info_coop_checkpoint_touch ( edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );
void G_CheckCoopVictory (void); /* FS: Coop: Added */

spawn_t spawns[] = {
	{"item_health", SP_item_health},
	{"item_health_small", SP_item_health_small},
	{"item_health_large", SP_item_health_large},
	{"item_health_mega", SP_item_health_mega},

	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_coop", SP_info_player_coop},
	{"info_player_intermission", SP_info_player_intermission},

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	{"func_door", SP_func_door},
	{"func_door_secret", SP_func_door_secret},
	{"func_door_rotating", SP_func_door_rotating},
	{"func_rotating", SP_func_rotating},
	{"func_train", SP_func_train},
	{"func_water", SP_func_water},
	{"func_conveyor", SP_func_conveyor},
	{"func_areaportal", SP_func_areaportal},
	{"func_clock", SP_func_clock},
	{"func_wall", SP_func_wall},
	{"func_object", SP_func_object},
	{"func_timer", SP_func_timer},
	{"func_explosive", SP_func_explosive},
	{"func_killbox", SP_func_killbox},

	{"func_object_repair", SP_object_repair}, /* FS: Coop: Xatrix specific */
	{"rotating_light", SP_rotating_light}, /* FS: Coop: Xatrix specific */

	{"trigger_always", SP_trigger_always},
	{"trigger_once", SP_trigger_once},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_relay", SP_trigger_relay},
	{"trigger_push", SP_trigger_push},
	{"trigger_hurt", SP_trigger_hurt},
	{"trigger_key", SP_trigger_key},
	{"trigger_counter", SP_trigger_counter},
	{"trigger_elevator", SP_trigger_elevator},
	{"trigger_gravity", SP_trigger_gravity},
	{"trigger_monsterjump", SP_trigger_monsterjump},

	{"target_temp_entity", SP_target_temp_entity},
	{"target_speaker", SP_target_speaker},
	{"target_explosion", SP_target_explosion},
	{"target_changelevel", SP_target_changelevel},
	{"target_secret", SP_target_secret},
	{"target_goal", SP_target_goal},
	{"target_splash", SP_target_splash},
	{"target_spawner", SP_target_spawner},
	{"target_blaster", SP_target_blaster},
	{"target_crosslevel_trigger", SP_target_crosslevel_trigger},
	{"target_crosslevel_target", SP_target_crosslevel_target},
	{"target_laser", SP_target_laser},
	{"target_help", SP_target_help},
	{"target_actor", SP_target_actor}, /* FS: Coop: Xatrix specific */
	{"target_lightramp", SP_target_lightramp},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},

	{"target_mal_laser", SP_target_mal_laser}, /* FS: Coop: Xatrix specific */


	{"worldspawn", SP_worldspawn},
	{"viewthing", SP_viewthing},

	{"light", SP_light},
	{"light_mine1", SP_light_mine1},
	{"light_mine2", SP_light_mine2},
	{"info_null", SP_info_null},
	{"func_group", SP_info_null},
	{"info_notnull", SP_info_notnull},
	{"path_corner", SP_path_corner},
	{"point_combat", SP_point_combat},

	{"misc_explobox", SP_misc_explobox},
	{"misc_banner", SP_misc_banner},
	{"misc_satellite_dish", SP_misc_satellite_dish},
	{"misc_actor", SP_misc_actor}, /* FS: Coop: Xatrix specific */
	{"misc_gib_arm", SP_misc_gib_arm},
	{"misc_gib_leg", SP_misc_gib_leg},
	{"misc_gib_head", SP_misc_gib_head},
	{"misc_insane", SP_misc_insane},
	{"misc_deadsoldier", SP_misc_deadsoldier},
	{"misc_viper", SP_misc_viper},
	{"misc_viper_bomb", SP_misc_viper_bomb},
	{"misc_bigviper", SP_misc_bigviper},
	{"misc_strogg_ship", SP_misc_strogg_ship},
	{"misc_teleporter", SP_misc_teleporter},
	{"misc_teleporter_dest", SP_misc_teleporter_dest},
	{"misc_blackhole", SP_misc_blackhole},
	{"misc_eastertank", SP_misc_eastertank},
	{"misc_easterchick", SP_misc_easterchick},
	{"misc_easterchick2", SP_misc_easterchick2},
	{"misc_crashviper", SP_misc_crashviper}, /* FS: Coop: Xatrix specific */
	{"misc_viper_missile", SP_misc_viper_missile}, /* FS: Coop: Xatrix specific */
	{"misc_amb4", SP_misc_amb4}, /* FS: Coop: Xatrix specific */
	{"misc_transport", SP_misc_transport}, /* FS: Coop: Xatrix specific */
	{"misc_nuke", SP_misc_nuke}, /* FS: Coop: Xatrix specific */
	
	{"monster_berserk", SP_monster_berserk},
	{"monster_gladiator", SP_monster_gladiator},
	{"monster_gunner", SP_monster_gunner},
	{"monster_infantry", SP_monster_infantry},
	{"monster_soldier_light", SP_monster_soldier_light},
	{"monster_soldier", SP_monster_soldier},
	{"monster_soldier_ss", SP_monster_soldier_ss},
	{"monster_tank", SP_monster_tank},
	{"monster_tank_commander", SP_monster_tank},
	{"monster_medic", SP_monster_medic},
	{"monster_flipper", SP_monster_flipper},
	{"monster_chick", SP_monster_chick},
	{"monster_parasite", SP_monster_parasite},
	{"monster_flyer", SP_monster_flyer},
	{"monster_brain", SP_monster_brain},
	{"monster_floater", SP_monster_floater},
	{"monster_hover", SP_monster_hover},
	{"monster_mutant", SP_monster_mutant},
	{"monster_supertank", SP_monster_supertank},
	{"monster_boss2", SP_monster_boss2},
	{"monster_boss3_stand", SP_monster_boss3_stand},
	{"monster_jorg", SP_monster_jorg},

	{"monster_commander_body", SP_monster_commander_body},

	{"monster_soldier_hypergun", SP_monster_soldier_hypergun}, /* FS: Coop: Xatrix specific */
	{"monster_soldier_lasergun", SP_monster_soldier_lasergun}, /* FS: Coop: Xatrix specific */
	{"monster_soldier_ripper",	SP_monster_soldier_ripper}, /* FS: Coop: Xatrix specific */
	{"monster_fixbot", SP_monster_fixbot}, /* FS: Coop: Xatrix specific */
	{"monster_gekk", SP_monster_gekk}, /* FS: Coop: Xatrix specific */
	{"monster_chick_heat", SP_monster_chick_heat}, /* FS: Coop: Xatrix specific */
	{"monster_gladb", SP_monster_gladb}, /* FS: Coop: Xatrix specific */
	{"monster_boss5", SP_monster_boss5}, /* FS: Coop: Xatrix specific */
	
	{"turret_breach", SP_turret_breach},
	{"turret_base", SP_turret_base},
	{"turret_driver", SP_turret_driver},

	{"func_plat2", SP_func_plat2}, /* FS: Coop: Rogue specific */
	{"func_door_secret2", SP_func_door_secret2}, /* FS: Coop: Rogue specific */
	{"func_force_wall", SP_func_force_wall}, /* FS: Coop: Rogue specific */
	{"trigger_teleport", SP_trigger_teleport}, /* FS: Coop: Rogue specific */
	{"trigger_disguise", SP_trigger_disguise}, /* FS: Coop: Rogue specific */
	{"info_teleport_destination", SP_info_teleport_destination}, /* FS: Coop: Rogue specific */
	{"info_player_coop_lava", SP_info_player_coop_lava}, /* FS: Coop: Rogue specific */
	{"monster_stalker", SP_monster_stalker}, /* FS: Coop: Rogue specific */
	{"monster_turret", SP_monster_turret}, /* FS: Coop: Rogue specific */
	{"target_steam", SP_target_steam}, /* FS: Coop: Rogue specific */
	{"target_anger", SP_target_anger}, /* FS: Coop: Rogue specific */
	{"target_killplayers", SP_target_killplayers}, /* FS: Coop: Rogue specific */
	{"target_blacklight", SP_target_blacklight}, /* FS: Coop: Rogue specific */
	{"target_orb", SP_target_orb}, /* FS: Coop: Rogue specific */
	{"monster_daedalus", SP_monster_hover}, /* FS: Coop: Rogue specific */
	{"hint_path", SP_hint_path}, /* FS: Coop: Rogue specific */
	{"monster_carrier", SP_monster_carrier}, /* FS: Coop: Rogue specific */
	{"monster_widow", SP_monster_widow}, /* FS: Coop: Rogue specific */
	{"monster_widow2", SP_monster_widow2}, /* FS: Coop: Rogue specific */
	{"monster_medic_commander", SP_monster_medic}, /* FS: Coop: Rogue specific */
	{"dm_tag_token", SP_dm_tag_token}, /* FS: Coop: Rogue specific */
	{"dm_dball_goal", SP_dm_dball_goal}, /* FS: Coop: Rogue specific */
	{"dm_dball_ball", SP_dm_dball_ball}, /* FS: Coop: Rogue specific */
	{"dm_dball_team1_start", SP_dm_dball_team1_start}, /* FS: Coop: Rogue specific */
	{"dm_dball_team2_start", SP_dm_dball_team2_start}, /* FS: Coop: Rogue specific */
	{"dm_dball_ball_start", SP_dm_dball_ball_start}, /* FS: Coop: Rogue specific */
	{"dm_dball_speed_change", SP_dm_dball_speed_change}, /* FS: Coop: Rogue specific */
	{"monster_kamikaze", SP_monster_kamikaze}, /* FS: Coop: Rogue specific */
	{"turret_invisible_brain", SP_turret_invisible_brain}, /* FS: Coop: Rogue specific */
	{"misc_nuke_core", SP_misc_nuke_core}, /* FS: Coop: Rogue specific */

	{"ammo_magslug", SP_xatrix_item}, /* FS: Coop: Rogue specific */
	{"ammo_trap", SP_xatrix_item}, /* FS: Coop: Rogue specific */
	{"item_quadfire", SP_xatrix_item}, /* FS: Coop: Rogue specific */
	{"weapon_boomer", SP_xatrix_item}, /* FS: Coop: Rogue specific */
	{"weapon_phalanx", SP_xatrix_item}, /* FS: Coop: Rogue specific */

// evolve map entities
	{"sound_echo", SP_sound_echo}, /* FS: Zaero specific game dll changes */
	{"misc_ired", SP_misc_lasertripbomb}, /* FS: Zaero specific game dll changes */
	{"trigger_laser", SP_trigger_laser}, /* FS: Zaero specific game dll changes */
	{"monster_autocannon", SP_monster_autocannon}, /* FS: Zaero specific game dll changes */
	{"monster_autocannon_floor", SP_monster_autocannon_floor}, /* FS: Zaero specific game dll changes */
	{"monster_sentien", SP_monster_sentien}, /* FS: Zaero specific game dll changes */
	{"misc_securitycamera", SP_misc_securitycamera}, /* FS: Zaero specific game dll changes */
	{"monster_hound", SP_monster_hound}, /* FS: Zaero specific game dll changes */
	{"monster_handler", SP_monster_handler}, /* FS: Zaero specific game dll changes */
	{"misc_commdish", SP_misc_commdish}, /* FS: Zaero specific game dll changes */

// mirror level's 
	{"load_mirrorlevel", SP_load_mirrorlevel}, /* FS: Zaero specific game dll changes */

	{"misc_crate", SP_misc_crate}, /* FS: Zaero specific game dll changes */
	{"misc_crate_medium", SP_misc_crate_medium}, /* FS: Zaero specific game dll changes */
	{"misc_crate_small", SP_misc_crate_small}, /* FS: Zaero specific game dll changes */

	{"monster_zboss", SP_monster_zboss}, /* FS: Zaero specific game dll changes */
	{"func_barrier", SP_func_barrier}, /* FS: Zaero specific game dll changes */
	{"misc_seat", SP_misc_seat}, /* FS: Zaero specific game dll changes */
	{"target_zboss_target", SP_target_zboss_target}, /* FS: Zaero specific game dll changes */

	{"info_coop_checkpoint", SP_info_coop_checkpoint}, /* FS: Coop: Added */
	{NULL, NULL}
};

/*
 * Finds the spawn function for the entity and calls it
 */
void
ED_CallSpawn(edict_t *ent)
{
	spawn_t *s;
	gitem_t *item;
	int i;

	if (!ent)
	{
		return;
	}

	if (!ent->classname)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "ED_CallSpawn: NULL classname\n");
		G_FreeEdict(ent);
		return;
	}

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		ent->gravityVector[0] = 0.0;
		ent->gravityVector[1] = 0.0;
		ent->gravityVector[2] = -1.0;

		if (!strcmp(ent->classname, "weapon_nailgun"))
		{
			ent->classname = (FindItem("ETF Rifle"))->classname;
		}

		if (!strcmp(ent->classname, "ammo_nails"))
		{
			ent->classname = (FindItem("Flechettes"))->classname;
		}

		if (!strcmp(ent->classname, "weapon_heatbeam"))
		{
			ent->classname = (FindItem("Plasma Beam"))->classname;
		}
	}

	/* check item spawn functions */
	for (i = 0, item = itemlist; i < game.num_items; i++, item++)
	{
		if (!item->classname)
		{
			continue;
		}

		if (!strcmp(item->classname, ent->classname))
		{
		 	/* found it */
			SpawnItem(ent, item);
			return;
		}
	}

	/* check normal spawn functions */
	for (s = spawns; s->name; s++)
	{
		if (!strcmp(s->name, ent->classname))
		{
		 	/* found it */
			s->spawn(ent);
			return;
		}
	}

	gi.dprintf(DEVELOPER_MSG_GAME, "%s doesn't have a spawn function\n", ent->classname);
}

char *
ED_NewString(const char *string)
{
	char *newb, *new_p;
	int i, l;

	if (!string)
	{
		return NULL;
	}

	l = strlen(string) + 1;

	newb = gi.TagMalloc(l, TAG_LEVEL);

	new_p = newb;

	for (i = 0; i < l; i++)
	{
		if ((string[i] == '\\') && (i < l - 1))
		{
			i++;

			if (string[i] == 'n')
			{
				*new_p++ = '\n';
			}
			else
			{
				*new_p++ = '\\';
			}
		}
		else
		{
			*new_p++ = string[i];
		}
	}

	return newb;
}

/*
 * Takes a key/value pair and sets
 * the binary values in an edict
 */
void
ED_ParseField(const char *key, const char *value, edict_t *ent)
{
	field_t *f;
	byte *b;
	float v;
	vec3_t vec;

	if (!ent || !value || !key)
	{
		return;
	}

	for (f = fields; f->name; f++)
	{
		if (!(f->flags & FFL_NOSPAWN) && !Q_strcasecmp(f->name, (char *)key))
		{
		 	/* found it */
			if (f->flags & FFL_SPAWNTEMP)
			{
				b = (byte *)&st;
			}
			else
			{
				b = (byte *)ent;
			}

			switch (f->type)
			{
				case F_LSTRING:
					*(char **)(b + f->ofs) = ED_NewString(value);
					break;
				case F_VECTOR:
					sscanf(value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
					((float *)(b + f->ofs))[0] = vec[0];
					((float *)(b + f->ofs))[1] = vec[1];
					((float *)(b + f->ofs))[2] = vec[2];
					break;
				case F_INT:
					*(int *)(b + f->ofs) = (int)strtol(value, (char **)NULL, 10);
					break;
				case F_FLOAT:
					*(float *)(b + f->ofs) = atof(value);
					break;
				case F_ANGLEHACK:
					v = atof(value);
					((float *)(b + f->ofs))[0] = 0;
					((float *)(b + f->ofs))[1] = v;
					((float *)(b + f->ofs))[2] = 0;
					break;
				case F_IGNORE:
					break;
				default:
					break;
			}

			return;
		}
	}

	gi.dprintf(DEVELOPER_MSG_GAME, "%s is not a field\n", key);
}

/*
 * Parses an edict out of the given string, returning the new position
 */
char *
ED_ParseEdict(char *data, edict_t *ent)
{
	qboolean init;
	char keyname[256];
	const char *com_token;

	if (!ent)
	{
		return NULL;
	}

	init = false;
	memset(&st, 0, sizeof(st));

	/* go through all the dictionary pairs */
	while (1)
	{
		/* parse key */
		com_token = COM_Parse(&data);

		if (com_token[0] == '}')
		{
			break;
		}

		if (!data)
		{
			gi.error("ED_ParseEntity: EOF without closing brace");
		}

		strncpy(keyname, com_token, sizeof(keyname) - 1);

		/* parse value */
		com_token = COM_Parse(&data);

		if (!data)
		{
			gi.error("ED_ParseEntity: EOF without closing brace");
		}

		if (com_token[0] == '}')
		{
			gi.error("ED_ParseEntity: closing brace without data");
		}

		init = true;

		/* keynames with a leading underscore are used for
		   utility comments, and are immediately discarded
		   by quake */
		if (keyname[0] == '_')
		{
			continue;
		}

		ED_ParseField(keyname, com_token, ent);
	}

	if (!init)
	{
		memset(ent, 0, sizeof(*ent));
	}

	return data;
}

/*
 * Chain together all entities with a matching team field.
 *
 * All but the first will have the FL_TEAMSLAVE flag set.
 * All but the last will have the teamchain field set to the next one
 */
void
G_FixTeams(void) /* FS: Coop: Rogue specific */

{
	edict_t *e, *e2, *chain;
	int i, j;
	int c, c2;

	c = 0;
	c2 = 0;

	for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse)
		{
			continue;
		}

		if (!e->team)
		{
			continue;
		}

		if(!e->classname)
		{
			continue;
		}

		if (!strcmp(e->classname, "func_train"))
		{
			if (e->flags & FL_TEAMSLAVE)
			{
				chain = e;
				e->teammaster = e;
				e->teamchain = NULL;
				e->flags &= ~FL_TEAMSLAVE;
				c++;
				c2++;

				for (j = 1, e2 = g_edicts + j;
					 j < globals.num_edicts;
					 j++, e2++)
				{
					if (e2 == e)
					{
						continue;
					}

					if (!e2->inuse)
					{
						continue;
					}

					if (!e2->team)
					{
						continue;
					}

					if (!strcmp(e->team, e2->team))
					{
						c2++;
						chain->teamchain = e2;
						e2->teammaster = e;
						e2->teamchain = NULL;
						chain = e2;
						e2->flags |= FL_TEAMSLAVE;
						e2->movetype = MOVETYPE_PUSH;
						e2->speed = e->speed;
					}
				}
			}
		}
	}

	gi.dprintf(DEVELOPER_MSG_GAME, "%i teams repaired\n", c);
}

void
G_FindTeams(void)
{
	edict_t *e, *e2, *chain;
	int i, j;
	int c, c2;

	c = 0;
	c2 = 0;

	for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse)
		{
			continue;
		}

		if (!e->team)
		{
			continue;
		}

		if (e->flags & FL_TEAMSLAVE)
		{
			continue;
		}

		chain = e;
		e->teammaster = e;
		c++;
		c2++;

		for (j = i + 1, e2 = e + 1; j < globals.num_edicts; j++, e2++)
		{
			if (!e2->inuse)
			{
				continue;
			}

			if (!e2->team)
			{
				continue;
			}

			if (e2->flags & FL_TEAMSLAVE)
			{
				continue;
			}

			if (!strcmp(e->team, e2->team))
			{
				c2++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		G_FixTeams();
	}

	gi.dprintf(DEVELOPER_MSG_GAME, "%i teams with %i entities.\n", c, c2);
}

/*
 * Creates a server's entity / program execution context by
 * parsing textual entity definitions out of an ent file.
 */
void
SpawnEntities(const char *mapname, char *entities, const char *spawnpoint)
{
	edict_t *ent;
	int inhibit;
	const char *com_token;
	int i;
	float skill_level;
	int checkpoint_index; /* FS: Added */

	if (!mapname || !entities || !spawnpoint)
	{
		return;
	}

	skill_level = floor(skill->value);

	if (skill_level < 0)
	{
		skill_level = 0;
	}

	if (skill_level > 3)
	{
		skill_level = 3;
	}

	if (skill->value != skill_level)
	{
		gi.cvar_forceset("skill", va("%f", skill_level));
	}

	SaveClientData();

	gi.FreeTags(TAG_LEVEL);

	memset(&level, 0, sizeof(level));
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));

	strncpy(level.mapname, mapname, sizeof(level.mapname) - 1);
	strncpy(game.spawnpoint, spawnpoint, sizeof(game.spawnpoint) - 1);

	gi.cprintf(NULL, PRINT_HIGH, "Map: %s\n", level.mapname); /* FS: Catch potentially bad maps in console logs */

	/* set client fields on player ents */
	for (i = 0; i < game.maxclients; i++)
	{
		g_edicts[i + 1].client = game.clients + i;
	}

	ent = NULL;
	inhibit = 0;

	/* parse ents */
	while (1)
	{
		/* parse the opening brace */
		com_token = COM_Parse(&entities);

		if (!entities)
		{
			break;
		}

		if (com_token[0] != '{')
		{
			gi.error("ED_LoadFromFile: found %s when expecting {", com_token);
		}

		if (!ent)
		{
			ent = g_edicts;
		}
		else
		{
			ent = G_Spawn();
			ent->spawnflags2 = 0; /* FS: Zaero specific game dll changes */
		}

		entities = ED_ParseEdict(entities, ent);

		if (ent->classname)
		{
			/* yet another map hack */
			if (!Q_stricmp(level.mapname, "command") &&
				!Q_stricmp(ent->classname, "trigger_once") &&
				ent->model &&
				!Q_stricmp(ent->model, "*27"))
			{
				ent->spawnflags &= ~SPAWNFLAG_NOT_HARD;
			}

			/* ahh, the joys of map hacks .. */
			/* FS: Coop: Rogue specific map hacks */
			if (!Q_stricmp(level.mapname, "rhangar2") &&
				!Q_stricmp(ent->classname, "func_door_rotating") &&
				ent->targetname && !Q_stricmp(ent->targetname, "t265"))
			{
				ent->spawnflags &= ~SPAWNFLAG_NOT_COOP;
			}

			if (!Q_stricmp(level.mapname, "rhangar2") &&
				!Q_stricmp(ent->classname, "trigger_always") &&
		   		ent->target && !Q_stricmp(ent->target, "t265"))
			{
				ent->spawnflags |= SPAWNFLAG_NOT_COOP;
			}

			if (!Q_stricmp(level.mapname, "rhangar2") &&
				!Q_stricmp(ent->classname, "func_wall") &&
				ent->model &&
		   		!Q_stricmp(ent->model, "*15"))
			{
				ent->spawnflags |= SPAWNFLAG_NOT_COOP;
			}
		}

		/* remove things (except the world) from
		   different skill levels or deathmatch */
		if (ent != g_edicts)
		{
			if (deathmatch->intValue)
			{
				if (ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
				{
					G_FreeEdict(ent);
					inhibit++;
					continue;
				}
			}
			else if (coop->intValue)
			{
				if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
				{
					if (ent->spawnflags & SPAWNFLAG_NOT_COOP)
					{
						G_FreeEdict(ent);
						inhibit++;
						continue;
					}

					/* stuff marked !easy & !med & !hard are coop only, all levels */
					if (!((ent->spawnflags & SPAWNFLAG_NOT_EASY) &&
						  (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM) &&
						  (ent->spawnflags & SPAWNFLAG_NOT_HARD)))
					{
						if (((skill->intValue == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
							((skill->intValue == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
							(((skill->intValue == 2) || (skill->intValue == 3)) && (ent->spawnflags & SPAWNFLAG_NOT_HARD)))
						{
							G_FreeEdict(ent);
							inhibit++;
							continue;
						}
					}
				}
				else
				{
					if(Spawn_CheckCoop_MapHacks(ent)) /* FS: Coop: Check if we have to modify some stuff for coop so we don't have to rely on distributing ent files. */
					{
						G_FreeEdict(ent);
						inhibit++;
						continue;
					}

					if (((skill->intValue == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
						((skill->intValue == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
						(((skill->intValue == 2) || (skill->intValue == 3)) && (ent->spawnflags & SPAWNFLAG_NOT_HARD)))
					{
						G_FreeEdict(ent);
						inhibit++;
						continue;
					}
				}
			}
			else /* FS: Coop: Single player */
			{
				if (((skill->intValue == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->intValue == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					(((skill->intValue == 2) || (skill->intValue == 3)) && (ent->spawnflags & SPAWNFLAG_NOT_HARD)))
				{
					G_FreeEdict(ent);
					inhibit++;
					continue;
				}
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY | SPAWNFLAG_NOT_MEDIUM |
				  SPAWNFLAG_NOT_HARD | SPAWNFLAG_NOT_COOP | SPAWNFLAG_NOT_DEATHMATCH);
		}

		if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
		{
			ent->gravityVector[0] = 0.0;
			ent->gravityVector[1] = 0.0;
			ent->gravityVector[2] = -1.0;
		}

		ED_CallSpawn(ent);

		ent->s.renderfx |= RF_IR_VISIBLE; /* FS: Coop: Rogue specific.  Probably OK as-is. */
	}

	if (coop->intValue) /* FS: Coop: Find checkpoints to spawn if we got them */
	{
		inhibit += G_SpawnCheckpoints(ent);
	}

	gi.dprintf(DEVELOPER_MSG_GAME, "%i entities inhibited.\n", inhibit);

	G_FindTeams();

	PlayerTrail_Init();

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		if (deathmatch->intValue)
		{
			if (randomrespawn && randomrespawn->intValue)
			{
				PrecacheForRandomRespawn();
			}
		}
		else
		{
			InitHintPaths();
		}

		if (deathmatch->intValue && gamerules && gamerules->intValue)
		{
			if (DMGame.PostInitSetup)
			{
				DMGame.PostInitSetup();
			}
		}
	}

	if (game.gametype == zaero_coop)
	{
		Z_SpawnDMItems(); /* FS: Zaero specific game dll changes */
	}

	if(coop->intValue) /* FS: Coop: Check if victory.pcx is the current map, workaround the "gamemap" crap in sv_init.c */
	{
		G_CheckCoopVictory();
	}

	checkpoint_index = gi.modelindex("models/items/tagtoken/tris.md2"); /* FS: info_coop_checkpoint */
	if(checkpoint_index < 220) /* FS: Moved this because of MAX_MODELS overflows in city3.bsp and probably others */
	{
		/* sexed models: THIS ORDER MUST MATCH THE DEFINES IN g_local.h
		   you can add more, max 19 (pete change)these models are only
		   loaded in coop or deathmatch. not singleplayer. */
		if ((coop->intValue) || deathmatch->intValue)
		{
			gi.modelindex("#w_blaster.md2");
			gi.modelindex("#w_shotgun.md2");
			gi.modelindex("#w_sshotgun.md2");
			gi.modelindex("#w_machinegun.md2");
			gi.modelindex("#w_chaingun.md2");
			gi.modelindex("#a_grenades.md2");
			gi.modelindex("#w_glauncher.md2");
			gi.modelindex("#w_rlauncher.md2");
			gi.modelindex("#w_hyperblaster.md2");
			gi.modelindex("#w_railgun.md2");
			gi.modelindex("#w_bfg.md2");
			if (game.gametype == rogue_coop)
			{
				gi.modelindex("#w_disrupt.md2"); /* FS: Coop: Rogue specific */
				gi.modelindex("#w_etfrifle.md2"); /* FS: Coop: Rogue specific */
				gi.modelindex("#w_plasma.md2"); /* FS: Coop: Rogue specific */
				gi.modelindex("#w_plauncher.md2"); /* FS: Coop: Rogue specific */
				gi.modelindex("#w_chainfist.md2"); /* FS: Coop: Rogue specific */
			}
			if (game.gametype == xatrix_coop)
			{
				gi.modelindex("#w_phalanx.md2"); /* FS: Coop: Xatrix specific */
				gi.modelindex("#w_ripper.md2"); /* FS: Coop: Xatrix specific */
			}
		}
	}
}

/* =================================================================== */

char *single_statusbar =
"yb	-24 "

/* health */
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

/* ammo */
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

/* armor */
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

/* selected item */
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

/* picked up item */
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

/* timer */
"if 9 "
"	xv	262 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

/*  help / weapon icon */
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

/* spectator */
"if 17 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

/* chase camera */
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "

// player origin /* FS: Zaero specific game dll changes */
"if 18 " /* FS: Must mirror STAT_SHOW_ORIGIN! */
"	xl 0 "
"	yb -120 "
"	num 5 19 " /* FS: Must mirror STAT_ORIGIN_X! */
"	yb -96 "
"	num 5 20 " /* FS: Must mirror STAT_ORIGIN_Y! */
"	yb -72 "
"	num 5 21 " /* FS: Must mirror STAT_ORIGIN_Z! */
"endif "
;

char *dm_statusbar =
"yb	-24 "

/* health */
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

/* ammo */
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

/* armor */
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

/* selected item */
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

/* picked up item */
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

/* timer */
"if 9 "
"	xv	246 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

/*  help / weapon icon */
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

/*  frags */
"xr	-50 "
"yt 2 "
"num 3 14 "

// player origin /* FS: Zaero specific game dll changes */
"if 18 " /* FS: Must mirror STAT_SHOW_ORIGIN! */
"	xl 0 "
"	yb -120 "
"	num 5 19 " /* FS: Must mirror STAT_ORIGIN_X! */
"	yb -96 "
"	num 5 20 " /* FS: Must mirror STAT_ORIGIN_Y! */
"	yb -72 "
"	num 5 21 " /* FS: Must mirror STAT_ORIGIN_Z! */
"endif "

/* spectator */
"if 17 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

/* chase camera */
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "
;

/*
 * QUAKED worldspawn (0 0 0) ?
 *
 * Only used for the world.
 * "sky"	environment map name
 * "skyaxis"	vector axis for rotating sky
 * "skyrotate"	speed of rotation in degrees/second
 * "sounds"	music cd track number
 * "gravity"	800 is default gravity
 * "message"	text to print at user logon
 */
void
SP_worldspawn(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true; /* since the world doesn't use G_Spawn() */
	ent->s.modelindex = 1; /* world model is always index 1 */
	ent->spawnflags2 = 0; /* FS: Zaero specific game dll changes */

	/* reserve some spots for dead player
	   bodies for coop / deathmatch */
	InitBodyQue();

	/* set configstrings for items */
	SetItemNames();

	if (st.nextmap)
	{
		strcpy(level.nextmap, st.nextmap);
	}

	/* make some data visible to the server */
	if (ent->message && ent->message[0])
	{
		gi.configstring(CS_NAME, ent->message);
		strncpy(level.level_name, ent->message, sizeof(level.level_name));
	}
	else
	{
		strncpy(level.level_name, level.mapname, sizeof(level.level_name));
	}

	if (st.sky && st.sky[0])
	{
		gi.configstring(CS_SKY, st.sky);
	}
	else
	{
		gi.configstring(CS_SKY, "unit1_");
	}

	gi.configstring(CS_SKYROTATE, va("%f", st.skyrotate));
	gi.configstring(CS_SKYAXIS, va("%f %f %f", st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]));
	SP_SetCDTrack(ent->sounds); /* FS: Coop: Moved here to get it right for all game modes */
	gi.configstring(CS_MAXCLIENTS, va("%i", maxclients->intValue));

	/* status bar program */
	if (deathmatch->intValue)
	{
		gi.configstring(CS_STATUSBAR, dm_statusbar);
	}
	else
	{
		gi.configstring(CS_STATUSBAR, single_statusbar);
	}

	/* help icon for statusbar */
	gi.imageindex("i_help");
	level.pic_health = gi.imageindex("i_health");
	gi.imageindex("help");
	gi.imageindex("field_3");

	if (!st.gravity)
	{
		gi.cvar_set("sv_gravity", "800");
	}
	else
	{
		gi.cvar_set("sv_gravity", st.gravity);
	}

	snd_fry = gi.soundindex("player/fry.wav"); /* standing in lava / slime */

	PrecacheItem(FindItem("Blaster"));

	gi.soundindex("player/lava1.wav");
	gi.soundindex("player/lava2.wav");

	gi.soundindex("misc/pc_up.wav");
	gi.soundindex("misc/talk1.wav");

	gi.soundindex("misc/udeath.wav");

	/* gibs */
	gi.soundindex("items/respawn1.wav");

	/* sexed sounds */
	gi.soundindex("*death1.wav");
	gi.soundindex("*death2.wav");
	gi.soundindex("*death3.wav");
	gi.soundindex("*death4.wav");
	gi.soundindex("*fall1.wav");
	gi.soundindex("*fall2.wav");
	gi.soundindex("*gurp1.wav"); /* drowning damage */
	gi.soundindex("*gurp2.wav");
	gi.soundindex("*jump1.wav"); /* player jump */
	gi.soundindex("*pain25_1.wav");
	gi.soundindex("*pain25_2.wav");
	gi.soundindex("*pain50_1.wav");
	gi.soundindex("*pain50_2.wav");
	gi.soundindex("*pain75_1.wav");
	gi.soundindex("*pain75_2.wav");
	gi.soundindex("*pain100_1.wav");
	gi.soundindex("*pain100_2.wav");

	/* FS: Sexed models moved to avoid MAX_MODELS overflows */

	/* ------------------- */

	gi.soundindex("player/gasp1.wav");      /* gasping for air */
	gi.soundindex("player/gasp2.wav");      /* head breaking surface, not gasping */

	gi.soundindex("player/watr_in.wav");    /* feet hitting water */
	gi.soundindex("player/watr_out.wav");   /* feet leaving water */

	gi.soundindex("player/watr_un.wav");    /* head going underwater */

	gi.soundindex("player/u_breath1.wav");
	gi.soundindex("player/u_breath2.wav");

	gi.soundindex("items/pkup.wav");        /* bonus item pickup */
	gi.soundindex("world/land.wav");        /* landing thud */
	gi.soundindex("misc/h2ohit1.wav");      /* landing splash */

	gi.soundindex("items/damage.wav");
	if (game.gametype == rogue_coop)
	{
		gi.soundindex("misc/ddamage1.wav"); /* FS: Coop: Rogue specific */
	}
	gi.soundindex("items/protect.wav");
	gi.soundindex("items/protect4.wav");
	gi.soundindex("weapons/noammo.wav");

	gi.soundindex("infantry/inflies1.wav");

	if (game.gametype == zaero_coop)
	{
		/* FS: Coop: Zaero */
		gi.imageindex ("a_flares");
		gi.imageindex ("w_flare");
		gi.modelindex ("models/weapons/v_flare/tris.md2");
		gi.modelindex ("models/objects/flare/tris.md2");
		gi.soundindex ("weapons/flare/flarehis.wav");
		gi.soundindex ("weapons/flare/shoot.wav");
		gi.modelindex ("models/items/plasma/tris.md2");
		gi.imageindex ("i_plasma");
		gi.modelindex ("sprites/plasmashield_fixed.sp2");
		gi.modelindex ("sprites/plasma1_fixed.sp2");
		gi.soundindex ("items/plasmashield/psactive.wav");
		gi.imageindex ("w_enuke");
		gi.modelindex ("models/weapons/g_enuke/tris.md2");
		gi.modelindex ("models/weapons/v_enuke/tris.md2");
		gi.soundindex ("items/empnuke/emp_trg.wav");
		gi.imageindex ("w_a2k");
		gi.modelindex ("models/weapons/g_a2k/tris.md2");
		gi.modelindex ("models/weapons/v_a2k/tris.md2");
		gi.soundindex ("weapons/a2k/countdn.wav");
		gi.soundindex ("weapons/a2k/ak_exp01.wav");
		gi.imageindex ("w_sonic");
		gi.modelindex ("models/weapons/g_sonic/tris.md2");
		gi.modelindex ("models/weapons/v_sonic/tris.md2");
		gi.soundindex ("weapons/sonic/sc_warm.wav"); 
		gi.soundindex ("weapons/sonic/sc_cool.wav"); 
		gi.soundindex ("weapons/sonic/sc_fire.wav");
		gi.imageindex ("w_sniper");
		gi.modelindex ("models/weapons/g_sniper/tris.md2");
		gi.modelindex ("models/weapons/v_sniper/tris.md2");
		gi.modelindex ("models/weapons/v_sniper/scope/tris.md2"); 
		gi.modelindex ("models/weapons/v_sniper/dmscope/tris.md2"); 
		gi.soundindex ("weapons/sniper/beep.wav"); 
		gi.soundindex ("weapons/sniper/fire.wav");
	}

	sm_meat_index = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
	gi.modelindex("models/objects/gibs/arm/tris.md2");
	gi.modelindex("models/objects/gibs/bone/tris.md2");
	gi.modelindex("models/objects/gibs/bone2/tris.md2");
	gi.modelindex("models/objects/gibs/chest/tris.md2");
	gi.modelindex("models/objects/gibs/skull/tris.md2");
	gi.modelindex("models/objects/gibs/head2/tris.md2");

	/* Setup light animation tables. 'a' is total darkness, 'z' is doublebright. */

	/* 0 normal */
	gi.configstring(CS_LIGHTS + 0, "m");

	/* 1 FLICKER (first variety) */
	gi.configstring(CS_LIGHTS + 1, "mmnmmommommnonmmonqnmmo");

	/* 2 SLOW STRONG PULSE */
	gi.configstring(CS_LIGHTS + 2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	/* 3 CANDLE (first variety) */
	gi.configstring(CS_LIGHTS + 3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	/* 4 FAST STROBE */
	gi.configstring(CS_LIGHTS + 4, "mamamamamama");

	/* 5 GENTLE PULSE 1 */
	gi.configstring(CS_LIGHTS + 5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	/* 6 FLICKER (second variety) */
	gi.configstring(CS_LIGHTS + 6, "nmonqnmomnmomomno");

	/* 7 CANDLE (second variety) */
	gi.configstring(CS_LIGHTS + 7, "mmmaaaabcdefgmmmmaaaammmaamm");

	/* 8 CANDLE (third variety) */
	gi.configstring(CS_LIGHTS + 8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

	/* 9 SLOW STROBE (fourth variety) */
	gi.configstring(CS_LIGHTS + 9, "aaaaaaaazzzzzzzz");

	/* 10 FLUORESCENT FLICKER */
	gi.configstring(CS_LIGHTS + 10, "mmamammmmammamamaaamammma");

	/* 11 SLOW PULSE NOT FADE TO BLACK */
	gi.configstring(CS_LIGHTS + 11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

	/* styles 32-62 are assigned by the light program for switchable lights */

	/* 63 testing */
	gi.configstring(CS_LIGHTS + 63, "a");
}

/*
 * Monster spawning code:
 * Used by the carrier, the medic_commander, and the black widow
 *
 * The sequence to create a flying monster is:
 *  FindSpawnPoint - tries to find suitable spot to spawn the monster in
 *  CreateFlyMonster  - this verifies the point as good and creates the monster
 *
 * To create a ground walking monster:
 *  FindSpawnPoint - same thing
 *  CreateGroundMonster - this checks the volume and makes sure the floor under the volume is suitable
 */

edict_t *
CreateMonster(vec3_t origin, vec3_t angles, char *classname) /* FS: Coop: Rogue specific */
{
	edict_t *newEnt;

	if (!classname)
	{
		return NULL;
	}

	newEnt = G_Spawn();

	VectorCopy(origin, newEnt->s.origin);
	VectorCopy(angles, newEnt->s.angles);
	newEnt->classname = ED_NewString(classname);
	newEnt->monsterinfo.aiflags |= AI_DO_NOT_COUNT;

	VectorSet(newEnt->gravityVector, 0, 0, -1);
	ED_CallSpawn(newEnt);
	newEnt->s.renderfx |= RF_IR_VISIBLE;

	return newEnt;
}

edict_t *
CreateFlyMonster(vec3_t origin, vec3_t angles, vec3_t mins, /* FS: Coop: Rogue specific */
		vec3_t maxs, char *classname)
{
	if (!classname)
	{
		return NULL;
	}

	if (!mins || !maxs ||
		VectorCompare(mins, vec3_origin) || VectorCompare(maxs, vec3_origin))
	{
		DetermineBBox(classname, mins, maxs);
	}

	if (!CheckSpawnPoint(origin, mins, maxs))
	{
		return NULL;
	}

	return CreateMonster(origin, angles, classname);
}

edict_t *
CreateGroundMonster(vec3_t origin, vec3_t angles, vec3_t entMins, /* FS: Coop: Rogue specific */
		vec3_t entMaxs, char *classname, int height)
{
	edict_t *newEnt;
	vec3_t mins, maxs;

	if (!classname)
	{
		return NULL;
	}

	/* if they don't provide us a bounding box, figure it out */
	if (!entMins || !entMaxs || VectorCompare(entMins,
				vec3_origin) || VectorCompare(entMaxs, vec3_origin))
	{
		DetermineBBox(classname, mins, maxs);
	}
	else
	{
		VectorCopy(entMins, mins);
		VectorCopy(entMaxs, maxs);
	}

	/* check the ground to make sure it's there, it's relatively flat, and it's not toxic */
	if (!CheckGroundSpawnPoint(origin, mins, maxs, height, -1))
	{
		return NULL;
	}

	newEnt = CreateMonster(origin, angles, classname);

	if (!newEnt)
	{
		return NULL;
	}

	return newEnt;
}

qboolean
FindSpawnPoint(vec3_t startpoint, vec3_t mins, vec3_t maxs, /* FS: Coop: Rogue specific */
		vec3_t spawnpoint, float maxMoveUp)
{
	trace_t tr;
	vec3_t top;

	tr = gi.trace(startpoint, mins, maxs, startpoint,
			NULL, MASK_MONSTERSOLID | CONTENTS_PLAYERCLIP);

	if ((tr.startsolid || tr.allsolid) || (tr.ent != world))
	{
		VectorCopy(startpoint, top);
		top[2] += maxMoveUp;

		tr = gi.trace(top, mins, maxs, startpoint, NULL, MASK_MONSTERSOLID);

		if (tr.startsolid || tr.allsolid)
		{
			return false;
		}
		else
		{
			VectorCopy(tr.endpos, spawnpoint);
			return true;
		}
	}
	else
	{
		VectorCopy(startpoint, spawnpoint);
		return true;
	}
}

qboolean
CheckSpawnPoint(vec3_t origin, vec3_t mins, vec3_t maxs) /* FS: Coop: Rogue specific */
{
	trace_t tr;

	if (!mins || !maxs ||
		VectorCompare(mins, vec3_origin) || VectorCompare(maxs, vec3_origin))
	{
		return false;
	}

	tr = gi.trace(origin, mins, maxs, origin, NULL, MASK_MONSTERSOLID);

	if (tr.startsolid || tr.allsolid)
	{
		return false;
	}

	if (tr.ent != world)
	{
		return false;
	}

	return true;
}

qboolean
CheckGroundSpawnPoint(vec3_t origin, vec3_t entMins, vec3_t entMaxs, /* FS: Coop: Rogue specific */
		float height, float gravity)
{
	trace_t tr;
	vec3_t start, stop;
	vec3_t mins, maxs;
	int x, y;
	float mid, bottom;

	if (!CheckSpawnPoint(origin, entMins, entMaxs))
	{
		return false;
	}


	VectorCopy(origin, stop);
	stop[2] = origin[2] + entMins[2] - height;

	tr = gi.trace(origin, entMins, entMaxs, stop,
			NULL, MASK_MONSTERSOLID | MASK_WATER);

	if ((tr.fraction < 1) && (tr.contents & MASK_MONSTERSOLID))
	{
		/* first, do the midpoint trace */
		VectorAdd(tr.endpos, entMins, mins);
		VectorAdd(tr.endpos, entMaxs, maxs);

		/* first, do the easy flat check */
		if (gravity > 0)
		{
			start[2] = maxs[2] + 1;
		}
		else
		{
			start[2] = mins[2] - 1;
		}

		for (x = 0; x <= 1; x++)
		{
			for (y = 0; y <= 1; y++)
			{
				start[0] = x ? maxs[0] : mins[0];
				start[1] = y ? maxs[1] : mins[1];

				if (gi.pointcontents(start) != CONTENTS_SOLID)
				{
					goto realcheck;
				}
			}
		}

		/* if it passed all four above checks, we're done */
		return true;

	realcheck:

		/* check it for real */
		start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
		start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
		start[2] = mins[2];

		tr = gi.trace(start, vec3_origin, vec3_origin,
				stop, NULL, MASK_MONSTERSOLID);

		if (tr.fraction == 1.0)
		{
			return false;
		}

		if (gravity < 0)
		{
			start[2] = mins[2];
			stop[2] = start[2] - STEPSIZE - STEPSIZE;
			mid = bottom = tr.endpos[2] + entMins[2];
		}
		else
		{
			start[2] = maxs[2];
			stop[2] = start[2] + STEPSIZE + STEPSIZE;
			mid = bottom = tr.endpos[2] - entMaxs[2];
		}

		for (x = 0; x <= 1; x++)
		{
			for (y = 0; y <= 1; y++)
			{
				start[0] = stop[0] = x ? maxs[0] : mins[0];
				start[1] = stop[1] = y ? maxs[1] : mins[1];

				tr = gi.trace(start, vec3_origin, vec3_origin,
						stop, NULL, MASK_MONSTERSOLID);

				if (gravity > 0)
				{
					if ((tr.fraction != 1.0) && (tr.endpos[2] < bottom))
					{
						bottom = tr.endpos[2];
					}

					if ((tr.fraction == 1.0) || (tr.endpos[2] - mid > STEPSIZE))
					{
						return false;
					}
				}
				else
				{
					if ((tr.fraction != 1.0) && (tr.endpos[2] > bottom))
					{
						bottom = tr.endpos[2];
					}

					if ((tr.fraction == 1.0) || (mid - tr.endpos[2] > STEPSIZE))
					{
						return false;
					}
				}
			}
		}

		return true; /* we can land on it, it's ok */
	}

	/* otherwise, it's either water (bad) or not
	 * there (too far) if we're here, it's bad below */
	return false;
}

void
DetermineBBox(char *classname, vec3_t mins, vec3_t maxs) /* FS: Coop: Rogue specific */
{
	edict_t *newEnt;

	if (!classname)
	{
		return;
	}

	newEnt = G_Spawn();

	VectorCopy(vec3_origin, newEnt->s.origin);
	VectorCopy(vec3_origin, newEnt->s.angles);
	newEnt->classname = ED_NewString(classname);
	newEnt->monsterinfo.aiflags |= AI_DO_NOT_COUNT;

	ED_CallSpawn(newEnt);

	VectorCopy(newEnt->mins, mins);
	VectorCopy(newEnt->maxs, maxs);

	G_FreeEdict(newEnt);
}


void
spawngrow_think(edict_t *self) /* FS: Coop: Rogue specific */
{
	int i;

	if (!self)
	{
		return;
	}

	for (i = 0; i < 2; i++)
	{
		self->s.angles[0] = rand() % 360;
		self->s.angles[1] = rand() % 360;
		self->s.angles[2] = rand() % 360;
	}

	if ((level.time < self->wait) && (self->s.frame < 2))
	{
		self->s.frame++;
	}

	if (level.time >= self->wait)
	{
		if (self->s.effects & EF_SPHERETRANS)
		{
			G_FreeEdict(self);
			return;
		}
		else if (self->s.frame > 0)
		{
			self->s.frame--;
		}
		else
		{
			G_FreeEdict(self);
			return;
		}
	}

	self->nextthink += FRAMETIME;
}

void
SpawnGrow_Spawn(vec3_t startpos, int size) /* FS: Coop: Rogue specific */
{
	edict_t *ent;
	int i;
	float lifespan;

	ent = G_Spawn();
	VectorCopy(startpos, ent->s.origin);

	for (i = 0; i < 2; i++)
	{
		ent->s.angles[0] = rand() % 360;
		ent->s.angles[1] = rand() % 360;
		ent->s.angles[2] = rand() % 360;
	}

	ent->solid = SOLID_NOT;
	ent->s.renderfx = RF_IR_VISIBLE;
	ent->movetype = MOVETYPE_NONE;
	ent->classname = "spawngro";

	if (size <= 1)
	{
		lifespan = SPAWNGROW_LIFESPAN;
		ent->s.modelindex = gi.modelindex("models/items/spawngro2/tris.md2");
	}
	else if (size == 2)
	{
		ent->s.modelindex = gi.modelindex("models/items/spawngro3/tris.md2");
		lifespan = 2;
	}
	else
	{
		ent->s.modelindex = gi.modelindex("models/items/spawngro/tris.md2");
		lifespan = SPAWNGROW_LIFESPAN;
	}

	ent->think = spawngrow_think;

	ent->wait = level.time + lifespan;
	ent->nextthink = level.time + FRAMETIME;

	if (size != 2)
	{
		ent->s.effects |= EF_SPHERETRANS;
	}

	gi.linkentity(ent);
}

void
widowlegs_think(edict_t *self) /* FS: Coop: Rogue specific */
{
	vec3_t offset;
	vec3_t point;
	vec3_t f, r, u;

	if (!self)
	{
		return;
	}

	if (self->s.frame == 17)
	{
		VectorSet(offset, 11.77, -7.24, 23.31);
		AngleVectors(self->s.angles, f, r, u);
		G_ProjectSource2(self->s.origin, offset, f, r, u, point);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_EXPLOSION1);
		gi.WritePosition(point);
		gi.multicast(point, MULTICAST_ALL);
		ThrowSmallStuff(self, point);
	}

	if (self->s.frame < MAX_LEGSFRAME)
	{
		self->s.frame++;
		self->nextthink = level.time + FRAMETIME;
		return;
	}
	else if (self->wait == 0)
	{
		self->wait = level.time + LEG_WAIT_TIME;
	}

	if (level.time > self->wait)
	{
		AngleVectors(self->s.angles, f, r, u);

		VectorSet(offset, -65.6, -8.44, 28.59);
		G_ProjectSource2(self->s.origin, offset, f, r, u, point);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_EXPLOSION1);
		gi.WritePosition(point);
		gi.multicast(point, MULTICAST_ALL);
		ThrowSmallStuff(self, point);

		ThrowWidowGibSized(self, "models/monsters/blackwidow/gib1/tris.md2",
				80 + (int)(random() * 20.0), GIB_METALLIC, point, 0, true);
		ThrowWidowGibSized(self, "models/monsters/blackwidow/gib2/tris.md2",
				80 + (int)(random() * 20.0), GIB_METALLIC, point, 0, true);

		VectorSet(offset, -1.04, -51.18, 7.04);
		G_ProjectSource2(self->s.origin, offset, f, r, u, point);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_EXPLOSION1);
		gi.WritePosition(point);
		gi.multicast(point, MULTICAST_ALL);
		ThrowSmallStuff(self, point);

		ThrowWidowGibSized(self, "models/monsters/blackwidow/gib1/tris.md2",
				80 + (int)(random() * 20.0), GIB_METALLIC, point, 0, true);
		ThrowWidowGibSized(self, "models/monsters/blackwidow/gib2/tris.md2",
				80 + (int)(random() * 20.0), GIB_METALLIC, point, 0, true);
		ThrowWidowGibSized(self, "models/monsters/blackwidow/gib3/tris.md2",
				80 + (int)(random() * 20.0), GIB_METALLIC, point, 0, true);

		G_FreeEdict(self);
		return;
	}

	if ((level.time > (self->wait - 0.5)) && (self->count == 0))
	{
		self->count = 1;
		AngleVectors(self->s.angles, f, r, u);

		VectorSet(offset, 31, -88.7, 10.96);
		G_ProjectSource2(self->s.origin, offset, f, r, u, point);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_EXPLOSION1);
		gi.WritePosition(point);
		gi.multicast(point, MULTICAST_ALL);

		VectorSet(offset, -12.67, -4.39, 15.68);
		G_ProjectSource2(self->s.origin, offset, f, r, u, point);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_EXPLOSION1);
		gi.WritePosition(point);
		gi.multicast(point, MULTICAST_ALL);

		self->nextthink = level.time + FRAMETIME;
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void
Widowlegs_Spawn(vec3_t startpos, vec3_t angles) /* FS: Coop: Rogue specific */
{
	edict_t *ent;

	ent = G_Spawn();
	VectorCopy(startpos, ent->s.origin);
	VectorCopy(angles, ent->s.angles);
	ent->solid = SOLID_NOT;
	ent->s.renderfx = RF_IR_VISIBLE;
	ent->movetype = MOVETYPE_NONE;
	ent->classname = "widowlegs";

	ent->s.modelindex = gi.modelindex("models/monsters/legs/tris.md2");
	ent->think = widowlegs_think;

	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity(ent);
}

void
SP_SetCDTrack(int track) /* FS: Coop: Added */
{
	int modifiedTrack;

	if (game.gametype == rogue_coop)
	{
		if (track >= 2 && track <= 11)
			modifiedTrack = track + 10;
		else
			modifiedTrack = track;
	}
	// an out-of-order mix from Q2 and Rogue CDs
	else if (game.gametype == xatrix_coop)
	{
		switch(track)
		{
			case 2: modifiedTrack = 9;	break;
			case 3: modifiedTrack = 13;	break;
			case 4: modifiedTrack = 14;	break;
			case 5: modifiedTrack = 7;	break;
			case 6: modifiedTrack = 16;	break;
			case 7: modifiedTrack = 2;	break;
			case 8: modifiedTrack = 15;	break;
			case 9: modifiedTrack = 3;	break;
			case 10: modifiedTrack = 4;	break;
			case 11: modifiedTrack = 18; break;
			default: modifiedTrack = track; break;
		}
	}
	else
		modifiedTrack = track;

	gi.configstring(CS_CDTRACK, va("%i", modifiedTrack));
}

qboolean Spawn_CheckCoop_MapHacks (edict_t *ent) /* FS: Coop: Check if we have to modify some stuff for coop so we don't have to rely on distributing ent files. */
{
	if(!coop->intValue || !ent)
	{
		return false;
	}

	if (game.gametype == xatrix_coop) /* FS: Coop: Progress breaker hack in xsewer1.bsp */
	{
		if(!Q_stricmp(level.mapname, "xsewer1"))
		{
			if(ent->classname && !Q_stricmp(ent->classname, "trigger_relay") && ent->target && !Q_stricmp(ent->target, "t3") && ent->targetname && !Q_stricmp(ent->targetname, "t2"))
			{
				return true;
			}

			if(ent->classname && !Q_stricmp(ent->classname, "func_button") && ent->target && !Q_stricmp(ent->target, "t16") && ent->model && !Q_stricmp(ent->model, "*71"))
			{
				ent->message = "Overflow valve maintenance\nhatch A opened.";
				return false;
			}

			if(ent->classname && !Q_stricmp(ent->classname, "trigger_once") && ent->model && !Q_stricmp(ent->model, "*3"))
			{
				ent->message = "Overflow valve maintenance\nhatch B opened.";
				return false;
			}
		}
	}

	return false;
}

int G_SpawnCheckpoints (edict_t *ent)
{
	char fileName[MAX_OSPATH];
	FILE *f = NULL;
	long fileSize;
	char *fileBuffer = NULL;
	size_t toEOF = 0;

	int inhibit = 0, found = 0;
	const char *com_token;

	Sys_Mkdir(va("%s/maps", gamedir->string));
	Com_sprintf(fileName, sizeof(fileName), "%s/maps/%s_checkpoints.txt", gamedir->string, level.mapname);
	f = fopen(fileName, "r");
	if(!f)
	{
//		gi.cprintf(NULL, PRINT_CHAT, "G_SpawnCheckpoints: couldn't find '%s'!\n", fileName);
		return 0;
	}

	/* obtain file size */
	fseek (f, 0, SEEK_END);
	fileSize = ftell (f);
	fseek (f, 0, SEEK_SET);
	fileBuffer = (char *)gi.TagMalloc((sizeof(char)*(fileSize+2)), TAG_LEVEL); /* FS: In case we have to add a newline terminator */
	if(!fileBuffer)
	{
		gi.cprintf(NULL, PRINT_CHAT, "G_SpawnCheckpoints: can't allocate memory for fileBuffer!\n");
		fclose(f);
		return 0;
	}

	toEOF = fread(fileBuffer, sizeof(char), fileSize, f);
	fclose(f);
	if(toEOF <= 0)
	{
		gi.cprintf(NULL, PRINT_CHAT, "G_SpawnCheckpoints: cannot read file '%s' into memory!\n", fileName);
		if(fileBuffer)
		{
			gi.TagFree(fileBuffer);
		}
		return false;
	}

	/* FS: Add newline terminator for some paranoia */
	fileBuffer[toEOF] = '\n';
	fileBuffer[toEOF+1] = '\0';

	/* parse ents */
	while (1)
	{
		/* parse the opening brace */
		com_token = COM_Parse(&fileBuffer);

		if (!fileBuffer)
		{
			break;
		}

		if (com_token[0] != '{')
		{
			gi.error("ED_LoadFromFile: found %s when expecting {", com_token);
		}

		if (!ent)
		{
			ent = g_edicts;
		}
		else
		{
			ent = G_Spawn();
		}

		fileBuffer = ED_ParseEdict(fileBuffer, ent);

		if (ent != g_edicts)
		{
			if(!ent->classname || Q_stricmp(ent->classname,"info_coop_checkpoint")) /* FS: No funky stuff in these special overrides please, use ent files for that. */
			{
				gi.cprintf(NULL, PRINT_CHAT, "WARNING: Not an info_coop_checkpoint: %s.  Removing...\n", ent->classname);
				G_FreeEdict(ent);
				inhibit++;
				continue;
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY | SPAWNFLAG_NOT_MEDIUM |
				  SPAWNFLAG_NOT_HARD | SPAWNFLAG_NOT_COOP | SPAWNFLAG_NOT_DEATHMATCH);
		}

		if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
		{
			ent->gravityVector[0] = 0.0;
			ent->gravityVector[1] = 0.0;
			ent->gravityVector[2] = -1.0;
		}

		ED_CallSpawn(ent);

		ent->s.renderfx |= RF_IR_VISIBLE; /* FS: Coop: Rogue specific.  Probably OK as-is. */
		inhibit++;
		found++;
	}

	if(found)
	{
		gi.cprintf(NULL, PRINT_CHAT, "Found %i checkpoints to add\n", found);
	}
	return inhibit;
}

//QW// Generalized to detect .pcx used as a map.
void G_CheckCoopVictory (void) /* FS: Coop: Check if a pcx is the current map, workaround the "gamemap" crap in sv_init.c */
{
	if (strstr(level.mapname, ".pcx"))
	{
		switch (game.gametype)
		{
			default:
			case vanilla_coop:
				gi.cvar_forceset("sv_coop_gamemode", "xatrix");
				gi.cvar_forceset("sv_coop_gamemode_vote", "xatrix");
				gi.cvar_forceset("nextserver", "map \"xswamp\"\n");
				break;
			case xatrix_coop:
				gi.cvar_forceset("sv_coop_gamemode", "rogue");
				gi.cvar_forceset("sv_coop_gamemode_vote", "rogue");
				gi.cvar_forceset("nextserver", "map \"rmine1\"\n");
				break;
			case rogue_coop:
			case zaero_coop:
				gi.cvar_forceset("sv_coop_gamemode", "vanilla");
				gi.cvar_forceset("sv_coop_gamemode_vote", "vanilla");
				gi.cvar_forceset("nextserver", "map \"base1\"\n");
				break;
		}
	}
}
