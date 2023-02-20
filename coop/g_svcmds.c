
#include "g_local.h"


void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

typedef struct
{
	unsigned	mask;
	unsigned	compare;
} ipfilter_t;

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[MAX_IPFILTERS];
int			numipfilters;

#define MAX_NAMEFILTERS 1024

char		namefilters[MAX_NAMEFILTERS][256];
int			numnamefilters;

/*
=================
StringToFilter
=================
*/
qboolean
StringToFilter(char *s, ipfilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];

	if (!s || !f)
	{
		return false;
	}

	for (i = 0; i < 4; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}

	for (i = 0; i < 4; i++)
	{
		if ((*s < '0') || (*s > '9'))
		{
			gi.cprintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
			return false;
		}

		j = 0;

		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}

		num[j] = 0;
		b[i] = atoi(num);

		if (b[i] != 0)
		{
			m[i] = 255;
		}

		if (!*s)
		{
			break;
		}

		s++;
	}

	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;

	return true;
}

qboolean
SV_FilterPacket(char *from)
{
	int i;
	unsigned in;
	byte m[4];
	char *p;

	if (!from)
	{
		return false;
	}

	i = 0;
	p = from;

	while (*p && i < 4)
	{
		m[i] = 0;

		while (*p >= '0' && *p <= '9')
		{
			m[i] = m[i] * 10 + (*p - '0');
			p++;
		}

		if (!*p || (*p == ':'))
		{
			break;
		}

		i++, p++;
	}

	in = *(unsigned *)m;

	for (i = 0; i < numipfilters; i++)
	{
		if ((in & ipfilters[i].mask) == ipfilters[i].compare)
		{
			return filterban->intValue;
		}
	}

	return !filterban->intValue;
}

void
SVCmd_AddIP_f(void)
{
	int i;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n");
		return;
	}

	for (i = 0; i < numipfilters; i++)
	{
		if (ipfilters[i].compare == 0xffffffff)
		{
			break; /* free spot */
		}
	}

	if (i == numipfilters)
	{
		if (numipfilters == MAX_IPFILTERS)
		{
			gi.cprintf(NULL, PRINT_HIGH, "IP filter list is full\n");
			return;
		}

		numipfilters++;
	}

	if (!StringToFilter(gi.argv(2), &ipfilters[i]))
	{
		ipfilters[i].compare = 0xffffffff;
	}
}

void
SVCmd_RemoveIP_f(void)
{
	ipfilter_t f;
	int i, j;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
		return;
	}

	if (!StringToFilter(gi.argv(2), &f))
	{
		return;
	}

	for (i = 0; i < numipfilters; i++)
	{
		if ((ipfilters[i].mask == f.mask) &&
			(ipfilters[i].compare == f.compare))
		{
			for (j = i + 1; j < numipfilters; j++)
			{
				ipfilters[j - 1] = ipfilters[j];
			}

			numipfilters--;
			gi.cprintf(NULL, PRINT_HIGH, "Removed.\n");
			return;
		}
	}

	gi.cprintf(NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
}

void
SVCmd_ListIP_f(void)
{
	int i;
	byte b[4];

	gi.cprintf(NULL, PRINT_HIGH, "Filter list:\n");

	for (i = 0; i < numipfilters; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		gi.cprintf(NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
	}
}

void
SVCmd_WriteIP_f(void)
{
	FILE *f;
	char name[MAX_OSPATH];
	byte b[4];
	int i;
	cvar_t *game;

	game = gi.cvar("game", "", 0);

	if (!*game->string)
	{
		sprintf(name, "%s/listip.cfg", GAMEVERSION);
	}
	else
	{
		sprintf(name, "%s/listip.cfg", game->string);
	}

	gi.cprintf(NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen(name, "wb");

	if (!f)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}

	fprintf(f, "set filterban %d\n", filterban->intValue);

	for (i = 0; i < numipfilters; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		fprintf(f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
	}

	fclose(f);
}

void SVCmd_SayPerson_f (void)
{
	char msg[2000]; /* FS: Q2Admin magic limit */
	char *p = NULL;
	edict_t	*client;
	int playernum;
	size_t messageLen;

	memset(msg, 0, sizeof(msg));

	if (gi.argc() < 5)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv !say_person CL <playernum> <message>\n");
		return;
	}

	playernum = atoi(gi.argv(3));

	if(playernum > game.maxclients)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Error: Playernum %d greater than maxclients!\n", playernum);
		return;
	}

	client = &g_edicts[playernum + 1];

	if (!client || !client->inuse || !client->client /* || !client->client->pers.connected */) /* FS: FIXME: Unreliable check */
	{
		gi.cprintf(NULL, PRINT_HIGH, "Didn't find %d.\n", playernum);
		return;
	}

	Com_sprintf(msg, sizeof(msg), "!say_person CL %s ", gi.argv(3));
	messageLen = strlen(msg);
	p = gi.args() + messageLen;
	if(!p)
	{
		return;
	}

	Com_sprintf (msg, sizeof(msg), "(private message) ");
	Com_strcat (msg, sizeof(msg), p);

	/* don't let text be too long for malicious reasons */
	if (strlen(msg) > sizeof(msg)-1) /* FS: This is unfortunate, but this is what TSAdmin uses and wallfly can hit about 512 sometimes... */
	{
		msg[sizeof(msg)-1] = '\0';
	}

	Com_strcat (msg, sizeof(msg), "\n");

	gi.cprintf(client, PRINT_CHAT, "%s", msg);
}

