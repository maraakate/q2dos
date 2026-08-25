// Knightmare- code file for screen scaling

#ifndef REF_DLL
#include "client.h"
#endif

viddef_t hudscale_list[] =
{
	{ -1, -1 },
	{ 1400, 1050 },
	{ 1280, 960 },
	{ 1152, 864 },
	{ 1024, 768 },
	{ 960, 720 },
	{ 800, 600 },
	{ 720, 540 },
	{ 640, 480 }
};

#define HUDSCALE_NUM_SIZES ( sizeof(hudscale_list) / sizeof(hudscale_list[0]) )

float		scr_hudscale;
float		scr_crosshairscale;
float		scr_screenAspect;
int			scr_minHudSize;

const scrnscale_t SCALEZERO = { 0 };

void SCR_InitScale (viddef_t vid_def, scrscaletype_t hudtype)
{
	float	real_width, real_height;
	float	refWidth, refHeight;
	int		sizeIndex, scaleSize;
	float *scale = &scr_hudscale;

	switch (hudtype)
	{
		case HUDTYPE_CROSSHAIR:
			scale = &scr_crosshairscale;
			scaleSize = scr_crosshairsize->intValue;
			break;
		default:
			scale = &scr_hudscale;
			scaleSize = scr_hudsize->intValue;
			break;
	}

	real_width = vid_def.width;
	real_height = vid_def.height;
	sizeIndex = bound_max_inline(bound_min_inline(scaleSize, 0), HUDSCALE_NUM_SIZES - 1);

	if (sizeIndex == 0)
	{
		refWidth = real_width;
		refHeight = real_height;
	}
	else
	{
		refWidth = hudscale_list[sizeIndex].width;
		refHeight = hudscale_list[sizeIndex].height;
	}

	if (real_width > refWidth && real_height > refHeight)
		*scale = min_inline((real_width / refWidth), (real_height / refHeight));
	else
		*scale = 1.0f;

	scr_screenAspect = (float)vid_def.width / (float)vid_def.height;
}

float SCR_GetHudScale (viddef_t vid_def)
{
	if (scr_hudsize->modified)
	{
		SCR_InitScale (vid_def, HUDTYPE_NONE);
		scr_hudsize->modified = false;
	}

	return scr_hudscale;
}

float SCR_GetCrosshairScale (viddef_t vid_def)
{
	if (scr_crosshairsize->modified)
	{
		SCR_InitScale(vid_def, HUDTYPE_CROSSHAIR);
		scr_crosshairsize->modified = false;
	}

	return scr_crosshairscale;
}

int SCR_GetMinHudSize (viddef_t vid_def)
{
	int	i, out = 0;

	for (i = 1; i < HUDSCALE_NUM_SIZES; i++)
	{
		if ((hudscale_list[i].width < vid_def.width) && (hudscale_list[i].height < vid_def.height))
		{
			out = i;
			break;
		}
	}
	return out;
}

int SCR_GetNumHudSizes (void)
{
	return HUDSCALE_NUM_SIZES;
}

