#include "g_local.h"

/*
======================================================================

INTERMISSION

======================================================================
*/

void
MoveClientToIntermission(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if (deathmatch->intValue || coop->intValue)
	{
		ent->client->showscores = true;
	}

	VectorCopy(level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0] * 8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1] * 8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2] * 8;
	VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	/* clean up powerup info */
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	/* FS: Coop: Rogue specific */
	ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
	ent->client->ir_framenum = 0;
	ent->client->nuke_framenum = 0;
	ent->client->double_framenum = 0;

	/* FS: Coop: Xatrix specific */
	ent->client->quadfire_framenum = 0;
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex4 = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	gi.linkentity(ent);

	/* add the layout */
	if (deathmatch->intValue || coop->intValue)
	{
		DeathmatchScoreboardMessage(ent, NULL);
		gi.unicast(ent, true);
	}
}

void
BeginIntermission(edict_t *targ)
{
	int i, n;
	edict_t *ent, *client;

	if (!targ)
	{
		return;
	}

	if (level.intermissiontime)
	{
		return; /* already activated */
	}

	game.autosaved = false;

	/* respawn any dead clients */
	for (i = 0; i < maxclients->intValue; i++)
	{
		client = g_edicts + 1 + i;

		if (!client->inuse)
		{
			continue;
		}

		if (client->health <= 0)
		{
			respawn(client);
		}
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (game.gametype == zaero_coop) /* FS: Zaero specific game dll changes */
	{
		if (Q_stricmp(level.mapname, "zboss") == 0 && !deathmatch->intValue)
		{
			level.fadeFrames = 50;
		}
	}

	if (level.changemap && strchr(level.changemap, '*'))
	{
		if (coop->intValue)
		{
			for (i = 0; i < maxclients->intValue; i++)
			{
				client = g_edicts + 1 + i;

				if (!client->inuse)
				{
					continue;
				}

				/* strip players of all keys between units */
				for (n = 0; n < game.num_items; n++)
				{
					if (itemlist[n].flags & IT_KEY)
					{
						client->client->pers.inventory[n] = 0;
					}
				}

				client->client->pers.power_cubes = 0;
			}
		}
	}
	else
	{
		if (!deathmatch->intValue)
		{
			level.exitintermission = 1; /* go immediately to the next level */
			return;
		}
	}

	level.exitintermission = 0;

	/* find an intermission spot */
	ent = G_Find(NULL, FOFS(classname), "info_player_intermission");

	if (!ent)
	{
		/* the map creator forgot to put in an intermission point... */
		ent = G_Find(NULL, FOFS(classname), "info_player_start");

		if (!ent)
		{
			ent = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
		}
	}
	else
	{
		/* chose one of four spots */
		i = rand() & 3;

		while (i--)
		{
			ent = G_Find(ent, FOFS(classname), "info_player_intermission");

			if (!ent) /* wrap around the list */
			{
				ent = G_Find(ent, FOFS(classname), "info_player_intermission");
			}
		}
	}

	VectorCopy(ent->s.origin, level.intermission_origin);
	VectorCopy(ent->s.angles, level.intermission_angle);

	/* move all clients to the intermission point */
	for (i = 0; i < maxclients->intValue; i++)
	{
		client = g_edicts + 1 + i;

		if (!client->inuse)
		{
			continue;
		}

		MoveClientToIntermission(client);
	}
}

void
DeathmatchScoreboardMessage(edict_t *ent, edict_t *killer /* can be NULL */)
{
	char entry[LAYOUT_MAX_LENGTH];
	char string[LAYOUT_MAX_LENGTH];
	int stringlength;
	int i, j, k;
	int sorted[MAX_CLIENTS];
	int sortedscores[MAX_CLIENTS];
	int score, total;
	int x, y;
	gclient_t *cl;
	edict_t *cl_ent;
	char *tag;

	if (!ent)
	{
		return;
	}

	/* sort the clients by score */
	total = 0;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;

		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		score = game.clients[i].resp.score;

		for (j = 0; j < total; j++)
		{
			if (score > sortedscores[j])
			{
				break;
			}
		}

		for (k = total; k > j; k--)
		{
			sorted[k] = sorted[k - 1];
			sortedscores[k] = sortedscores[k - 1];
		}

		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	/* print level name and exit rules */
	string[0] = 0;

	stringlength = strlen(string);

	/* add the clients in sorted order */
	if (total > 12)
	{
		total = 12;
	}

	for (i = 0; i < total; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		x = (i >= 6) ? 160 : 0;
		y = 32 + 32 * (i % 6);

		/* add a dogtag */
		if (cl_ent == ent)
		{
			tag = "tag1";
		}
		else if (cl_ent == killer)
		{
			tag = "tag2";
		}
		else
		{
			tag = NULL;
		}

		/* FS: Coop: Xatrix specific */
		/* allow new DM games to override the tag picture */
		if (gamerules && gamerules->intValue)
		{
			if (DMGame.DogTag)
			{
				DMGame.DogTag(cl_ent, killer, &tag);
			}
		}

		if (tag)
		{
			Com_sprintf(entry, sizeof(entry), "xv %i yv %i picn %s ", x + 32, y, tag);
			j = strlen(entry);

			if (stringlength + j > 1024)
			{
				break;
			}

			strcpy(string + stringlength, entry);
			stringlength += j;
		}

		/* send the layout */
		Com_sprintf(entry, sizeof(entry), "client %i %i %i %i %i %i ",
				x, y, sorted[i], cl->resp.score, cl->ping,
				(level.framenum - cl->resp.enterframe) / 600);
		j = strlen(entry);

		if (stringlength + j > 1024)
		{
			break;
		}

		strcpy(string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent) /* FS: Coop: Xatrix */
{
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}

/*
 * Draw help computer.
 */
void
HelpComputerMessage(edict_t *ent)
{
	char string[1024];
	char *sk;

	if (!ent)
	{
		return;
	}

	if (skill->intValue == 0)
	{
		sk = "easy";
	}
	else if (skill->intValue == 1)
	{
		sk = "medium";
	}
	else if (skill->intValue == 2)
	{
		sk = "hard";
	}
	else
	{
		sk = "hard+";
	}

	/* send the layout */
	Com_sprintf(string, sizeof(string),
			"xv 32 yv 8 picn help "			/* background */
			"xv 202 yv 12 string2 \"%s\" "  /* skill */
			"xv 0 yv 24 cstring2 \"%s\" "   /* level name */
			"xv 0 yv 54 cstring2 \"%s\" "   /* help 1 */
			"xv 0 yv 110 cstring2 \"%s\" "  /* help 2 */
			"xv 50 yv 164 string2 \" kills     goals    secrets\" "
			"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ",
			sk, level.level_name,
			game.helpmessage1,
			game.helpmessage2,
			level.killed_monsters, level.total_monsters,
			level.found_goals, level.total_goals,
			level.found_secrets, level.total_secrets);

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast (ent, true); /* FS: Don't remove this DOS needs this! */
}

/*
 * Display the current help message
 */
void
InventoryMessage(edict_t *ent)
{
	int i;

	if (!ent)
	{
		return;
	}

	gi.WriteByte(svc_inventory);

	for (i = 0; i < MAX_ITEMS; i++)
	{
		gitem_t *it = &itemlist[i]; /* FS: Zaero specific game dll changes */

		if ((game.gametype == zaero_coop) && (it->hideFlags & HIDE_FROM_INVENTORY))
		{
			gi.WriteShort(0);	// this is a hack and will work as long as
								// the client continues to hide items that
								// the user has none of
		}
		else
		{
			gi.WriteShort(ent->client->pers.inventory[i]);
		}
	}
}

/* ======================================================================= */

void
G_SetStats(edict_t *ent)
{
	gitem_t *item;
	int index, cells = 0;
	int power_armor_type;

	if (!ent)
	{
		return;
	}

	/* health */
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	/* ammo */
	if (!ent->client->ammo_index)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(item->icon);
		ent->client->ps.stats[STAT_AMMO] =
			ent->client->pers.inventory[ent->client->ammo_index];
	}

	/* armor */
	power_armor_type = PowerArmorType(ent);

	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))];

		if (cells == 0)
		{
			/* ran out of cells for power armor */
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex( "misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;
		}
	}

	index = ArmorIndex(ent);

	if (power_armor_type && (!index || (level.framenum & 8)))
	{
		/* flash between power armor and other armor icon */
		/* Knightmare- show correct icon for power screen */
		if (power_armor_type == POWER_ARMOR_SHIELD)
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex("i_powershield");
		else
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powerscreen");
		/* end Knightmare */
		ent->client->ps.stats[STAT_ARMOR] = cells;
	}
	else if (index)
	{
		item = GetItemByIndex(index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex(item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	/* pickup message */
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	/* timers */
	if (ent->client->a2kFramenum > level.framenum) /* FS: Zaero specific game dll changes */
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("w_a2k");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->a2kFramenum - level.framenum)/10;
	}
	else if (ent->client->sniperFramenum >= level.framenum) /* FS: Zaero specific game dll changes */
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("w_sniper");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->sniperFramenum - level.framenum)/10;
	}
	else if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum) / 10;
	}
	else if (ent->client->double_framenum > level.framenum) /* FS: Coop: Rogue specific */
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_double");
		ent->client->ps.stats[STAT_TIMER] =
			(ent->client->double_framenum - level.framenum) / 10;
	}
	else if (ent->client->quadfire_framenum > level.framenum) /* FS: Coop: Xatrix specific */
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_quadfire");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quadfire_framenum
			   	- level.framenum) / 10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex( "p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum) / 10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum) / 10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum) / 10;
	}
	else if (ent->client->owned_sphere) /* FS: Coop: Rogue specific */
	{
		if (ent->client->owned_sphere->spawnflags == 1) /* defender */
		{
			ent->client->ps.stats[STAT_TIMER_ICON] =
				gi.imageindex("p_defender");
		}
		else if (ent->client->owned_sphere->spawnflags == 2) /* hunter */
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_hunter");
		}
		else if (ent->client->owned_sphere->spawnflags == 4) /* vengeance */
		{
			ent->client->ps.stats[STAT_TIMER_ICON] =
				gi.imageindex("p_vengeance");
		}
		else /* error case */
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("i_fixme");
		}

		ent->client->ps.stats[STAT_TIMER] =
			(int)(ent->client->owned_sphere->wait - level.time);
	}
	else if (ent->client->ir_framenum > level.framenum) /* FS: Coop: Rogue specific */
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_ir");
		ent->client->ps.stats[STAT_TIMER] =
			(ent->client->ir_framenum - level.framenum) / 10;
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

	/* selected item */
	if (ent->client->pers.selected_item == -1)
	{
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	}
	else
	{
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex(itemlist[ent->client->pers.selected_item].icon);
	}

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	/* layouts */
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->intValue)
	{
		if ((ent->client->pers.health <= 0) || level.intermissiontime || ent->client->showscores)
		{
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		}

		if (ent->client->showinventory && (ent->client->pers.health > 0))
		{
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
		}
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
		{
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		}

		if (ent->client->showinventory && (ent->client->pers.health > 0))
		{
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
		}
	}

	/* frags */
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	/* help icon / current weapon if not shown */
	if (ent->client->pers.helpchanged && (level.framenum & 8))
	{
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex("i_help");
	}
	else if (((ent->client->pers.hand == CENTER_HANDED) ||
			  (ent->client->ps.fov > 91)) && ent->client->pers.weapon)
	{
		cvar_t *gun;
		gun = gi.cvar("cl_gun", "2", 0);

		if (gun->intValue != 2)
		{
			ent->client->ps.stats[STAT_HELPICON] = gi.imageindex(ent->client->pers.weapon->icon);
		}
		else
		{
			ent->client->ps.stats[STAT_HELPICON] = 0;
		}
	}
	else
	{
		ent->client->ps.stats[STAT_HELPICON] = 0;
	}

	// origin
	if (ent->client->showOrigin) /* FS: Zaero specific game dll changes: special debug command */
	{
		ent->client->ps.stats[STAT_SHOW_ORIGIN] = 1;
		ent->client->ps.stats[STAT_ORIGIN_X] = (int)ent->s.origin[0];
		ent->client->ps.stats[STAT_ORIGIN_Y] = (int)ent->s.origin[1];
		ent->client->ps.stats[STAT_ORIGIN_Z] = (int)ent->s.origin[2];
	}
	else
	{
		ent->client->ps.stats[STAT_SHOW_ORIGIN] = 0;
	}

	if (ent->client->zCameraTrack)  /* FS: Zaero specific game dll changes */
	{
		ent->client->ps.stats[STAT_CAMERA_ICON] = gi.imageindex("i_visor");
		ent->client->ps.stats[STAT_CAMERA_TIMER] = ent->client->pers.visorFrames/10;
	}
	else
	{
		ent->client->ps.stats[STAT_CAMERA_ICON] = 0;
	}

	ent->client->ps.stats[STAT_SPECTATOR] = 0;
}

