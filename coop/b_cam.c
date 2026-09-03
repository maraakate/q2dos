// b_coopcam.c
// blinky
// Created: 2000/06
// Edited:  2000/07/17
//

#include "g_local.h"

#define SCANNER_UNIT 10

static void Blinky_ResetView (edict_t *ent) /* FS */
{
	if (!ent || !ent->inuse || !ent->client)
		return;

	VectorClear(ent->s.angles);
	VectorClear(ent->client->ps.viewangles);
	VectorClear(ent->client->v_angle);
	VectorClear(ent->client->kick_angles);
	VectorClear(ent->client->kick_origin);
	VectorClear(ent->client->oldviewangles);
	VectorClear(ent->client->ps.gunangles);
	VectorClear(ent->client->ps.kick_angles);
	ent->client->v_dmg_pitch = ent->client->v_dmg_roll = ent->client->v_dmg_time = 0.0f;
	ent->client->ps.pmove.delta_angles[0] = ent->client->ps.pmove.delta_angles[1] = ent->client->ps.pmove.delta_angles[2] = 0;
}

static qboolean IsSpectator (edict_t *ent)
{
	if (!ent || !ent->inuse || !ent->client)
	{
		return false;
	}

	return ent->client->pers.spectator;
}

void Blinky_RunRun (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t *client = NULL;
	BlinkyClient_t *bdata = NULL;

	if (!ent || !ent->inuse || !ent->client || !ucmd)
	{
		return;
	}

	client = ent->client;
	bdata = &client->blinky_client;

	/* the runrun code - continuous running	 */
	if (bdata->runrun)
	{
		if (ucmd->forwardmove > 0)
			bdata->runrun = 1;
		else if (ucmd->forwardmove < 0)
			bdata->runrun = 2;
		else if (ucmd->sidemove > 0)
			bdata->runrun = 3;
		else if (ucmd->sidemove < 0)
			bdata->runrun = 4;
		switch (bdata->runrun)
		{
			case 1: ucmd->forwardmove = 400; break;
			case 2: ucmd->forwardmove = -400; break;
			case 3: ucmd->sidemove = 400; break;
			case 4: ucmd->sidemove = -400; break;
		}
	}
}

