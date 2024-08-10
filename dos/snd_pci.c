/* snd_pci.c -- PCI sound card support through libau */

#ifdef USE_SNDPCI

#include <dpmi.h>
#include <dos.h>
#include "../client/client.h"
#include "../client/snd_loc.h"
#include "libau.h"

static au_context *ctx = NULL;

#ifndef SNDPCI_DXE
#define AU_search_fp AU_search
#define AU_start_fp AU_start
#define AU_close_fp AU_close
#define AU_getinfo_fp AU_getinfo
#define AU_setrate_fp AU_setrate
#define AU_setmixer_all_fp AU_setmixer_all
#define AU_cardbuf_space_fp AU_cardbuf_space
#else
static void *sndpci_dxe;
static au_context * (*AU_search_fp)(unsigned int);
static void (*AU_start_fp)(au_context *);
static void (*AU_close_fp)(au_context *);
static const struct auinfo_s * (*AU_getinfo_fp)(au_context *);
static void (*AU_setrate_fp)(au_context *, unsigned int *, unsigned int *, unsigned int *);
static void (*AU_setmixer_all_fp)(au_context *, unsigned int);
static unsigned int (*AU_cardbuf_space_fp)(au_context *);

static int load_sndpci_dxe(void)
{
	if ((sndpci_dxe = Sys_dlopen("sndpci.dxe", 0)) == NULL)
		goto fail;
	AU_search_fp = (au_context * (*)(unsigned int)) Sys_dlsym(sndpci_dxe, "_AU_search");
	AU_start_fp = (void (*)(au_context*)) Sys_dlsym(sndpci_dxe, "_AU_start");
	AU_close_fp = (void (*)(au_context*)) Sys_dlsym(sndpci_dxe, "_AU_close");
	AU_getinfo_fp = (const struct auinfo_s * (*)(au_context*)) Sys_dlsym(sndpci_dxe, "_AU_getinfo");
	AU_setrate_fp = (void (*)(au_context*, unsigned int*, unsigned int*, unsigned int*)) Sys_dlsym(sndpci_dxe, "_AU_setrate");
	AU_setmixer_all_fp = (void (*)(au_context*, unsigned int)) Sys_dlsym(sndpci_dxe, "_AU_setmixer_all");
	AU_cardbuf_space_fp = (unsigned int (*)(au_context*)) Sys_dlsym(sndpci_dxe, "_AU_cardbuf_space");
	if (!AU_search_fp || !AU_start_fp || !AU_close_fp || !AU_getinfo_fp ||
	    !AU_setrate_fp || !AU_setmixer_all_fp || !AU_cardbuf_space_fp) {
	fail:	Com_Printf("PCI Audio: failed loading sndpci.dxe\n");
		return -1;
	}
	return 0;
}

static void close_sndpci_dxe(void)
{
	if (sndpci_dxe) Sys_dlclose(sndpci_dxe);
	sndpci_dxe = NULL;
	AU_search_fp = NULL;
	AU_start_fp = NULL;
	AU_close_fp = NULL;
	AU_getinfo_fp = NULL;
	AU_setrate_fp = NULL;
	AU_setmixer_all_fp = NULL;
	AU_cardbuf_space_fp = NULL;
}
#endif

/* detect for SBEMU: returns INT 2D multiplex or -1 if not found */
/* inspired from code found in https://github.com/wbcbz7/sndlib-watcom */
static int sbemu_detect(void)
{
	__dpmi_raddr addr;
	__dpmi_regs  regs;
	uint32_t r_addr;
	char appstring[16];
	int mx;

	/* check for INT2D vector == NULL */
	__dpmi_get_real_mode_interrupt_vector(0x2D, &addr);
	if (!addr.segment || !addr.offset16) return -1;

	/* scan all multiplexes of INT 2D */
	for (mx = 0; mx < 256; mx++)
	{
		memset(&regs, 0, sizeof(regs));
		regs.h.ah = mx;
		__dpmi_int(0x2D, &regs);
		if (regs.h.al != 0xFF)
			continue;

		/* check for SBEMU application string */
		r_addr = ((uint32_t)regs.x.dx<<4) + (uint32_t)regs.x.di;
		if (!r_addr) continue;
		dosmemget(r_addr, 16, appstring);
		if (memcmp(appstring + 8, "SBEMU", 5) == 0)
			return mx;
	}

	return -1;
}

/*
==================
PCI_Init

Returns false if nothing is found.
==================
*/
qboolean PCI_Init(void)
{
	const struct auinfo_s *aui;
	unsigned int speed;
	unsigned int samplebits;
	unsigned int channels;
	int sbemu_multiplex;

	if (!COM_CheckParm("-sndpci"))
		return false;

	sbemu_multiplex = sbemu_detect();
	if (sbemu_multiplex >= 0)
	{
		Com_Printf("PCI Audio: refusing to work with SBEMU present (multiplex=%d)\n", sbemu_multiplex);
		return false;
	}

#ifdef SNDPCI_DXE
	if (load_sndpci_dxe() < 0)
		return false;
#endif

	ctx = AU_search_fp(1);/* 1: stereo speaker output (meaningful only for Intel HDA chips) */
	if (!ctx)
	{
		Com_Printf("PCI Audio: Detection failed.\n");
		return false;
	}

	speed = 22050;
	samplebits = 16;
	channels = 2;
	if (s_khz->intValue >= 11025) /* FS */
		speed = s_khz->intValue;
	AU_setrate_fp(ctx, &speed, &samplebits, &channels);

	aui = AU_getinfo_fp(ctx);

	dma.speed = aui->freq_card;
	dma.samplebits = aui->bits_set;
	dma.channels = aui->chan_set;
	if (dma.speed != s_khz->intValue) /* FS: our rate was not liked, so force the change. */
		Cvar_SetValue("s_khz", dma.speed);
	dma.samples = aui->card_dmasize / aui->bytespersample_card;
	dma.samplepos = 0;
	dma.submission_chunk = 1;
	dma.buffer = (unsigned char *) aui->card_DMABUFF;

	Com_Printf("%s\n", aui->infostr);

	AU_setmixer_all_fp(ctx, 80); /* 80% volume */
	AU_start_fp(ctx); /* also clears dma buffer */

	return true;
}

/*
==============
PCI_GetDMAPos

return the current sample position (in mono samples read)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up.
===============
*/
int PCI_GetDMAPos(void)
{
	dma.samplepos = AU_cardbuf_space_fp(ctx);
	return dma.samplepos;
}

/*
==============
PCI_Shutdown

Stop and close the sound device for exiting
===============
*/
void PCI_Shutdown(void)
{
	AU_close_fp(ctx);
	ctx = NULL;
#ifdef SNDPCI_DXE
	close_sndpci_dxe();
#endif
}

#endif /* USE_SNDPCI */

