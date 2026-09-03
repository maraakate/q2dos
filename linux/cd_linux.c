/* cd_linux.c
 * Copyright (C) 1996-2001  Id Software, Inc.
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

#include "client.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <linux/cdrom.h>
#include <paths.h>

static qboolean cdValid = false;
static qboolean	playing = false;
static qboolean	wasPlaying = false;
static qboolean	initialized = false;
static qboolean	enabled = true;
static qboolean playLooping = false;
static byte	remap[100];
static byte	playTrack;
static byte	maxTrack;

static int	cdfile = -1;
static cvar_t	*cd_volume;
static cvar_t	*cd_nocd;
static cvar_t	*cd_dev;

static float	old_cdvolume;
static qboolean	hw_vol_works = true;
static struct cdrom_volctrl	orig_vol;	/* original setting to be restored upon exit */
static struct cdrom_volctrl	drv_vol;	/* the volume setting we'll be using */


#define IOCTL_FAILURE(__name)	do {							\
	int __err = errno;								\
	Com_DPrintf(DEVELOPER_MSG_CD, "ioctl %s failed (%d: %s)\n", #__name, __err, strerror(__err));	\
} while (0)


static void CDAudio_Eject(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (ioctl(cdfile, CDROMEJECT) == -1)
		IOCTL_FAILURE(CDROMEJECT);
}

static void CDAudio_CloseDoor(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (ioctl(cdfile, CDROMCLOSETRAY) == -1)
		IOCTL_FAILURE(CDROMCLOSETRAY);
}

static int CDAudio_GetAudioDiskInfo(void)
{
	struct cdrom_tochdr tochdr;

	if (cdfile == -1)
		return -1;

	cdValid = false;

	if (ioctl(cdfile, CDROMREADTOCHDR, &tochdr) == -1)
	{
		IOCTL_FAILURE(CDROMREADTOCHDR);
		return -1;
	}

	if (tochdr.cdth_trk0 < 1)
	{
		Com_DPrintf(DEVELOPER_MSG_CD, "CDAudio: no music tracks\n");
		return -1;
	}

	cdValid = true;
	maxTrack = tochdr.cdth_trk1;

	return 0;
}

void CDAudio_Pause(void);

void CDAudio_Play(int track, qboolean looping)
{
	struct cdrom_tocentry entry;
	struct cdrom_ti ti;

	if (cdfile == -1 || !enabled)
		return;

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
			return;
	}

	track = remap[track];

	if (track < 1 || track > maxTrack)
	{
		Com_DPrintf(DEVELOPER_MSG_CD, "CDAudio: Bad track number %u.\n", track);
		return;
	}

	/* don't try to play a non-audio track */
	entry.cdte_track = track;
	entry.cdte_format = CDROM_MSF;
	if (ioctl(cdfile, CDROMREADTOCENTRY, &entry) == -1)
	{
		IOCTL_FAILURE(CDROMREADTOCENTRY);
		return;
	}
	if (entry.cdte_ctrl == CDROM_DATA_TRACK)
	{
		Com_Printf("CDAudio: track %i is not audio\n", track);
		return;
	}

	if (playing)
	{
		if (playTrack == track)
			return;
		CDAudio_Stop();
	}

	ti.cdti_trk0 = track;
	ti.cdti_trk1 = track;
	ti.cdti_ind0 = 1;
	ti.cdti_ind1 = 99;

	if (ioctl(cdfile, CDROMPLAYTRKIND, &ti) == -1)
	{
		IOCTL_FAILURE(CDROMPLAYTRKIND);
		return;
	}

	if (ioctl(cdfile, CDROMRESUME) == -1)
	{
		IOCTL_FAILURE(CDROMRESUME);
		return;
	}

	playLooping = looping;
	playTrack = track;
	playing = true;

	if (cd_volume->value == 0) /* don't bother advancing */
		CDAudio_Pause ();
}

void CDAudio_Stop(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (!playing)
		return;

	if (ioctl(cdfile, CDROMSTOP) == -1)
		IOCTL_FAILURE(CDROMSTOP);

	wasPlaying = false;
	playing = false;
}

void CDAudio_Pause(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (!playing)
		return;

	if (ioctl(cdfile, CDROMPAUSE) == -1)
		IOCTL_FAILURE(CDROMPAUSE);

	wasPlaying = playing;
	playing = false;
}

