/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * Coop specific code.
 *
 * =======================================================================
 */

#include "g_local.h"

#define COOP_VERSION			0.02
#define COOP_VSTRING2(x) #x
#define COOP_VSTRING(x) COOP_VSTRING2(x)
#define COOP_STRING_VERSION  COOP_VSTRING(COOP_VERSION)

void CoopOpenJoinMenu(edict_t *ent);
void CoopOpenVoteMenu(edict_t *ent);
void CoopReturnToMain(edict_t *ent, pmenuhnd_t *p);
void CoopReturnToVoteMenu(edict_t *ent, pmenuhnd_t *p);
void CoopCredits(edict_t *ent, pmenuhnd_t *p);
void CoopChaseCam(edict_t *ent, pmenuhnd_t *p);
void CoopJoinGame(edict_t *ent, pmenuhnd_t *p);
void CoopBlinkyCam(edict_t *ent, pmenuhnd_t *p);
void CoopMotd(edict_t *ent, pmenuhnd_t *p);

void CoopVoteMenu(edict_t *ent, pmenuhnd_t *p);
void CoopVoteDifficulty(edict_t *ent, pmenuhnd_t *p);
void CoopVoteMap(edict_t *ent, pmenuhnd_t *p);
void CoopVoteRestartMap(edict_t *ent, pmenuhnd_t *p);
void CoopVotePlayerReq(edict_t *ent, pmenuhnd_t *p);
void CoopVotePlayerKick(edict_t *ent, pmenuhnd_t *p);
void CoopVotePlayerBan(edict_t *ent, pmenuhnd_t *p);
void CoopVotePlayerSilence(edict_t *silence, pmenuhnd_t *p);
void CoopVoteHookMenu(edict_t *ent, pmenuhnd_t *p);
void CoopUpdateVoteHookMenu(edict_t *ent);
void CoopVoteToggleHook(edict_t *ent, pmenuhnd_t *p);
void CoopCheckGamemode(edict_t *ent, pmenuhnd_t *p);
void CoopCheckDifficulty(edict_t *ent, pmenuhnd_t *p);
void votemenu_loadmaplist (void);
void CoopVoteChangeMap(edict_t *ent, pmenuhnd_t *p);
void votemenu_cleanup_all (void);
void votemenu_cleanup_filebuffer (void);
void CoopUpdateBlinkyMenu(edict_t *ent);
void CoopBlinkyChaseMenu(edict_t *ent, pmenuhnd_t *p);
void CoopBlinkySummonMenu(edict_t *ent, pmenuhnd_t *p);
void CoopBlinkyTeleportMenu(edict_t *ent, pmenuhnd_t *p);
void CoopBlinkyToggleSummon(edict_t *ent, pmenuhnd_t *p);

void CoopGamemodeInit (void);
int CoopGamemodeExists (const char *gamemode);
void CoopGamemodeAdd (const char *gamemode, const char *realgamemode, const char *mapname);
void CoopVoteGamemodeDynamic(edict_t *ent, pmenuhnd_t *p /* unused */);

extern void VoteMenuOpen(edict_t *ent);
/*-----------------------------------------------------------------------*/

pmenu_t creditsmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Programming", PMENU_ALIGN_CENTER, NULL},
	{"'[HCI]Mara'akate'", PMENU_ALIGN_LEFT, NULL},
	{"'Freewill'", PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Additional Code", PMENU_ALIGN_CENTER, NULL},
	{"Blinky Coop Cam", PMENU_ALIGN_LEFT, NULL},
	{"Knightmare Quake2", PMENU_ALIGN_LEFT, NULL},
	{"Yamagi Quake2", PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

static const int jmenu_level = 4;
static const int jmenu_gamemode = 5;
static const int jmenu_skill = 7;
static const int jmenu_players = 8;
static const int jmenu_spectators = 9;
static const int jmenu_blinky = 13;
static const int jmenu_vote = 14;
static const int jmenu_motd = 15;

pmenu_t joinmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL}, /* 10 */
	{"Join The Game", PMENU_ALIGN_LEFT, CoopJoinGame},
	{"Become a Spectator", PMENU_ALIGN_LEFT, CoopChaseCam},
	{"Blinky Camera", PMENU_ALIGN_LEFT, CoopBlinkyCam},
	{"Voting Menu", PMENU_ALIGN_LEFT, CoopVoteMenu},
	{"Message of The Day", PMENU_ALIGN_LEFT, CoopMotd},
	{"Credits", PMENU_ALIGN_LEFT, CoopCredits}, /* 16 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{"v" COOP_STRING_VERSION, PMENU_ALIGN_RIGHT, NULL},
};

pmenu_t nochasemenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"No one to chase", PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

pmenu_t votemenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Gamemode", PMENU_ALIGN_LEFT, CoopVoteGamemodeDynamic},
	{"Difficulty", PMENU_ALIGN_LEFT, CoopVoteDifficulty},
	{"Change Map", PMENU_ALIGN_LEFT, CoopVoteMap},
	{"Restart Map", PMENU_ALIGN_LEFT, CoopVoteRestartMap},
	{"Exit Requirements", PMENU_ALIGN_LEFT, CoopVotePlayerReq},
	{"Kick Player", PMENU_ALIGN_LEFT, CoopVotePlayerKick},
	{"Ban Player", PMENU_ALIGN_LEFT, CoopVotePlayerBan},
	{"Silence Player", PMENU_ALIGN_LEFT, CoopVotePlayerSilence},
	{"Grappling Hook", PMENU_ALIGN_LEFT, CoopVoteHookMenu},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define VGAMEMODEMENU_GAMEMODE 6
#define VGAMEMODEMENU_VANILLA 0
#define VGAMEMODEMENU_XATRIX 1
#define VGAMEMODEMENU_ROGUE 2
#define VGAMEMODEMENU_ZAERO 3

pmenu_t votegamemodemenuheader[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Gamemode", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Current Gamemode: ", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
};

pmenu_t *votegamemodemenu = NULL;

#define VSKILLMENU_SKILL 6
#define VSKILLMENU_EASY 8
#define VSKILLMENU_MEDIUM 9
#define VSKILLMENU_HARD 10
#define VSKILLMENU_NIGHTMARE 11

pmenu_t voteskillmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Difficulty", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Current Skill: ", PMENU_ALIGN_CENTER, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Easy", PMENU_ALIGN_LEFT, CoopCheckDifficulty}, /* 8 */
	{"Medium", PMENU_ALIGN_LEFT, CoopCheckDifficulty},
	{"Hard", PMENU_ALIGN_LEFT, CoopCheckDifficulty},
	{"Nightmare", PMENU_ALIGN_LEFT, CoopCheckDifficulty}, /* 11 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Voting Menu", PMENU_ALIGN_LEFT, CoopReturnToVoteMenu}
};

#define MOTDMENU_START 4
#define MOTDMENU_END 16
#define MOTDMENU_MAXLINES ((MOTDMENU_END)-(MOTDMENU_START))
pmenu_t motdmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define BLINKYMENU_SUMMON 10
pmenu_t blinkymenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Blinky Camera", PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Chase Player", PMENU_ALIGN_LEFT, CoopBlinkyChaseMenu}, /* 6 */
	{"Summon Player", PMENU_ALIGN_LEFT, CoopBlinkySummonMenu},
	{"Teleport Player", PMENU_ALIGN_LEFT, CoopBlinkyTeleportMenu},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{"Summon: ", PMENU_ALIGN_LEFT, CoopBlinkyToggleSummon}, /* 10 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define SUMMONMENU_START 6
pmenu_t summonmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Summon Player", PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define TELEPORTMENU_START 6
pmenu_t teleportmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Teleport to Player", PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define CHASEMENU_START 6
pmenu_t chasemenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Chase Cam Player", PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define PLAYERKICKMENU_START 6
pmenu_t playerkickmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Kick Player", PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define PLAYERBANMENU_START 6
pmenu_t playerbanmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Ban Player", PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define PLAYERSILENCEMENU_START 6
pmenu_t playersilencemenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Silence Player", PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

#define HOOKMENU_STATUS 6
#define HOOKMENU_VOTEOPTION 8
pmenu_t hookmenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Grappling Hook", PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Status: LOADING...", PMENU_ALIGN_LEFT, NULL}, /* 6 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{"PLEASE WAIT...", PMENU_ALIGN_LEFT, CoopVoteToggleHook}, /* 8 */
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 16 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Return to Main Menu", PMENU_ALIGN_LEFT, CoopReturnToMain}
};

