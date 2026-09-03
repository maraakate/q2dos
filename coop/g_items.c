#include "g_local.h"

#define HEALTH_IGNORE_MAX 1
#define HEALTH_TIMED 2

qboolean Pickup_Weapon(edict_t *ent, edict_t *other);
qboolean Pickup_CoopBackpack(edict_t *ent, edict_t *other); /* FS: Coop: Spawn a backpack with our stuff */
void Spawn_CoopBackpack(edict_t *ent); /* FS: Coop: Spawn a backpack with our stuff */

void Use_Weapon(edict_t *ent, gitem_t *inv);
void Use_Weapon2(edict_t *ent, gitem_t *inv); /* FS: Coop: Xatrix and Rogue specific */
void Drop_Weapon(edict_t *ent, gitem_t *inv);

void Weapon_Blaster(edict_t *ent);
void Weapon_Shotgun(edict_t *ent);
void Weapon_SuperShotgun(edict_t *ent);
void Weapon_Machinegun(edict_t *ent);
void Weapon_Chaingun(edict_t *ent);
void Weapon_HyperBlaster(edict_t *ent);
void Weapon_RocketLauncher(edict_t *ent);
void Weapon_Grenade(edict_t *ent);
void Weapon_GrenadeLauncher(edict_t *ent);
void Weapon_Railgun(edict_t *ent);
void Weapon_BFG(edict_t *ent);

void Weapon_ChainFist(edict_t *ent); /* FS: Coop: Rogue specific */
void Weapon_Disintegrator(edict_t *ent); /* FS: Coop: Rogue specific */
void Weapon_ETF_Rifle(edict_t *ent); /* FS: Coop: Rogue specific */
void Weapon_Heatbeam(edict_t *ent); /* FS: Coop: Rogue specific */
void Weapon_Prox(edict_t *ent); /* FS: Coop: Rogue specific */
void Weapon_Tesla(edict_t *ent); /* FS: Coop: Rogue specific */
void Weapon_ProxLauncher(edict_t *ent); /* FS: Coop: Rogue specific */

void Weapon_Ionripper(edict_t *ent); /* FS: Coop: Xatrix specific */
void Weapon_Phalanx(edict_t *ent); /* FS: Coop: Xatrix specific */
void Weapon_Trap(edict_t *ent); /* FS: Coop: Xatrix specific */

gitem_armor_t jacketarmor_info = {25, 50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info = {50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info = {100, 200, .80, .60, ARMOR_BODY};

int jacket_armor_index;
int combat_armor_index;
int body_armor_index;
static int power_screen_index;
static int power_shield_index;
// Knightmare-- added indexes
int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;
int	shells_index;
int	bullets_index;
int	grenades_index;
int	rockets_index;
int	cells_index;
int	slugs_index;
int	flares_index;
int	tbombs_index;
int	empnuke_index;
int	plasmashield_index;
int	a2k_index;
// Knightmare-- end

void Use_Quad(edict_t *ent, gitem_t *item);
void Use_QuadFire(edict_t *ent, gitem_t *item); /* FS: Coop: Xatrix specific */

static int quad_drop_timeout_hack;
static int quad_fire_drop_timeout_hack; /* FS: Coop: Xatrix specific */

extern void ED_CallSpawn(edict_t *ent); /* FS: Coop: For SP_Xatrix_item */

// *** Zaero prototypes ***
void Weapon_FlareGun (edict_t *ent); /* FS: Zaero specific game dll changes */
void Weapon_SniperRifle(edict_t *ent); /* FS: Zaero specific game dll changes */
void Weapon_LaserTripBomb(edict_t *ent); /* FS: Zaero specific game dll changes */
void Weapon_SonicCannon (edict_t *ent); /* FS: Zaero specific game dll changes */
void Weapon_EMPNuke (edict_t *ent); /* FS: Zaero specific game dll changes */
void Weapon_A2k (edict_t *ent); /* FS: Zaero specific game dll changes */
void Use_Visor (edict_t *ent, gitem_t *item); /* FS: Zaero specific game dll changes */
void Action_Push(edict_t *ent); /* FS: Zaero specific game dll changes */
void Use_PlasmaShield (edict_t *ent, gitem_t *item); /* FS: Zaero specific game dll changes */


/* ====================================================================== */

gitem_t *
GetItemByIndex(int index)
{
	if ((index == 0) || (index >= game.num_items))
	{
		return NULL;
	}

	return &itemlist[index];
}

gitem_t *
FindItemByClassname(char *classname)
{
	int i;
	gitem_t *it;

	if (!classname)
	{
		return NULL;
	}

	it = itemlist;

	for (i = 0; i < game.num_items; i++, it++)
	{
		if (!it->classname)
		{
			continue;
		}

		if (!Q_stricmp(it->classname, classname))
		{
			return it;
		}
	}

	return NULL;
}

gitem_t *
FindItem(char *pickup_name)
{
	int i;
	gitem_t *it;

	if (!pickup_name)
	{
		return NULL;
	}

	it = itemlist;

	for (i = 0; i < game.num_items; i++, it++)
	{
		if (!it->pickup_name)
		{
			continue;
		}

		if (!Q_stricmp(it->pickup_name, pickup_name))
		{
			return it;
		}
	}

	return NULL;
}

/* ====================================================================== */

qboolean Coop_Respawn (void) /* FS: Coop */
{
	if(coop->intValue && coop_item_respawn->intValue)
	{
		return true;
	}

	return false;
}

void
DoRespawn(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if (ent->team)
	{
		edict_t *master;
		int count;
		int choice;

		master = ent->teammaster;
		if (!master)
		{
			return;
		}

		for (count = 0, ent = master; ent; ent = ent->chain, count++)
		{
		}

		choice = count ? rand() % count : 0;

		for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
		{
		}
	}

	if (randomrespawn && randomrespawn->intValue) /* FS: Coop: Rogue specific */
	{
		edict_t *newEnt;

		newEnt = DoRandomRespawn(ent);

		/* if we've changed entities, then do some sleight
		 * of hand. otherwise, the old entity will respawn */
		if (newEnt)
		{
			G_FreeEdict(ent);
			ent = newEnt;
		}
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity(ent);

	/* send an effect */
	ent->s.event = EV_ITEM_RESPAWN;
}

void
SetRespawn(edict_t *ent, float delay)
{
	if (!ent)
	{
		return;
	}

	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	gi.linkentity(ent);
}

/* ====================================================================== */

qboolean
Pickup_Powerup(edict_t *ent, edict_t *other)
{
	int quantity;

    if (!ent || !other)
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (((skill->intValue == 1) &&
		 (quantity >= 2)) || ((skill->intValue >= 2) && (quantity >= 1)))
	{
		return false;
	}

	if ((coop->intValue) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->intValue || Coop_Respawn()) /* FS: Coop: Added */
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			SetRespawn(ent, ent->item->quantity);
		}

		if ((dmflags->intValue & DF_INSTANT_ITEMS) ||
			((ent->item->use == Use_Quad) &&
			 (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_Quad) &&
				(ent->spawnflags & DROPPED_PLAYER_ITEM))
			{
				quad_drop_timeout_hack =
					(ent->nextthink - level.time) / FRAMETIME;
			}

			if (ent->item->use)
			{
				ent->item->use(other, ent->item);
			}
			else /* FS: Coop: Rogue specific.  Looks like a possible fix.  Probably OK as-is. */
			{
				gi.dprintf(DEVELOPER_MSG_GAME, "Powerup has no use function!\n");
			}
		}
		else if ((dmflags->intValue & DF_INSTANT_ITEMS) || /* FS: Coop: Xatrix specific */
				 ((ent->item->use == Use_QuadFire) &&
				  (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_QuadFire) &&
				(ent->spawnflags & DROPPED_PLAYER_ITEM))
			{
				quad_fire_drop_timeout_hack =
					(ent->nextthink - level.time) / FRAMETIME;
			}

			ent->item->use(other, ent->item);
		}
	}

	return true;
}

void
Drop_General(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}
	Drop_Item(ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);
}

/* ====================================================================== */

qboolean
Pickup_Adrenaline(edict_t *ent, edict_t *other)
{
	if (!ent || !other)
	{
		return false;
	}

	if (!deathmatch->intValue)
	{
		other->max_health += 1;
	}

	if (coop->intValue) /* FS: Coop: Keep +1 bonuses during respawn */
	{
		other->client->resp.coop_respawn.max_health += 1;
	}

	if (other->health < other->max_health)
	{
		other->health = other->max_health;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->intValue || Coop_Respawn()) ) /* FS: Coop: Added */
	{
		SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

qboolean
Pickup_AncientHead(edict_t *ent, edict_t *other)
{
 	if (!ent || !other)
	{
		return false;
	}

	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->intValue || Coop_Respawn()) ) /* FS: Coop: Added */
	{
		SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

qboolean
Pickup_Bandolier(edict_t *ent /* may be null */, edict_t *other)
{
	gitem_t *item;
	int index;

	if (!other)
	{
		return false;
	}

	if (other->client->pers.max_bullets < 250)
	{
		other->client->pers.max_bullets = 250;
	}

	if (other->client->pers.max_shells < 150)
	{
		other->client->pers.max_shells = 150;
	}

	if (other->client->pers.max_cells < 250)
	{
		other->client->pers.max_cells = 250;
	}

	if (other->client->pers.max_slugs < 75)
	{
		other->client->pers.max_slugs = 75;
	}

	if (game.gametype == rogue_coop)
	{
		if (other->client->pers.max_flechettes < 250) /* FS: Coop: Rogue specific */
		{
			other->client->pers.max_flechettes = 250;
		}
		if (other->client->pers.max_rounds < 150) /* Knightmare: Coop: Rogue specific */
		{
			other->client->pers.max_rounds = 150;
		}
	}

	if (game.gametype == xatrix_coop)
	{
		if (other->client->pers.max_magslug < 75) /* FS: Coop: Xatrix specific */
		{
			other->client->pers.max_magslug = 75;
		}
	}

	if (game.gametype == zaero_coop)
	{
		if (other->client->pers.max_flares < 45) /* Knightmare: Zaero specific */
		{
			other->client->pers.max_flares = 45;
		}
	}

	item = FindItem("Bullets");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_bullets)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_bullets;
		}
	}

	item = FindItem("Shells");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_shells)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_shells;
		}
	}

	if ((ent) && (!(ent->spawnflags & DROPPED_ITEM)) && (deathmatch->intValue || Coop_Respawn()) ) /* FS: Coop: Added */
	{
		SetRespawn(ent, ent->item->quantity);
	}

	if (coop->intValue) /* FS: Coop: Keep bandolier during respawn */
	{
		/* Knightmare- Make sure client isn't already set to respawn with a pack before setting this value! */
		if (other->client->pers.ammoUpgrade != COOP_BACKPACK)
			other->client->pers.ammoUpgrade = other->client->resp.coop_respawn.ammoUpgrade = COOP_BANDOLIER;
	}

	return true;
}

