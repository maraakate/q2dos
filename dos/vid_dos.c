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

// vid_dos.c -- DOS video driver frontend

#include "../client/client.h"
#include "../client/qmenu.h"
#include "vid_dos.h"

#define NUM_VID_DRIVERS 2
#define REF_SOFT	0
#define REF_OPENGL	1

cvar_t *vid_ref;
static cvar_t *vid_fullscreen;
static cvar_t *vid_gamma;
extern cvar_t *scr_viewsize;	/* client/cl_scrn.c */

static cvar_t *sw_stipplealpha;
static cvar_t *sw_waterwarp; /* FS */
static cvar_t *r_contentblend; /* FS */

extern void M_ForceMenuOff (void);
extern const char *Default_MenuKey( menuframework_s *m, int key );
static void VID_ListModes_f (void);
static void VID_Restart_f (void);

#define SOFTWARE_MENU	0
#define OPENGL_MENU		1

static menuframework_s  s_software_menu;
static menuframework_s	s_opengl_menu;
static menuframework_s *s_current_menu;
static int				s_current_menu_index;

static menulist_s		s_ref_list[NUM_VID_DRIVERS];
static menulist_s		s_mode_list[NUM_VID_DRIVERS];
static menuslider_s		s_screensize_slider[NUM_VID_DRIVERS];
static menuslider_s		s_brightness_slider[NUM_VID_DRIVERS];
static menulist_s		s_stipple_box;
static menulist_s		s_contentblend_box;	/* FS */
static menulist_s		s_waterwarp_box;	/* FS */
static menuslider_s		s_tq_slider;
static menulist_s  		s_paletted_texture_box;
static menulist_s		s_texfilter_box;

static menuaction_s		s_apply_action[NUM_VID_DRIVERS];
static menuaction_s		s_defaults_action[NUM_VID_DRIVERS];

static vmodeinfo_t		*vid_modes;
static int			vid_nummodes;

static const char *		resolution_names[MAX_VIDEOMODES + 1];
				/* initialized by VID_LoadRefresh() */

viddef_t	viddef; /* global video state */

refexport_t	re;
#ifdef REF_HARD_LINKED
refexport_t GetRefAPI (refimport_t rimp);
#else
static void *reflib_library; /* Handle to refresh DLL. */
#endif
static qboolean reflib_active = false;


/*
==========================================================================

DIRECT LINK GLUE

==========================================================================
*/

#define	MAXPRINTMSG	4096
static void __attribute__((__format__(__printf__,2,3)))
VID_Printf (int print_level, const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	vsnprintf (msg,sizeof(msg),fmt,argptr);
	va_end (argptr);

	if (print_level == PRINT_ALL)
		Com_Printf ("%s", msg);
	else
		Com_DPrintf(DEVELOPER_MSG_GFX, "%s", msg);
}

static void __attribute__((__noreturn__, __format__(__printf__,2,3)))
VID_Error (int err_level, const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	vsnprintf (msg,sizeof(msg),fmt,argptr);
	va_end (argptr);

	Com_Error (err_level, "%s", msg);
}

static void VID_NewWindow (int width, int height)
{
	viddef.width = width;
	viddef.height = height;

	cl.force_refdef = true; /* can't use a paused refdef */
}

/*
** VID_GetModeInfo
*/
static qboolean VID_GetModeInfo(int *width, int *height, int mode)
{
	if (mode < 0 || mode >= vid_nummodes)
		return false;

	*width  = vid_modes[mode].width;
	*height = vid_modes[mode].height;

	return true;
}

static void RefreshCallback( void *unused )
{
	s_ref_list[!s_current_menu_index].curvalue = s_ref_list[s_current_menu_index].curvalue;

	if ( s_ref_list[s_current_menu_index].curvalue == 0 )
	{
		s_current_menu = &s_software_menu;
		s_current_menu_index = 0;
	}
	else
	{
		s_current_menu = &s_opengl_menu;
		s_current_menu_index = 1;
	}

}

static void ScreenSizeCallback( void *s )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	Cvar_SetValue( "viewsize", slider->curvalue * 10 );
}