pmenu_t votemapheadermenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Maps", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
};

char *coopMapFileBuffer = NULL;
int mapCount = 0;
int gamemodeCount = 0;
pmenu_t *votemapmenu = NULL;

gamemode_t gamemode_array[MAX_GAMEMODES];

char *GetSkillString (void)
{
	switch(skill->intValue)
	{
	case 0:
		return "Easy";
	case 1:
		return "Medium";
	case 2:
		return "Hard";
	case 3:
		return "Nightmare";
	}

	return "Unknown";
}

void
CoopReturnToMain(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	CoopOpenJoinMenu(ent);
}

void
CoopReturnToVoteMenu(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	CoopOpenVoteMenu(ent);
}

static void
SetLevelName(pmenu_t *p)
{
	static char levelname[33];

	if (!p)
	{
		return;
	}

	levelname[0] = '*';

	if (g_edicts[0].message)
	{
		strncpy(levelname + 1, g_edicts[0].message, sizeof(levelname) - 2);
	}
	else
	{
		strncpy(levelname + 1, level.mapname, sizeof(levelname) - 2);
	}

	levelname[sizeof(levelname) - 1] = 0;
	p->text = levelname;
}

void CoopUpdateJoinMenu(edict_t *ent)
{
	static char players[32];
	static char spectators[32];
	int numplayers, numspectators, i;
	pmenuhnd_t *p;

	if(!ent || !ent->client)
	{
		return;
	}

	p = ent->client->menu;
	if(!p)
	{
		return;
	}

	numplayers = numspectators = 0;

	for (i = 0; i < maxclients->intValue; i++)
	{
		if (!g_edicts[i + 1].inuse)
		{
			continue;
		}

		if (game.clients[i].resp.spectator)
		{
			numspectators++;
		}
		else
		{
			numplayers++;
		}
	}

	Com_sprintf(players, sizeof(players), "  (%d players)", numplayers);
	Com_sprintf(spectators, sizeof(spectators), "  (%d spectators)", numspectators);

	PMenu_UpdateEntry(p->entries + jmenu_players, players, PMENU_ALIGN_CENTER, NULL);
	PMenu_UpdateEntry(p->entries + jmenu_spectators, spectators, PMENU_ALIGN_CENTER, NULL);
}

void
CoopInitJoinMenu(edict_t *ent)
{
	static char players[32];
	static char spectators[32];
	static char stats[32];
	static char gamemode[32];
	int numplayers, numspectators, i;

	if(!ent || !ent->client)
	{
		return;
	}

	SetLevelName(joinmenu + jmenu_level);

	numplayers = numspectators = 0;

	for (i = 0; i < maxclients->intValue; i++)
	{
		if (!g_edicts[i + 1].inuse)
		{
			continue;
		}

		if (game.clients[i].resp.spectator)
		{
			numspectators++;
		}
		else
		{
			numplayers++;
		}
	}

	Com_sprintf(gamemode, sizeof(gamemode), "*Gamemode: %s", sv_coop_gamemode->string);
	Com_sprintf(players, sizeof(players), "  (%d players)", numplayers);
	Com_sprintf(spectators, sizeof(spectators), "  (%d spectators)", numspectators);
	Com_sprintf(stats, sizeof(stats), "Difficulty: %s", GetSkillString());

	joinmenu[jmenu_gamemode].text = gamemode;
	joinmenu[jmenu_players].text = players;
	joinmenu[jmenu_spectators].text = spectators;
	joinmenu[jmenu_skill].text = stats;

	if(!motd || !motd->string[0])
	{
		joinmenu[jmenu_motd].text = NULL;
		joinmenu[jmenu_motd].SelectFunc = NULL;
	}
}

