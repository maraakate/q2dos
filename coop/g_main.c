#ifndef GAME_HARD_LINKED /* FS: Coop: For Sys_Mkdir */
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h> /* mkdir() */
#endif
#endif

#include "g_local.h"

game_locals_t game;
level_locals_t level;
game_import_t gi;
game_export_t globals;
spawn_temp_t st;

int sm_meat_index;
int snd_fry;
int meansOfDeath;

edict_t *g_edicts;

cvar_t *deathmatch;
cvar_t *coop;
cvar_t *coop_item_respawn; /* FS: Coop: Added */
cvar_t *coop_checkpoints; /* FS: Coop: Added */
cvar_t *sv_coop_gamemode; /* FS: Coop: Added */
cvar_t *sv_coop_gamemode_vote; /* FS: Coop: Added */
cvar_t *sv_vote_enabled; /* FS: Coop: Voting */
cvar_t *sv_vote_disallow_flags; /* FS: Coop: Voting */
cvar_t *sv_vote_assume_yes; /* FS: Coop: Voting */
cvar_t *sv_vote_timer; /* FS: Coop: Voting */
cvar_t *sv_vote_private; /* FS: Coop: Voting */
cvar_t *sv_vote_chat_commands; /* FS: Coop: Voting */
cvar_t *sv_coop_reset_hack; /* FS: Coop: Long uptime reset hack :( */
cvar_t *sv_coop_check_player_exit; /* FS: Added */
cvar_t *sv_coop_maplist; /* FS: Added */
cvar_t *sv_coop_summon_time; /* FS: Added */
cvar_t *sv_coop_announce_name_change; /* FS: Added */
cvar_t *sv_coop_name_timeout; /* FS: Added */
cvar_t *sv_coop_blinky_cam_disallowflags; /* FS: Added */
cvar_t *sv_drop_timeout; /* FS: Added */
cvar_t *sv_spawn_protection; /* FS: Coop: Spawn protection */
cvar_t *sv_spawn_protection_time; /* FS: Coop: Spawn protection */
cvar_t *sv_allow_hook; /* FS: Coop: Added */
cvar_t *motd; /* FS: Coop: Added */
cvar_t *adminpass; /* FS: Coop: Admin goodies */
cvar_t *vippass; /* FS: Coop: VIP goodies */
cvar_t *gamedir; /* FS: Coop: Added */
cvar_t *nextserver; /* FS: Coop: Added */
cvar_t *coop_cameraoffset; /* FS: Blinyk's Coop Camera */
cvar_t *dmflags;
cvar_t *zdmflags; /* FS: Zaero specific game dll changes */
cvar_t *skill;
cvar_t *fraglimit;
cvar_t *timelimit;
cvar_t *password;
cvar_t *spectator_password;
cvar_t *needpass;
cvar_t *maxclients;
cvar_t *maxspectators;
cvar_t *maxentities;
cvar_t *g_select_empty;
cvar_t *dedicated;
cvar_t *flashlightmode;
cvar_t *sv_filter_wallfly_ip;

cvar_t *filterban;

cvar_t *sv_maxvelocity;
cvar_t *sv_gravity;

cvar_t *sv_rollspeed;
cvar_t *sv_rollangle;
cvar_t *gun_x;
cvar_t *gun_y;
cvar_t *gun_z;

cvar_t *run_pitch;
cvar_t *run_roll;
cvar_t *bob_up;
cvar_t *bob_pitch;
cvar_t *bob_roll;

cvar_t *sv_cheats;

cvar_t *flood_msgs;
cvar_t *flood_persecond;
cvar_t *flood_waitdelay;

cvar_t *sv_maplist;
cvar_t *sv_stopspeed; /* FS: Coop: Rogue specific */

cvar_t *gamerules; /* FS: Coop: Rogue specific */
cvar_t *huntercam; /* FS: Coop: Rogue specific */
cvar_t *strong_mines; /* FS: Coop: Rogue specific */
cvar_t *randomrespawn; /* FS: Coop: Rogue specific */

