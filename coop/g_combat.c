// g_combat.c

#include "g_local.h"

void M_SetEffects(edict_t *self); /* FS: Coop: Rogue specific */

/*
 * clean up heal targets for medic
 */
void
cleanupHealTarget(edict_t *ent) /* FS: Coop: Rogue specific */
{
	if (!ent)
	{
		return;
	}

	ent->monsterinfo.healer = NULL;
	ent->takedamage = DAMAGE_YES;
	ent->monsterinfo.aiflags &= ~AI_RESURRECTING;
	M_SetEffects(ent);
}

/*
 * Returns true if the inflictor can directly damage the
 * target. Used for explosions and melee attacks.
 */
qboolean
CanDamage(edict_t *targ, edict_t *inflictor)
{
	vec3_t dest;
	trace_t trace;

	if (!targ || !inflictor)
	{
		return false;
	}

	/* bmodels need special checking because their origin is 0,0,0 */
	if (targ->movetype == MOVETYPE_PUSH)
	{
		VectorAdd(targ->absmin, targ->absmax, dest);
		VectorScale(dest, 0.5, dest);
		trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin,
				dest, inflictor, MASK_SOLID);

		if (trace.fraction == 1.0)
		{
			return true;
		}

		if (trace.ent == targ)
		{
			return true;
		}

		return false;
	}

	trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin,
			targ->s.origin, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
	{
		return true;
	}

	VectorCopy(targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin,
			dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
	{
		return true;
	}

	VectorCopy(targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin,
			dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
	{
		return true;
	}

	VectorCopy(targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin,
			dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
	{
		return true;
	}

	VectorCopy(targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin,
			dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
	{
		return true;
	}

	return false;
}

char *GetProperMonsterName (char *monsterName) /* FS: Coop: Get proper name for classname */
{
	if(!monsterName)
		return "Unknown Monster";
	else if(!strcmp("monster_soldier", monsterName))
		return "Shotgun Guard";
	else if(!strcmp("monster_soldier_light", monsterName))
		return "Light Guard";
	else if(!strcmp("monster_soldier_ss", monsterName))
		return "Machinegun Guard";
	else if(!strcmp("monster_flipper", monsterName))
		return "Barracuda Shark";
	else if(!strcmp("monster_flyer", monsterName))
		return "Flyer";
	else if(!strcmp("monster_infantry", monsterName))
		return "Enforcer";
	else if(!strcmp("monster_parasite", monsterName))
		return "Parasite";
	else if(!strcmp("monster_gunner", monsterName))
		return "Gunner";
	else if(!strcmp("monster_chick", monsterName))
		return "Iron Maiden";
	else if(!strcmp("monster_floater", monsterName))
		return "Technician";
	else if(!strcmp("monster_berserk", monsterName))
		return "Berserker";
	else if(!strcmp("monster_hover", monsterName))
		return "Icarus";
	else if(!strcmp("monster_medic", monsterName))
		return "Medic";
	else if(!strcmp("monster_mutant", monsterName))
		return "Mutant";
	else if(!strcmp("monster_brain", monsterName))
		return "Brain";
	else if(!strcmp("monster_gladiator", monsterName))
		return "Gladiator";
	else if(!strcmp("monster_tank", monsterName))
		return "Tank";
	else if(!strcmp("monster_tank_commander", monsterName))
		return "Tank Commander";
	else if(!strcmp("monster_supertank", monsterName))
		return "Super Tank";
	else if(!strcmp("monster_boss2", monsterName))
		return "Hornet";
	else if(!strcmp("monster_jorg", monsterName))
		return "Jorg";
	/* Knightmare- use correct monster classname */
	else if(!strcmp("monster_makron", monsterName))
		return "Makron";
	else if(!strcmp("misc_insane", monsterName))
		return "Prisoner";
	/* FS: Xatrix specific stuff */
	else if(!strcmp("monster_gekk", monsterName))
		return "Gekk";
	else if(!strcmp("monster_soldier_hypergun", monsterName))
		return "Hyper Blaster Guard";
	else if(!strcmp("monster_soldier_lasergun", monsterName))
		return "Laser Guard";
	else if(!strcmp("monster_soldier_ripper", monsterName))
		return "Ripper Guard";
	else if(!strcmp("monster_fixbot", monsterName))
		return "Repair Bot";
	else if(!strcmp("monster_chick_heat", monsterName))
		return "Iron Maiden (Beta Class)";
	else if(!strcmp("monster_gladb", monsterName))
		return "Gladiator (Beta Class)";
	else if(!strcmp("monster_boss5", monsterName))
		return "Tank Boss";
	/* FS: Rogue specific stuff */
	else if(!strcmp("monster_stalker", monsterName))
		return "Stalker";
	else if(!strcmp("monster_turret", monsterName))
		return "Turret";
	else if(!strcmp("monster_daedalus", monsterName))
		return "Daedalus";
	else if(!strcmp("monster_carrier", monsterName))
		return "Carrier";
	else if(!strcmp("monster_medic_commander", monsterName))
		return "Medic Commander";
	else if(!strcmp("monster_widow", monsterName))
		return "Black Widow Guardian";
	else if(!strcmp("monster_widow2", monsterName))
		return "Black Widow Guardian";
	else if(!strcmp("monster_kamikaze", monsterName))
		return "Kamikaze Flyer";
	/* FS: Zaero specific stuff */
	else if(!strcmp("monster_handler", monsterName))
		return "Enforcer Hound Handler";
	else if(!strcmp("monster_hound", monsterName))
		return "Hound Dog";
	else if(!strcmp("monster_sentien", monsterName))
		return "Sentien";
	else
		return monsterName;
}

char *GetCoopInsult (void) /* FS: Coop: Pick a random insult */
{
	int x;

	srand ((unsigned)time (NULL));
	x = rand() % 10;
	switch(x)
	{
		case 0:
			return "was killed by";
		case 1:
			return "was creamed by";
		case 2:
			return "was annihilated by";
		case 3:
			return "was torn to bits by";
		case 4:
			return "sucked it down to";
		case 5:
			return "was ripped apart by";
		case 6:
			return "bowed down to";
		case 7:
			return "was taken to the pearly gates by";
		case 8:
			return "was given a dirtnap by";
		case 9:
			return "saw the light thanks to";
	}
	return "was killed by";
}

void GetCoopMeansOfDeath (char *monsterName, char *playerName)
{
	int mod;
	char *message = NULL;
	char *message2 = "";

	if (!monsterName || !playerName)
	{
		return;
	}

	if (random() <= 0.20f) /* FS: One in five chance of getting a random insult to help spice it up. */
	{
		gi.bprintf(PRINT_MEDIUM, "%s %s %s\n", GetProperMonsterName(monsterName), GetCoopInsult(), playerName);
		return;
	}

	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;

	switch (mod)
	{
		case MOD_BLASTER:
			message = "was blasted by";
			break;
		case MOD_SHOTGUN:
			message = "was gunned down by";
			break;
		case MOD_SSHOTGUN:
			message = "was blown away by";
			message2 = "'s super shotgun";
			break;
		case MOD_MACHINEGUN:
			message = "was machinegunned by";
			break;
		case MOD_CHAINGUN:
			message = "was cut in half by";
			message2 = "'s chaingun";
			break;
		case MOD_GRENADE:
			message = "was popped by";
			message2 = "'s grenade";
			break;
		case MOD_G_SPLASH:
			message = "was shredded by";
			message2 = "'s shrapnel";
			break;
		case MOD_ROCKET:
			message = "ate";
			message2 = "'s rocket";
			break;
		case MOD_R_SPLASH:
			message = "almost dodged";
			message2 = "'s rocket";
			break;
		case MOD_HYPERBLASTER:
			message = "was melted by";
			message2 = "'s hyperblaster";
			break;
		case MOD_RAILGUN:
			message = "was railed by";
			break;
		case MOD_BFG_LASER:
			message = "saw the pretty lights from";
			message2 = "'s BFG";
			break;
		case MOD_BFG_BLAST:
			message = "was disintegrated by";
			message2 = "'s BFG blast";
			break;
		case MOD_BFG_EFFECT:
			message = "couldn't hide from";
			message2 = "'s BFG";
			break;
		case MOD_HANDGRENADE:
			message = "caught";
			message2 = "'s handgrenade";
			break;
		case MOD_HG_SPLASH:
			message = "didn't see";
			message2 = "'s handgrenade";
			break;
		case MOD_HELD_GRENADE:
			message = "feels";
			message2 = "'s pain";
			break;
		case MOD_TELEFRAG:
			message = "tried to invade";
			message2 = "'s personal space";
			break;
		case MOD_CHAINFIST: /* FS: Coop: Rogue specific */
			message = "was shredded by";
			message2 = "'s ripsaw";
			break;
		case MOD_DISINTEGRATOR: /* FS: Coop: Rogue specific */
			message = "lost his grip courtesy of";
			message2 = "'s disintegrator";
			break;
		case MOD_ETF_RIFLE: /* FS: Coop: Rogue specific */
			message = "was perforated by";
			break;
		case MOD_HEATBEAM: /* FS: Coop: Rogue specific */
			message = "was scorched by";
			message2 = "'s plasma beam";
			break;
		case MOD_TESLA: /* FS: Coop: Rogue specific */
			message = "was enlightened by";
			message2 = "'s tesla mine";
			break;
		case MOD_PROX: /* FS: Coop: Rogue specific */
			message = "got too close to";
			message2 = "'s proximity mine";
			break;
		case MOD_NUKE: /* FS: Coop: Rogue specific */
			message = "was nuked by";
			message2 = "'s antimatter bomb";
			break;
		case MOD_VENGEANCE_SPHERE: /* FS: Coop: Rogue specific */
			message = "was purged by";
			message2 = "'s vengeance sphere";
			break;
		case MOD_DEFENDER_SPHERE: /* FS: Coop: Rogue specific */
			message = "had a blast with";
			message2 = "'s defender sphere";
			break;
		case MOD_HUNTER_SPHERE: /* FS: Coop: Rogue specific */
			message = "was killed like a dog by";
			message2 = "'s hunter sphere";
			break;
		case MOD_TRACKER: /* FS: Coop: Rogue specific */
			message = "was annihilated by";
			message2 = "'s disruptor";
			break;
		case MOD_DOPPLE_EXPLODE: /* FS: Coop: Rogue specific */
			message = "was blown up by";
			message2 = "'s doppleganger";
			break;
		case MOD_DOPPLE_VENGEANCE: /* FS: Coop: Rogue specific */
			message = "was purged by";
			message2 = "'s doppleganger";
			break;
		case MOD_DOPPLE_HUNTER: /* FS: Coop: Rogue specific */
			message = "was hunted down by";
			message2 = "'s doppleganger";
			break;
		case MOD_RIPPER: /* FS: Coop: Xatrix specific */
			message = "ripped to shreds by";
			message2 = "'s ripper gun";
			break;
		case MOD_PHALANX: /* FS: Coop: Xatrix specific */
			message = "was evaporated by";
			break;
		case MOD_TRAP: /* FS: Coop: Xatrix specific */
			message = "caught in trap by";
			break;
		default:
			break;
	}

	if(message && message[0])
	{
		gi.bprintf(PRINT_MEDIUM, "%s %s %s%s\n", GetProperMonsterName(monsterName), message, playerName, message2);
	}
	else /* FS: Couldn't get means of death for some reason.  So just pick a random insult. */
	{
		gi.bprintf(PRINT_MEDIUM, "%s %s %s\n", GetProperMonsterName(monsterName), GetCoopInsult(), playerName);
	}
}

void
Killed(edict_t *targ, edict_t *inflictor, edict_t *attacker,
		int damage, vec3_t point)
{
	if (!targ || !targ->die || !inflictor || !attacker) /* FS: Need to check targ->die.  Got a crash from a player_noise classname. */
	{
		return;
	}

	if (targ->health < -999)
	{
		targ->health = -999;
	}

	if ((game.gametype == rogue_coop) && (targ->monsterinfo.aiflags & AI_MEDIC)) /* FS: Coop: Rogue specific */
	{
		if (targ->enemy)
		{
			cleanupHealTarget(targ->enemy);
		}

		/* clean up self */
		targ->monsterinfo.aiflags &= ~AI_MEDIC;
		targ->enemy = attacker;
	}
	else
	{
		targ->enemy = attacker;
	}

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
		if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
		{
			/* free up slot for spawned monster if it's spawned */
			if (targ->monsterinfo.aiflags & AI_SPAWNED_CARRIER)
			{
				if (targ->monsterinfo.commander &&
					targ->monsterinfo.commander->inuse &&
					targ->monsterinfo.commander->classname &&
					!strcmp(targ->monsterinfo.commander->classname, "monster_carrier"))
				{
					targ->monsterinfo.commander->monsterinfo.monster_slots++;
				}
			}

			if (targ->monsterinfo.aiflags & AI_SPAWNED_MEDIC_C)
			{
				if (targ->monsterinfo.commander)
				{
					if (targ->monsterinfo.commander->inuse &&
						targ->monsterinfo.commander->classname &&
						!strcmp(targ->monsterinfo.commander->classname, "monster_medic_commander"))
					{
						targ->monsterinfo.commander->monsterinfo.monster_slots++;
					}
				}
			}

			if (targ->monsterinfo.aiflags & AI_SPAWNED_WIDOW)
			{
				/* need to check this because we can
				   have variable numbers of coop players */
				if (targ->monsterinfo.commander &&
					targ->monsterinfo.commander->inuse &&
					targ->monsterinfo.commander->classname &&
					!strncmp(targ->monsterinfo.commander->classname, "monster_widow", 13))
				{
					if (targ->monsterinfo.commander->monsterinfo.monster_used > 0)
					{
						targ->monsterinfo.commander->monsterinfo.monster_used--;
					}
				}
			}
		}

		if ((!(targ->monsterinfo.aiflags & AI_GOOD_GUY)) &&
			(!(targ->monsterinfo.aiflags & AI_DO_NOT_COUNT))) /* FS: Coop: Rogue specific -- Added AI_DO_NOT_COUNT */
		{
			level.killed_monsters++;

			if (coop->intValue && attacker->client)
			{
				attacker->client->resp.score++;
			}

			if (game.gametype != rogue_coop) /* FS: Coop: Rogue handles this elsewhere */
			{
				/* medics won't heal monsters that they kill themselves */
				if (attacker && attacker->classname && strcmp(attacker->classname, "monster_medic") == 0)
				{
					targ->owner = attacker;
				}
			}
		}
	}

	if ((targ->movetype == MOVETYPE_PUSH) ||
		(targ->movetype == MOVETYPE_STOP) ||
		(targ->movetype == MOVETYPE_NONE))
	{
		/* doors, triggers, etc */
		if(game.gametype == rogue_coop)
		{
			if((targ->svflags & SVF_MONSTER) &&
				attacker &&
				attacker->client &&
				attacker->client->pers.netname[0] &&
				targ->classname &&
				!Q_stricmp(targ->classname, "monster_turret")) /* FS: Coop: monster_turrets have MOVETYPE_NONE so they stop here. */
			{
				GetCoopMeansOfDeath(targ->classname, attacker->client->pers.netname);
			}
		}
		targ->die(targ, inflictor, attacker, damage, point);
		return;
	}

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
		targ->touch = NULL;
		if(attacker && attacker->client && attacker->client->pers.netname[0] && targ && targ->classname) /* FS: Coop: Announce who we killed */
		{
			GetCoopMeansOfDeath(targ->classname, attacker->client->pers.netname);
		}
		monster_death_use(targ);
	}

	targ->die(targ, inflictor, attacker, damage, point);
}

void
SpawnDamage(int type, vec3_t origin, vec3_t normal)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WritePosition(origin);
	gi.WriteDir(normal);
	gi.multicast(origin, MULTICAST_PVS);
}

