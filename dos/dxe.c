/* Dynamic module loading/unloading with DJGPP DXE3
 * Copyright (C) 2015 Q2DOS developers.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <dlfcn.h>
#include <sys/dxe.h>

#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <dir.h>
#include <io.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <dos.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <sys/movedata.h>
#include <setjmp.h>
#include <crt0.h>
#include <string.h>
#include <float.h> /* __dj_float_epsilon */
#include <ctype.h>
#if 1 /* for mesa w/o 3dfx glide */
#include <stubinfo.h>
#include <sys/exceptn.h>
#endif
#ifndef REF_HARD_LINKED /* for ref_soft */
#include "dosisms.h"
#endif
#if defined(GAMESPY) && !defined(GAMESPY_HARD_LINKED) /* for watt-32 symbols */
#include <tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "../qcommon/qcommon.h"

DXE_EXPORT_TABLE (syms)
	/* dlfcn */
	DXE_EXPORT (dlclose)
	DXE_EXPORT (dlopen)
	DXE_EXPORT (dlsym)

	/* assert */
	DXE_EXPORT (__dj_assert)
	/* errno */
	DXE_EXPORT (errno)
	/* setjmp */
	DXE_EXPORT (longjmp)
	DXE_EXPORT (setjmp)
	/* signal */
	DXE_EXPORT (signal)

	/* stdlib */
	DXE_EXPORT (abort)
	DXE_EXPORT (exit)
	DXE_EXPORT (rand)
	DXE_EXPORT (srand)
	DXE_EXPORT (bsearch)
	DXE_EXPORT (atol)
	DXE_EXPORT (strtod)
	DXE_EXPORT (strtol)
	DXE_EXPORT (strtoul)
	DXE_EXPORT (qsort)
	DXE_EXPORT (getenv)
	DXE_EXPORT (putenv)
	DXE_EXPORT (malloc)
	DXE_EXPORT (calloc)
	DXE_EXPORT (realloc)
	DXE_EXPORT (free)

	/* string */
	DXE_EXPORT (memcmp)
	DXE_EXPORT (memcpy)
	DXE_EXPORT (memset)
	DXE_EXPORT (memmove)
	DXE_EXPORT (strcat)
	DXE_EXPORT (strncat)
	DXE_EXPORT (memchr)
	DXE_EXPORT (strchr)
	DXE_EXPORT (strcmp)
	DXE_EXPORT (strcpy)
	DXE_EXPORT (strdup)
	DXE_EXPORT (strlen)
	DXE_EXPORT (strncmp)
	DXE_EXPORT (strncpy)
	DXE_EXPORT (strrchr)
	DXE_EXPORT (strstr)
	DXE_EXPORT (strcspn)
	DXE_EXPORT (strtok)
#if 0
	DXE_EXPORT (strtok_r)
