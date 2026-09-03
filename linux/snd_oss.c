/* snd_oss.c
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
#include "snd_loc.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/soundcard.h>
#include <errno.h>

static int snd_inited = 0;
static int audio_fd = -1;
static unsigned long mmaplen;

static cvar_t *sndbits;
static cvar_t *sndchannels;
static cvar_t *snddevice;

static const int tryrates[] = { 11025, 22050, 44100, 48000, 96000, 16000, 24000, 8000 };
static const int MAX_TRYRATES = sizeof(tryrates)/sizeof(tryrates[0]);

qboolean SNDDMA_Init(void)
{
	int		i, caps, tmp;
	unsigned long		sz;
	struct audio_buf_info	info;

	if (snd_inited)
		return true;

	if (!sndbits) {
		sndbits = Cvar_Get("sndbits", "16", CVAR_ARCHIVE);
		sndchannels = Cvar_Get("sndchannels", "2", CVAR_ARCHIVE);
		snddevice = Cvar_Get("snddevice", "/dev/dsp", CVAR_ARCHIVE);
	}

	audio_fd = open(snddevice->string, O_RDWR|O_NONBLOCK);
	if (audio_fd == -1)
	{	/* retry up to 3 times if it's busy */
		tmp = 3;
		while (audio_fd == -1 && tmp-- &&
				(errno == EAGAIN || errno == EBUSY))
		{
			usleep (300000);
			audio_fd = open(snddevice->string, O_RDWR|O_NONBLOCK);
		}
		if (audio_fd == -1)
		{
			Com_Printf("Could not open %s. %s\n", snddevice->string, strerror(errno));
			return false;
		}
	}

	if (ioctl(audio_fd, SNDCTL_DSP_RESET, 0) == -1)
	{
		Com_Printf("Could not reset %s. %s\n", snddevice->string, strerror(errno));
		goto error;
	}

	if (ioctl(audio_fd, SNDCTL_DSP_GETCAPS, &caps) == -1)
	{
		Com_Printf("Couldn't retrieve soundcard capabilities. %s\n", strerror(errno));
		goto error;
	}

	if (!(caps & DSP_CAP_TRIGGER) || !(caps & DSP_CAP_MMAP))
	{
		Com_Printf("Audio driver doesn't support mmap or trigger\n");
		goto error;
	}

	memset (&dma, 0, sizeof(dma_t));

	/* set format & rate */
	tmp = (sndbits->intValue == 16) ? AFMT_S16_NE : AFMT_U8;
	if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &tmp) == -1)
	{
		Com_Printf("Problems setting %d bit format, trying alternatives..\n", sndbits->intValue);
		/* try what the device gives us */
		if (ioctl(audio_fd, SNDCTL_DSP_GETFMTS, &tmp) == -1)
		{
			Com_Printf("Unable to retrieve supported formats. %s\n", strerror(errno));
			goto error;
		}
		i = tmp;
		if (i & AFMT_S16_NE)
			tmp = AFMT_S16_NE;
		else if (i & AFMT_U8)
			tmp = AFMT_U8;
		else
		{
			Com_Printf("Neither 8 nor 16 bit format supported.\n");
			goto error;
		}
		if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &tmp) == -1)
		{
			Com_Printf("Unable to set sound format. %s\n", strerror(errno));
			goto error;
		}
	}
	if (tmp == AFMT_S16_NE)
		dma.samplebits = 16;
	else if (tmp == AFMT_U8)
		dma.samplebits = 8;
	else { /* unreached */
		goto error;
	}

	if (s_khz->intValue < 11025)
		tmp = 11025;
	else
		tmp = s_khz->intValue;
	dma.speed = tmp;

	if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &tmp) == -1)
	{
		Com_Printf("Problems setting sample rate, trying alternatives..\n");
		dma.speed = 0;
		for (i = 0; i < MAX_TRYRATES; i++)
		{
			tmp = tryrates[i];
			if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &tmp) == -1)
			{
				Com_DPrintf (DEVELOPER_MSG_SOUND, "Unable to set sample rate %d\n", tryrates[i]);
			}
			else
			{
				if (tmp != tryrates[i])
				{
					Com_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", tmp, tryrates[i]);
				/*	goto error;*/
				}
				dma.speed = tmp;
				break;
			}
		}
		if (dma.speed == 0)
		{
			Com_Printf("Unable to set any sample rates.\n");
			goto error;
		}
	}
	else
	{
		if (tmp != dma.speed)
		{
			Com_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", tmp, dma.speed);
		/*	goto error;*/
		}
		dma.speed = tmp;
	}

	tmp = (sndchannels->intValue == 2) ? 1 : 0;
	if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &tmp) == -1)
	{
		Com_Printf ("Problems setting channels to %s, retrying for %s\n",
				(sndchannels->intValue == 2) ? "stereo" : "mono",
				(sndchannels->intValue == 2) ? "mono" : "stereo");
		tmp = (sndchannels->intValue == 2) ? 0 : 1;
		if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &tmp) == -1)
		{
			Com_Printf("unable to set desired channels. %s\n", strerror(errno));
			goto error;
		}
	}
	dma.channels = tmp +1;

	if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info) == -1)
	{
		Com_Printf("Couldn't retrieve buffer status. %s\n", strerror(errno));
		goto error;
	}

	dma.samples = info.fragstotal * info.fragsize / (dma.samplebits / 8);
	dma.submission_chunk = 1;

	/* memory map the dma buffer */
	sz = sysconf (_SC_PAGESIZE);
	mmaplen = info.fragstotal * info.fragsize;
	mmaplen += sz - 1;
	mmaplen &= ~(sz - 1);
	dma.buffer = (unsigned char *) mmap(NULL, mmaplen, PROT_READ|PROT_WRITE,
					     MAP_FILE|MAP_SHARED, audio_fd, 0);
	if (dma.buffer == MAP_FAILED)
	{
		Com_Printf("Could not mmap %s. %s\n", snddevice->string, strerror(errno));
		goto error;
	}
	Com_Printf ("OSS: mmaped %lu bytes buffer\n", mmaplen);

	/* toggle the trigger & start her up */
	tmp = 0;
	if (ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp) == -1)
	{
		Com_Printf("Could not toggle %s. %s\n", snddevice->string, strerror(errno));
		goto error;
	}
	tmp = PCM_ENABLE_OUTPUT;
	if (ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp) == -1)
	{
		Com_Printf("Could not toggle %s. %s\n", snddevice->string, strerror(errno));
		goto error;
	}

	dma.samplepos = 0;
	snd_inited = 1;
	return true;

