#include "g_local.h"
#include "m_player.h"

/* FS: Flags for sv_vote_disallow_flags */
#define	VOTE_NOGAMEMODE			0x00000002 /* 2 */
#define	VOTE_NOVANILLA			0x00000004 /* 4 */
#define	VOTE_NOXATRIX			0x00000008 /* 8 */
#define	VOTE_NOROGUE			0x00000010 /* 16 */
#define	VOTE_NOZAERO			0x00000020 /* 32 */
#define	VOTE_NOCOOPSKILL		0x00000040 /* 64 */
#define	VOTE_NOMAP				0x00000080 /* 128 */
#define	VOTE_NORANDOMMAPS		0x00000100 /* 256 */
#define VOTE_NORESETMAP			0x00000200 /* 512 */
#define VOTE_NOPLAYEREXIT		0x00000400 /* 1024 */
#define VOTE_NOPLAYERKICK		0x00000800 /* 2048 */
#define VOTE_NOPLAYERBAN		0x00001000 /* 4096 */
#define VOTE_NOHOOK				0x00002000 /* 8192 */
#define VOTE_NOSILENCE			0x00004000 /* 16384 */

/* Globals */
qboolean bVoteInProgress;
int voteNo;
int voteYes;
int voteClients;
int printOnce;
int voteCoopSkill;
char voteCbufCmdExecute[MAX_OSPATH];
char voteMap[MAX_OSPATH];
char voteType[16];
char whatAreWeVotingFor[MAX_OSPATH];
char voteGamemode[32];
char voteGamemodeStartMap[MAX_QPATH];
char votePlayerBanIP[32];
int votePlayerBanEntNum;
float voteTimer;
int lastUpdate;

#define MAX_RANDOM_HISTORY 6 /* FS: Remember the last 6 random maps chosen */
char random_map_array[MAX_RANDOM_HISTORY][MAX_OSPATH];
int random_map_head = 0, random_map_tail = 0;
int random_map_index = 0;

/* Prototypes */
void vote_map (edict_t *ent, const char *mapName);
void vote_gamemode (edict_t *ent, const char *gamemode);
void vote_coopskill (edict_t *ent, int skillVote);
void vote_random (edict_t *ent);
void vote_restartmap (edict_t *ent);
void vote_stop (edict_t *ent);
void vote_yes (edict_t *ent, qboolean bAssume);
void vote_no (edict_t *ent);
void vote_disconnect_recalc (edict_t *ent);
void vote_Passed (void);
void vote_Failed (qboolean bTied);
void vote_Reset (void);
void vote_Broadcast (const char *fmt, ...);
void vote_Think (void);
void Vote_Random_History_Add (const char *mapname);
void Vote_Clear_Random_History (void);
qboolean Vote_Check_Random_History (const char *mapname);
void vote_DefaultNoVotes (void);
void vote_progress (edict_t *ent);
qboolean vote_mapcheck (edict_t *ent, const char *mapName);
void vote_warp (edict_t *ent, const char *mapName);
void VoteMenuChoice (edict_t *ent, pmenuhnd_t *p);
void vote_menu_broadcast (void);
void VoteMenuOpen (edict_t *ent);
void vote_playerexit (edict_t *ent);
void vote_kickban (edict_t *ent, char *playerName, qboolean banPlayer);
void vote_kickban_menu (edict_t *ent, int entNum, qboolean banPlayer);
void vote_silence (edict_t *ent, char *playerName);
void vote_silence_menu (edict_t *ent, int entNum);
void vote_hook(edict_t *ent);

#define VOTEMENU_TYPE 6
#define VOTEMENU_PROGYES 8
#define VOTEMENU_PROGNO 9
#define VOTEMENU_PROGTIMER 10
#define VOTEMENU_YES 12
#define VOTEMENU_NO 13

pmenu_t voteyesnomenu[] = {
	{"*Quake II", PMENU_ALIGN_CENTER, NULL},
	{"*Mara'akate and Freewill", PMENU_ALIGN_CENTER, NULL},
	{"*Custom Coop", PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"*Vote in progress for:", PMENU_ALIGN_CENTER, NULL}, /* 4 */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_CENTER, NULL}, /* 6: TYPE */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 8: YES */
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 9: NO */
	{NULL, PMENU_ALIGN_LEFT, NULL}, /* 10: TIMER */
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Vote Yes", PMENU_ALIGN_LEFT, VoteMenuChoice}, /* 12 */
	{"Vote No", PMENU_ALIGN_LEFT, VoteMenuChoice}, /* 13*/
	{NULL, PMENU_ALIGN_CENTER, NULL},
	{"Exit", PMENU_ALIGN_LEFT, VoteMenuChoice} /* 14 */
};