void SpawnEntities(char *mapname, char *entities, char *spawnpoint);
void ClientThink(edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect(edict_t *ent, char *userinfo);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);
void ClientDisconnect(edict_t *ent);
void ClientBegin(edict_t *ent);
void ClientCommand(edict_t *ent);
void RunEntity(edict_t *ent);
void WriteGame(char *filename, qboolean autosave);
void ReadGame(char *filename);
void WriteLevel(char *filename);
void ReadLevel(char *filename);
void InitGame(void);
void G_RunFrame(void);
void G_ResetTimer_Hack (void); /* FS: Some of the grossest shit of all time.  Reset everything to avoid the integer to float overflow timer bullshit */

//#define FRAMENUM_TIMER_TEST
#ifdef FRAMENUM_TIMER_TEST /* FS: Force framenum overflow and reset testing */
#ifdef _DEBUG
cvar_t *sv_force_framenum_hack;
static qboolean firstStart = true;
#endif // _DEBUG
#endif // FRAMENUM_TIMER_TEST

/* =================================================================== */

void
ShutdownGame(void)
{
	gi.dprintf(DEVELOPER_MSG_GAME, "==== ShutdownGame ====\n");

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);
}


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

#ifdef FRAMENUM_TIMER_TEST /* FS: Force framenum overflow and reset testing */
#ifdef _DEBUG
	sv_force_framenum_hack = gi.cvar("sv_force_framenum_hack", "0", 0);
	sv_force_framenum_hack->modified = false;
#endif // _DEBUG
#endif // FRAMENUM_TIMER_TEST

	return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);
	text[sizeof(text)-1] = 0;

	gi.error ("%s", text);
}

void Com_Printf (const char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);
	text[sizeof(text)-1] = 0;

	gi.dprintf(DEVELOPER_MSG_GAME, "%s", text);
}

void Sys_Mkdir (char *path) /* FS: Coop: FIXME -- This could be added to something like gi.FS_CreatePath instead or similiar, but that would be non-standard to other port authors.  However, it's trivial to add.  So what-do? */
{
#ifdef _WIN32
	_mkdir (path);
#else
	mkdir (path, 0777);
#endif
}

#endif

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<maxclients->intValue; i++)
	{
		ent = g_edicts + 1 + i;

		if (!ent->inuse || !ent->client)
		{
			continue;
		}

		ClientEndServerFrame (ent);
	}
}

/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	if (!map)
	{
		return NULL;
	}

	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	edict_t *ent;
	char *s, *t, *tPtr, *f;
	static const char *seps = " ,\n\r";

	/* stay on same level flag */
	if (dmflags->intValue & DF_SAME_LEVEL)
	{
		BeginIntermission(CreateTargetChangeLevel(level.mapname));
		return;
	}

	/* see if it's in the map list */
	if (*sv_maplist->string)
	{
		s = strdup(sv_maplist->string);
		f = NULL;
		t = strtok_r(s, seps, &tPtr);

		while (t != NULL)
		{
			if (Q_stricmp(t, level.mapname) == 0)
			{
				/* it's in the list, go to the next one */
				t = strtok_r(NULL, seps, &tPtr);

				if (t == NULL) /* end of list, go to first one */
				{
					if (f == NULL) /* there isn't a first one, same level */
					{
						BeginIntermission(CreateTargetChangeLevel(level.mapname));
					}
					else
					{
						BeginIntermission(CreateTargetChangeLevel(f));
					}
				}
				else
				{
					BeginIntermission(CreateTargetChangeLevel(t));
				}

				free(s);
				return;
			}

			if (!f)
			{
				f = t;
			}

			t = strtok_r(NULL, seps, &tPtr);
		}

		free(s);
	}

	if (level.nextmap[0]) /* go to a specific map */
	{
		BeginIntermission(CreateTargetChangeLevel(level.nextmap));
	}
	else /* search for a changelevel */
	{
		ent = G_Find(NULL, FOFS(classname), "target_changelevel");

		if (!ent)
		{
			/* the map designer didn't include a changelevel,
			   so create a fake ent that goes back to the same
			   level */
			BeginIntermission(CreateTargetChangeLevel(level.mapname));
			return;
		}

		BeginIntermission(ent);
	}
}


