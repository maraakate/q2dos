/* cd_sdl.c
 * Copyright (C) 1996-1997  Id Software, Inc.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

#ifndef	SDL_INIT_CDROM

/* SDL dropped support for
   cd audio since v1.3.0 */
#pragma message("Warning: SDL CDAudio support disabled")
#include "../null/cd_null.c"

#else	/* SDL_INIT_CDROM */

#include "client.h"

static qboolean cdValid = false;
static qboolean	playing = false;
static qboolean	wasPlaying = false;
static qboolean	enabled = true;
static qboolean playLooping = false;
static byte	remap[100];
static byte	playTrack;
static int	endOfTrack = -1, pausetime = -1;
static SDL_CD	*cd_handle;
static int	cd_index = -1;
static float	old_cdvolume;
static qboolean	hw_vol_works = true;

static cvar_t	*cd_volume;
static cvar_t	*cd_nocd;
static cvar_t	*cd_dev;

static void CDAudio_Eject(void)
{
	if (!cd_handle || !enabled)
		return;

	if (SDL_CDEject(cd_handle) == -1)
		Com_Printf ("Unable to eject CD-ROM: %s\n", SDL_GetError ());
}

static int CDAudio_GetAudioDiskInfo(void)
{
	cdValid = false;

	if (!cd_handle)
		return -1;

	if ( ! CD_INDRIVE(SDL_CDStatus(cd_handle)) )
		return -1;

	cdValid = true;

	return 0;
}

void CDAudio_Pause(void);

void CDAudio_Play(int track, qboolean looping)
{
	int	len_m, len_s, len_f;

	if (!cd_handle || !enabled)
		return;

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
			return;
	}

	track = remap[track];

	if (track < 1 || track > cd_handle->numtracks)
	{
		Com_Printf ("%s: Bad track number %d.\n", __FUNCTION__, track);
		return;
	}

	if (cd_handle->track[track-1].type == SDL_DATA_TRACK)
	{
		Com_Printf ("%s: track %d is not audio\n", __FUNCTION__, track);
		return;
	}

	if (playing)
	{
		if (playTrack == track)
			return;
		CDAudio_Stop();
	}

	if (SDL_CDPlay(cd_handle, cd_handle->track[track-1].offset, cd_handle->track[track-1].length) == -1)
	{
		Com_Printf ("%s: Unable to play track %d: %s\n", __FUNCTION__, track, SDL_GetError ());
		return;
	}

	playLooping = looping;
	playTrack = track;
	playing = true;

	/* curtime is in milliseconds */
	FRAMES_TO_MSF(cd_handle->track[track-1].length, &len_m, &len_s, &len_f);
	endOfTrack = (curtime / 1000) + (len_m * 60) + len_s + (len_f / CD_FPS);

	/* Add the pregap for the next track.  This means that disc-at-once CDs
	 * won't loop smoothly, but they wouldn't anyway so it doesn't really
	 * matter.  SDL doesn't give us pregap information anyway, so you'll
	 * just have to live with it.  */
	endOfTrack += 2;
	pausetime = -1;

	if (cd_volume->value == 0) /* don't bother advancing */
		CDAudio_Pause ();
}

void CDAudio_Stop(void)
{
	if (!cd_handle || !enabled)
		return;

	if (!playing)
		return;

	if (SDL_CDStop(cd_handle) == -1)
		Com_Printf ("%s: Unable to stop CD-ROM (%s)\n", __FUNCTION__, SDL_GetError());

	wasPlaying = false;
	playing = false;
	pausetime = -1;
	endOfTrack = -1;
}

void CDAudio_Pause(void)
{
	if (!cd_handle || !enabled)
		return;

	if (!playing)
		return;

	if (SDL_CDPause(cd_handle) == -1)
		Com_Printf ("Unable to pause CD-ROM: %s\n", SDL_GetError());

	wasPlaying = playing;
	playing = false;
	pausetime = curtime / 1000;
}

void CDAudio_Resume(void)
{
	if (!cd_handle || !enabled)
		return;

	if (!cdValid)
		return;

	if (!wasPlaying)
		return;

	if (SDL_CDResume(cd_handle) == -1)
		Com_Printf ("Unable to resume CD-ROM: %s\n", SDL_GetError());
	playing = true;
	endOfTrack += (curtime / 1000) - pausetime;
	pausetime = -1;
}

