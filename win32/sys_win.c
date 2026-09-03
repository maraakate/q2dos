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
// sys_win.h

#include "../qcommon/qcommon.h"
#include "winquake.h"
#include "service.h"
#include "resource.h"
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

#define MINIMUM_WIN_MEMORY	0x0a00000
#define MAXIMUM_WIN_MEMORY	0x1000000

//#define DEMO

qboolean s_win95;

int			starttime;
qboolean	ActiveApp;
qboolean	Minimized;

#ifndef NEW_DED_CONSOLE
static HANDLE		hinput, houtput;
#endif

unsigned int	sys_msg_time;
unsigned int	sys_frame_time;

int		argc;
char	*argv[MAX_NUM_ARGVS];

char cmdline[4096] = { "\0" };

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

// Knightmare- replaced by new dedicated console
#ifndef NEW_DED_CONSOLE
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	CL_Shutdown ();
	Qcommon_Shutdown ();

	va_start (argptr, error);
	_vsnprintf(text, sizeof(text), error, argptr);
	va_end (argptr);
	text[sizeof(text)-1] = 0;

	MessageBoxA(NULL, text, "Error", 0 /* MB_OK */);

	exit (1);
}
#endif	// NEW_DED_CONSOLE

void Sys_Quit (void)
{
	timeEndPeriod(1);

	CL_Shutdown();
	Qcommon_Shutdown ();
// Knightmare added- new dedicated console
#ifdef NEW_DED_CONSOLE
	Sys_ShutdownConsole ();
#else
	if (dedicated && dedicated->value)
		FreeConsole ();
#endif

	Cvar_Shutdown(); /* FS: Free our CVAR memory too */

#ifdef DEDICATED_ONLY
	if (!bRunningAsService)
#endif
		exit (0);
}


void WinError (void)
{
	LPVOID lpMsgBuf;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPSTR) &lpMsgBuf,
		0,
		NULL
	);

	// Display the string.
	MessageBoxA(NULL, (const char *)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION);

	// Free the buffer.
	LocalFree(lpMsgBuf);
}

/*
================
Sys_ScanForCD

================
*/
char *Sys_ScanForCD (void)
{
	static char	cddir[MAX_OSPATH];
	static qboolean	done;
#ifndef DEMO
	char		drive[4];
	FILE		*f;
	char		test[MAX_QPATH];

	if (done)		// don't re-check
		return cddir;

	// no abort/retry/fail errors
	SetErrorMode (SEM_FAILCRITICALERRORS);

	drive[0] = 'c';
	drive[1] = ':';
	drive[2] = '\\';
	drive[3] = 0;

	done = true;

	// scan the drives
	for (drive[0] = 'c' ; drive[0] <= 'z' ; drive[0]++)
	{
		// where activision put the stuff...
		sprintf (cddir, "%sinstall\\data", drive);
		sprintf (test, "%sinstall\\data\\quake2.exe", drive);
		f = fopen(test, "r");
		if (f)
		{
			fclose (f);
			if (GetDriveTypeA(drive) == DRIVE_CDROM)
				return cddir;
		}
	}
#endif

	cddir[0] = 0;

	return NULL;
}

/*
================
Sys_CopyProtect

================
*/
void Sys_CopyProtect (void)
{
#ifndef DEMO
	char	*cddir;

	cddir = Sys_ScanForCD();
	if (!cddir || !cddir[0])
		Com_Error (ERR_FATAL, "You must have the Quake2 CD in the drive to play.");
#endif
}