/*
 * targ			entity that is being damaged
 * inflictor	entity that is causing the damage
 * attacker		entity that caused the inflictor to damage targ
 *      example: targ=monster, inflictor=rocket, attacker=player
 *
 * dir			direction of the attack
 * point		point at which the damage is being inflicted
 * normal		normal vector from that point
 * damage		amount of damage being inflicted
 * knockback	force to be applied against targ as a result of the damage
 *
 * dflags -> these flags are used to control how T_Damage works
 *      DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
 *      DAMAGE_NO_ARMOR			armor does not protect from this damage
 *      DAMAGE_ENERGY			damage is from an energy based weapon
 *      DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
 *      DAMAGE_BULLET			damage is from a bullet (used for ricochets)
 *      DAMAGE_NO_PROTECTION	kills godmode, armor, everything
 */

int
CheckPowerArmor(edict_t *ent, vec3_t point, vec3_t normal, int damage,
		int dflags)
{
	gclient_t *client;
	int save;
	int power_armor_type;
	int index;
	int damagePerCell;
	int pa_te_type;
	int power = 0;
	int power_used;

	if (!ent)
	{
		return 0;
	}

	if (!damage)
	{
		return 0;
	}

	index = 0;

	client = ent->client;

	if (dflags & (DAMAGE_NO_ARMOR | DAMAGE_NO_POWER_ARMOR)) /* FS: Coop: Rogue specific -- Added DAMAGE_NO_POWER_ARMOR */
	{
		return 0;
	}

	if((game.gametype == zaero_coop) && (EMPNukeCheck(ent, point))) /* FS: Zaero specific game dll changes */
	{
		return 0;
	}
  
	if (client)
	{
		power_armor_type = PowerArmorType(ent);

		if (power_armor_type != POWER_ARMOR_NONE)
		{
			index = ITEM_INDEX(FindItem("Cells"));
			power = client->pers.inventory[index];
		}
	}
	else if (ent->svflags & SVF_MONSTER)
	{
		power_armor_type = ent->monsterinfo.power_armor_type;
		power = ent->monsterinfo.power_armor_power;
		index = 0;
	}
	else if((game.gametype == zaero_coop) && (ent->classname) && (strcmp(ent->classname, "PlasmaShield") == 0)) /* FS: Zaero specific game dll changes */
	{
		power_armor_type = POWER_ARMOR_SHIELD;
		power = ent->health;
	}
	else
	{
		return 0;
	}

	if (power_armor_type == POWER_ARMOR_NONE)
	{
		return 0;
	}

	if (!power)
	{
		return 0;
	}

	if (power_armor_type == POWER_ARMOR_SCREEN)
	{
		vec3_t vec;
		float dot;
		vec3_t forward;

		/* only works if damage point is in front */
		AngleVectors(ent->s.angles, forward, NULL, NULL);
		VectorSubtract(point, ent->s.origin, vec);
		VectorNormalize(vec);
		dot = DotProduct(vec, forward);

		if (dot <= 0.3)
		{
			return 0;
		}

		damagePerCell = 1;
		pa_te_type = TE_SCREEN_SPARKS;

		if(game.gametype == zaero_coop) /* FS: Zaero specific game dll changes */
		{
			if(!(dflags & DAMAGE_ARMORMOSTLY))
			{
				damage = damage / 3;
			}
		}
		else
		{
			damage = damage / 3;
		}
	}
	else
	{
		damagePerCell = 2;
		pa_te_type = TE_SHIELD_SPARKS;

		if(game.gametype == zaero_coop) /* FS: Zaero specific game dll changes */
		{
			if(!(dflags & DAMAGE_ARMORMOSTLY))
			{
	 	 		damage = (2 * damage) / 3;
			}
		}
		else
		{
			damage = (2 * damage) / 3;
		}
	}

	/* etf rifle */
	if ((game.gametype == rogue_coop) && (dflags & DAMAGE_NO_REG_ARMOR)) /* FS: Coop: Rogue specific */
	{
		save = (power * damagePerCell) / 2;
	}
	else
	{
		save = power * damagePerCell;
	}

	if (!save)
	{
		return 0;
	}

    if((game.gametype == zaero_coop) && (!(dflags & DAMAGE_ARMORMOSTLY))) /* FS: Zaero specific game dll changes */
	{
		save *= 2;
	}

	if (save > damage)
	{
		save = damage;
	}

	SpawnDamage(pa_te_type, point, normal);
	ent->powerarmor_time = level.time + 0.2;

	if ((game.gametype == rogue_coop) && (dflags & DAMAGE_NO_REG_ARMOR)) /* FS: Coop: Rogue specific */
	{
		power_used = (save / damagePerCell) * 2;
	}
	else
	{
		power_used = save / damagePerCell;
	}

	if (client)
	{
		client->pers.inventory[index] -= power_used;
	}
	else if(game.gametype == zaero_coop) /* FS: Zaero specific game dll changes */
	{
		if(ent->svflags & SVF_MONSTER)
		{
			ent->monsterinfo.power_armor_power -= power_used;
		}
	}
	else
	{
		ent->monsterinfo.power_armor_power -= power_used;
	}

	return save;
}

