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

// sys_dos.c -- dos system driver, adapated from Quake 1

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dos.h>
#include <dir.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dpmi.h>
#include <crt0.h> /* FS: Fake Mem Fix for Win9x (QIP) */
#include <sys/nearptr.h>
#include <conio.h>
#include <float.h>

int _crt0_startup_flags = _CRT0_FLAG_UNIX_SBRK; /* FS: Fake Mem Fix for Win9x (QIP) */

unsigned _stklen = 1048576; /* FS: Up minimum stack space.  Ggorts reports he had problems until this value was raised with the ref_gl driver */

#include "dosisms.h"
#include "../qcommon/qcommon.h"
#include "../client/keys.h"

float			fptest_temp;

extern char	start_of_memory __asm__("start");

#define	KEYBUF_SIZE	256
static unsigned char	keybuf[KEYBUF_SIZE];
static int	keybuf_head = 0;
static int	keybuf_tail = 0;

static byte scantokey[128] =
{
//	0        1       2       3       4       5       6       7
//	8        9       A       B       C       D       E       F
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=', K_BACKSPACE, 9,	// 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',     13,   K_CTRL,  'a',    's',	// 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	'\'',   '`',  K_SHIFT,  '\\',   'z',    'x',    'c',    'v',	// 2
	'b',    'n',    'm',    ',',    '.',    '/',  K_SHIFT,  '*',
	K_ALT,  ' ',     0 ,    K_F1,   K_F2,   K_F3,   K_F4,  K_F5,	// 3
	K_F6,  K_F7,   K_F8,    K_F9,  K_F10,    0 ,     0 , K_HOME,
	K_UPARROW,K_PGUP,'-',K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END,	// 4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,   0 ,    0 ,     0 ,  K_F11,
	K_F12,   0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 5
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 6
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0	// 7
};

static void TrapKey(void)
{
	keybuf[keybuf_head] = dos_inportb(0x60);
	dos_outportb(0x20, 0x20);

	keybuf_head = (keybuf_head + 1) & (KEYBUF_SIZE-1);
}

unsigned int	sys_msg_time;
unsigned int	sys_frame_time;
int	sys_checksum;

int		end_of_memory;
static qboolean	lockmem, lockunlockmem, unlockmem;
static qboolean	skipwincheck, skiplfncheck, win95;

/* FS: Stuff for /memstats */
static int physicalMemStart;
static unsigned long virtualMemStart;

/* FS: Q2 needs it badly -- See http://www.delorie.com/djgpp/doc/libc/libc_380.html for more information

   ATTENTION FORKERS
   DO NOT REMOVE THE SLEEP OR WARNING!
   THIS IS SERIOUS, NO LFN AND SOME SKIN NAMES GET TRUNCATED
   WEIRD SHIT HAPPENS
   DON'T SEND ME BUG REPORTS FROM A SESSION WITH NO LFN DRIVER LOADED!
*/
static void Sys_DetectLFN (void)
{
	if(skiplfncheck)
		return;
	if(!(_get_volume_info(NULL, 0, 0, NULL) & _FILESYS_LFN_SUPPORTED))
	{
		printf("WARNING: Long file name support not detected!  Grab a copy of DOSLFN!\n");
		sleep(2);
		printf("Continuing to load Quake II. . .\n");
	}
}

static qboolean Sys_DetectWinNT (void) /* FS: Wisdom from Gisle Vanem */
{
	/* FS: Might sound crazy, but you could use that swsvpkt driver in NTVDM... */
	if(_get_dos_version(1) == 0x0532)
		return true;
	return false;
}

static void Sys_DetectWin95 (void)
{
	__dpmi_regs r;

	r.x.ax = 0x160a; /* Get Windows Version */
	__dpmi_int(0x2f, &r);

	if (((r.x.ax || r.h.bh < 4) && !Sys_DetectWinNT())) /* Not windows or earlier than Win95 */
	{
		win95 = 0;
		lockmem = true;
		lockunlockmem = false;
		unlockmem = true;
	}
	else
	{
		printf("Microsoft Windows detected.  Please run Q2DOS in pure DOS for best stability.\n"); /* FS: Warning */
		win95 = 1;
		lockunlockmem = COM_CheckParm ("-winlockunlock");
		if (lockunlockmem)
			lockmem = true;
		else
			lockmem = COM_CheckParm ("-winlock");
		unlockmem = lockmem && !lockunlockmem;
	}
}

static __dpmi_meminfo	info; /* FS: Sigh, moved this here because everyone wants me to free this shit at exit.  Again, I'm pretty sure CWSDPMI is already taking care of this... */

static int Sys_Get_Physical_Memory(void) /* FS: From DJGPP tutorial */
{
	_go32_dpmi_meminfo meminfo;

	_go32_dpmi_get_free_memory_information(&meminfo);
	if (meminfo.available_physical_pages != -1)
		return meminfo.available_physical_pages * 4096;

	return meminfo.available_memory;
}