void Blinky_UpdateCameraThink (edict_t *ent)
{
	gclient_t *client = NULL;
	BlinkyClient_t *bdata = NULL;

	if (!ent || !ent->inuse || !ent->client)
	{
		return;
	}

	client = ent->client;
	bdata = &client->blinky_client;

	if (bdata->cam_target) /* FS: Follow the player and ignore input */
	{
		int i = 0;

		if (bdata->cam_target->deadflag)
		{
			ent->client->ps.pmove.pm_type = PM_DEAD;
		}
		else
		{
			ent->client->ps.pmove.pm_type = PM_FREEZE;
		}

		for (i = 0; i < 3; i++)
		{
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(
				bdata->cam_target->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
		}

		if (bdata->cam_target->deadflag)
		{
			ent->client->ps.viewangles[ROLL] = 40;
			ent->client->ps.viewangles[PITCH] = -15;
			ent->client->ps.viewangles[YAW] = bdata->cam_target->client->killer_yaw;
		}
		else
		{
			VectorCopy(bdata->cam_target->client->v_angle, ent->client->ps.viewangles);
			VectorCopy(bdata->cam_target->client->v_angle, ent->client->v_angle);
		}
	}

	if (bdata->cam_target && bdata->cam_target->inuse)
	{
		ent->client->ps.stats[STAT_CHASE] = CS_PLAYERSKINS +
			(bdata->cam_target - g_edicts) - 1;
	}
	else
	{
		ent->client->ps.stats[STAT_CHASE] = 0;
	}
}

static void Blinky_BeginServerFrameClient (edict_t *ent)
{
	edict_t *target = NULL;
	edict_t *decoy = NULL;

	if (!ent || !ent->inuse || !ent->client)
	{
		return;
	}

	target = ent->client->blinky_client.cam_target;
	decoy = ent->client->blinky_client.cam_decoy;

	if (target && decoy)
	{
		decoy->s = ent->s;
	}
}

void Blinky_BeginRunFrame (void)
{
	int i;
	edict_t *ent;

	for (i = 0; i < maxclients->intValue; i++)
	{
		ent = g_edicts + 1 + i;

		if (!ent->inuse || !ent->client)
		{
			continue;
		}

		Blinky_BeginServerFrameClient(ent);
	}
}

static void ShowStats (edict_t *ent, edict_t *player)
{
	vec3_t v;
	char stats[500];
	char pname[11];
	int health, armor, armorpow;
	int xd, yd, zd;
	vec3_t dp, normal = { 0,0,-1 };
	static int CellsIndex = -1;
	int index;

	if (!ent || !player || !player->client)
	{
		return;
	}

	armor = 0;

	if (CellsIndex == -1)
	{
		CellsIndex = ITEM_INDEX(FindItem("cells"));
	}

	VectorSubtract(ent->s.origin, player->s.origin, v);
	zd = -v[2] / SCANNER_UNIT; // save height differential
	v[2] = 0; // remove height component

	RotatePointAroundVector(dp, normal, v, ent->s.angles[1]);
	xd = -dp[0] / SCANNER_UNIT;
	yd = dp[1] / SCANNER_UNIT;

	//if (player->client)
	{
		strncpy(pname, player->client->pers.netname, sizeof(pname) - 1);
	}
	//else if (player->classname)
	//{
	//	strncpy(pname, player->classname, sizeof(pname)-1);
	//}
	//else
	//{
	//	Com_sprintf(pname, sizeof(pname), "unnamed");
	//}

	pname[sizeof(pname) - 1] = 0;
	health = player->health;

	armorpow = 0;
	if (PowerArmorType(player))
	{
		armorpow = player->client->pers.inventory[CellsIndex];
	}

	index = ArmorIndex (player);
	if (index)
	{
		armor = player->client->pers.inventory[index];
	}

	/* FS: See cl_scrn.c.  Values for armor can be negative and are ignored */
	if (armor < 1)
	{
		armor = 0;
	}
	if (armorpow < 1)
	{
		armorpow = 0;
	}

	if (armorpow)
	{
		Com_sprintf(stats, sizeof(stats), "%s: armor=%3d(%3d), health=%3d (f=%+5d,r=%+5d,u=%+5d)\n"
			, pname, armor, armorpow, health, xd, yd, zd);
	}
	else
	{
		Com_sprintf(stats, sizeof(stats), "%s: armor=%3d, health=%3d (f=%+5d,r=%+5d,u=%+5d)\n"
			, pname, armor, health, xd, yd, zd);
	}

	gi.cprintf(ent, PRINT_CHAT, "%s", stats);
}

void MoveToAngles (edict_t *ent, vec3_t pv1)
{
	int i;

	if (!ent || !ent->inuse || !ent->client)
	{
		return;
	}

	for (i = 0; i < 3; i++)
	{
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(pv1[i] - ent->client->resp.cmd_angles[i]);
	}
}

void stopBlinkyCam (edict_t *ent)
{
	BlinkyClient_t *bdata = NULL;

	if (!ent || !ent->inuse || !ent->client)
	{
		return;
	}

	bdata = &ent->client->blinky_client;
	if (!bdata->cam_decoy)
	{
		return;
	}

	bdata->cam_target = NULL;
	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	bdata->cam_decoy->svflags |= SVF_NOCLIENT;
	VectorCopy(bdata->cam_decoy->s.origin, ent->s.origin);
	MoveToAngles(ent, bdata->cam_decoy->s.angles);
	ent->client->ps.fov = bdata->save_fov;
	ent->client->pers.hand = bdata->save_hand;
	ent->client->ps.gunindex = bdata->save_weapon;
	ent->client->ps.stats[STAT_CHASE] = 0;

	gi.linkentity(ent);

	/* FS: Get rid of the decoy, free up some edicts */
	G_FreeEdict(bdata->cam_decoy);
	bdata->cam_decoy = NULL;
	//	gi.linkentity(bdata->cam_decoy);
}

void StartCam (edict_t *ent, edict_t *target)
{
	BlinkyClient_t *bdata = NULL;
	edict_t *decoy = NULL;

	if (!ent || !ent->inuse || !ent->client || !target || !target->inuse || !target->client)
	{
		return;
	}

	bdata = &ent->client->blinky_client;
	decoy = bdata->cam_decoy;

	bdata->cam_target = target;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	if (!decoy)
	{
		bdata->cam_decoy = G_Spawn();
		decoy = bdata->cam_decoy;
		decoy->client = ent->client;
		decoy->s = ent->s;
	}
	bdata->cam_decoy->svflags &= ~SVF_NOCLIENT;
	VectorCopy(ent->s.origin, bdata->cam_decoy->s.origin);
	MoveToAngles(ent, target->s.angles);
	VectorCopy(ent->s.angles, bdata->cam_decoy->s.angles);
	bdata->save_fov = ent->client->ps.fov;
	bdata->save_hand = ent->client->pers.hand;
	bdata->save_weapon = ent->client->ps.gunindex; /* FS: Added */
	ent->client->pers.hand = CENTER_HANDED;
	if (target->client)
		ent->client->ps.fov = target->client->ps.fov;
	ent->client->ps.gunindex = 0;
	gi.linkentity(ent);
	gi.linkentity(bdata->cam_decoy);
}

void Cmd_Stats_f (edict_t *ent)
{
	char *name = gi.args();
	edict_t *player;

	if (!ent)
	{
		return;
	}

	if (sv_coop_blinky_cam_disallowflags->intValue & BCAM_NOSTATS)
	{
		gi.cprintf(ent, PRINT_HIGH, "Summon command disabled on this server!\n");
		return;
	}

	for (player = &g_edicts[0] + 1; player < &g_edicts[0] + (int)(maxclients->intValue) + 1; player++)
	{
		if (!player->inuse || !player->client || player->client->pers.spectator)
			continue;
		// if a specific name requested & doesn't match, skip
		if (name[0])
		{
			if (Q_stricmp(name, player->client->pers.netname))
				continue;
		}
		ShowStats(ent, player);
	}
}

void Cmd_Cam_f (edict_t *ent)
{
	char *name = gi.args();
	qboolean bFindFailed = true;
	// obj1 is how to tell when we've looped
	edict_t *obj1;
	edict_t *target;
	BlinkyClient_t *bdata;

	if (!ent || !ent->inuse || !ent->client || ent->client->pers.spectator)
	{
		return;
	}

	if (sv_coop_blinky_cam_disallowflags->intValue & BCAM_NOCHASE)
	{
		gi.cprintf(ent, PRINT_HIGH, "Chase camera command disabled on this server!\n");
		return;
	}

	bdata = &ent->client->blinky_client;
	obj1 = bdata->cam_target;

	if (!coop->intValue || IsSpectator(ent))
	{
		return;
	}

	if (name[0] && !Q_stricmp(name, ent->client->pers.netname))
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't spectate yourself!\n");
		return;
	}

	if (name[0])
	{
		if (!strncmp(name, "CL ", 3))
		{
			int playernum;

			name += 3;
			playernum = atoi(name);
			if (playernum > game.maxclients)
			{
				gi.cprintf(ent, PRINT_HIGH, "Player #%d greater than maxclients.  Aborting search!\n", playernum);
				return;
			}

			target = &g_edicts[playernum + 1];
			if (!target || !target->inuse || !target->client)
			{
				gi.cprintf(ent, PRINT_HIGH, "Couldn't find player #%d to chase!\n", playernum);
				return;
			}

			if (ent->client->blinky_client.cam_target == target) /* FS: If we ask for it twice and we're already viewing them then bust out */
			{
				stopBlinkyCam(ent);
				return;
			}

			if (IsSpectator(target))
			{
				gi.cprintf(ent, PRINT_HIGH, "You can't chase a spectator!\n");
				return;
			}

			if (target == ent)
			{
				gi.cprintf(ent, PRINT_HIGH, "You can't chase yourself!\n");
				return;
			}

			StartCam(ent, target);
			ShowStats(ent, target);
			return;
		}

		if (!strncmp(name, "LIKE ", 5))
		{
			name += 5;
			target = Find_LikePlayer(ent, name, false);
		}
		else
		{
			target = Find_LikePlayer(ent, name, true);
		}

		if (!target || !target->inuse || !target->client)
		{
			gi.cprintf(ent, PRINT_HIGH, "Couldn't find player %s to chase!\n", name);
			return;
		}

		if (ent->client->blinky_client.cam_target == target) /* FS: If we ask for it twice and we're already viewing them then bust out */
		{
			stopBlinkyCam(ent);
			return;
		}

		if (IsSpectator(target))
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't chase a spectator!\n");
			return;
		}

		if (target == ent)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't chase yourself!\n");
			return;
		}

		StartCam(ent, target);
		ShowStats(ent, target);
		return;
	}
	else
	{
		if (!obj1)
		{
			gi.cprintf(ent, PRINT_HIGH, "Blinky cam setup\n");
			obj1 = ent;
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "Blinky cam stop\n");
			stopBlinkyCam(ent); /* FS: Stop the camera or else it gets all flabbergasted when cycling */
		}

		target = obj1;

		while (1)
		{
			// advance loop thru edicts
			// to do - this ought to cycle at maxclients instead of num_edicts
			if (target < &g_edicts[0] + (int)(maxclients->intValue) + 1)
				target++;
			else
				target = g_edicts + 1;
			if (target == obj1)
				break;
			// only look at (in use) players
			if (!target->inuse || !target->client || IsSpectator(target))
				continue;

			// found cam target
			if (target == ent)
			{
				bFindFailed = false;
				stopBlinkyCam(ent);
			}
			else
			{
				bFindFailed = false;
				StartCam(ent, target);
				ShowStats(ent, target);
			}
			break;
		}

		if (bFindFailed)
		{
			gi.cprintf(ent, PRINT_HIGH, "Can't find a player to chase!\n");
		}
	}
}