qboolean
Pickup_Pack(edict_t *ent /* may be null */, edict_t *other)
{
	gitem_t *item;
	int index;

 	if (!other)
	{
		return false;
	}

	if (other->client->pers.max_bullets < 300)
	{
		other->client->pers.max_bullets = 300;
	}

	if (other->client->pers.max_shells < 200)
	{
		other->client->pers.max_shells = 200;
	}

	if (other->client->pers.max_rockets < 100)
	{
		other->client->pers.max_rockets = 100;
	}

	if (other->client->pers.max_grenades < 100)
	{
		other->client->pers.max_grenades = 100;
	}

	if (other->client->pers.max_cells < 300)
	{
		other->client->pers.max_cells = 300;
	}

	if (other->client->pers.max_slugs < 100)
	{
		other->client->pers.max_slugs = 100;
	}

	if (game.gametype == rogue_coop)
	{
		if (other->client->pers.max_flechettes < 300) /* FS: Coop: Rogue specific */
		{
			other->client->pers.max_flechettes = 300; /* Knightmare- this was 200 instead of 300! */
		}
		if (other->client->pers.max_rounds < 200) /* Knightmare: Coop: Rogue specific */
		{
			other->client->pers.max_rounds = 200;
		}
		if (other->client->pers.max_prox < 100) /* Knightmare: Coop: Rogue specific */
		{
			other->client->pers.max_prox = 100;
		}
		if (other->client->pers.max_tesla < 100) /* Knightmare: Coop: Rogue specific */
		{
			other->client->pers.max_tesla = 100;
		}
	}

	if (game.gametype == xatrix_coop)
	{
		if (other->client->pers.max_magslug < 100) /* FS: Coop: Xatrix specific */
		{
			other->client->pers.max_magslug = 100;
		}
	}

	if (game.gametype == zaero_coop)
	{
		if (other->client->pers.max_tbombs < 100) /* FS: Zaero specific game dll changes */
		{
			other->client->pers.max_tbombs = 100;
		}

		if (other->client->pers.max_flares < 60) /* Knightmare: Zaero specific game dll changes */
		{
			other->client->pers.max_flares = 60;
		}

		if (other->client->pers.max_a2k < 1) /* FS: Zaero specific game dll changes */
		{
			other->client->pers.max_a2k = 1;
		}

		if (other->client->pers.max_empnuke < 100) /* FS: Zaero specific game dll changes */
		{
			other->client->pers.max_empnuke = 100;
		}

		if (other->client->pers.max_plasmashield < 40) /* FS: Zaero specific game dll changes */
		{
			other->client->pers.max_plasmashield = 40;
		}
	}

	item = FindItem("Bullets");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_bullets)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_bullets;
		}
	}

	item = FindItem("Shells");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_shells)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_shells;
		}
	}

	item = FindItem("Cells");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_cells)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_cells;
		}
	}

	item = FindItem("Grenades");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_grenades)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_grenades;
		}
	}

	item = FindItem("Rockets");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_rockets)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_rockets;
		}
	}

	item = FindItem("Slugs");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_slugs)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_slugs;
		}
	}

	if (game.gametype == rogue_coop)
	{
		item = FindItem("Flechettes"); /* FS: Coop: Rogue specific */

		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;

			if (other->client->pers.inventory[index] >
				other->client->pers.max_flechettes)
			{
				other->client->pers.inventory[index] =
					other->client->pers.max_flechettes;
			}
		}
	}

	if (game.gametype == xatrix_coop)
	{
		item = FindItem("Mag Slug"); /* FS: Coop: Xatrix specific */

		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;

			if (other->client->pers.inventory[index] >
				other->client->pers.max_magslug)
			{
				other->client->pers.inventory[index] =
					other->client->pers.max_magslug;
			}
		}
	}

	if(game.gametype == zaero_coop)
	{
		item = FindItem("IRED"); /* FS: Zaero specific game dll changes */
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > 
				other->client->pers.max_tbombs)
			{
				other->client->pers.inventory[index] = 
					other->client->pers.max_tbombs;
			}
		}

		item = FindItem("A2k"); /* FS: Zaero specific game dll changes */
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_a2k)
				other->client->pers.inventory[index] = other->client->pers.max_a2k;
		}

		item = FindItem("EMPNuke"); /* FS: Zaero specific game dll changes */
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_empnuke)
				other->client->pers.inventory[index] = other->client->pers.max_empnuke;
		}

		item = FindItem("Plasma Shield"); /* FS: Zaero specific game dll changes */
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_plasmashield)
				other->client->pers.inventory[index] = other->client->pers.max_plasmashield;
		}
	}

	if ((ent) && (!(ent->spawnflags & DROPPED_ITEM)) && (deathmatch->intValue || Coop_Respawn()) ) /* FS: Coop: Added */
	{
		SetRespawn(ent, ent->item->quantity);
	}

	if (coop->intValue)
	{
		other->client->pers.ammoUpgrade = other->client->resp.coop_respawn.ammoUpgrade = COOP_BACKPACK; /* FS: Coop: Keep backpack during respawn */
	}

	return true;
}

qboolean
Pickup_Nuke(edict_t *ent, edict_t *other) /* FS: Coop: Rogue specific */
{
	int quantity;

	if (!ent || !other)
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (quantity >= 1)
	{
		return false;
	}

	if ((coop->intValue) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->intValue)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			SetRespawn(ent, ent->item->quantity);
		}
	}

	return true;
}

void
Use_IR(edict_t *ent, gitem_t *item) /* FS: Coop: Rogue specific */
{
	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->ir_framenum > level.framenum)
	{
		ent->client->ir_framenum += 600;
	}
	else
	{
		ent->client->ir_framenum = level.framenum + 600;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ir_start.wav"), 1, ATTN_NORM, 0);
}

void
Use_Double(edict_t *ent, gitem_t *item) /* FS: Coop: Rogue specific */
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->double_framenum > level.framenum)
	{
		ent->client->double_framenum += 300;
	}
	else
	{
		ent->client->double_framenum = level.framenum + 300;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ddamage1.wav"), 1, ATTN_NORM, 0);
}

void
Use_Compass(edict_t *ent, gitem_t *item) /* FS: Coop: Rogue specific */
{
	int ang;

	if (!ent || !item)
	{
		return;
	}

	ang = (int)(ent->client->v_angle[1]);

	if (ang < 0)
	{
		ang += 360;
	}

	gi.cprintf(ent, PRINT_HIGH, "Origin: %0.0f,%0.0f,%0.0f    Dir: %d\n",
			ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], ang);
}

void
Use_Nuke(edict_t *ent, gitem_t *item) /* FS: Coop: Rogue specific */
{
	vec3_t forward, right, start;
	float speed;

	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorCopy(ent->s.origin, start);
	speed = 100;
	fire_nuke(ent, start, forward, speed);
}

void
Use_Doppleganger(edict_t *ent, gitem_t *item) /* FS: Coop: Rogue specific */
{
	vec3_t forward, right;
	vec3_t createPt, spawnPt;
	vec3_t ang;

	if (!ent || !item)
	{
		return;
	}

	VectorClear(ang);
	ang[YAW] = ent->client->v_angle[YAW];
	AngleVectors(ang, forward, right, NULL);

	VectorMA(ent->s.origin, 48, forward, createPt);

	if (!FindSpawnPoint(createPt, ent->mins, ent->maxs, spawnPt, 32))
	{
		return;
	}

	if (!CheckGroundSpawnPoint(spawnPt, ent->mins, ent->maxs, 64, -1))
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	SpawnGrow_Spawn(spawnPt, 0);
	fire_doppleganger(ent, spawnPt, forward);
}

qboolean
Pickup_Doppleganger(edict_t *ent, edict_t *other) /* FS: Coop: Rogue specific */
{
	int quantity;

	if (!ent || !other)
	{
		return false;
	}

	if (!(deathmatch->intValue))
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (quantity >= 1)
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

qboolean
Pickup_Sphere(edict_t *ent, edict_t *other) /* FS: Coop: Rogue specific */
{
	int quantity;

	if (!ent || !other || !other->client)
	{
		return false;
	}

	if (other->client && other->client->owned_sphere)
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (((skill->intValue == 1) &&
		 (quantity >= 2)) || ((skill->intValue >= 2) && (quantity >= 1)))
	{
		return false;
	}

	if ((coop->intValue) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->intValue)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			SetRespawn(ent, ent->item->quantity);
		}

		if ((dmflags->intValue & DF_INSTANT_ITEMS))
		{
			if (ent->item->use)
			{
				ent->item->use(other, ent->item);
			}
			else
			{
				gi.dprintf(DEVELOPER_MSG_GAME, "Powerup has no use function!\n");
			}
		}
	}

	return true;
}