int
CheckArmor(edict_t *ent, vec3_t point, vec3_t normal, int damage,
		int te_sparks, int dflags)
{
	gclient_t *client;
	int save;
	int index;
	gitem_t *armor;

	if (!ent)
	{
		return 0;
	}

	if (!damage)
	{
		return 0;
	}

	client = ent->client;

	if (!client)
	{
		return 0;
	}

	if (((game.gametype == zaero_coop) && (dflags & DAMAGE_NO_ARMOR)) || ((game.gametype != zaero_coop) && (dflags & (DAMAGE_NO_ARMOR | DAMAGE_NO_REG_ARMOR)))) /* FS: Coop: Rogue specific -- Added DAMAGE_NO_REG_ARMOR */
	{
		return 0;
	}

	index = ArmorIndex(ent);

	if (!index)
	{
		return 0;
	}

	armor = GetItemByIndex(index);

	if (dflags & DAMAGE_ENERGY)
	{
		save = ceil(((gitem_armor_t *)armor->info)->energy_protection * damage);
	}
	else
	{
		save = ceil(((gitem_armor_t *)armor->info)->normal_protection * damage);
	}

	if (save >= client->pers.inventory[index])
	{
		save = client->pers.inventory[index];
	}

	if (!save)
	{
		return 0;
	}

	client->pers.inventory[index] -= save;

	if((game.gametype == zaero_coop) && (dflags & DAMAGE_ARMORMOSTLY)) /* FS: Zaero specific game dll changes */
	{
		save *= 2;
	}

	SpawnDamage (te_sparks, point, normal);

	return save;
}