static void Summon (edict_t *ent, edict_t *other)
{
	vec3_t offset, forward, right, start;
	trace_t tr;

	if (!ent || !ent->inuse || !ent->client || !other || !other->inuse || !other->client)
	{
		return;
	}

	if (ent->client->summon_time > level.time)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must wait at least %d second(s) before you can summon another player.\n", (int)ent->client->summon_time - (int)level.time);
		return;
	}

	if (other->client->pers.noSummon)
	{
		gi.cprintf(ent, PRINT_HIGH, "Player \"%s\" has summon disabled!\n", other->client->pers.netname);
		return;
	}

	if (other->deadflag == DEAD_DEAD)
	{
		gi.cprintf(ent, PRINT_HIGH, "Player \"%s\" is dead, aborting summon!\n", other->client->pers.netname);
		return;
	}

	VectorSet(offset, 40, 0, ent->viewheight - 8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// code mostly copied from KillBox

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	tr = gi.trace (start, other->mins, other->maxs, start, NULL, MASK_PLAYERSOLID);
	if (tr.fraction < 1.0f)
	{
		gi.cprintf(ent, PRINT_HIGH, "Collision issue.  Can not summon \"%s\".  Try from a different location.\n", other->client->pers.netname);
		gi.linkentity (other);
		return;
	}

	VectorCopy (start, other->s.origin);
	VectorCopy (start, other->s.old_origin);
	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160 >> 3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	MoveToAngles(other, ent->s.angles);

	Blinky_ResetView(other); /* FS */

	// kill anything at the destination
	KillBox (other);

	gi.cprintf(ent, PRINT_HIGH, "Summoned \"%s\"!\n", other->client->pers.netname);
	gi.cprintf(other, PRINT_HIGH, "Player \"%s\" has summoned you!  To disable this feature use /nosummon in console.\n", ent->client->pers.netname);

	other->s.event = EV_OTHER_TELEPORT;

	gi.linkentity (other);

	Blinky_ResetView(other); /* FS */

	ent->client->summon_time = level.time + sv_coop_summon_time->intValue;
}