void vote_command (edict_t *ent)
{
	int argc = 0;

	argc = gi.argc();

	if (maxclients->intValue <= 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting is only available in multiplayer.\n");
		return;
	}

	if (!sv_vote_enabled->intValue)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting disabled on this server!\n");
		return;
	}

	if (ent->client->pers.spectator)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting is not allowed by spectators.\n");
		return;
	}

	if (level.intermissiontime)
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't vote during an intermission!\n");
		return;
	}

	if (argc <= 1 ||
		(argc >= 2 && (!Q_stricmp(gi.argv(1), "help") || !Q_stricmp(gi.argv(1), "list") || !Q_stricmp(gi.argv(1), "cmds") || !Q_stricmp(gi.argv(1), "commands"))))
	{
		gi.cprintf(ent, PRINT_HIGH, "usage: vote map <mapname>, vote gamemode <gamemode>, vote skill <coopskill>, vote kick <playername>, vote ban <playername>, vote restartmap, vote playerexit, vote yes, vote no, vote stop, and vote progress.\n");
		return;
	}

	if (!Q_stricmp(gi.argv(1), "map"))
	{
		if (argc <= 2)
		{
			gi.cprintf(ent, PRINT_HIGH, "error: you must supply a map name! i.e. vote map base1\n");
			return;
		}
		else
		{
			vote_map(ent, gi.argv(2));
			return;
		}
	}
	else if (!Q_stricmp(gi.argv(1), "restartmap"))
	{
		vote_restartmap(ent);
		return;
	}
	else if (!Q_stricmp(gi.argv(1), "gamemode"))
	{
		if (argc <= 2)
		{
			gi.cprintf(ent, PRINT_HIGH, "error: you must supply a gamemode!\n");
			return;
		}
		else
		{
			vote_gamemode(ent, gi.argv(2));
			return;
		}
	}
	else if (!Q_stricmp(gi.argv(1), "skill"))
	{
		if (argc <= 2)
		{
			gi.cprintf(ent, PRINT_HIGH, "error: you must supply a coop difficulty level!  valid options are: 0 (easy), 1 (medium), 2 (hard), and 3 (nightmare).\n");
			return;
		}
		else
		{
			vote_coopskill(ent, atoi(gi.argv(2)));
			return;
		}
	}
	else if (!Q_stricmp(gi.argv(1), "yes"))
	{
		vote_yes(ent, false);
		return;
	}
	else if (!Q_stricmp(gi.argv(1), "no"))
	{
		vote_no(ent);
		return;
	}
	else if (!Q_stricmp(gi.argv(1), "stop"))
	{
		vote_stop(ent);
		return;
	}
	/* FS: Not ready yet... */
	//	else if (!Q_stricmp(gi.argv(1), "random"))
	//	{
	//		vote_random(ent);
	//		return;
	//	}
	else if (!Q_stricmp(gi.argv(1), "warp"))
	{
		if (argc <= 2)
		{
			gi.cprintf(ent, PRINT_HIGH, "error: you must supply a map name! i.e. vote warp base1\n");
			return;
		}
		else
		{
			vote_warp(ent, gi.argv(2));
			return;
		}
	}
	else if (!Q_stricmp(gi.argv(1), "playerexit"))
	{
		vote_playerexit(ent);
		return;
	}
	else if (!Q_stricmp(gi.argv(1), "progress"))
	{
		vote_progress(ent);
		return;
	}
	else if (!Q_stricmp(gi.argv(1), "kick"))
	{
		if (argc <= 2)
		{
			gi.cprintf(ent, PRINT_HIGH, "error: you must supply a player name! i.e. vote kick quakekiller\n");
			return;
		}
		else
		{
			vote_kickban(ent, gi.argv(2), false);
			return;
		}
	}
	else if (!Q_stricmp(gi.argv(1), "ban"))
	{
		if (argc <= 2)
		{
			gi.cprintf(ent, PRINT_HIGH, "error: you must supply a player name! i.e. vote ban quakekiller\n");
			return;
		}
		else
		{
			vote_kickban(ent, gi.argv(2), true);
			return;
		}
	}
	else if (!Q_stricmp(gi.argv(1), "silence"))
	{
		if (argc <= 2)
		{
			gi.cprintf(ent, PRINT_HIGH, "error: you must supply a player name! i.e. vote silence quakekiller\n");
			return;
		}
		else
		{
			vote_silence(ent, gi.argv(2));
			return;
		}
	}
	else if (!Q_stricmp(gi.argv(1), "silence_menu")) /* FS: Internal menu commands */
	{
		if (argc <= 2)
		{
			return;
		}
		else
		{
			vote_silence_menu(ent, atoi(gi.argv(2)));
		}
	}
	else if (!Q_stricmp(gi.argv(1), "kick_menu")) /* FS: Internal menu commands */
	{
		if (argc <= 2)
		{
			return;
		}
		else
		{
			vote_kickban_menu(ent, atoi(gi.argv(2)), false);
		}
	}
	else if (!Q_stricmp(gi.argv(1), "ban_menu")) /* FS: Internal menu commands */
	{
		if (argc <= 2)
		{
			return;
		}
		else
		{
			vote_kickban_menu(ent, atoi(gi.argv(2)), true);
		}
	}
	else if (!Q_stricmp(gi.argv(1), "hook"))
	{
		vote_hook(ent);
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Unknown vote command: %s.  ", gi.argv(1));
		gi.cprintf(ent, PRINT_HIGH, "valid options are: vote map <mapname>, vote gamemode <gamemode>, vote skill <coopskill>, vote kick <playername>, vote ban <playername>, vote silence <playername>, vote restartmap, vote playerexit, vote hook, vote yes, vote no, vote stop, and vote progress.\n");
		return;
	}
}