void SVCmd_StuffCmd_f (void)
{
	char cmd[2000]; /* FS: Q2Admin magic limit */
	char *p = NULL;
	edict_t	*client = NULL;
	int playernum;
	size_t messageLen;
	qboolean bIsCL = false;

	memset(cmd, 0, sizeof(cmd));

	if (gi.argc() < 4)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv !stuff CL <playernum> <cmd> OR sv !stuff <player_name> <cmd>\n");
		return;
	}

	if(!Q_stricmp(gi.argv(2), "CL"))
	{
		playernum = atoi(gi.argv(3));
		bIsCL = true;
	}
	else
	{
		int i = 0;
		int hits = 0;

		bIsCL = false;

		for (i = 1; i <= game.maxclients; i++)
		{
			client = &g_edicts[i];

			if(!client || !client->inuse || !client->client /* || !client->client->pers.connected */ || !client->client->pers.netname[0]) /* FS: FIXME: Unreliable check */
			{
				continue;
			}
			else
			{
				if(!Q_stricmp(gi.argv(2), client->client->pers.netname))
				{
					hits++;
					playernum = i;
				}
			}
		}

		if (hits > 1)
		{
			gi.cprintf(NULL, PRINT_HIGH, "2 or more player name matches.\n");
			return;
		}

		if(hits == 0)
		{
			gi.cprintf(NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
			return;
		}
		else
		{
			client = &g_edicts[playernum];
		}
	}

	if(playernum > game.maxclients)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Error: Playernum %d greater than maxclients!\n", playernum);
		return;
	}

	if(bIsCL)
	{
		client = &g_edicts[playernum + 1];

		if (!client || !client->inuse || !client->client /* || !client->client->pers.connected */) /* FS: FIXME: Unreliable check */
		{
			gi.cprintf(NULL, PRINT_HIGH, "Didn't find %d.\n", playernum);
			return;
		}

		Com_sprintf(cmd, sizeof(cmd), "!stuff CL %s ", gi.argv(3));
	}
	else
	{
		Com_sprintf(cmd, sizeof(cmd), "!stuff %s ", gi.argv(2));
	}

	messageLen = strlen(cmd);
	p = gi.args() + messageLen;
	if(!p)
	{
		return;
	}

	Com_sprintf (cmd, sizeof(cmd), "%s\n", p);

	gi.WriteByte(svc_stufftext);
	gi.WriteString(cmd);
	gi.unicast(client, true);
}

void
SVCmd_Coop_Gamemode_f(void)
{
	char command[1024];
	char *cmd;
	int argc = 0;

	argc = gi.argc();
	cmd = gi.argv(2);
	if (argc != 3 || !cmd)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Valid gamemodes: vanilla, rogue, xatrix, and zaero\n");
		return;
	}

	if(!Q_stricmp(cmd, "vanilla"))
	{
		gi.bprintf(PRINT_HIGH, "Changing gamemode to iD coop!\n");
		gi.cvar_forceset("sv_coop_gamemode", "vanilla");
		Com_sprintf(command, sizeof(command), "map base1\n");
		gi.AddCommandString(command);
	}
	else if(!Q_stricmp(cmd, "rogue"))
	{
		gi.bprintf(PRINT_HIGH, "Changing gamemode to Rogue coop!\n");
		gi.cvar_forceset("sv_coop_gamemode", "rogue");
		Com_sprintf(command, sizeof(command), "map rmine1\n");
		gi.AddCommandString(command);
	}
	else if(!Q_stricmp(cmd, "xatrix"))
	{
		gi.bprintf(PRINT_HIGH, "Changing gamemode to Xatrix coop!\n");
		gi.cvar_forceset("sv_coop_gamemode", "xatrix");
		Com_sprintf(command, sizeof(command), "map xswamp\n");
		gi.AddCommandString(command);
	}
	else if(!Q_stricmp(cmd, "zaero"))
	{
		gi.bprintf(PRINT_HIGH, "Changing gamemode to Zaero coop!\n");
		gi.cvar_forceset("sv_coop_gamemode", "zaero");
		Com_sprintf(command, sizeof(command), "map zbase1\n");
		gi.AddCommandString(command);
	}
	else
	{
		gi.cprintf(NULL, PRINT_HIGH, "Unknown gamemode: %s\n", cmd);
	}
}

