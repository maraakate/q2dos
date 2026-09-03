//****************************************************
//		Flashlight
//****************************************************

#include "g_local.h"
#include "flashlight.h"

void FlashlightReset(edict_t* self)
{
	if (!self)
	{
		return;
	}

	if (self->owner && self->owner->client && self->owner->client->flashlight)
	{
		float volume = 1.0f;

		if (self->owner->client->silencer_shots)
			volume = 0.2f;
		gi.sound(self->owner, CHAN_AUTO, gi.soundindex("misc/flashlight.wav"), volume, ATTN_NORM, 0.0f);
		self->owner->client->flashlight = NULL;
		self->owner->client->flashtype = 0;
	}

	G_FreeEdict(self);
}

void flashlight_think(edict_t* self)
{
	vec3_t	forward, right, up, offset = { 0 }, start, end;
	trace_t	tr;
	int content = (CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);

	if (!self || !self->inuse)
	{
		return;
	}

	if (level.intermissiontime || !self->owner || !self->owner->inuse || self->owner->deadflag)
	{
		FlashlightReset(self);
		return;
	}
	AngleVectors(self->owner->client->v_angle, forward, right, up);
	VectorSet(offset, 0, 8, self->owner->viewheight - 8.0f);
	P_ProjectSource(self->owner->client, self->owner->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, self->owner, content);
	if (flashlightmode->intValue)
		self->svflags &= ~SVF_NOCLIENT;
	else
	{
		if (!((tr.surface) && (tr.surface->flags & SURF_SKY)) && tr.fraction < 1.0)
			self->svflags &= ~SVF_NOCLIENT;
		else
			self->svflags |= SVF_NOCLIENT;
	}

	vectoangles(tr.plane.normal, self->s.angles);
	VectorCopy(tr.endpos, self->s.origin);
	self->nextthink = level.time + FRAMETIME;

	if (flashlightmode->intValue)
	{
		//QW// new, nice white flashlight
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_FLASHLIGHT);
		gi.WritePosition(self->s.origin);
		gi.WriteShort(self - g_edicts);
		gi.multicast(self->s.origin, MULTICAST_PVS);
	}
	gi.linkentity(self);
}


void Use_Flashlight(edict_t* player)
{
	if (!player || !player->client || !player->inuse)
	{
		return;
	}

	if (player->client->flashlight)
	{
		if (player->client->flashtype == 0)
		{
			player->client->flashtype = 1;
			player->client->flashlight->s.effects = 0;
			player->client->flashlight->s.renderfx = RF_TRANSLUCENT;
			gi.linkentity(player->client->flashlight);
		}
		else if (player->client->flashtype == 1)
		{
			player->client->flashtype = 2;
			player->client->flashlight->s.effects = EF_HYPERBLASTER;
			gi.linkentity(player->client->flashlight);
		}
		else
		{
			player->client->flashtype = 0;
			FlashlightReset(player->client->flashlight);
		}
	}
	else
	{
		float volume = 1.0;

		if (player->client->silencer_shots)
			volume = 0.2f;

		gi.sound(player, CHAN_AUTO, gi.soundindex("misc/flashlight.wav"), volume, ATTN_NORM, 0);

		player->client->flashlight = G_Spawn();
		VectorCopy(player->s.origin, player->client->flashlight->s.origin);
		player->client->flashlight->movetype = MOVETYPE_NOCLIP;
		player->client->flashlight->clipmask = MASK_SHOT;
		player->client->flashlight->solid = SOLID_NOT;
		player->client->flashlight->owner = player;
		player->client->flashlight->nextthink = level.time + FRAMETIME;
		player->client->flashlight->think = flashlight_think;
		player->client->flashlight->classname = "flashlight";
		player->client->flashlight->svflags = SVF_NOCLIENT;
		player->client->flashlight->s.frame = 0;
		player->client->flashlight->s.skinnum = 0;
		player->client->flashlight->s.modelindex = 0;

		VectorClear(player->client->flashlight->mins);
		VectorClear(player->client->flashlight->maxs);

		if (flashlightmode->intValue == 0)
		{
			//QW// old, dim flashlight
			player->client->flashlight->s.effects = EF_HYPERBLASTER;
			player->client->flashlight->s.renderfx |= RF_TRANSLUCENT;
			player->client->flashlight->s.renderfx |= RF_FULLBRIGHT;
			player->client->flashlight->s.modelindex = gi.modelindex("models/objects/dummy/tris.md2");
		}

		gi.linkentity(player->client->flashlight);
	}
}

void Cmd_Flashlight(edict_t* ent)
{
	if (!ent || !ent->inuse || !ent->client || ent->health <= 0)
		return;

	if (ent->client->pers.spectator || ent->client->blinky_client.cam_target) /* FS: Coop: No spectators and cheats, please */
	{
		gi.cprintf(ent, PRINT_HIGH,
				"Spectators can't use this command.\n");
		return;
	}

	if (ent->client->flashlight == NULL)
	{
		gi.cprintf(ent, PRINT_HIGH, "Flashlight On\n");
		Use_Flashlight(ent);
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Flashlight Off\n");
		FlashlightReset(ent->client->flashlight);
	}
}
