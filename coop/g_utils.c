// g_utils.c -- misc utility functions for game module

#include "g_local.h"

#define MAXCHOICES 8

vec3_t VEC_UP = {0, -1, 0};
vec3_t MOVEDIR_UP = {0, 0, 1};
vec3_t VEC_DOWN = {0, -2, 0};
vec3_t MOVEDIR_DOWN = {0, 0, -1};

void
G_ProjectSource(vec3_t point, vec3_t distance, vec3_t forward,
		vec3_t right, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] +
				distance[2];
}

void
G_ProjectSource2(vec3_t point, vec3_t distance, vec3_t forward, /* FS: Coop: Rogue specific */
		vec3_t right, vec3_t up, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1] +
				up[0] * distance[2];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1] +
				up[1] * distance[2];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] +
				up[2] * distance[2];
}

/*
 * Searches all active entities for the next
 * one that holds the matching string at fieldofs
 * (use the FOFS() macro) in the structure.
 *
 * Searches beginning at the edict after from, or
 * the beginning. If NULL, NULL will be returned
 * if the end of the list is reached.
 */
edict_t *
G_Find(edict_t *from, int fieldofs, char *match)
{
	char *s;

	if (!match)
	{
		return NULL;
	}

	if (!from)
	{
		from = g_edicts;
	}
	else
	{
		from++;
	}

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
		{
			continue;
		}

		s = *(char **)((byte *)from + fieldofs);

		if (!s)
		{
			continue;
		}

		if (!Q_stricmp(s, match))
		{
			return from;
		}
	}

	return NULL;
}

/*
 * Returns entities that have origins
 * within a spherical area
 */
edict_t *
findradius(edict_t *from, vec3_t org, float rad)
{
	vec3_t eorg;
	int j;

	if (!from)
	{
		from = g_edicts;
	}
	else
	{
		from++;
	}

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
		{
			continue;
		}

		if (from->solid == SOLID_NOT)
		{
			continue;
		}

		for (j = 0; j < 3; j++)
		{
			eorg[j] = org[j] - (from->s.origin[j] +
					   (from->mins[j] + from->maxs[j]) * 0.5);
		}

		if (VectorLength(eorg) > rad)
		{
			continue;
		}

		return from;
	}

	return NULL;
}

/*
 * Returns entities that have origins within a spherical area
 */
edict_t *
findradius2(edict_t *from, vec3_t org, float rad) /* FS: Coop: Rogue specific */
{
	/* rad must be positive */
	vec3_t eorg;
	int j;

	if (!from)
	{
		from = g_edicts;
	}
	else
	{
		from++;
	}

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
		{
			continue;
		}

		if (from->solid == SOLID_NOT)
		{
			continue;
		}

		if (!from->takedamage)
		{
			continue;
		}

		if (!(from->svflags & SVF_DAMAGEABLE))
		{
			continue;
		}

		for (j = 0; j < 3; j++)
		{
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j]) * 0.5);
		}

		if (VectorLength(eorg) > rad)
		{
			continue;
		}

		return from;
	}

	return NULL;
}

/*
 * Searches all active entities for
 * the next one that holds the matching
 * string at fieldofs (use the FOFS() macro)
 * in the structure.
 *
 * Searches beginning at the edict after from,
 * or the beginning. If NULL, NULL will be
 * returned if the end of the list is reached.
 */
edict_t *
G_PickTarget(char *targetname)
{
	edict_t *ent = NULL;
	int num_choices = 0;
	edict_t *choice[MAXCHOICES];

	if (!targetname)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while (1)
	{
		ent = G_Find(ent, FOFS(targetname), targetname);

		if (!ent)
		{
			break;
		}

		choice[num_choices++] = ent;

		if (num_choices == MAXCHOICES)
		{
			break;
		}
	}

	if (!num_choices)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "G_PickTarget: target %s not found\n", targetname);
		return NULL;
	}

	return choice[rand() % num_choices];
}

void
Think_Delay(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	G_UseTargets(ent, ent->activator);
	G_FreeEdict(ent);
}

/*
 * The global "activator" should be set to
 * the entity that initiated the firing.
 *
 * If self.delay is set, a DelayedUse entity
 * will be created that will actually do the
 * SUB_UseTargets after that many seconds have passed.
 *
 * Centerprints any self.message to the activator.
 *
 * Search for (string)targetname in all entities that
 * match (string)self.target and call their .use function
 */