#endif
	DXE_EXPORT (strcasecmp)
	DXE_EXPORT (strncasecmp)
	DXE_EXPORT (stricmp)
	DXE_EXPORT (strnicmp)
	DXE_EXPORT (strlwr)
	DXE_EXPORT (strupr)
	DXE_EXPORT (stpcpy)

	/* stdio */
	DXE_EXPORT (__dj_stderr)
	DXE_EXPORT (__dj_stdout)
	DXE_EXPORT (fopen)
	DXE_EXPORT (freopen)
	DXE_EXPORT (fclose)
	DXE_EXPORT (fflush)
	DXE_EXPORT (fread)
	DXE_EXPORT (fwrite)
	DXE_EXPORT (fseek)
	DXE_EXPORT (ftell)
	DXE_EXPORT (feof)
	DXE_EXPORT (getc)
	DXE_EXPORT (ungetc)
	DXE_EXPORT (fgetc)
	DXE_EXPORT (fgets)
	DXE_EXPORT (fputc)
	DXE_EXPORT (fputs)
	DXE_EXPORT (putc)
	DXE_EXPORT (putchar)
	DXE_EXPORT (puts)
	DXE_EXPORT (fprintf)
	DXE_EXPORT (printf)
	DXE_EXPORT (sprintf)
	DXE_EXPORT (vsprintf)
	DXE_EXPORT (vsnprintf)
	DXE_EXPORT (vfprintf)
	DXE_EXPORT (fscanf)
	DXE_EXPORT (sscanf)

	/* dir */
	DXE_EXPORT (findfirst)
	DXE_EXPORT (findnext)
	/* sys/stat */
	DXE_EXPORT (mkdir)
	/* unistd */
	DXE_EXPORT (usleep)
	DXE_EXPORT (write)
	/* time */
	DXE_EXPORT (clock)
	DXE_EXPORT (uclock)
	DXE_EXPORT (time)
	DXE_EXPORT (gettimeofday)
	DXE_EXPORT (localtime)
	DXE_EXPORT (asctime)
	DXE_EXPORT (strftime)

	/* ctype */
	DXE_EXPORT (__dj_ctype_tolower)
	DXE_EXPORT (__dj_ctype_toupper)
	DXE_EXPORT (__dj_ctype_flags)
	DXE_EXPORT (tolower)
	DXE_EXPORT (toupper)

	/* math */
	DXE_EXPORT (__dj_huge_val)
	/* FLT_EPSILON may be defined as
	 * __dj_float_epsilon */
	DXE_EXPORT (__dj_float_epsilon)
	DXE_EXPORT (acos)
	DXE_EXPORT (asin)
	DXE_EXPORT (atan)
	DXE_EXPORT (atan2)
	DXE_EXPORT (atof)
	DXE_EXPORT (atoi)
	DXE_EXPORT (ceil)
	DXE_EXPORT (sin)
	DXE_EXPORT (cos)
	DXE_EXPORT (tan)
	DXE_EXPORT (floor)
	DXE_EXPORT (sqrt)
	DXE_EXPORT (log)
	DXE_EXPORT (pow)
	DXE_EXPORT (exp)
	DXE_EXPORT (frexp)
	DXE_EXPORT (ldexp)
	DXE_EXPORT (fmod)

	/* crt0 */
	DXE_EXPORT (_crt0_startup_flags)
	/* nearptr */
	DXE_EXPORT (__djgpp_base_address)
	DXE_EXPORT (__djgpp_nearptr_enable)
	DXE_EXPORT (__djgpp_nearptr_disable)
	/* movedata */
	DXE_EXPORT (dosmemput)
	DXE_EXPORT (movedata)
	/* dos */
	DXE_EXPORT (enable)
	DXE_EXPORT (disable)
	DXE_EXPORT (int86)

#ifndef REF_HARD_LINKED
	/* dpmi */
	DXE_EXPORT (__dpmi_int)
	DXE_EXPORT (__dpmi_physical_address_mapping)
	DXE_EXPORT (__dpmi_free_physical_address_mapping)

#if 1 /* for mesa w/o 3dfx glide */
	DXE_EXPORT (__dpmi_allocate_ldt_descriptors)
	DXE_EXPORT (__dpmi_free_ldt_descriptor)
	DXE_EXPORT (__dpmi_get_segment_base_address)
	DXE_EXPORT (__dpmi_set_segment_base_address)
	DXE_EXPORT (__dpmi_set_segment_limit)
	/* stubinfo.h, exceptn.h */
	DXE_EXPORT (_stubinfo)
	DXE_EXPORT (__djgpp_dos_sel)
#endif

	/* dos_v2.c functions for ref_soft */
	DXE_EXPORT (dos_getmemory)
	DXE_EXPORT (dos_freememory)
	DXE_EXPORT (ptr2real)
	DXE_EXPORT (real2ptr)
#endif /* ! REF_HARD_LINKED */

#if defined(GAMESPY) && !defined(GAMESPY_HARD_LINKED) /* watt-32 library symbols */
	DXE_EXPORT(send)
	DXE_EXPORT(sendto)
	DXE_EXPORT(socket)
	DXE_EXPORT(inet_ntoa)
	DXE_EXPORT(recv)
	DXE_EXPORT(recvfrom)
	DXE_EXPORT(gethostbyname)
	DXE_EXPORT(connect)
	DXE_EXPORT(setsockopt)
	DXE_EXPORT(ioctlsocket)
	DXE_EXPORT(closesocket)
	DXE_EXPORT(select_s)
	DXE_EXPORT(inet_addr)