/*
================
Sys_Init
================
*/
void Sys_Init (void)
{
	OSVERSIONINFO	vinfo;

	timeBeginPeriod(1);

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if (!GetVersionEx (&vinfo))
		Sys_Error ("Couldn't get OS info");

	if (vinfo.dwMajorVersion < 4)
		Sys_Error ("Quake2 requires windows version 4 or greater");
	if (vinfo.dwPlatformId == VER_PLATFORM_WIN32s)
		Sys_Error ("Quake2 doesn't run on Win32s");
	else if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		s_win95 = true;

// Knightmare- removed for new dedicated console
#ifndef NEW_DED_CONSOLE
	if (dedicated->value)
	{
		if (!AllocConsole ())
			Sys_Error ("Couldn't create dedicated server console");
		hinput = GetStdHandle (STD_INPUT_HANDLE);
		houtput = GetStdHandle (STD_OUTPUT_HANDLE);
	}
#endif	// NEW_DED_CONSOLE
// end Knightmare
}

// Knightmare- removed for new dedicated console
#ifndef NEW_DED_CONSOLE
static char	console_text[256];
static int	console_textlen;

/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
	INPUT_RECORD	recs[1024];
	int		ch;
	DWORD		dummy, numread, numevents;

	if (!dedicated || !dedicated->value)
		return NULL;

	for ( ;; )
	{
		if (!GetNumberOfConsoleInputEvents (hinput, &numevents))
			Sys_Error ("Error getting # of console events");

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			Sys_Error ("Error reading console input");

		if (numread != 1)
			Sys_Error ("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
					case '\r':
						WriteFile(houtput, "\r\n", 2, &dummy, NULL);

						if (console_textlen)
						{
							console_text[console_textlen] = 0;
							console_textlen = 0;
							return console_text;
						}
						break;

					case '\b':
						if (console_textlen)
						{
							console_textlen--;
							WriteFile(houtput, "\b \b", 3, &dummy, NULL);
						}
						break;

					default:
						if (ch >= ' ')
						{
							if (console_textlen < sizeof(console_text)-2)
							{
								WriteFile(houtput, &ch, 1, &dummy, NULL);
								console_text[console_textlen] = ch;
								console_textlen++;
							}
						}

						break;
				}
			}
		}
	}

	return NULL;
}


/*
================
Sys_ConsoleOutput

Print text to the dedicated console
================
*/
void Sys_ConsoleOutput (char *string)
{
	DWORD dummy;
	char text[256];

	if (!dedicated || !dedicated->value)
		return;

	if (console_textlen)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen+1] = '\r';
		text[console_textlen+2] = 0;
		WriteFile(houtput, text, console_textlen+2, &dummy, NULL);
	}

	WriteFile(houtput, string, strlen(string), &dummy, NULL);

	if (console_textlen)
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);
}
#endif	// NEW_DED_CONSOLE
// end Knightmare

/*
================
Sys_Sleep

Knightmare- added this to fix CPU usage
================
*/
void Sys_Sleep (unsigned msec)
{
	Sleep (msec);
}

/*
================
Sys_SendKeyEvents

Send Key_Event calls
================
*/
void Sys_SendKeyEvents (void)
{
	MSG		msg;

	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage (&msg, NULL, 0, 0))
			Sys_Quit ();
		sys_msg_time = msg.time;
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}

	// grab frame time
	sys_frame_time = timeGetTime();	// FIXME: should this be at start?
}

/*
================
Sys_GetClipboardData

================
*/
char *Sys_GetClipboardData (void)
{
	char *data = NULL;
	char *cliptext;

	if (OpenClipboard(NULL) != 0)
	{
		HANDLE hClipboardData;

		if ((hClipboardData = GetClipboardData(CF_TEXT)) != NULL)
		{
			if ((cliptext = (char *)GlobalLock(hClipboardData)) != NULL)
			{
				data = (char *)malloc(GlobalSize(hClipboardData) + 1);
				if (!data)
				{
					Sys_Error("Sys_GetClipboardData:  Failed to allocate memory.\n");
					return NULL;
				}

				strcpy(data, cliptext);
				GlobalUnlock(hClipboardData);
			}
		}
		CloseClipboard();
	}
	return data;
}


/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/

