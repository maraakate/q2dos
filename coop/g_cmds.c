#include "g_local.h"
#include "p_hook.h"
#include "m_player.h"
#include "flashlight.h"

extern void SP_info_coop_checkpoint (edict_t * self );
extern void stopCamera(edict_t *self); /* FS: Zaero specific game dll changes */
extern qboolean bVoteInProgress;

static qboolean Client_CanCheat (edict_t *self) /* FS: Added */
{
	if (!self || !self->client)
	{
		return false;
	}

	if (self->client->pers.isAdmin || self->client->pers.isVIP) /* FS: Admins and VIPS can cheat all they want >:D */
	{
		return true;
	}

	if (sv_cheats->intValue) /* FS: Otherwise, we need sv_cheats enabled for normal players. */
	{
		return true;
	}

	return false;
}

static char *
ClientTeam(edict_t *ent, char* value)
{
	char *p;

	value[0] = 0;

	if (!ent)
	{
		return value;
	}

	if (!ent->client)
	{
		return value;
	}

	strcpy(value, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');

	if (!p)
	{
		return value;
	}

	if (dmflags->intValue & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	return ++p;
}

qboolean
OnSameTeam(edict_t *ent1, edict_t *ent2)
{
	char ent1Team[512];
	char ent2Team[512];

	if (!ent1 || !ent2)
	{
		return false;
	}

	if(coop->intValue) /* FS: Coop: No friendly fire bullshit */
	{
		if(ent1->client && ent2->client)
		{
			return true;
		}
	}

	if (!(dmflags->intValue & (DF_MODELTEAMS | DF_SKINTEAMS)))
	{
		return false;
	}

	ClientTeam(ent1, ent1Team);
	ClientTeam(ent2, ent2Team);

	if (ent1Team[0] != '\0' && strcmp(ent1Team, ent2Team) == 0)
	{
		return true;
	}

	return false;
}

void
SelectNextItem(edict_t *ent, int itflags)
{
	gclient_t *cl;
	int i, index;
	gitem_t *it;

	if (!ent)
	{
		return;
	}

	cl = ent->client;

	if (cl->menu)
	{
		PMenu_Next(ent);
		return;
	}
	else if (cl->chase_target)
	{
		ChaseNext(ent);
		return;
	}

	/* scan  for the next valid one */
	for (i = 1; i <= MAX_ITEMS; i++)
	{
		index = (cl->pers.selected_item + i) % MAX_ITEMS;

		if (!cl->pers.inventory[index])
		{
			continue;
		}

		it = &itemlist[index];

		if ((game.gametype == zaero_coop) && (it->hideFlags & HIDE_FROM_INVENTORY)) // don't show this /* FS: Zaero specific game dll changes */
		{
			continue;
		}

		if (!it->use)
		{
			continue;
		}

		if (!(it->flags & itflags))
		{
			continue;
		}

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void
SelectPrevItem(edict_t *ent, int itflags)
{
	gclient_t *cl;
	int i, index;
	gitem_t *it;

	if (!ent)
	{
		return;
	}

	cl = ent->client;

	if (cl->menu)
	{
		PMenu_Prev(ent);
		return;
	}
	else if (cl->chase_target)
	{
		ChasePrev(ent);
		return;
	}

	/* scan  for the next valid one */
	for (i = 1; i <= MAX_ITEMS; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i) % MAX_ITEMS;

		if (!cl->pers.inventory[index])
		{
			continue;
		}

		it = &itemlist[index];

		if ((game.gametype == zaero_coop) && (it->hideFlags & HIDE_FROM_INVENTORY)) /* FS: Zaero specific game dll changes */
		{
			continue;
		}

		if (!it->use)
		{
			continue;
		}

		if (!(it->flags & itflags))
		{
			continue;
		}

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void
ValidateSelectedItem(edict_t *ent)
{
	gclient_t *cl;

	if (!ent)
	{
		return;
	}

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
	{
		return; /* valid */
	}

	SelectNextItem(ent, -1);
}

qboolean
Check_Coop_Item_Flags (gitem_t *it)
{
	if (!it)
	{
		return false;
	}

	switch(game.gametype) /* FS: Check our game modes and don't add what we aren't supposed to have. */
	{
		default:
		case vanilla_coop:
			if (it->flags & IT_XATRIX)
			{
				return false;
			}
			if (it->flags & IT_ROGUE)
			{
				return false;
			}
			if (it->flags & IT_ZAERO)
			{
				return false;
			}
			break;
		case xatrix_coop:
			if (it->flags & IT_ROGUE)
			{
				return false;
			}
			if (it->flags & IT_ZAERO)
			{
				return false;
			}
			break;
		case rogue_coop:
			if (it->flags & IT_XATRIX)
			{
				return false;
			}
			if (it->flags & IT_ZAERO)
			{
				return false;
			}
			break;
		case zaero_coop:
			if (it->flags & IT_XATRIX)
			{
				return false;
			}
			if (it->flags & IT_ROGUE)
			{
				return false;
			}
			break;
	}

	return true;
}

/* ================================================================================= */

/*
 * Give items to a client
 */
void
Cmd_Give_f(edict_t *ent)
{
	char *name;
	gitem_t *it;
	int index;
	int i;
	qboolean give_all;
	edict_t *it_ent;

	if (!ent || !ent->client)
	{
		return;
	}

	if ((deathmatch->intValue || coop->intValue) && !Client_CanCheat(ent))
	{
		gi.cprintf(ent, PRINT_HIGH,
				"You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if(ent->client->pers.spectator || ent->client->blinky_client.cam_target) /* FS: Coop: No spectators and cheats, please */
	{
		gi.cprintf(ent, PRINT_HIGH,
				"Spectators can't use this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
	{
		give_all = true;
	}
	else
	{
		give_all = false;
	}

	if (give_all || (Q_stricmp(gi.argv(1), "health") == 0))
	{
		if (gi.argc() == 3)
		{
			ent->health = atoi(gi.argv(2));
		}
		else
		{
			ent->health = ent->max_health;
		}

		if (!give_all)
		{
			return;
		}
	}

	if (give_all || (Q_stricmp(name, "weapons") == 0))
	{
		for (i = 0; i < game.num_items; i++)
		{
			it = itemlist + i;

			if (!it->pickup)
			{
				continue;
			}

			if (!(it->flags & IT_WEAPON))
			{
				continue;
			}

			if(!Check_Coop_Item_Flags(it)) /* FS: Coop: Only give stuff we're supposed to have in each gamemode */
			{
				continue;
			}

			ent->client->pers.inventory[i] += 1;
		}

		if (!give_all)
		{
			return;
		}
	}

	if (give_all || (Q_stricmp(name, "ammo") == 0))
	{
		for (i = 0; i < game.num_items; i++)
		{
			it = itemlist + i;

			if (!it->pickup)
			{
				continue;
			}

			if (!(it->flags & IT_AMMO))
			{
				continue;
			}

			if(!Check_Coop_Item_Flags(it)) /* FS: Coop: Only give stuff we're supposed to have in each gamemode */
			{
				continue;
			}

			Add_Ammo(ent, it, 1000);
		}

		if (!give_all)
		{
			return;
		}
	}

	if (give_all || (Q_stricmp(name, "armor") == 0))
	{
		gitem_armor_t *info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
		{
			return;
		}
	}

	if ((game.gametype == zaero_coop) && (give_all || Q_stricmp(name, "Visor") == 0)) /* FS: Zaero specific game dll changes */
	{
		it = FindItem("Visor");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);

		if (it_ent->inuse)
		{
			G_FreeEdict(it_ent);
		}

		if (!give_all)
		{
			return;
		}
	}

	if (give_all || (Q_stricmp(name, "Power Shield") == 0))
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem(it_ent, it);
		Touch_Item(it_ent, ent, NULL, NULL);

		if (it_ent->inuse)
		{
			G_FreeEdict(it_ent);
		}

		if (!give_all)
		{
			return;
		}
	}

	if (give_all)
	{
		for (i = 0; i < game.num_items; i++)
		{
			it = itemlist + i;

			if (!it->pickup)
			{
				continue;
			}

			if ((game.gametype == rogue_coop) && (it->flags & IT_NOT_GIVEABLE)) /* FS: Coop: Rogue specific */
			{
				if(!ent->client->pers.isAdmin || !ent->client->pers.isVIP) /* FS: Coop: Admin and VIP goodies */
				{
					continue;
				}
			}

			if (it->flags & (IT_ARMOR | IT_WEAPON | IT_AMMO))
			{
				continue;
			}

			if(!Check_Coop_Item_Flags(it)) /* FS: Coop: Only give stuff we're supposed to have in each gamemode */
			{
				continue;
			}

			ent->client->pers.inventory[i] = 1;
		}

		return;
	}

	it = FindItem(name);

	if (!it)
	{
		name = gi.argv(1);
		it = FindItem(name);

		if (!it)
		{
			gi.cprintf(ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf(ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		if ((it->flags & IT_NOT_GIVEABLE) && (!ent->client->pers.isAdmin || !ent->client->pers.isVIP)) /* FS: Coop: Admin and VIP goodies */
		{
			gi.cprintf(ent, PRINT_HIGH, "item cannot be given\n"); /* FS: Was Dprintf */
			return;
		}
	}

	if(!Check_Coop_Item_Flags(it)) /* FS: Coop: Only give stuff we're supposed to have in each gamemode */
	{
		gi.cprintf(ent, PRINT_HIGH, "item cannot be given in this gamemode\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
		{
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		}
		else
		{
			ent->client->pers.inventory[index] += it->quantity;
		}
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem(it_ent, it);

		/* since some items don't actually spawn when you say to .. */
		if ((game.gametype == rogue_coop) && (!it_ent->inuse)) /* FS: Coop: Rogue specific */
		{
			return;
		}

		Touch_Item(it_ent, ent, NULL, NULL);

		if (it_ent->inuse)
		{
			G_FreeEdict(it_ent);
		}
	}
}

/*
 * Sets client to godmode
 */
void
Cmd_God_f(edict_t *ent)
{
	char *msg;

	if (!ent || !ent->client)
	{
		return;
	}

	if ((deathmatch->intValue || coop->intValue) && !Client_CanCheat(ent))
	{
		gi.cprintf(ent, PRINT_HIGH,
				"You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if(ent->client->pers.spectator || ent->client->blinky_client.cam_target) /* FS: Coop: No spectators and cheats, please */
	{
		gi.cprintf(ent, PRINT_HIGH,
				"Spectators can't use this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;

	if (!(ent->flags & FL_GODMODE))
	{
		msg = "godmode OFF\n";
	}
	else
	{
		msg = "godmode ON\n";
	}

	gi.cprintf(ent, PRINT_HIGH, msg);
}

/*
 * Sets client to notarget
 */
void
Cmd_Notarget_f(edict_t *ent)
{
	char *msg;

	if (!ent || !ent->client)
	{
		return;
	}

	if ((deathmatch->intValue || coop->intValue) && !Client_CanCheat(ent))
	{
		gi.cprintf(ent, PRINT_HIGH,
				"You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if(ent->client->pers.spectator || ent->client->blinky_client.cam_target) /* FS: Coop: No spectators and cheats, please */
	{
		gi.cprintf(ent, PRINT_HIGH,
				"Spectators can't use this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;

	if (!(ent->flags & FL_NOTARGET))
	{
		msg = "notarget OFF\n";
	}
	else
	{
		msg = "notarget ON\n";
	}

	gi.cprintf(ent, PRINT_HIGH, msg);
}

void
Cmd_Noclip_f(edict_t *ent)
{
	char *msg;

	if (!ent || !ent->client)
	{
		return;
	}

	if ((deathmatch->intValue || coop->intValue) && !Client_CanCheat(ent))
	{
		gi.cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if(ent->client->pers.spectator || ent->client->blinky_client.cam_target) /* FS: Coop: No spectators and cheats, please */
	{
		gi.cprintf(ent, PRINT_HIGH, "Spectators can't use this command.\n");
		return;
	}

	if (ent->deadflag) /* FS: Apparently this can screw up movetype occasionally if you do this */
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't use this command when dead!\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf(ent, PRINT_HIGH, msg);
}

 /* FS: Zaero specific game dll changes */
#define MAX_ALT 2
struct altsel_s
{
	int num;
	char *weapon[MAX_ALT];
} alternates[] = 
{
	{0}, // filler
	{2,{"Blaster", "Flare Gun"}},
	{1,{"Shotgun"}},
	{1,{"Super Shotgun"}},
	{1,{"Machinegun"}},
	{1,{"Chaingun"}},
	{1,{"Grenade Launcher"}},
	{1,{"Rocket Launcher"}},
	{1,{"HyperBlaster"}},
	{2,{"Railgun", "Sniper Rifle"}},
	{2,{"BFG10K", "Sonic Cannon"}}
};

qboolean tryUse(edict_t *ent, char *s) /* FS: Zaero specific game dll changes */
{
	int index = 0;
	gitem_t *it = FindItem(s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return false;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return false;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return false;
	}
	it->use (ent, it);
	return true;
}

void findNext(edict_t *ent, struct altsel_s *ptr, int offset) /* FS: Zaero specific game dll changes */
{
	int start = offset;

	while (1)
	{
		if (tryUse(ent, ptr->weapon[offset]))
			break;
		
		offset++;
		// wrap around
		if (offset >= ptr->num)
			offset = 0;
		// back where we began?
		if (offset == start)
			break;
	}
}

void altSelect(edict_t *ent, int num) /* FS: Zaero specific game dll changes */
{
	int offset = -1;
	int i = 0;
	struct altsel_s *ptr = NULL;
	gitem_t *it = NULL;
	
	// within range?
	if (num < 1 || num > 10)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid weapon index: %i\n", num);
		return;
	}

	ptr = &alternates[num];

	// is our current weapon in this list?
	for (i = 0; i < ptr->num; i++)
	{
		it = FindItem(ptr->weapon[i]);
		if (it == ent->client->pers.weapon)
		{
			offset = i;
			break;
		}
	}

	if (offset == -1)
		offset = 0;
	else
	{
		offset = ((offset + 1) % (ptr->num));
	}
	
	// now select this offset
	findNext(ent, ptr, offset);
}

/*
 * Use an inventory item
 */
void
Cmd_Use_f(edict_t *ent)
{
	int index;
	gitem_t *it;
	char *s;

	if (!ent || !ent->client)
	{
		return;
	}

	if(ent->client->pers.spectator || ent->client->blinky_client.cam_target) /* FS: Coop: No spectators and cheats, please */
	{
		gi.cprintf(ent, PRINT_HIGH,
				"Spectators can't use this command.\n");
		return;
	}

	s = gi.args();
	it = FindItem(s);

	if (!it)
	{
		gi.cprintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}

	if (!it->use)
	{
		gi.cprintf(ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}

	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		if(game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
		{
			/* FS: Added dual weapon switching from Xatrix code */
			if (strcmp (it->pickup_name, "Blaster") == 0)
			{
				it = FindItem ("Chainfist");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else if (strcmp (it->pickup_name, "Chaingun") == 0)
			{
				it = FindItem ("ETF Rifle");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else if (strcmp (it->pickup_name, "Grenade Launcher") == 0)
			{
				it = FindItem ("Prox Launcher");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else if (strcmp (it->pickup_name, "HyperBlaster") == 0)
			{
				it = FindItem ("Plasma Beam");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else
			{
				gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		else if (game.gametype == xatrix_coop) /* FS: Coop: Xatrix specific */
		{
			// RAFAEL
			if (strcmp (it->pickup_name, "HyperBlaster") == 0)
			{
				it = FindItem ("Ionripper");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			// RAFAEL
			else if (strcmp (it->pickup_name, "Railgun") == 0)
			{
				it = FindItem ("Phalanx");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else
			{
				gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
			return;
		}
	}

	it->use(ent, it);
}

/*
 * Drop an inventory item
 */
void
Cmd_Drop_f(edict_t *ent)
{
	int dropTimer; /* FS */
	int index;
	gitem_t *it;
	char *s;

	if (!ent || !ent->client)
	{
		return;
	}

	if(ent->client->pers.spectator || ent->client->blinky_client.cam_target) /* FS: Coop: No spectators and cheats, please */
	{
		gi.cprintf(ent, PRINT_HIGH,
				"Spectators can't use this command.\n");
		return;
	}

	if (sv_drop_timeout->value) /* FS: Stop trouble makers */
	{
		dropTimer = (ent->client->dropTimeout + sv_drop_timeout->value) - level.time;
		if (dropTimer > 0)
		{
			gi.cprintf(ent, PRINT_HIGH, "You must wait at least %d second(s) before you can use this command.\n", dropTimer);
			return;
		}
	}

	s = gi.args();
	it = FindItem(s);

	if (!it)
	{
		gi.cprintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}

	if (!it->drop)
	{
		gi.cprintf(ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}

	if (!G_SpawnCheck(256))
	{
		gi.cprintf(ent, PRINT_HIGH, "Too many items in world.  Cannot drop right now.\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (!ent->client->pers.inventory[index])
	{
		if(game.gametype == rogue_coop) /* FS: Coop: Rogue Specific */
		{
			/* FS: Added dual weapon switching from Xatrix code */
			if (strcmp (it->pickup_name, "Blaster") == 0)
			{
				it = FindItem ("Chainfist");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else if (strcmp (it->pickup_name, "Chaingun") == 0)
			{
				it = FindItem ("ETF Rifle");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else if (strcmp (it->pickup_name, "Grenade Launcher") == 0)
			{
				it = FindItem ("Prox Launcher");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else if (strcmp (it->pickup_name, "HyperBlaster") == 0)
			{
				it = FindItem ("Plasma Beam");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else
			{
				gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		else if(game.gametype == xatrix_coop) /* FS: Coop: Xatrix specific */
		{
			// RAFAEL
			if (strcmp (it->pickup_name, "HyperBlaster") == 0)
			{
				it = FindItem ("Ionripper");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			// RAFAEL
			else if (strcmp (it->pickup_name, "Railgun") == 0)
			{
				it = FindItem ("Phalanx");
				index = ITEM_INDEX (it);
				if (!ent->client->pers.inventory[index])
				{
					gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
					return;
				}
			}
			else
			{
				gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
			return;
		}
	}

	it->drop(ent, it);
	ent->client->dropTimeout = level.time; /* FS */
}

void
Cmd_Score_f(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (ent->client->menu)
	{
		PMenu_Close(ent);
	}

	if (!deathmatch->intValue && !coop->intValue)
	{
		return;
	}

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		ent->client->update_chase = true;
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboardMessage(ent, ent->enemy);
	gi.unicast(ent, true);
}

void
Cmd_Help_f(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	/* this is for backwards compatability */
	if (deathmatch->intValue)
	{
		Cmd_Score_f(ent);
		return;
	}

	if(coop->intValue && ent->client->showscores) /* FS: Coop: Remove scoreboard on next tab and exit */
	{
		ent->client->showinventory = false;
		ent->client->showscores = false;
		ent->client->showhelp = false;
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp)
	{
		ent->client->showhelp = false;
		if (coop->intValue) /* FS: Coop: Show scoreboard on next tab */
		{
			Cmd_Score_f(ent);
		}
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;
	HelpComputerMessage(ent);
}

void
Cmd_Inven_f(edict_t *ent)
{
	gclient_t *cl;

	if (!ent || !ent->client)
	{
		return;
	}

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

	if (ent->client->menu)
	{
		PMenu_Close(ent);
		ent->client->update_chase = true;
	}
	else
	{
		if(!cl->showinventory && maxclients->intValue > 1)
		{
			CoopOpenJoinMenu(ent);
			return;
		}
	}

	if (cl->showinventory || ent->client->pers.spectator)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;
	
	InventoryMessage(ent);
	gi.unicast(ent, true);
}

void
Cmd_InvUse_f(edict_t *ent)
{
	gitem_t *it;

	if (!ent)
	{
		return;
	}

	if (ent->client->menu)
	{
		PMenu_Select(ent);
		return;
	}

	if (ent->client->blinky_client.cam_target)
	{
		gi.cprintf(ent, PRINT_HIGH, "Can't use this command in chase mode!\n");
		return;
	}

	ValidateSelectedItem(ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf(ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];

	if (!it->use)
	{
		gi.cprintf(ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}

	it->use(ent, it);
}

void
Cmd_WeapPrev_f(edict_t *ent)
{
	gclient_t *cl;
	int i, index;
	gitem_t *it;
	int selected_weapon;

	if (!ent)
	{
		return;
	}

	cl = ent->client;

	if (!cl->pers.weapon)
	{
		return;
	}

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	/* scan  for the next valid one */
	for (i = 1; i <= MAX_ITEMS; i++)
	{
		/* prevent scrolling through ALL weapons */
		index = (selected_weapon + MAX_ITEMS - i) % MAX_ITEMS;

		if (!cl->pers.inventory[index])
		{
			continue;
		}

		it = &itemlist[index];

		if ((game.gametype == zaero_coop) && (it->hideFlags & HIDE_FROM_SELECTION)) /* FS: Zaero specific game dll changes */
		{
			continue;
		}

		if (!it->use)
		{
			continue;
		}

		if (!(it->flags & IT_WEAPON))
		{
			continue;
		}

		it->use(ent, it);

		/* prevent scrolling through ALL weapons */
		if (cl->newweapon == it)
		{
			return;
		}
	}
}

void
Cmd_WeapNext_f(edict_t *ent)
{
	gclient_t *cl;
	int i, index;
	gitem_t *it;
	int selected_weapon;

	if (!ent)
	{
		return;
	}

	cl = ent->client;

	if (!cl->pers.weapon)
	{
		return;
	}

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	/* scan  for the next valid one */
	for (i = 1; i <= MAX_ITEMS; i++)
	{
		/* prevent scrolling through ALL weapons */
		index = (selected_weapon + i) % MAX_ITEMS;

		if (!cl->pers.inventory[index])
		{
			continue;
		}

		it = &itemlist[index];

		if ((game.gametype == zaero_coop) && (it->hideFlags & HIDE_FROM_SELECTION)) /* FS: Zaero specific game dll changes */
		{
			continue;
		}

		if (!it->use)
		{
			continue;
		}

		if (!(it->flags & IT_WEAPON))
		{
			continue;
		}

		it->use(ent, it);

		/* prevent scrolling through ALL weapons */
		if (cl->newweapon == it)
		{
			return;
		}
	}
}

void
Cmd_WeapLast_f(edict_t *ent)
{
	gclient_t *cl;
	int index;
	gitem_t *it;

	if (!ent)
	{
		return;
	}

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
	{
		return;
	}

	index = ITEM_INDEX(cl->pers.lastweapon);

	if (!cl->pers.inventory[index])
	{
		return;
	}

	it = &itemlist[index];

	if (!it->use)
	{
		return;
	}

	if (!(it->flags & IT_WEAPON))
	{
		return;
	}

	it->use(ent, it);
}

void
Cmd_InvDrop_f(edict_t *ent)
{
	gitem_t *it;

	if (!ent)
	{
		return;
	}

	ValidateSelectedItem(ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf(ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];

	if (!it->drop)
	{
		gi.cprintf(ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}

	it->drop(ent, it);
}

void
Cmd_Kill_f(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if (((level.time - ent->client->respawn_time) < 5) ||
		(ent->client->resp.spectator))
	{
		return;
	}

	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;

	if(game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		/* make sure no trackers are still hurting us. */
		if (ent->client->tracker_pain_framenum)
		{
			RemoveAttackingPainDaemons(ent);
		}

		if (ent->client->owned_sphere)
		{
			G_FreeEdict(ent->client->owned_sphere);
			ent->client->owned_sphere = NULL;
		}
	}

	player_die(ent, ent, ent, 100000, vec3_origin);

	if (game.gametype == zaero_coop) /* FS: Zaero specific game dll changes */
	{
		/* don't even bother waiting for death frames */
		ent->deadflag = DEAD_DEAD;
		respawn (ent);
	}
}

void
Cmd_PutAway_f(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;

	if ((game.gametype == zaero_coop) && (ent->client->zCameraTrack)) /* FS: Zaero specific game dll changes */
	{
		stopCamera(ent);
	}
}

int
PlayerSort(void const *a, void const *b)
{
	int anum, bnum;

	if (!a || !b)
	{
		return 0;
	}

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
	{
		return -1;
	}

	if (anum > bnum)
	{
		return 1;
	}

	return 0;
}

void
Cmd_Players_f(edict_t *ent)
{
	int i;
	int count;
	char small[64];
	char large[1280];
	int index[256];

	if (!ent)
	{
		return;
	}

	count = 0;

	for (i = 0; i < maxclients->intValue; i++)
	{
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}
	}

	/* sort by frags */
	qsort(index, count, sizeof(index[0]), PlayerSort);

	/* print information */
	large[0] = 0;

	for (i = 0; i < count; i++)
	{
		Com_sprintf(small, sizeof(small), "[%2d] %3i %s\n",
				index[i],
				game.clients[index[i]].ps.stats[STAT_FRAGS],
				game.clients[index[i]].pers.netname);

		if (strlen(small) + strlen(large) > sizeof(large) - 100)
		{
			/* can't print all of them in one packet */
			strcat(large, "...\n");
			break;
		}

		strcat(large, small);
	}

	gi.cprintf(ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

void
Cmd_Wave_f(edict_t *ent)
{
	int i;

	if (!ent)
	{
		return;
	}

	i = atoi(gi.argv(1));

	/* can't wave when ducked */
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		return;
	}

	if (ent->client->anim_priority > ANIM_WAVE)
	{
		return;
	}

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
		case 0:
			gi.cprintf(ent, PRINT_HIGH, "flipoff\n");
			ent->s.frame = FRAME_flip01 - 1;
			ent->client->anim_end = FRAME_flip12;
			break;
		case 1:
			gi.cprintf(ent, PRINT_HIGH, "salute\n");
			ent->s.frame = FRAME_salute01 - 1;
			ent->client->anim_end = FRAME_salute11;
			break;
		case 2:
			gi.cprintf(ent, PRINT_HIGH, "taunt\n");
			ent->s.frame = FRAME_taunt01 - 1;
			ent->client->anim_end = FRAME_taunt17;
			break;
		case 3:
			gi.cprintf(ent, PRINT_HIGH, "wave\n");
			ent->s.frame = FRAME_wave01 - 1;
			ent->client->anim_end = FRAME_wave11;
			break;
		case 4:
		default:
			gi.cprintf(ent, PRINT_HIGH, "point\n");
			ent->s.frame = FRAME_point01 - 1;
			ent->client->anim_end = FRAME_point12;
			break;
	}
}

/* FS: Map voting from chat messages */
void sayCmd_CheckVote(edict_t *ent, char *voteChat)
{

	if(!voteChat || voteChat[0] == '\0')
		return;

	if(!Q_stricmp(voteChat, "vote yes"))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString("vote yes\n");
		gi.unicast(ent, true);
	}
	else if(!Q_stricmp(voteChat, "vote no"))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString("vote no\n");
		gi.unicast(ent, true);
	}
	else if (!Q_stricmp(voteChat, "vote stop"))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString("vote stop\n");
		gi.unicast(ent, true);
	}
	else if (!Q_stricmp(voteChat, "vote restartmap"))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString("vote restartmap\n");
		gi.unicast(ent, true);
	}
	else if (!Q_stricmp(voteChat, "vote playerexit"))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString("vote playerexit\n");
		gi.unicast(ent, true);
	}
	else if(!strncmp(voteChat, "vote map ", 9))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(va("%s\n", voteChat));
		gi.unicast(ent, true);
	}
	else if(!strncmp(voteChat, "vote warp ", 10))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(va("%s\n", voteChat));
		gi.unicast(ent, true);
	}
	else if(!strncmp(voteChat, "vote gamemode ", 14))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(va("%s\n", voteChat));
		gi.unicast(ent, true);
	}
	else if(!strncmp(voteChat, "vote skill ", 11))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(va("%s\n", voteChat));
		gi.unicast(ent, true);
	}
	else if(!strncmp(voteChat, "vote kick ", 10))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(va("%s\n", voteChat));
		gi.unicast(ent, true);
	}
	else if(!strncmp(voteChat, "vote ban ", 9))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString(va("%s\n", voteChat));
		gi.unicast(ent, true);
	}
	else if (!Q_stricmp(voteChat, "vote help") || !Q_stricmp(voteChat, "vote list") || !Q_stricmp(voteChat, "vote cmds") || !Q_stricmp(voteChat, "vote commands"))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString("vote help\n");
		gi.unicast(ent, true);
	}
	else if (!Q_stricmp(voteChat, "vote progress"))
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString("vote progress\n");
		gi.unicast(ent, true);
	}
	else if (sv_vote_chat_commands->intValue) /* FS: Too many people assuming "yes" and "no" as chat messages are appropriate */
	{
		if(bVoteInProgress && !ent->hasVoted)
		{
			if (!Q_stricmp(voteChat, "yes"))
			{
				gi.WriteByte(svc_stufftext);
				gi.WriteString("vote yes\n");
				gi.unicast(ent, true);
			}
			else if (!Q_stricmp(voteChat, "no"))
			{
				gi.WriteByte(svc_stufftext);
				gi.WriteString("vote no\n");
				gi.unicast(ent, true);
			}
		}
	}
}

void
Cmd_Say_f(edict_t *ent, qboolean team, qboolean arg0)
{
	int i, j;
	edict_t *other;
	char *p;
	char text[2048];
	gclient_t *cl;

	if (!ent)
	{
		return;
	}

	if ((gi.argc() < 2) && !arg0)
	{
		return;
	}

	if (!(dmflags->intValue & (DF_MODELTEAMS | DF_SKINTEAMS)))
	{
		team = false;
	}

	if (team)
	{
		Com_sprintf(text, sizeof(text), "(%s): ", ent->client->pers.netname);
	}
	else
	{
		Com_sprintf(text, sizeof(text), "%s: ", ent->client->pers.netname);
	}

	if (arg0)
	{
		p = strchr(text, 0); /*  for sayCmd_CheckVote()  */
		strcat(text, gi.argv(0));
		strcat(text, " ");
		strcat(text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p) - 1] = 0;
		}

		strcat(text, p);
	}

	/* don't let text be too long for malicious reasons */
	if (strlen(text) > 150)
	{
		text[150] = 0;
	}

	strcat(text, "\n");

	/* FS: Admins, VIPs, and WallFly are exempt from this. */
	if (!ent->client->pers.isAdmin && !ent->client->pers.isVIP && !ent->client->pers.isWallFly && flood_msgs->value)
	{
		cl = ent->client;

		if (level.time < cl->flood_locktill)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
					(int)(cl->flood_locktill - level.time));
			return;
		}

		i = cl->flood_whenhead - flood_msgs->value + 1;

		if (i < 0)
		{
			i = (sizeof(cl->flood_when) / sizeof(cl->flood_when[0])) + i;
		}

		if (cl->flood_when[i] &&
			(level.time - cl->flood_when[i] < flood_persecond->value))
		{
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT,
					"Flood protection:  You can't talk for %d seconds.\n",
					(int)flood_waitdelay->value);
			return;
		}

		cl->flood_whenhead = (cl->flood_whenhead + 1) % (sizeof(cl->flood_when) /
							  sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->intValue)
	{
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);
	}

	sayCmd_CheckVote(ent, p); /* FS: Check if it's a voting command */

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];

		if (!other->inuse)
		{
			continue;
		}

		if (!other->client)
		{
			continue;
		}

		if (team)
		{
			if (!OnSameTeam(ent, other))
			{
				continue;
			}
		}

		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void
Cmd_Ent_Count_f(edict_t *ent) /* FS: Coop: Rogue specific */
{
	int x;
	edict_t *e;

	if (game.gametype != rogue_coop || !ent)
	{
		return;
	}

	x = 0;

	for (e = g_edicts; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e->inuse)
		{
			x++;
		}
	}

	gi.dprintf(DEVELOPER_MSG_GAME, "%d entites active\n", x);
}

void
Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	if (!ent)
	{
		return;
	}

	/* connect time, ping, score, name */
	*text = 0;

	for (i = 0, e2 = g_edicts + 1; i < maxclients->intValue; i++, e2++)
	{
		if (!e2->inuse)
		{
			continue;
		}

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
				(level.framenum - e2->client->resp.enterframe) / 600,
				((level.framenum - e2->client->resp.enterframe) % 600) / 10,
				e2->client->ping,
				e2->client->resp.score,
				e2->client->pers.netname,
				e2->client->resp.spectator ? " (spectator)" : "");

		if (strlen(text) + strlen(st) > sizeof(text) - 50)
		{
			sprintf(text + strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}

		strcat(text, st);
	}

	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}

void
Cmd_EdictCount_f (edict_t *ent) /* FS: Coop: Added for debugging */
{
	int i = 0, edictCount = 0, freeCount = 0, badFreeCount = 0;
	edict_t *e;

	if (!ent || !ent->client || !ent->client->pers.isAdmin)
	{
		return;
	}

	e = &g_edicts[1]; /* FS: Skip worldspawn, but count maxclients */

	for (i = 1; i < globals.num_edicts; i++, e++)
	{
		if (e->inuse)
		{
			edictCount++;
		}
		else
		{
			if (gi.argc() > 1) /* FS: Can cause overflows with too many cprintfs here.  So only request it if I want to see it. */
			{
				gi.cprintf(ent, PRINT_HIGH, "Freed edict time: %f.  Level.time: %f\n", e->freetime, level.time);
			}
			if(e->freetime > level.time) /* FS: This should be impossible now.  But let's keep track of it just in-case. */
			{
				badFreeCount++;
			}
			freeCount++;
		}
	}

	gi.cprintf(ent, PRINT_HIGH, "Edicts in use: %i.  Edicts freed: %i.  Bad freetimes: %i.  Total edicts: %i.  Max edicts: %i\n", edictCount, freeCount, badFreeCount, globals.num_edicts, globals.max_edicts);
}

void Cmd_Beam_f (edict_t *ent) /* FS: From YamaqiQ2: Beam us to direct coordinates given, for debugging. */
{
	vec3_t dest = {0};
	int i = 0;

	if (!ent || !ent->client || !ent->client->pers.isAdmin)
	{
		return;
	}

	if ((deathmatch->intValue || coop->intValue) && !Client_CanCheat(ent))
	{
		gi.cprintf(ent, PRINT_HIGH,
				"You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if(gi.argc() < 4)
	{
		gi.cprintf(ent, PRINT_HIGH, "Usage: beam X-Coord Y-Coord Z-Coord\n");
		return;
	}

	for(i=0; i<3; ++i)
	{
		dest[i] = atof(gi.argv(i+1));
	}

	VectorCopy(dest, ent->s.origin);
	VectorCopy(dest, ent->s.old_origin);
	ent->s.origin[2] += 10;

	ent->client->ps.pmove.pm_time = 160 >> 3;       /* hold time */
	ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	VectorClear(ent->s.angles);
	VectorClear(ent->client->ps.viewangles);
}

void Cmd_PlaceCheckpoint_f (edict_t *ent) /* FS: Added */
{
	edict_t *spawn;

	if (!ent || !ent->client || !ent->client->pers.isAdmin || !coop->intValue)
	{
		return;
	}

	spawn = G_Spawn();
	if(!spawn)
	{
		gi.cprintf(ent, PRINT_HIGH, "Unable to create a checkpoint!\n");
		return;
	}

	spawn->classname = "info_coop_checkpoint";
	VectorCopy(ent->s.origin, spawn->s.origin);
	spawn->s.origin[2] += 20;

	spawn->s.angles[YAW] = ent->client->ps.viewangles[YAW];

	SP_info_coop_checkpoint(spawn);

	gi.cprintf(ent, PRINT_HIGH, "Coop checkpoint created!\n");
}

void Cmd_SaveCheckpoint_f (edict_t *ent) /* FS: Added */
{
	char checkpointString[1024];
	char fileName[MAX_OSPATH];
	edict_t *e;
	FILE *f = NULL;
	int count = 0;

	if (!ent || !ent->client || !ent->client->pers.isAdmin || !coop->intValue)
	{
		return;
	}

	Sys_Mkdir(va("%s/maps", gamedir->string));

	Com_sprintf(fileName, sizeof(fileName), "%s/maps/%s_checkpoints.txt", gamedir->string, level.mapname);

	f = fopen(fileName, "w");
	if(!f)
	{
		gi.cprintf(ent, PRINT_HIGH, "Error: Can't open %s for writing!\n", fileName);
		return;
	}

	for (e = g_edicts; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (!e->inuse || !e->classname)
		{
			continue;
		}

		if(!Q_stricmp(e->classname, "info_coop_checkpoint"))
		{
			Com_sprintf(checkpointString, sizeof(checkpointString), "{\n\"classname\" \"%s\"\n\"angle\" \"%i\"\n\"origin\" \"%i %i %i\"\n}\n", e->classname, (int)e->s.angles[YAW], (int)e->s.origin[0], (int)e->s.origin[1], (int)e->s.origin[2]+15);
			fputs(checkpointString, f);
			count++;
		}
	}

	fclose(f);

	if(count)
	{
		gi.cprintf(ent, PRINT_HIGH, "Saved %i checkpoints\n", count);
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "No checkpoints to save.  Deleting file.\n");
		remove(fileName);
	}
}

void Cmd_DeleteCheckpoints_f (edict_t *ent) /* FS: Added */
{
	char fileName[MAX_OSPATH];
	FILE *f = NULL;

	if (!ent || !ent->client || !ent->client->pers.isAdmin || !coop->intValue)
	{
		return;
	}

	if(gi.argc() == 2 && !Q_stricmp(gi.argv(1), "all")) /* FS: Remove all from the map.  Maybe it was saved, transitioned this map and I don't want to restart the whole map. */
	{
		int count = 0;
		edict_t *coop_checkpoint;

		for (coop_checkpoint = g_edicts; coop_checkpoint < &g_edicts[globals.num_edicts]; coop_checkpoint++)
		{
			if(!coop_checkpoint->inuse || !coop_checkpoint->classname)
			{
				continue;
			}

			if(!Q_stricmp(coop_checkpoint->classname, "info_coop_checkpoint"))
			{
				G_FreeEdict(coop_checkpoint);
				count++;
			}

			level.current_coop_checkpoint = NULL;
		}

		gi.cprintf(ent, PRINT_HIGH, "Removed %d info_coop_checkpoints from map.\n", count);
	}

	Com_sprintf(fileName, sizeof(fileName), "%s/maps/%s_checkpoints.txt", gamedir->string, level.mapname);

	f = fopen(fileName, "w");
	if(!f)
	{
		gi.cprintf(ent, PRINT_HIGH, "Error: Can't open %s for writing!\n", fileName);
		return;
	}

	fclose(f);

	gi.cprintf(ent, PRINT_HIGH, "Deleting checkpoint file: %s.\n", fileName);
	remove(fileName);
}

void
Cmd_SayPerson_f(edict_t *ent) /* FS: Tastyspleen/Q2Admin stuff.  By request. */
{
	int i;
	edict_t *other;
	char *p;
	char text[2048], entHeader[2048];
	gclient_t *cl;
	qboolean bIsPlayerNum = false;
	qboolean bIsSearch = false;

	if (!ent)
	{
		return;
	}

	if ((gi.argc() < 2))
	{
		gi.cprintf(ent, PRINT_HIGH, "Usage: say_person [LIKE/CL] <player_name> <message>\n");
		return;
	}

	Com_sprintf(text, sizeof(text), "(%s)(private message) ", ent->client->pers.netname);
	Com_sprintf(entHeader, sizeof(entHeader), "(%s)(private message to: ", ent->client->pers.netname);

	p = gi.args();

	if(!strncmp(p, "CL ", 3))
	{
		int playernum;

		playernum = atoi(gi.argv(2));
		p+=3;
		if(playernum > game.maxclients)
		{
			return;
		}

		other = &g_edicts[playernum + 1];
		if(!other || !other->inuse || !other->client)
		{
			return;
		}

		p+=strlen(gi.argv(2))+1;
		if(!strncmp(p, "\" ", 2))
		{
			p+=2;
		}
	}
	else if(!strncmp(p, "LIKE ", 5))
	{
		char *name;

		name = gi.argv(2);
		p+=5;

		other = Find_LikePlayer(ent, name, false);
		if(!other)
		{
			return;
		}

		p+=strlen(gi.argv(2))+1;
		if(!strncmp(p, "\" ", 2))
		{
			p+=2;
		}
	}
	else
	{
		char *name;

		name = gi.argv(1);

		other = Find_LikePlayer(ent, name, true);
		if(!other)
		{
			return;
		}

		p+=strlen(gi.argv(1))+1;
		if(!strncmp(p, "\" ", 2))
		{
			p+=2;
		}
	}

	if (*p == '"')
	{
		p++;
		p[strlen(p) - 1] = 0;
	}

	strcat(text, p);

	/* don't let text be too long for malicious reasons */
	if (strlen(text) > 2000) /* FS: This is unfortunate, but this is what TSAdmin uses and wallfly can hit about 512 sometimes... */
	{
		text[2000] = 0;
	}

	strcat(text, "\n");

	strcat(entHeader, other->client->pers.netname);
	strcat(entHeader, ") ");
	strcat(entHeader, p);
	if(strlen(entHeader) > 2000) /* FS: This is unfortunate, but this is what TSAdmin uses and wallfly can hit about 512 sometimes... */
	{
		entHeader[2000] = 0;
	}
	strcat(entHeader, "\n");

	/* FS: Admins, VIPs, and WallFly are exempt from this. */
	if (!ent->client->pers.isAdmin && !ent->client->pers.isVIP && !ent->client->pers.isWallFly && flood_msgs->value)
	{
		cl = ent->client;

		if (level.time < cl->flood_locktill)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
					(int)(cl->flood_locktill - level.time));
			return;
		}

		i = cl->flood_whenhead - flood_msgs->value + 1;

		if (i < 0)
		{
			i = (sizeof(cl->flood_when) / sizeof(cl->flood_when[0])) + i;
		}

		if (cl->flood_when[i] &&
			(level.time - cl->flood_when[i] < flood_persecond->value))
		{
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT,
					"Flood protection:  You can't talk for %d seconds.\n",
					(int)flood_waitdelay->value);
			return;
		}

		cl->flood_whenhead = (cl->flood_whenhead + 1) % (sizeof(cl->flood_when) /
							  sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->intValue)
	{
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);
	}

	gi.cprintf(other, PRINT_CHAT, "%s", text);
	gi.cprintf(ent, PRINT_CHAT, "%s", entHeader);
}

void
ClientCommand(edict_t *ent)
{
	char *cmd;

	if (!ent)
	{
		return;
	}

	if (!ent->client)
	{
		return; /* not fully in game yet */
	}

	cmd = gi.argv(0);

	if ((game.gametype == zaero_coop) && (ent->client->zCameraTrack && !level.intermissiontime)) /* FS: Zaero specific game dll changes */
	{
		// if we're viewing thru the camera, only allow some things to happen
		if (Q_stricmp (cmd, "putaway") == 0)
		{
			Cmd_PutAway_f(ent);
		}
		else if (Q_stricmp(cmd, "use") == 0)
		{
			if (Q_stricmp(gi.args(), "Visor") == 0)
			{
				Cmd_Use_f(ent);
			}
		}
		else if (Q_stricmp (cmd, "invuse") == 0)
		{
			// only use the visor
			if (ent->client->pers.selected_item == ITEM_INDEX(FindItem("Visor")))
			{
				Cmd_InvUse_f (ent);
			}
		}
		else if (Q_stricmp (cmd, "invnext") == 0)
		{
			SelectNextItem (ent, -1);
		}
		else if (Q_stricmp (cmd, "invprev") == 0)
		{
			SelectPrevItem (ent, -1);
		}
	
		return;
	}

	if (Q_stricmp(cmd, "flashlight") == 0)
	{
		Cmd_Flashlight(ent);
		return;
	}

	if (Q_stricmp(cmd, "vote") == 0) /* FS: Coop: Voting */
	{
		vote_command(ent);
		return;
	}

	if (Q_stricmp(cmd, "players") == 0)
	{
		Cmd_Players_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "say") == 0)
	{
		Cmd_Say_f(ent, false, false);
		return;
	}

	if (Q_stricmp(cmd, "say_team") == 0)
	{
		Cmd_Say_f(ent, true, false);
		return;
	}

	if (Q_stricmp(cmd, "score") == 0)
	{
		Cmd_Score_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "help") == 0)
	{
		Cmd_Help_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "menu") == 0) /* FS: Added */
	{
		CoopOpenJoinMenu(ent);
		return;
	}

	if (level.intermissiontime)
	{
		return;
	}

	if(game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		if (Q_stricmp(cmd, "entcount") == 0)
		{
			Cmd_Ent_Count_f(ent);
		}
		else if (Q_stricmp(cmd, "disguise") == 0)
		{
			ent->flags |= FL_DISGUISED;
		}
	}

	if (Q_stricmp(cmd, "use") == 0)
	{
		Cmd_Use_f(ent);
	}
	else if (Q_stricmp(cmd, "drop") == 0)
	{
		Cmd_Drop_f(ent);
	}
	else if (Q_stricmp(cmd, "give") == 0)
	{
		Cmd_Give_f(ent);
	}
	else if (Q_stricmp(cmd, "god") == 0)
	{
		Cmd_God_f(ent);
	}
	else if (Q_stricmp(cmd, "notarget") == 0)
	{
		Cmd_Notarget_f(ent);
	}
	else if (Q_stricmp(cmd, "noclip") == 0)
	{
		Cmd_Noclip_f(ent);
	}
	else if (Q_stricmp(cmd, "inven") == 0)
	{
		Cmd_Inven_f(ent);
	}
	else if (Q_stricmp(cmd, "invnext") == 0)
	{
		SelectNextItem(ent, -1);
	}
	else if (Q_stricmp(cmd, "invprev") == 0)
	{
		SelectPrevItem(ent, -1);
	}
	else if (Q_stricmp(cmd, "invnextw") == 0)
	{
		SelectNextItem(ent, IT_WEAPON);
	}
	else if (Q_stricmp(cmd, "invprevw") == 0)
	{
		SelectPrevItem(ent, IT_WEAPON);
	}
	else if (Q_stricmp(cmd, "invnextp") == 0)
	{
		SelectNextItem(ent, IT_POWERUP);
	}
	else if (Q_stricmp(cmd, "invprevp") == 0)
	{
		SelectPrevItem(ent, IT_POWERUP);
	}
	else if (Q_stricmp(cmd, "invuse") == 0)
	{
		Cmd_InvUse_f(ent);
	}
	else if (Q_stricmp(cmd, "invdrop") == 0)
	{
		Cmd_InvDrop_f(ent);
	}
	else if (Q_stricmp(cmd, "weapprev") == 0)
	{
		Cmd_WeapPrev_f(ent);
	}
	else if (Q_stricmp(cmd, "weapnext") == 0)
	{
		Cmd_WeapNext_f(ent);
	}
	else if (Q_stricmp(cmd, "weaplast") == 0)
	{
		Cmd_WeapLast_f(ent);
	}
	else if (Q_stricmp(cmd, "kill") == 0)
	{
		Cmd_Kill_f(ent);
	}
	else if (Q_stricmp(cmd, "putaway") == 0)
	{
		Cmd_PutAway_f(ent);
	}
	else if (Q_stricmp(cmd, "wave") == 0)
	{
		Cmd_Wave_f(ent);
	}
	else if (Q_stricmp(cmd, "playerlist") == 0)
	{
		Cmd_PlayerList_f(ent);
	}
	else if (Q_stricmp(cmd, "edictcount") == 0) /* FS: Coop: Added for debugging */
	{
		Cmd_EdictCount_f(ent);
	}
	else if (Q_stricmp(cmd, "beam") == 0) /* FS: From YamagiQ2 */
	{
		Cmd_Beam_f(ent);
	}
	else if (Q_stricmp(cmd, "createcheckpoint") == 0) /* FS: Coop: Added checkpoints */
	{
		Cmd_PlaceCheckpoint_f(ent);
	}
	else if (Q_stricmp(cmd, "savecheckpoints") == 0) /* FS: Coop: Added checkpoints */
	{
		Cmd_SaveCheckpoint_f(ent);
	}
	else if (Q_stricmp(cmd, "deletecheckpoints") == 0) /* FS: Coop: Added checkpoints */
	{
		Cmd_DeleteCheckpoints_f(ent);
	}
	else if (Q_stricmp(cmd, "showorigin") == 0) /* FS: Zaero specific game dll changes */
	{
		ent->client->showOrigin = !ent->client->showOrigin;
		if (ent->client->showOrigin)
			gi.cprintf(ent, PRINT_HIGH, "Show origin ON\n");
		else
			gi.cprintf(ent, PRINT_HIGH, "Show origin OFF\n");
	}
	else if (Q_stricmp(cmd, "cam") == 0) /* FS: Blinkys's Coop Camera */
	{
		Cmd_Cam_f(ent);
	}
	else if (Q_stricmp(cmd, "stats") == 0) /* FS: Blinkys's Coop Camera */
	{
		Cmd_Stats_f(ent);
	}
	else if (Q_stricmp(cmd, "summon") == 0) /* FS: Blinkys's Coop Camera */
	{
		Cmd_Summon_f(ent);
	}
	else if (Q_stricmp(cmd, "teleport") == 0) /* FS: Added */
	{
		Cmd_Teleport_f(ent);
	}
	else if (Q_stricmp(cmd, "nosummon") == 0) /* FS: Blinkys's Coop Camera */
	{
		Cmd_NoSummon_f(ent);
	}
	else if (Q_stricmp(cmd, "runrun") == 0) /* FS: Blinkys's Coop Camera */
	{
		Cmd_Runrun_f(ent);
	}
	else if (Q_stricmp(cmd, "say_person") == 0) /* FS: Tastyspleen/Q2Admin stuff.  By request. */
	{
		Cmd_SayPerson_f(ent);
	}
	else if (bVoteInProgress && !ent->hasVoted)
	{
		if (!Q_stricmp(cmd, "yes") || !Q_stricmp(cmd,"agree"))
		{
			gi.WriteByte(svc_stufftext);
			gi.WriteString("vote yes\n");
			gi.unicast(ent, true);
		}
		else if (!Q_stricmp(cmd, "no"))
		{
			gi.WriteByte(svc_stufftext);
			gi.WriteString("vote no\n");
			gi.unicast(ent, true);
		}
	}
	else if ((Q_stricmp(cmd, "push") == 0) || (Q_stricmp(cmd,"pull") == 0))
	{
		/* FS: Purposely do nothing.  This somehow got in my cfgs, and some other users.  I see this happen to people during vid_restarts and vid_restarts are firing off mwheelup and mwheeldown for some reason... */
	}
	else if (Q_stricmp(cmd, "hook") == 0)
	{
		hook_fire(ent);
	}
	else if (Q_stricmp(cmd, "unhook") == 0)
	{
		if (ent->client->hook)
			hook_reset(ent->client->hook);
	}
	else /* anything that doesn't match a command will be a chat */
	{
		Cmd_Say_f(ent, false, true);
	}
}

