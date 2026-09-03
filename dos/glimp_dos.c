/*
** GLIMP_DOS.C
**
** This file contains ALL DOS specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
**
*/

#include "../ref_gl/gl_local.h"
#include "../client/keys.h"
#include "vid_dos.h"
#include "glimp_dos.h"

/*****************************************************************************/

void *opengl_dxe = NULL;

/* DOSGL interface */
int  (*DOSGL_InitCtx ) (int *width, int *height, int *bpp);
void (*DOSGL_Shutdown) (void);
void (*DOSGL_EndFrame) (void);
void * (*DOSGL_GetProcAddress) (const char *);
const char * (*DOSGL_APIName) (void);

// Gamma stuff
#define	USE_GAMMA_RAMPS			0

/* 3dfx gamma hacks: stuff are in fx_gamma.c
 * Note: gamma ramps crashes voodoo graphics */
#define	USE_3DFX_RAMPS			0
#if defined(USE_3DFXGAMMA)
#include "fx_gamma.h"
#endif

#if (USE_GAMMA_RAMPS) || (defined(USE_3DFXGAMMA) && (USE_3DFX_RAMPS))
static unsigned short	orig_ramps[3][256];
#endif

static qboolean	fx_gamma   = false;	// 3dfx-specific gamma control

#if !defined(USE_3DFXGAMMA)
static inline int Init_3dfxGammaCtrl (void)		{ return 0; }
static inline void Shutdown_3dfxGamma (void)		{ }
static inline int do3dfxGammaCtrl (float value)			{ return 0; }
static inline int glGetDeviceGammaRamp3DFX (void *arrays)	{ return 0; }
static inline int glSetDeviceGammaRamp3DFX (void *arrays)	{ return 0; }
static inline qboolean VID_Check3dfxGamma (void)	{ return false; }
#else
static qboolean VID_Check3dfxGamma (void)
{
	int		ret;

#if USE_3DFX_RAMPS /* not recommended for Voodoo1, currently crashes */
	ret = glGetDeviceGammaRamp3DFX(orig_ramps);
	if (ret != 0)
	{
		ri.Con_Printf(PRINT_ALL, "Using 3dfx glide3 specific gamma ramps\n");
		return true;
	}
#else
	ret = Init_3dfxGammaCtrl();
	if (ret > 0)
	{
		ri.Con_Printf(PRINT_ALL, "Using 3dfx glide%d gamma controls\n", ret);
		return true;
	}
#endif
	return false;
}
#endif	/* USE_3DFXGAMMA */

static void VID_InitGamma (void)
{
	const char *gl_renderer;

	gl_state.gammaRamp = fx_gamma = false;
	if (r_ignorehwgamma->value) {
		ri.Con_Printf(PRINT_ALL, "ignoring hardware gamma\n");
		return;
	}

	/* we don't have WGL_3DFX_gamma_control or an equivalent in dos. */
	/* Here is an evil hack abusing the exposed Glide symbols: */
	gl_renderer = (const char *)qglGetString (GL_RENDERER);
	if (!strnicmp(gl_renderer, "3dfx", 4)	  ||
	    !strnicmp(gl_renderer, "SAGE Glide", 10) ||
	    !strnicmp(gl_renderer, "Glide ", 6)	  || /* possible with Mesa 3.x/4.x/5.0.x */
	    !strnicmp(gl_renderer, "Mesa Glide", 10))
	{
		fx_gamma = VID_Check3dfxGamma();
		gl_state.gammaRamp = fx_gamma;
	}

	if (!gl_state.gammaRamp)
		ri.Con_Printf(PRINT_ALL, "gamma adjustment not available\n");
	else	vid_gamma->modified = true;/* let refresh loop update gamma */
}

static void VID_ShutdownGamma (void)
{
#if USE_3DFX_RAMPS
	if (fx_gamma) glSetDeviceGammaRamp3DFX(orig_ramps);
#else
/*	if (fx_gamma) do3dfxGammaCtrl(1);*/
#endif
	Shutdown_3dfxGamma();
	gl_state.gammaRamp = fx_gamma = false;
}

