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

#include <dpmi.h>
#include <sys/nearptr.h>
#include "../ref_soft/r_local.h"
#include "vid_dos.h"
#include "swimp_dos.h"
#include "dosisms.h"
/* FS: For planar modes */
#include "vregset.h"
#include "vgamodes.h"

static int currentmode = 0;

void VID_DrawBanked(void);

#if	!id386
void MaskExceptions (void) {
}
void Sys_SetFPCW (void) {
}

void VGA_UpdatePlanarScreen (void *srcbuffer)
{
/* based on vga_copytoplanar256() of SVGAlib */
	byte *p;
	int ofs;
	int plane, x, y;

	plane = 0;
	/* Copy pixels that belong in plane. */
	_docopy:
	outportb(SC_INDEX, MAP_MASK);
	outportb(SC_DATA, 1 << plane);
	p = (byte *) srcbuffer;
	ofs = 0;
	for (y = 0; y < VGA_height; y++)
	{
		x = 0;
		while (x * 4 + 32 < VGA_width)
		{
			VGA_pagebase[ofs + x + 0] = p[x * 4 + plane + 0];
			VGA_pagebase[ofs + x + 1] = p[x * 4 + plane + 4];
			VGA_pagebase[ofs + x + 2] = p[x * 4 + plane + 8];
			VGA_pagebase[ofs + x + 3] = p[x * 4 + plane + 12];
			VGA_pagebase[ofs + x + 4] = p[x * 4 + plane + 16];
			VGA_pagebase[ofs + x + 5] = p[x * 4 + plane + 20];
			VGA_pagebase[ofs + x + 6] = p[x * 4 + plane + 24];
			VGA_pagebase[ofs + x + 7] = p[x * 4 + plane + 28];
			x += 8;
		}
		while (x * 4 < VGA_width)
		{
			VGA_pagebase[ofs + x] = p[x * 4 + plane];
			x++;
		}
		p += VGA_bufferrowbytes;	/* Next line. */
		ofs += VGA_rowbytes;
	}
	if (++plane < 4)
		goto _docopy;
}

#if 0 /* not used */
void VGA_UpdateLinearScreen (	void *srcptr, void *destptr,
				int width, int height,
				int srcrowbytes, int destrowbytes)
{
/* quick'n'dirty C-only version */
	byte *s, *d;
	int y;

	s = (byte *) srcptr;
	d = (byte *) destptr;
	for (y = 0; y < height; y++)
	{
		memcpy (d, s, width);
		s += srcrowbytes;
		d += destrowbytes;
	}
}
#endif
#endif	/* !id386 */

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
/* MS-DOS is always writeable */
}

void SWimp_BeginFrame(float camera_separation)
{
}

void SWimp_EndFrame (void)
{
	switch (vga_modes[currentmode].type) {
	case VGA_PLANAR:
		VGA_UpdatePlanarScreen(vid.buffer);
		break;
	case VGA_MODE13: /* JASON */
		dosmemput(vid.buffer,vga_modes[currentmode].height*vga_modes[currentmode].width,0xA0000);
		break;
	case VGA_VESALFB:
		memcpy(vga_modes[currentmode].address, vid.buffer, (vid.height*vid.width));
		break;
	case VGA_BANKED:
		VID_DrawBanked();
		break;
	}
}

/* Windows style hook */
qboolean SWimp_Init(void *nummodes, void *modeinfos)
{
	vmodeinfo_t *vi;
	int i;

	currentmode = 0;

	VID_InitExtra(); /* probe VESA */
	if (vga_nummodes == 0)
		return false;

	/* HACK HACK HACK: sending the video mode infos to vid_dos.c
	 * by exploiting our params. See: vid_dos.c:VID_LoadRefresh() */
	vi = (vmodeinfo_t *) malloc(vga_nummodes * sizeof(vmodeinfo_t));
	for (i = 0; i < vga_nummodes; ++i)
	{
		vi[i].height = vga_modes[i].height;
		vi[i].width = vga_modes[i].width;
		strcpy(vi[i].menuname, vga_modes[i].menuname);
	}
	*(int *) nummodes = vga_nummodes;
	*(vmodeinfo_t **) modeinfos = vi;

	return true;
}

/*
** SWimp_SetPalette
**
** System specific palette setting routine.  A NULL palette means
** to use the existing palette.  The palette is expected to be in
** a padded 4-byte xRGB format.
*/
void SWimp_SetPalette(const unsigned char *palette)
{
	const int shiftcomponents = 2;
	int i;

	if (palette == NULL)
		return;

	outportb(0x3c8, 0);		/* JASON: this means we are going to set the pallette */

	for (i = 0; i < 1024; i++)	/* JASON: we do it this way to skip a byte since it's padded */
	{
		outportb(0x3c9, palette[i]>>shiftcomponents); i++;
		outportb(0x3c9, palette[i]>>shiftcomponents); i++;
		outportb(0x3c9, palette[i]>>shiftcomponents); i++;
	}
}

