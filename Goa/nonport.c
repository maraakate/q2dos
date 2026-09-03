/******
nonport.c
GameSpy Developer SDK

Copyright 1999 GameSpy Industries, Inc

Suite E-204
2900 Bristol Street
Costa Mesa, CA 92626
(714)549-7689
Fax(714)549-0757

******/
#include "nonport.h"

unsigned long current_time(void) /* returns current time in msec */
{
#ifndef _WIN32
	struct timeval time;

	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
#else
	return (GetTickCount());
#endif
}

void msleep(unsigned long msec)
{
#ifndef _WIN32
	usleep(msec * 1000);
#else
	Sleep(msec);
#endif
}

void SocketStartUp(void)
{
#ifdef _WIN32
	WSADATA data;
	WSAStartup(0x0101, &data);
#endif
}

void SocketShutDown(void)
{
#ifdef _WIN32
	WSACleanup();
#endif
}

#if !defined(GAMESPY_HARD_LINKED)
char * /* from OpenBSD */
gs_strtok (char *s, const char *delim, char **last)
{
	const char *spanp;
	int c, sc;
	char *tok;

	if (s == NULL && (s = *last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = '\0';
				*last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}
#endif

