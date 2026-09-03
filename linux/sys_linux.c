#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef __FreeBSD__
#include <sys/sysctl.h>
#endif
#include <dlfcn.h>

#ifdef SDL_CLIENT
#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif
#endif

#include "qcommon.h"

#ifdef __APPLE__
#include "sys_osx.h"
#endif

#ifndef DEDICATED_ONLY
#include "rw_linux.h" /* for KBD_ function pointers */
#endif

cvar_t *nostdout;

unsigned	sys_frame_time;

qboolean stdin_active = true;

static char exe_dir[MAX_OSPATH];
static char pref_dir[MAX_OSPATH];

// =======================================================================
// General routines
// =======================================================================

void Sys_ConsoleOutput (char *string)
{
	if (nostdout && nostdout->value)
		return;

	fputs(string, stdout);
	fflush (stdout);
}

void Sys_Printf (const char *fmt, ...)
{
	va_list		argptr;
	char		text[2048];
	unsigned char		*p;

	if (nostdout && nostdout->value)
		return;

	va_start (argptr,fmt);
	vsnprintf(text, sizeof(text), fmt, argptr);
	va_end (argptr);

	for (p = (unsigned char *)text; *p; p++) {
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
	}
}

void Sys_Quit (void)
{
	CL_Shutdown ();
	Qcommon_Shutdown ();
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
	exit (0);
}

void Sys_Init(void)
{
}

#if !defined(Sys_ErrorMessage)
#define Sys_ErrorMessage(T)	do {} while (0)
#endif

void Sys_Error (const char *error, ...)
{
	va_list argptr;
	char string[2048];

	CL_Shutdown ();
	Qcommon_Shutdown ();

// TODO: DG: why change stdin wen we just wanna print to stderr?
// change stdin to non blocking
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);

	va_start (argptr,error);
	vsnprintf(string, sizeof(string), error, argptr);
	va_end (argptr);

	fprintf(stderr, "Error: %s\n", string);
	Sys_ErrorMessage(string);

	exit (1);
}


void floating_point_exception_handler(int whatever)
{
//	Sys_Warn("floating point exception\n");
	signal(SIGFPE, floating_point_exception_handler);
}

char *Sys_ConsoleInput(void)
{
	static char text[256];
	int     len;
	fd_set	fdset;
	struct timeval timeout;

	if (!dedicated || !dedicated->value)
		return NULL;

	if (!stdin_active)
		return NULL;

	FD_ZERO(&fdset);
	FD_SET(0, &fdset); // stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if (select (1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		return NULL;

	len = read (0, text, sizeof(text));
	if (len == 0) { // eof!
		stdin_active = false;
		return NULL;
	}
	if (len < 1)
		return NULL;
	text[len-1] = 0; // rip off the /n and terminate

	return text;
}

/*****************************************************************************/

void Sys_AppActivate (void)
{
}

void Sys_SendKeyEvents (void)
{
#ifndef DEDICATED_ONLY
	if (KBD_Update_fp)
		KBD_Update_fp();
#endif
	// grab frame time 
	sys_frame_time = Sys_Milliseconds();
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//	Sys_GetGameAPI
//	
//	Loads the game dll
///////////////////////////////////////////////////////////////////////////////

#ifdef GAME_HARD_LINKED
void *GetGameAPI (void *import);
void	Sys_UnloadGame (void)
{
}
void	*Sys_GetGameAPI (void *parms)
{
	return GetGameAPI (parms);
}
#else
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
	char	*path;
#ifdef __APPLE__
	const char *gamename = "game.dylib";
#else
	const char *gamename = "game.so";
#endif

	if (game_library)
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadGame");

	Com_Printf("------- Loading %s -------\n", gamename);

	/* now run through the search paths */
	for (path = NULL; ; )
	{
		path = FS_NextPath (path);
		if (!path) return NULL; /* couldn't find one anywhere */

		Com_sprintf (name, sizeof(name), "%s/%s", path, gamename);
		game_library = dlopen (name, RTLD_LAZY);
		if (game_library)
		{
			Com_Printf ("Loaded %s\n", name);
			break;
		}
	}

	GetGameAPI = (void* (*)(void*)) dlsym (game_library, "GetGameAPI");
	if (!GetGameAPI)
	{
		Sys_UnloadGame ();
		Com_Printf("dlsym() failed on %s\n", gamename);
		return NULL;
	}
	return GetGameAPI (parms);
}
#endif /* GAME_HARD_LINKED */