static void CD_f (void)
{
	const char	*command;
	int		ret, n;

	if (Cmd_Argc() < 2)
	{
		Com_Printf("commands:");
		Com_Printf("on, off, reset, remap, \n");
		Com_Printf("play, stop, loop, pause, resume\n");
		Com_Printf("eject, info\n");
		return;
	}

	command = Cmd_Argv (1);

	if (Q_stricmp(command, "on") == 0)
	{
		enabled = true;
		return;
	}

	if (Q_stricmp(command, "off") == 0)
	{
		if (playing)
			CDAudio_Stop();
		enabled = false;
		return;
	}

	if (Q_stricmp(command, "reset") == 0)
	{
		enabled = true;
		if (playing)
			CDAudio_Stop();
		for (n = 0; n < 100; n++)
			remap[n] = n;
		CDAudio_GetAudioDiskInfo();
		return;
	}

	if (Q_stricmp(command, "remap") == 0)
	{
		ret = Cmd_Argc() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 100; n++)
				if (remap[n] != n)
					Com_Printf("  %u -> %u\n", n, remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			remap[n] = atoi(Cmd_Argv (n + 1));
		return;
	}

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
		{
			Com_Printf("No CD in player.\n");
			return;
		}
	}

	if (Q_stricmp(command, "play") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), false);
		return;
	}

	if (Q_stricmp(command, "loop") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), true);
		return;
	}

	if (Q_stricmp(command, "stop") == 0)
	{
		CDAudio_Stop();
		return;
	}

	if (Q_stricmp(command, "pause") == 0)
	{
		CDAudio_Pause();
		return;
	}

	if (Q_stricmp(command, "resume") == 0)
	{
		CDAudio_Resume();
		return;
	}

	if (Q_stricmp(command, "eject") == 0)
	{
		if (playing)
			CDAudio_Stop();
		CDAudio_Eject();
		cdValid = false;
		return;
	}

	if (Q_stricmp(command, "info") == 0)
	{
		int	current_min, current_sec, current_frame;
		int	length_min, length_sec, length_frame;

		Com_Printf ("%u tracks\n", cd_handle->numtracks);

		if (playing)
			Com_Printf("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		else if (wasPlaying)
			Com_Printf("Paused %s track %u\n", playLooping ? "looping" : "playing", playTrack);

		if (playing || wasPlaying)
		{
			SDL_CDStatus(cd_handle);
			FRAMES_TO_MSF(cd_handle->cur_frame, &current_min, &current_sec, &current_frame);
			FRAMES_TO_MSF(cd_handle->track[playTrack-1].length, &length_min, &length_sec, &length_frame);

			Com_Printf ("Current position: %d:%02d.%02d (of %d:%02d.%02d)\n",
						current_min, current_sec, current_frame * 60 / CD_FPS,
						length_min, length_sec, length_frame * 60 / CD_FPS);
		}
		Com_Printf("Volume is %f\n", cd_volume->value);

		return;
	}
}

static qboolean CD_GetVolume (void *unused)
{
/* FIXME: write proper code in here when SDL
   supports cdrom volume control some day. */
	return false;
}

static qboolean CD_SetVolume (void *unused)
{
/* FIXME: write proper code in here when SDL
   supports cdrom volume control some day. */
	return false;
}

static qboolean CDAudio_SetVolume (float value)
{
	if (!cd_handle || !enabled)
		return false;

	old_cdvolume = value;

	if (value == 0.0f)
		CDAudio_Pause ();
	else
		CDAudio_Resume();

	if (!hw_vol_works)
	{
		return false;
	}
	else
	{
/* FIXME: write proper code in here when SDL
   supports cdrom volume control some day. */
		return CD_SetVolume (NULL);
	}
}

void CDAudio_Update(void)
{
	CDstatus	curstat;

	if (!cd_handle || !enabled)
		return;

	if (old_cdvolume != cd_volume->value)
		CDAudio_SetVolume (cd_volume->value);

	if (playing && curtime / 1000 > endOfTrack)
	{
		curstat = SDL_CDStatus(cd_handle);
		if (curstat != CD_PLAYING && curstat != CD_PAUSED)
		{
			playing = false;
			endOfTrack = -1;
			if (playLooping)
				CDAudio_Play(playTrack, true);
		}
	}
}