void CDAudio_Resume(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (!cdValid)
		return;

	if (!wasPlaying)
		return;

	if (ioctl(cdfile, CDROMRESUME) == -1)
		IOCTL_FAILURE(CDROMRESUME);
	playing = true;
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
		Com_Printf("eject, close, info\n");
		return;
	}

	command = Cmd_Argv (1);

	if (strcasecmp(command, "on") == 0)
	{
		enabled = true;
		return;
	}

	if (strcasecmp(command, "off") == 0)
	{
		if (playing)
			CDAudio_Stop();
		enabled = false;
		return;
	}

	if (strcasecmp(command, "reset") == 0)
	{
		enabled = true;
		if (playing)
			CDAudio_Stop();
		for (n = 0; n < 100; n++)
			remap[n] = n;
		CDAudio_GetAudioDiskInfo();
		return;
	}

	if (strcasecmp(command, "remap") == 0)
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
			remap[n] = atoi(Cmd_Argv (n+1));
		return;
	}

	if (strcasecmp(command, "close") == 0)
	{
		CDAudio_CloseDoor();
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

	if (strcasecmp(command, "play") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), false);
		return;
	}

	if (strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), true);
		return;
	}

	if (strcasecmp(command, "stop") == 0)
	{
		CDAudio_Stop();
		return;
	}

	if (strcasecmp(command, "pause") == 0)
	{
		CDAudio_Pause();
		return;
	}

	if (strcasecmp(command, "resume") == 0)
	{
		CDAudio_Resume();
		return;
	}

	if (strcasecmp(command, "eject") == 0)
	{
		if (playing)
			CDAudio_Stop();
		CDAudio_Eject();
		cdValid = false;
		return;
	}

	if (strcasecmp(command, "info") == 0)
	{
		Com_Printf("%u tracks\n", maxTrack);
		if (playing)
			Com_Printf("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		else if (wasPlaying)
			Com_Printf("Paused %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		Com_Printf("Volume is %f\n", cd_volume->value);
		return;
	}
}

static qboolean CD_GetVolume (struct cdrom_volctrl *vol)
{
	if (ioctl(cdfile, CDROMVOLREAD, vol) == -1)
	{
		IOCTL_FAILURE(CDROMVOLREAD);
		return false;
	}
	return true;
}

static qboolean CD_SetVolume (struct cdrom_volctrl *vol)
{
	if (ioctl(cdfile, CDROMVOLCTRL, vol) == -1)
	{
		IOCTL_FAILURE(CDROMVOLCTRL);
		return false;
	}
	return true;
}

static qboolean CDAudio_SetVolume (float value)
{
	if (cdfile == -1 || !enabled)
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
		drv_vol.channel0 = drv_vol.channel2 =
		drv_vol.channel1 = drv_vol.channel3 = value * 255.0f;
		return CD_SetVolume (&drv_vol);
	}
}

void CDAudio_Update(void)
{
	struct cdrom_subchnl subchnl;
	static time_t lastchk;

	if (cdfile == -1 || !enabled)
		return;

	if (old_cdvolume != cd_volume->value)
		CDAudio_SetVolume (cd_volume->value);

	if (playing && lastchk < time(NULL))
	{
		lastchk = time(NULL) + 2; /* two seconds between chks */
		subchnl.cdsc_format = CDROM_MSF;
		if (ioctl(cdfile, CDROMSUBCHNL, &subchnl) == -1)
		{
			IOCTL_FAILURE(CDROMSUBCHNL);
			playing = false;
			return;
		}
		if (subchnl.cdsc_audiostatus != CDROM_AUDIO_PLAY &&
			subchnl.cdsc_audiostatus != CDROM_AUDIO_PAUSED)
		{
			playing = false;
			if (playLooping)
				CDAudio_Play(playTrack, true);
		}
		else
		{
			playTrack = subchnl.cdsc_trk;
		}
	}
}

int CDAudio_Init(void)
{
	int i;

	cd_nocd = Cvar_Get ("cd_nocd", "0", CVAR_ARCHIVE);
	cd_volume = Cvar_Get ("cd_volume", "1", CVAR_ARCHIVE);
	cd_dev = Cvar_Get ("cd_dev", _PATH_DEV "cdrom", CVAR_ARCHIVE);

	if ((Cvar_Get ("nocdaudio", "0", CVAR_NOSET))->value)
		return -1;
	if (cd_nocd->value)
		return -1;

	if ((cdfile = open(cd_dev->string, O_RDONLY | O_NONBLOCK)) == -1)
	{
		i = errno;
		Com_Printf("%s: open of \"%s\" failed (%d: %s)\n",
				__FUNCTION__, cd_dev, i, strerror(i));
		cdfile = -1;
		return -1;
	}

	for (i = 0; i < 100; i++)
		remap[i] = i;
	initialized = true;
	enabled = true;
	old_cdvolume = cd_volume->value;

	Com_Printf("CDAudio initialized (using Linux ioctls)\n");

	if (CDAudio_GetAudioDiskInfo())
	{
		Com_Printf("%s: No CD in drive\n", __FUNCTION__);
		cdValid = false;
	}

	Cmd_AddCommand ("cdplayer", CD_f);

	hw_vol_works = CD_GetVolume (&orig_vol);
	if (hw_vol_works)
		hw_vol_works = CDAudio_SetVolume (cd_volume->value);

	return 0;
}

void CDAudio_Shutdown(void)
{
	if (!initialized)
		return;
	CDAudio_Stop();
	if (hw_vol_works)
		CD_SetVolume (&orig_vol);
	close(cdfile);
	cdfile = -1;
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