void
Use_Defender(edict_t *ent, gitem_t *item) /* FS: Coop: Rogue specific */
{
	if (!ent || !ent->client || !item)
	{
		return;
	}

	if (ent->client->owned_sphere)
	{
		gi.cprintf(ent, PRINT_HIGH, "Only one sphere at a time!\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	Defender_Launch(ent);
}

void
Use_Hunter(edict_t *ent, gitem_t *item) /* FS: Coop: Rogue specific */
{
	if (!ent || !ent->client || !item)
	{
		return;
	}

	if (ent->client->owned_sphere)
	{
		gi.cprintf(ent, PRINT_HIGH, "Only one sphere at a time!\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	Hunter_Launch(ent);
}

void
Use_Vengeance(edict_t *ent, gitem_t *item) /* FS: Coop: Rogue specific */
{
	if (!ent || !ent->client || !item)
	{
		return;
	}

	if (ent->client->owned_sphere)
	{
		gi.cprintf(ent, PRINT_HIGH, "Only one sphere at a time!\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	Vengeance_Launch(ent);
}

/* ====================================================================== */

void
Use_Quad(edict_t *ent, gitem_t *item)
{
	int timeout;

	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->quad_framenum += timeout;
	}
	else
	{
		ent->client->quad_framenum = level.framenum + timeout;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

/* ====================================================================== */

void Use_QuadFire (edict_t *ent, gitem_t *item) /* FS: Coop: Xatrix specific */
{
	int timeout;

	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (quad_fire_drop_timeout_hack)
	{
		timeout = quad_fire_drop_timeout_hack;
		quad_fire_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quadfire_framenum > level.framenum)
	{
		ent->client->quadfire_framenum += timeout;
	}
	else
	{
		ent->client->quadfire_framenum = level.framenum + timeout;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/quadfire1.wav"), 1, ATTN_NORM, 0);
}

void
Use_Breather(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->breather_framenum += 300;
	}
	else
	{
		ent->client->breather_framenum = level.framenum + 300;
	}
}

/* ====================================================================== */

void
Use_Envirosuit(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->enviro_framenum += 300;
	}
	else
	{
		ent->client->enviro_framenum = level.framenum + 300;
	}
}

/* ====================================================================== */

void
Use_Invulnerability(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->invincible_framenum += 300;
	}
	else
	{
		ent->client->invincible_framenum = level.framenum + 300;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

/* ====================================================================== */

void
Use_Silencer(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);
	ent->client->silencer_shots += 30;
}

/* ====================================================================== */

qboolean
Pickup_Key(edict_t *ent, edict_t *other)
{
	qboolean firstHit = true;

	if (!ent || !other)
	{
		return false;
	}

	if (coop->intValue) /* FS: Coop: Distribute keys and special items */
	{
		int i;
		edict_t *client;
		qboolean coopReturn = false;

		/* respawn any dead clients */
		for (i = 0; i < maxclients->intValue; i++)
		{
			client = g_edicts + 1 + i;

			if (!client || !client->inuse)
			{
				continue;
			}

			if ((ent->classname) && !(ent->spawnflags & ITEM_NO_TOUCH) && (!strcmp(ent->classname, "key_power_cube")))
			{
				if (client->client->pers.power_cubes &
					((ent->spawnflags & 0x0000ff00) >> 8))
				{
					continue;
				}

				client->client->pers.inventory[ITEM_INDEX(ent->item)]++;
				client->client->pers.power_cubes |=
					((ent->spawnflags & 0x0000ff00) >> 8);

				if (firstHit) /* FS: Coop: Global inventory for newcomers to server */
				{
					game.inventory[ITEM_INDEX(ent->item)]++;
					firstHit = false;
				}
			}
			else
			{
				if (client->client->pers.inventory[ITEM_INDEX(ent->item)])
				{
					continue;
				}

				client->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;

				if (firstHit) /* FS: Coop: Global inventory for newcomers to server */
				{
					game.inventory[ITEM_INDEX(ent->item)] = 1;
					firstHit = false;
				}
			}

			if(client == other) /* FS: Coop: If this client is the one who walked over then set the proper return qboolean */
				coopReturn = true;
		}

		if(ent->item && ent->item->pickup_name && other->client->pers.netname[0] && coopReturn == true)
		{
			gi.bprintf(PRINT_HIGH,"\x02[%s]: ", other->client->pers.netname);
			gi.bprintf(PRINT_HIGH, "Team, everyone has the %s!\n", ent->item->pickup_name);

			for (i = 0; i < maxclients->intValue; i++)
			{
				client = g_edicts + 1 + i;

				if (!client || !client->inuse)
				{
					continue;
				}

				gi.sound(client, CHAN_AUTO, gi.soundindex("misc/talk.wav"), 1, ATTN_NORM, 0);
			}
		}

		return coopReturn;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

/* ====================================================================== */

qboolean
Add_Ammo(edict_t *ent, gitem_t *item, int count)
{
	int index;
	int max;

	if (!ent || !item)
	{
		return false;
	}

	if (!ent->client)
	{
		return false;
	}

	if (item->tag == AMMO_BULLETS)
	{
		max = ent->client->pers.max_bullets;
	}
	else if (item->tag == AMMO_SHELLS)
	{
		max = ent->client->pers.max_shells;
	}
	else if (item->tag == AMMO_ROCKETS)
	{
		max = ent->client->pers.max_rockets;
	}
	else if (item->tag == AMMO_GRENADES)
	{
		max = ent->client->pers.max_grenades;
	}
	else if (item->tag == AMMO_CELLS)
	{
		max = ent->client->pers.max_cells;
	}
	else if (item->tag == AMMO_SLUGS)
	{
		max = ent->client->pers.max_slugs;
	}
	else if (item->tag == AMMO_FLECHETTES) /* FS: Coop: Rogue specific */
	{
		max = ent->client->pers.max_flechettes;
	}
	else if (item->tag == AMMO_PROX) /* FS: Coop: Rogue specific */
	{
		max = ent->client->pers.max_prox;
	}
	else if (item->tag == AMMO_TESLA) /* FS: Coop: Rogue specific */
	{
		max = ent->client->pers.max_tesla;
	}
	else if (item->tag == AMMO_MAGSLUG) /* FS: Coop: Xatrix specific */
	{
		max = ent->client->pers.max_magslug;
	}
	else if (item->tag == AMMO_TRAP) /* FS: Coop: Xatrix specific */
	{
		max = ent->client->pers.max_trap;
	}
// [evolve
	else if (item->tag == AMMO_LASERTRIPBOMB) /* FS: Zaero specific game dll changes */
	{
		max = ent->client->pers.max_tbombs;
	}
	else if (item->tag == AMMO_FLARES) /* FS: Zaero specific game dll changes */
	{
		max = ent->client->pers.max_flares;
	}
	else if (item->tag == AMMO_EMPNUKE) /* FS: Zaero specific game dll changes */
	{
		max = ent->client->pers.max_empnuke;
	}
	else if (item->tag == AMMO_A2K) /* FS: Zaero specific game dll changes */
	{
		max = ent->client->pers.max_a2k;
	}
	else if (item->tag == AMMO_PLASMASHIELD) /* FS: Zaero specific game dll changes */
	{
		max = ent->client->pers.max_plasmashield;
	}
// evolve]
	else
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "undefined ammo type\n");
		return false;
	}

	index = ITEM_INDEX(item);

	if (ent->client->pers.inventory[index] == max)
	{
		return false;
	}

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
	{
		ent->client->pers.inventory[index] = max;
	}

	return true;
}

qboolean
Pickup_Ammo(edict_t *ent, edict_t *other)
{
	int oldcount;
	int count;
	qboolean weapon;

	if (!ent || !other)
	{
		return false;
	}

	weapon = (ent->item->flags & IT_WEAPON);

	if ((weapon) && (dmflags->intValue & DF_INFINITE_AMMO))
	{
		count = 1000;
	}
	else if (ent->count)
	{
		count = ent->count;
	}
	else
	{
		count = ent->item->quantity;
	}

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if((game.gametype == zaero_coop) && (ent->spawnflags & 0x08)) /* FS: Zaero specific game dll changes */
	{
		if(oldcount >= count)
		{
			return false;
		}

		count -= oldcount;
	}

	if (!Add_Ammo(other, ent->item, count))
	{
		return false;
	}

	if (weapon && !oldcount)
	{
		/* don't switch to tesla */
		if ((other->client->pers.weapon != ent->item) &&
			(!deathmatch->intValue || (other->client->pers.weapon == FindItem("blaster"))) &&
			(ent->classname && strcmp(ent->classname, "ammo_tesla") != 0)) /* FS: Coop: Rogue specific -- ammo_tesla */
		{
			other->client->newweapon = ent->item;
		}
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) &&
		(deathmatch->intValue || Coop_Respawn()) ) /* FS: Coop: Added */
	{
		SetRespawn(ent, 30);
	}
	else if ((game.gametype == zaero_coop) && (ent->spawnflags & 0x04)) /* FS: Zaero specific game dll changes */
	{
		SetRespawn (ent, 15);
	}

	return true;
}

void
Drop_Ammo(edict_t *ent, gitem_t *item)
{
	edict_t *dropped;
	int index;

	if (!ent || !item)
	{
		return;
	}

	index = ITEM_INDEX(item);
	dropped = Drop_Item(ent, item);

	if (ent->client->pers.inventory[index] >= item->quantity)
	{
		dropped->count = item->quantity;
	}
	else
	{
		dropped->count = ent->client->pers.inventory[index];
	}

	if (ent->client->pers.weapon &&
		(ent->client->pers.weapon->tag == AMMO_GRENADES) &&
		(item->tag == AMMO_GRENADES) &&
		(ent->client->pers.inventory[index] - dropped->count <= 0))
	{
		gi.cprintf(ent, PRINT_HIGH, "Can't drop current weapon\n");
		G_FreeEdict(dropped);
		return;
	}

	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem(ent);
}


qboolean Pickup_A2k (edict_t *ent, edict_t *other) /* FS: Zaero specific game dll changes */
{
	// do we already have an a2k?
	if (other->client->pers.inventory[ITEM_INDEX(ent->item)] == 1)
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
	if (deathmatch->intValue)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, 30);
	}

	return true;
}

/* ====================================================================== */

void
MegaHealth_think(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (self->owner->health > self->owner->max_health)
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->intValue || Coop_Respawn()) ) /* FS: Coop: Added */
	{
		SetRespawn(self, 20);
	}
	else
	{
		G_FreeEdict(self);
	}
}

qboolean
Pickup_Health(edict_t *ent, edict_t *other)
{
	if (!ent || !other)
	{
		return false;
	}

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health >= other->max_health)
		{
			return false;
		}
	}

	other->health += ent->count;

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
		{
			other->health = other->max_health;
		}
	}

	if (ent->style & HEALTH_TIMED)
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->intValue || Coop_Respawn()) ) /* FS: Coop: Added */
		{
			SetRespawn(ent, 30);
		}
	}

	return true;
}

