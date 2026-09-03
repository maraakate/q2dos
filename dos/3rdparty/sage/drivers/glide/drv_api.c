/**
 * \file drv_api.c
 * Glide driver API.
 */


#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <glide.h>
#include <g3ext.h>
#include "GL/gl.h"
#include "GL/sage.h"

#include "glinternal.h"
#include "main/context.h"
#include "tnl/tnl.h"
#include "main/glapi.h"
#include "util/alloc.h"
#include "util/cfg.h"
#include "driver.h"
#include "drv.h"


/** Check if specific variable is set to "y" */
#define YES(v) !strcmp(cfg_get(v, "n"), "y")


static int hardware = -1;	/**< Hardware type */

static int hwext_texuma;	/**< Hardware has TEXUMA */
static int hwext_texmirror;	/**< Hardware has TEXMIRROR */
static int hwext_fogcoord;	/**< Hardware has FOGCOORD */
static int hwext_texfmt;	/**< Hardware has TEXFMT */
static int hwext_pixext;	/**< Hardware has PIXEXT */
static int hwext_combine;	/**< Hardware has COMBINE */

static sageContext *current;	/**< Current context */

GrVertex *vb;			/**< Vertex buffer */
char renderer_name[64];		/**< Renderer name (GL_RENDERER) */
const char *opengl_ver = NULL;	/**< OpenGL version (GL_VERSION) */
GrFog_t *fogtable;		/**< Hardware fog synthetic function */
int tmu0_source, tmu1_source;	/**< TMU order when single-/multitexturing */
int screen_width, screen_height;/**< Screen geometry */
int fb_color, fb_alpha;		/**< Framebuffer bits */
GLuint tex_bound_mask;		/**< Chunk limit for texture cache */
int max_texture_levels;		/**< Hardware limit for texture size */
GrBuffer_t render_buffer;	/**< Write buffer */

int allow_texuma;		/**< Unified Memory Architecture */
int allow_texmirror;		/**< Texture mirror (GL_ARB_texture_mirrored_repeat) */
int allow_fogcoord;		/**< Fog coordinate (GL_EXT_fog_coord) */
int allow_32bpt;		/**< Allow 32bit textures on 16bit visuals */
int allow_combine;		/**< Enable or emulate texture env combine */
int allow_blendsquare;		/**< Allow blend square (GL_NV_blend_square) */
int allow_compressed;		/**< Allow compressed textures */
int allow_multitex;		/**< Allow multitexture if available */

int (*drv_multipass) (int pass);
void (*drv_emitvertices) (int n);
void (*drv_copypv) (int vdst, int vsrc);
void (*drv_interp) (float t, int vdst, int vout, int vin);


/** Get registry or environment string. */
gfGetRegistryOrEnvironmentStringExt_f gfGetRegistryOrEnvironmentStringExt;
/** Open hardware context. */
gfSstWinOpenExt_f gfSstWinOpenExt;
/** Clear buffer. */
gfBufferClearExt_f gfBufferClearExt;
/** Color mask. */
gfColorMaskExt_f gfColorMaskExt;
/** Stencil function. */
gfStencilFuncExt_f gfStencilFuncExt;
/** Stencil mask. */
gfStencilMaskExt_f gfStencilMaskExt;
/** Stencil op. */
gfStencilOpExt_f gfStencilOpExt;
/** Color combiner. */
gfColorCombineExt_f gfColorCombineExt;
/** Alpha combiner. */
gfAlphaCombineExt_f gfAlphaCombineExt;
/** Texture color combiner. */
gfTexColorCombineExt_f gfTexColorCombineExt;
/** Texture alpha combiner. */
gfTexAlphaCombineExt_f gfTexAlphaCombineExt;
/** Constant color. */
gfConstantColorValueExt_f gfConstantColorValueExt;


/**
 * Query hardware.
 *
 * \param pname parameter
 *
 * \return value, or -1 if error
 */
static FxI32
getInteger (FxU32 pname)
{
    FxI32 result;

    if (grGet(pname, 4, &result)) {
	return result;
    }

    return -1;
}


/**
 * Query screen width and height.
 *
 * \param[out] w width
 * \param[out] h height
 *
 * \return 0 if success, -1 if error
 */
static FxI32
getGeometry (int *w, int *h)
{
    FxI32 result[4];

    if (grGet(GR_VIEWPORT, 16, result)) {
	*w = result[2];
	*h = result[3];
	return 0;
    }

    return -1;
}