void
M_ReactToDamage(edict_t *targ, edict_t *attacker, edict_t *inflictor)
{
	qboolean new_tesla; /* FS: Coop: Rogue specific */

    if (!targ || !attacker)
	{
		return;
	}

	if (targ->health <= 0)
	{
		return;
	}

	if (!(attacker->client) && !(attacker->svflags & SVF_MONSTER))
	{
		return;
	}

	if ((game.gametype == zaero_coop) && ( !(attacker->client) && !(attacker->svflags & SVF_MONSTER) && (attacker->classname) && (strcmp(attacker->classname, "monster_autocannon") != 0))) /* FS: Zaero specific game dll changes */
	{
		return;
	}

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		/* logic for tesla - if you are hit by a tesla,
		   and can't see who you should be mad at (attacker)
		   attack the tesla also, target the tesla if it's
		   a "new" tesla */
		if ((inflictor) && (inflictor->classname) && (!strcmp(inflictor->classname, "tesla")))
		{
			new_tesla = MarkTeslaArea(targ, inflictor);

			if (new_tesla)
			{
				TargetTesla(targ, inflictor);
			}

			return;
		}
	}

	if ((attacker == targ) || (attacker == targ->enemy))
	{
		return;
	}

	/* if we are a good guy monster and
	   our attacker is a player or another
	   good guy, do not get mad at them */
	if (targ->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (attacker->client || (attacker->monsterinfo.aiflags & AI_GOOD_GUY))
		{
			return;
		}
	}

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		/* if we're currently mad at something
		   a target_anger made us mad at, ignore
		   damage */
		if (targ->enemy && targ->monsterinfo.aiflags & AI_TARGET_ANGER)
		{
			float percentHealth;

			/* make sure whatever we were pissed at is still around. */
			if (targ->enemy->inuse)
			{
				percentHealth = (float)(targ->health) / (float)(targ->max_health);

				if (targ->enemy->inuse && (percentHealth > 0.33))
				{
					return;
				}
			}

			/* remove the target anger flag */
			targ->monsterinfo.aiflags &= ~AI_TARGET_ANGER;
		}

		/* if we're healing someone, do like above and try to stay with them */
		if ((targ->enemy) && (targ->monsterinfo.aiflags & AI_MEDIC))
		{
			float percentHealth;

			percentHealth = (float)(targ->health) / (float)(targ->max_health);

			/* ignore it some of the time */
			if (targ->enemy->inuse && (percentHealth > 0.25))
			{
				return;
			}

			/* remove the medic flag */
			targ->monsterinfo.aiflags &= ~AI_MEDIC;
			cleanupHealTarget(targ->enemy);
		}
	}

	/* if attacker is a client, get mad at
	   them because he's good and we're not */
	if (attacker->client)
	{
		targ->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

		/* this can only happen in coop (both new and old
		   enemies are clients)  only switch if can't see
		   the current enemy */
		if (targ->enemy && targ->enemy->client)
		{
			if (visible(targ, targ->enemy))
			{
				targ->oldenemy = attacker;
				return;
			}

			targ->oldenemy = targ->enemy;
		}

		targ->enemy = attacker;

		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		{
			FoundTarget(targ);
		}

		return;
	}

	/* it's the same base (walk/swim/fly) type and a
	   different classname and it's not a tank
	   (they spray too much), get mad at them */
	if (((targ->flags & (FL_FLY | FL_SWIM)) ==
		(attacker->flags & (FL_FLY | FL_SWIM))) &&
		(targ->classname) &&
		(attacker->classname) &&
		(strcmp(targ->classname, attacker->classname) != 0) &&
		(strcmp(attacker->classname, "monster_tank") != 0) &&
		(strcmp(attacker->classname, "monster_supertank") != 0) &&
		(strcmp(attacker->classname, "monster_makron") != 0) &&
		(strcmp(attacker->classname, "monster_jorg") != 0) && 
		!(attacker->monsterinfo.aiflags & AI_IGNORE_SHOTS) && /* FS: Coop: Rogue specific */
		!(targ->monsterinfo.aiflags & AI_IGNORE_SHOTS) && /* FS: Coop: Rogue specific */
		(!(attacker->mteam && targ->mteam && strcmp(attacker->mteam, targ->mteam) == 0))) /* FS: Zaero specific: Added monster team edict */
	{
		if (targ->enemy && targ->enemy->client)
		{
			targ->oldenemy = targ->enemy;
		}

		targ->enemy = attacker;

		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		{
			FoundTarget(targ);
		}
	}
	/* if they *meant* to shoot us, then shoot back */
	else if (attacker->enemy == targ)
	{
		if (targ->enemy && targ->enemy->client)
		{
			targ->oldenemy = targ->enemy;
		}

		targ->enemy = attacker;

		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		{
			FoundTarget(targ);
		}
	}
	/* otherwise get mad at whoever they are mad at (help our buddy) unless it is us! */
	else if (attacker->enemy && (attacker->enemy != targ))
	{
		if (targ->enemy && targ->enemy->client)
		{
			targ->oldenemy = targ->enemy;
		}

		targ->enemy = attacker->enemy;

		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		{
			FoundTarget(targ);
		}
	}
}