qboolean vote_mapcheck (edict_t *ent, const char *mapName)
{
	char mapCheck[MAX_QPATH];
	char fileName[MAX_OSPATH];
	char *mapToken = NULL;
	char *mapToken2 = NULL;
	char *fileBuffer = NULL;
	char *fileBuffer2 = NULL;
	char *listPtr = NULL;
	char *listPtr2 = NULL;
	char separators[] = ",\n";
	//	char separators2[] = ",";
	long fileSize;
	int mapCount = 0;
	FILE *f = NULL;
	size_t toEOF = 0;
	qboolean retval = false;

	if (!ent || !mapName)
	{
		return false;
	}

	if (sv_coop_maplist->string[0] == 0)
	{
		gi.cprintf(NULL, PRINT_CHAT, "vote_mapcheck: sv_coop_maplist CVAR empty!\n");
		return false;
	}

	Sys_Mkdir(va("%s/maps", gamedir->string));
	Com_sprintf(fileName, sizeof(fileName), "%s/%s", gamedir->string, sv_coop_maplist->string);

	f = fopen(fileName, "r");
	if (!f)
	{
		gi.cprintf(NULL, PRINT_CHAT, "vote_mapcheck: couldn't find '%s'!\n", sv_coop_maplist->string);
		return false;
	}

	/* obtain file size */
	fseek(f, 0, SEEK_END);
	fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);
	fileBuffer = (char *)malloc(sizeof(char) * (fileSize + 2)); /* FS: In case we have to add a newline terminator */
	if (!fileBuffer)
	{
		gi.cprintf(NULL, PRINT_CHAT, "vote_mapcheck: can't allocate memory for fileBuffer!\n");
		fclose(f);
		return false;
	}
	toEOF = fread(fileBuffer, sizeof(char), fileSize, f);
	fclose(f);
	if (toEOF <= 0)
	{
		gi.cprintf(NULL, PRINT_CHAT, "vote_mapcheck: cannot read file '%s' into memory!\n", sv_coop_maplist->string);
		if (fileBuffer)
		{
			free(fileBuffer);
		}
		return false;
	}

	/* FS: Add newline terminator for some paranoia */
	fileBuffer[toEOF] = '\n';
	fileBuffer[toEOF + 1] = '\0';

	fileBuffer2 = strdup(fileBuffer);

	mapToken = strtok_r(fileBuffer, separators, &listPtr);
	mapToken2 = strtok_r(fileBuffer2, "\n", &listPtr2);
	if (!mapToken || !mapToken2)
	{
		retval = false;
	}

	Com_sprintf(mapCheck, sizeof(mapCheck), "%s.bsp", mapName);

	while (mapToken)
	{
		if (!Q_stricmp(mapToken, mapCheck))
		{
			size_t mapTokenSize = strlen(mapToken);
			while (mapToken2)
			{
				if (!strncmp(mapToken, mapToken2, mapTokenSize))
				{
					mapToken2 = strtok_r(mapToken2, ",", &listPtr2);
					mapToken2 = strtok_r(NULL, ",\n", &listPtr2);
					if (mapToken2)
					{
						Com_sprintf(voteGamemode, sizeof(voteGamemode), "%s", mapToken2);
						retval = true;
						goto cleanup;
					}
					else
					{
						gi.cprintf(NULL, PRINT_CHAT, "vote_mapcheck: %s with no gamemode in '%s'!\n", mapCheck, sv_coop_maplist->string);
						retval = false;
						goto cleanup;
					}
				}
				mapToken2 = strtok_r(NULL, "\n", &listPtr2);
			}
		}
		mapToken = strtok_r(NULL, separators, &listPtr);
	}
cleanup:
	if (fileBuffer)
		free(fileBuffer);
	if (fileBuffer2)
		free(fileBuffer2);

	return retval;
}

void vote_map (edict_t *ent, const char *mapName)
{
	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_map from a non-player!\n");
		return;
	}

	if (!mapName)
	{
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (strchr(mapName, '.'))
	{
		gi.cprintf(ent, PRINT_HIGH, "Map name can not contain '.'\n");
		return;
	}
	else if (strchr(mapName, ';'))
	{
		gi.cprintf(ent, PRINT_HIGH, "Map name can not contain ';'\n");
		return;
	}

	if (sv_vote_disallow_flags->intValue & VOTE_NOMAP)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for map changes are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	if (!vote_mapcheck(ent, mapName))
	{
		gi.cprintf(ent, PRINT_HIGH, "Map %s doesn't exist!\n", mapName);
		return;
	}

	Com_sprintf(voteMap, sizeof(voteMap), "%s", mapName);
	vote_Broadcast("%s votes for %s! Use vote yes or vote no to submit your vote!\n", ent->client->pers.netname, voteMap);
	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "%s", voteMap);
	Com_sprintf(voteType, sizeof(voteType), "map");

	ent->voteInitiator = true;

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();
}

void vote_warp (edict_t *ent, const char *mapName)
{
	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_warp from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (strchr(mapName, '.'))
	{
		gi.cprintf(ent, PRINT_HIGH, "Map name can not contain '.'\n");
		return;
	}
	else if (strchr(mapName, ';'))
	{
		gi.cprintf(ent, PRINT_HIGH, "Map name can not contain ';'\n");
		return;
	}

	if (sv_vote_disallow_flags->intValue & VOTE_NOMAP)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for map changes are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	if (!vote_mapcheck(ent, mapName))
	{
		gi.cprintf(ent, PRINT_HIGH, "Map %s doesn't exist!\n", mapName);
		return;
	}
	else
	{
		if (Q_stricmp(sv_coop_gamemode->string, voteGamemode))
		{
			gi.cprintf(ent, PRINT_HIGH, "Game mode is %s but Map is for game mode %s.  Vote cancelled!\n", sv_coop_gamemode->string, voteGamemode);
			return;
		}
	}

	Com_sprintf(voteMap, sizeof(voteMap), "%s", mapName);
	vote_Broadcast("%s votes for warping to %s! Use vote yes or vote no to submit your vote!\n", ent->client->pers.netname, voteMap);
	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "%s", voteMap);
	Com_sprintf(voteType, sizeof(voteType), "warp");

	ent->voteInitiator = true;

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();
}