/* FS: Ban by name BEGIN */
void SVCmd_AddName_f (void)
{
	int i;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  addname <name>\n");
		return;
	}

	for (i = 0; i < numnamefilters; i++)
	{
		if (!Q_stricmp(namefilters[i], gi.argv(2)))
		{
			return;
		}
	}

	Q_strncpyz(namefilters[i], gi.argv(2), sizeof(namefilters[i]));
	numnamefilters++;
}

void SVCmd_RemoveName_f (void)
{
	int i;

	if (gi.argc() < 3)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  removename <name>\n");
		return;
	}

	for (i = 0; i < numnamefilters; i++)
	{
		if (!Q_stricmp(namefilters[i], gi.argv(2)))
		{
			namefilters[i][0] = '\0';
			numnamefilters--;
			break;
		}
	}
}

qboolean SV_FilterName (char *from)
{
	int i;

	if (!from)
	{
		return false;
	}

	for (i = 0; i < numnamefilters; i++)
	{
		if (!Q_stricmp(namefilters[i], from))
		{
			return filterban->intValue;
		}
	}

	return !filterban->intValue;
}

void SVCmd_ListName_f (void)
{
	int i;

	gi.cprintf(NULL, PRINT_HIGH, "Filter list:\n");

	for (i = 0; i < numnamefilters; i++)
	{
		gi.cprintf(NULL, PRINT_HIGH, "%s\n", namefilters[i]);
	}
}

void SVCmd_WriteName_f (void)
{
	FILE *f;
	char name[MAX_OSPATH];
	int i;
	cvar_t *game;

	game = gi.cvar("game", "", 0);

	if (!*game->string)
	{
		sprintf(name, "%s/listname.cfg", GAMEVERSION);
	}
	else
	{
		sprintf(name, "%s/listname.cfg", game->string);
	}

	gi.cprintf(NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen(name, "wb");

	if (!f)
	{
		gi.cprintf(NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}

	fprintf(f, "set filterban %d\n", filterban->intValue);

	for (i = 0; i < numnamefilters; i++)
	{
		fprintf(f, "sv addname %s\n", namefilters[i]);
	}

	fclose(f);
}

/* FS: Ban by name END */

/*
 * ServerCommand will be called when an "sv" command is issued.
 * The game can issue gi.argc() / gi.argv() commands to get the
 * rest of the parameters
 */
void
ServerCommand(void)
{
	char *cmd;

	cmd = gi.argv(1);

	if (Q_stricmp(cmd, "test") == 0)
	{
		Svcmd_Test_f();
	}
	else if (Q_stricmp(cmd, "addip") == 0)
	{
		SVCmd_AddIP_f();
	}
	else if (Q_stricmp(cmd, "removeip") == 0)
	{
		SVCmd_RemoveIP_f();
	}
	else if (Q_stricmp(cmd, "listip") == 0)
	{
		SVCmd_ListIP_f();
	}
	else if (Q_stricmp(cmd, "writeip") == 0)
	{
		SVCmd_WriteIP_f();
	}
	else if (Q_stricmp(cmd, "!say_person") == 0) /* FS: Tastyspleen q2admin additions */
	{
		SVCmd_SayPerson_f();
	}
	else if (Q_stricmp(cmd, "!stuff") == 0) /* FS: Tastyspleen q2admin additions */
	{
		SVCmd_StuffCmd_f();
	}
	else if (Q_stricmp(cmd, "gamemode") == 0) /* FS: Coop: Added */
	{
		SVCmd_Coop_Gamemode_f();
	}
	else if (Q_stricmp(cmd, "addname") == 0)
	{
		SVCmd_AddName_f();
	}
	else if (Q_stricmp(cmd, "removename") == 0)
	{
		SVCmd_RemoveName_f();
	}
	else if (Q_stricmp(cmd, "listname") == 0)
	{
		SVCmd_ListName_f();
	}
	else if (Q_stricmp(cmd, "writename") == 0)
	{
		SVCmd_WriteName_f();
	}
	else
	{
		gi.cprintf(NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
	}
}