static void Teleport (edict_t *ent, edict_t *other)
{
	vec3_t offset, forward, right, start;
	trace_t tr;

	if (!ent || !ent->inuse || !ent->client || !other || !other->inuse || !other->client)
	{
		return;
	}

	if (other->client->summon_time > level.time)
	{
		gi.cprintf(other, PRINT_HIGH, "You must wait at least %d second(s) before you can teleport to another player.\n", (int)other->client->summon_time - (int)level.time);
		return;
	}

	if (ent->deadflag == DEAD_DEAD)
	{
		gi.cprintf(other, PRINT_HIGH, "Player \"%s\" is dead, aborting teleport!\n", ent->client->pers.netname);
		return;
	}

	VectorSet(offset, 40, 0, ent->viewheight - 8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// code mostly copied from KillBox

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	tr = gi.trace (start, other->mins, other->maxs, start, NULL, MASK_PLAYERSOLID);
	if (tr.fraction < 1.0)
	{
		gi.cprintf(other, PRINT_HIGH, "Collision issue.  Can not teleport to \"%s\".\n", ent->client->pers.netname);
		gi.linkentity (other);
		return;
	}

	VectorCopy (start, other->s.origin);
	VectorCopy (start, other->s.old_origin);
	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160 >> 3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	MoveToAngles(other, ent->s.angles);

	Blinky_ResetView(other); /* FS */

	// kill anything at the destination
	KillBox (other);

	gi.cprintf(other, PRINT_HIGH, "Teleported to \"%s\"!\n", ent->client->pers.netname);

	other->s.event = EV_OTHER_TELEPORT;
	gi.linkentity (other);

	other->client->summon_time = level.time + sv_coop_summon_time->intValue;
}

void Cmd_NoSummon_f (edict_t *ent)
{
	if (!ent || !ent->inuse || !ent->client || ent->client->pers.spectator)
	{
		return;
	}

	if (sv_coop_blinky_cam_disallowflags->intValue & BCAM_NOSUMMON)
	{
		gi.cprintf(ent, PRINT_HIGH, "Summon command disabled on this server!\n");
		return;
	}

	if (!ent->client->pers.noSummon)
	{
		gi.cprintf(ent, PRINT_HIGH, "Summon: Disabled!\n");
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Summon: Enabled!\n");
	}
	ent->client->pers.noSummon = !ent->client->pers.noSummon;
}

void Cmd_Runrun_f (edict_t *ent)
{
	int runrun;

	if (!ent || !ent->client || ent->client->pers.spectator)
	{
		return;
	}

	if (sv_coop_blinky_cam_disallowflags->intValue & BCAM_NORUNRUN)
	{
		gi.cprintf(ent, PRINT_HIGH, "Auto-run command disabled on this server!\n");
		return;
	}

	if (ent->client->blinky_client.runrun)
	{
		gi.cprintf(ent, PRINT_HIGH, "Auto-run: Disabled!\n");
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Auto-run: Enabled!\n");
	}

	runrun = !ent->client->blinky_client.runrun;
	ent->client->blinky_client.runrun = runrun;
}

void Cmd_Summon_f (edict_t *ent)
{
	char *name = gi.args();
	edict_t *target = NULL;
	int i;

	if (!ent || !ent->client || ent->client->pers.spectator)
	{
		return;
	}

	if (sv_coop_blinky_cam_disallowflags->intValue & BCAM_NOSUMMON)
	{
		gi.cprintf(ent, PRINT_HIGH, "Summon command disabled on this server!\n");
		return;
	}

	if (name[0] && !Q_stricmp(name, ent->client->pers.netname))
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't summon yourself!\n");
		return;
	}

	if (name[0])
	{
		if (!strncmp(name, "CL ", 3))
		{
			int playernum;

			name += 3;
			playernum = atoi(name);
			if (playernum > game.maxclients)
			{
				gi.cprintf(ent, PRINT_HIGH, "Player #%d greater than maxclients.  Aborting search!\n", playernum);
				return;
			}

			target = &g_edicts[playernum + 1];
			if (!target || !target->inuse || !target->client)
			{
				gi.cprintf(ent, PRINT_HIGH, "Couldn't find a player to summon!\n");
				return;
			}

			if (IsSpectator(target))
			{
				gi.cprintf(ent, PRINT_HIGH, "You can't summon a spectator!\n");
				return;
			}

			if (target == ent)
			{
				gi.cprintf(ent, PRINT_HIGH, "You can't summon yourself!\n");
				return;
			}

			Summon(ent, target);
			return;
		}

		if (!strncmp(name, "LIKE ", 5))
		{
			name += 5;
			target = Find_LikePlayer(ent, name, false);
		}
		else
		{
			target = Find_LikePlayer(ent, name, true);
		}

		if ((target) && (target == ent))
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't summon yourself!\n");
			return;
		}

		if ((target) && (IsSpectator(target)))
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't summon a spectator!\n");
			return;
		}
	}
	else
	{
		for (i = 0; i < game.maxclients; i++)
		{
			target = &g_edicts[i + 1];

			if (!target->inuse || !target->client || IsSpectator(target))
				continue;
			if (target == ent)
				continue;

			Summon(ent, target);
			return;
		}
	}

	if (target && target->inuse && !IsSpectator(target) && target != ent)
	{
		Summon(ent, target);
		return;
	}

	if (name[0])
	{
		gi.cprintf(ent, PRINT_HIGH, "Couldn't find player \"%s\" to summon!\n", name);
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Couldn't find a player to summon!\n");
	}
}