void vote_gamemode (edict_t *ent, const char *gamemode)
{
	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_gamemode from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (sv_vote_disallow_flags->intValue & VOTE_NOGAMEMODE)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for gamemode changes are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	if (!Q_stricmp((char *)gamemode, "vanilla"))
	{
		if (sv_vote_disallow_flags->intValue & VOTE_NOVANILLA)
		{
			gi.cprintf(ent, PRINT_HIGH, "Voting for Vanilla gamemode change is not allowed on this server.  Vote cancelled.\n");
			return;
		}

		Com_sprintf(voteGamemode, sizeof(voteGamemode), "vanilla");
		Com_sprintf(voteGamemodeStartMap, sizeof(voteGamemodeStartMap), "%s", gamemode_array[0].mapname);
	}
	else if (!Q_stricmp((char *)gamemode, "xatrix"))
	{
		if (sv_vote_disallow_flags->intValue & VOTE_NOXATRIX)
		{
			gi.cprintf(ent, PRINT_HIGH, "Voting for Xatrix gamemode change is not allowed on this server.  Vote cancelled.\n");
			return;
		}

		Com_sprintf(voteGamemode, sizeof(voteGamemode), "xatrix");
		Com_sprintf(voteGamemodeStartMap, sizeof(voteGamemodeStartMap), "%s", gamemode_array[1].mapname);
	}
	else if (!Q_stricmp((char *)gamemode, "rogue"))
	{
		if (sv_vote_disallow_flags->intValue & VOTE_NOROGUE)
		{
			gi.cprintf(ent, PRINT_HIGH, "Voting for Rogue gamemode change is not allowed on this server.  Vote cancelled.\n");
			return;
		}

		Com_sprintf(voteGamemode, sizeof(voteGamemode), "rogue");
		Com_sprintf(voteGamemodeStartMap, sizeof(voteGamemodeStartMap), "%s", gamemode_array[2].mapname);
	}
	else if (!Q_stricmp((char *)gamemode, "zaero"))
	{
		if (sv_vote_disallow_flags->intValue & VOTE_NOZAERO)
		{
			gi.cprintf(ent, PRINT_HIGH, "Voting for Rogue gamemode change is not allowed on this server.  Vote cancelled.\n");
			return;
		}

		Com_sprintf(voteGamemode, sizeof(voteGamemode), "zaero");
		Com_sprintf(voteGamemodeStartMap, sizeof(voteGamemodeStartMap), "%s", gamemode_array[3].mapname);
	}
	else
	{
		int i;

		i = CoopGamemodeExists(gamemode);
		if (i >= 0)
		{
			Com_sprintf(voteGamemode, sizeof(voteGamemode), "%s", gamemode);
			Com_sprintf(voteGamemodeStartMap, sizeof(voteGamemodeStartMap), "%s", gamemode_array[i].mapname);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "error: invalid gamemode %s.  Vote cancelled.\n", gamemode);
			return;
		}
	}

	COM_StripExtension(voteGamemodeStartMap, voteGamemodeStartMap);

	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "%s", voteGamemode);
	Com_sprintf(voteType, sizeof(voteType), "gamemode");
	vote_Broadcast("%s votes for %s: %s! Use vote yes or vote no to submit your vote!\n", ent->client->pers.netname, voteType, whatAreWeVotingFor);

	if (sv_vote_assume_yes->intValue)
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */

	vote_menu_broadcast();

	ent->voteInitiator = true;
}

void vote_coopskill (edict_t *ent, int skillVote)
{
	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_coopskill from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (!coop->intValue)
	{
		gi.cprintf(ent, PRINT_HIGH, "error: not in a coop game!\n");
		return;
	}

	if (sv_vote_disallow_flags->intValue & VOTE_NOCOOPSKILL)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for coop skill changes are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	switch (skillVote)
	{
		case 0:
			voteCoopSkill = 0;
			Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "easy");
			break;
		case 1:
			voteCoopSkill = 1;
			Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "medium");
			break;
		case 2:
			voteCoopSkill = 2;
			Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "hard");
			break;
		case 3:
			voteCoopSkill = 3;
			Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "nightmare");
			break;
		default:
			gi.cprintf(ent, PRINT_HIGH, "error: invalid coop difficulty level!  valid options are: 0 (easy), 1 (medium), 2 (hard), and 3 (nightmare).\n");
			return;
	}

	if (skill->intValue == skillVote)
	{
		gi.cprintf(ent, PRINT_HIGH, "error: skill is already %s!  Vote cancelled.\n", whatAreWeVotingFor);
		return;
	}

	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	Com_sprintf(voteType, sizeof(voteType), "coop difficulty");
	vote_Broadcast("%s votes for %s: %s! Use vote yes or vote no to submit your vote!\n", ent->client->pers.netname, voteType, whatAreWeVotingFor);

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();

	ent->voteInitiator = true;
}

void vote_random (edict_t *ent)
{
	/* FS: TODO: Not ready yet... */
}

void vote_restartmap (edict_t *ent)
{
	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_map from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (sv_vote_disallow_flags->intValue & VOTE_NORESETMAP)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for map restarts are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	Com_sprintf(voteMap, sizeof(voteMap), "%s", level.mapname);
	vote_Broadcast("%s votes for restarting the map! Use vote yes or vote no to submit your vote!\n", ent->client->pers.netname);
	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "%s", voteMap);
	Com_sprintf(voteType, sizeof(voteType), "restartmap");

	ent->voteInitiator = true;

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();
}

void vote_stop (edict_t *ent)
{
	if (!ent || !ent->client) /* FS: This should never happen, but you never know... */
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_stop from a non-player!\n");
		return;
	}

	if (!bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "No vote in progress!\n");
		return;
	}

	if (ent->voteInitiator == true)
	{
		vote_Broadcast("Voting cancelled by %s!\n", ent->client->pers.netname);
		vote_Reset();
		return;
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't cancel a vote you didn't initiate!\n");
		return;
	}
}

void vote_yes (edict_t *ent, qboolean bAssume)
{
	if (!ent || !ent->client) /* FS: This should never happen, but you never know... */
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_yes from a non-player!\n");
		return;
	}

	if (!bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "No vote in progress, ");
		gi.cprintf(ent, PRINT_HIGH, "use vote map <mapname>, vote gamemode <gamemode>, vote skill <coopskill>, vote fraglimit <fraglimit>, or vote timelimit <timelimit> to start a vote!\n");
		return;
	}

	if (ent->hasVoted)
	{
		gi.cprintf(ent, PRINT_HIGH, "You have already voted!\n");
		return;
	}
	else
	{
		/* FS: count a vote... */
		ent->hasVoted = VOTE_YES;
		voteYes++;
		if (sv_vote_private->intValue && !bAssume)
			gi.cprintf(ent, PRINT_HIGH, "Your vote 'yes' for %s has been counted\n", whatAreWeVotingFor);
		else if (!bAssume)
			vote_Broadcast("%s votes yes. Yes: %i, No: %i.\n", ent->client->pers.netname, voteYes, voteNo);
		vote_Think();
		return;
	}
}

void vote_no (edict_t *ent)
{
	if (!ent || !ent->client) /* FS: This should never happen, but you never know... */
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_no from a non-player!\n");
		return;
	}

	if (!bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "No vote in progress, ");
		gi.cprintf(ent, PRINT_HIGH, "use vote map <mapname>, vote gamemode <gamemode>, vote skill <coopskill>, vote fraglimit <fraglimit>, or vote timelimit <timelimit> to start a vote!\n");
		return;
	}

	if (ent->hasVoted)
	{
		gi.cprintf(ent, PRINT_HIGH, "You have already voted!\n");
		return;
	}
	else
	{
		/* FS: count a vote... */
		ent->hasVoted = VOTE_NO;
		voteNo++;

		if (sv_vote_private->intValue)
		{
			gi.cprintf(ent, PRINT_HIGH, "Your vote 'no' for %s has been counted\n", whatAreWeVotingFor);
		}
		else
		{
			vote_Broadcast("%s votes no. Yes: %i, No: %i.\n", ent->client->pers.netname, voteYes, voteNo);
		}

		vote_Think();
		return;
	}
}

