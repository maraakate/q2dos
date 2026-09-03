#include "../client/client.h"
#include "../client/qmenu.h"

/*
====================================================================

REF stuff ...
Used to dynamically load the menu with only those vid_ref's that
are present on this system

====================================================================
*/

#define REF_SOFT	0
#define REF_GL		1

#if defined(SDL_CLIENT) /* to load SDL's default opengl driver -- see qgl_linux.c */
#define DEFAULT_LIBGL ""
#elif defined(__APPLE__)
#define DEFAULT_LIBGL "/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib"
#elif defined(__NetBSD__)||defined(__OpenBSD__)||defined(__sgi__)
#define DEFAULT_LIBGL "libGL.so"
#else /* other unix */
#define DEFAULT_LIBGL "libGL.so.1"
#endif

/*
====================================================================
*/

extern cvar_t *vid_ref;
extern cvar_t *vid_fullscreen;
extern cvar_t *vid_gamma;
extern cvar_t *scr_viewsize;

static cvar_t *gl_mode;
static cvar_t *gl_driver;
static cvar_t *gl_picmip;
static cvar_t *gl_ext_palettedtexture;

static cvar_t *sw_mode;
static cvar_t *sw_stipplealpha;

static cvar_t *_windowed_mouse;

extern void M_ForceMenuOff( void );
extern const char *Default_MenuKey( menuframework_s *m, int key );

/*
====================================================================

MENU INTERACTION

====================================================================
*/
#define SOFTWARE_MENU 0
#define OPENGL_MENU   1

static menuframework_s  s_software_menu;
static menuframework_s	s_opengl_menu;
static menuframework_s *s_current_menu;
static int				s_current_menu_index;

static menulist_s		s_mode_list[2];
static menulist_s		s_ref_list[2];
static menuslider_s		s_tq_slider;
static menuslider_s		s_screensize_slider[2];
static menuslider_s		s_brightness_slider[2];
static menulist_s  		s_fs_box[2];
static menulist_s  		s_stipple_box;
static menulist_s		s_contentblend_box;	/* FS */
static menulist_s		s_waterwarp_box;	/* FS */
static menulist_s  		s_paletted_texture_box;
static menulist_s  		s_texfilter_box;
static menulist_s  		s_aniso_box;
static menulist_s  		s_windowed_mouse;
static menuaction_s		s_apply_action[2];
static menuaction_s		s_defaults_action[2];