/**
 * Query hardware type.
 *
 * \return hardware
 */
static FxI32
getHardware (void)
{
    const char *str;

    str = grGetString(GR_HARDWARE);

    if (strstr(str, "Rush")) {
	return GR_SSTTYPE_SST96;
    } else if (strstr(str, "Voodoo2")) {
	return GR_SSTTYPE_Voodoo2;
    } else if (strstr(str, "Voodoo Banshee")) {
	return GR_SSTTYPE_Banshee;
    } else if (strstr(str, "Voodoo3")) {
	return GR_SSTTYPE_Voodoo3;
    } else if (strstr(str, "Voodoo4")) {
	return GR_SSTTYPE_Voodoo4;
    } else if (strstr(str, "Voodoo5")) {
	return GR_SSTTYPE_Voodoo5;
    } else {
	return GR_SSTTYPE_VOODOO;
    }
}


/**
 * Determine FSAA settings.
 *
 * \param numChips number of GPUs
 *
 * \return 1 = noFSAA, 2 = 2xFSAA, 4 = 4xFSAA, 8 = 8xFSAA
 *
 * \note Always return 1 if not Napalm
 */
static int
getFSAA (int numChips)
{
    int numSLI;
    int samplesPerChip;
    int sliaa = 0;

    if (hardware >= GR_SSTTYPE_Voodoo4) {
	/* number of SLI units and AA Samples per chip */
	sliaa = atoi(cfg_get("SSTH3_SLI_AA_CONFIGURATION", "0"));
    }

    switch (numChips) {
	case 4: /* 4 chips */
	    switch (sliaa) {
		case 8: /* 8 Sample AA */
		    numSLI         = 1;
		    samplesPerChip = 2;
		    break;
		case 7: /* 4 Sample AA */
		    numSLI         = 1;
		    samplesPerChip = 1;
		    break;
		case 6: /* 2 Sample AA */
		    numSLI         = 2;
		    samplesPerChip = 1;
		    break;
		default:
		    numSLI         = 4;
		    samplesPerChip = 1;
	    }
	    break;
	case 2: /* 2 chips */
	    switch (sliaa) {
		case 4: /* 4 Sample AA */
		    numSLI         = 1;
		    samplesPerChip = 2;
		    break;
		case 3: /* 2 Sample AA */
		    numSLI         = 1;
		    samplesPerChip = 1;
		    break;
		default:
		    numSLI         = 2;
		    samplesPerChip = 1;
	    }
	    break;
	default: /* 1 chip */
	    switch (sliaa) {
		case 1: /* 2 Sample AA */
		    numSLI         = 1;
		    samplesPerChip = 2;
		    break;
		default:
		    numSLI         = 1;
		    samplesPerChip = 1;
	    }
    }

    return samplesPerChip * numChips / numSLI;
}


/**
 * Find best resolution for given geometry.
 *
 * \param width desired width in pixels
 * \param height desired height in pixels
 *
 * \return best resolution, or 640x480 as default
 */
static GrScreenResolution_t
findBestRes (int width, int height)
{
#define GLIDE_RESOLUTION(w, h) { GR_RESOLUTION_##w##x##h, w, h }
    static int resolutions[][3] = {
	GLIDE_RESOLUTION(320, 200),
	GLIDE_RESOLUTION(320, 240),
	GLIDE_RESOLUTION(512, 384),
	GLIDE_RESOLUTION(640, 400),
	GLIDE_RESOLUTION(640, 480),
	GLIDE_RESOLUTION(800, 600),
	GLIDE_RESOLUTION(1024, 768),
	GLIDE_RESOLUTION(1280, 1024),
	GLIDE_RESOLUTION(1600, 1200),
	/**/
	GLIDE_RESOLUTION(400, 300),
	GLIDE_RESOLUTION(856, 480),
	GLIDE_RESOLUTION(960, 720),
	GLIDE_RESOLUTION(1152, 864),
	GLIDE_RESOLUTION(1280, 960),
	GLIDE_RESOLUTION(1600, 1024),
	GLIDE_RESOLUTION(1792, 1344),
	GLIDE_RESOLUTION(1856, 1392),
	GLIDE_RESOLUTION(1920, 1440),
	GLIDE_RESOLUTION(2048, 1536),
	GLIDE_RESOLUTION(2048, 2048)
    };
    unsigned i;
    for (i = 0; i < sizeof(resolutions) / sizeof(resolutions[0]); i++) {
	if (resolutions[i][1] == width && resolutions[i][2] == height) {
	    return resolutions[i][0];
	}
    }
    return GR_RESOLUTION_640x480;
#undef GLIDE_RESOLUTION
}


