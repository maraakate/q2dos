/*
Copyright (C) 1997-2001 Id Software, Inc.

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

// extended video modes: VESA-specific DOS video stuff

#include <stdio.h>
#include <dpmi.h>

#include "../ref_soft/r_local.h"
#include "vid_dos.h"
#include "swimp_dos.h"
#include "dosisms.h"

static int	totalvidmem;

vgamode_t vga_modes[MAX_VIDEOMODES];
int vga_nummodes = 0;

#define MODE_SUPPORTED_IN_HW		0x0001
#define COLOR_MODE			0x0008
#define GRAPHICS_MODE			0x0010
#define VGA_INCOMPATIBLE		0x0020
#define LINEAR_FRAME_BUFFER		0x0080
#define LINEAR_MODE			0x4000

int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes;
byte	*VGA_pagebase;

typedef struct
{
	int	modenum;
	int	mode_attributes;
	int	winasegment;
	int	winbsegment;
	int	bytes_per_scanline; // bytes per logical scanline (+16)
	int	win;		// window number (A=0, B=1)
	int	win_size;	// window size (+6)
	int	granularity;	// how finely i can set the window in vid mem (+4)
	int	width, height;	// displayed width and height (+18, +20)
	int	bits_per_pixel;	// er, better be 8, 15, 16, 24, or 32 (+25)
	int	bytes_per_pixel; // er, better be 1, 2, or 4
	int	memory_model;	// and better be 4 or 6, packed or direct color (+27)
	int	num_pages;	// number of complete frame buffer pages (+29)
	int	red_width;	// the # of bits in the red component (+31)
	int	red_pos;	// the bit position of the red component (+32)
	int	green_width;	// etc.. (+33)
	int	green_pos;	// (+34)
	int	blue_width;	// (+35)
	int	blue_pos;	// (+36)
	int	pptr;
	int	pagesize;
	int	numpages;
} modeinfo_t;

static modeinfo_t modeinfo;

// all bytes to avoid problems with compiler field packing
typedef struct vbeinfoblock_s {
	byte	VbeSignature[4];
	byte	VbeVersion[2];
	byte	OemStringPtr[4];
	byte	Capabilities[4];
	byte	VideoModePtr[4];
	byte	TotalMemory[2];
	byte	OemSoftwareRev[2];
	byte	OemVendorNamePtr[4];
	byte	OemProductNamePtr[4];
	byte	OemProductRevPtr[4];
	byte	Reserved[222];
	byte	OemData[256];
} vbeinfoblock_t;

static qboolean VID_ExtraGetModeInfo(int modenum);
/* FS: New stuff for planar/banked modes */
static void VID_AddPlanarModes(void);
static void VID_AddBankedModes(void);

/*
================
VID_ExtraFarToLinear
================
*/
static void *VID_ExtraFarToLinear (unsigned long addr)
{
	return real2ptr(((addr & 0xFFFF0000) >> 12) + (addr & 0xFFFF));
}