/*
=================
Sys_AppActivate
=================
*/
void Sys_AppActivate (void)
{
#ifndef DEDICATED_ONLY
	ShowWindow (cl_hwnd, SW_RESTORE);
	SetForegroundWindow (cl_hwnd);
#endif
}

/*
========================================================================

GAME DLL

========================================================================
*/

static HINSTANCE	game_library;

/*
=================
Sys_UnloadGame
=================
*/
void Sys_UnloadGame (void)
{
#ifndef GAME_HARD_LINKED
	if (!FreeLibrary (game_library))
		Com_Error (ERR_FATAL, "FreeLibrary failed for game library");
#endif
	game_library = NULL;
}

/*
=================
Sys_GetGameAPI

Loads the game dll
=================
*/
#ifdef GAME_HARD_LINKED
void *GetGameAPI (void *import);
#endif	//static game needs a real prototype

void *Sys_GetGameAPI (void *parms)
{
#ifndef GAME_HARD_LINKED
	void	*(*GetGameAPI) (void *);
	char	name[MAX_OSPATH];
	char	*path;
	char	cwd[MAX_OSPATH];

#if defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__)
	const char *gamename = "gamex64.dll";
	#ifdef NDEBUG
	const char *debugdir = "releasex64";
	#else
	const char *debugdir = "debugx64";
	#endif
#elif defined(_M_IX86) || defined(__i386__)
	const char *gamename = "gamex86.dll";
	#ifdef NDEBUG
	const char *debugdir = "release";
	#else
	const char *debugdir = "debug";
	#endif
#elif defined _M_ALPHA
	const char *gamename = "gameaxp.dll";
	#ifdef NDEBUG
	const char *debugdir = "releaseaxp";
	#else
	const char *debugdir = "debugaxp";
	#endif
#endif

	if (game_library)
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadGame");

	// check the current debug directory first for development purposes
	_getcwd (cwd, sizeof(cwd));
	Com_sprintf (name, sizeof(name), "%s/%s/%s", cwd, debugdir, gamename);
	game_library = LoadLibraryA(name);
	if (game_library)
	{
		Com_DPrintf(DEVELOPER_MSG_STANDARD, "LoadLibrary (%s)\n", name);
	}
	else
	{
		// check the current directory for other development purposes
		Com_sprintf (name, sizeof(name), "%s/%s", cwd, gamename);
		game_library = LoadLibraryA(name);
		if (game_library)
		{
			Com_DPrintf(DEVELOPER_MSG_STANDARD, "LoadLibrary (%s)\n", name);
		}
		else
		{
			// now run through the search paths
			path = NULL;
			while (1)
			{
				path = FS_NextPath (path);
				if (!path)
					return NULL;		// couldn't find one anywhere
				Com_sprintf (name, sizeof(name), "%s/%s", path, gamename);
				game_library = LoadLibraryA(name);
				if (game_library)
				{
					Com_DPrintf(DEVELOPER_MSG_STANDARD, "LoadLibrary (%s)\n",name);
					break;
				}
			}
		}
	}

	GetGameAPI = (void* (*)(void*))GetProcAddress (game_library, "GetGameAPI");
	if (!GetGameAPI)
	{
		Sys_UnloadGame ();
		return NULL;
	}
#endif	/* ! GAME_HARD_LINKED */

	return GetGameAPI (parms);
}

//=======================================================================


/*
==================
ParseCommandLine

==================
*/
void ParseCommandLine (LPSTR lpCmdLine)
{
	static char exe[] = "exe";
	argc = 1;
	argv[0] = exe;

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
		}
	}
}

typedef enum Q2_PROCESS_DPI_AWARENESS {
	Q2_PROCESS_DPI_UNAWARE = 0,
	Q2_PROCESS_SYSTEM_DPI_AWARE = 1,
	Q2_PROCESS_PER_MONITOR_DPI_AWARE = 2
} Q2_PROCESS_DPI_AWARENESS;