/**
 * Find best pixel format.
 *
 * \param[in,out] red_size desired red channel bits
 * \param[in,out] green_size desired green channel bits
 * \param[in,out] blue_size desired blue channel bits
 * \param[in,out] alpha_size desired alpha channel bits
 * \param[in,out] depth_size desired depth channel bits
 * \param[in,out] stencil_size desired stencil channel bits
 *
 * \return best pixel format
 */
static GrPixelFormat_t
findPixelFormat (int *red_size, int *green_size, int *blue_size,
		 int *alpha_size, int *depth_size, int *stencil_size)
{
    int colorDepth = *red_size + *green_size + *blue_size;
    int depthDepth = *depth_size;

    GrPixelFormat_t pixFmt = 0;

    int numChips = getInteger(GR_NUM_FB);
    int fsaa = getFSAA(numChips);

    if (hwext_pixext && colorDepth < 16) {
	/* Napalm_Spec.pdf, page 121
	 * There is no capability when running in 15 BPP rendering mode to
	 * selectively disable writes to only the RGB or the alpha planes - both
	 * the RGB planes and the 1-bit alpha plane are controlled together by
	 * fbzMode bit(9).
	 * Note that when running in 15 BPP rendering mode, the auxiliary buffer
	 * must be setup as a depth buffer (fbzMode bit(18)=0) and thus fbzMode
	 * bit(10) is used to enable writes to the depth buffer - fbzMode bit(9)
	 * is then used to enable writes to the 1-bit alpha plane.
	 *
	 * Unfortunately, the 15bpp mode is not reliable:
	 *	gsst.c: renderMode must be OR'ed with SST_RM_ALPHA_MSB
	 *	gglide.c: grBufferClear MUST use zaColor to fastfill 1-bit alpha
	 */
	*red_size = 5;
	*green_size = 5;
	*blue_size = 5;
	*alpha_size = 1;
	*depth_size = 16;
	*stencil_size = 0;
	switch (fsaa) {
	    case 8:
		pixFmt = GR_PIXFMT_AA_8_ARGB_1555;
		break;
	    case 4:
		pixFmt = GR_PIXFMT_AA_4_ARGB_1555;
		break;
	    case 2:
		pixFmt = GR_PIXFMT_AA_2_ARGB_1555;
		break;
	    default:
		pixFmt = GR_PIXFMT_ARGB_1555;
	}
    } else if (hwext_pixext && colorDepth > 16) {
	*red_size = 8;
	*green_size = 8;
	*blue_size = 8;
	*alpha_size = 8;
	*depth_size = 24;
	*stencil_size = 8;
	switch (fsaa) {
	    case 8:
		pixFmt = GR_PIXFMT_AA_8_ARGB_8888;
		break;
	    case 4:
		pixFmt = GR_PIXFMT_AA_4_ARGB_8888;
		break;
	    case 2:
		pixFmt = GR_PIXFMT_AA_2_ARGB_8888;
		break;
	    default:
		pixFmt = GR_PIXFMT_ARGB_8888;
	}
    } else {
	/* grBufferClear has a bug in Windowed or SDRAM alphabuffer clears
	 *	depth = (depth << 8) | depth
	 *
	 * To enable alphabuffer, we need:
	 *	grDepthMask(FXFALSE);
	 *	grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
	 * and never EVER enable depthmask
	 */
	*red_size = 5;
	*green_size = 6;
	*blue_size = 5;
	*alpha_size = depthDepth ? 0 : 8;
	*depth_size = depthDepth ? 16 : 0;
	*stencil_size = 0;
	switch (fsaa) {
	    case 8:
		pixFmt = GR_PIXFMT_AA_8_RGB_565;
		break;
	    case 4:
		pixFmt = GR_PIXFMT_AA_4_RGB_565;
		break;
	    case 2:
		pixFmt = GR_PIXFMT_AA_2_RGB_565;
		break;
	    default:
		pixFmt = GR_PIXFMT_RGB_565;
	}
    }

    return pixFmt;
}