qboolean
CheckTeamDamage(edict_t *targ, edict_t *attacker) /* FS: Coop: Rogue specific */
{
	return false;
}

void
T_Damage(edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir,
		vec3_t point, vec3_t normal, int damage, int knockback, int dflags,
		int mod)
{
	gclient_t *client;
	int take;
	int save;
	int asave;
	int psave;
	int te_sparks;
	int sphere_notified; /* FS: Coop: Rogue specific */

	if (!targ || !inflictor || !attacker)
	{
		return;
	}

	if (!targ->takedamage)
	{
		return;
	}

	sphere_notified = false; /* FS: Coop: Rogue specific */

	/* friendly fire avoidance. If enabled you can't
	   hurt teammates (but you can hurt yourself)
	   knockback still occurs */
	if ((targ != attacker) && ((deathmatch->intValue &&
		 (dmflags->intValue & (DF_MODELTEAMS | DF_SKINTEAMS))) ||
		 coop->intValue))
	{
		if (OnSameTeam(targ, attacker))
		{
			/* nukes kill everyone */
			if (((dmflags->intValue & DF_NO_FRIENDLY_FIRE) || coop->intValue) && (mod != MOD_TELEFRAG) && /* FS: Coop: No friendly fire in Coop, except for telefrags */
				(mod != MOD_NUKE)) /* FS: Coop: Rogue specific */
			{
				damage = 0;
			}
			else
			{
				mod |= MOD_FRIENDLY_FIRE;
			}
		}
	}

	meansOfDeath = mod;

	/* allow the deathmatch game to change values */
	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		if (deathmatch->intValue && gamerules && gamerules->intValue)
		{
			if (DMGame.ChangeDamage)
			{
				damage = DMGame.ChangeDamage(targ, attacker, damage, mod);
			}

			if (DMGame.ChangeKnockback)
			{
				knockback = DMGame.ChangeKnockback(targ, attacker, knockback, mod);
			}

			if (!damage)
			{
				return;
			}
		}
	}

	/* easy mode takes half damage */
	if ((skill->intValue == 0) && (deathmatch->intValue == 0) && targ->client)
	{
		damage *= 0.5;

		if (!damage)
		{
			damage = 1;
		}
	}

	client = targ->client;

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		/* defender sphere takes half damage */
		if ((client) && (client->owned_sphere) &&
			(client->owned_sphere->spawnflags == 1))
		{
			damage *= 0.5;

			if (!damage)
			{
				damage = 1;
			}
		}
	}

	if (game.gametype == zaero_coop) /* FS: Zaero specific game dll changes */
	{
		if ((targ->svflags & SVF_MONSTER) && ((targ->monsterinfo.aiflags & AI_REDUCEDDAMAGE) ||
					((targ->monsterinfo.aiflags & AI_MONREDUCEDDAMAGE) && (inflictor->svflags & SVF_MONSTER))))
		{
			damage *= targ->monsterinfo.reducedDamageAmount;
			if (!damage)
			{
				damage = 1;
			}
		}
	}

	if (dflags & DAMAGE_BULLET)
	{
		te_sparks = TE_BULLET_SPARKS;
	}
	else
	{
		te_sparks = TE_SPARKS;
	}

	VectorNormalize(dir);

	/* bonus damage for suprising a monster */
	if (!(dflags & DAMAGE_RADIUS) && (targ->svflags & SVF_MONSTER) &&
		(attacker->client) && (!targ->enemy) && (targ->health > 0))
	{
		damage *= 2;
	}

	if (targ->flags & FL_NO_KNOCKBACK)
	{
		knockback = 0;
	}
	/* FS: Zaero specific: Knockback code moved foreward to take into account for DAMAGE_ARMORMOSTLY now */

	take = damage;
	save = 0;

	/* check for godmode */
	if ((targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		take = 0;
		save = damage;
		SpawnDamage(te_sparks, point, normal);
	}

	/* check for a2k invincibility */
	if (game.gametype == zaero_coop) /* FS: Zaero specific game dll changes */
	{
		if (client && client->a2kFramenum > level.framenum)
		{
			if (targ->pain_debounce_time < level.time)
			{
				gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
				targ->pain_debounce_time = level.time + 2;
			}
			take = 0;
			save = damage;
		}
	}

	/* check for invincibility */
	if ((client && (client->invincible_framenum > level.framenum)) &&
		!(dflags & DAMAGE_NO_PROTECTION) && (mod != MOD_TRAP)) /* FS: Coop: Xatrix specific: MOD_TRAP */
	{
		if (targ->pain_debounce_time < level.time)
		{
			gi.sound(targ, CHAN_ITEM, gi.soundindex( "items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2;
		}

		take = 0;
		save = damage;
	}

	/* check for monster invincibility */
	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		if (((targ->svflags & SVF_MONSTER) &&
			 (targ->monsterinfo.invincible_framenum > level.framenum)) &&
			!(dflags & DAMAGE_NO_PROTECTION))
		{
			if (targ->pain_debounce_time < level.time)
			{
				gi.sound(targ, CHAN_ITEM, gi.soundindex( "items/protect4.wav"), 1, ATTN_NORM, 0);
				targ->pain_debounce_time = level.time + 2;
			}

			take = 0;
			save = damage;
		}
	}

	psave = CheckPowerArmor(targ, point, normal, take, dflags);
	take -= psave;

	asave = CheckArmor(targ, point, normal, take, te_sparks, dflags);
	take -= asave;

	/* treat cheat/powerup savings the same as armor */
	asave += save;

	/* figure momentum add */
	if (!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if ((knockback) && (targ->movetype != MOVETYPE_NONE) &&
			(targ->movetype != MOVETYPE_BOUNCE) &&
			(targ->movetype != MOVETYPE_BOUNCEFLY) && /* FS: Zaero specific */
			(targ->movetype != MOVETYPE_PUSH) &&
			(targ->movetype != MOVETYPE_STOP))
		{
			vec3_t kvel;
			float mass;

			if (game.gametype == zaero_coop) /* FS: Zaero specific */
			{
				if((dflags & DAMAGE_ARMORMOSTLY) && damage > take)
				{
					knockback = (int)((float)knockback * (((float)(damage - take) / (float)damage) + 1.0));
				}
			}

			if (targ->mass < 50)
			{
				mass = 50;
			}
			else
			{
				mass = targ->mass;
			}

			if (targ->client && (attacker == targ))
			{
				/* This allows rocket jumps */
				VectorScale(dir, 1600.0 * (float)knockback / mass, kvel);
			}
			else
			{
				VectorScale(dir, 500.0 * (float)knockback / mass, kvel);
			}

			VectorAdd(targ->velocity, kvel, targ->velocity);
		}
	}

	/* team damage avoidance */
	if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage(targ, attacker))
	{
		return;
	}

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		/* this option will do damage both to the armor
		   and person. originally for DPU rounds */
		if (dflags & DAMAGE_DESTROY_ARMOR)
		{
			if (!(targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION) &&
				!(client && (client->invincible_framenum > level.framenum)))
			{
				take = damage;
			}
		}
	}

	/* do the damage */
	if (take)
	{
		/* need more blood for chainfist. */
		if ((game.gametype == rogue_coop) && (targ->flags & FL_MECHANICAL)) /* FS: Coop: Rogue specific */
		{
			SpawnDamage(TE_ELECTRIC_SPARKS, point, normal);
		}
		else if ((targ->svflags & SVF_MONSTER) || (client))
		{
			/* Knightmare- enumerated blood types */
			if (targ->blood_type == 1)
			{
				SpawnDamage (TE_GREENBLOOD, point, normal);
			}
			else if (targ->blood_type == 2)
			{
				SpawnDamage (TE_SPARKS, point, normal);
				SpawnDamage (TE_SPARKS, point, normal);
			}
			else if (targ->blood_type == 3)
			{
				SpawnDamage (TE_SPARKS, point, normal);
				SpawnDamage (TE_SPARKS, point, normal);
				SpawnDamage (TE_BLOOD, point, normal);
			}			
			else if (targ->blood_type == 4)
			{
				SpawnDamage (TE_ELECTRIC_SPARKS, point, normal);
			}
			/* end Knightmare */
			else if ((game.gametype == rogue_coop) && (mod == MOD_CHAINFIST)) /* FS: Coop: Rogue specific */
			{
				SpawnDamage(TE_MOREBLOOD, point, normal);
			}
			/*	Knightmare- this is now redundant due to using blood_type field */
			/* FS: Coop: Xatrix specific */
		/*	else if ((game.gametype == xatrix_coop) && (targ->classname) && (strcmp(targ->classname, "monster_gekk") == 0))
			{
				SpawnDamage(TE_GREENBLOOD, point, normal);
			} */
			else
			{
				SpawnDamage(TE_BLOOD, point, normal);
			}
		}
		else
		{
			SpawnDamage(te_sparks, point, normal);
		}

		if(game.gametype == zaero_coop)
		{
			if(targ->takedamage != DAMAGE_IMMORTAL)
			{
				targ->health = targ->health - take;
			}
		}
		else
		{
			targ->health = targ->health - take;
		}

		if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
		{
			/* spheres need to know who to shoot at */
			if (client && client->owned_sphere)
			{
				sphere_notified = true;

				if (client->owned_sphere->pain)
				{
					client->owned_sphere->pain(client->owned_sphere, attacker, 0, 0);
				}
			}
		}

		/* kill the entity */
		if (targ->health <= 0)
		{
			if ((targ->svflags & SVF_MONSTER) || (client))
			{
				targ->flags |= FL_NO_KNOCKBACK;
			}

			Killed(targ, inflictor, attacker, take, point);
			return;
		}
	}

	if (game.gametype == rogue_coop) /* FS: Coop: Rogue specific */
	{
		/* spheres need to know who to shoot at */
		if (!sphere_notified)
		{
			if (client && client->owned_sphere)
			{
				if (client->owned_sphere->pain)
				{
					client->owned_sphere->pain(client->owned_sphere, attacker, 0,
							0);
				}
			}
		}
	}

	if (targ->svflags & SVF_MONSTER)
	{
		M_ReactToDamage(targ, attacker, inflictor);

		if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take))
		{
			if (targ->pain)
			{
				targ->pain(targ, attacker, knockback, take);
			}

			/* nightmare mode monsters don't go into pain frames often */
			if (skill->intValue == 3)
			{
				if (targ->pain)
				{
					targ->pain_debounce_time = level.time + 5;
				}
			}
		}
	}
	else if (client)
	{
		if (!(targ->flags & FL_GODMODE) && (take))
		{
			if (targ->pain)
			{
				targ->pain(targ, attacker, knockback, take);
			}
		}
	}
	else if (take)
	{
		if (targ->pain)
		{
			targ->pain(targ, attacker, knockback, take);
		}
	}

	/* add to the damage inflicted on a player this frame
	   the total will be turned into screen blends and view
	   angle kicks at the end of the frame */
	if (client)
	{
		client->damage_parmor += psave;
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		VectorCopy(point, client->damage_from);
	}
}