void SCR_ScaleCoords (viddef_t vid_def, float *xPos, float *yPos, float *width, float *height, scralign_t align, scrscaletype_t hudtype)
{
	float	real_width, real_height;
	float	xScale, lb_xScale, yScale, minScale, vertScale;
	float	tmp_x, tmp_y, tmp_w, tmp_h;
	float	xLeft, xRight;

	if (scr_hudsize->modified)
	{
		SCR_InitScale(vid_def, HUDTYPE_NONE);
		scr_hudsize->modified = false;
	}

	if (scr_crosshairsize->modified)
	{
		SCR_InitScale(vid_def, HUDTYPE_CROSSHAIR);
		scr_crosshairsize->modified = false;
	}

	real_width = (float)vid_def.width;
	real_height = (float)vid_def.height;

	xLeft = 0.0f;
	xRight = real_width;
	xScale = real_width / SCR_VIRTUAL_WIDTH;

	lb_xScale = real_width / SCR_VIRTUAL_WIDTH;
	yScale = real_height / SCR_VIRTUAL_HEIGHT;

	if (hudtype == HUDTYPE_NONE)
	{
		minScale = min_inline(xScale, yScale);
	}
	else
	{
		switch (hudtype)
		{
			case HUDTYPE_CROSSHAIR:
				minScale = scr_crosshairscale;
				break;
			default:
				minScale = scr_hudscale;
				break;
		}
		// hud items are never stretched
		if (align == ALIGN_LETTERBOX || align == ALIGN_TOP_STRETCH
			|| align == ALIGN_BOTTOM_STRETCH || align == ALIGN_STRETCH)
		{
			align = ALIGN_CENTER;
		}
	}

	// aspect-ratio independent scaling
	switch (align)
	{
		case ALIGN_CENTER:
			if (width)
				*width *= minScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = (tmp_x - (0.5 * SCR_VIRTUAL_WIDTH)) * minScale + (0.5 * real_width);
			}
			if (yPos)
			{
				tmp_y = *yPos;
				*yPos = (tmp_y - (0.5 * SCR_VIRTUAL_HEIGHT)) * minScale + (0.5 * real_height);
			}
			break;
		case ALIGN_LETTERBOX:
			// special case: video mode (eyefinity?) is wider than object
			if (width != NULL && height != NULL && ((real_width / real_height) > *width / *height))
			{
				tmp_h = *height;
				vertScale = real_height / tmp_h;
				if (xPos != NULL)
				{
					tmp_x = *xPos;
					tmp_w = *width;
					*xPos = tmp_x * lb_xScale - (0.5 * (tmp_w * vertScale - tmp_w * lb_xScale));
				}

				if (yPos)
					*yPos = 0;

				*width *= vertScale;
				*height *= vertScale;
			}
			else
			{
				if (xPos)
					*xPos *= xScale;
				if (yPos != NULL && height != NULL)
				{
					tmp_y = *yPos;
					tmp_h = *height;
					*yPos = tmp_y * yScale - (0.5 * (tmp_h * xScale - tmp_h * yScale));
				}
				if (width)
					*width *= xScale;
				if (height)
					*height *= xScale;
			}
			break;
		case ALIGN_TOP:
			if (width)
				*width *= minScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = (tmp_x - (0.5 * SCR_VIRTUAL_WIDTH)) * minScale + (0.5 * real_width);
			}
			if (yPos)
				*yPos *= minScale;
			break;
		case ALIGN_BOTTOM:
			if (width)
				*width *= minScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = (tmp_x - (0.5 * SCR_VIRTUAL_WIDTH)) * minScale + (0.5 * real_width);
			}
			if (yPos)
			{
				tmp_y = *yPos;
				*yPos = (tmp_y - SCR_VIRTUAL_HEIGHT) * minScale + real_height;
			}
			break;
		case ALIGN_RIGHT:
			if (width)
				*width *= minScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = (tmp_x - SCR_VIRTUAL_WIDTH) * minScale + xRight;
			}
			if (yPos)
			{
				tmp_y = *yPos;
				*yPos = (tmp_y - (0.5 * SCR_VIRTUAL_HEIGHT)) * minScale + (0.5 * real_height);
			}
			break;
		case ALIGN_LEFT:
			if (width)
				*width *= minScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = tmp_x * minScale + xLeft;
			}
			if (yPos)
			{
				tmp_y = *yPos;
				*yPos = (tmp_y - (0.5 * SCR_VIRTUAL_HEIGHT)) * minScale + (0.5 * real_height);
			}
			break;
		case ALIGN_TOPRIGHT:
			if (width)
				*width *= minScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = (tmp_x - SCR_VIRTUAL_WIDTH) * minScale + xRight;
			}
			if (yPos)
				*yPos *= minScale;
			break;
		case ALIGN_TOPLEFT:
			if (width)
				*width *= minScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = tmp_x * minScale + xLeft;
			}
			if (yPos)
				*yPos *= minScale;
			break;
		case ALIGN_BOTTOMRIGHT:
			if (width)
				*width *= minScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = (tmp_x - SCR_VIRTUAL_WIDTH) * minScale + xRight;
			}
			if (yPos)
			{
				tmp_y = *yPos;
				*yPos = (tmp_y - SCR_VIRTUAL_HEIGHT) * minScale + real_height;
			}
			break;
		case ALIGN_BOTTOMLEFT:
			if (width)
				*width *= minScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = tmp_x * minScale + xLeft;
			}
			if (yPos)
			{
				tmp_y = *yPos;
				*yPos = (tmp_y - SCR_VIRTUAL_HEIGHT) * minScale + real_height;
			}
			break;
		case ALIGN_TOP_STRETCH:
			if (width)
				*width *= xScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos *= tmp_x * xScale + xLeft;
			}
			if (yPos)
				*yPos *= minScale;
			break;
		case ALIGN_BOTTOM_STRETCH:
			if (width)
				*width *= xScale;
			if (height)
				*height *= minScale;
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = tmp_x * xScale + xLeft;
			}
			if (yPos)
			{
				tmp_y = *yPos;
				*yPos = (tmp_y - SCR_VIRTUAL_HEIGHT) * minScale + real_height;
			}
			break;
		case ALIGN_STRETCH_ALL:
			if (xPos)
				*xPos *= lb_xScale;
			if (yPos)
				*yPos *= yScale;
			if (width)
				*width *= lb_xScale;
			if (height)
				*height *= yScale;
			break;
		case ALIGN_STRETCH:
		default:
			if (xPos)
			{
				tmp_x = *xPos;
				*xPos = tmp_x * xScale + xLeft;
			}
			if (yPos)
				*yPos *= yScale;
			if (width)
				*width *= xScale;
			if (height)
				*height *= yScale;
			break;
	}
}
