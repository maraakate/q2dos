// Knightmare 5/27/12- header for screen scaling code
#ifndef __SCRNSCALE_H
#define __SCRNSCALE_H

#define SCR_VIRTUAL_WIDTH	640.0f
#define SCR_VIRTUAL_HEIGHT	480.0f

typedef enum
{
	ALIGN_UNSET = 0,
	ALIGN_STRETCH,
	ALIGN_STRETCH_ALL,
	ALIGN_CENTER,
	ALIGN_LETTERBOX,
	ALIGN_TOP,
	ALIGN_BOTTOM,
	ALIGN_RIGHT,
	ALIGN_LEFT,
	ALIGN_TOPRIGHT,
	ALIGN_TOPLEFT,
	ALIGN_BOTTOMRIGHT,
	ALIGN_BOTTOMLEFT,
	ALIGN_TOP_STRETCH,
	ALIGN_BOTTOM_STRETCH
} scralign_t;

typedef enum
{
	HUDTYPE_NONE = 0,
	HUDTYPE_HUD,
	HUDTYPE_CROSSHAIR
} scrscaletype_t;

void	SCR_ScaleCoords (viddef_t vid_def, float *xPos, float *yPos, float *width, float *height, scralign_t align, scrscaletype_t hudtype);
void	SCR_InitScale (viddef_t vid_def, scrscaletype_t hudtype);
float	SCR_GetHudScale (viddef_t vid_def);
float	SCR_GetCrosshairScale (viddef_t vid_def); /* FS */
int	SCR_GetMinHudSize (viddef_t vid_def);
int	SCR_GetNumHudSizes (void);

extern const scrnscale_t SCALEZERO;

#endif	// __SCRNSCALE_H