void CheckNeedPass (void)
{
	int need;

	/* if password or spectator_password has
	   changed, update needpass as needed */
	if (password->modified || spectator_password->modified)
	{
		password->modified = spectator_password->modified = false;

		need = 0;

		if (*password->string && Q_stricmp(password->string, "none"))
		{
			need |= 1;
		}

		if (*spectator_password->string &&
			Q_stricmp(spectator_password->string, "none"))
		{
			need |= 2;
		}

		gi.cvar_set("needpass", va("%d", need));
	}
}

void
CheckDMRules(void)
{
	int i;
	gclient_t *cl;

	if (level.intermissiontime)
	{
		return;
	}

	if (!deathmatch->intValue)
	{
		return;
	}

	if (gamerules && gamerules->intValue && DMGame.CheckDMRules) /* FS: Coop: Rogue specific */
	{
		if (DMGame.CheckDMRules())
		{
			return;
		}
	}

	if (timelimit->value)
	{
		if (level.time >= timelimit->value * 60)
		{
			gi.bprintf(PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel();
			return;
		}
	}

	if (fraglimit->intValue)
	{
		for (i = 0; i < maxclients->intValue; i++)
		{
			cl = game.clients + i;

			if (!g_edicts[i + 1].inuse)
			{
				continue;
			}

			if (cl->resp.score >= fraglimit->intValue)
			{
				gi.bprintf(PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel();
				return;
			}
		}
	}
}

void
ExitLevel(void)
{
	int i;
	edict_t *ent;
	char command[256];

	Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString(command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	level.fadeFrames = 0; /* FS: Zaero specific game dll changes */
	level.current_coop_checkpoint = NULL;

	vote_Reset(); /* FS: Coop: Voting */
	ClientEndServerFrames();

	/* clear some things before going to next level */
	for (i = 0; i < maxclients->intValue; i++)
	{
		ent = g_edicts + 1 + i;

		if (!ent->inuse)
		{
			continue;
		}

		if (ent->health > ent->client->pers.max_health)
		{
			ent->health = ent->client->pers.max_health;
		}
	}

	gibsthisframe = 0;
	lastgibframe = 0;
}

/*
 * Advances the world by 0.1 seconds
 */
void
G_RunFrame(void)
{
	int i;
	edict_t *ent;

	Blinky_BeginRunFrame(); /* FS: Blinky's Coop Camera */

#ifdef FRAMENUM_TIMER_TEST /* FS: Force framenum overflow and reset testing */
#ifdef _DEBUG
	if(firstStart && level.framenum > 15) /* FS: Need a few frames for physics to settle down and activate before you can fudge it forward */
	{
		level.framenum = 10790*10*8;
		firstStart = false;
//		gi.cprintf(NULL, PRINT_CHAT, "framenum: %i\n", level.framenum);
	}

	if(sv_force_framenum_hack->modified)
	{
		level.framenum = 10790*10*8;
		gi.cvar_forceset("sv_force_framenum_hack", "0");
		sv_force_framenum_hack->modified = false;
	}
#endif // _DEBUG
#endif // FRAMENUM_TIMER_TEST

	level.framenum++;
	level.time = level.framenum * FRAMETIME;

	G_ResetTimer_Hack(); /* FS: Some of the most grossest shit of all time.  Reset the counters after long uptimes */
//	gi.cprintf(NULL, PRINT_CHAT, "framenum: %i [%f]\n", level.framenum, level.time);

	/* choose a client for monsters to target this frame */
	AI_SetSightClient();

	/* exit intermissions */
	if ((game.gametype == zaero_coop) && (level.fadeFrames > 1)) /* FS: Zaero specific game dll changes */
	{
		level.fadeFrames--;
	}
	else if (level.exitintermission)
	{
		ExitLevel();
		return;
	}

	/* treat each object in turn  even the
	   world gets a chance to think */
	ent = &g_edicts[0];

	for (i = 0; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->inuse)
		{
			continue;
		}

		level.current_entity = ent;

		if (game.gametype == zaero_coop) /* FS: Zaero specific game dll changes */
		{
		    if(!(ent->flags & FL_DONTSETOLDORIGIN))
			{
				VectorCopy (ent->s.origin, ent->s.old_origin);
			}
		}
		else
		{
			VectorCopy(ent->s.origin, ent->s.old_origin);
		}

		/* if the ground entity moved, make sure we are still on it */
		if ((ent->groundentity) &&
			(ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;

			if (!(ent->flags & (FL_SWIM | FL_FLY)) &&
				(ent->svflags & SVF_MONSTER))
			{
				M_CheckGround(ent);
			}
		}

		if ((i > 0) && (i <= maxclients->intValue))
		{
			ClientBeginServerFrame(ent);
			continue;
		}

		G_RunEntity(ent);
	}

	/* see if it is time to end a deathmatch */
	CheckDMRules();

	/* see if needpass needs updated */
	CheckNeedPass();

	/* build the playerstate_t structures for all players */
	ClientEndServerFrames();

}

void G_ResetTimer_Hack (void) /* FS: Some of the grossest shit of all time.  Reset everything to avoid the integer to float overflow timer bullshit */
{
	int i;
	edict_t *ent;

	if(sv_coop_reset_hack->intValue && (level.time > 10800.0f) && !P_Clients_Connected(true)) /* FS: Every 3 hours reset the timers.  The game seems to start fucking up around 4 hours of idleness */
	{
		gi.cprintf(NULL, PRINT_HIGH, "Resetting timers...\n");

		level.framenum = 55; /* FS: Dont start at 0, advance it a bit */
		level.time = level.framenum * FRAMETIME;
		if (level.intermissiontime) /* Phatman: This fixes the issue when the intermission gets stuck after idling with no players */
			level.intermissiontime = level.time;
		lastgibframe = 0;

		ent = &g_edicts[0];

		for (i = 0; i < globals.num_edicts; i++, ent++)
		{
			qboolean setStand = false;
			qboolean setSearch = false;
			qboolean setIdle = false;
			char *className = "Unknown Entity";

			if (!ent->inuse)
			{
				if(ent->freetime)
				{
					gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set freetime for freed entity: %f\n", ent->freetime);
					ent->freetime = level.time;
				}
				continue;
			}

			if(ent->classname)
			{
				className = ent->classname;
			}

			if(ent->monsterinfo.search_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set search_time for %s: %f\n", className, ent->monsterinfo.search_time);
				ent->monsterinfo.search_time = level.time + 0.1f;
				setSearch = true;
			}

			if(ent->monsterinfo.trail_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set trail_time for %s: %f\n", className, ent->monsterinfo.trail_time);
				ent->monsterinfo.trail_time = level.time + 0.1f;
			}
			if(ent->monsterinfo.idle_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set idle_time for %s: %f\n", className, ent->monsterinfo.idle_time);
				ent->monsterinfo.idle_time = level.time + 0.1f;
				setIdle = true;
			}

			if(ent->monsterinfo.pausetime)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set pausetime for %s: %f\n", className, ent->monsterinfo.pausetime);
				ent->monsterinfo.pausetime = level.time + 100000000;
				setStand = true;
			}

			if(ent->monsterinfo.last_hint_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set last_hint_time for %s: %f\n", className, ent->monsterinfo.last_hint_time);
				ent->monsterinfo.last_hint_time = level.time + 0.1f;
			}

			if(ent->monsterinfo.next_duck_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set next_duck_time for %s: %f\n", className, ent->monsterinfo.next_duck_time);
				ent->monsterinfo.next_duck_time = level.time + 0.1f;
			}

			if(ent->monsterinfo.duck_wait_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set duck_wait_time for %s: %f\n", className, ent->monsterinfo.duck_wait_time);
				ent->monsterinfo.duck_wait_time = level.time + 0.1f;
			}

			if(ent->monsterinfo.blind_fire_delay)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set blind_fire_delay for %s: %f\n", className, ent->monsterinfo.blind_fire_delay);
				ent->monsterinfo.blind_fire_delay = level.time + 0.1f;
			}

			if(ent->monsterinfo.quad_framenum)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set quad_framenum for %s: %f\n", className, ent->monsterinfo.quad_framenum);
				ent->monsterinfo.quad_framenum = 0.0f;
			}

			if(ent->monsterinfo.invincible_framenum)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set invincible_framenum for %s: %f\n", className, ent->monsterinfo.invincible_framenum);
				ent->monsterinfo.invincible_framenum = 0.0f;
			}

			if(ent->monsterinfo.double_framenum)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set double_framenum for %s: %f\n", className, ent->monsterinfo.double_framenum);
				ent->monsterinfo.double_framenum = 0.0f;
			}

			if(ent->air_finished)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set air_finished for %s: %f\n", className, ent->air_finished);
				ent->air_finished = level.time + 1.0f;
			}

			if(ent->powerarmor_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set powerarmor_time for %s: %f\n", className, ent->powerarmor_time);
				ent->powerarmor_time = level.time + 0.2f;
			}

			/* FS: TODO: Check ent->wait and ent->delay and re-apply per situation since a few rely on level.time */

			if(ent->last_sound_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set last_sound_time for %s: %f\n", className, ent->last_sound_time);
				ent->last_sound_time = level.time;
			}

			if(ent->touch_debounce_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set touch_debounce_time for %s: %f\n", className, ent->touch_debounce_time);
				ent->touch_debounce_time = level.time + 0.1f;
			}

			if(ent->pain_debounce_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set pain_debounce_time for %s: %f\n", className, ent->pain_debounce_time);
				ent->pain_debounce_time = level.time + 0.1f;
			}

			if(ent->damage_debounce_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set damage_debounce_time for %s: %f\n", className, ent->damage_debounce_time);
				ent->damage_debounce_time = level.time + 0.1f;
			}

			if(ent->fly_sound_debounce_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set fly_sound_debounce_time for %s: %f\n", className, ent->fly_sound_debounce_time);
				ent->fly_sound_debounce_time = level.time + 0.1f;
			}

			if(ent->last_move_time)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set last_move_time for %s: %f\n", className, ent->last_move_time);
				ent->last_move_time = 0.1f;
			}

			if(ent->timestamp)
			{
				gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set timestamp for %s: %f\n", className, ent->timestamp);
				ent->timestamp = level.time + 0.1f;
			}

			if(ent->think)
			{
				if(ent->nextthink)
				{
					gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set nextthink for %s: %f\n", className, ent->nextthink);
					ent->nextthink = level.time + 0.1f;
				}
			}

			if(setStand)
			{
				if(ent->monsterinfo.stand)
				{
					gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set Stand AI for %s\n", className);
					ent->monsterinfo.stand(ent);
				}
			}
			else if(setSearch)
			{
				if(ent->monsterinfo.search)
				{
					gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set Search AI for %s\n", className);
					ent->monsterinfo.search(ent);
				}
			}
			else if(setIdle)
			{
				if(ent->monsterinfo.idle)
				{
					gi.dprintf(DEVELOPER_MSG_VERBOSE, "Set Idle AI for %s\n", className);
					ent->monsterinfo.idle(ent);
				}
			}
			else /* FS: Not sure what we're supposed to be doing with ourselves, so just stand there for now */
			{
				if(ent->monsterinfo.stand)
				{
					gi.dprintf(DEVELOPER_MSG_VERBOSE, "WARNING: Unknown what monster is doing.  Seting stand AI by default for %s\n", className);
					ent->monsterinfo.stand(ent);
				}
			}
		}
	}
}