void vote_disconnect_recalc (edict_t *ent)
{
	if (!ent || !ent->client) /* FS: This should never happen, but you never know... */
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_disconnect_recalc from a non-player!\n");
		return;
	}

	if (!bVoteInProgress)
	{
		return;
	}

	if (ent->hasVoted)
	{
		if ((ent->hasVoted == VOTE_YES) && (voteYes))
		{
			voteYes--;
		}
		else if ((ent->hasVoted == VOTE_NO) && (voteNo))
		{
			voteNo--;
		}
	}
}

void vote_Reset (void)
{
	int z;
	edict_t *pClient;

	voteClients = voteNo = voteYes = voteCoopSkill = 0;
	voteTimer = level.time + sv_vote_timer->value;
	lastUpdate = (int)(level.time);
	bVoteInProgress = false;
	printOnce = 0;
	votePlayerBanEntNum = 0;
	whatAreWeVotingFor[0] = voteMap[0] = voteGamemode[0] = voteGamemodeStartMap[0] = voteType[0] = votePlayerBanIP[0] = '\0';

	/* FS: Don't do this in single player */
	if (game.maxclients <= 1)
	{
		return;
	}

	/* FS: Reset the hasVoted qbooleanean. */
	for (z = 0; z < game.maxclients; z++)
	{
		pClient = &g_edicts[z + 1];
		if (!pClient->inuse)
		{
			continue;
		}

		if (pClient->client)
		{
			pClient->hasVoted = NOT_VOTED;
			pClient->voteInitiator = false;
			if ((pClient->client->menu) && (pClient->client->menu->menutype == PMENU_VOTE)) /* FS: Close the voting booth */
			{
				PMenu_Close(pClient);
			}
		}
	}
}

static void vote_SetGamemodeCVAR (char *gamemode)
{
	int i;

	if (!gamemode || !strlen(gamemode))
	{
		return;
	}

	i = CoopGamemodeExists(gamemode);
	if (i >= 0)
	{
		gi.cvar_forceset("sv_coop_gamemode", gamemode_array[i].realgamemode);
		gi.cvar_forceset("sv_coop_gamemode_vote", gamemode_array[i].gamemode);
	}
	else
	{
		gi.cvar_forceset("sv_coop_gamemode", gamemode);
		gi.cvar_forceset("sv_coop_gamemode_vote", gamemode);
	}
}

void vote_Passed (void)
{
	vote_Broadcast("Vote passed for %s: %s! Yes: %i, No: %i\n", voteType, whatAreWeVotingFor, voteYes, voteNo);

	if (!Q_stricmp(voteType, "gamemode"))
	{
		vote_SetGamemodeCVAR(voteGamemode);
		Com_sprintf(voteCbufCmdExecute, sizeof(voteCbufCmdExecute), "deathmatch 0; coop 1; wait;wait;wait;wait;wait;map %s\n", voteGamemodeStartMap);
	}
	else if (!Q_stricmp(voteType, "coop difficulty"))
	{
		vote_SetGamemodeCVAR(sv_coop_gamemode->string);
		Com_sprintf(voteCbufCmdExecute, sizeof(voteCbufCmdExecute), "skill %i; wait;wait;wait;wait;wait;map %s", voteCoopSkill, level.mapname);
	}
	else if (!Q_stricmp(voteType, "restartmap"))
	{
		vote_SetGamemodeCVAR(sv_coop_gamemode->string);
		Com_sprintf(voteCbufCmdExecute, sizeof(voteCbufCmdExecute), "wait;wait;wait;wait;wait;map %s\n", voteMap); /* FS: Might want to force a DLL unload for coop overflow fuckery */
	}
	else if (!Q_stricmp(voteType, "warp"))
	{
		Com_sprintf(voteCbufCmdExecute, sizeof(voteCbufCmdExecute), "wait;wait;wait;wait;wait;gamemap %s\n", voteMap);
	}
	else if (!Q_stricmp(voteType, "playerexit"))
	{
		gi.cvar_forceset("sv_coop_check_player_exit", voteGamemode);
		Com_sprintf(voteCbufCmdExecute, sizeof(voteCbufCmdExecute), "\n");
	}
	else if (!Q_stricmp(voteType, "hook"))
	{
		gi.cvar_forceset("sv_allow_hook", voteGamemode);
		Com_sprintf(voteCbufCmdExecute, sizeof(voteCbufCmdExecute), "\n");
	}
	else if (!Q_stricmp(voteType, "kick player"))
	{
		Com_sprintf(voteCbufCmdExecute, sizeof(voteCbufCmdExecute), "kick %d\n", votePlayerBanEntNum);
	}
	else if (!Q_stricmp(voteType, "ban player"))
	{
		Com_sprintf(voteCbufCmdExecute, sizeof(voteCbufCmdExecute), "sv addip %s;sv writeip;kick %d\n", votePlayerBanIP, votePlayerBanEntNum);
	}
	else if (!Q_stricmp(voteType, "silence player"))
	{
		edict_t *ent = &g_edicts[votePlayerBanEntNum];
		if (ent && ent->inuse && ent->client)
		{
			ent->client->pers.isSilenced = true;
			ent->client->resp.isSilenced = true;

			gi.cprintf(ent, PRINT_CHAT, "You have been silenced.\n");
		}
	}
	else
	{
		vote_SetGamemodeCVAR(voteGamemode);
		Com_sprintf(voteCbufCmdExecute, sizeof(voteCbufCmdExecute), "wait;wait;wait;wait;wait;map %s\n", voteMap);
	}

	vote_Reset();

	gi.AddCommandString(voteCbufCmdExecute);
	voteCbufCmdExecute[0] = '\0';
}