/* FS: For /memstats */
void Sys_Memory_Stats_f (void)
{
	Com_Printf("%d Mb available for Q2DOS.  Started with %d.\n", (Sys_Get_Physical_Memory() / 0x100000), physicalMemStart );
	Com_Printf("%lu Virtual Mb available for Q2DOS. Started with %lu.\n", (_go32_dpmi_remaining_virtual_memory() / 0x100000), virtualMemStart );
}

/*
================
Sys_PageInProgram

walks the text, data, and bss to make sure it's all paged in so that the
actual physical memory detected by Sys_GetMemory is correct.
================
*/
static void Sys_PageInProgram(void)
{
	int		i, j;

	end_of_memory = (int)sbrk(0);

	if (lockmem)
	{
		if (dos_lockmem ((void *)&start_of_memory,
						 end_of_memory - (int)&start_of_memory))
			Sys_Error ("Couldn't lock text and data");
	}

	if (lockunlockmem)
	{
		dos_unlockmem((void *)&start_of_memory,
						 end_of_memory - (int)&start_of_memory);
		printf ("Locked and unlocked %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}
	else if (lockmem)
	{
		printf ("Locked %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}
	else
	{
		printf ("Loaded %d Mb image\n",
				(end_of_memory - (int)&start_of_memory) / 0x100000);
	}

// touch the entire image, doing the 16-page skip so Win95 doesn't think we're
// trying to page ourselves in
	for (j=0 ; j<4 ; j++)
	{
		for(i=(int)&start_of_memory ; i<(end_of_memory - 16 * 0x1000) ; i += 4)
		{
			sys_checksum += *(int *)i;
			sys_checksum += *(int *)(i + 16 * 0x1000);
		}
	}

	/* FS: Report total amount available and save it for later if we run /memstats */
	physicalMemStart = (Sys_Get_Physical_Memory() / 0x100000);
	virtualMemStart = (_go32_dpmi_remaining_virtual_memory() / 0x100000);

	printf("%d Mb available for Q2DOS.\n", physicalMemStart);
	printf("%lu Virtual Mb available for Q2DOS.\n", virtualMemStart);
}

static void Sys_SetTextMode (void) /* FS: This was used twice, let's make it a little cleaner if we can. */
{
	__dpmi_regs r;

	// return to text mode
	r.x.ax = 3;
	__dpmi_int(0x10, &r);
}

void Sys_Error (const char *error, ...)
{
	va_list		argptr;

	if (!dedicated || !dedicated->value)
		dos_restoreintr(9); /* FS: Give back the keyboard */

	Sys_SetTextMode();

	printf ("Sys_Error: ");
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

	__dpmi_free_physical_address_mapping(&info);
	__djgpp_nearptr_disable(); /* FS: Everyone else is a master DOS DPMI programmer.  Pretty sure CWSDPMI is already taking care of this... */

#if 0
{	/* we crash here so we can get a backtrace.  Yes it is ugly, and no this should never be in production! */
	volatile int j,k;
	fflush(stdout);
	j=0;
	k=5/j;	/* divide by zero! */
}
#endif
	exit (1);
}

void Sys_Quit (void)
{
	if(!dedicated || !dedicated->value)
		dos_restoreintr(9); /* FS: Give back the keyboard */

	Cvar_Shutdown(); /* FS: Free our CVAR memory too */

	if (unlockmem)
	{
		dos_unlockmem (&start_of_memory,
					   end_of_memory - (int)&start_of_memory);
	}

	Sys_SetTextMode();

	__dpmi_free_physical_address_mapping(&info);
	__djgpp_nearptr_disable(); /* FS: Everyone else is a master DOS DPMI programmer.  Pretty sure CWSDPMI is already taking care of this... */

	exit (0);
}

#ifdef GAME_HARD_LINKED
void *GetGameAPI (void *import);
void	Sys_UnloadGame (void)
{
}
void	*Sys_GetGameAPI (void *parms)
{
	return GetGameAPI (parms);
}
#endif

#if defined(GAMESPY) && defined(GAMESPY_HARD_LINKED)
void *GetGameSpyAPI (void *import); /* need prototype. */
void	Sys_UnloadGameSpy (void)
{
}
void	*Sys_GetGameSpyAPI (void *parms)
{
	return GetGameSpyAPI (parms);
}
#endif /* GAMESPY ... */

char *Sys_ConsoleInput (void)
{
	static char     text[256];
	static int      len = 0;
	char            ch;

	if (!dedicated || !dedicated->value)
		return NULL;

	if (! kbhit())
		return NULL;

	ch = getche();

	switch (ch)
	{
		case '\r':
			putch('\n');
			if (len)
			{
				text[len] = 0;
				len = 0;
				return text;
			}
			break;

		case '\b':
			putch(' ');
			if (len)
			{
				len--;
				putch('\b');
			}
			break;

		default:
			text[len] = ch;
			len = (len + 1) & 0xff;
			break;
	}

	return NULL;
}

void	Sys_ConsoleOutput (char *string)
{
	if (!dedicated || !dedicated->value)
		return;
	printf("%s",string);
}

// Knightmare- added this to fix CPU usage
void Sys_Sleep (unsigned msec)
{
	usleep (msec*1000);
}

#define	SC_UPARROW	0x48
#define	SC_DOWNARROW	0x50
#define	SC_LEFTARROW	0x4b
#define	SC_RIGHTARROW	0x4d
#define	SC_LEFTSHIFT	0x2a
#define	SC_RIGHTSHIFT	0x36

void Sys_SendKeyEvents (void)
{
	int	k, next;
	int	outkey;

// grab frame time
	sys_msg_time = Sys_Milliseconds();

// get key events

	while (keybuf_head != keybuf_tail)
	{
		k = keybuf[keybuf_tail++];
		keybuf_tail &= (KEYBUF_SIZE - 1);

		if (k == 0xe0)
			continue;		// special / pause keys
		next = keybuf[(keybuf_tail - 2) & (KEYBUF_SIZE - 1)];
		// Pause generates e1 1d 45 e1 9d c5 when pressed, and
		// nothing when released. e1 is generated only for the
		// pause key.
		if (next == 0xe1)
			continue;		// pause key bullshit
		if (k == 0xc5 && next == 0x9d)
		{
			Key_Event (K_PAUSE, true, sys_msg_time);
			continue;
		}

		// extended keyboard shift key bullshit
		if ( (k & 0x7f) == SC_LEFTSHIFT || (k & 0x7f) == SC_RIGHTSHIFT )
		{
			if (keybuf[(keybuf_tail - 2) & (KEYBUF_SIZE - 1)] == 0xe0)
				continue;
			k &= 0x80;
			k |= SC_RIGHTSHIFT;
		}

		if (k == 0xc5 && keybuf[(keybuf_tail - 2) & (KEYBUF_SIZE - 1)] == 0x9d)
			continue;	// more pause bullshit

		outkey = scantokey[k & 0x7f];

		if (k & 0x80)
			Key_Event (outkey, false, sys_msg_time);
		else
			Key_Event (outkey, true, sys_msg_time);
	}

	sys_frame_time = Sys_Milliseconds(); /* FS: Need to update this for input to work properly */
}

void Sys_AppActivate (void)
{
}

void Sys_CopyProtect (void)
{
}

char *Sys_GetClipboardData(void)
{
	return NULL;
}

void	Sys_Init (void)
{
	_go32_interrupt_stack_size = 4 * 1024;
	_go32_rmcb_stack_size = 4 * 1024;

	Sys_InitDXE3 ();
}

//=============================================================================

static void Sys_ParseEarlyArgs(int argc, char **argv) /* FS: Parse some very specific args before Qcommon_Init */
{
	int i;
	for (i = 1; i < argc; i++)
	{
		if(stricmp(argv[i],"-skipwincheck") == 0)
			skipwincheck = true;
		if(stricmp(argv[i],"-skiplfncheck") == 0)
			skiplfncheck = true;
	}
}

/*
================
Sys_NoFPUExceptionHandler
================
*/
static void Sys_NoFPUExceptionHandler(int whatever)
{
	printf ("\nError: Quake 2 for MS-DOS requires a floating-point processor\n");
	exit (0);
}

/*
================
Sys_DefaultExceptionHandler
================
*/
static void Sys_DefaultExceptionHandler(int whatever)
{
}

//=============================================================================

int main (int argc, char **argv)
{
	int time, oldtime, newtime;

	printf ("Quake 2 DOS v%4.2f\n", VERSION);

// make sure there's an FPU
	signal(SIGNOFP, Sys_NoFPUExceptionHandler);
	signal(SIGABRT, Sys_DefaultExceptionHandler);
	signal(SIGALRM, Sys_DefaultExceptionHandler);
	signal(SIGKILL, Sys_DefaultExceptionHandler);
	signal(SIGQUIT, Sys_DefaultExceptionHandler);
	signal(SIGINT, Sys_DefaultExceptionHandler);

	if (fptest_temp >= 0.0)
		fptest_temp += 0.1;

	Sys_ParseEarlyArgs(argc, argv);
	Sys_DetectLFN();
	Sys_DetectWin95 ();
	Sys_PageInProgram ();

	_crt0_startup_flags &= ~_CRT0_FLAG_UNIX_SBRK; /* FS: We walked through all the data, now remove the sbrk flag so Win9x doesn't barf. */

	Sys_Init();

	Qcommon_Init (argc, argv);

	oldtime = Sys_Milliseconds ();

	if (!dedicated || !dedicated->value)
	{
		dos_registerintr(9, TrapKey);
	}

    /* main window message loop */
	while (1)
	{
		do
		{
			newtime = Sys_Milliseconds ();
			time = newtime - oldtime;
		}
		while (time < 1);
		_control87(PC_24, MCW_PC); /* FS: Win32 is doing this. */
		Qcommon_Frame (time);
		oldtime = newtime;
	}
}