#endif
DXE_EXPORT_END


static int num_unres;

static int dxe_fail ()
{
	return 0;
}

static void *dxe_res (const char *sym)
{
	FILE *f = fopen ("dxe.log", "a");
	fprintf (f, "%s: unresolved symbol.\n", sym);
	fflush (f);
	fclose (f);
	++num_unres;
	return (void *)dxe_fail;
}

void Sys_InitDXE3 (void)
{
	remove ("dxe.log");

	/* Register the symbols exported into dynamic modules */
	dlregsym (syms);
}

void *Sys_dlopen (const char *filename, qboolean globalmode)
{
	void *lib;

	_dlsymresolver = dxe_res;
	num_unres = 0;
	lib = dlopen (filename, (globalmode)? RTLD_GLOBAL : 0);
	_dlsymresolver = NULL;
	if (num_unres)
		Sys_Error ("Unresolved symbol(s) in %s. See DXE.LOG for details.", filename);

	return lib;
}

void *Sys_dlsym (void *handle, const char *symbol)
{
	return dlsym (handle, symbol);
}

int Sys_dlclose (void *handle)
{
	return dlclose (handle);
}

#ifndef GAME_HARD_LINKED
static void *game_library;

void Sys_UnloadGame (void)
{
	if (game_library)
		dlclose (game_library);
	game_library = NULL;
}

void *Sys_GetGameAPI (void *parms)
{
	void	*(*GetGameAPI) (void *);
	char	name[MAX_OSPATH];
	char	curpath[MAX_OSPATH];
	char	*path;
	const char *gamename = "gamex86.dxe";

	getcwd(curpath, sizeof(curpath));
	if (game_library)
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadGame");

	Com_Printf("------- Loading %s -------\n", gamename);

	/* now run through the search paths */
	for (path = NULL; ; )
	{
		path = FS_NextPath (path);
		if (!path) return NULL; /* couldn't find one anywhere */

		Com_sprintf (name, sizeof(name), "%s/%s/%s", curpath, path, gamename);
		game_library = Sys_dlopen (name, false);
		if (game_library)
		{
			Com_Printf ("Loaded %s\n", name);
			break;
		}
	}

	GetGameAPI = (void *) dlsym (game_library, "_GetGameAPI");
	if (!GetGameAPI)
	{
		Sys_UnloadGame ();
		Com_Printf("dlsym() failed on %s\n", gamename);
		return NULL;
	}
	return GetGameAPI (parms);
}
#endif /* GAME_HARD_LINKED */

#if defined(GAMESPY) && !defined(GAMESPY_HARD_LINKED)
static void *gamespy_library;

void *Sys_GetGameSpyAPI(void *parms)
{
	const char *dxename = "gamespy.dxe";
	char name[MAX_OSPATH];
	char curpath[MAX_OSPATH];
	void	*(*GetGameSpyAPI) (void *);

	getcwd(curpath, sizeof(curpath));

	Com_Printf("------- Loading %s -------\n", dxename);

	Com_sprintf(name, sizeof(name), "%s/%s", curpath, dxename);
	gamespy_library = Sys_dlopen (name, false);
	if (!gamespy_library)
		return NULL;

	GetGameSpyAPI = (void *) dlsym (gamespy_library, "_GetGameSpyAPI");
	if (!GetGameSpyAPI)
	{
		dlclose(gamespy_library);
		Com_Printf("dlsym() failed on %s\n", dxename);
		gamespy_library = NULL;
		return NULL;
	}

	return GetGameSpyAPI (parms);
}

void Sys_UnloadGameSpy(void)
{
	if (gamespy_library)
		dlclose(gamespy_library);
	gamespy_library = NULL;
}
#endif /* GAMESPY ... */