static void BrightnessCallback( void *s )
{
	menuslider_s	*slider = ( menuslider_s * ) s;
	float			gamma;

	if ( s_current_menu_index == SOFTWARE_MENU )
		s_brightness_slider[1].curvalue = s_brightness_slider[0].curvalue;
	else
		s_brightness_slider[0].curvalue = s_brightness_slider[1].curvalue;

//	gamma = (0.8 - (slider->curvalue/10.0 - 0.5)) + 0.5;
	gamma = 1.3 - (slider->curvalue/20.0);
	Cvar_SetValue( "vid_gamma", gamma );
}

// Knightmare- callback for texture mode
static void TexFilterCallback( void *s )
{
	if (s_texfilter_box.curvalue == 1)
		Cvar_Set ("gl_texturemode", "GL_LINEAR_MIPMAP_LINEAR");
	else // (s_texfilter_box.curvalue == 0)
		Cvar_Set ("gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST");
}

static void ResetDefaults( void *unused )
{
	VID_MenuInit();
}

static void ApplyChanges( void *unused )
{
	float	gamma;
	int		temp;

	/*
	** make values consistent
	*/
	s_brightness_slider[!s_current_menu_index].curvalue = s_brightness_slider[s_current_menu_index].curvalue;
	s_ref_list[!s_current_menu_index].curvalue = s_ref_list[s_current_menu_index].curvalue;

	/*
	** invert sense so greater = brighter, and scale to a range of 0.5 to 1.3
	*/
//	gamma = (0.8 - (s_brightness_slider[s_current_menu_index].curvalue/10.0 - 0.5)) + 0.5;
	gamma = (1.3 - (s_brightness_slider[s_current_menu_index].curvalue/20.0));

	Cvar_SetValue("vid_gamma", gamma);
	Cvar_SetValue("sw_stipplealpha", s_stipple_box.curvalue);
	Cvar_SetValue("r_contentblend", s_contentblend_box.curvalue);	/* FS */
	Cvar_SetValue("sw_waterwarp", s_waterwarp_box.curvalue);	/* FS */
	Cvar_SetValue ("gl_picmip", 3 - s_tq_slider.curvalue);
	Cvar_SetValue ("gl_ext_palettedtexture", s_paletted_texture_box.curvalue);

	temp = s_mode_list[SOFTWARE_MENU].curvalue;
	Cvar_SetValue ("sw_mode", temp);
	temp = s_mode_list[OPENGL_MENU].curvalue;
	Cvar_SetValue ("gl_mode", temp);

	// Knightmare- texture filter mode
	if (s_texfilter_box.curvalue == 1)
		Cvar_Set ("gl_texturemode", "GL_LINEAR_MIPMAP_LINEAR");
	else // (s_texfilter_box.curvalue == 0)
		Cvar_Set ("gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST");


	switch ( s_ref_list[s_current_menu_index].curvalue )
	{
	case REF_SOFT:
		Cvar_Set( "vid_ref", "soft" );
		break;
	case REF_OPENGL:
		Cvar_Set( "vid_ref", "gl" );
		Cvar_Set( "gl_dosdriver", "gl.dxe" );
		break;
	}

	M_ForceMenuOff();
}

static void VID_FreeReflib (void)
{
#ifndef REF_HARD_LINKED
	if (reflib_library)
		Sys_dlclose(reflib_library);
	reflib_library = NULL;
#endif
	memset (&re, 0, sizeof(re));
	reflib_active  = false;
	vid_nummodes = 0;
	free(vid_modes);
	vid_modes = NULL;
}