error:
	if (dma.buffer && dma.buffer != MAP_FAILED)
		munmap (dma.buffer, mmaplen);
	dma.buffer = NULL;
	close(audio_fd);
	audio_fd = -1;
	return false;
}

int SNDDMA_GetDMAPos (void)
{
	struct count_info	count;

	if (!snd_inited) return 0;

	if (ioctl(audio_fd, SNDCTL_DSP_GETOPTR, &count) == -1)
	{
		Com_Printf("Uh, sound dead. %s\n", strerror(errno));
		munmap (dma.buffer, mmaplen);
		dma.buffer = NULL;
		close(audio_fd);
		audio_fd = -1;
		snd_inited = 0;
		return 0;
	}
	dma.samplepos = count.ptr / (dma.samplebits / 8);

	return dma.samplepos;
}

void SNDDMA_Shutdown (void)
{
	int	tmp = 0;

	if (!snd_inited) return;
	Com_Printf ("Shutting down OSS sound\n");
	munmap (dma.buffer, mmaplen);
	dma.buffer = NULL;
	ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
	close(audio_fd);
	audio_fd = -1;
	snd_inited = 0;
}

/*
==============
SNDDMA_BeginPainting

Makes sure dma buffer is valid
==============
*/
void SNDDMA_BeginPainting (void)
{
	/* nothing to do here */
}

/*
==============
SNDDMA_Submit

Unlock the dma buffer /
Send sound to the device
===============
*/
void SNDDMA_Submit(void)
{
}