/*
================
VID_InitExtra
================
*/
void VID_InitExtra (void)
{
	short		*pmodenums;
	vbeinfoblock_t	*pinfoblock;
	unsigned long	addr;
	__dpmi_regs	r;
	cvar_t		*var;

	memset(vga_modes,0x0,sizeof(vga_modes));
	vga_nummodes = 0;

	/* mode 13 VGA first */
	vga_modes[vga_nummodes].mode=vga_nummodes;
	vga_modes[vga_nummodes].vesa_mode=-1;
	vga_modes[vga_nummodes].height=200;
	vga_modes[vga_nummodes].width=320;
	vga_modes[vga_nummodes].type = VGA_MODE13;
	Com_sprintf(vga_modes[vga_nummodes].menuname, sizeof(vga_modes[vga_nummodes].menuname), "[VGA 320x200]");
	vga_nummodes++;

	VID_AddPlanarModes();

	var = ri.Cvar_Get("vid_vgaonly", "0", 0);
	if(var->intValue)
		return;

	pinfoblock = (vbeinfoblock_t *) dos_getmemory(sizeof(vbeinfoblock_t));
	if (!pinfoblock) {
		ri.Con_Printf(PRINT_ALL, "VID_InitExtra: Unable to allocate low memory.\n");
		return;
	}

	pinfoblock->VbeSignature[0] = 'V';
	pinfoblock->VbeSignature[1] = 'B';
	pinfoblock->VbeSignature[2] = 'E';
	pinfoblock->VbeSignature[3] = '2';

// see if VESA support is available
	r.x.ax = 0x4f00;
	r.x.es = ptr2real(pinfoblock) >> 4;
	r.x.di = ptr2real(pinfoblock) & 0xf;
	__dpmi_int(0x10, &r);

	if (r.x.ax != 0x4f)
	{
		dos_freememory(pinfoblock);
		return;		// no VESA support
	}

	if (pinfoblock->VbeVersion[1] < 0x02)
	{
		dos_freememory(pinfoblock);
		return;		// not VESA 2.0 or greater
	}

	addr = ( (pinfoblock->OemStringPtr[0]      ) |
		 (pinfoblock->OemStringPtr[1] <<  8) |
		 (pinfoblock->OemStringPtr[2] << 16) |
		 (pinfoblock->OemStringPtr[3] << 24));
	ri.Con_Printf(PRINT_ALL, "VESA 2.0 compliant adapter:\n%s\n",
			(char *) VID_ExtraFarToLinear(addr));

	totalvidmem  = ( (pinfoblock->TotalMemory[0]     ) |
			 (pinfoblock->TotalMemory[1] << 8) ) << 16;
//	ri.Con_Printf(PRINT_DEVELOPER, "%dk video memory\n", totalvidmem >> 10);

	addr = ( (pinfoblock->VideoModePtr[0]      ) |
		 (pinfoblock->VideoModePtr[1] <<  8) |
		 (pinfoblock->VideoModePtr[2] << 16) |
		 (pinfoblock->VideoModePtr[3] << 24));
	pmodenums = (short *) VID_ExtraFarToLinear(addr);

// find 8 bit modes until we either run out of space or run out of modes
	while ((*pmodenums != -1) && (vga_nummodes < MAX_VIDEOMODES))
	{
		VID_ExtraGetModeInfo (*pmodenums);
		pmodenums++;
	}

	dos_freememory(pinfoblock);

	/* FS: Add banked modes at the end of the video list. */
	var = ri.Cvar_Get("vid_bankedvga", "0", 0);
	if (! var->intValue)
		VID_AddBankedModes();
}

static void VID_AddPlanarModes(void)
{
	/* add only Mode-X 320x240 */
	vga_modes[vga_nummodes].mode=vga_nummodes;
	vga_modes[vga_nummodes].vesa_mode=-1;
	vga_modes[vga_nummodes].height=240;
	vga_modes[vga_nummodes].width=320;
	vga_modes[vga_nummodes].address = (void *)real2ptr(0xA0000);
	vga_modes[vga_nummodes].type = VGA_PLANAR;

	Com_sprintf(vga_modes[vga_nummodes].menuname, sizeof(vga_modes[vga_nummodes].menuname), "[VGA-X 320x240]");
	vga_nummodes++;
}

static void VID_AddBankedModes(void)
{
	if (vga_nummodes == MAX_VIDEOMODES)
		return;
	vga_modes[vga_nummodes].mode=vga_nummodes;
	vga_modes[vga_nummodes].vesa_mode=0x0101;
	vga_modes[vga_nummodes].height=480;
	vga_modes[vga_nummodes].width=640;
	vga_modes[vga_nummodes].type = VGA_BANKED;
	Com_sprintf(vga_modes[vga_nummodes].menuname, sizeof(vga_modes[vga_nummodes].menuname), "[VGA-B 640x480]");
	vga_nummodes++;

	if (vga_nummodes == MAX_VIDEOMODES)
		return;
	vga_modes[vga_nummodes].mode=vga_nummodes;
	vga_modes[vga_nummodes].vesa_mode=0x0103;
	vga_modes[vga_nummodes].height=600;
	vga_modes[vga_nummodes].width = 800;
	vga_modes[vga_nummodes].type = VGA_BANKED;
	Com_sprintf(vga_modes[vga_nummodes].menuname, sizeof(vga_modes[vga_nummodes].menuname), "[VGA-B 800x600]");
	vga_nummodes++;

	if (vga_nummodes == MAX_VIDEOMODES)
		return;
	vga_modes[vga_nummodes].mode=vga_nummodes;
	vga_modes[vga_nummodes].vesa_mode=0x0105;
	vga_modes[vga_nummodes].height=768;
	vga_modes[vga_nummodes].width = 1024;
	vga_modes[vga_nummodes].type = VGA_BANKED;
	Com_sprintf(vga_modes[vga_nummodes].menuname, sizeof(vga_modes[vga_nummodes].menuname), "[VGA-B 1024x768]");
	vga_nummodes++;
}