void Cmd_Teleport_f (edict_t *ent)
{
	char *name = gi.args();
	edict_t *player;
	qboolean exactMatch = true;

	if (!ent || !ent->client || ent->client->pers.spectator)
	{
		return;
	}

	if (sv_coop_blinky_cam_disallowflags->intValue & BCAM_NOTELEPORT)
	{
		gi.cprintf(ent, PRINT_HIGH, "Teleport command disabled on this server!\n");
		return;
	}

	if (!name[0])
	{
		gi.cprintf(ent, PRINT_HIGH, "Usage: teleport [LIKE/CL] <playername>\n");
		return;
	}

	if (!Q_stricmp(name, ent->client->pers.netname))
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't teleport to yourself!\n");
		return;
	}

	if (!strncmp(name, "LIKE ", 5))
	{
		name += 5;
		exactMatch = false;
	}

	if (!strncmp(name, "CL ", 3))
	{
		int playernum;

		name += 3;
		playernum = atoi(name);
		if (playernum > game.maxclients)
		{
			gi.cprintf(ent, PRINT_HIGH, "Player #%d greater than maxclients.  Aborting search!\n", playernum);
		}

		player = &g_edicts[playernum + 1];
		if (!player || !player->inuse || !player->client)
		{
			gi.cprintf(ent, PRINT_HIGH, "Couldn't find player \"%s\" to teleport to!\n", name);
		}

		if (IsSpectator(player))
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't teleport to spectators!\n");
			return;
		}

		if (player == ent)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't teleport to yourself!\n");
			return;
		}

		Teleport(player, ent);
		return;
	}

	player = Find_LikePlayer(ent, name, exactMatch);

	if (player && player->inuse && !IsSpectator(player) && player != ent)
	{
		Teleport(player, ent);
		return;
	}

	if ((player) && (IsSpectator(player)))
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't teleport to spectators!\n");
		return;
	}

	if ((player) && (player == ent))
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't teleport to yourself!\n");
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "Couldn't find player \"%s\" to teleport to!\n", name);
}