static void DriverCallback( void *unused )
{
	s_ref_list[!s_current_menu_index].curvalue = s_ref_list[s_current_menu_index].curvalue;

	if ( s_ref_list[s_current_menu_index].curvalue < 1 )
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
	menuslider_s *slider = ( menuslider_s * ) s;
	float			gamma;

	if ( s_current_menu_index == 0)
		s_brightness_slider[1].curvalue = s_brightness_slider[0].curvalue;
	else
		s_brightness_slider[0].curvalue = s_brightness_slider[1].curvalue;

//	gamma = ( 0.8 - ( slider->curvalue/10.0 - 0.5 ) ) + 0.5;
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

// Knightmare- anisotropic filtering
static void AnisoCallback( void *s )
{
	switch ((int)s_aniso_box.curvalue)
	{
		case 1: Cvar_SetValue ("gl_anisotropic", 2.0); break;
		case 2: Cvar_SetValue ("gl_anisotropic", 4.0); break;
		case 3: Cvar_SetValue ("gl_anisotropic", 8.0); break;
		case 4: Cvar_SetValue ("gl_anisotropic", 16.0); break;
		default:
		case 0: Cvar_SetValue ("gl_anisotropic", 0.0); break;
	}
}

static void ResetDefaults( void *unused )
{
	VID_MenuInit();
}

static void ApplyChanges( void *unused )
{
	float gamma;

	/*
	** make values consistent
	*/
	s_fs_box[!s_current_menu_index].curvalue = s_fs_box[s_current_menu_index].curvalue;
	s_brightness_slider[!s_current_menu_index].curvalue = s_brightness_slider[s_current_menu_index].curvalue;
	s_ref_list[!s_current_menu_index].curvalue = s_ref_list[s_current_menu_index].curvalue;

	/*
	** invert sense so greater = brighter, and scale to a range of 0.5 to 1.3
	*/
//	gamma = ( 0.8 - ( s_brightness_slider[s_current_menu_index].curvalue/10.0 - 0.5 ) ) + 0.5;
	gamma = (1.3 - (s_brightness_slider[s_current_menu_index].curvalue/20.0));

	Cvar_SetValue( "vid_gamma", gamma );
	Cvar_SetValue( "sw_stipplealpha", s_stipple_box.curvalue );
	Cvar_SetValue( "r_contentblend", s_contentblend_box.curvalue ); // FS
	Cvar_SetValue( "sw_waterwarp", s_waterwarp_box.curvalue ); // FS
	Cvar_SetValue( "gl_picmip", 3 - s_tq_slider.curvalue );
	Cvar_SetValue( "vid_fullscreen", s_fs_box[s_current_menu_index].curvalue );
	Cvar_SetValue( "gl_ext_palettedtexture", s_paletted_texture_box.curvalue );
	Cvar_SetValue( "sw_mode", s_mode_list[SOFTWARE_MENU].curvalue );
	Cvar_SetValue( "gl_mode", s_mode_list[OPENGL_MENU].curvalue );
	Cvar_SetValue( "_windowed_mouse", s_windowed_mouse.curvalue);

	// Knightmare- texture filter mode
	if (s_texfilter_box.curvalue == 1)
		Cvar_Set ("gl_texturemode", "GL_LINEAR_MIPMAP_LINEAR");
	else // (s_texfilter_box.curvalue == 0)
		Cvar_Set ("gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST");

	// Knightmare- anisotropic filtering
	switch ((int)s_aniso_box.curvalue)
	{
		case 1: Cvar_SetValue ("gl_anisotropic", 2.0); break;
		case 2: Cvar_SetValue ("gl_anisotropic", 4.0); break;
		case 3: Cvar_SetValue ("gl_anisotropic", 8.0); break;
		case 4: Cvar_SetValue ("gl_anisotropic", 16.0); break;
		default:
		case 0: Cvar_SetValue ("gl_anisotropic", 0.0); break;
	}

	switch ( s_ref_list[s_current_menu_index].curvalue )
	{
	case REF_SOFT:
		Cvar_Set( "vid_ref", "soft" );
		break;
	case REF_GL:
		Cvar_Set( "vid_ref", "gl" );
		Cvar_Get( "gl_driver", DEFAULT_LIBGL, 0 );
		if (gl_driver->modified)
			vid_ref->modified = true;
		break;
	}

	M_ForceMenuOff();
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

// Knightmare- anisotropic filtering
static const char *aniso0_names[] =
{
	"not supported",
	NULL
};

static const char *aniso2_names[] =
{
	"off",
	"2x",
	NULL
};

static const char *aniso4_names[] =
{
	"off",
	"2x",
	"4x",
	NULL
};

static const char *aniso8_names[] =
{
	"off",
	"2x",
	"4x",
	"8x",
	NULL
};

static const char *aniso16_names[] =
{
	"off",
	"2x",
	"4x",
	"8x",
	"16x",
	NULL
};

static const char **GetAnisoNames ()
{
	float aniso_avail = Cvar_VariableValue("gl_anisotropic_avail");
	if (aniso_avail < 2.0)
		return aniso0_names;
	else if (aniso_avail < 4.0)
		return aniso2_names;
	else if (aniso_avail < 8.0)
		return aniso4_names;
	else if (aniso_avail < 16.0)
		return aniso8_names;
	else // >= 16.0
		return aniso16_names;
}

float GetAnisoCurValue ()
{
	float aniso_avail = Cvar_VariableValue("gl_anisotropic_avail");
	float anisoValue = ClampCvar (0, aniso_avail, Cvar_VariableValue("gl_anisotropic"));
	if (aniso_avail == 0) // not available
		return 0;
	if (anisoValue < 2.0)
		return 0;
	else if (anisoValue < 4.0)
		return 1;
	else if (anisoValue < 8.0)
		return 2;
	else if (anisoValue < 16.0)
		return 3;
	else // >= 16.0
		return 4;
}
// end Knightmare

/*
** VID_MenuInit
*/
void VID_MenuInit( void )
{
	static const char *resolutions[] = 
	{
		"[320 240  ]",
		"[400 300  ]",
		"[512 384  ]",
		"[640 480  ]",
		"[800 600  ]",
		"[960 720  ]",
		"[1024 768 ]",
		"[1152 864 ]",
		"[1280 1024]",
		"[1600 1200]",
		"[2048 1536]",
		"[1024 480 ]", /* sony vaio pocketbook */
		"[1152 768 ]", /* Apple TiBook */
		"[1280 854 ]", /* apple TiBook */
		"[640 400  ]", /* generic 16:10 widescreen resolutions */
 		"[800 500  ]", /* as found on many modern notebooks    */
 		"[1024 640 ]",
 		"[1280 800 ]",
 		"[1680 1050]",
 		"[1920 1200]",
		NULL
	};
	static const char *refs[] =
	{
		"[software       ]",
		"[OpenGL         ]",
		NULL
	};
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		NULL
	};
	static const char *filter_names[] =
	{
		"no",
		"yes",
		"bilinear",
		"trilinear",
		NULL
	};
	int i;

	if ( !gl_driver )
		gl_driver = Cvar_Get( "gl_driver", DEFAULT_LIBGL, 0 );
	if ( !gl_picmip )
		gl_picmip = Cvar_Get( "gl_picmip", "0", 0 );
	if ( !gl_mode )
		gl_mode = Cvar_Get( "gl_mode", "3", 0 );
	if ( !sw_mode )
		sw_mode = Cvar_Get( "sw_mode", "0", 0 );
	if ( !gl_ext_palettedtexture )
		gl_ext_palettedtexture = Cvar_Get( "gl_ext_palettedtexture", "1", CVAR_ARCHIVE );

	if ( !sw_stipplealpha )
		sw_stipplealpha = Cvar_Get( "sw_stipplealpha", "0", CVAR_ARCHIVE );

	if ( !_windowed_mouse)
		_windowed_mouse = Cvar_Get( "_windowed_mouse", "0", CVAR_ARCHIVE );

	s_mode_list[SOFTWARE_MENU].curvalue = sw_mode->value;
	s_mode_list[OPENGL_MENU].curvalue = gl_mode->value;

	if ( !scr_viewsize )
		scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE);

	s_screensize_slider[SOFTWARE_MENU].curvalue = scr_viewsize->value/10;
	s_screensize_slider[OPENGL_MENU].curvalue = scr_viewsize->value/10;

	if ( strcmp( vid_ref->string, "soft" ) == 0)
	{
		s_current_menu_index = SOFTWARE_MENU;
		s_ref_list[0].curvalue = s_ref_list[1].curvalue = REF_SOFT;
	}
	else if ( strcmp( vid_ref->string, "gl" ) == 0 )
	{
		s_current_menu_index = OPENGL_MENU;
		s_ref_list[s_current_menu_index].curvalue = REF_GL;
	}

	s_software_menu.x = viddef.width * 0.50;
	s_software_menu.nitems = 0;
	s_opengl_menu.x = viddef.width * 0.50;
	s_opengl_menu.nitems = 0;

	for ( i = 0; i < 2; i++ )
	{
		s_ref_list[i].generic.type = MTYPE_SPINCONTROL;
		s_ref_list[i].generic.name = "driver";
		s_ref_list[i].generic.x = 0;
		s_ref_list[i].generic.y = 0;
		s_ref_list[i].generic.callback = DriverCallback;
		s_ref_list[i].itemnames = refs;

		s_mode_list[i].generic.type = MTYPE_SPINCONTROL;
		s_mode_list[i].generic.name = "video mode";
		s_mode_list[i].generic.x = 0;
		s_mode_list[i].generic.y = 10;
		s_mode_list[i].itemnames = resolutions;

		s_screensize_slider[i].generic.type	= MTYPE_SLIDER;
		s_screensize_slider[i].generic.x		= 0;
		s_screensize_slider[i].generic.y		= 20;
		s_screensize_slider[i].generic.name	= "screen size";
		s_screensize_slider[i].minvalue = 3;
		s_screensize_slider[i].maxvalue = 12;
		s_screensize_slider[i].generic.callback = ScreenSizeCallback;

		s_brightness_slider[i].generic.type	= MTYPE_SLIDER;
		s_brightness_slider[i].generic.x	= 0;
		s_brightness_slider[i].generic.y	= 30;
		s_brightness_slider[i].generic.name	= "brightness";
		s_brightness_slider[i].generic.callback = BrightnessCallback;
		s_brightness_slider[i].minvalue = 5;
		s_brightness_slider[i].maxvalue = 13;
		s_brightness_slider[i].curvalue = ( 1.3 - vid_gamma->value + 0.5 ) * 10;

		s_fs_box[i].generic.type = MTYPE_SPINCONTROL;
		s_fs_box[i].generic.x	= 0;
		s_fs_box[i].generic.y	= 40;
		s_fs_box[i].generic.name	= "fullscreen";
		s_fs_box[i].itemnames = yesno_names;
		s_fs_box[i].curvalue = vid_fullscreen->value;

		s_defaults_action[i].generic.type = MTYPE_ACTION;
		s_defaults_action[i].generic.name = "reset to default";
		s_defaults_action[i].generic.x    = 0;
		s_defaults_action[i].generic.y    = 130;
		s_defaults_action[i].generic.callback = ResetDefaults;

		s_apply_action[i].generic.type = MTYPE_ACTION;
		s_apply_action[i].generic.name = "apply";
		s_apply_action[i].generic.x    = 0;
		s_apply_action[i].generic.y    = 140;
		s_apply_action[i].generic.callback = ApplyChanges;
	}

	s_stipple_box.generic.type = MTYPE_SPINCONTROL;
	s_stipple_box.generic.x	= 0;
	s_stipple_box.generic.y	= 60;
	s_stipple_box.generic.name	= "stipple alpha";
	s_stipple_box.curvalue = sw_stipplealpha->value;
	s_stipple_box.itemnames = yesno_names;

	s_windowed_mouse.generic.type = MTYPE_SPINCONTROL;
	s_windowed_mouse.generic.x  = 0;
	s_windowed_mouse.generic.y  = 110;
	s_windowed_mouse.generic.name   = "windowed mouse";
	s_windowed_mouse.curvalue = _windowed_mouse->value;
	s_windowed_mouse.itemnames = yesno_names;

	/* FS */
	s_contentblend_box.generic.type = MTYPE_SPINCONTROL;
	s_contentblend_box.generic.x	= 0;
	s_contentblend_box.generic.y	= 70;
	s_contentblend_box.generic.name	= "content blending";
	s_contentblend_box.curvalue = Cvar_VariableValue("r_contentblend");
	s_contentblend_box.itemnames = yesno_names;

	/* FS */
	s_waterwarp_box.generic.type = MTYPE_SPINCONTROL;
	s_waterwarp_box.generic.x	= 0;
	s_waterwarp_box.generic.y	= 80;
	s_waterwarp_box.generic.name	= "water warping";
	s_waterwarp_box.curvalue = Cvar_VariableValue("sw_waterwarp");
	s_waterwarp_box.itemnames = yesno_names;

	s_tq_slider.generic.type	= MTYPE_SLIDER;
	s_tq_slider.generic.x		= 0;
	s_tq_slider.generic.y		= 60;
	s_tq_slider.generic.name	= "texture quality";
	s_tq_slider.minvalue = 0;
	s_tq_slider.maxvalue = 3;
	s_tq_slider.curvalue = 3-gl_picmip->value;

	s_paletted_texture_box.generic.type = MTYPE_SPINCONTROL;
	s_paletted_texture_box.generic.x	= 0;
	s_paletted_texture_box.generic.y	= 70;
	s_paletted_texture_box.generic.name	= "8-bit textures";
	s_paletted_texture_box.itemnames = yesno_names;
	s_paletted_texture_box.curvalue = gl_ext_palettedtexture->value;

	s_texfilter_box.generic.type		= MTYPE_SPINCONTROL;
	s_texfilter_box.generic.x			= 0;
	s_texfilter_box.generic.y			= 90;
	s_texfilter_box.generic.name		= "texture filter";
	s_texfilter_box.curvalue			= texfilter_box_setval();
	s_texfilter_box.itemnames			= filter_names;
	s_texfilter_box.generic.statusbar	= "changes texture filtering mode";
	s_texfilter_box.generic.callback	= TexFilterCallback;

	s_aniso_box.generic.type		= MTYPE_SPINCONTROL;
	s_aniso_box.generic.x			= 0;
	s_aniso_box.generic.y			= 100;
	s_aniso_box.generic.name		= "anisotropic filter";
	s_aniso_box.curvalue			= GetAnisoCurValue();
	s_aniso_box.itemnames			= GetAnisoNames();
	s_aniso_box.generic.statusbar	= "changes level of anisotropic mipmap filtering";
	s_aniso_box.generic.callback	= AnisoCallback;

	Menu_AddItem( &s_software_menu, ( void * ) &s_ref_list[SOFTWARE_MENU] );
	Menu_AddItem( &s_software_menu, ( void * ) &s_mode_list[SOFTWARE_MENU] );
	Menu_AddItem( &s_software_menu, ( void * ) &s_screensize_slider[SOFTWARE_MENU] );
	Menu_AddItem( &s_software_menu, ( void * ) &s_brightness_slider[SOFTWARE_MENU] );
	Menu_AddItem( &s_software_menu, ( void * ) &s_fs_box[SOFTWARE_MENU] );
	Menu_AddItem( &s_software_menu, ( void * ) &s_stipple_box );
	Menu_AddItem( &s_software_menu, ( void * ) &s_contentblend_box ); // FS
	Menu_AddItem( &s_software_menu, ( void * ) &s_waterwarp_box ); // FS
	Menu_AddItem( &s_software_menu, ( void * ) &s_windowed_mouse );

	Menu_AddItem( &s_opengl_menu, ( void * ) &s_ref_list[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_mode_list[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_screensize_slider[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_brightness_slider[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_fs_box[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_tq_slider );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_paletted_texture_box );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_texfilter_box );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_aniso_box );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_windowed_mouse );

	Menu_AddItem( &s_software_menu, ( void * ) &s_defaults_action[SOFTWARE_MENU] );
	Menu_AddItem( &s_software_menu, ( void * ) &s_apply_action[SOFTWARE_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_defaults_action[OPENGL_MENU] );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_apply_action[OPENGL_MENU] );

	Menu_Center( &s_software_menu );
	Menu_Center( &s_opengl_menu );
	s_opengl_menu.x -= 8;
	s_software_menu.x -= 8;
}

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
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

/*
================
VID_MenuKey
================
*/
const char *VID_MenuKey( int key )
{
	return Default_MenuKey (s_current_menu, key);
}