/* ====================================================================== */

int
ArmorIndex(edict_t *ent)
{
	if (!ent)
	{
		return 0;
	}

	if (!ent->client)
	{
		return 0;
	}

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
	{
		return jacket_armor_index;
	}

	if (ent->client->pers.inventory[combat_armor_index] > 0)
	{
		return combat_armor_index;
	}

	if (ent->client->pers.inventory[body_armor_index] > 0)
	{
		return body_armor_index;
	}

	return 0;
}

qboolean
Pickup_Armor(edict_t *ent, edict_t *other)
{
	int old_armor_index;
	gitem_armor_t *oldinfo;
	gitem_armor_t *newinfo;
	int newcount;
	float salvage;
	int salvagecount;

	if (!ent || !other)
	{
		return false;
	}

	/* get info on new armor */
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex(other);

	/* handle armor shards specially */
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
		{
			other->client->pers.inventory[jacket_armor_index] = 2;
		}
		else
		{
			other->client->pers.inventory[old_armor_index] += 2;
		}
	}

	/* if player has no armor, just use it */
	else if (!old_armor_index)
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] =
			newinfo->base_count;
	}

	/* use the better armor */
	else
	{
		/* get info on old armor */
		if (old_armor_index == jacket_armor_index)
		{
			oldinfo = &jacketarmor_info;
		}
		else if (old_armor_index == combat_armor_index)
		{
			oldinfo = &combatarmor_info;
		}
		else
		{
			oldinfo = &bodyarmor_info;
		}

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			/* calc new armor values */
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage * other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;

			if (newcount > newinfo->max_count)
			{
				newcount = newinfo->max_count;
			}

			/* zero count of old armor so it goes away */
			other->client->pers.inventory[old_armor_index] = 0;

			/* change armor to new item with computed value */
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
		}
		else
		{
			/* calc new armor values */
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[old_armor_index] + salvagecount;

			if (newcount > oldinfo->max_count)
			{
				newcount = oldinfo->max_count;
			}

			/* if we're already maxed out then we don't need the new armor */
			if (other->client->pers.inventory[old_armor_index] >= newcount)
			{
				return false;
			}

			/* update current armor value */
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->intValue || Coop_Respawn()) ) /* FS: Coop: Added */
	{
		SetRespawn(ent, 20);
	}

	return true;
}

/* ====================================================================== */

int
PowerArmorType(edict_t *ent)
{
	if (!ent)
	{
		return POWER_ARMOR_NONE;
	}

	if (!ent->client)
	{
		return POWER_ARMOR_NONE;
	}

	if (!(ent->flags & FL_POWER_ARMOR))
	{
		return POWER_ARMOR_NONE;
	}

	if (ent->client->pers.inventory[power_shield_index] > 0)
	{
		return POWER_ARMOR_SHIELD;
	}

	if (ent->client->pers.inventory[power_screen_index] > 0)
	{
		return POWER_ARMOR_SCREEN;
	}

	return POWER_ARMOR_NONE;
}

void
Use_PowerArmor(edict_t *ent, gitem_t *item)
{
	int index;

	if (!ent || !ent->client || !item)
	{
		return;
	}

	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;

		if (coop->intValue) /* FS: Save power armor state for respawn. */
		{
			ent->client->resp.coop_respawn.savedFlags &= ~FL_POWER_ARMOR;
		}

		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("cells"));

		if (!ent->client->pers.inventory[index])
		{
			gi.cprintf(ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}

		ent->flags |= FL_POWER_ARMOR;

		if (coop->intValue) /* FS: Save power armor state for respawn. */
		{
			ent->client->resp.coop_respawn.savedFlags |= FL_POWER_ARMOR;
		}

		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean
Pickup_PowerArmor(edict_t *ent, edict_t *other)
{
	int quantity;

	if (!ent || !other)
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	/* Knightmare- only allow players to pick up 2 power shields in coop. */
	/* This prevents dropped item spamming. */
	if ((coop->intValue) && (quantity >= 2))
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->intValue || Coop_Respawn() ) /* FS: Coop: Added */
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			SetRespawn(ent, ent->item->quantity);
		}

		/* auto-use for DM only if we didn't already have one */
		if (!quantity)
		{
			ent->item->use(other, ent->item);
		}
	}

	return true;
}

void
Drop_PowerArmor(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	if ((ent->flags & FL_POWER_ARMOR) &&
		(ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
	{
		Use_PowerArmor(ent, item);
	}

	Drop_General(ent, item);
}

//======================================================================

qboolean Pickup_PlasmaShield(edict_t *ent, edict_t *other) /* FS: Zaero specific game dll changes */
{
  if(other->client->pers.inventory[ITEM_INDEX(ent->item)])
  {
    return false;
  }

	other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;

	if (deathmatch->intValue)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
	}

	return true;
}


qboolean Pickup_Visor(edict_t *ent, edict_t *other) /* FS: Zaero specific game dll changes */
{
	// do we already have a visor?
	if (other->client->pers.inventory[ITEM_INDEX(ent->item)] == 1 &&
		other->client->pers.visorFrames == 300)
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;

	if (ent->spawnflags & DROPPED_ITEM)
		other->client->pers.visorFrames += ent->visorFrames;
	else
		other->client->pers.visorFrames = 300;

	if (deathmatch->intValue)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, 30);
	}

	return true;
}

void Drop_Visor(edict_t *ent, gitem_t *item) /* FS: Zaero specific game dll changes */
{
	edict_t *visor = Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
	ValidateSelectedItem (ent);
	visor->visorFrames = ent->client->pers.visorFrames;
	ent->client->pers.visorFrames = 0;
}

/* ====================================================================== */

void
Touch_Item(edict_t *ent, edict_t *other, cplane_t *plane /* unused */, csurface_t *surf /* unused */)
{
	qboolean taken;

	if (!ent || !other)
	{
		return;
	}

	if (!other->client)
	{
		return;
	}

	if (other->health < 1)
	{
		return; /* dead people can't pickup */
	}

	if (!ent->item->pickup)
	{
		return; /* not a grabbable item? */
	}

	taken = ent->item->pickup(ent, other);

	if (taken)
	{
		/* flash the screen */
		other->client->bonus_alpha = 0.25;

		/* show icon and name on status bar */
		other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + ITEM_INDEX(ent->item);
		other->client->pickup_msg_time = level.time + 3.0;

		/* change selected item */
		if (ent->item->use)
		{
			other->client->pers.selected_item =
				other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);
		}

		if (ent->item->pickup == Pickup_Health)
		{
			if (ent->count == 2)
			{
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_NORM, 0);
			}
			else if (ent->count == 10)
			{
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
			}
			else if (ent->count == 25)
			{
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
			}
			else /* (ent->count == 100) */
			{
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
			}
		}
		else if (ent->item->pickup_sound)
		{
			gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
		}
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets(ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (!taken)
	{
		return;
	}

	if (!((coop->intValue) && (ent->item->flags & IT_STAY_COOP)) ||
		(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
		{
			ent->flags &= ~FL_RESPAWN;
		}
		else
		{
			G_FreeEdict(ent);
		}
	}
}

/* ====================================================================== */

void
drop_temp_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!ent || !other)
	{
		return;
	}

	if (other == ent->owner)
	{
		return;
	}

	/* plane and surf are unused in Touch_Item
	   but since the function is part of the
	   game <-> client interface dropping
	   them is too much pain. */
	Touch_Item(ent, other, plane, surf);
}

void
drop_make_touchable(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	ent->touch = Touch_Item;

	if (deathmatch->intValue)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}