/*
================
VID_ExtraGetModeInfo
================
*/
static qboolean VID_ExtraGetModeInfo(int modenum)
{
	char	*infobuf;
	int		numimagepages;
	__dpmi_meminfo	phys_mem_info;
	__dpmi_regs	r;
	cvar_t		*var;

	infobuf = (char *) dos_getmemory(256);

	r.x.ax = 0x4f01;
	r.x.cx = modenum;
	r.x.es = ptr2real(infobuf) >> 4;
	r.x.di = ptr2real(infobuf) & 0xf;
	__dpmi_int(0x10, &r);

	if (r.x.ax != 0x4f)
	{
		dos_freememory(infobuf);
		return false;
	}
	else
	{
		modeinfo.modenum = modenum;
		modeinfo.bits_per_pixel = *(char*)(infobuf+25);
		modeinfo.bytes_per_pixel = (modeinfo.bits_per_pixel+1)/8;
		modeinfo.width = *(short*)(infobuf+18);
		modeinfo.height = *(short*)(infobuf+20);

	// we do only 8-bpp in software
		if ((modeinfo.bits_per_pixel != 8) ||
			(modeinfo.bytes_per_pixel != 1) ||
			(modeinfo.width > MAXWIDTH) ||
			(modeinfo.height > MAXHEIGHT))
		{
			dos_freememory(infobuf);
			return false;
		}

		modeinfo.mode_attributes = *(short*)infobuf;

	// we only want color graphics modes that are supported by the hardware
		if ((modeinfo.mode_attributes &
			 (MODE_SUPPORTED_IN_HW | COLOR_MODE | GRAPHICS_MODE)) !=
			(MODE_SUPPORTED_IN_HW | COLOR_MODE | GRAPHICS_MODE))
		{
			dos_freememory(infobuf);
			return false;
		}

	// we only work with linear frame buffers, except for 320x200, which can
	// effectively be linear when banked at 0xA000
		if (!(modeinfo.mode_attributes & LINEAR_FRAME_BUFFER))
		{
			if ((modeinfo.width != 320) || (modeinfo.height != 200))
			{
				dos_freememory(infobuf);
				return false;
			}
		}

		modeinfo.bytes_per_scanline = *(short*)(infobuf+16);

		modeinfo.pagesize = modeinfo.bytes_per_scanline * modeinfo.height;

		if (modeinfo.pagesize > totalvidmem)
		{
			dos_freememory(infobuf);
			return false;
		}

	// force to one page if the adapter reports it doesn't support more pages
	// than that, no matter how much memory it has--it may not have hardware
	// support for page flipping
		numimagepages = *(unsigned char *)(infobuf+29);

		if (numimagepages <= 0)
		{
		// wrong, but there seems to be an ATI VESA driver that reports 0
			modeinfo.numpages = 1;
		}
		else if (numimagepages < 3)
		{
			modeinfo.numpages = numimagepages;
		}
		else
		{
			modeinfo.numpages = 3;
		}

		if (*(char*)(infobuf+2) & 5)
		{
			modeinfo.winasegment = *(unsigned short*)(infobuf+8);
			modeinfo.win = 0;
		}
		else if (*(char*)(infobuf+3) & 5)
		{
			modeinfo.winbsegment = *(unsigned short*)(infobuf+8);
			modeinfo.win = 1;
		}
		modeinfo.granularity = *(short*)(infobuf+4) * 1024;
		modeinfo.win_size = *(short*)(infobuf+6) * 1024;
		modeinfo.bits_per_pixel = *(char*)(infobuf+25);
		modeinfo.bytes_per_pixel = (modeinfo.bits_per_pixel+1)/8;
		modeinfo.memory_model = *(unsigned char*)(infobuf+27);
		modeinfo.num_pages = *(char*)(infobuf+29) + 1;

		modeinfo.red_width = *(char*)(infobuf+31);
		modeinfo.red_pos = *(char*)(infobuf+32);
		modeinfo.green_width = *(char*)(infobuf+33);
		modeinfo.green_pos = *(char*)(infobuf+34);
		modeinfo.blue_width = *(char*)(infobuf+35);
		modeinfo.blue_pos = *(char*)(infobuf+36);

		modeinfo.pptr = *(long *)(infobuf+40);

		/* 8 bit linear only */
		if (modeinfo.memory_model == 0x4 && modeinfo.bits_per_pixel == 8)
		{
			ri.Con_Printf(PRINT_ALL, "VESA mode 0x%0x %dx%d supported\n",modeinfo.modenum,modeinfo.width,modeinfo.height);
			/* vga_nummodes already checked by our caller. */
			vga_modes[vga_nummodes].mode=vga_nummodes;
			vga_modes[vga_nummodes].vesa_mode=modeinfo.modenum;
			vga_modes[vga_nummodes].height=modeinfo.height;
			vga_modes[vga_nummodes].width=modeinfo.width;

			phys_mem_info.address = (int)modeinfo.pptr;
			phys_mem_info.size = 0x400000;

			if (__dpmi_physical_address_mapping(&phys_mem_info))
				return false;

			vga_modes[vga_nummodes].address = real2ptr (phys_mem_info.address);

			/* FS: FIXME, just add all modes for banked */
			var = ri.Cvar_Get("vid_bankedvga", "0", 0);
			vga_modes[vga_nummodes].type = (var->intValue)? VGA_BANKED : VGA_VESALFB;

			Com_sprintf(vga_modes[vga_nummodes].menuname, sizeof(vga_modes[vga_nummodes].menuname), "[VESA %dx%d]",modeinfo.width,modeinfo.height);
			vga_nummodes++;
		}
#ifdef VESA_DEBUG_OUTPUT
		printf("VID: (VESA) info for mode 0x%x\n", modeinfo.modenum);
		printf("  mode attrib = 0x%0x\n", modeinfo.mode_attributes);
		printf("  win a attrib = 0x%0x\n", *(unsigned char*)(infobuf+2));
		printf("  win b attrib = 0x%0x\n", *(unsigned char*)(infobuf+3));
		printf("  win a seg 0x%0x\n", (int) modeinfo.winasegment);
		printf("  win b seg 0x%0x\n", (int) modeinfo.winbsegment);
		printf("  bytes per scanline = %d\n", modeinfo.bytes_per_scanline);
		printf("  width = %d, height = %d\n", modeinfo.width, modeinfo.height);
		printf("  win = %c\n", 'A' + modeinfo.win);
		printf("  win granularity = %d\n", modeinfo.granularity);
		printf("  win size = %d\n", modeinfo.win_size);
		printf("  bits per pixel = %d\n", modeinfo.bits_per_pixel);
		printf("  bytes per pixel = %d\n", modeinfo.bytes_per_pixel);
		printf("  memory model = 0x%x\n", modeinfo.memory_model);
		printf("  num pages = %d\n", modeinfo.num_pages);
		printf("  red width = %d\n", modeinfo.red_width);
		printf("  red pos = %d\n", modeinfo.red_pos);
		printf("  green width = %d\n", modeinfo.green_width);
		printf("  green pos = %d\n", modeinfo.green_pos);
		printf("  blue width = %d\n", modeinfo.blue_width);
		printf("  blue pos = %d\n", modeinfo.blue_pos);
		printf("  phys mem = %x\n", modeinfo.pptr);
#endif
	}

	dos_freememory(infobuf);
	return true;
}