static void Sys_SetHighDPIMode(void)
{
	HINSTANCE shcoreDLL, userDLL;

	/* For Vista, Win7 and Win8 */
	BOOL(WINAPI *SetProcessDPIAware)(void) = NULL;

	/* Win8.1 and later */
	HRESULT(WINAPI *SetProcessDpiAwareness)(Q2_PROCESS_DPI_AWARENESS dpiAwareness) = NULL;

	userDLL = LoadLibraryA("USER32.DLL");

	if (userDLL)
	{
		SetProcessDPIAware = (BOOL(WINAPI *)(void)) GetProcAddress(userDLL,
				"SetProcessDPIAware");
	}


	shcoreDLL = LoadLibraryA("SHCORE.DLL");

	if (shcoreDLL)
	{
		SetProcessDpiAwareness = (HRESULT(WINAPI *)(Q2_PROCESS_DPI_AWARENESS))
			GetProcAddress(shcoreDLL, "SetProcessDpiAwareness");
	}


	if (SetProcessDpiAwareness) {
		SetProcessDpiAwareness(Q2_PROCESS_PER_MONITOR_DPI_AWARE);
	}
	else if (SetProcessDPIAware) {
		SetProcessDPIAware();
	}
}

void Detect_WinNT (void) /* FS: Detect if we're using Windows XP for alt+tab appcompat */
{
	DWORD WinVersion;
	DWORD WinLowByte, WinHiByte;
	DWORD dwDisp, dwDisp2;
	DWORD dwAppCompat = 1;
	HKEY hk, hk2;

	WinVersion = GetVersion();
	WinLowByte = (DWORD)(LOBYTE(LOWORD(WinVersion)));
	WinHiByte = (DWORD)(HIBYTE(HIWORD(WinVersion)));

	if (WinLowByte > 4)
	{
		Sys_SetHighDPIMode();

		if (WinLowByte == 5 && WinHiByte > 0) /* FS: Because Win2k is version 5.0 and XP is 5.1 and 2003 is 5.2 */
		{
			RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, &dwDisp);
			RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Custom", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk2, &dwDisp2);
			RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\InstalledSDB", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk2, &dwDisp2);

			RegSetValueEx(hk, "{b1899c0f-fdfd-42d0-b489-c254bdbb539d}", 0, REG_DWORD, (const byte *) &dwAppCompat, sizeof(DWORD));
			RegCloseKey(hk);
			Com_DPrintf(DEVELOPER_MSG_STANDARD, "Windows NT Version: %u\n", (unsigned int)WinLowByte);
		}

		if (WinLowByte == 6) /* FS: Windows 7 */
		{
			dwAppCompat = 119; /* FS: Different value for updated AppCompat */
			RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, &dwDisp);
			RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Custom", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk2, &dwDisp2);
			RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\InstalledSDB", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk2, &dwDisp2);

			RegSetValueEx(hk, "{b1899c0f-fdfd-42d0-b489-c254bdbb539d}", 0, REG_DWORD, (const byte *) &dwAppCompat, sizeof(DWORD));
			RegCloseKey(hk);
			Com_DPrintf(DEVELOPER_MSG_STANDARD, "Windows NT Version: %u\n", (unsigned int)WinLowByte);
		}
	}
}

static void WinParseEarlyParms (void)
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (argv[i])
		{
#ifdef DEDICATED_ONLY
			if (!strnicmp(argv[i], "-cwd", 4) && argv[i+1])
			{
				if (_chdir(argv[i+1]))
				{
					switch (errno)
					{
						case ENOENT:
							printf( "Unable to locate the directory: %s\n", argv[i+1] );
							break;
						case EINVAL:
							printf( "Invalid buffer.\n");
							break;
						default:
							printf( "Unknown error.\n");
					}
				}
				else
				{
					char buff[250];
					_getcwd(buff, sizeof(buff)-1);
					printf("CWD set to %s\n", buff);
				}
			}
#endif
		}
	}
}