edict_t *
Drop_Item(edict_t *ent, gitem_t *item)
{
	edict_t *dropped;
	vec3_t forward, right;
	vec3_t offset;

	if (!ent || !item)
	{
		return NULL;
	}

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	if(!coop->intValue || (!(item->flags & IT_STAY_COOP) && !(item->flags & IT_KEY))) /* FS: Zaero stupidly sets it so some enemies drop the key.  So they will drop it once killed and you only get one chance to pick it up. */
	{
		dropped->spawnflags = DROPPED_ITEM;
	}
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW | RF_IR_VISIBLE; /* FS: Coop: Rogue specific add of RF_IR_VISIBLE.  Probably OK as-is. */

	if (game.gametype != rogue_coop) /* FS: Coop: Vanilla and Xatrix specific */
	{
		if (rand() > 0.5)
		{
			dropped->s.angles[1] += rand()*45;
		}
		else
		{
			dropped->s.angles[1] -= rand()*45;
		}
		VectorSet(dropped->mins, -16, -16, -16);
		VectorSet(dropped->maxs, 16, 16, 16);
	}
	else
	{
		VectorSet(dropped->mins, -15, -15, -15);
		VectorSet(dropped->maxs, 15, 15, 15);
	}
	gi.setmodel(dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t trace;

		AngleVectors(ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource(ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace(ent->s.origin, dropped->mins, dropped->maxs,
				dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy(trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors(ent->s.angles, forward, right, NULL);
		VectorCopy(ent->s.origin, dropped->s.origin);
	}

	VectorScale(forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity(dropped);

	return dropped;
}

void
Use_Item(edict_t *ent, edict_t *other /* unused */, edict_t *activator /* unused */)
{
	if (!ent)
	{
		return;
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity(ent);
}

/* ====================================================================== */

void
droptofloor(edict_t *ent)
{
	trace_t tr;
	vec3_t dest;
	float *v;

	if (!ent)
	{
		return;
	}

	v = tv(-15, -15, -15);
	VectorCopy(v, ent->mins);
	v = tv(15, 15, 15);
	VectorCopy(v, ent->maxs);

	if (ent->model)
	{
		gi.setmodel(ent, ent->model);
	}
	else if (ent->item->world_model) /* FS: Coop: Rogue specific sanity check.  Looks OK to keep. */
	{
		gi.setmodel(ent, ent->item->world_model);
	}
	else
	{
		/* FS: FIXME: Should we bail? */
	}

	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;
	ent->touch = Touch_Item;

	v = tv(0, 0, -128);
	VectorAdd(ent->s.origin, v, dest);

	tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);

	if (tr.startsolid)
	{
		if ((game.gametype == xatrix_coop) && (ent->classname) && (!strcmp(ent->classname, "foodcube"))) /* FS: Coop: Xatrix specific */
		{
			VectorCopy(ent->s.origin, tr.endpos);
			ent->velocity[2] = 0;
		}
		else
		{
			gi.dprintf(DEVELOPER_MSG_GAME, "droptofloor: %s startsolid at %s\n",
					ent->classname, vtos(ent->s.origin));
			G_FreeEdict(ent);
			return;
		}
	}

	VectorCopy(tr.endpos, ent->s.origin);

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;

		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}

	gi.linkentity(ent);
}

/*
 * Precaches all data needed for a given item.
 * This will be called for each item spawned in a level,
 * and for each item in each client's inventory.
 */
void
PrecacheItem(gitem_t *it)
{
	char *s, *start;
	char data[MAX_QPATH];
	int len;
	gitem_t *ammo;

	if (!it)
	{
		return;
	}

	if (it->pickup_sound)
	{
		gi.soundindex(it->pickup_sound);
	}

	if (it->world_model)
	{
		gi.modelindex(it->world_model);
	}

	if (it->view_model)
	{
		gi.modelindex(it->view_model);
	}

	if (it->icon)
	{
		gi.imageindex(it->icon);
	}

	/* parse everything for its ammo */
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem(it->ammo);

		if (ammo != it)
		{
			PrecacheItem(ammo);
		}
	}

	/* parse the space seperated precache string for other items */
	s = it->precaches;

	if (!s || !s[0])
	{
		return;
	}

	while (*s)
	{
		start = s;

		while (*s && *s != ' ')
		{
			s++;
		}

		len = s - start;

		if ((len >= MAX_QPATH) || (len < 5))
		{
			gi.error("PrecacheItem: %s has bad precache string", it->classname);
		}

		memcpy(data, start, len);
		data[len] = 0;

		if (*s)
		{
			s++;
		}

		/* determine type based on extension */
		if (!strcmp(data + len - 3, "md2"))
		{
			gi.modelindex(data);
		}
		else if (!strcmp(data + len - 3, "sp2"))
		{
			gi.modelindex(data);
		}
		else if (!strcmp(data + len - 3, "wav"))
		{
			gi.soundindex(data);
		}

		if (!strcmp(data + len - 3, "pcx"))
		{
			gi.imageindex(data);
		}
	}
}

/*
 * Create the item marked for spawn creation
 */
void
Item_TriggeredSpawn(edict_t *self, edict_t *other /* unused */, edict_t *activator /* unused */) /* FS: Coop: Rogue specific */
{
	if (!self)
	{
		return;
	}

	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;

	if ((self->classname) && (strcmp(self->classname, "key_power_cube")))
	{
		self->spawnflags = 0;
	}

	droptofloor(self);
}

/*
 * Set up an item to spawn in later.
 */
void
SetTriggeredSpawn(edict_t *ent) /* FS: Coop: Rogue specific */
{
	if (!ent)
	{
		return;
	}

	/* don't do anything on key_power_cubes. */
	if ((ent->classname) && (!strcmp(ent->classname, "key_power_cube")))
	{
		return;
	}

	ent->think = NULL;
	ent->nextthink = 0;
	ent->use = Item_TriggeredSpawn;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
}

/*
 * ============
 * Sets the clipping size and
 * plants the object on the floor.
 *
 * Items can't be immediately dropped
 * to floor, because they might be on
 * an entity that hasn't spawned yet.
 * ============
 */
void
SpawnItem(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	if (game.gametype == zaero_coop) /* FS: Zaero specific game dll changes */
	{
		PrecacheItem (item);

		if (ent->spawnflags)
		{
			if (!(item->flags & IT_KEY) && !(item->flags & IT_AMMO))
			{
				ent->spawnflags = 0;
				gi.dprintf(DEVELOPER_MSG_GAME, "%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
			}
		}
	}
	else
	{
		if ( ((game.gametype == rogue_coop) && (ent->spawnflags > 1)) || /* FS: Coop: Rogue specific */
			((game.gametype != rogue_coop) && (ent->spawnflags)) )
		{
			if ((ent->classname) && (strcmp(ent->classname, "key_power_cube") != 0))
			{
				ent->spawnflags = 0;
				gi.dprintf(DEVELOPER_MSG_GAME, "%s at %s has invalid spawnflags set\n",
						ent->classname, vtos(ent->s.origin));
			}
		}
	}

	/* some items will be prevented in deathmatch */
	if (deathmatch->intValue)
	{
		if (dmflags->intValue & DF_NO_ARMOR)
		{
			if ((item->pickup == Pickup_Armor) ||
				(item->pickup == Pickup_PowerArmor))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if (dmflags->intValue & DF_NO_ITEMS)
		{
			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict(ent);
				return;
			}

			if (item->pickup == Pickup_Sphere) /* FS: Coop: Rogue specific */
			{
				G_FreeEdict(ent);
				return;
			}

			if (item->pickup == Pickup_Doppleganger) /* FS: Coop: Rogue specific */
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if (dmflags->intValue & DF_NO_HEALTH)
		{
			if ((item->pickup == Pickup_Health) ||
				(item->pickup == Pickup_Adrenaline) ||
				(item->pickup == Pickup_AncientHead))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if (dmflags->intValue & DF_INFINITE_AMMO)
		{
			if ((item->flags == IT_AMMO) ||
				(ent->classname && strcmp(ent->classname, "weapon_bfg") == 0))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if (dmflags->intValue & DF_NO_MINES) /* FS: Coop: Rogue specific */
		{
			if (ent->classname && (!strcmp(ent->classname, "ammo_prox") ||
				!strcmp(ent->classname, "ammo_tesla")))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if (dmflags->intValue & DF_NO_NUKES) /* FS: Coop: Rogue specific */
		{
			if (ent->classname && !strcmp(ent->classname, "ammo_nuke"))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if (dmflags->intValue & DF_NO_SPHERES) /* FS: Coop: Rogue specific */
		{
			if (item->pickup == Pickup_Sphere)
			{
				G_FreeEdict(ent);
				return;
			}
		}
	}

	/* DM only items */
	if (!deathmatch->intValue) /* FS: Coop: Rogue specific */
	{
		if ((item->pickup == Pickup_Doppleganger) ||
			(item->pickup == Pickup_Nuke))
		{
			G_FreeEdict(ent);
			return;
		}

		if ((item->use == Use_Vengeance) || (item->use == Use_Hunter))
		{
			G_FreeEdict(ent);
			return;
		}
	}

	PrecacheItem(item);

	if ((coop->intValue) && (ent->classname) && (strcmp(ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

	/* don't let them drop items that stay in a coop game */
	if ((coop->intValue) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME; /* items start after other solids */
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;

	if (ent->model)
	{
		gi.modelindex(ent->model);
	}

	if ((game.gametype == rogue_coop) && (ent->spawnflags & 1)) /* FS: Coop: Rogue specific */
	{
		SetTriggeredSpawn(ent);
	}
}

/* ====================================================================== */

gitem_t itemlist[] = {
	{
		NULL
	}, /* leave index 0 alone */

	//
	// ARMOR
	//

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_body",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/body/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_bodyarmor",
/* pickup */	"Body Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&bodyarmor_info,
		ARMOR_BODY,
/* precache */ ""
	},

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_combat",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/combat/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_combatarmor",
/* pickup */	"Combat Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&combatarmor_info,
		ARMOR_COMBAT,
/* precache */ ""
	},

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_jacket",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/jacket/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Jacket Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&jacketarmor_info,
		ARMOR_JACKET,
/* precache */ ""
	},

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_shard",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"models/items/armor/shard/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Armor Shard",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
	},


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_screen",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powerscreen",
/* pickup */	"Power Screen",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_shield",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/shield/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powershield",
/* pickup */	"Power Shield",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */ "misc/power2.wav misc/power1.wav"
	},


	//
	// WEAPONS 
	//

/*QUAKED weapon_hand (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_push", /* FS: Zaero specific game dll changes */
		NULL,
		Use_Weapon,
		NULL,
		Action_Push,
/* pickup sound */	NULL,
		NULL, 0,
		"models/weapons/v_hand/tris.md2",
/* icon */		NULL,
/* pickup */	"Push",
		0,
		0,
		NULL,
		IT_WEAPON|IT_ZAERO|IT_STAY_COOP,
		WEAP_NONE,
		NULL,
		0,
/* precache */ "weapons/push/contact.wav",// weapons/push/grunt.wav",
/* hide flags */ HIDE_FROM_INVENTORY | HIDE_DONT_KEEP | HIDE_FROM_SELECTION
	},

/*QUAKED weapon_flaregun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_flaregun", /* FS: Zaero specific game dll changes */
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_FlareGun,
		"misc/w_pkup.wav",
		"models/weapons/g_flare/tris.md2", EF_ROTATE,
		"models/weapons/v_flare/tris.md2",
/* icon */		"w_flare",
/* pickup */	"Flare Gun",
		0,
		1,
		"Flares",
		IT_WEAPON|IT_ZAERO|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "models/weapons/v_flare/tris.md2 models/objects/flare/tris.md2 weapons/flare/shoot.wav weapons/flare/flarehis.wav"
	},

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster",
		NULL,
		Use_Weapon2, /* FS: Coop: Rogue specific */
		NULL,
		Weapon_Blaster,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Blaster",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SuperShotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */	"Super Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
		"weapons/sshotf1b.wav"
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Machinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"Machinegun",
		0,
		1,
		"Bullets",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun",
		Pickup_Weapon,
		Use_Weapon2, /* FS: Coop: Rogue specific */
		Drop_Weapon,
		Weapon_Chaingun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Chaingun",
		0,
		1,
		"Bullets",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
	},

	/* QUAKED weapon_etf_rifle (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"weapon_etf_rifle",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ETF_Rifle,
		"misc/w_pkup.wav",
		"models/weapons/g_etf_rifle/tris.md2", EF_ROTATE,
		"models/weapons/v_etf_rifle/tris.md2",
		"w_etf_rifle",
		"ETF Rifle",
		0,
		1,
		"Flechettes",
		IT_WEAPON|IT_ROGUE, /* FS: Coop: Rogue specific */
		WEAP_ETFRIFLE,
		NULL,
		0,
		"weapons/nail1.wav models/proj/flechette/tris.md2",
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Grenades",
/* width */		3,
		5,
		"grenades",
		IT_AMMO | IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED ammo_ired (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_ired", /* FS: Zaero specific game dll changes */
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_LaserTripBomb,
		"misc/am_pkup.wav",
		"models/items/ammo/ireds/tris.md2", 0,
		"models/weapons/v_ired/tris.md2",
/* icon */		"w_ired",
/* pickup */	"IRED",
/* width */		3,
		3,
		"IRED",
		IT_AMMO|IT_ZAERO|IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		AMMO_LASERTRIPBOMB,
/* precache */ "models/weapons/v_ired/hand.md2 models/objects/ired/tris.md2 modes/objects models/objects/shrapnel/tris.md2 "
	"weapons/ired/las_set.wav weapons/ired/las_arm.wav "
	/*"weapons/ired/las_tink.wav "weapons/ired/las_trig.wav "*/
	/*"weapons/ired/las_glow.wav"*/,
	},

/*QUAKED ammo_trap (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_trap",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Trap,
		"misc/am_pkup.wav",
		"models/weapons/g_trap/tris.md2", EF_ROTATE,
		"models/weapons/v_trap/tris.md2",
/* icon */		"a_trap",
/* pickup */	"Trap",
/* width */		3,
		1,
		"trap",
		IT_AMMO|IT_WEAPON|IT_XATRIX, /* FS: Coop: Xatrix specific */
		0,
		NULL,
		AMMO_TRAP,
/* precache */ "weapons/trapcock.wav weapons/traploop.wav weapons/trapsuck.wav weapons/trapdown.wav"
// "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

	
/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon2, /* FS: Coop: Rogue specific */
		Drop_Weapon,
		Weapon_GrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

	/* QUAKED weapon_proxlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"weapon_proxlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ProxLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_plaunch/tris.md2", EF_ROTATE,
		"models/weapons/v_plaunch/tris.md2",
		"w_proxlaunch",
		"Prox Launcher",
		0,
		1,
		"Prox",
		IT_WEAPON|IT_ROGUE, /* FS: Coop: Rogue specific */
		WEAP_PROXLAUNCH,
		NULL,
		AMMO_PROX,
		"weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav weapons/proxwarn.wav weapons/proxopen.wav",
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster",
		Pickup_Weapon,
		Use_Weapon2, /* FS: Coop: Xatrix and Rogue specific */
		Drop_Weapon,
		Weapon_HyperBlaster,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"HyperBlaster",
		0,
		1,
		"Cells",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},

/*QUAKED weapon_boomer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/

	{
		"weapon_boomer", /* FS: Coop: Xatrix specific */
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Ionripper,
		"misc/w_pkup.wav",
		"models/weapons/g_boom/tris.md2", EF_ROTATE,
		"models/weapons/v_boomer/tris.md2",
/* icon */	"w_ripper",
/* pickup */ "Ionripper",
		0,
		2,
		"Cells",
		IT_WEAPON|IT_XATRIX,
		WEAP_BOOMER,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav weapons/rippfire.wav"
	},
// END 14-APR-98

	/* QUAKED weapon_plasmabeam (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"weapon_plasmabeam", /* FS: Coop: Rogue specific */
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Heatbeam,
		"misc/w_pkup.wav",
		"models/weapons/g_beamer/tris.md2", EF_ROTATE,
		"models/weapons/v_beamer/tris.md2",
		"w_heatbeam",
		"Plasma Beam",
		0,
		2,
		"Cells",
		IT_WEAPON|IT_ROGUE,
		WEAP_PLASMA,
		NULL,
		0,
		"models/weapons/v_beamer2/tris.md2 weapons/bfg__l1a.wav models/proj/beam/tris.md2", /* FS: Missing beam precache */
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun",
		Pickup_Weapon,
		Use_Weapon2, /* FS: Coop: Xatrix specific */
		Drop_Weapon,
		Weapon_Railgun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Railgun",
		0,
		1,
		"Slugs",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav"
	},

// RAFAEL 14-APR-98
/*QUAKED weapon_phalanx (.3 .3 1) (-16 -16 -16) (16 16 16)
*/

	{
		"weapon_phalanx",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Phalanx,
		"misc/w_pkup.wav",
		"models/weapons/g_shotx/tris.md2", EF_ROTATE,
		"models/weapons/v_shotx/tris.md2",
/* icon */	"w_phallanx",
/* pickup */ "Phalanx",
		0,
		1,
		"Mag Slug",
		IT_WEAPON|IT_XATRIX, /* FS: Coop: Xatrix specific */
		WEAP_PHALANX,
		NULL,
		0,
/* precache */ "weapons/plasshot.wav"
	},

/*QUAKED weapon_sniperrifle (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_sniperrifle",  /* FS: Zaero specific game dll changes */
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SniperRifle,
		"misc/w_pkup.wav",
		"models/weapons/g_sniper/tris.md2", EF_ROTATE,
		"models/weapons/v_sniper/tris.md2",
/* icon */		"w_sniper",
/* pickup */	"Sniper Rifle",
		0,
		3,
		"Slugs",
		IT_WEAPON|IT_ZAERO|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "models/weapons/v_sniper/scope/tris.md2 models/weapons/v_sniper/dmscope/tris.md2 weapons/sniper/beep.wav weapons/sniper/fire.wav "
		/*"weapons/sniper/snip_act.wav weapons/sniper/snip_bye.wav"*/,
		HIDE_FROM_SELECTION
	},


/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BFG,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"BFG10K",
		0,
		50,
		"Cells",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
	},

	/* QUAKED weapon_chainfist (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"weapon_chainfist", /* FS: Coop: Rogue specific */
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ChainFist,
		"misc/w_pkup.wav",
		"models/weapons/g_chainf/tris.md2", EF_ROTATE,
		"models/weapons/v_chainf/tris.md2",
		"w_chainfist",
		"Chainfist",
		0,
		0,
		NULL,
		IT_WEAPON | IT_MELEE | IT_ROGUE,
		WEAP_CHAINFIST,
		NULL,
		1,
		"weapons/sawidle.wav weapons/sawhit.wav",
	},

	/* QUAKED weapon_disintegrator (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"weapon_disintegrator", /* FS: Coop: Rogue specific */
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Disintegrator,
		"misc/w_pkup.wav",
		"models/weapons/g_dist/tris.md2", EF_ROTATE,
		"models/weapons/v_dist/tris.md2",
		"w_disintegrator",
		"Disruptor",
		0,
		1,
		"Rounds",
		IT_WEAPON | IT_STAY_COOP | IT_ROGUE,
		WEAP_DISRUPTOR,
		NULL,
		1,
		"models/items/spawngro/tris.md2 models/proj/disintegrator/tris.md2 weapons/disrupt.wav weapons/disint2.wav weapons/disrupthit.wav",
	},

