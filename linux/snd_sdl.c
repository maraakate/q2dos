/* snd_sdl.c
 * Copyright (C) 1999-2005 Id Software, Inc.
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

#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

static int	buffersize;

static cvar_t *sndbits;
static cvar_t *sndchannels;

static void SDLCALL paint_audio (void *unused, Uint8 *stream, int len)
{
	int	pos, tobufend;
	int	len1, len2;

	if (!dma.buffer)
	{	/* shouldn't happen, but just in case */
		memset(stream, 0, len);
		return;
	}

	pos = (dma.samplepos * (dma.samplebits / 8));
	if (pos >= buffersize)
		dma.samplepos = pos = 0;

	tobufend = buffersize - pos;  /* bytes to buffer's end. */
	len1 = len;
	len2 = 0;

	if (len1 > tobufend)
	{
		len1 = tobufend;
		len2 = len - len1;
	}

	memcpy(stream, dma.buffer + pos, len1);

	if (len2 <= 0)
	{
		dma.samplepos += (len1 / (dma.samplebits / 8));
	}
	else
	{	/* wraparound? */
		memcpy(stream + len1, dma.buffer, len2);
		dma.samplepos = (len2 / (dma.samplebits / 8));
	}

	if (dma.samplepos >= buffersize)
		dma.samplepos = 0;
}

qboolean SNDDMA_Init (void)
{
	SDL_AudioSpec desired, obtained;
	int		tmp, val;
	char	drivername[128];

	if (!sndbits) {
		sndbits = Cvar_Get("sndbits", "16", CVAR_ARCHIVE);
		sndchannels = Cvar_Get("sndchannels", "2", CVAR_ARCHIVE);
	}

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		Com_Printf("Couldn't init SDL audio: %s\n", SDL_GetError());
		return false;
	}

	/* Set up the desired format */
	if (s_khz->intValue < 11025)
		tmp = 11025;
	else
		tmp = s_khz->intValue;
	desired.freq = val = tmp;
	desired.format = (sndbits->intValue == 16)? AUDIO_S16SYS : AUDIO_U8;
	desired.channels = sndchannels->intValue;
	if (desired.freq <= 11025)
		desired.samples = 256;
	else if (desired.freq <= 22050)
		desired.samples = 512;
	else if (desired.freq <= 44100)
		desired.samples = 1024;
	else if (desired.freq <= 56000)
		desired.samples = 2048; /* for 48 kHz */
	else
		desired.samples = 4096; /* for 96 kHz */
	desired.callback = paint_audio;
	desired.userdata = NULL;

	/* Open the audio device */
	if (SDL_OpenAudio(&desired, &obtained) == -1)
	{
		Com_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	/* Make sure we can support the audio format */
	switch (obtained.format)
	{
	case AUDIO_U8:
	case AUDIO_S16SYS:
	/* Supported */
		break;
	default:
		Com_Printf ("Unsupported audio format received (%u)\n", obtained.format);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	memset (&dma, 0, sizeof(dma_t));

	/* Fill the audio DMA information block */
	dma.samplebits = (obtained.format & 0xFF); /* first byte of format is bits */
	if (obtained.freq != val)
		Com_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", obtained.freq, val);
	dma.speed = obtained.freq;
	dma.channels = obtained.channels;
	tmp = (obtained.samples * obtained.channels) * 10;
	if (tmp & (tmp - 1)) {
	/* make it a power of two */
		val = 1;
		while (val < tmp)
			val <<= 1;
		tmp = val;
	}
	dma.samples = tmp;
	dma.samplepos = 0;
	dma.submission_chunk = 1;

	Com_Printf ("SDL audio spec  : %d Hz, %d samples, %d channels\n",
			obtained.freq, obtained.samples, obtained.channels);
	if (SDL_AudioDriverName(drivername, sizeof(drivername)) == NULL)
		strcpy(drivername, "(UNKNOWN)");
	buffersize = dma.samples * (dma.samplebits / 8);
	Com_Printf ("SDL audio driver: %s, %d bytes buffer\n", drivername, buffersize);

	dma.buffer = (unsigned char *) calloc (1, buffersize);
	if (!dma.buffer)
	{
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		memset (&dma, 0, sizeof(dma_t));
		Com_Printf ("Failed allocating memory for SDL audio\n");
		return false;
	}

	SDL_PauseAudio(0);

	return true;
}

int SNDDMA_GetDMAPos (void)
{
	return dma.samplepos;
}

void SNDDMA_Shutdown (void)
{
	if (dma.buffer)
	{
		Com_Printf ("Shutting down SDL sound\n");
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		free (dma.buffer);
		memset (&dma, 0, sizeof(dma_t));
	}
}

void SNDDMA_BeginPainting (void)
{
	SDL_LockAudio ();
}

void SNDDMA_Submit (void)
{
	SDL_UnlockAudio();
}