void vote_Failed (qboolean bTied)
{
	if (bTied)
	{
		vote_Broadcast("Vote ended in a tie for %s, not changing the %s! Yes: %i, No: %i\n", whatAreWeVotingFor, voteType, voteYes, voteNo);
	}
	else
	{
		vote_Broadcast("Vote failed for %s! Yes: %i, No: %i\n", whatAreWeVotingFor, voteYes, voteNo);
	}

	vote_Reset();
}

void vote_Broadcast (const char *fmt, ...)
{
	int z;
	edict_t *pClient;
	char        msg[1024];
	va_list     argptr;

	va_start(argptr, fmt);
	Q_vsnprintf(msg, sizeof(msg), fmt, argptr); /* Knightmare 10/28/12- buffer-safe version */
	va_end(argptr);

	if (dedicated->intValue)
	{
		gi.cprintf(NULL, PRINT_CHAT, "%s", msg); /* FS: Send it to dedicated console too. */
	}

	for (z = 0; z < game.maxclients; z++)
	{
		pClient = &g_edicts[z + 1];
		if (!pClient->inuse)
		{
			continue;
		}

		if (pClient->client)
		{
			gi.cprintf(pClient, PRINT_CHAT, msg);
			gi.sound(pClient, CHAN_AUTO, gi.soundindex("misc/talk.wav"), 1, ATTN_NORM, 0);
		}
	}
}

void vote_Decide (void)
{
	/* FS: tally votes and decide something. */
	if (voteNo == 0 && voteYes == 0) /* FS: This isn't a true tie, nobody voted! */
	{
		vote_Failed(false);
		return;
	}

	if (voteYes > voteNo)
	{
		vote_Passed();
		return;
	}

	if (voteNo > voteYes)
	{
		vote_Failed(false);
		return;
	}

	if (voteNo == voteYes) /* FS: Tie is a failure */
	{
		vote_Failed(true);
		return;
	}

	return;
}

void vote_updateMenu (void) /* FS: Gotta force the menu to be "dirty" so it updates continously for the timer and progress */
{
	int z, currentTime;
	edict_t *pClient;

	currentTime = (int)(level.time);

	if (currentTime > lastUpdate) /* FS: Don't do this all the time, floods too many packets */
	{
		for (z = 0; z < game.maxclients; z++)
		{
			pClient = &g_edicts[z + 1];
			if (!pClient->inuse)
			{
				continue;
			}

			if (pClient->client)
			{
				if ((pClient->client->menu) && (pClient->client->menu->menutype == PMENU_VOTE))
				{
					pClient->client->menudirty = true;
				}
			}
		}

		lastUpdate = currentTime;
	}
}

void vote_Think (void)
{
	if (!bVoteInProgress || !sv_vote_enabled->intValue)
		return;

	voteClients = P_Clients_Connected(false);

	vote_updateMenu(); /* FS: Gotta force the menu to be "dirty" so it updates continously for the timer and progress */

	if (voteNo + voteYes >= voteClients)
	{
		vote_Decide();
		return;
	}

	if (voteYes > (voteClients / 2)) /* FS: Greater than 50% yes votes?  Finish it up */
	{
		vote_Decide();
		return;
	}

	if (level.time > voteTimer)
	{
		if (voteNo + voteYes < voteClients) /* FS: Timer expired and not enough votes?  Count those who did not vote as No votes. */
		{
			vote_DefaultNoVotes();
		}
		vote_Decide();
		return;
	}
	else if (level.time + 10.0f >= voteTimer && level.time + 9.0f <= voteTimer && printOnce == 0)
	{
		vote_Broadcast("10 seconds remaining for %s vote: %s.\n", voteType, whatAreWeVotingFor);
		printOnce = 1;
		return;
	}
	else if (level.time + 5.0f >= voteTimer && level.time + 4.0f <= voteTimer && printOnce == 1)
	{
		vote_Broadcast("5 seconds remaining for %s vote: %s.\n", voteType, whatAreWeVotingFor);
		printOnce = 2;
		return;
	}
	else if (level.time + 4.0f >= voteTimer && level.time + 3.0f <= voteTimer && printOnce == 2)
	{
		vote_Broadcast("4 seconds remaining for %s vote: %s.\n", voteType, whatAreWeVotingFor);
		printOnce = 3;
		return;
	}
	else if (level.time + 3.0f >= voteTimer && level.time + 2.0f <= voteTimer && printOnce == 3)
	{
		vote_Broadcast("3 seconds remaining for %s vote: %s.\n", voteType, whatAreWeVotingFor);
		printOnce = 4;
		return;
	}
	else if (level.time + 2.0f >= voteTimer && level.time + 1.0f <= voteTimer && printOnce == 4)
	{
		vote_Broadcast("2 seconds remaining for %s vote: %s.\n", voteType, whatAreWeVotingFor);
		printOnce = 5;
		return;
	}
	else if (level.time + 1.0f >= voteTimer && level.time + 0.0f <= voteTimer && printOnce == 5)
	{
		vote_Broadcast("1 second remaining for %s vote: %s.\n", voteType, whatAreWeVotingFor);
		printOnce = 0;
		return;
	}
}

void vote_connect (edict_t *ent)
{
	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_connect from a non-player!\n");
		return;
	}

	if (bVoteInProgress && !ent->hasVoted && !ent->client->pers.spectator)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is in progress for %s: %s. Use vote yes or vote no to submit your vote!\n", voteType, whatAreWeVotingFor);
		VoteMenuOpen(ent);
	}
}

void Vote_Random_History_Add (const char *mapname)
{
	if (!mapname || mapname[0] == 0)
		return;

	/* Taniwha's ring array */
	strcpy(random_map_array[random_map_head], mapname);
	random_map_head = (random_map_head + 1) % MAX_RANDOM_HISTORY;
	if (random_map_head == random_map_tail)
		random_map_tail = (random_map_tail + 1) % MAX_RANDOM_HISTORY;
	//	random_map_array[random_map_head][0] = 0;
	random_map_index = random_map_index;
	/* end */
}