void
G_UseTargets(edict_t *ent, edict_t *activator)
{
	edict_t *t;
	edict_t *master; /* FS: Coop: Rogue specific */
	qboolean done = false; /* FS: Coop: Rogue specific */

	if (!ent) /* FS: DO NOT PARANOIA CHECK ACTIVATOR! BREAKS PLATS IN XWARE.BSP! */
	{
		return;
	}

	/* check for a delay */
	if (ent->delay)
	{
		/* create a temp object to fire at a later time */
		t = G_Spawn();
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = Think_Delay;
		t->activator = activator;

		if (!activator)
		{
			gi.dprintf(DEVELOPER_MSG_GAME, "Think_Delay with no activator\n");
		}

		t->message = ent->message;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
		return;
	}

	/* print the message */
	if ((activator) && (ent->message) && !(activator->svflags & SVF_MONSTER))
	{
		if(coop->intValue && activator->client && activator->client->pers.netname[0] && strcmp("This item must be activated to use it.", ent->message) != 0) /* FS: Coop: Print any use target stuff as global map message to all players */
		{
			gi.bprintf(PRINT_HIGH, "\x02[MAPMSG][%s]: ", activator->client->pers.netname);
			gi.bprintf(PRINT_HIGH, "%s\n", ent->message);
		}

		gi.centerprintf(activator, "%s", ent->message);

		if (ent->noise_index)
		{
			gi.sound(activator, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound(activator, CHAN_AUTO, gi.soundindex(
							"misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
	}

	/* kill killtargets */
	if (ent->killtarget)
	{
		t = NULL;

		while ((t = G_Find(t, FOFS(targetname), ent->killtarget)))
		{
			if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
			{
				/* if this entity is part of a train, cleanly remove it */
				if (t->flags & FL_TEAMSLAVE)
				{
					if (t->teammaster)
					{
						master = t->teammaster;

						while (!done)
						{
							if (master->teamchain == t)
							{
								master->teamchain = t->teamchain;
								done = true;
							}

							master = master->teamchain;
						}
					}
				}
			}

			if(t->classname)
			{
				/* decrement secret count if target_secret is removed */
				if (!Q_stricmp(t->classname,"target_secret"))
				{
					level.total_secrets--;
				}
				/* same deal with target_goal, but also turn off CD music if applicable */
				else if (!Q_stricmp(t->classname,"target_goal"))
				{
					level.total_goals--;

					if (level.found_goals >= level.total_goals)
					{
						gi.configstring (CS_CDTRACK, "0");
					}
				}
			}

			G_FreeEdict(t);

			if (!ent->inuse)
			{
				gi.dprintf(DEVELOPER_MSG_GAME, "entity was removed while using killtargets\n");
				return;
			}
		}
	}

	/* fire targets */
	if (ent->target)
	{
		t = NULL;

		while ((t = G_Find(t, FOFS(targetname), ent->target)))
		{
			/* doors fire area portals in a specific way */
			if ((t->classname && !Q_stricmp(t->classname, "func_areaportal")) &&
				(ent->classname && (!Q_stricmp(ent->classname, "func_door") ||
				 !Q_stricmp(ent->classname, "func_door_rotating"))))
			{
				continue;
			}

			if (t == ent)
			{
				gi.dprintf(DEVELOPER_MSG_GAME, "WARNING: Entity used itself.\n");
			}
			else
			{
				if (t->use)
				{
					t->use(t, ent, activator);
				}
			}

			if (!ent->inuse)
			{
				gi.dprintf(DEVELOPER_MSG_GAME, "entity was removed while using targets\n");
				return;
			}
		}
	}
}

/*
 * This is just a convenience function
 * for making temporary vectors for function calls
 */
float *
tv(float x, float y, float z)
{
	static int index;
	static vec3_t vecs[8];
	float *v;

	/* use an array so that multiple
	   tempvectors won't collide
	   for a while */
	v = vecs[index];
	index = (index + 1) & 7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

/*
 * This is just a convenience function
 * for printing vectors
 */
char *
vtos(vec3_t v)
{
	static int index;
	static char str[8][32];
	char *s;

	/* use an array so that multiple vtos won't collide */
	s = str[index];
	index = (index + 1) & 7;

	Com_sprintf(s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}

void
G_SetMovedir(vec3_t angles, vec3_t movedir)
{
	if (VectorCompare(angles, VEC_UP))
	{
		VectorCopy(MOVEDIR_UP, movedir);
	}
	else if (VectorCompare(angles, VEC_DOWN))
	{
		VectorCopy(MOVEDIR_DOWN, movedir);
	}
	else
	{
		AngleVectors(angles, movedir, NULL, NULL);
	}

	VectorClear(angles);
}

float
vectoyaw_rogue(vec3_t vec) /* FS: Coop: Rogue specific */
{
	float yaw;

	if (vec[PITCH] == 0)
	{
		if (vec[YAW] == 0)
		{
			yaw = 0;
		}
		else if (vec[YAW] > 0)
		{
			yaw = 90;
		}
		else
		{
			yaw = 270;
		}
	}
	else
	{
		yaw = (int)(atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);

		if (yaw < 0)
		{
			yaw += 360;
		}
	}

	return yaw;
}

float
vectoyaw(vec3_t vec)
{
	float yaw;

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		yaw = vectoyaw_rogue(vec);
		return yaw;
	}

	if (vec[PITCH] == 0)
	{
		yaw = 0;

		if (vec[YAW] > 0)
		{
			yaw = 90;
		}
		else if (vec[YAW] < 0)
		{
			yaw = -90;
		}
	}
	else
	{
		yaw = (int)(atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);

		if (yaw < 0)
		{
			yaw += 360;
		}
	}

	return yaw;
}

float
vectoyaw2(vec3_t vec) /* FS: Coop: Rogue specific */
{
	float yaw;

	if (vec[PITCH] == 0)
	{
		if (vec[YAW] == 0)
		{
			yaw = 0;
		}
		else if (vec[YAW] > 0)
		{
			yaw = 90;
		}
		else
		{
			yaw = 270;
		}
	}
	else
	{
		yaw = (atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);

		if (yaw < 0)
		{
			yaw += 360;
		}
	}

	return yaw;
}

void
vectoangles(vec3_t value1, vec3_t angles)
{
	float forward;
	float yaw, pitch;

	if ((value1[1] == 0) && (value1[0] == 0))
	{
		yaw = 0;

		if (value1[2] > 0)
		{
			pitch = 90;
		}
		else
		{
			pitch = 270;
		}
	}
	else
	{
		if (value1[0])
		{
			yaw = (int)(atan2(value1[1], value1[0]) * 180 / M_PI);
		}
		else if (value1[1] > 0)
		{
			yaw = 90;
		}
		else
		{
			if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
			{
				yaw = 270;
			}
			else
			{
				yaw = -90;
			}
		}

		if (yaw < 0)
		{
			yaw += 360;
		}

		forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
		pitch = (int)(atan2(value1[2], forward) * 180 / M_PI);

		if (pitch < 0)
		{
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

#ifndef GAME_HARD_LINKED
/* vectoangles2() is duplicated at the engine side in client/cl_newfx.c
 * remember that if you ever attempt to change it!!
 */
void
vectoangles2(vec3_t value1, vec3_t angles) /* FS: Coop: Rogue specific */
{
	float forward;
	float yaw, pitch;

	if ((value1[1] == 0) && (value1[0] == 0))
	{
		yaw = 0;

		if (value1[2] > 0)
		{
			pitch = 90;
		}
		else
		{
			pitch = 270;
		}
	}
	else
	{
	/* PMM - fixed to correct for pitch of 0 */
		if (value1[0])
		{
			yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
		}
		else if (value1[1] > 0)
		{
			yaw = 90;
		}
		else
		{
			yaw = 270;
		}

		if (yaw < 0)
		{
			yaw += 360;
		}

		forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
		pitch = (atan2(value1[2], forward) * 180 / M_PI);

		if (pitch < 0)
		{
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}
#endif /* !GAME_HARD_LINKED */

char *
G_CopyString(char *in)
{
	char *out;

	if (!in)
	{
		return NULL;
	}

	out = gi.TagMalloc(strlen(in) + 1, TAG_LEVEL);
	strcpy(out, in);
	return out;
}

void
G_InitEdict(edict_t *e)
{
	if (!e)
	{
		return;
	}

	if ((game.gametype == rogue_coop) && (e->nextthink)) /* FS: Coop: Rogue specific.  Possible fix/workaround? */
	{
		e->nextthink = 0;
	}

	e->inuse = true;
	e->classname = "noclass";
	e->gravity = 1.0;
	e->s.number = e - g_edicts;

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		e->gravityVector[0] = 0.0;
		e->gravityVector[1] = 0.0;
		e->gravityVector[2] = -1.0;
	}
}

/*
 * Either finds a free edict, or allocates a new one.
 * Try to avoid reusing an entity that was recently freed,
 * because it can cause the client to think the entity
 * morphed into something else instead of being removed
 * and recreated, which can cause interpolated angles and
 * bad trails.
 */
edict_t *
G_Spawn(void)
{
	int i;
	edict_t *e;

	e = &g_edicts[maxclients->intValue + 1];

	for (i = maxclients->intValue + 1; i < globals.num_edicts; i++, e++)
	{
		/* the first couple seconds of server time can involve a lot of
		   freeing and allocating, so relax the replacement policy */
		if (!e->inuse &&
			((e->freetime < 2) || (level.time - e->freetime > 0.5)))
		{
			G_InitEdict(e);
			return e;
		}
	}

	if (i == game.maxentities)
	{
		gi.error("ED_Alloc: no free edicts");
	}

	globals.num_edicts++;
	G_InitEdict(e);
	return e;
}

/*
 * Marks the edict as free
 */
void
G_FreeEdict(edict_t *ed)
{

	if (!ed)
	{
		return;
	}

	gi.unlinkentity(ed); /* unlink from world */

	if (deathmatch->intValue || coop->intValue)
	{
		if ((ed - g_edicts) <= (maxclients->intValue + BODY_QUEUE_SIZE))
		{
			return;
		}
	}
	else
	{
		if ((ed - g_edicts) <= maxclients->intValue)
		{
			return;
		}
	}

	memset(ed, 0, sizeof(*ed));
	ed->classname = "freed";
	ed->freetime = level.time;
	ed->inuse = false;
	ed->nextthink = 0;    // just in case freed before a nextthink... /* FS: Zaero specific game dll changes */
}

void
G_TouchTriggers(edict_t *ent)
{
	int i, num;
	edict_t *touch[MAX_EDICTS], *hit;

	if (!ent)
	{
		return;
	}

	/* dead things don't activate triggers! */
	if ((ent->client || (ent->svflags & SVF_MONSTER)) && (ent->health <= 0))
	{
		return;
	}

	num = gi.BoxEdicts(ent->absmin, ent->absmax, touch,
			MAX_EDICTS, AREA_TRIGGERS);

	/* be careful, it is possible to have an entity in this
	   list removed before we get to it (killtriggered) */
	for (i = 0; i < num; i++)
	{
		hit = touch[i];

		if (!hit->inuse)
		{
			continue;
		}

		if (!hit->touch)
		{
			continue;
		}

		hit->touch(hit, ent, NULL, NULL);
	}
}

/*
 * Call after linking a new trigger
 * in during gameplay to force all
 * entities it covers to immediately
 * touch it
 */
void
G_TouchSolids(edict_t *ent)
{
	int i, num;
	edict_t *touch[MAX_EDICTS], *hit;

	if (!ent)
	{
		return;
	}

	num = gi.BoxEdicts(ent->absmin, ent->absmax, touch,
			MAX_EDICTS, AREA_SOLID);

	/* be careful, it is possible to have an entity in this
	   list removed before we get to it (killtriggered) */
	for (i = 0; i < num; i++)
	{
		hit = touch[i];

		if (!hit->inuse)
		{
			continue;
		}

		if (ent->touch)
		{
			ent->touch(hit, ent, NULL, NULL);
		}

		if (!ent->inuse)
		{
			break;
		}
	}
}

/*
 * Kills all entities that would touch the
 * proposed new positioning of ent. Ent s
 * hould be unlinked before calling this!
 */
qboolean
KillBox(edict_t *ent)
{
	trace_t tr;

	if (!ent)
	{
		return false;
	}

	while (1)
	{
		tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, ent->s.origin,
				NULL, MASK_PLAYERSOLID);

		if (!tr.ent)
		{
			break;
		}

		/* nail it */
		T_Damage(tr.ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin,
				100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);

		/* if we didn't kill it, fail */
		if (tr.ent->solid)
		{
			return false;
		}
	}

	return true; /* all clear */
}

/*
=================
MonsterKillBox

Kills all entities except players that would touch the proposed new 
positioning of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean MonsterKillBox (edict_t *ent) /* FS: Zaero specific game dll changes */
{
	trace_t		tr;

	while (1)
	{
		tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, NULL, MASK_PLAYERSOLID);
		if (!tr.ent)
			break;

    if(!((ent->svflags & SVF_MONSTER) && tr.ent->client && tr.ent->health))
    {
		  // nail it
		  T_Damage (tr.ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
    }

		// if we didn't kill it, fail
		if (tr.ent->solid)
			return false;
	}

	return true;		// all clear
}

/*
=================
MonsterPlayerKillBox

Kills all entities except players that would touch the proposed new 
positioning of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean MonsterPlayerKillBox (edict_t *ent) /* FS: Zaero specific game dll changes */
{
	trace_t		tr;

	while (1)
	{
		tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, ent, MASK_PLAYERSOLID);
		if (!tr.ent)
			break;

    if((ent->svflags & SVF_MONSTER) && tr.ent->client && tr.ent->health)
    {
		  // nail myself
		  T_Damage (ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
      return true;
    }
    else
    {
		  // nail it
		  T_Damage (tr.ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
    }

		// if we didn't kill it, fail
		if (tr.ent->solid)
			return false;
	}

	return true;		// all clear
}

edict_t *Find_LikePlayer (edict_t *ent, char *name, qboolean exactMatch) /* FS: People want this for various Tastyspleen-like commands */
{
	int i, count;
	edict_t *player = NULL;
	edict_t *foundPlayer = NULL;
	char *nameLwrd = NULL;

	if (!ent || !ent->client || !name || !name[0])
	{
		return NULL;
	}

	if (!strncmp(name, "LIKE ", 5))
	{
		name += 5;
		if (!name || !name[0])
		{
			gi.cprintf(ent, PRINT_HIGH, "No name specified after LIKE.  Aborting search!\n");
			return NULL;
		}
	}

	i = count = 0;

	nameLwrd = Q_strlwr(name);
	if (!nameLwrd)
	{
		gi.cprintf(ent, PRINT_HIGH, "Name is NULL.  Aborting search!\n");
		return NULL;
	}

	for (i = 0; i < maxclients->intValue; i++)
	{
		char *netName;
		player = &g_edicts[i + 1];

		if (!player || !player->inuse || !player->client)
		{
			continue;
		}

		netName = strdup(player->client->pers.netname);

		netName = Q_strlwr(netName);

		if (exactMatch)
		{
			if (!Q_stricmp(netName, nameLwrd))
			{
				foundPlayer = player;
				count++;
			}
		}
		else
		{
			if (netName && strstr(netName, nameLwrd))
			{
				foundPlayer = player;
				count++;
			}
		}

		if (netName)
		{
			free(netName);
			netName = NULL;
		}
	}

	if (count > 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "2 or more player name matches.\n");
		return NULL;
	}

	if (count)
	{
		return foundPlayer;
	}

	gi.cprintf(ent, PRINT_HIGH, "no player name matches found.\n");
	return NULL;
}

/* FS: This is needed so trouble makers can't spam dropping ammo on respawn.
 *     If that happens nobody can connect.
 *     So make sure we have at least 256 free entities.  Should be enough room
 *     and not too many people use this in coop anyways because of the shared
 *     inventory.
 */
qboolean G_SpawnCheck(int cap)
{
	int i;
	edict_t *e;

	e = &g_edicts[maxclients->intValue + 1];

	if (!cap)
		cap = 1;

	for (i = maxclients->intValue + 1; i < globals.num_edicts; i++, e++)
	{
		/* the first couple seconds of server time can involve a lot of
		   freeing and allocating, so relax the replacement policy */
		if (!e->inuse &&
			((e->freetime < 2) || (level.time - e->freetime > 0.5))
			&& (i <= game.maxentities - cap))
		{
//			Com_Printf("%d / %d / %d\n", i, game.maxentities-cap, game.maxentities);
			return true;
		}
	}

//	Com_Printf("%d / %d / %d\n", i, game.maxentities-cap, game.maxentities);
	if (i >= game.maxentities - cap)
	{
		return false;
	}

	return true;
}
