#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for mremap() */
#endif
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <string.h>
#include <ctype.h>

#ifdef __FreeBSD__
#include <machine/param.h>
#endif

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#include "glob.h"
#include "qcommon.h"

//===============================================================================

static byte *membase;
static int maxhunksize;
static int curhunksize;

void *Hunk_Begin (int maxsize)
{
	// reserve a huge chunk of memory, but don't commit any yet
	maxhunksize = maxsize + sizeof(int);
	curhunksize = 0;
	membase = (byte *) mmap(NULL, maxhunksize, PROT_READ|PROT_WRITE,
				MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (membase == MAP_FAILED)
		Sys_Error("unable to virtual allocate %d bytes", maxsize);

	*((int *)membase) = curhunksize;

	return membase + sizeof(int);
}

void *Hunk_Alloc (int size)
{
	byte *buf;

	// round to cacheline
	size = (size+31)&~31;
	if (curhunksize + size > maxhunksize)
		Sys_Error("Hunk_Alloc overflow");
	buf = membase + sizeof(int) + curhunksize;
	curhunksize += size;
	return buf;
}

int Hunk_End (void)
{
	byte *n = NULL;

#if defined(__linux__)
	n = (byte *)mremap(membase, maxhunksize, curhunksize + sizeof(int), 0);
#elif defined(__FreeBSD__)
	size_t old_size = maxhunksize;
	size_t new_size = curhunksize + sizeof(int);
	void *unmap_base;
	size_t unmap_len;

	new_size = round_page(new_size);
	old_size = round_page(old_size);

	if (new_size > old_size)
	{
		n = NULL; /* error */
	}
	else if (new_size < old_size)
	{
		unmap_base = (caddr_t)(membase + new_size);
		unmap_len = old_size - new_size;
		n = munmap(unmap_base, unmap_len) + membase;
	}

#else
	size_t old_size = maxhunksize;
	size_t new_size = curhunksize + sizeof(int);
	void *unmap_base;
	size_t unmap_len;
	static size_t page_size = 0;
 #ifndef round_page
 #define round_page(x) (((size_t)(x) + (page_size - 1)) / page_size) * page_size
 #endif
	if (!page_size) {
		long sz = sysconf(_SC_PAGESIZE);
		if (sz < 1) Sys_Error("Hunk_End: failed getting pagesize");
		page_size = (size_t) sz;
	}

	new_size = round_page(new_size);
	old_size = round_page(old_size);

	if (new_size > old_size)
	{
		n = NULL; /* error */
	}
	else if (new_size < old_size)
	{
		unmap_base = (caddr_t)(membase + new_size);
		unmap_len = old_size - new_size;
		n = munmap(unmap_base, unmap_len) + membase;
	}
#endif

	if (n != membase)
		Sys_Error("Hunk_End: Could not remap virtual block (%d)", errno);
	*((int *)membase) = curhunksize + sizeof(int);

	return curhunksize;
}

void Hunk_Free (void *base)
{
	byte *m;

	if (base == (void *)(-1L)) return;
	if (base) {
		m = ((byte *)base) - sizeof(int);
		if (munmap(m, *((int *)m)))
			Sys_Error("Hunk_Free: munmap failed (%d)", errno);
	}
}

/*
================
Sys_Milliseconds
================
*/
int curtime;
int Sys_Milliseconds (void)
{
	struct timeval tp;
	static time_t secbase;

	gettimeofday(&tp, NULL);

	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	curtime = (tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000;

	return curtime;
}

//===============================================================================

void Sys_Mkdir (char *path)
{
	mkdir (path, 0777);
}

//============================================

static	char	findbase[PATH_MAX];
static	char	findpath[PATH_MAX];
static	char	findpattern[PATH_MAX];
static	DIR		*fdir;

static qboolean CompareAttributes(const char *path, const char *name,
				  unsigned musthave, unsigned canthave)
{
	struct stat st;
	char fn[PATH_MAX];

// . and .. never match
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return false;

	Com_sprintf(fn, sizeof(fn), "%s/%s", path, name);
	if (stat(fn, &st) == -1)
		return false; // shouldn't happen

	if ((st.st_mode & S_IFDIR) && (canthave & SFF_SUBDIR))
		return false;

	if ((musthave & SFF_SUBDIR) && !(st.st_mode & S_IFDIR))
		return false;

	return true;
}

char *Sys_FindFirst (char *path, unsigned musthave, unsigned canhave)
{
	struct dirent *d;
	char *p;

//	if (fdir)
//		Sys_Error ("Sys_BeginFind without close");

//	COM_FilePath (path, findbase);
	strcpy(findbase, path);

	if ((p = strrchr(findbase, '/')) != NULL) {
		*p = 0;
		strcpy(findpattern, p + 1);
	} else
		strcpy(findpattern, "*");

	if (strcmp(findpattern, "*.*") == 0)
		strcpy(findpattern, "*");
	
	if ((fdir = opendir(findbase)) == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				Com_sprintf(findpath, sizeof(findpath), "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

char *Sys_FindNext (unsigned musthave, unsigned canhave)
{
	struct dirent *d;

	if (fdir == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				Com_sprintf(findpath, sizeof(findpath), "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

void Sys_FindClose (void)
{
	if (fdir != NULL)
		closedir(fdir);
	fdir = NULL;
}