void Vote_Clear_Random_History (void)
{
	int i = MAX_RANDOM_HISTORY - 1;
	while (1)
	{
		if (i < 0)
		{
			return;
		}
		random_map_array[i][0] = 0;
		i--;
	}
}

qboolean Vote_Check_Random_History (const char *mapname)
{
	int i = MAX_RANDOM_HISTORY - 1;

	while (1)
	{
		if (i < 0)
		{
			return true;
		}
		if (random_map_array[i][0] != 0)
		{
			if (!strcmp(mapname, random_map_array[i]))
			{
				return false;
			}
		}
		i--;
	}
}

/* FS: Timer expired and not enough votes?  Count those who did not vote as No votes. */
void vote_DefaultNoVotes (void)
{
	int z;
	edict_t *pClient;

	for (z = 0; z < game.maxclients; z++)
	{
		pClient = &g_edicts[z + 1];
		if (!pClient->inuse)
		{
			continue;
		}

		if (pClient->client)
		{
			if (pClient->client->pers.spectator) /* FS: Don't count spectators */
			{
				continue;
			}

			if (pClient->hasVoted)
			{
				continue;
			}

			pClient->hasVoted = VOTE_NO;
			voteNo++;
		}
	}
}

void vote_progress (edict_t *ent)
{
	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_NewPlayerMessage from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is in progress for %s: %s! Yes; %i, No %i. Time remaining: %1.0f seconds.\n", voteType, whatAreWeVotingFor, voteYes, voteNo, voteTimer - level.time);
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "No vote in progress, ");
		gi.cprintf(ent, PRINT_HIGH, "use vote map <mapname>, vote gamemode <gamemode>, vote skill <coopskill>, vote fraglimit <fraglimit>, or vote timelimit <timelimit> to start a vote!\n");
	}
}

void VoteMenuUpdate (edict_t *ent)
{
	static char progressYes[32];
	static char progressNo[32];
	static char timeRemaining[32];
	pmenuhnd_t *p;

	if (!ent || !ent->client)
	{
		return;
	}

	if (!bVoteInProgress)
	{
		PMenu_Close(ent);
		return;
	}

	p = ent->client->menu;
	if (!p)
	{
		return;
	}

	Com_sprintf(progressYes, sizeof(progressYes), "Yes: %d\n", voteYes);
	Com_sprintf(progressNo, sizeof(progressNo), "No: %d\n", voteNo);
	Com_sprintf(timeRemaining, sizeof(timeRemaining), "Vote Time: %d\n", (int)(voteTimer - level.time));

	PMenu_UpdateEntry(p->entries + VOTEMENU_PROGYES, progressYes, PMENU_ALIGN_LEFT, NULL);
	PMenu_UpdateEntry(p->entries + VOTEMENU_PROGNO, progressNo, PMENU_ALIGN_LEFT, NULL);
	PMenu_UpdateEntry(p->entries + VOTEMENU_PROGTIMER, timeRemaining, PMENU_ALIGN_LEFT, NULL);
}

void VoteMenuInit (edict_t *ent)
{
	static char votestring[32];
	static char progressYes[32];
	static char progressNo[32];
	static char timeRemaining[32];

	if (!ent || !ent->client)
	{
		return;
	}

	Com_sprintf(votestring, sizeof(votestring), "%s: %s", voteType, whatAreWeVotingFor);
	Com_sprintf(progressYes, sizeof(progressYes), "Yes: %d\n", voteYes);
	Com_sprintf(progressNo, sizeof(progressNo), "No: %d\n", voteNo);
	Com_sprintf(timeRemaining, sizeof(timeRemaining), "Vote Time: %d\n", (int)(voteTimer - level.time));

	voteyesnomenu[VOTEMENU_TYPE].text = votestring;
	voteyesnomenu[VOTEMENU_PROGYES].text = progressYes;
	voteyesnomenu[VOTEMENU_PROGNO].text = progressNo;
	voteyesnomenu[VOTEMENU_PROGTIMER].text = timeRemaining;
}

void VoteMenuOpen (edict_t *ent)
{
	if (!ent || !ent->client)
	{
		return;
	}

	PMenu_Close(ent);
	VoteMenuInit(ent);
	PMenu_Open(ent, voteyesnomenu, NULL, 0, sizeof(voteyesnomenu) / sizeof(pmenu_t), 0, NULL, PMENU_VOTE);
	ent->client->menu_update = VoteMenuUpdate;
}

void VoteMenuChoice (edict_t *ent, pmenuhnd_t *p)
{
	if (!ent || !ent->client || !p)
	{
		return;
	}

	switch (p->cur)
	{
		case VOTEMENU_YES:
			vote_yes(ent, false);
			break;
		case VOTEMENU_NO:
			vote_no(ent);
			break;
		default:
			break;
	}

	PMenu_Close(ent);
}

void vote_menu_broadcast (void)
{
	int z;
	edict_t *pClient;

	for (z = 0; z < game.maxclients; z++)
	{
		pClient = &g_edicts[z + 1];
		if (!pClient->inuse)
		{
			continue;
		}

		if (pClient->client)
		{
			if (pClient->client->pers.spectator) /* FS: Don't count spectators */
			{
				continue;
			}

			if (pClient->hasVoted)
			{
				continue;
			}

			VoteMenuOpen(pClient);
		}
	}
}

void vote_playerexit (edict_t *ent)
{
	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_playerexit from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (sv_vote_disallow_flags->intValue & VOTE_NOPLAYEREXIT)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for changing player exit requirements are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	if (sv_coop_check_player_exit->intValue)
	{
		vote_Broadcast("%s votes for changing player exit requirements to disabled! Use vote yes or vote no to submit your vote!\n", ent->client->pers.netname);
		Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "Req. Disabled");
		Com_sprintf(voteGamemode, sizeof(voteGamemode), "0");
	}
	else
	{
		vote_Broadcast("%s votes for changing player exit requirements to enabled! Use vote yes or vote no to submit your vote!\n", ent->client->pers.netname);
		Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "Req. Enabled");
		Com_sprintf(voteGamemode, sizeof(voteGamemode), "1");
	}

	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	Com_sprintf(voteType, sizeof(voteType), "playerexit");

	ent->voteInitiator = true;

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();
}