void
T_RadiusDamage(edict_t *inflictor, edict_t *attacker, float damage,
		edict_t *ignore, float radius, int mod)
{
	float points;
	edict_t *ent = NULL;
	vec3_t v;
	vec3_t dir;

	if (!inflictor || !attacker)
	{
		return;
	}

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
		{
			continue;
		}

		if (!ent->takedamage)
		{
			continue;
		}

		VectorAdd(ent->mins, ent->maxs, v);
		VectorMA(ent->s.origin, 0.5, v, v);
		VectorSubtract(inflictor->s.origin, v, v);
		points = damage - 0.5 * VectorLength(v);

		if (ent == attacker)
		{
			points = points * 0.5;
		}

		if (points > 0)
		{
			if (CanDamage(ent, inflictor))
			{
				VectorSubtract(ent->s.origin, inflictor->s.origin, dir);
				T_Damage(ent, inflictor, attacker, dir, inflictor->s.origin,
						vec3_origin, (int)points, (int)points, DAMAGE_RADIUS,
						mod);
			}
		}
	}
}

void
T_RadiusNukeDamage(edict_t *inflictor, edict_t *attacker, float damage, /* FS: Coop: Rogue specific */
		edict_t *ignore, float radius, int mod)
{
	float points;
	edict_t *ent = NULL;
	vec3_t v;
	vec3_t dir;
	float len;
	float killzone, killzone2;
	trace_t tr;
	float dist;

	killzone = radius;
	killzone2 = radius * 2.0;

	if (!inflictor || !attacker || !ignore)
	{
		return;
	}

	while ((ent = findradius(ent, inflictor->s.origin, killzone2)) != NULL)
	{
		/* ignore nobody */
		if (ent == ignore)
		{
			continue;
		}

		if (!ent->takedamage)
		{
			continue;
		}

		if (!ent->inuse)
		{
			continue;
		}

		if (!(ent->client || (ent->svflags & SVF_MONSTER) ||
			  (ent->svflags & SVF_DAMAGEABLE)))
		{
			continue;
		}

		VectorAdd(ent->mins, ent->maxs, v);
		VectorMA(ent->s.origin, 0.5, v, v);
		VectorSubtract(inflictor->s.origin, v, v);
		len = VectorLength(v);

		if (len <= killzone)
		{
			if (ent->client)
			{
				ent->flags |= FL_NOGIB;
			}

			points = 10000;
		}
		else if (len <= killzone2)
		{
			points = (damage / killzone) * (killzone2 - len);
		}
		else
		{
			points = 0;
		}

		if (points > 0)
		{
			if (ent->client)
			{
				ent->client->nuke_framenum = level.framenum + 20;
			}

			VectorSubtract(ent->s.origin, inflictor->s.origin, dir);
			T_Damage(ent, inflictor, attacker, dir, inflictor->s.origin,
					vec3_origin, (int)points, (int)points, DAMAGE_RADIUS,
					mod);
		}
	}

	/* skip the worldspawn */
	ent = g_edicts + 1;

	/* cycle through players */
	while (ent)
	{
		if ((ent->client) &&
			(ent->client->nuke_framenum != level.framenum + 20) && (ent->inuse))
		{
			tr = gi.trace(inflictor->s.origin, NULL, NULL, ent->s.origin,
					inflictor, MASK_SOLID);

			if (tr.fraction == 1.0)
			{
				ent->client->nuke_framenum = level.framenum + 20;
			}
			else
			{
				dist = realrange(ent, inflictor);

				if (dist < 2048)
				{
					ent->client->nuke_framenum = max(ent->client->nuke_framenum,
							level.framenum + 15);
				}
				else
				{
					ent->client->nuke_framenum = max(ent->client->nuke_framenum,
							level.framenum + 10);
				}
			}

			ent++;
		}
		else
		{
			ent = NULL;
		}
	}
}