void
CoopOpenJoinMenu(edict_t *ent)
{
	extern qboolean bVoteInProgress;

	if(!ent || !ent->client)
	{
		return;
	}

	ent->client->pers.didMotd = ent->client->resp.didMotd = true;

	if(bVoteInProgress) /* FS: If a vote is happening don't show the other fun options... */
	{
		VoteMenuOpen(ent);
		return;
	}

	CoopInitJoinMenu(ent);
	PMenu_Open(ent, joinmenu, NULL, 0, sizeof(joinmenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
	ent->client->menu_update = CoopUpdateJoinMenu;
}

void
CoopOpenVoteMenu(edict_t *ent)
{
	if(!ent || !ent->client)
	{
		return;
	}

	ent->client->pers.didMotd = ent->client->resp.didMotd = true;
	PMenu_Open(ent, votemenu, NULL, 0, sizeof(votemenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
}

void
CoopCredits(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, NULL, -1,
			sizeof(creditsmenu) / sizeof(pmenu_t), 0,
			NULL, PMENU_NORMAL);
}

void CoopChaseCam(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	gi.WriteByte (svc_stufftext);
	gi.WriteString ("spectator 1\n");
	gi.unicast(ent, true);
}

void CoopJoinGame(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	gi.WriteByte (svc_stufftext);
	gi.WriteString ("spectator 0\n");
	gi.unicast(ent, true);
}

void CoopUpdateMotdMenu (void)
{
	char separators[] = "|\n";
	char *motdString;
	char *listPtr = NULL;
	char *motdToken = NULL;
	int lineLength = 0;
	int currentLine = 0;
	int i = MOTDMENU_START;

	if(!motd || !motd->string || !motd->string[0]) /* FS: TODO: Read this from a file instead of a CVAR because CVAR seems to limit at 256 chars.  Max we could possibly squeeze is about 324 chars. */
	{
		return;
	}

	motdString = strdup(motd->string);

	motdToken = strtok_r(motdString, separators, &listPtr);
	if(!motdToken)
	{
		return;
	}

	for (i = MOTDMENU_START; i < MOTDMENU_MAXLINES; i++) /* FS: Clear it out first in case if we change it later */
	{
		motdmenu[i].text = NULL;
	}

	i = MOTDMENU_START;

	while(motdToken)
	{
		if(i >= MOTDMENU_MAXLINES)
		{
			break;
		}

		currentLine++;
		lineLength = strlen(motdToken);
		motdmenu[i].text = strdup(motdToken);
		if(lineLength > 27)
		{
			gi.cprintf(NULL, PRINT_CHAT, "Warning: MOTD string on line %i greater than 27 chars, current length is %i.  Truncating.\n", currentLine, lineLength);
			motdmenu[i].text[27] = 0;
		}
		motdToken = strtok_r(NULL, separators, &listPtr);
		i++;
	}

	if(motdString)
	{
		free(motdString);
	}
}

void CoopMotd(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	int i;

	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	CoopUpdateMotdMenu();
	PMenu_Open(ent, motdmenu, NULL, 0, sizeof(motdmenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);

	for (i = MOTDMENU_START; i < MOTDMENU_MAXLINES; i++) /* FS: Clear it out first in case if we change it later */
	{
		if(motdmenu[i].text)
		{
			free(motdmenu[i].text);
		}
		motdmenu[i].text = NULL;
	}
}

void CoopVoteMenu(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	PMenu_Open(ent, votemenu, NULL, 0, sizeof(votemenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
}

void CoopUpdateGamemodeMenu(edict_t *ent)
{
	static char gamemodestring[32];
	static char gamemode[32];
	int i;

	if(!ent || !ent->client)
	{
		return;
	}

	switch(game.gametype)
	{
		case vanilla_coop:
			if(!Q_stricmp(sv_coop_gamemode->string, "vanilla")) /* FS: If in a "custom" mode show Vanilla as an option */
			{
				Com_sprintf(gamemode, sizeof(gamemode), "Vanilla");
				votegamemodemenu[VGAMEMODEMENU_VANILLA].text = NULL;
				votegamemodemenu[VGAMEMODEMENU_VANILLA].SelectFunc = NULL;
			}
			else
			{
				Com_sprintf(gamemode, sizeof(gamemode), "%s", sv_coop_gamemode->string);
				i = CoopGamemodeExists(gamemode);
				votegamemodemenu[VGAMEMODEMENU_VANILLA+i].text = NULL;
				votegamemodemenu[VGAMEMODEMENU_VANILLA+i].SelectFunc = NULL;
			}
			break;
		case xatrix_coop:
			if(!Q_stricmp(sv_coop_gamemode->string, "xatrix")) /* FS: If in a "custom" mode show Xatrix as an option */
			{
				Com_sprintf(gamemode, sizeof(gamemode), "Xatrix");
				votegamemodemenu[VGAMEMODEMENU_XATRIX].text = NULL;
				votegamemodemenu[VGAMEMODEMENU_XATRIX].SelectFunc = NULL;
			}
			else
			{
				Com_sprintf(gamemode, sizeof(gamemode), "%s", sv_coop_gamemode->string);
				i = CoopGamemodeExists(gamemode);
				votegamemodemenu[VGAMEMODEMENU_VANILLA+i].text = NULL;
				votegamemodemenu[VGAMEMODEMENU_VANILLA+i].SelectFunc = NULL;
			}
			break;
		case rogue_coop:
			if(!Q_stricmp(sv_coop_gamemode->string, "rogue")) /* FS: If in a "custom" mode show Rogue as an option */
			{
				Com_sprintf(gamemode, sizeof(gamemode), "Rogue");
				votegamemodemenu[VGAMEMODEMENU_ROGUE].text = NULL;
				votegamemodemenu[VGAMEMODEMENU_ROGUE].SelectFunc = NULL;
			}
			else
			{
				Com_sprintf(gamemode, sizeof(gamemode), "%s", sv_coop_gamemode->string);
				i = CoopGamemodeExists(gamemode);
				votegamemodemenu[VGAMEMODEMENU_VANILLA+i].text = NULL;
				votegamemodemenu[VGAMEMODEMENU_VANILLA+i].SelectFunc = NULL;
			}
			break;
		case zaero_coop:
			if(!Q_stricmp(sv_coop_gamemode->string, "zaero")) /* FS: If in a "custom" mode show Zaero as an option */
			{
				Com_sprintf(gamemode, sizeof(gamemode), "Zaero");
				votegamemodemenu[VGAMEMODEMENU_ZAERO].text = NULL;
				votegamemodemenu[VGAMEMODEMENU_ZAERO].SelectFunc = NULL;
			}
			else
			{
				Com_sprintf(gamemode, sizeof(gamemode), "%s", sv_coop_gamemode->string);
				i = CoopGamemodeExists(gamemode);
				votegamemodemenu[VGAMEMODEMENU_VANILLA+i].text = NULL;
				votegamemodemenu[VGAMEMODEMENU_VANILLA+i].SelectFunc = NULL;
			}
			break;
		default:
			Com_sprintf(gamemode, sizeof(gamemode), "Custom");
			break;
	}

	Com_sprintf(gamemodestring, sizeof(gamemodestring), "*Current Gamemode: %s", gamemode);

	votegamemodemenuheader[VGAMEMODEMENU_GAMEMODE].text = gamemodestring;
	votegamemodemenuheader[VGAMEMODEMENU_GAMEMODE].align = PMENU_ALIGN_CENTER;
	votegamemodemenuheader[VGAMEMODEMENU_GAMEMODE].SelectFunc = NULL;
}

void CoopUpdateDifficultyMenu(edict_t *ent)
{
	static char skillstring[32];
	static char skillmodifier[32];

	if(!ent || !ent->client)
	{
		return;
	}

	switch(skill->intValue)
	{
		case 0:
			voteskillmenu[VSKILLMENU_EASY].text = NULL;
			voteskillmenu[VSKILLMENU_EASY].SelectFunc = NULL;
			break;
		case 1:
			voteskillmenu[VSKILLMENU_MEDIUM].text = NULL;
			voteskillmenu[VSKILLMENU_MEDIUM].SelectFunc = NULL;
			break;
		case 2:
			voteskillmenu[VSKILLMENU_HARD].text = NULL;
			voteskillmenu[VSKILLMENU_HARD].SelectFunc = NULL;
			break;
		case 3:
			voteskillmenu[VSKILLMENU_NIGHTMARE].text = NULL;
			voteskillmenu[VSKILLMENU_NIGHTMARE].SelectFunc = NULL;
			break;
		default:
			break;
	}

	Com_sprintf(skillmodifier, sizeof(skillmodifier), "%s", GetSkillString());
	Com_sprintf(skillstring, sizeof(skillstring), "*Current Skill: %s", skillmodifier);

	voteskillmenu[VSKILLMENU_SKILL].text = skillstring;
}

void CoopVoteDifficulty(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	CoopUpdateDifficultyMenu(ent);
	PMenu_Open(ent, voteskillmenu, NULL, 0, sizeof(voteskillmenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
}

void CoopUpdateVoteMapMenu(edict_t *ent)
{
	int i = 0;
	char *mapToken = NULL;
	char *coopMapTokenBuffer = NULL;
	char *listPtr = NULL;
	char separators[] = "\n";

	if(!ent || !ent->client || !coopMapFileBuffer)
	{
		return;
	}

	coopMapTokenBuffer = strdup(coopMapFileBuffer);

	mapToken = strtok_r(coopMapTokenBuffer, separators, &listPtr);
	if(!mapToken)
	{
		if(coopMapTokenBuffer)
		{
			free(coopMapTokenBuffer);
		}

		return;
	}

	while(mapToken)
	{
		votemapmenu[i].text = strdup(mapToken);
		votemapmenu[i].align = PMENU_ALIGN_LEFT;
		votemapmenu[i].SelectFunc = CoopVoteChangeMap;
		mapToken = strtok_r(NULL, separators, &listPtr);
		i++;
	}

	if(coopMapTokenBuffer)
	{
		free(coopMapTokenBuffer);
	}
}

void CoopVoteMap(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	size_t size;

	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	votemenu_loadmaplist();
	size = sizeof(pmenu_t) * (mapCount + 2);
	votemapmenu = malloc(size);
	if (!votemapmenu)
	{
		gi.error("CoopVoteMap:  Failed allocating memory.\n");
		return;
	}
	memset((pmenu_t *)votemapmenu, 0, size);
	CoopUpdateVoteMapMenu(ent);

	votemapmenu[mapCount].text = NULL;
	votemapmenu[mapCount].align = PMENU_ALIGN_LEFT;
	votemapmenu[mapCount].SelectFunc = NULL;

	votemapmenu[mapCount+1].text = "Return to Voting Menu";
	votemapmenu[mapCount+1].align = PMENU_ALIGN_LEFT;
	votemapmenu[mapCount+1].SelectFunc = CoopReturnToVoteMenu;

	PMenu_Open(ent, votemapmenu, votemapheadermenu, 0, mapCount + 2, sizeof(votemapheadermenu) / sizeof(pmenu_t), NULL, PMENU_SCROLLING);
	votemenu_cleanup_all();
}

void CoopVoteRestartMap(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	gi.WriteByte (svc_stufftext);
	gi.WriteString ("cmd vote restartmap\n");
	gi.unicast(ent, true);
}

void CoopVotePlayerReq(edict_t *ent, pmenuhnd_t *p)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	gi.WriteByte (svc_stufftext);
	gi.WriteString ("cmd vote playerexit\n");
	gi.unicast(ent, true);
}

void CoopVoteChangeMap(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	char mapVote[64];
	char votestring[64];

	if(!ent || !ent->client || !p)
	{
		return;
	}

	Com_sprintf(mapVote, sizeof(mapVote), "%s", p->entries[p->cur].text);
	COM_StripExtension (mapVote, mapVote);
	Com_sprintf(votestring, sizeof(votestring), "cmd vote map %s\n", mapVote);

	PMenu_Close(ent);

	gi.WriteByte (svc_stufftext);
	gi.WriteString (votestring);
	gi.unicast(ent, true);
}

void CoopCheckGamemode(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	char votestring[64];

	if(!ent || !ent->client)
	{
		return;
	}

	switch(p->cur)
	{
		case VGAMEMODEMENU_VANILLA:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote gamemode vanilla\n");
			break;
		case VGAMEMODEMENU_XATRIX:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote gamemode xatrix\n");
			break;
		case VGAMEMODEMENU_ROGUE:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote gamemode rogue\n");
			break;
		case VGAMEMODEMENU_ZAERO:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote gamemode zaero\n");
			break;
		default:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote gamemode %s\n", gamemode_array[p->cur-VGAMEMODEMENU_VANILLA].gamemode);
			break;
	}
	PMenu_Close(ent);

	if(votestring[0])
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(votestring);
		gi.unicast(ent, true);
	}
}

void CoopCheckDifficulty(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	char votestring[64] = { 0 };

	if(!ent || !ent->client)
	{
		return;
	}

	switch(p->cur)
	{
		case VSKILLMENU_EASY:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote skill 0\n");
			break;
		case VSKILLMENU_MEDIUM:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote skill 1\n");
			break;
		case VSKILLMENU_HARD:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote skill 2\n");
			break;
		case VSKILLMENU_NIGHTMARE:
			Com_sprintf(votestring, sizeof(votestring), "cmd vote skill 3\n");
			break;
		default:
			break;
	}
	PMenu_Close(ent);

	if(votestring[0])
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(votestring);
		gi.unicast(ent, true);
	}
}

void votemenu_cleanup_all (void)
{
	int i;

	if (votemapmenu)
	{
		for(i = 0; i < mapCount; i++)
		{
			if(votemapmenu[i].text)
			{
				free(votemapmenu[i].text);
			}
			votemapmenu[i].text = NULL;
		}

		free(votemapmenu);
		votemapmenu = NULL;
	}

	votemenu_cleanup_filebuffer();
}

void votemenu_cleanup_filebuffer (void)
{
	if(coopMapFileBuffer)
	{
		free(coopMapFileBuffer);
		coopMapFileBuffer = NULL;
	}
}

void votemenu_loadmaplist (void)
{
	char fileName[MAX_OSPATH];
	char *mapToken = NULL;
	char *coopMapTokenBuffer = NULL;
	char *listPtr = NULL;
	char separators[] = "\n";
	long fileSize;
	FILE *f = NULL;
	size_t toEOF = 0;

	if(sv_coop_maplist->string[0] == 0)
	{
		gi.cprintf(NULL, PRINT_CHAT, "votemenu_loadmaplist: sv_coop_maplist CVAR empty!\n");
		return;
	}

	Sys_Mkdir(va("%s/maps", gamedir->string));
	Com_sprintf(fileName, sizeof(fileName), "%s/%s", gamedir->string, sv_coop_maplist->string);

	f = fopen(fileName, "r");
	if(!f)
	{
		gi.cprintf(NULL, PRINT_CHAT, "votemenu_loadmaplist: couldn't find '%s'!\n", sv_coop_maplist->string);
		return;
	}

	/* obtain file size */
	fseek (f, 0, SEEK_END);
	fileSize = ftell (f);
	fseek (f, 0, SEEK_SET);

	votemenu_cleanup_filebuffer();

	coopMapFileBuffer = (char *)malloc(sizeof(char)*(fileSize+2)); /* FS: In case we have to add a newline terminator */
	if(!coopMapFileBuffer)
	{
		gi.cprintf(NULL, PRINT_CHAT, "votemenu_loadmaplist: can't allocate memory for coopMapFileBuffer!\n");
		fclose(f);
		return;
	}
	toEOF = fread(coopMapFileBuffer, sizeof(char), fileSize, f);
	fclose(f);
	if(toEOF <= 0)
	{
		gi.cprintf(NULL, PRINT_CHAT, "votemenu_loadmaplist: cannot read file '%s' into memory!\n", sv_coop_maplist->string);
		votemenu_cleanup_filebuffer();
		return;
	}

	/* FS: Add newline terminator for some paranoia */
	coopMapFileBuffer[toEOF] = '\n';
	coopMapFileBuffer[toEOF+1] = '\0';

	coopMapTokenBuffer = strdup(coopMapFileBuffer);

	mapToken = strtok_r(coopMapTokenBuffer, separators, &listPtr);
	if(!mapToken)
	{
		if(coopMapTokenBuffer)
		{
			free(coopMapTokenBuffer);
		}
		return;
	}

	mapCount = 0;
	while(mapToken)
	{
		mapCount++;
		mapToken = strtok_r(NULL, separators, &listPtr);
	}

	if(coopMapTokenBuffer)
	{
		free(coopMapTokenBuffer);
	}
}

void CoopInitBlinkyMenu(edict_t *ent)
{
	if(!ent || !ent->client)
	{
		return;
	}
}

void CoopBlinkyToggleSummon(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	static char summon[32];
	static char cmd[32];

	if(!ent || !ent->client)
	{
		return;
	}

	if(ent->client->pers.noSummon)
	{
		Com_sprintf(summon, sizeof(summon), "Summon: enabled");
		Com_sprintf(cmd, sizeof(cmd), "nosummon 0");
	}
	else
	{
		Com_sprintf(summon, sizeof(summon), "Summon: disabled");
		Com_sprintf(cmd, sizeof(cmd), "nosummon 1");
	}

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(ent, true);

	ent->client->menudirty = true;
}

void CoopUpdateBlinkyMenu(edict_t *ent)
{
	static char summon[32];
	pmenuhnd_t *p;

	if(!ent || !ent->client)
	{
		return;
	}

	p = ent->client->menu;
	if(!p)
	{
		return;
	}

	if(ent->client->pers.noSummon)
	{
		Com_sprintf(summon, sizeof(summon), "Summon: disabled");
	}
	else
	{
		Com_sprintf(summon, sizeof(summon), "Summon: enabled");
	}

	PMenu_UpdateEntry(p->entries + BLINKYMENU_SUMMON, summon, PMENU_ALIGN_LEFT, CoopBlinkyToggleSummon);
}

void CoopBlinkyCam(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	CoopInitBlinkyMenu(ent);
	PMenu_Open(ent, blinkymenu, NULL, 0, sizeof(blinkymenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
	ent->client->menu_update = CoopUpdateBlinkyMenu;
}

static void CoopBlinkyChaseCamExec(edict_t *ent, pmenuhnd_t *p)
{
	static char cmd[32];
	int playernum;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	playernum = p->cur - CHASEMENU_START;
	other = &g_edicts[playernum+1];
	if(!other || !other->client)
	{
		return;
	}

	Com_sprintf(cmd, sizeof(cmd), "cam CL %d\n", playernum);

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(ent, true);

	PMenu_Close(ent);
}

static void CoopInitBlinkyChaseMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			chasemenu[i + CHASEMENU_START].text = NULL;
			chasemenu[i + CHASEMENU_START].SelectFunc = NULL;
			continue;
		}

		chasemenu[i + CHASEMENU_START].text = other->client->pers.netname;
		chasemenu[i + CHASEMENU_START].SelectFunc = CoopBlinkyChaseCamExec;
	}
}

static void CoopUpdateBlinkyChaseMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;
	pmenuhnd_t *p;

	if(!ent || !ent->client)
	{
		return;
	}

	p = ent->client->menu;
	if(!p)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			PMenu_UpdateEntry(p->entries + i + CHASEMENU_START, NULL, PMENU_ALIGN_LEFT, NULL);
			continue;
		}

		PMenu_UpdateEntry(p->entries + i + CHASEMENU_START, other->client->pers.netname, PMENU_ALIGN_LEFT, CoopBlinkyChaseCamExec);
	}
}

void CoopBlinkyChaseMenu(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	CoopInitBlinkyChaseMenu(ent);
	PMenu_Open(ent, chasemenu, NULL, 0, sizeof(chasemenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
	ent->client->menu_update = CoopUpdateBlinkyChaseMenu;
}

static void CoopBlinkySummonExec(edict_t *ent, pmenuhnd_t *p)
{
	static char cmd[32];
	int playernum;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	playernum = p->cur - SUMMONMENU_START;
	other = &g_edicts[playernum+1];
	if(!other || !other->client)
	{
		return;
	}

	Com_sprintf(cmd, sizeof(cmd), "summon CL %d\n", playernum);

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(ent, true);

	PMenu_Close(ent);
}

static void CoopInitBlinkySummonMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			summonmenu[i + SUMMONMENU_START].text = NULL;
			summonmenu[i + SUMMONMENU_START].SelectFunc = NULL;
			continue;
		}

		summonmenu[i + SUMMONMENU_START].text = other->client->pers.netname;
		summonmenu[i + SUMMONMENU_START].SelectFunc = CoopBlinkySummonExec;
	}
}

static void CoopUpdateBlinkySummonMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;
	pmenuhnd_t *p;

	if(!ent || !ent->client)
	{
		return;
	}

	p = ent->client->menu;
	if(!p)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			PMenu_UpdateEntry(p->entries + i + SUMMONMENU_START, NULL, PMENU_ALIGN_LEFT, NULL);
			continue;
		}

		PMenu_UpdateEntry(p->entries + i + SUMMONMENU_START, other->client->pers.netname, PMENU_ALIGN_LEFT, CoopBlinkySummonExec);
	}
}

void CoopBlinkySummonMenu(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	CoopInitBlinkySummonMenu(ent);
	PMenu_Open(ent, summonmenu, NULL, 0, sizeof(summonmenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
	ent->client->menu_update = CoopUpdateBlinkySummonMenu;
}

static void CoopBlinkyTeleportExec(edict_t *ent, pmenuhnd_t *p)
{
	static char cmd[32];
	int playernum;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	playernum = p->cur - TELEPORTMENU_START;
	other = &g_edicts[playernum+1];
	if(!other || !other->client)
	{
		return;
	}

	Com_sprintf(cmd, sizeof(cmd), "teleport CL %d\n", playernum);

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(ent, true);

	PMenu_Close(ent);
}

static void CoopInitBlinkyTeleportMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			teleportmenu[i + TELEPORTMENU_START].text = NULL;
			teleportmenu[i + TELEPORTMENU_START].SelectFunc = NULL;
			continue;
		}

		teleportmenu[i + TELEPORTMENU_START].text = other->client->pers.netname;
		teleportmenu[i + TELEPORTMENU_START].SelectFunc = CoopBlinkyTeleportExec;
	}
}

static void CoopUpdateBlinkyTeleportMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;
	pmenuhnd_t *p;

	if(!ent || !ent->client)
	{
		return;
	}

	p = ent->client->menu;
	if(!p)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			PMenu_UpdateEntry(p->entries + i + TELEPORTMENU_START, NULL, PMENU_ALIGN_LEFT, NULL);
			continue;
		}

		PMenu_UpdateEntry(p->entries + i + TELEPORTMENU_START, other->client->pers.netname, PMENU_ALIGN_LEFT, CoopBlinkyTeleportExec);
	}
}

void CoopBlinkyTeleportMenu(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	CoopInitBlinkyTeleportMenu(ent);
	PMenu_Open(ent, teleportmenu, NULL, 0, sizeof(teleportmenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
	ent->client->menu_update = CoopUpdateBlinkyTeleportMenu;
}

void CoopGamemodeInit (void)
{
	char fileName[MAX_OSPATH];
	char *mapToken = NULL;
	char *gamemodeToken = NULL;
	char *fileBuffer = NULL;
	char *listPtr = NULL;
	char *listPtr2 = NULL;
	char separators[] = "\n";
	long fileSize;
	FILE *f = NULL;
	size_t toEOF = 0;

	memset(gamemode_array, 0, sizeof(gamemode_t)*MAX_GAMEMODES);

	CoopGamemodeAdd("vanilla", "vanilla", "base1.bsp");
	CoopGamemodeAdd("xatrix", "xatrix", "xswamp.bsp");
	CoopGamemodeAdd("rogue", "rogue", "rmine1.bsp");
	CoopGamemodeAdd("zaero", "zaero", "zbase1.bsp");

	if(sv_coop_maplist->string[0] == 0)
	{
		gi.cprintf(NULL, PRINT_CHAT, "CoopGamemodeInit: sv_coop_maplist CVAR empty!\n");
		return;
	}

	Sys_Mkdir(va("%s/maps", gamedir->string));
	Com_sprintf(fileName, sizeof(fileName), "%s/%s", gamedir->string, sv_coop_maplist->string);

	f = fopen(fileName, "r");
	if(!f)
	{
		gi.cprintf(NULL, PRINT_CHAT, "CoopGamemodeInit: couldn't find '%s'!\n", sv_coop_maplist->string);
		return;
	}

	/* obtain file size */
	fseek (f, 0, SEEK_END);
	fileSize = ftell (f);
	fseek (f, 0, SEEK_SET);

	fileBuffer = (char *)malloc(sizeof(char)*(fileSize+2)); /* FS: In case we have to add a newline terminator */
	if(!fileBuffer)
	{
		gi.cprintf(NULL, PRINT_CHAT, "CoopGamemodeInit: can't allocate memory for coopGamemodeBuffer!\n");
		fclose(f);
		return;
	}
	toEOF = fread(fileBuffer, sizeof(char), fileSize, f);
	fclose(f);
	if(toEOF <= 0)
	{
		gi.cprintf(NULL, PRINT_CHAT, "CoopGamemodeInit: cannot read file '%s' into memory!\n", sv_coop_maplist->string);
		if(fileBuffer)
		{
			free(fileBuffer);
		}
		return;
	}

	/* FS: Add newline terminator for some paranoia */
	fileBuffer[toEOF] = '\n';
	fileBuffer[toEOF+1] = '\0';

	mapToken = strtok_r(fileBuffer, separators, &listPtr);

	while(mapToken)
	{
		char mapname[64];

		Com_sprintf(mapname, sizeof(mapname), "%s", mapToken);
		gamemodeToken = strtok_r(mapname, ",", &listPtr2);
		gamemodeToken = strtok_r(NULL, ",\n", &listPtr2);
		if (gamemodeToken)
		{
			char gamemode[64];
			char realgamemode[64] = "vanilla";

			Com_sprintf(gamemode, sizeof(gamemode), "%s", gamemodeToken);

			gamemodeToken = strtok_r(NULL, ",\n", &listPtr2);
			if (gamemodeToken)
			{
				Com_sprintf(realgamemode, sizeof(realgamemode), "%s", gamemodeToken);
			}

			if((gamemode[0]) && (CoopGamemodeExists(gamemode) == GAMEMODE_AVAILABLE))
			{
				gi.dprintf(DEVELOPER_MSG_GAME, "CoopGamemodeInit: Adding %s %s %s\n", mapname, gamemode, realgamemode);
				CoopGamemodeAdd(gamemode, realgamemode, mapname);
			}
		}
		mapToken = strtok_r(NULL, separators, &listPtr);
	}

	if(fileBuffer)
	{
		free(fileBuffer);
	}
}
int CoopGamemodeExists (const char *gamemode)
{
	int i;

	if (!gamemode || !strlen(gamemode))
		return GAMEMODE_ERROR;

	for (i = 0; i < MAX_GAMEMODES; i++)
	{
		if (!strcmp(gamemode_array[i].gamemode, gamemode))
			return i;
	}

	return GAMEMODE_AVAILABLE;
}

void CoopGamemodeAdd (const char *gamemode, const char *realgamemode, const char *mapname)
{
	static int gamemode_index;

	if (!gamemode || !strlen(gamemode) || !mapname || !strlen(mapname) || !realgamemode || !strlen(realgamemode))
		return;

	if (gamemode_index > MAX_GAMEMODES-1)
		gi.error("Can't add gamemode %s to voting list.  Limit %d reached.");

	Com_sprintf(gamemode_array[gamemode_index].gamemode, sizeof(gamemode_array[gamemode_index].gamemode), "%s", gamemode);
	Com_sprintf(gamemode_array[gamemode_index].mapname, sizeof(gamemode_array[gamemode_index].mapname), "%s", mapname);
	Com_sprintf(gamemode_array[gamemode_index].realgamemode, sizeof(gamemode_array[gamemode_index].realgamemode), "%s", realgamemode);

	gamemode_index++;
	gamemodeCount++;
}

void CoopVoteGamemodeDynamic(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	int i;
	size_t size;

	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	votemenu_loadmaplist();
	size = sizeof(pmenu_t) * (gamemodeCount + 2);
	votegamemodemenu = malloc(size);
	if (!votegamemodemenu)
	{
		gi.error("CoopVoteGamemodeDynamic:  Failed allocating memory.\n");
		return;
	}
	memset((pmenu_t *)votegamemodemenu, 0, size);

	for (i = 0; i < gamemodeCount; i++)
	{
		if(i < MAX_GAMEMODES && gamemode_array[i].gamemode[0] != '\0')
		{
			votegamemodemenu[i].text = gamemode_array[i].gamemode;
			votegamemodemenu[i].align = PMENU_ALIGN_LEFT;
			votegamemodemenu[i].SelectFunc = CoopCheckGamemode;
		}
	}

	votegamemodemenu[i].text = NULL;
	votegamemodemenu[i].align = PMENU_ALIGN_CENTER;
	votegamemodemenu[i].SelectFunc = NULL;

	votegamemodemenu[i+1].text = "Return to Voting Menu";
	votegamemodemenu[i+1].align = PMENU_ALIGN_LEFT;
	votegamemodemenu[i+1].SelectFunc = CoopReturnToVoteMenu;

	CoopUpdateGamemodeMenu(ent); /* FS: For current gamemode */

	PMenu_Open(ent, votegamemodemenu, votegamemodemenuheader, 0, gamemodeCount + 2, sizeof(votegamemodemenuheader) / sizeof(pmenu_t), NULL, PMENU_SCROLLING);

	if (votegamemodemenu)
	{
		free(votegamemodemenu);
	}
	votegamemodemenu = NULL;
}

static void CoopVotePlayerKickExec(edict_t *ent, pmenuhnd_t *p)
{
	static char cmd[32];
	int playernum;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	playernum = p->cur - PLAYERKICKMENU_START;
	other = &g_edicts[playernum+1];
	if(!other || !other->client)
	{
		return;
	}

	Com_sprintf(cmd, sizeof(cmd), "vote kick_menu %d\n", playernum);

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(ent, true);

	PMenu_Close(ent);
}

static void CoopVotePlayerBanExec(edict_t *ent, pmenuhnd_t *p)
{
	static char cmd[32];
	int playernum;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	playernum = p->cur - PLAYERBANMENU_START;
	other = &g_edicts[playernum+1];
	if(!other || !other->client)
	{
		return;
	}

	Com_sprintf(cmd, sizeof(cmd), "vote ban_menu %d\n", playernum);

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(ent, true);

	PMenu_Close(ent);
}

static void CoopVotePlayerSilenceExec(edict_t *ent, pmenuhnd_t *p)
{
	static char cmd[32];
	int playernum;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	playernum = p->cur - PLAYERKICKMENU_START;
	other = &g_edicts[playernum+1];
	if(!other || !other->client)
	{
		return;
	}

	Com_sprintf(cmd, sizeof(cmd), "vote silence_menu %d\n", playernum);

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(ent, true);

	PMenu_Close(ent);
}

static void CoopInitVotePlayerKickMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			playerkickmenu[i + PLAYERKICKMENU_START].text = NULL;
			playerkickmenu[i + PLAYERKICKMENU_START].SelectFunc = NULL;
			continue;
		}

		playerkickmenu[i + PLAYERKICKMENU_START].text = other->client->pers.netname;
		playerkickmenu[i + PLAYERKICKMENU_START].SelectFunc = CoopVotePlayerKickExec;
	}
}

static void CoopInitVotePlayerBanMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			playerbanmenu[i + PLAYERBANMENU_START].text = NULL;
			playerbanmenu[i + PLAYERBANMENU_START].SelectFunc = NULL;
			continue;
		}

		playerbanmenu[i + PLAYERBANMENU_START].text = other->client->pers.netname;
		playerbanmenu[i + PLAYERBANMENU_START].SelectFunc = CoopVotePlayerBanExec;
	}
}

static void CoopInitVotePlayerSilenceMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;

	if(!ent || !ent->client)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			playersilencemenu[i + PLAYERSILENCEMENU_START].text = NULL;
			playersilencemenu[i + PLAYERSILENCEMENU_START].SelectFunc = NULL;
			continue;
		}

		playersilencemenu[i + PLAYERSILENCEMENU_START].text = other->client->pers.netname;
		playersilencemenu[i + PLAYERSILENCEMENU_START].SelectFunc = CoopVotePlayerSilenceExec;
	}
}

static void CoopUpdateVotePlayerKickMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;
	pmenuhnd_t *p;

	if(!ent || !ent->client)
	{
		return;
	}

	p = ent->client->menu;
	if(!p)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			PMenu_UpdateEntry(p->entries + i + PLAYERKICKMENU_START, NULL, PMENU_ALIGN_LEFT, NULL);
			continue;
		}

		PMenu_UpdateEntry(p->entries + i + PLAYERKICKMENU_START, other->client->pers.netname, PMENU_ALIGN_LEFT, CoopVotePlayerKickExec);
	}
}

static void CoopUpdateVotePlayerBanMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;
	pmenuhnd_t *p;

	if(!ent || !ent->client)
	{
		return;
	}

	p = ent->client->menu;
	if(!p)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			PMenu_UpdateEntry(p->entries + i + PLAYERBANMENU_START, NULL, PMENU_ALIGN_LEFT, NULL);
			continue;
		}

		PMenu_UpdateEntry(p->entries + i + PLAYERBANMENU_START, other->client->pers.netname, PMENU_ALIGN_LEFT, CoopVotePlayerBanExec);
	}
}

static void CoopUpdateVotePlayerSilenceMenu(edict_t *ent)
{
	int i;
	edict_t *other = NULL;
	pmenuhnd_t *p;

	if(!ent || !ent->client)
	{
		return;
	}

	p = ent->client->menu;
	if(!p)
	{
		return;
	}

	for (i = 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];
		if(!other || !other->inuse || !other->client || other->client->pers.spectator)
		{
			PMenu_UpdateEntry(p->entries + i + PLAYERSILENCEMENU_START, NULL, PMENU_ALIGN_LEFT, NULL);
			continue;
		}

		PMenu_UpdateEntry(p->entries + i + PLAYERSILENCEMENU_START, other->client->pers.netname, PMENU_ALIGN_LEFT, CoopVotePlayerSilenceExec);
	}
}