/*
==================
WinMain
==================
*/
HINSTANCE	global_hInstance;
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG			msg;
	int				time, oldtime, newtime;
	char			*cddir;

	/* previous instances do not exist in Win32 */
	if (hPrevInstance)
		return 0;

	if (hInstance)
		strncpy (cmdline, lpCmdLine, sizeof(cmdline) - 1);

	global_hInstance = hInstance;

	Detect_WinNT(); /* FS */

	ParseCommandLine (lpCmdLine);
	WinParseEarlyParms();

//hInstance is empty when we are back here with service code
#ifdef DEDICATED_ONLY
	if (hInstance && argc > 1)
	{
		if (!strcmp(argv[1], "-service"))
		{
			bRunningAsService = true;
			return main();
		}
	}
#endif

// Knightmare added- new dedicated console
#ifdef NEW_DED_CONSOLE
	Sys_InitDedConsole ();
	Com_Printf ("Q2DOS for Windows %4.2f %s %s %s\n", VERSION, CPUSTRING, __DATE__, BUILDSTRING);
#endif
// end Knightmare

	// if we find the CD, add a +set cddir xxx command line
	cddir = Sys_ScanForCD ();
	if (cddir && argc < MAX_NUM_ARGVS - 3)
	{
		int i;

		// don't override a cddir on the command line
		for (i=0 ; i<argc ; i++)
			if (!strcmp(argv[i], "cddir"))
				break;

		if (i == argc)
		{
			static char arg_set[] = "+set";
			static char arg_cddir[] = "cddir";
			argv[argc++] = arg_set;
			argv[argc++] = arg_cddir;
			argv[argc++] = cddir;
		}
	}

	Qcommon_Init (argc, argv);

	oldtime = Sys_Milliseconds ();

    /* main window message loop */
	while (1)
	{
		// if at a full screen console, don't update unless needed
		if (Minimized || (dedicated && dedicated->value))
		{
			Sleep (1);
		}

		while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage (&msg, NULL, 0, 0))
				Com_Quit ();
			sys_msg_time = msg.time;
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}

		do
		{
			newtime = Sys_Milliseconds ();
			time = newtime - oldtime;
		} while (time < 1);
//			Con_Printf ("time:%5.2f - %5.2f = %5.2f\n", newtime, oldtime, time);
#if defined(_M_IX86) || defined(__i386__)
		_controlfp(_PC_24, _MCW_PC);
#endif
		Qcommon_Frame (time);
		oldtime = newtime;
#ifdef DEDICATED_ONLY
		Sys_DedConsoleCheckHostname();
#endif
	}

	// never gets here
	return TRUE;
}

//===============================================================================
#ifdef GAMESPY
#ifndef GAMESPY_HARD_LINKED /* dynamic linking */
static HINSTANCE	gamespy_library;
#else
void *GetGameSpyAPI (void *import); /* need prototype. */
#endif

void *Sys_GetGameSpyAPI(void *parms)
{
#ifndef GAMESPY_HARD_LINKED
	const char *dllname = "gamespy.dll";
	void	*(*GetGameSpyAPI) (void *);

	Com_Printf("------- Loading %s -------\n", dllname);

	if ((gamespy_library = LoadLibraryA(dllname)) == NULL)
	{
		Com_Printf( "LoadLibrary(\"%s\") failed\n", dllname);
		return NULL;
	}
	if ((GetGameSpyAPI = (void *) GetProcAddress(gamespy_library, "GetGameSpyAPI")) == NULL)
	{
		FreeLibrary (gamespy_library);
		gamespy_library = NULL;
		Com_Printf("GetProcAddress failed on %s", dllname);
		return NULL;
	}
#endif

	return GetGameSpyAPI (parms);
}

void Sys_UnloadGameSpy(void)
{
#ifndef GAMESPY_HARD_LINKED
	if (gamespy_library)
		FreeLibrary (gamespy_library);
	gamespy_library = NULL;
#endif
}
#endif /* GAMESPY ... */