#ifdef GAMESPY
#ifdef GAMESPY_HARD_LINKED
void *GetGameSpyAPI (void *import); /* need prototype. */
void *Sys_GetGameSpyAPI(void *parms)
{
	return GetGameSpyAPI (parms);
}
void Sys_UnloadGameSpy(void)
{
}
#else /* dynamic linking */
static void *gamespy_library;

void *Sys_GetGameSpyAPI(void *parms)
{
	#ifdef __APPLE__
	const char *soname = "gamespy.dylib";
	#else
	const char *soname = "gamespy.so";
	#endif
	char	name[MAX_OSPATH];
	char	*path;
	void	*(*GetGameSpyAPI) (void *);

	Com_Printf("------- Loading %s -------\n", soname);

	path = Cvar_Get ("basedir", ".", CVAR_NOSET)->string;
	Com_sprintf(name, sizeof(name), "%s/%s", path, soname);
	gamespy_library = dlopen (name, false);
	if (!gamespy_library)
		return NULL;

	GetGameSpyAPI = (void *) dlsym (gamespy_library, "_GetGameSpyAPI");
	if (!GetGameSpyAPI)
	{
		dlclose(gamespy_library);
		Com_Printf("dlsym() failed on %s\n", soname);
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
#endif
#endif /* GAMESPY */

void Sys_CopyProtect(void)
{
}


//=======================================================================

void Sys_Sleep (unsigned ms)
{
	usleep(ms * 1000);
}

const char* Sys_ExeDir(void)
{
	return exe_dir;
}

const char* Sys_PrefDir(void)
{
	return pref_dir;
}

static void Init_ExeDir(const char* argv0)
{
	char buf[MAX_OSPATH] = {0};
	const char* lastSlash;
	size_t len;

#ifdef __linux__
	readlink("/proc/self/exe", buf, MAX_OSPATH-1);
#elif defined(__FreeBSD__)
	static int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
	size_t buflen = sizeof(buf) - 1;
	sysctl(mib, sizeof(mib)/sizeof(*mib), buf, &buflen, NULL, 0);
#endif

	if (!*buf)
	{
		printf("WARNING: Couldn't get path to executable, reading from argv[0]!\n");
		if (strlen(argv0) < sizeof(buf)) {
			strcpy(buf, argv0);
		}
		else {
			buf[0] = '\0';
		}
	}

	memset(exe_dir, 0, sizeof(exe_dir));
	// starting at the last slash the executable name begins - we only want the path up to there
	lastSlash = strrchr(buf, '/');
	len = lastSlash ? (lastSlash - buf) : 0;
	if(lastSlash == NULL || len >= sizeof(exe_dir) || len == 0)
	{
		printf("WARNING: Couldn't get path to executable! Defaulting to \".\"!\n");
		sprintf(exe_dir, ".");
	}
	else
	{
		memcpy(exe_dir, buf, len);
	}
}

static void Init_PrefDir()
{
	char *pp = getenv("XDG_DATA_HOME");

	memset(pref_dir, 0, sizeof(pref_dir));

	if(pp == NULL)
	{
		snprintf(pref_dir, sizeof(pref_dir), "%s/.local/share/quake2", getenv("HOME"));
		return;
	}

	if(strlen(pp) >= sizeof(pref_dir) - 1)
	{
		printf("WARNING: $XDG_DATA_HOME contains a too long path, defaulting to installation dir!\n");
		strcpy(pref_dir, exe_dir);
		return;
	}

	strcpy(pref_dir, pp);
}

//=======================================================================

#ifdef SDL_CLIENT
static void Sys_AtExit (void)
{
	SDL_Quit();
}
#endif

int main (int argc, char **argv)
{
	int time, oldtime, newtime, frametime;

	Init_ExeDir(argv[0]);
	Init_PrefDir();
#ifdef SDL_CLIENT
	if (SDL_Init(0) < 0)
		Sys_Error("SDL failed to initialize.");
	atexit (Sys_AtExit);
#endif

	Qcommon_Init(argc, argv);

	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
	nostdout = Cvar_Get ("nostdout", "0", 0);
	if (!nostdout->value)
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

	oldtime = Sys_Milliseconds ();
	while (1)
	{
		do {
			newtime = Sys_Milliseconds();
			time = newtime - oldtime;
		} while (time < 1);

		Qcommon_Frame (time);
		oldtime = newtime;

		// DG: sleeping to get 8ms frames in MP (to not cause 100% load) should be enough
		if (dedicated && dedicated->value) {
			frametime = Sys_Milliseconds () - newtime;
			if (frametime < 8)
				Sys_Sleep (8 - frametime);
		}
	}

	return 0; /* NOT REACHED */
}
