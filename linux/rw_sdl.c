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

/*
** RW_SDL.C
**
** This file contains ALL Linux specific stuff having to do with the
** software refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** SWimp_EndFrame
** SWimp_Init
** SWimp_InitGraphics
** SWimp_SetPalette
** SWimp_Shutdown
** SWimp_SwitchFullscreen
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

#include "r_local.h"
#include "rw_linux.h"

#include "keys.h"

/*****************************************************************************/

static qboolean rw_active = false;
static const SDL_VideoInfo *vinfo;
static SDL_Surface *surface;
static int sdl_palettemode = SDL_LOGPAL;

/*****************************************************************************/
/* MOUSE & KEYBOARD                                                          */

#include "rw_input.h"
#include "in_sdl.h"

/*****************************************************************************/

#include "icon_sdl.h"


/*
** SWimp_Init
**
** This routine is responsible for initializing the implementation
** specific stuff in a software rendering subsystem.
*/
qboolean SWimp_Init(void *hInstance, void *wndProc)
{
	if (SDL_Init(0) < 0) {
		Sys_Error("SDL Init failed: %s\n", SDL_GetError());
		return false;
	}
	if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
			Sys_Error("SDL Init failed: %s\n", SDL_GetError());
			return false;
		}
	}
	vinfo = SDL_GetVideoInfo();
	/* Okay, I am going to query SDL for the "best" pixel format.
	 * If the depth is not 8, use SetPalette with logical pal,
	 * else use SetColors.
	 * Hopefully this works all the time.
	 */
	sdl_palettemode = (vinfo->vfmt->BitsPerPixel == 8)? (SDL_PHYSPAL|SDL_LOGPAL) : SDL_LOGPAL;
	return true;
}

/*
** SWimp_InitGraphics
**
** This initializes the software refresh's implementation specific
** graphics subsystem.  In the case of Windows it creates DIB or
** DDRAW surfaces.
**
** The necessary width and height parameters are grabbed from
** vid.width and vid.height.
*/
static qboolean SWimp_InitGraphics(qboolean fullscreen)
{
	int flags;

	/* Just toggle fullscreen if that's all that has been changed */
	if (surface && (surface->w == vid.width) && (surface->h == vid.height)) {
		int isfullscreen = (surface->flags & SDL_FULLSCREEN) ? 1 : 0;
		if (fullscreen != isfullscreen)
			SDL_WM_ToggleFullScreen(surface);

		isfullscreen = (surface->flags & SDL_FULLSCREEN) ? 1 : 0;
		if (fullscreen == isfullscreen)
			return true;
	}

	srandom(getpid());

	// free resources in use
	if (surface)
		SDL_FreeSurface(surface);

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (vid.width, vid.height);

	flags = SDL_SWSURFACE|SDL_HWPALETTE;
	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	SetSDLIcon(); /* currently uses q2icon.xbm data */

	if ((surface = SDL_SetVideoMode(vid.width, vid.height, 8, flags)) == NULL) {
		Sys_Error("(SOFTSDL) SDL SetVideoMode failed: %s\n", SDL_GetError());
		return false;
	}

	SDL_WM_SetCaption("Quake II", "Quake II");

	SDL_ShowCursor(0);

	vid.rowbytes = surface->pitch;
	vid.buffer = (pixel_t *) surface->pixels;

	rw_active = true;

	return true;
}

/*
** SWimp_EndFrame
**
** This does an implementation specific copy from the backbuffer to the
** front buffer.  In the Win32 case it uses BitBlt or BltFast depending
** on whether we're using DIB sections/GDI or DDRAW.
*/
void SWimp_EndFrame (void)
{
	/* SDL_Flip(surface); */
	SDL_UpdateRect(surface, 0, 0, 0, 0);
}

/*
** SWimp_SetMode
*/
rserr_t SWimp_SetMode(int *pwidth, int *pheight, int mode, qboolean fullscreen)
{
	ri.Con_Printf (PRINT_ALL, "setting mode %d:", mode);

	if (!ri.Vid_GetModeInfo(pwidth, pheight, mode))
	{
		ri.Con_Printf(PRINT_ALL, " invalid mode\n");
		return rserr_invalid_mode;
	}

	ri.Con_Printf(PRINT_ALL, " %d %d\n", *pwidth, *pheight);

	if (!SWimp_InitGraphics(fullscreen)) {
		// failed to set the mode
		return rserr_invalid_mode;
	}

	R_GammaCorrectAndSetPalette((const unsigned char *) d_8to24table);

	return rserr_ok;
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
	SDL_Color colors[256];
	int i;

	if (!rw_active)
		return;

	if (!palette)
		palette = (const unsigned char *) sw_state.currentpalette;

	for (i = 0; i < 256; i++) {
		colors[i].r = palette[i*4+0];
		colors[i].g = palette[i*4+1];
		colors[i].b = palette[i*4+2];
	}

	SDL_SetPalette(surface, sdl_palettemode, colors, 0, 256);
}

/*
** SWimp_Shutdown
**
** System specific graphics subsystem shutdown routine.  Destroys
** DIBs or DDRAW surfaces as appropriate.
*/
void SWimp_Shutdown(void)
{
	if (surface)
		SDL_FreeSurface(surface);
	surface = NULL;

	if (SDL_WasInit(/*SDL_INIT_EVERYTHING*/SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_CDROM|SDL_INIT_JOYSTICK) == SDL_INIT_VIDEO)
		SDL_Quit();
	else
		SDL_QuitSubSystem(SDL_INIT_VIDEO);

	rw_active = false;
}

/*
** SWimp_AppActivate
*/
void SWimp_AppActivate(qboolean active)
{
}

//===============================================================================

/*
================
Sys_MakeCodeWriteable
================
*/
#if id386 && defined(__i386__)
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
	int		r;
	unsigned long	endaddr = startaddr + length;
/* systems with mprotect but not getpagesize (or similar) probably
 * don't need to page align the arguments to mprotect (eg, QNX)  */
#if !(defined(__QNX__) || defined(__QNXNTO__))
/*	int		psize = getpagesize ();*/
	long		psize = sysconf (_SC_PAGESIZE);
	startaddr &= ~(psize - 1);
	endaddr = (endaddr + psize - 1) & ~(psize - 1);
#endif
	r = mprotect ((char *) startaddr, endaddr - startaddr, PROT_WRITE | PROT_READ | PROT_EXEC);
	if (r == -1)
		Sys_Error("Protection change failed\n");
}
#endif