static qboolean VID_LoadRefresh (const char *name)
{
	int		i;
	refimport_t	ri;
#ifndef REF_HARD_LINKED
	GetRefAPI_t	GetRefAPI;
#endif

	if (reflib_active)
	{
		re.Shutdown();
		VID_FreeReflib ();
	}

#ifndef REF_HARD_LINKED
	Com_Printf("------- Loading %s -------\n", name);

	if ((reflib_library = Sys_dlopen(name, false)) == NULL)
	{
		Com_Printf("dlopen(\"%s\") failed\n", name);
		return false;
	}
	if ((GetRefAPI = (GetRefAPI_t) Sys_dlsym(reflib_library, "_GetRefAPI")) == NULL)
	{
		Com_Error(ERR_FATAL, "dlsym() failed on %s\n", name);
		return false;
	}
#endif

	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.Cmd_Argc = Cmd_Argc;
	ri.Cmd_Argv = Cmd_Argv;
	ri.Cmd_ExecuteText = Cbuf_ExecuteText;
	ri.Con_Printf = VID_Printf;
	ri.Sys_Error = VID_Error;
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_Gamedir = FS_Gamedir;
	ri.Cvar_Get = Cvar_Get;
	ri.Cvar_Set = Cvar_Set;
	ri.Cvar_SetValue = Cvar_SetValue;
	ri.Cvar_SetDescription = Cvar_SetDescription; /* FS */
	ri.Cvar_ForceSet = Cvar_ForceSet; /* FS */
	ri.Vid_GetModeInfo = VID_GetModeInfo;
	ri.Vid_MenuInit = VID_MenuInit;
	ri.Vid_NewWindow = VID_NewWindow;

	re = GetRefAPI(ri);

	if (re.api_version != API_VERSION)
	{
		VID_FreeReflib ();
		Com_Error (ERR_FATAL, "%s has incompatible api_version", name);
	}

	/* HACK HACK HACK: retrieving the video modes list into here
	 * using the hInstance and wndProc parameters of re.Init(). */
	if (re.Init (&vid_nummodes, &vid_modes) == -1)
	{
		re.Shutdown();
		VID_FreeReflib ();
		return false;
	}

	reflib_active = true;

	vidref_val = VIDREF_OTHER;
	if(vid_ref)
	{
		if(!strcmp (vid_ref->string, "gl"))
			vidref_val = VIDREF_GL;
		else if(!strcmp(vid_ref->string, "soft"))
			vidref_val = VIDREF_SOFT;
	}

	for (i = 0; i < MAX_VIDEOMODES; ++i)
	{
		resolution_names[i] = (i < vid_nummodes)?
					vid_modes[i].menuname : NULL;
	}
	resolution_names[MAX_VIDEOMODES] = NULL;

	return true;
}

static char fxmesa_env_multitex[32] = "FX_DONT_FAKE_MULTITEX=1";
static char fxglide_env_nosplash[32] = "FX_GLIDE_NO_SPLASH=1";

void	VID_Init (void)
{
	/* putenv() from DJGPP libc will copy, so
	 * it is OK to use an automatic var here. */
	int	i, bpp;

	viddef.width = 320;
	viddef.height = 240;

	vid_ref = Cvar_Get ("vid_ref", "soft", CVAR_ARCHIVE);
	Cvar_SetDescription("vid_ref", "Video renderer to use.");
	vid_fullscreen = Cvar_Get ("vid_fullscreen", "1", CVAR_ARCHIVE);
	Cvar_SetDescription("vid_fullscreen", "Enable fullscreen video.  Locked to fullscreen in Q2DOS.");
	vid_gamma = Cvar_Get("vid_gamma", "1", CVAR_ARCHIVE);
	Cvar_SetDescription("vid_gamma", "Video gamma.  Use lower values for a brighter screen display.");

	/* cvars for vga/vesa code */
	Cvar_Get("vid_vgaonly", (COM_CheckParm("-vgaonly"))? "1" : "0", 0);
	Cvar_Get("vid_bankedvga", (COM_CheckParm("-bankedvga"))? "1" : "0", 0);
	/* cvars for opengl code */
	Cvar_Get("vid_glbpp", "16", 0);
	i = COM_CheckParm("-bpp");
	if (i && i < com_argc-1) {
		bpp = atoi(com_argv[i+1]);
		switch (bpp) {
		case 15: case 16: case 32:
			Cvar_SetValue("vid_glbpp", bpp);
			break;
		}
	}
	Cvar_Get ("r_ignorehwgamma", "0", CVAR_ARCHIVE);
	/* we only have 3dfx hw accel in dos: */
	if (COM_CheckParm("-no3dfxgamma"))
		Cvar_Set("r_ignorehwgamma", "1");

	/* Add some console commands that we want to handle */
	Cmd_AddCommand("vid_restart", VID_Restart_f);
	Cmd_AddCommand("vid_listmodes", VID_ListModes_f);

	/* don't let fxMesa cheat multitexturing */
	putenv(fxmesa_env_multitex);
	/* disable the 3dfx splash screen. */
	putenv(fxglide_env_nosplash);

	/* Start the graphics mode and load refresh DLL */
	VID_CheckChanges ();
}