/**
 * Initialize hardware vertex format.
 */
static void
setupGrVertexLayout (void)
{
    grReset(GR_VERTEX_PARAMETER);

    grCoordinateSpace(GR_WINDOW_COORDS);
    grVertexLayout(GR_PARAM_XY, GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);
#if FX_PACKEDCOLOR
    grVertexLayout(GR_PARAM_PARGB, GR_VERTEX_PARGB_OFFSET << 2, GR_PARAM_ENABLE);
#else  /* !FX_PACKEDCOLOR */
    grVertexLayout(GR_PARAM_RGB, GR_VERTEX_RGB_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_A, GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);
#endif /* !FX_PACKEDCOLOR */
    grVertexLayout(GR_PARAM_Q, GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Z, GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2,
		   GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2,
		   GR_PARAM_DISABLE);
    grVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2,
		   GR_PARAM_DISABLE);
    grVertexLayout(GR_PARAM_Q1, GR_VERTEX_OOW_TMU1_OFFSET << 2,
		   GR_PARAM_DISABLE);
}


int
sage_init (void)
{
    const char *str;

    if (hardware >= 0) {
	return (hardware >= GR_SSTTYPE_Voodoo4) ? 32 : 16;
    }

    grGlideInit();

    if (getInteger(GR_NUM_BOARDS) <= 0) {
	grGlideShutdown();
	return 0;
    }

#if defined(__MSDOS__)||defined(_WIN32)
    cfg_load("sage.ini");
#else
    if (cfg_load("sage.ini") != 0) {
	cfg_load("/etc/sage.ini");
    }
#endif

    grSstSelect(0);

    hardware = getHardware();

    strcpy(renderer_name, "SAGE ");
    str = grGetString(GR_RENDERER);
    strcat(renderer_name, str);
    strcat(renderer_name, " ");
    str = grGetString(GR_VERSION);
    strcat(renderer_name, str);

    str = grGetString(GR_EXTENSION);
    hwext_texuma = (strstr(str, "TEXUMA") != NULL);
    hwext_texmirror = (strstr(str, "TEXMIRROR") != NULL);
    hwext_fogcoord = (strstr(str, "FOGCOORD") != NULL);
    hwext_texfmt = (strstr(str, "TEXFMT") != NULL);
    hwext_pixext = (strstr(str, "PIXEXT") != NULL);
    hwext_combine = (strstr(str, "COMBINE") != NULL);

    gfGetRegistryOrEnvironmentStringExt = (gfGetRegistryOrEnvironmentStringExt_f) grGetProcAddress("grGetRegistryOrEnvironmentStringExt");
    gfSstWinOpenExt = (gfSstWinOpenExt_f) grGetProcAddress("grSstWinOpenExt");
    gfBufferClearExt = (gfBufferClearExt_f) grGetProcAddress("grBufferClearExt");
    gfColorMaskExt = (gfColorMaskExt_f) grGetProcAddress("grColorMaskExt");
    gfStencilFuncExt = (gfStencilFuncExt_f) grGetProcAddress("grStencilFuncExt");
    gfStencilMaskExt = (gfStencilMaskExt_f) grGetProcAddress("grStencilMaskExt");
    gfStencilOpExt = (gfStencilOpExt_f) grGetProcAddress("grStencilOpExt");
    gfColorCombineExt = (gfColorCombineExt_f) grGetProcAddress("grColorCombineExt");
    gfAlphaCombineExt = (gfAlphaCombineExt_f) grGetProcAddress("grAlphaCombineExt");
    gfTexColorCombineExt = (gfTexColorCombineExt_f) grGetProcAddress("grTexColorCombineExt");
    gfTexAlphaCombineExt = (gfTexAlphaCombineExt_f) grGetProcAddress("grTexAlphaCombineExt");
    gfConstantColorValueExt = (gfConstantColorValueExt_f) grGetProcAddress("grConstantColorValueExt");

    current = NULL;

    opengl_ver = cfg_get("3dfx.opengl.version", NULL);

    return (hardware >= GR_SSTTYPE_Voodoo4) ? 32 : 16;
}