/*****************************************************************************/

extern cvar_t *vid_fullscreen;
extern cvar_t *vid_ref;

static vmodeinfo_t resolutions[] = {
	{ 240,320,  "[320x240]" },
	{ 300,400,  "[400x300]" },
	{ 384,512,  "[512x384]" },
	{ 480,640,  "[640x480]" },
	{ 600,800,  "[800x600]" },
	{ 480,856,  "[856x480]" },
	{ 720,960,  "[960x720]" },
	{ 768,1024,  "[1024x768]" },
	{ 1024,1280,  "[1280x1024]" },
	{ 1200,1600,  "[1600x1200]" }
};
static const int NUM_GL_RESOLUTIONS = (int) (sizeof(resolutions) / sizeof(resolutions[0]));

/*
** GLimp_SetMode
*/
rserr_t GLimp_SetMode( int *pwidth, int *pheight, int mode, rdisptype_t fullscreen )
{
	int width, height, bpp;
	int stencil = 0;

	ri.Con_Printf( PRINT_ALL, "Initializing OpenGL display\n");

	ri.Con_Printf (PRINT_ALL, "...setting mode %d:", mode );

	if ( !ri.Vid_GetModeInfo( &width, &height, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	bpp = (ri.Cvar_Get("vid_glbpp", "16", 0))->intValue;
	ri.Con_Printf( PRINT_ALL, " %dx%dx%d\n", width, height, bpp );

	// destroy the existing window
	GLimp_Shutdown ();

	if (DOSGL_InitCtx(&width, &height, &bpp) < 0)
		return rserr_invalid_mode;

	VID_InitGamma();

	qglGetIntegerv(GL_STENCIL_BITS, &stencil);
	gl_config.have_stencil = !!stencil;
	if (gl_config.have_stencil)
		ri.Con_Printf(PRINT_ALL, "... Using %d bit stencil buffer\n", stencil);
	else	ri.Con_Printf(PRINT_ALL, "... Stencil buffer not found\n");

	*pwidth = width;
	*pheight = height;

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (width, height);

	return rserr_ok;
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GLimp_Shutdown( void )
{
	VID_ShutdownGamma();
	if (DOSGL_Shutdown)
		DOSGL_Shutdown();
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.
*/
qboolean GLimp_Init(void *nummodes, void *modeinfos)
{
	vmodeinfo_t *vi;
	int	i;

	/* HACK HACK HACK: sending the video mode infos to vid_dos.c
	 * by exploiting our params. See: vid_dos.c:VID_LoadRefresh() */
	vi = (vmodeinfo_t *) malloc(sizeof(resolutions) * sizeof(vmodeinfo_t));
	for (i = 0; i < NUM_GL_RESOLUTIONS; ++i)
	{
		vi[i].height = resolutions[i].height;
		vi[i].width = resolutions[i].width;
		strcpy(vi[i].menuname, resolutions[i].menuname);
	}
	*(int *) nummodes = NUM_GL_RESOLUTIONS;
	*(vmodeinfo_t **) modeinfos = vi;

	return true;
}

/*
** GLimp_BeginFrame
*/
void GLimp_BeginFrame( float camera_seperation )
{
}

/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
	DOSGL_EndFrame ();
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate( qboolean active )
{
}

#define GAMMA_MAX	3.0
void UpdateGammaRamp (void)
{
#if (!USE_GAMMA_RAMPS) || (!USE_3DFX_RAMPS)
	float	value = (vid_gamma->value > (1.0 / GAMMA_MAX)) ?
			(1.0 / vid_gamma->value) : GAMMA_MAX;
#endif
#if USE_3DFX_RAMPS
	if (fx_gamma) glSetDeviceGammaRamp3DFX(ramps);
#else
	if (fx_gamma) do3dfxGammaCtrl(value);
#endif
}