void Blinky_OnClientTerminate (edict_t *self)
{
	edict_t *player;
	BlinkyClient_t *bdata;

	if (!self)
	{
		return;
	}

	/* disconnect any cams */
	for (player = &g_edicts[0] + 1; player < &g_edicts[0] + (int)(maxclients->intValue) + 1; player++)
	{
		if (!player->client || !player->inuse)
			continue;
		bdata = &player->client->blinky_client;
		if (bdata->cam_target == self)
			stopBlinkyCam(player);
	}
}

void Blinky_CalcViewOffsets (edict_t *ent, vec3_t v)
{
	int i;
	BlinkyClient_t *bdata = NULL;
	vec3_t forward;
	edict_t *target = NULL;

	if (!ent || !ent->inuse || !ent->client)
		return;

	bdata = &ent->client->blinky_client;
	target = bdata->cam_target;
	if (!target || !target->inuse || !target->client)
		return;

	VectorSet(v, 0.0f, 0.0f, 0.0f);

	for (i = 0; i < 3; i++)
	{
		ent->client->ps.pmove.origin[i] = target->s.origin[i] * 8;
	}

	v[2] += target->viewheight;

	// move cam forward
	if (coop_cameraoffset->intValue)
	{
		AngleVectors (target->client->v_angle, forward, NULL, NULL);
		VectorMA (v, coop_cameraoffset->intValue, forward, v);
	}
}
