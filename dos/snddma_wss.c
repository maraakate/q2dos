/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/* DOS sound module using RayeR's WSS library (http://rayer.g6.cz) */

#include <dpmi.h>
#include "../client/client.h"
#include "../client/snd_loc.h"
#include "wss.h"

static  short *dma_buffer = NULL;

typedef enum
{
	dma_none,
} dmacard_t;

static dmacard_t		 dmacard;


void GUS_ClearDMA(void)
{
}

qboolean SNDDMA_Init(void)
{
	w_set_device_master_volume(0);
	if(w_sound_device_init(13, 11025) == FALSE) {
		Com_Printf("WSS error:%s\n", w_get_error_message());
		return false;
	}

	dmacard = dma_none;
	dma.speed = 11025;
	s_khz->value=11025;
	dma.channels = 2;
	/* FIXME: how to retrieve the format from the library? */
	dma.samplebits = 8;
	Cvar_SetValue( "s_loadas8bit", true );
	Com_Printf("wss is using [%s]\n",w_get_device_name());
	Com_Printf("wss rate is %d\n",w_get_actual_sample_rate());
	/*Com_Printf("wss format is %d\n",w_get_nominal_sample_format());*/

	dma_buffer = malloc(SND_BUFFER_SIZE);
	//dma.size = 1024*64;
	dma.samples = SND_BUFFER_SIZE/(dma.samplebits/8);
	dma.samplepos = 0;
	dma.submission_chunk = 1;
	dma.buffer = (unsigned char *) dma_buffer;

	Com_Printf("%5d stereo\n", dma.channels - 1);
	Com_Printf("%5d samples\n", dma.samples);
	Com_Printf("%5d samplepos\n", dma.samplepos);
	Com_Printf("%5d samplebits\n", dma.samplebits);
	Com_Printf("%5d submission_chunk\n", dma.submission_chunk);
	Com_Printf("%5d speed\n", dma.speed);
	Com_Printf("%p sound buffer\n", dma.buffer);
	return true;
}

int	SNDDMA_GetDMAPos(void)
{
	w_lock_mixing_buffer(0);
	if(dma.samplebits==16)
		w_mixing_stereo((short *)dma.buffer, SND_BUFFER_SIZE, 255, 255);
	else	w_mixing8((char *)dma.buffer, SND_BUFFER_SIZE/2, 255, 255);
//	Com_Printf("dma.samples %d %d\n",dma.samplepos,dma.samplepos/11025*16*2);
	w_unlock_mixing_buffer();
	__dpmi_yield();
//	pretend we are done?
	dma.samplepos = 0;
	dma.submission_chunk = 1;
	return 0;
}

void SNDDMA_Shutdown(void)
{
	Com_Printf("SNDDMA_Shutdown\n");
	w_sound_device_exit();
}

void SNDDMA_BeginPainting (void)
{
}

void SNDDMA_Submit(void)
{
}

void S_Activate (qboolean active)
{
}