/*
 * Like T_RadiusDamage, but ignores
 * anything with classname=ignoreClass
 */
void
T_RadiusClassDamage(edict_t *inflictor, edict_t *attacker, float damage, /* FS: Coop: Rogue specific */
		char *ignoreClass, float radius, int mod)
{
	float points;
	edict_t *ent = NULL;
	vec3_t v;
	vec3_t dir;

	if (!inflictor || !attacker || !ignoreClass)
	{
		return;
	}

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent->classname && !strcmp(ent->classname, ignoreClass))
		{
			continue;
		}

		if (!ent->takedamage)
		{
			continue;
		}

		VectorAdd(ent->mins, ent->maxs, v);
		VectorMA(ent->s.origin, 0.5, v, v);
		VectorSubtract(inflictor->s.origin, v, v);
		points = damage - 0.5 * VectorLength(v);

		if (ent == attacker)
		{
			points = points * 0.5;
		}

		if (points > 0)
		{
			if (CanDamage(ent, inflictor))
			{
				VectorSubtract(ent->s.origin, inflictor->s.origin, dir);
				T_Damage(ent, inflictor, attacker, dir, inflictor->s.origin,
						vec3_origin, (int)points, (int)points, DAMAGE_RADIUS,
						mod);
			}
		}
	}
}


/*
============
T_RadiusDamagePosition
============
*/
void T_RadiusDamagePosition (vec3_t origin, edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod) /* FS: Zaero specific game dll changes */
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

	while ((ent = findradius(ent, origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (origin, v, v);
		points = damage - 0.5 * VectorLength (v);
		if (ent == attacker)
			points = points * 0.5;
		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				VectorSubtract (ent->s.origin, origin, dir);
				T_Damage (ent, inflictor, attacker, dir, origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
			}
		}
	}
}