sageContext *
sage_open (int db_flag,
	   int red_size, int green_size, int blue_size,
	   int alpha_size, int depth_size, int stencil_size)
{
    const char *str;
    sageContext *ctx;

    if (hardware < 0) {
	goto exit_error;
    }

    /* Create context */
    ctx = malloc(sizeof(sageContext));
    if (ctx == NULL) {
	goto exit_error;
    }
    ctx->gr_ctx = 0;

    /* Choose a visual */
    ctx->fmt = findPixelFormat(&red_size, &green_size, &blue_size,
			       &alpha_size, &depth_size, &stencil_size);
    fb_color = red_size + green_size + blue_size;
    fb_alpha = alpha_size;

    /* Initialize the core */
    if (ctx_init(db_flag,
		 red_size, green_size, blue_size,
		 alpha_size, depth_size, stencil_size) != 0) {
	goto exit_error1;
    }

    /* Set driver capabilities */
    allow_texuma      = hwext_texuma && !YES("3dfx.disable.texuma");
    allow_texmirror   = hwext_texmirror && !YES("3dfx.disable.texmirror");
    allow_fogcoord    = hwext_fogcoord && !YES("3dfx.disable.fogcoord");
    allow_32bpt       = hwext_texfmt && !YES("3dfx.disable.32bpt");
#if 0 /* FS: only check if we have the ARGB_8888 & co, like Mesa. */
    if (fb_color != 24) allow_32bpt = 0;
#endif
    allow_blendsquare = (hardware >= GR_SSTTYPE_Voodoo4) && !YES("3dfx.disable.blendsquare");
    allow_combine     = GL_TRUE && !YES("3dfx.disable.combine");
    allow_multitex    = (getInteger(GR_NUM_TMU) > 1) && !YES("3dfx.disable.multitex");

    allow_compressed = atoi(cfg_get("3dfx.texture.compression", "3"));
    if (hardware < GR_SSTTYPE_Voodoo4) {
	allow_compressed &= ~2;
    }
    if (!ctx_texcodec) {
	allow_compressed &= ~1;
    }

    if (hardware < GR_SSTTYPE_Voodoo4) {
	allow_combine = -allow_combine;
    }

    /* Update core with driver capabilities */
    ctx_const_max_texture_size = getInteger(GR_MAX_TEXTURE_SIZE);
    ctx_const_max_texture_units = allow_multitex ? 2 : 1;

    max_texture_levels = 0;
    while ((GLuint)(1 << max_texture_levels) < ctx_const_max_texture_size) {
	max_texture_levels++;
    }
    ctx_const_max_lod_bias = max_texture_levels;

    str = cfg_get("3dfx.maxlod", NULL);
    if (str != NULL) {
	ctx_const_max_texture_size = 1 << atoi(str);
    }

    /* Finish driver setup */
    tex_bound_mask = (hardware < GR_SSTTYPE_Banshee) ? 0x1fffff : -1U;

    /** FS: moved tm_init() to sage_bind(), see comments in there. **/

    if (vb_init() != 0) {
	goto exit_error2;
    }

    fogtable = malloc(getInteger(GR_FOG_TABLE_ENTRIES) * sizeof(GrFog_t));
    if (fogtable == NULL) {
	goto exit_error3;
    }

    /* XXX getInteger(GR_GLIDE_STATE_SIZE) */

    drv_multipass = drv_multipass_none;
    init_tri_pointers();

    /* Finish core setup */
    ext_set("GL_EXT_texture_env_add", GL_TRUE);
    ext_set("GL_EXT_texture_edge_clamp", GL_TRUE);
    ext_set("GL_EXT_compiled_vertex_array", GL_TRUE);
    ext_set("GL_EXT_blend_func_separate", GL_TRUE);
    ext_set("GL_EXT_texture_lod_bias", GL_TRUE);
    if (allow_multitex) {
	ext_set("GL_ARB_multitexture", GL_TRUE);
    }
    if (allow_fogcoord) {
	ext_set("GL_EXT_fog_coord", GL_TRUE);
    }
    if (allow_texmirror) {
	ext_set("GL_ARB_texture_mirrored_repeat", GL_TRUE);
    }
    if (stencil_size) {
	ext_set("GL_EXT_stencil_wrap", GL_TRUE);
    }
    if (allow_blendsquare) {
	ext_set("GL_NV_blend_square", GL_TRUE);
    }
    if (allow_combine) {
	ext_set("GL_ARB_texture_env_combine", GL_TRUE);
	ext_set("GL_EXT_texture_env_combine", GL_TRUE);
    }
    if (allow_compressed) {
	ext_set("GL_ARB_texture_compression", GL_TRUE);
	ext_set("GL_EXT_texture_compression_s3tc", GL_TRUE);
	ext_set("GL_3DFX_texture_compression_FXT1", GL_TRUE);
	ext_set("GL_S3_s3tc", GL_TRUE);
    }
    /* XXX not really */
    ext_set("GL_EXT_separate_specular_color", GL_TRUE);
    ext_set("GL_EXT_secondary_color", GL_TRUE);

    return ctx;

  exit_error3:
    free_a(vb);
  exit_error2:
    ctx_fini();
  exit_error1:
    free(ctx);
  exit_error:
    return NULL;
}