static const char *get_cddev_arg (const char *arg)
{
#if defined(_WIN32)
/* arg should be like "D:\", make sure it is so,
 * but tolerate args like "D" or "D:", as well. */
	static char drive[4];
	if (!arg || ! *arg)
		return NULL;
	if (arg[1] != '\0')
	{
		if (arg[1] != ':')
			return NULL;
		if (arg[2] != '\0')
		{
			if (arg[2] != '\\' &&
			    arg[2] != '/')
				return NULL;
			if (arg[3] != '\0')
				return NULL;
		}
	}
	if (*arg >= 'A' && *arg <= 'Z')
	{
		drive[0] = *arg;
		drive[1] = ':';
		drive[2] = '\\';
		drive[3] = '\0';
		return drive;
	}
	else if (*arg >= 'a' && *arg <= 'z')
	{
	/* make it uppercase for SDL */
		drive[0] = *arg - ('a' - 'A');
		drive[1] = ':';
		drive[2] = '\\';
		drive[3] = '\0';
		return drive;
	}
	return NULL;
#else
	if (!arg || ! *arg)
		return NULL;
	return arg;
#endif
}

static void export_cddev_arg (void)
{
/* Bad ugly hack to workaround SDL's cdrom device detection.
 * not needed for windows due to the way SDL_cdrom works. */
#if !defined(_WIN32)
	if (cd_dev->string[0] != '\0')
	{
		static char arg[64];
		Com_sprintf(arg, sizeof(arg), "SDL_CDROM=%s", cd_dev->string);
		putenv(arg);
	}
#endif
}

int CDAudio_Init(void)
{
	int	i, sdl_num_drives;

	cd_nocd = Cvar_Get ("cd_nocd", "0", CVAR_ARCHIVE);
	cd_volume = Cvar_Get ("cd_volume", "1", CVAR_ARCHIVE);
	cd_dev = Cvar_Get ("cd_dev", "", CVAR_ARCHIVE);

	if ((Cvar_Get ("nocdaudio", "0", CVAR_NOSET))->value)
		return -1;
	if (cd_nocd->value)
		return -1;

	export_cddev_arg();

	if (SDL_InitSubSystem(SDL_INIT_CDROM) < 0)
	{
		Com_Printf("Couldn't init SDL cdrom: %s\n", SDL_GetError());
		return -1;
	}

	sdl_num_drives = SDL_CDNumDrives ();
	Com_Printf ("SDL detected %d CD-ROM drive%c\n", sdl_num_drives,
					sdl_num_drives == 1 ? ' ' : 's');

	if (sdl_num_drives < 1)
		return -1;

	if (cd_dev->string[0] != '\0')
	{
		const char *userdev = get_cddev_arg(cd_dev->string);
		if (!userdev)
		{
			Com_Printf("Invalid cd_dev value\n");
			return -1;
		}
		for (i = 0; i < sdl_num_drives; i++)
		{
			if (!Q_stricmp(SDL_CDName(i), userdev))
			{
				cd_index = i;
				break;
			}
		}
		if (cd_index == -1)
		{
			Com_Printf("SDL couldn't find cdrom device %s\n", userdev);
			return -1;
		}
	}

	if (cd_index == -1)
		cd_index = 0;	/* default drive */

	cd_handle = SDL_CDOpen(cd_index);
	if (!cd_handle)
	{
		Com_Printf ("%s: Unable to open CD-ROM drive %s (%s)\n",
				__FUNCTION__, SDL_CDName(cd_index), SDL_GetError());
		return -1;
	}

	for (i = 0; i < 100; i++)
		remap[i] = i;
	enabled = true;
	old_cdvolume = cd_volume->value;

	Com_Printf("CDAudio initialized (SDL, using %s)\n", SDL_CDName(cd_index));

	if (CDAudio_GetAudioDiskInfo())
	{
		Com_Printf("%s: No CD in drive\n", __FUNCTION__);
		cdValid = false;
	}

	Cmd_AddCommand ("cdplayer", CD_f);

	hw_vol_works = CD_GetVolume (NULL); /* no SDL support at present. */
	if (hw_vol_works)
		hw_vol_works = CDAudio_SetVolume (cd_volume->value);

	return 0;
}

void CDAudio_Shutdown(void)
{
	if (!cd_handle)
		return;
	CDAudio_Stop();
	if (hw_vol_works)
		CD_SetVolume (NULL); /* no SDL support at present. */
	SDL_CDClose(cd_handle);
	cd_handle = NULL;
	cd_index = -1;
	SDL_QuitSubSystem(SDL_INIT_CDROM);
}

/*
===========
CDAudio_Activate

Called when the main window gains or loses focus.
The window have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void CDAudio_Activate (qboolean active)
{
	if (active)
		CDAudio_Resume ();
	else
		CDAudio_Pause ();
}

qboolean CDAudio_Active (void)
{
	return playing;
}

#endif	/* SDL_INIT_CDROM */