void vote_kickban (edict_t *ent, char *playerName, qboolean banPlayer)
{
	edict_t *badPlayer = NULL;
	char *ipInfoKey = NULL;
	char userinfo[MAX_INFO_STRING];

	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_kickban from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (!banPlayer && (sv_vote_disallow_flags->intValue & VOTE_NOPLAYERKICK))
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for kicking players are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	if (banPlayer && (sv_vote_disallow_flags->intValue & VOTE_NOPLAYERBAN))
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for banning players are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	if (!playerName)
	{
		gi.cprintf(ent, PRINT_HIGH, "Error: You must provide a player name!\n");
		return;
	}

	badPlayer = Find_LikePlayer(ent, playerName, true);
	if (!badPlayer)
	{
		gi.cprintf(ent, PRINT_HIGH, "Vote cancelled.\n");
		return;
	}

	memcpy(userinfo, badPlayer->client->pers.userinfo, sizeof(userinfo));
	ipInfoKey = Info_ValueForKey(userinfo, "ip");
	votePlayerBanEntNum = badPlayer - g_edicts - 1;

	Com_sprintf(votePlayerBanIP, sizeof(votePlayerBanIP), "%s", ipInfoKey);
	Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "%s", playerName);

	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	if (!banPlayer)
		Com_sprintf(voteType, sizeof(voteType), "kick player");
	else
		Com_sprintf(voteType, sizeof(voteType), "ban player");

	ent->voteInitiator = true;

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();
}

void vote_kickban_menu (edict_t *ent, int entNum, qboolean banPlayer)
{
	edict_t *badPlayer = NULL;
	char *ipInfoKey = NULL;
	char userinfo[MAX_INFO_STRING];

	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_kickban from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (!banPlayer && (sv_vote_disallow_flags->intValue & VOTE_NOPLAYERKICK))
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for kicking players are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	if (banPlayer && (sv_vote_disallow_flags->intValue & VOTE_NOPLAYERBAN))
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for banning players are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	badPlayer = g_edicts + entNum + 1;
	if (!badPlayer)
	{
		gi.cprintf(ent, PRINT_HIGH, "Vote cancelled.\n");
		return;
	}

	memcpy(userinfo, badPlayer->client->pers.userinfo, sizeof(userinfo));
	ipInfoKey = Info_ValueForKey(userinfo, "ip");
	votePlayerBanEntNum = badPlayer - g_edicts - 1;

	Com_sprintf(votePlayerBanIP, sizeof(votePlayerBanIP), "%s", ipInfoKey);
	Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "%s", badPlayer->client->pers.netname);

	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	if (!banPlayer)
		Com_sprintf(voteType, sizeof(voteType), "kick player");
	else
		Com_sprintf(voteType, sizeof(voteType), "ban player");

	ent->voteInitiator = true;

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();
}

void vote_silence (edict_t *ent, char *playerName)
{
	edict_t *badPlayer = NULL;

	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_silence from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (sv_vote_disallow_flags->intValue & VOTE_NOSILENCE)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for silencing players are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	if (!playerName)
	{
		gi.cprintf(ent, PRINT_HIGH, "Error: You must provide a player name!\n");
		return;
	}

	badPlayer = Find_LikePlayer(ent, playerName, true);
	if (!badPlayer)
	{
		gi.cprintf(ent, PRINT_HIGH, "Vote cancelled.\n");
		return;
	}

	votePlayerBanEntNum = badPlayer - g_edicts;

	Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "%s", playerName);

	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	Com_sprintf(voteType, sizeof(voteType), "silence player");

	ent->voteInitiator = true;

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();
}

void vote_silence_menu (edict_t *ent, int entNum)
{
	edict_t *badPlayer = NULL;

	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_silence from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (sv_vote_disallow_flags->intValue & VOTE_NOSILENCE)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for silencing players are not allowed on this server.  Vote cancelled.\n");
		return;
	}

	badPlayer = g_edicts + entNum + 1;
	if (!badPlayer)
	{
		gi.cprintf(ent, PRINT_HIGH, "Vote cancelled.\n");
		return;
	}

	votePlayerBanEntNum = badPlayer - g_edicts;

	Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "%s", badPlayer->client->pers.netname);

	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	Com_sprintf(voteType, sizeof(voteType), "silence player");

	ent->voteInitiator = true;

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();
}

void vote_hook(edict_t *ent)
{
	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Error: vote_hook from a non-player!\n");
		return;
	}

	if (bVoteInProgress)
	{
		gi.cprintf(ent, PRINT_HIGH, "A vote is already in progress for %s: %s!\n", voteType, whatAreWeVotingFor);
		return;
	}

	vote_Reset();

	if (sv_vote_disallow_flags->intValue & VOTE_NOHOOK)
	{
		gi.cprintf(ent, PRINT_HIGH, "Voting for changing grappling hook is not allowed on this server.  Vote cancelled.\n");
		return;
	}

	if (sv_allow_hook->intValue)
	{
		vote_Broadcast("%s votes for disabling the grappling hook! Use vote yes or vote no to submit your vote!\n", ent->client->pers.netname);
		Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "Disabled");
		Com_sprintf(voteGamemode, sizeof(voteGamemode), "0");
	}
	else
	{
		vote_Broadcast("%s votes for enabling the grappling hook! Use vote yes or vote no to submit your vote!\n", ent->client->pers.netname);
		Com_sprintf(whatAreWeVotingFor, sizeof(whatAreWeVotingFor), "Enabled");
		Com_sprintf(voteGamemode, sizeof(voteGamemode), "1");
	}

	voteClients = P_Clients_Connected(false);
	bVoteInProgress = true;
	Com_sprintf(voteType, sizeof(voteType), "hook");

	ent->voteInitiator = true;

	if (sv_vote_assume_yes->intValue)
	{
		vote_yes(ent, true); /* FS: I assume you would want to vote yes if you initiated the vote. */
	}

	vote_menu_broadcast();
}
