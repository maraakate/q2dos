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
** GL_SDL.C
**
** This file contains ALL Linux specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

#include "gl_local.h"
#include "glw_linux.h"
#include "rw_linux.h"

#include "keys.h"

/*****************************************************************************/

static qboolean rw_active = false;
static const SDL_VideoInfo *vinfo;
static SDL_Surface *surface;

typedef struct {
  int red, green, blue, alpha, depth, stencil;
} attributes_t;

glwstate_t glw_state;

/*****************************************************************************/
/* MOUSE & KEYBOARD                                                          */

#include "rw_input.h"
#include "in_sdl.h"

/*****************************************************************************/
/* Gamma stuff */

#define GAMMA_MAX 3.0
#define USE_GAMMA_RAMPS 1 /* 0: use SetGamma & co. 1: use GammaRamp funcs. */

#if (USE_GAMMA_RAMPS)
static Uint16 org_gammaRamp[3][256];
static Uint16 cur_gammaRamp[3][256];

static void InitGammaRamp (void)
{
	if (r_ignorehwgamma->intValue)
		gl_state.gammaRamp = false;
	else {
		gl_state.gammaRamp =	 (SDL_GetGammaRamp(org_gammaRamp[0], org_gammaRamp[1], org_gammaRamp[2]) == 0);
		if (gl_state.gammaRamp)
		    gl_state.gammaRamp = (SDL_SetGammaRamp(org_gammaRamp[0], org_gammaRamp[1], org_gammaRamp[2]) == 0);
	}
	if (gl_state.gammaRamp)
		vid_gamma->modified = true;
}
static void ShutdownGammaRamp (void)
{
	if (gl_state.gammaRamp) SDL_SetGammaRamp(org_gammaRamp[0], org_gammaRamp[1], org_gammaRamp[2]);
}
static void HWGamma_Toggle (qboolean enable)
{
	if (gl_state.gammaRamp) {
		if (!enable)
			SDL_SetGammaRamp(org_gammaRamp[0], org_gammaRamp[1], org_gammaRamp[2]);
		else	SDL_SetGammaRamp(cur_gammaRamp[0], cur_gammaRamp[1], cur_gammaRamp[2]);
	}
}
void UpdateGammaRamp (void)
{
	int	i, j, p;
	float	g;

	if (!gl_state.gammaRamp)
		return;
	g = vid_gamma->value + 0.3f;/* 0.6f */
	memcpy (cur_gammaRamp, org_gammaRamp, sizeof(org_gammaRamp));
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 256; j++)
		{
			p = 255 * pow(((float)j + 0.5f) / 255.5f, g) + 0.5f;
			if (p < 0) p = 0;
			else if (p > 255)
				p = 255;
			cur_gammaRamp[i][j] = ((Uint16) p) << 8;
		}
	}
	SDL_SetGammaRamp(cur_gammaRamp[0], cur_gammaRamp[1], cur_gammaRamp[2]);
}

#else /* ! USE_GAMMA_RAMPS : */

static void InitGammaRamp (void)
{
	if (r_ignorehwgamma->intValue)
		gl_state.gammaRamp = false;
	else	gl_state.gammaRamp = (SDL_SetGamma(1, 1, 1) == 0);
	if (gl_state.gammaRamp)
		vid_gamma->modified = true;
}
static void ShutdownGammaRamp (void)
{
	if (gl_state.gammaRamp) SDL_SetGamma (1, 1, 1);
}
void UpdateGammaRamp (void)
{
	if (gl_state.gammaRamp) {
		float g = (vid_gamma->value > (1.0 / GAMMA_MAX))?
				(1.0 / vid_gamma->value) : GAMMA_MAX;
		SDL_SetGamma(g, g, g);
	}
}
static void HWGamma_Toggle (qboolean enable)
{
	if (!gl_state.gammaRamp) return;
	if (!enable)
		SDL_SetGamma (1, 1, 1);
	else {
		float g = (vid_gamma->value > (1.0 / GAMMA_MAX))?
				(1.0 / vid_gamma->value) : GAMMA_MAX;
		SDL_SetGamma(g, g, g);
	}
}
#endif /* ... GAMMA */

/*****************************************************************************/

#include "icon_sdl.h"

qboolean GLimp_Init(void *hInstance, void *wndProc)
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
	return true;
}

static qboolean GLimp_InitGraphics(int width, int height, int bpp, qboolean fullscreen)
{
	int flags, p;
	attributes_t a;

	/* Just toggle fullscreen if that's all that has been changed */
	if (surface && (surface->w == width) && (surface->h == height)) {
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
	ri.Vid_NewWindow (width, height);

	if (bpp < 24) {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	}
	else {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	flags = SDL_OPENGL;
	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	SetSDLIcon(); /* currently uses q2icon.xbm data */

	if ((surface = SDL_SetVideoMode(width, height, (bpp < 24)? 16: 32, flags)) == NULL) {
		Sys_Error("(SDLGL) SDL SetVideoMode failed: %s\n", SDL_GetError());
		return false;
	}

	SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &p);
	ri.Con_Printf(PRINT_ALL, "Video Mode Set : %dx%dx%d\n", width, height, p);
	memset (&a, 0, sizeof(attributes_t));
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &a.red);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &a.green);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &a.blue);
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &a.alpha);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &a.depth);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &a.stencil);
	ri.Con_Printf(PRINT_ALL, "R:%d G:%d B:%d A:%d, Z:%d, S:%d\n",
				a.red, a.green, a.blue, a.alpha, a.depth, a.stencil);

	SDL_WM_SetCaption("Quake II", "Quake II");

	SDL_ShowCursor(0);

	rw_active = true;

	return true;
}

void GLimp_BeginFrame(float camera_seperation)
{
}

void GLimp_EndFrame(void)
{
	SDL_GL_SwapBuffers();
}

rserr_t GLimp_SetMode(int *pwidth, int *pheight, int mode, rdisptype_t fullscreen)
{
	int bpp;

	ri.Con_Printf (PRINT_ALL, "setting mode %d:", mode);

	if (!ri.Vid_GetModeInfo(pwidth, pheight, mode))
	{
		ri.Con_Printf(PRINT_ALL, " invalid mode\n");
		return rserr_invalid_mode;
	}

	ri.Con_Printf(PRINT_ALL, " %d %d\n", *pwidth, *pheight);

	bpp = (vinfo)? vinfo->vfmt->BitsPerPixel : 16;
	if (!GLimp_InitGraphics(*pwidth, *pheight, bpp, fullscreen)) {
		// failed to set the mode
		return rserr_invalid_mode;
	}

	InitGammaRamp ();

	return rserr_ok;
}

void GLimp_Shutdown(void)
{
	ShutdownGammaRamp ();

	if (surface)
		SDL_FreeSurface(surface);
	surface = NULL;

	if (SDL_WasInit(/*SDL_INIT_EVERYTHING*/SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_CDROM|SDL_INIT_JOYSTICK) == SDL_INIT_VIDEO)
		SDL_Quit();
	else
		SDL_QuitSubSystem(SDL_INIT_VIDEO);

	rw_active = false;
}

void GLimp_AppActivate(qboolean active)
{
	HWGamma_Toggle (active);
}