int
sage_bind (sageContext *ctx, void *win, int width, int height)
{
    if (hardware < 0) {
	return -1;
    }

    if (ctx == NULL || win == NULL) {
	if (current != NULL) {
	    /* unbind */
	    grSstWinClose(current->gr_ctx);
	    current = NULL;
	}
	return 0;
    }
#ifdef __MSDOS__
    win = NULL; /* DOS: no window handle. */
#endif
    if (ctx == current) {
	if (win == current->drawable) {
	    /* nop */
	    return 0;
	}
	/* XXX can't rebind */
	return -1;
    }
    if (ctx->gr_ctx != 0) {
	/* XXX can't rebind */
	return -1;
    }

    if (hwext_pixext) {
	ctx->gr_ctx = gfSstWinOpenExt((FxU32)win,
				findBestRes(width, height),
				GR_REFRESH_60Hz,
				GR_COLORFORMAT_ABGR,
				GR_ORIGIN_LOWER_LEFT,
				ctx->fmt, 2, 1);
    } else if (ctx->fmt == GR_PIXFMT_RGB_565) {
	ctx->gr_ctx = grSstWinOpen((FxU32)win,
				findBestRes(width, height),
				GR_REFRESH_60Hz,
				GR_COLORFORMAT_ABGR,
				GR_ORIGIN_LOWER_LEFT, 2, 1);
    }
    if (ctx->gr_ctx == 0) {
	return -1;
    }

    /* FS: This has to be moved here, otherwise grTexMaxAddress() will fail
     * on Windows, because context needs creating from grSstWinOpen() first.
     * See GLIDE_CHECK_CONTEXT in glide3x. */
    if (tm_init() != 0) {
	return -1;
    }
    ctx->drawable = win;
    current = ctx;

    /* XXX implement Glide{Get|Set}State */
    setupGrVertexLayout();
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
    drv_setupDepth();
    drv_setupTexture();

    getGeometry(&screen_width, &screen_height);

    GLCALL(Viewport)(0, 0, width, height);

    /* XXX grDisable(GR_PASSTHRU); */
    return 0;
}


void
sage_shut (sageContext *ctx)
{
    if (hardware < 0) {
	return;
    }
    if (ctx == NULL) {
	return;
    }

    sage_bind(NULL, NULL, 0, 0);

    free(fogtable);
    free_a(vb);
    ctx_fini();
    tm_fini();
    free(ctx);
}


void
sage_fini (void)
{
    if (hardware < 0) {
	return;
    }

    if (current != NULL) {
	sage_shut(current);
    }

    cfg_kill();
    grGlideShutdown();

    hardware = -1;
}


void
sage_swap (int interval)
{
    GLCALL(Flush)();
    grBufferSwap(interval);
}


#ifdef __MSDOS__
static struct {
    const char *name;
    SageProc proc;
} functab[] = {
#define ALIAS(x, y) \
    { "gl" #x #y, (SageProc)gl##x },
#include "../../main/alias.h"
    { "glLockArraysEXT", (SageProc)glLockArraysEXT },
    { "glUnlockArraysEXT", (SageProc)glUnlockArraysEXT },
    { "glPolygonOffsetEXT", (SageProc)glPolygonOffsetEXT }
};

SageProc
sage_GetProcAddress (const char *procname)
{
    const int n = sizeof(functab) / sizeof(functab[0]);
    int i;
    for (i = 0; i < n; i++) {
	if (!strcmp(procname, functab[i].name)) {
	    return functab[i].proc;
	}
    }
    return NULL;
}
#endif

#ifdef X86
/* validate structure sizes and member offsets in x86/drv.inc: */
#include "drvchk.h"
#endif
