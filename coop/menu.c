/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * In-game menu, ripped from CTF game module.
 *
 * =======================================================================
 */

#include "g_local.h"

void PMenu_Do_Scrolling_Update(edict_t *ent);

/* Note that the pmenu entries are duplicated 
   this is so that a static set of pmenu entries can be used 
   for multiple clients and changed without interference 
   note that arg will be freed when the menu is closed, it
    must be allocated memory */
pmenuhnd_t *
PMenu_Open(edict_t *ent, pmenu_t *entries, pmenu_t *header, int cur, int num, int numheader, void *arg, int menutype)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;
	int i;

	if (!ent->client)
	{
		return NULL;
	}

	if (ent->client->menu)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "warning, ent already has a menu\n");
		PMenu_Close(ent);
	}

	hnd = malloc(sizeof(*hnd));
	if (!hnd)
	{
		gi.error("PMenu_Open:  Failed allocating memory.\n");
		return NULL;
	}

	hnd->arg = arg;
	hnd->entries = malloc(sizeof(pmenu_t) * num);
	if (!hnd->entries)
	{
		gi.error("PMenu_Open:  Failed allocating memory.\n");
		free(hnd);
		return NULL;
	}
	hnd->menutype = menutype;
	memcpy(hnd->entries, entries, sizeof(pmenu_t) * num);

	if (header && numheader) /* FS */
	{
		hnd->header = malloc(sizeof(pmenu_t) * numheader);
		if (!hnd->header)
		{
			gi.error("PMenu_Open:  Failed allocating memory.\n");
			free(hnd->entries);
			free(hnd);
			return NULL;
		}
		memcpy(hnd->header, header, sizeof(pmenu_t) * numheader);

		for (i = 0; i < numheader; i++)
		{
			if (header[i].text)
			{
				hnd->header[i].text = strdup(header[i].text);
			}
		}
	}
	else
	{
		hnd->header = NULL;
	}

	/* duplicate the strings since they may be from static memory */
	for (i = 0; i < num; i++)
	{
		if (entries[i].text)
		{
			hnd->entries[i].text = strdup(entries[i].text);
		}
	}

	hnd->num = num;
	hnd->numheader = numheader; /* FS */

	if ((cur < 0) || !entries[cur].SelectFunc)
	{
		for (i = 0, p = entries; i < num; i++, p++)
		{
			if (p->SelectFunc)
			{
				break;
			}
		}
	}
	else
	{
		i = cur;
	}

	if (i >= num)
	{
		hnd->cur = -1;
	}
	else
	{
		hnd->cur = i;
	}

	ent->client->showscores = true;
	ent->client->inmenu = true;
	ent->client->menu = hnd;
	ent->client->menu_update = NULL; /* FS: Added */

	PMenu_Do_Update(ent);
	gi.unicast(ent, true);

	return hnd;
}

void
PMenu_Close(edict_t *ent)
{
	int i;
	pmenuhnd_t *hnd;

	if (!ent->client->menu)
	{
		return;
	}

	hnd = ent->client->menu;

	for (i = 0; i < hnd->num; i++)
	{
		if (hnd->entries[i].text)
		{
			free(hnd->entries[i].text);
		}
	}

	free(hnd->entries);

	if (hnd->header) /* FS */
	{
		for (i = 0; i < hnd->numheader; i++)
		{
			if (hnd->header[i].text)
			{
				free(hnd->header[i].text);
			}
		}

		free(hnd->header);
	}

	if (hnd->arg)
	{
		free(hnd->arg);
	}

	free(hnd);
	ent->client->menu = NULL;
	ent->client->menu_update = NULL; /* FS: Added */
	ent->client->showscores = false;
}

/* 
 * Only use on pmenu's that have been called with PMenu_Open 
 */
void
PMenu_UpdateEntry(pmenu_t *entry, const char *text, int align,
		SelectFunc_t SelectFunc)
{
	if (entry->text)
	{
		free(entry->text);
	}

	entry->text = strdup(text);
	entry->align = align;
	entry->SelectFunc = SelectFunc;
}

void
PMenu_Do_Update(edict_t *ent)
{
	char string[LAYOUT_MAX_LENGTH];
	int i;
	pmenu_t *p;
	int x;
	pmenuhnd_t *hnd;
	char *t;
	qboolean alt = false;

	if (!ent->client->menu)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "warning:  ent has no menu\n");
		return;
	}

	if(ent->client->menu_update) /* FS: Added for dynamically updating menus */
	{
		ent->client->menu_update(ent);
		if(!ent->client->menu) /* FS: Forced close in the update function.  Done here. */
		{
			return;
		}
	}

	hnd = ent->client->menu;
	if(hnd->menutype == PMENU_SCROLLING) /* FS: Special stuff for scrolling menu */
	{
		PMenu_Do_Scrolling_Update(ent);
		return;
	}

	strcpy(string, "xv 32 yv 8 picn inventory ");

	for (i = 0, p = hnd->entries; i < hnd->num; i++, p++)
	{
		if (!p->text || !*(p->text))
		{
			continue; /* blank line */
		}

		t = p->text;

		if (*t == '*')
		{
			alt = true;
			t++;
		}

		sprintf(string + strlen(string), "yv %d ", 32 + i * 8);

		if (p->align == PMENU_ALIGN_CENTER)
		{
			x = 196 / 2 - strlen(t) * 4 + 64;
		}
		else if (p->align == PMENU_ALIGN_RIGHT)
		{
			x = 64 + (196 - strlen(t) * 8);
		}
		else
		{
			x = 64;
		}

		sprintf(string + strlen(string), "xv %d ", x - ((hnd->cur == i) ? 8 : 0));

		if (hnd->cur == i)
		{
			sprintf(string + strlen(string), "string2 \"\x0d%s\" ", t);
		}
		else if (alt)
		{
			sprintf(string + strlen(string), "string2 \"%s\" ", t);
		}
		else
		{
			sprintf(string + strlen(string), "string \"%s\" ", t);
		}

		alt = false;
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

void
PMenu_Update(edict_t *ent)
{
	if (!ent->client->menu)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "warning:  ent has no menu\n");
		return;
	}

	if (level.time - ent->client->menutime >= 1.0)
	{
		/* been a second or more since last update, update now */
		PMenu_Do_Update(ent);
		gi.unicast(ent, true);
		ent->client->menutime = level.time;
		ent->client->menudirty = false;
	}

	ent->client->menutime = level.time + 0.2;
	ent->client->menudirty = true;
}