/*QUAKED weapon_soniccannon  (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_soniccannon",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SonicCannon,
		"misc/w_pkup.wav",
		"models/weapons/g_sonic/tris.md2", EF_ROTATE,
		"models/weapons/v_sonic/tris.md2",
/* icon */		"w_sonic",
/* pickup */	"Sonic Cannon",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_ZAERO|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/sonic/sc_warm.wav weapons/sonic/sc_cool.wav weapons/sonic/sc_fire.wav" // weapons/sonic/sc_act.wav weapons/sonic/sc_dact.wav  
	},

/*QUAKED ammo_a2k (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_a2k", /* FS: Zaero specific game dll changes */
		Pickup_A2k,
		Use_Weapon,
		Drop_Ammo,
		Weapon_A2k,
		"misc/am_pkup.wav",
		"models/weapons/g_a2k/tris.md2", 0,
		"models/weapons/v_a2k/tris.md2",
/* icon */		"w_a2k",
/* pickup */	"A2K",
/* width */		1,
		1,
		"A2k",
		IT_POWERUP|IT_ZAERO,
		WEAP_NONE,
		NULL,
		AMMO_A2K,
/* precache */ "weapons/a2k/countdn.wav weapons/a2k/ak_exp01.wav",
		HIDE_FROM_SELECTION
	},

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_flares (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_flares", /* FS: Zaero specific game dll changes */
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/flares/tris.md2", 0,
		NULL,
/* icon */		"a_flares",
/* pickup */	"Flares",
/* width */		3,
		3,
		NULL,
		IT_AMMO|IT_ZAERO,
		0,
		NULL,
		AMMO_FLARES,
/* precache */ ""
	},

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_shells",
/* pickup */	"Shells",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_bullets",
/* pickup */	"Bullets",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/cells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_cells",
/* pickup */	"Cells",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_rockets",
/* pickup */	"Rockets",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/slugs/medium/tris.md2", 0,
		NULL,