void CoopVotePlayerKick(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	CoopInitVotePlayerKickMenu(ent);
	PMenu_Open(ent, playerkickmenu, NULL, 0, sizeof(playerkickmenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
	ent->client->menu_update = CoopUpdateVotePlayerKickMenu;
}

void CoopVotePlayerBan(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	CoopInitVotePlayerBanMenu(ent);
	PMenu_Open(ent, playerbanmenu, NULL, 0, sizeof(playerbanmenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
	ent->client->menu_update = CoopUpdateVotePlayerBanMenu;
}

void CoopVotePlayerSilence(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if(!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);

	CoopInitVotePlayerSilenceMenu(ent);
	PMenu_Open(ent, playersilencemenu, NULL, 0, sizeof(playersilencemenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
	ent->client->menu_update = CoopUpdateVotePlayerSilenceMenu;
}

static void CoopInitVoteHookMenu(edict_t *ent)
{
	CoopUpdateVoteHookMenu(ent);
}

void CoopVoteHookMenu(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	if (!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	CoopInitVoteHookMenu(ent);
	PMenu_Open(ent, hookmenu, NULL, 0, sizeof(hookmenu) / sizeof(pmenu_t), 0, NULL, PMENU_NORMAL);
	ent->client->menu_update = CoopUpdateVoteHookMenu;
}

void CoopUpdateVoteHookMenu(edict_t *ent)
{
	static char status[20], option[20];
	pmenuhnd_t *p;

	if (!ent || !ent->client)
	{
		return;
	}

	p = ent->client->menu;
	if (!p)
	{
		return;
	}

	if (sv_allow_hook->intValue)
	{
		Com_sprintf(status, sizeof(status), "Status: enabled");
		Com_sprintf(option, sizeof(option), "Disable");
	}
	else
	{
		Com_sprintf(status, sizeof(status), "Status: disabled");
		Com_sprintf(option, sizeof(option), "Enable");
	}

	PMenu_UpdateEntry(p->entries + HOOKMENU_STATUS, status, PMENU_ALIGN_LEFT, NULL);
	PMenu_UpdateEntry(p->entries + HOOKMENU_VOTEOPTION, option, PMENU_ALIGN_LEFT, CoopVoteToggleHook);
}

void CoopVoteToggleHook(edict_t *ent, pmenuhnd_t *p /* unused */)
{
	static char cmd[11];

	if (!ent || !ent->client)
	{
		return;
	}

	Com_sprintf(cmd, sizeof(cmd), "vote hook\n");

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(ent, true);

	PMenu_Close(ent);
}