void	VID_Shutdown (void)
{
	if (reflib_active)
	{
		re.Shutdown ();
		VID_FreeReflib ();
	}
}

void	VID_CheckChanges (void)
{
	char name[100];

	if ( vid_ref->modified)
	{
		cl.force_refdef = true; /* can't use a paused refdef */
		S_StopAllSounds();
	}
	while (vid_ref->modified)
	{
		/*
		** refresh has changed
		*/
		vid_ref->modified = false;
		vid_fullscreen->modified = true;
		cl.refresh_prepped = false;
		cls.disable_screen = true;

		Com_sprintf(name, sizeof(name), "ref_%s.dxe", vid_ref->string);
		if (!VID_LoadRefresh(name))
		{
			if (strcmp (vid_ref->string, "soft") == 0)
				Com_Error (ERR_FATAL, "Couldn't fall back to software refresh!");
			Cvar_Set("vid_ref", "soft");

			/*
			** drop the console if we fail to load a refresh
			*/
			if (cls.key_dest != key_console)
			{
				Con_ToggleConsole_f();
			}
		}
		cls.disable_screen = false;
	}
}

// Knightmare- texture filter mode
int texfilter_box_setval (void)
{
	char *texmode = Cvar_VariableString ("gl_texturemode");
	if (!Q_strcasecmp(texmode, "GL_LINEAR_MIPMAP_NEAREST"))
		return 0;
	else
		return 1;
}