/* icon */		"a_slugs",
/* pickup */	"Slugs",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SLUGS,
/* precache */ ""
	},

/*QUAKED ammo_empnuke (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_empnuke", /* FS: Zaero specific game dll changes */
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_EMPNuke,
		"misc/am_pkup.wav",
		"models/weapons/g_enuke/tris.md2", EF_ROTATE,
		"models/weapons/v_enuke/tris.md2",
/* icon */		"w_enuke",
/* pickup */	"EMPNuke",
/* width */		3,
		1,
		"EMPNuke",
		IT_AMMO|IT_ZAERO,
		0,
		NULL,
		AMMO_EMPNUKE,
/* precache */ "items/empnuke/emp_trg.wav"  //items/empnuke/emp_act.wav items/empnuke/emp_spin.wav items/empnuke/emp_idle.wav 
	},

	/* QUAKED ammo_flechettes (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"ammo_flechettes",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/ammo/am_flechette/tris.md2", 0,
		NULL,
		"a_flechettes",
		"Flechettes",
		3,
		50,
		NULL,
		IT_AMMO | IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		AMMO_FLECHETTES
	},

	/* QUAKED ammo_prox (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"ammo_prox",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/ammo/am_prox/tris.md2", 0,
		NULL,
		"a_prox",
		"Prox",
		3,
		5,
		NULL,
		IT_AMMO | IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		AMMO_PROX,
		"models/weapons/g_prox/tris.md2 weapons/proxwarn.wav"
	},

	/* QUAKED ammo_tesla (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"ammo_tesla",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Tesla,
		"misc/am_pkup.wav",
		"models/ammo/am_tesl/tris.md2", 0,
		"models/weapons/v_tesla/tris.md2",
		"a_tesla",
		"Tesla",
		3,
		5,
		"Tesla",
		IT_AMMO | IT_WEAPON | IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		AMMO_TESLA,
		"models/weapons/v_tesla2/tris.md2 weapons/teslaopen.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav models/weapons/g_tesla/tris.md2 models/proj/lightning/tris.md2" /* FS: Was misisng lightning.  Needed for precache */
	},

	/* QUAKED ammo_nuke (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"ammo_nuke",
		Pickup_Nuke,
		Use_Nuke,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/g_nuke/tris.md2", EF_ROTATE,
		NULL,
		"p_nuke",
		"A-M Bomb",
		3,
		300,
		"A-M Bomb",
		IT_POWERUP | IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
		"weapons/nukewarn2.wav world/rumble.wav models/objects/r_explode2/tris.md2" /* FS: Was missing r_explode2.  Needed for precache */
	},

	/* QUAKED ammo_disruptor (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"ammo_disruptor",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/ammo/am_disr/tris.md2", 0,
		NULL,
		"a_disruptor",
		"Rounds",
		3,
		15,
		NULL,
		IT_AMMO | IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
	},


/*QUAKED ammo_magslug (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_magslug",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/objects/ammo/tris.md2", 0,
		NULL,
/* icon */		"a_mslugs",
/* pickup */	"Mag Slug",
/* width */		3,
		10,
		NULL,
		IT_AMMO|IT_XATRIX, /* FS: Coop: Xatrix specific */
		0,
		NULL,
		AMMO_MAGSLUG,
/* precache */ ""
	},

	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quad", 
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_quad",
/* pickup */	"Quad Damage",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
	},

/*QUAKED item_quadfire (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quadfire", 
		Pickup_Powerup,
		Use_QuadFire,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quadfire/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_quadfire",

/* pickup */	"DualFire Damage",
/* width */		2,
		60,
		NULL,
		IT_POWERUP|IT_XATRIX, /* FS: Coop: Xatrix specific */
		0,
		NULL,
		0,
/* precache */ "items/quadfire1.wav items/quadfire2.wav items/quadfire3.wav"
	},

	
/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_invulnerability",
/* pickup */	"Invulnerability",
/* width */		2,
		300,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_silencer",
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_silencer",
/* pickup */	"Silencer",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Rebreather",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP | IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_enviro",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/enviro/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_envirosuit",
/* pickup */	"Environment Suit",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP | IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_visor (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_visor", /* FS: Zaero specific game dll changes */
		Pickup_Visor,
		Use_Visor,
		Drop_Visor,
		NULL,
		"items/pkup.wav",
		"models/items/visor/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_visor",
/* pickup */	"Visor",
/* width */		1,
		30,
		"Cells",
		IT_STAY_COOP|IT_POWERUP|IT_ZAERO,
		0,
		NULL,
		0,
/* precache */ "items/visor/act.wav items/visor/deact.wav"// items/visor/next.wav"
	},

  
/*QUAKED ammo_plasmashield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_plasmashield", /* FS: Zaero specific game dll changes */
		Pickup_Ammo,
		Use_PlasmaShield,
		Drop_Ammo,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/plasma/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_plasma",
/* pickup */	"Plasma Shield",
/* width */		1,
		5,
		"",
		IT_AMMO|IT_ZAERO,
		0,
		NULL,
		AMMO_PLASMASHIELD,
/* precache */ "items/plasmashield/psactive.wav sprites/plasmashield_fixed.sp2"
	},

