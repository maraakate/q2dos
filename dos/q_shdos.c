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

#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h> /* mkdir() */
#include <errno.h>
#include <stdio.h>
#include <dos.h>
#include <dir.h>
#include <io.h>
#include <sys/time.h>

#ifndef PATH_MAX
#define PATH_MAX 260
#endif

#include "../qcommon/qcommon.h"

static byte	*membase;
static int	maxhunksize;
static int	curhunksize;

void	*Hunk_Begin (int maxsize)
{
	/* reserve a huge chunk of memory, but don't commit any yet */
	maxhunksize = maxsize;
	curhunksize = 0;
	membase = (byte *)malloc (maxhunksize);
	if (!membase)
		Sys_Error ("VirtualAlloc reserve failed %d bytes",maxsize);

	memset (membase, 0, maxsize);
	return (void *)membase;
}

void	*Hunk_Alloc (int size)
{
	/* round to cacheline */
	size = (size+31)&~31;

	curhunksize += size;
	if (curhunksize > maxhunksize)
		Sys_Error ("Hunk_Alloc overflow");

	return (void *)(membase+curhunksize-size);
}

void	Hunk_Free (void *buf)
{
	free (buf);
}

int	Hunk_End (void)
{
/* for realloc() to be useful here: you either need DJGPP-2.05 or newer,
 * or you need to replace malloc() & friends in any older DJGPP version
 * with nmalloc() as in DJGPP-2.05. */
	byte *n = (byte *)realloc(membase, curhunksize);
	if (n != membase)
		Sys_Error("Hunk_End:  Could not remap virtual block (%d)", errno);

	return curhunksize;
}

int	curtime;
int	Sys_Milliseconds (void)
{
	curtime = (double) uclock() / (UCLOCKS_PER_SEC / 1000.0);
	return curtime;
}

int	Sys_DOSTime (void) /* FS: DOS needs this for random qport */
{
	static time_t secbase;
	struct timeval tp;

	gettimeofday(&tp, NULL);
	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000;
	}
	return (tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000;
}

void	Sys_Mkdir (char *path)
{
	mkdir (path, 0777);
}

static	struct ffblk	finddata;
static	int	findhandle = -1;
static	char	findbase[PATH_MAX];
static	char	findpath[PATH_MAX];

static qboolean CompareAttributes(const struct ffblk *ff,
				  unsigned musthave, unsigned canthave)
{
	/* . and .. never match */
	if (strcmp(ff->ff_name, ".") == 0 || strcmp(ff->ff_name, "..") == 0)
		return false;

	if (ff->ff_attrib & _A_VOLID) /* shouldn't happen */
		return false;

	if (ff->ff_attrib & _A_SUBDIR) {
		if (canthave & SFF_SUBDIR)
			return false;
	}
	else {
		if (musthave & SFF_SUBDIR)
			return false;
	}

	if (ff->ff_attrib & _A_RDONLY) {
		if (canthave & SFF_RDONLY)
			return false;
	}
	else {
		if (musthave & SFF_RDONLY)
			return false;
	}

	if (ff->ff_attrib & _A_HIDDEN) {
		if (canthave & SFF_HIDDEN)
			return false;
	}
	else {
		if (musthave & SFF_HIDDEN)
			return false;
	}

	if (ff->ff_attrib & _A_SYSTEM) {
		if (canthave & SFF_SYSTEM)
			return false;
	}
	else {
		if (musthave & SFF_SYSTEM)
			return false;
	}

	return true;
}

char *Sys_FindFirst (char *path, unsigned musthave, unsigned canthave)
{
	int attribs;

	if (findhandle == 0)
		Sys_Error ("Sys_BeginFind without close");

	COM_FilePath (path, findbase);
	memset (&finddata, 0, sizeof(finddata));

	attribs = FA_ARCH|FA_RDONLY;
	if (!(canthave & SFF_SUBDIR))
		attribs |= FA_DIREC;
	if (musthave & SFF_HIDDEN)
		attribs |= FA_HIDDEN;
	if (musthave & SFF_SYSTEM)
		attribs |= FA_SYSTEM;

	findhandle = findfirst(path, &finddata, attribs);
	if (findhandle != 0)
		return NULL;
	if (CompareAttributes(&finddata, musthave, canthave)) {
		Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, finddata.ff_name);
		return findpath;
	}
	return Sys_FindNext(musthave, canthave);
}

char *Sys_FindNext (unsigned musthave, unsigned canthave)
{
	if (findhandle != 0)
		return NULL;

	while (findnext(&finddata) == 0) {
		if (CompareAttributes(&finddata, musthave, canthave)) {
			Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, finddata.ff_name);
			return findpath;
		}
	}

	return NULL;
}

void Sys_FindClose (void)
{
	findhandle = -1;
}