void	VID_MenuInit (void)
{
	static const char *yesno_names[] = {
		"no",
		"yes",
		NULL
	};

	static const char *refs[] =
	{
		"[software      ]",
		"[3Dfx OpenGL   ]",
		NULL
	};

	static const char *filter_names[] =
	{
		"bilinear",
		"trilinear",
		NULL
	};

	int		i;
	float	temp;

	if (!sw_stipplealpha)
		sw_stipplealpha = Cvar_Get("sw_stipplealpha", "0", CVAR_ARCHIVE);

	if (!r_contentblend) /* FS */
		r_contentblend = Cvar_Get( "r_contentblend", "1", CVAR_ARCHIVE);

	if (!sw_waterwarp) /* FS */
		sw_waterwarp = Cvar_Get( "sw_waterwarp", "1", CVAR_ARCHIVE);

	temp = Cvar_VariableValue("sw_mode");
	s_mode_list[SOFTWARE_MENU].curvalue = temp;
	temp = Cvar_VariableValue("gl_mode");
	s_mode_list[OPENGL_MENU].curvalue = temp;

	if ( !scr_viewsize )
		scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE);

	s_screensize_slider[SOFTWARE_MENU].curvalue = scr_viewsize->value/10;
	s_screensize_slider[OPENGL_MENU].curvalue = scr_viewsize->value/10;

	if ( strcmp( Cvar_VariableString("vid_ref"), "soft" ) == 0 )
	{
		s_current_menu_index = SOFTWARE_MENU;
		s_ref_list[0].curvalue = s_ref_list[1].curvalue = REF_SOFT;
	}
	else if ( strcmp( Cvar_VariableString("vid_ref"), "gl" ) == 0 )
	{
		s_current_menu_index = OPENGL_MENU;
		s_ref_list[s_current_menu_index].curvalue = REF_OPENGL;
	}

	s_software_menu.x = viddef.width * 0.50;
	s_software_menu.y = viddef.height * 0.50 - 58;
	s_software_menu.nitems = 0;
	s_opengl_menu.x = viddef.width * 0.50;
	s_opengl_menu.y = viddef.height * 0.50 - 58;
	s_opengl_menu.nitems = 0;

	for (i = 0; i < NUM_VID_DRIVERS; i++)
	{
		s_ref_list[i].generic.type		= MTYPE_SPINCONTROL;
		s_ref_list[i].generic.name		= "graphics renderer";
		s_ref_list[i].generic.x			= 0;
		s_ref_list[i].generic.y			= 0;
		s_ref_list[i].generic.callback	= RefreshCallback;
		s_ref_list[i].itemnames			= refs;
		s_ref_list[i].generic.statusbar	= "changes video refresh";

		s_mode_list[i].generic.type = MTYPE_SPINCONTROL;
		s_mode_list[i].generic.name = "video resolution";
		s_mode_list[i].generic.x = 0;
		s_mode_list[i].generic.y = 10; /* FS: Y position for where the spin control ends up */
		s_mode_list[i].generic.callback = NULL;
		s_mode_list[i].itemnames = resolution_names;
		s_mode_list[i].generic.statusbar	= "changes screen resolution";

		s_screensize_slider[i].generic.type	= MTYPE_SLIDER;
		s_screensize_slider[i].generic.x		= 0;
		s_screensize_slider[i].generic.y		= 20;
		s_screensize_slider[i].generic.name	= "screen size";
		s_screensize_slider[i].minvalue = 3;
		s_screensize_slider[i].maxvalue = 12;
		s_screensize_slider[i].generic.callback = ScreenSizeCallback;
		s_screensize_slider[i].generic.statusbar = "changes visible screen size";

		s_brightness_slider[i].generic.type	= MTYPE_SLIDER;
		s_brightness_slider[i].generic.x	= 0;
		s_brightness_slider[i].generic.y	= 30;
		s_brightness_slider[i].generic.name	= "brightness";
		s_brightness_slider[i].generic.callback = BrightnessCallback;
		s_brightness_slider[i].minvalue = 5;
		s_brightness_slider[i].maxvalue = 13;
//		s_brightness_slider[i].curvalue = ( 1.3 - vid_gamma->value + 0.5 ) * 10;
		s_brightness_slider[i].curvalue = ( 1.3 - vid_gamma->value ) * 20;
		s_brightness_slider[i].generic.statusbar	= "changes display brightness";

		s_defaults_action[i].generic.type		= MTYPE_ACTION;
		s_defaults_action[i].generic.name		= "reset to defaults";
		s_defaults_action[i].generic.x			= 0;
		s_defaults_action[i].generic.y			= 130;
		s_defaults_action[i].generic.callback	= ResetDefaults;
		s_defaults_action[i].generic.statusbar	= "resets all video settings to internal defaults";

		s_apply_action[i].generic.type		= MTYPE_ACTION;
		s_apply_action[i].generic.name		= "apply changes";
		s_apply_action[i].generic.x			= 0;
		s_apply_action[i].generic.y			= 140;
		s_apply_action[i].generic.callback	= ApplyChanges;
	}

	s_stipple_box.generic.type = MTYPE_SPINCONTROL;
	s_stipple_box.generic.x	= 0;
	s_stipple_box.generic.y	= 60;
	s_stipple_box.generic.name	= "stipple alpha";
	s_stipple_box.curvalue = sw_stipplealpha->value;
	s_stipple_box.itemnames = yesno_names;
	s_stipple_box.generic.statusbar	= "enables stipple drawing of trans surfaces";

	/* FS */
	s_contentblend_box.generic.type = MTYPE_SPINCONTROL;
	s_contentblend_box.generic.x	= 0;
	s_contentblend_box.generic.y	= 70;
	s_contentblend_box.generic.name	= "content blending";
	s_contentblend_box.curvalue = r_contentblend->intValue;
	s_contentblend_box.itemnames = yesno_names;

	/* FS */
	s_waterwarp_box.generic.type = MTYPE_SPINCONTROL;
	s_waterwarp_box.generic.x	= 0;
	s_waterwarp_box.generic.y	= 80;
	s_waterwarp_box.generic.name	= "water warping";
	s_waterwarp_box.curvalue = sw_waterwarp->intValue;
	s_waterwarp_box.itemnames = yesno_names;

	/* FS: OpenGL stuff */
	s_tq_slider.generic.type			= MTYPE_SLIDER;
	s_tq_slider.generic.x				= 0;
	s_tq_slider.generic.y				= 60;
	s_tq_slider.generic.name			= "texture quality";
	s_tq_slider.minvalue				= 0;
	s_tq_slider.maxvalue				= 3;
	s_tq_slider.curvalue				= 3-Cvar_VariableValue("gl_picmip");
	s_tq_slider.generic.statusbar		= "changes detail level of textures";

	s_paletted_texture_box.generic.type			= MTYPE_SPINCONTROL;
	s_paletted_texture_box.generic.x			= 0;
	s_paletted_texture_box.generic.y			= 70;
	s_paletted_texture_box.generic.name			= "8-bit textures";
	s_paletted_texture_box.itemnames			= yesno_names;
	s_paletted_texture_box.curvalue				= Cvar_VariableValue("gl_ext_palettedtexture");
	s_paletted_texture_box.generic.statusbar	= "enables rendering of textures in 8-bit form";

	s_texfilter_box.generic.type		= MTYPE_SPINCONTROL;
	s_texfilter_box.generic.x			= 0;
	s_texfilter_box.generic.y			= 80;
	s_texfilter_box.generic.name		= "texture filter";
	s_texfilter_box.curvalue			= texfilter_box_setval();
	s_texfilter_box.itemnames			= filter_names;
	s_texfilter_box.generic.statusbar	= "changes texture filtering mode";
	s_texfilter_box.generic.callback	= TexFilterCallback;

	/* FS: ATTN  AddItem order has to be the order you want to see it in or else the cursor gets wonky! */
	Menu_AddItem(&s_software_menu, (void *) &s_ref_list[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, (void *) &s_mode_list[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, (void *) &s_screensize_slider[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, (void *) &s_brightness_slider[SOFTWARE_MENU]);
	Menu_AddItem(&s_software_menu, (void *) &s_stipple_box);
	Menu_AddItem(&s_software_menu, (void *) &s_contentblend_box); /* FS */
	Menu_AddItem(&s_software_menu, (void *) &s_waterwarp_box); /* FS */

	Menu_AddItem( &s_opengl_menu, ( void * ) &s_ref_list[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_mode_list[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_screensize_slider[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_brightness_slider[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_tq_slider );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_paletted_texture_box );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_texfilter_box );
	Menu_AddItem( &s_software_menu, ( void * ) &s_defaults_action[SOFTWARE_MENU] );
	Menu_AddItem( &s_software_menu, ( void * ) &s_apply_action[SOFTWARE_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_defaults_action[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_apply_action[OPENGL_MENU] );
}

void	VID_MenuDraw (void)
{
	int w, h;

	if ( s_current_menu_index == 0 )
		s_current_menu = &s_software_menu;
	else
		s_current_menu = &s_opengl_menu;

	/*
	** draw the banner
	*/
	re.DrawGetPicSize( &w, &h, "m_banner_video" );
	re.DrawPic( viddef.width / 2 - w / 2, viddef.height /2 - 110, "m_banner_video" );

	/*
	** move cursor to a reasonable starting position
	*/
	Menu_AdjustCursor( s_current_menu, 1 );

	/*
	** draw the menu
	*/
	Menu_Draw( s_current_menu );
}

const char *VID_MenuKey (int k)
{
	return (const char *)Default_MenuKey (s_current_menu, k);
}

static void VID_Restart_f (void)
{
	vid_ref->modified = true;
}

static void VID_ListModes_f (void)
{
	int i;

	for(i = 0; i < vid_nummodes; i++)
	{
		if(vid_modes[i].menuname[0] != 0)
			Com_Printf("[Mode %02d] %s\n", i, vid_modes[i].menuname);
	}
	Com_Printf("Available modes: %d\n", vid_nummodes);
}