/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16)
Special item that gives +2 to maximum health
*/
	{
		"item_ancient_head",
		Pickup_AncientHead,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/c_head/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_fixme",
/* pickup */	"Ancient Head",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
*/
	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_adrenaline",
/* pickup */	"Adrenaline",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/band/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_bandolier",
/* pickup */	"Bandolier",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_pack",
		Pickup_Pack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_pack",
/* pickup */	"Ammo Pack",
/* width */		2,
		180,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

	/* QUAKED item_ir_goggles (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"item_ir_goggles",
		Pickup_Powerup,
		Use_IR,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/goggles/tris.md2", EF_ROTATE,
		NULL,
		"p_ir",
		"IR Goggles",
		2,
		60,
		NULL,
		IT_POWERUP|IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
		"misc/ir_start.wav"
	},

	/* QUAKED item_double (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"item_double",
		Pickup_Powerup,
		Use_Double,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/ddamage/tris.md2", EF_ROTATE,
		NULL,
		"p_double",
		"Double Damage",
		2,
		60,
		NULL,
		IT_POWERUP|IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
		"misc/ddamage1.wav misc/ddamage2.wav misc/ddamage3.wav"
	},

	/* QUAKED item_compass (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"item_compass",
		Pickup_Powerup,
		Use_Compass,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/objects/fire/tris.md2", EF_ROTATE,
		NULL,
		"p_compass",
		"compass",
		2,
		60,
		NULL,
		IT_POWERUP|IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
	},

	/* QUAKED item_sphere_vengeance (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"item_sphere_vengeance",
		Pickup_Sphere,
		Use_Vengeance,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/vengnce/tris.md2", EF_ROTATE,
		NULL,
		"p_vengeance",
		"vengeance sphere",
		2,
		60,
		NULL,
		IT_POWERUP|IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
		"spheres/v_idle.wav"
	},

	/* QUAKED item_sphere_hunter (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"item_sphere_hunter",
		Pickup_Sphere,
		Use_Hunter,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/hunter/tris.md2", EF_ROTATE,
		NULL,
		"p_hunter",
		"hunter sphere",
		2,
		120,
		NULL,
		IT_POWERUP|IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
		"spheres/h_idle.wav spheres/h_active.wav spheres/h_lurk.wav"
	},

	/* QUAKED item_sphere_defender (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"item_sphere_defender",
		Pickup_Sphere,
		Use_Defender,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/defender/tris.md2", EF_ROTATE,
		NULL,
		"p_defender",
		"defender sphere",
		2,
		60,
		NULL,
		IT_POWERUP|IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
		"models/proj/laser2/tris.md2 models/items/shell/tris.md2 spheres/d_idle.wav"
	},

	/* QUAKED item_doppleganger (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"item_doppleganger",
		Pickup_Doppleganger,
		Use_Doppleganger,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/dopple/tris.md2",
		EF_ROTATE,
		NULL,
		"p_doppleganger",
		"Doppleganger",
		0,
		90,
		NULL,
		IT_POWERUP|IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
		"models/objects/dopplebase/tris.md2 models/items/spawngro2/tris.md2 models/items/hunter/tris.md2 models/items/vengnce/tris.md2",
	},

	{
		NULL,
		Tag_PickupToken,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/tagtoken/tris.md2",
		EF_ROTATE | EF_TAGTRAIL,
		NULL,
		"i_tagtoken",
		"Tag Token",
		0,
		0,
		NULL,
		IT_POWERUP | IT_NOT_GIVEABLE |IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		1,
		NULL,
	},


	//
	// KEYS
	//
/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
key for computer centers
*/
	{
		"key_data_cd",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"k_datacd",
		"Data CD",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
warehouse circuits
*/
	{
		"key_power_cube",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"k_powercube",
		"Power Cube",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the entrance of jail3
*/
	{
		"key_pyramid",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"Pyramid Key",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the city computer
*/
	{
		"key_data_spinner",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/spinner/tris.md2", EF_ROTATE,
		NULL,
		"k_dataspin",
		"Data Spinner",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
	{
		"key_pass",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
	{
		"key_blue_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"k_bluekey",
		"Blue Key",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - red
*/
	{
		"key_red_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/red_key/tris.md2", EF_ROTATE,
		NULL,
		"k_redkey",
		"Red Key",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_green_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
	{
		"key_green_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/green_key/tris.md2", EF_ROTATE,
		NULL,
		"k_green",
		"Green Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_XATRIX, /* FS: Coop: Xatrix specific */
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_commander_head",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
/* icon */		"k_comhead",
/* pickup */	"Commander's Head",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_airstrike_target",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_airstrike",
/* pickup */	"Airstrike Marker",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

	/* QUAKED key_nuke_container (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"key_nuke_container",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/weapons/g_nuke/tris.md2",
		EF_ROTATE,
		NULL,
		"i_contain",
		"Antimatter Pod",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY | IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
		NULL,
	},

	/* QUAKED key_nuke (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN */
	{
		"key_nuke",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/weapons/g_nuke/tris.md2",
		EF_ROTATE,
		NULL,
		"i_nuke",
		"Antimatter Bomb",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY| IT_ROGUE, /* FS: Coop: Rogue specific */
		0,
		NULL,
		0,
		NULL,
	},

// extra zaero keys
/*QUAKED key_landing_arena (0 .5 .8) (-16 -16 -16) (16 16 16)
landing arena key - blue
*/
	{
		"key_landing_area", /* FS: Zaero specific game dll changes */
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"k_bluekey",
		"Airfield Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_ZAERO,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_lab (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the laboratory
*/
	{
		"key_lab", /* FS: Zaero specific game dll changes */
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Laboratory Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_ZAERO,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_clearancepass (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
	{
		"key_clearancepass", /* FS: Zaero specific game dll changes */
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Clearance Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_ZAERO,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_energy (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
	{
		"key_energy", /* FS: Zaero specific game dll changes */
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/energy/tris.md2", EF_ROTATE,
		NULL,
		"k_energy",
		"Energy Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_ZAERO,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_lava (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_lava", /* FS: Zaero specific game dll changes */
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/lava/tris.md2", EF_ROTATE,
		NULL,
		"k_lava",
		"Lava Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_ZAERO,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_slime (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_slime", /* FS: Zaero specific game dll changes */
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/slime/tris.md2", EF_ROTATE,
		NULL,
		"k_slime",
		"Slime Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_ZAERO,
		0,
		NULL,
		0,
/* precache */ ""
	},

	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"i_health",
/* pickup */	"Health",
/* width */		3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{ /* FS: Coop: Spawn a backpack with our stuff */
		NULL,
		Pickup_CoopBackpack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_pack",
/* pickup */	"Coop Backpack",
/* width */		2,
		180,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

	/* end of list marker */
	{NULL}
};

/*
 * QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
 */
void
SP_item_health(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->intValue && (dmflags->intValue & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/n_health.wav");
}

/*
 * QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
 */
void
SP_item_health_small(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->intValue && (dmflags->intValue & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem(self, FindItem("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large (edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->intValue && (dmflags->intValue & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/l_health.wav");
}

/*
 * QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
 */
void
SP_item_health_mega(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->intValue && (dmflags->intValue & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX | HEALTH_TIMED;
}

void SP_item_foodcube (edict_t *self) /* FS: Coop: Xatrix specific */
{
	if (!self)
	{
		return;
	}

	if (deathmatch->intValue && (dmflags->intValue & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/objects/trapfx/tris.md2";
	SpawnItem(self, FindItem("Health"));
	self->spawnflags |= DROPPED_ITEM;
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex("items/s_health.wav");
	self->classname = "foodcube";
}

void
InitItems(void)
{
	game.num_items = sizeof(itemlist) / sizeof(itemlist[0]) - 1;
}

/*
 * Called by worldspawn
 */
void
SetItemNames(void)
{
	int i;
	gitem_t *it;

	for (i = 0; i < game.num_items; i++)
	{
		it = &itemlist[i];
		if (i != 0) //QW avoid redundant update of configstring, index 0 is always empty
			gi.configstring(CS_ITEMS + i, it->pickup_name);
	}

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index = ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
	shells_index = ITEM_INDEX(FindItem("Shells"));
	bullets_index = ITEM_INDEX(FindItem("Bullets"));
	grenades_index = ITEM_INDEX(FindItem("Grenades"));
	rockets_index = ITEM_INDEX(FindItem("Rockets"));
	cells_index = ITEM_INDEX(FindItem("Cells"));
	slugs_index = ITEM_INDEX(FindItem("Slugs"));
	flares_index = ITEM_INDEX(FindItem("Flares"));
	tbombs_index = ITEM_INDEX(FindItem("IRED"));
	empnuke_index = ITEM_INDEX(FindItem("EMPNuke"));
	plasmashield_index = ITEM_INDEX(FindItem("Plasma Shield"));
	a2k_index = ITEM_INDEX(FindItem("A2K"));
}

void
SP_xatrix_item(edict_t *self) /* FS: Coop: Rogue specific */
{
	gitem_t *item;
	int i;
	char *spawnClass = "";

	if (!self || !self->classname)
	{
		return;
	}

	if(game.gametype == xatrix_coop) /* FS: FIXME: Coop: This should work for Xatrix, but need to test the maps that have these items to be sure. */
	{
		ED_CallSpawn(self);
		return;
	}

	if (self->classname && !strcmp(self->classname, "ammo_magslug"))
	{
		spawnClass = "ammo_flechettes";
	}
	else if (self->classname && !strcmp(self->classname, "ammo_trap"))
	{
		spawnClass = "weapon_proxlauncher";
	}
	else if (self->classname && !strcmp(self->classname, "item_quadfire"))
	{
		float chance;

		chance = random();

		if (chance < 0.2)
		{
			spawnClass = "item_sphere_hunter";
		}
		else if (chance < 0.6)
		{
			spawnClass = "item_sphere_vengeance";
		}
		else
		{
			spawnClass = "item_sphere_defender";
		}
	}
	else if (self->classname && !strcmp(self->classname, "weapon_boomer"))
	{
		spawnClass = "weapon_etf_rifle";
	}
	else if (self->classname && !strcmp(self->classname, "weapon_phalanx"))
	{
		spawnClass = "weapon_plasmabeam";
	}

	/* check item spawn functions */
	for (i = 0, item = itemlist; i < game.num_items; i++, item++)
	{
		if (!item->classname)
		{
			continue;
		}

		if (!strcmp(item->classname, spawnClass))
		{
			/* found it */
			SpawnItem(self, item);
			return;
		}
	}
}

void Spawn_CoopBackpack(edict_t *ent)
{
	edict_t *backpack;
	gitem_t *it;
	int i;

	if (!ent || !ent->client)
	{
		gi.dprintf(DEVELOPER_MSG_VERBOSE, "Spawn_CoopBackpack: ent or ent->client is NULL!\n");
		return;
	}

	it = FindItem("Coop Backpack");
	if(!it)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "Spawn_CoopBackpack: Can't find Coop Backpack.\n");
		return;
	}

	backpack = G_Spawn();
	backpack->classname = it->classname = "item_coop_backpack"; /* FS: classname is purposely NULL for my paranoia of nobody adding it directly to an ent file override, etc. */
	SpawnItem(backpack, it);

	VectorCopy(ent->s.origin, backpack->s.origin); /* FS: Copy off the player's origin */

	for(i = 1; i < MAX_ITEMS; i++)
	{
		backpack->coopBackpackInventory[i] = ent->client->pers.inventory[i];
	}

	backpack->coopBackpackMaxHealth = ent->max_health;
	backpack->coopBackpackNetname = gi.TagMalloc(strlen(ent->client->pers.netname) + 1, TAG_GAME);
	strcpy(backpack->coopBackpackNetname, ent->client->pers.netname);

	backpack->coopBackpackAmmoUpgrade = ent->client->pers.ammoUpgrade;

	gi.dprintf(DEVELOPER_MSG_GAME, "Spawn_CoopBackpack: Spawn Coop Back for %s.\n", ent->client->pers.netname);
}

qboolean
Pickup_CoopBackpack(edict_t *ent, edict_t *other) /* FS: Coop: Spawn a backpack with the goodies */
{
	int i;

	if (!ent || !ent->coopBackpackNetname || !other || !other->client)
	{
		return false;
	}

	gi.cprintf(other, PRINT_HIGH, "You picked up %s's backpack.\n", ent->coopBackpackNetname);

	if(ent->coopBackpackMaxHealth > other->max_health) /* FS: Coop: Give back adrenaline boosts */
	{
		other->max_health = ent->coopBackpackMaxHealth;
		other->client->resp.coop_respawn.max_health = ent->coopBackpackMaxHealth;
		if (other->health < other->max_health)
		{
			other->health = other->max_health;
		}
	}

	if(ent->coopBackpackAmmoUpgrade > other->client->pers.ammoUpgrade)
	{
		switch(ent->coopBackpackAmmoUpgrade)
		{
			case COOP_BANDOLIER:
				Pickup_Bandolier(NULL, other);
				break;
			case COOP_BACKPACK:
				Pickup_Pack(NULL, other);
				break;
			default:
				break;
		}
	}

	for(i = 1; i < MAX_ITEMS; i++)
	{
		if(ent->coopBackpackInventory[i] > other->client->pers.inventory[i])
		{
			other->client->pers.inventory[i] = ent->coopBackpackInventory[i];
		}
	}

	gi.TagFree(ent->coopBackpackNetname);
	ent->coopBackpackNetname = NULL;

	return true;
}