void
G_CheckChaseStats(edict_t *ent)
{
	int i;
	gclient_t *cl;

	if (!ent)
	{
		return;
	}

	for (i = 1; i <= maxclients->intValue; i++)
	{
		cl = g_edicts[i].client;

		if (!g_edicts[i].inuse || (cl->chase_target != ent))
		{
			continue;
		}

		memcpy(cl->ps.stats, ent->client->ps.stats, sizeof(cl->ps.stats));
		G_SetSpectatorStats(g_edicts + i);
	}
}

void
G_SetSpectatorStats(edict_t *ent)
{
	gclient_t *cl;

	if (!ent)
	{
		return;
	}

	cl = ent->client;

	if (!cl->chase_target)
	{
		G_SetStats(ent);
	}

	cl->ps.stats[STAT_SPECTATOR] = 1;

	/* layouts are independant in spectator */
	cl->ps.stats[STAT_LAYOUTS] = 0;

	if ((cl->pers.health <= 0) || level.intermissiontime || cl->showscores)
	{
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	}

	if (cl->showinventory && (cl->pers.health > 0))
	{
		cl->ps.stats[STAT_LAYOUTS] |= 2;
	}

	if (cl->chase_target && cl->chase_target->inuse)
	{
		cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS +
								   (cl->chase_target - g_edicts) - 1;
	}
	else
	{
		cl->ps.stats[STAT_CHASE] = 0;
	}
}