void
PMenu_Next(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
	{
		return; /* no selectable entries */
	}

	i = hnd->cur;
	p = hnd->entries + hnd->cur;

	do
	{
		i++, p++;

		if (i == hnd->num)
		{
			i = 0, p = hnd->entries;
		}

		if (p->SelectFunc)
		{
			break;
		}
	}
	while (i != hnd->cur);

	hnd->cur = i;

	PMenu_Update(ent);
}

void
PMenu_Prev(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
	{
		return; /* no selectable entries */
	}

	i = hnd->cur;
	p = hnd->entries + hnd->cur;

	do
	{
		if (i == 0)
		{
			i = hnd->num - 1;
			p = hnd->entries + i;
		}
		else
		{
			i--, p--;
		}

		if (p->SelectFunc)
		{
			break;
		}
	}
	while (i != hnd->cur);

	hnd->cur = i;

	PMenu_Update(ent);
}

void
PMenu_Select(edict_t *ent)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;

	if (!ent->client->menu)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
	{
		return; /* no selectable entries */
	}

	p = hnd->entries + hnd->cur;

	if (p->SelectFunc)
	{
		p->SelectFunc(ent, hnd);
	}
}

void
PMenu_Do_Scrolling_Update(edict_t *ent) /* FS */
{
	char string[LAYOUT_MAX_LENGTH];
	int i, z, pos, scroll_lines;
	pmenu_t *p;
	int x;
	pmenuhnd_t *hnd;
	char *t;
	qboolean alt = false;

	if (!ent->client->menu)
	{
		gi.dprintf(DEVELOPER_MSG_GAME, "warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	strcpy(string, "xv 32 yv 8 picn inventory ");

	if (hnd->header && hnd->numheader)
	{
		for (i = 0, p = hnd->header; i < hnd->numheader; i++, p++)
		{
			if (!p->text || !*(p->text))
			{
				continue; /* blank line */
			}

			t = p->text;

			if (*t == '*')
			{
				alt = true;
				t++;
			}

			sprintf(string + strlen(string), "yv %d ", 32 + i * 8);

			if (p->align == PMENU_ALIGN_CENTER)
			{
				x = 196 / 2 - strlen(t) * 4 + 64;
			}
			else if (p->align == PMENU_ALIGN_RIGHT)
			{
				x = 64 + (196 - strlen(t) * 8);
			}
			else
			{
				x = 64;
			}

			sprintf(string + strlen(string), "xv %d ", x);

			if (alt)
			{
				sprintf(string + strlen(string), "string2 \"%s\" ", t);
			}
			else
			{
				sprintf(string + strlen(string), "string \"%s\" ", t);
			}

			alt = false;
		}
	}

	scroll_lines = 18;
	if (hnd->numheader)
	{
		scroll_lines = 18-hnd->numheader;
	}
	pos = hnd->cur % scroll_lines;

	for (z = 0, p = hnd->entries; z < hnd->cur - pos; z++)
	{
		p++;
	}

	for (i = 0; i < hnd->num; i++, p++)
	{
		if (i >= scroll_lines)
		{
			break;
		}

		if(hnd->cur >= hnd->num)
		{
			break;
		}

		if(i + (hnd->cur-pos) >= hnd->num)
		{
			break;
		}

		if (!p->text || !*(p->text))
		{
			continue; /* blank line */
		}

		t = p->text;

		if (*t == '*')
		{
			alt = true;
			t++;
		}

		sprintf(string + strlen(string), "yv %d ", 32 + (i+hnd->numheader) * 8);

		if (p->align == PMENU_ALIGN_CENTER)
		{
			x = 196 / 2 - strlen(t) * 4 + 64;
		}
		else if (p->align == PMENU_ALIGN_RIGHT)
		{
			x = 64 + (196 - strlen(t) * 8);
		}
		else
		{
			x = 64;
		}

		sprintf(string + strlen(string), "xv %d ", x - ((pos == i) ? 8 : 0));

		if (pos == i)
		{
			sprintf(string + strlen(string), "string2 \"\x0d%s\" ", t);
		}
		else if (alt)
		{
			sprintf(string + strlen(string), "string2 \"%s\" ", t);
		}
		else
		{
			sprintf(string + strlen(string), "string \"%s\" ", t);
		}

		alt = false;
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}