void SWimp_Shutdown(void)
{
	__dpmi_regs r;

	free(vid.buffer);
	vid.buffer = NULL;
	vid.rowbytes = 0;

	/* JASON: return to text mode */
	r.h.ah = 0;
	r.x.ax = 3;
	__dpmi_int(0x10, &r);
}

rserr_t SWimp_SetMode(int *pwidth, int *pheight, int mode, qboolean fullscreen)
{
	__dpmi_regs r;

	ri.Con_Printf(PRINT_ALL, "SWimp_SetMode %d\n",mode);

	if (!ri.Vid_GetModeInfo(pwidth, pheight, mode))
	{
		ri.Con_Printf(PRINT_ALL, " invalid mode\n");
		return rserr_invalid_mode;
	}

	ri.Con_Printf(PRINT_ALL, " %s %dx%d\n",vga_modes[mode].menuname, *pwidth, *pheight);

	currentmode = mode;

	vid.height = vga_modes[mode].height;
	vid.width = vga_modes[mode].width;
	vid.rowbytes = vid.width;

	free(vid.buffer);
	vid.buffer = (pixel_t *) malloc(vid.width*vid.height);

	/* FS: Make sure mode 13 is set again because if we go to
	 * high res mode and back it gets funky with banked modes.
	 * Also, Win9x wants this to happen before going to LFB. */
	r.h.ah = 0;
	r.x.ax = 0x13;
	__dpmi_int(0x10, &r);

	switch (vga_modes[mode].type) {
	case VGA_PLANAR:
		ri.Con_Printf(PRINT_DEVELOPER, "\x02Setting VGA-X mode!\n");

		if (VGA_pagebase)
		{
			/* enable all planes for writing */
			outportb (SC_INDEX, MAP_MASK);
			outportb (SC_DATA, 0x0F);
			memset (VGA_pagebase, 0, VGA_rowbytes * VGA_height);
		}

		VGA_pagebase = (byte *) vga_modes[mode].address;

		VideoRegisterSet(vrs320x240x256planar); /* FS: 320x240x8 only. */

		VGA_width = vid.width;
		VGA_height = vid.height;
		VGA_rowbytes = vga_modes[mode].width / 4;
		VGA_bufferrowbytes = vid.rowbytes;

		/* enable all planes for writing */
		outportb (SC_INDEX, MAP_MASK);
		outportb (SC_DATA, 0x0F);
		memset (VGA_pagebase, 0, VGA_rowbytes * VGA_height);
		break;

	case VGA_VESALFB:
		r.x.ax = 0x4F02;
		r.x.bx = vga_modes[mode].vesa_mode+0x4000; /* JASON: 0x4000 for Linear access */
		__dpmi_int(0x10, &r);
		if (r.h.ah)
			Sys_Error("Error setting VESA LFB mode 0x%0x",vga_modes[mode].vesa_mode);
		break;

	case VGA_BANKED:
		ri.Con_Printf(PRINT_ALL, "\x02Setting banked mode!\n");
		r.x.ax = 0x4F02;
		r.x.bx = vga_modes[mode].vesa_mode;
		__dpmi_int(0x10, &r);
		break;

	case VGA_MODE13:
	#if 0 /* FS: handled it above */
		r.x.ax = 0x13;
		__dpmi_int(0x10, &r);
	#endif
		break;

	default:
		Sys_Error("Unknown VGA access type %d",vga_modes[mode].type);
	}

	ri.Vid_NewWindow(vid.width,vid.height);

	return rserr_ok;
}

void SWimp_AppActivate(qboolean active)
{
}

void VID_DrawBanked(void) /* FS: Credit to ggorts */
{
	const unsigned char *buf = vid.buffer;
	int bank = 0;
	int h = vga_modes[currentmode].height;
	int w = vga_modes[currentmode].width;
	int todo = h * w;
	int copy;
	__dpmi_regs r;

	while (todo > 0)
	{
#if 0 /* FS: For page flipping? */
		r.x.ax = 0x4F07;
		r.x.bx = 0;
		r.x.cx = bank % w;
		r.x.dx = bank / w;
		__dpmi_int(0x10, &r);
#endif

		r.x.ax = 0x4F05;
		r.x.bx = 0;
		r.x.dx = bank;
		__dpmi_int(0x10, &r);

		copy = (todo > 65536)? 65536 : todo;
		dosmemput(buf, copy, 0xA0000);

		todo -= copy;
		buf += copy;
		bank++;
	}

	r.x.ax = 0x4f05;
	r.x.bx = 0;
	r.x.dx = 0;
	__dpmi_int(0x10, &r);
}
