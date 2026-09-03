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
// swimp_dos.h: header file for DOS-specific vga/vesa video stuff

enum {
	VGA_MODE13 = 0,
	VGA_PLANAR,
	VGA_BANKED,
	VGA_VESALFB
};

typedef struct vgamode_s {
	int mode;
	int type;
	int height;
	int width;
	int vesa_mode;
	void *address;
	char menuname[VIDNAME_LEN];
} vgamode_t;

#ifdef __cplusplus
extern "C" {
#endif
extern int vga_nummodes;
extern vgamode_t vga_modes[MAX_VIDEOMODES];

extern int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes;
extern byte		*VGA_pagebase;

void VID_InitExtra (void);
void VGA_UpdatePlanarScreen (void *srcbuffer);
void VGA_UpdateLinearScreen (void *srcptr, void *destptr, int width,
	int height, int srcrowbytes, int destrowbytes);
#ifdef __cplusplus
}
#endif
