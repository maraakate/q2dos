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
// net_wins.c

#include <winsock2.h>
#include <wsipx.h>
#include "../qcommon/qcommon.h"

#define	MAX_LOOPBACK	4

typedef struct
{
	byte	data[MAX_MSGLEN];
	int		datalen;
} loopmsg_t;

typedef struct
{
	loopmsg_t	msgs[MAX_LOOPBACK];
	int			get, send;
} loopback_t;


cvar_t		*net_shownet;
static cvar_t	*noudp;
static cvar_t	*noipx;

loopback_t	loopbacks[2];
SOCKET			ip_sockets[2];
SOCKET			ipx_sockets[2];

//=============================================================================

void NetadrToSockadr (netadr_t *a, struct sockaddr *s)
{
	memset (s, 0, sizeof(*s));

	if (a->type == NA_BROADCAST)
	{
		((struct sockaddr_in *)s)->sin_family = AF_INET;
		((struct sockaddr_in *)s)->sin_port = a->port;
		((struct sockaddr_in *)s)->sin_addr.s_addr = INADDR_BROADCAST;
	}
	else if (a->type == NA_IP)
	{
		((struct sockaddr_in *)s)->sin_family = AF_INET;
	//	((struct sockaddr_in *)s)->sin_addr.s_addr = *(int *)&a->ip;
		memcpy (& ((struct sockaddr_in *)s)->sin_addr, a->ip, 4);
		((struct sockaddr_in *)s)->sin_port = a->port;
	}
	else if (a->type == NA_IPX)
	{
		((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
		memcpy(((struct sockaddr_ipx *)s)->sa_netnum, &a->ipx[0], 4);
		memcpy(((struct sockaddr_ipx *)s)->sa_nodenum, &a->ipx[4], 6);
		((struct sockaddr_ipx *)s)->sa_socket = a->port;
	}
	else if (a->type == NA_BROADCAST_IPX)
	{
		((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
		memset(((struct sockaddr_ipx *)s)->sa_netnum, 0, 4);
		memset(((struct sockaddr_ipx *)s)->sa_nodenum, 0xff, 6);
		((struct sockaddr_ipx *)s)->sa_socket = a->port;
	}
}

void SockadrToNetadr (struct sockaddr *s, netadr_t *a)
{
	if (s->sa_family == AF_INET)
	{
		a->type = NA_IP;
	//	*(int *)&a->ip = ((struct sockaddr_in *)s)->sin_addr.s_addr;
		memcpy (a->ip, & ((struct sockaddr_in *)s)->sin_addr, 4);
		a->port = ((struct sockaddr_in *)s)->sin_port;
	}
	else if (s->sa_family == AF_IPX)
	{
		a->type = NA_IPX;
		memcpy(&a->ipx[0], ((struct sockaddr_ipx *)s)->sa_netnum, 4);
		memcpy(&a->ipx[4], ((struct sockaddr_ipx *)s)->sa_nodenum, 6);
		a->port = ((struct sockaddr_ipx *)s)->sa_socket;
	}
}

qboolean	NET_CompareAdr (netadr_t a, netadr_t b)
{
	if (a.type != b.type)
		return false;

	if (a.type == NA_LOOPBACK)
		return TRUE;

	if (a.type == NA_IP)
	{
		if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port)
			return true;
		return false;
	}
	if (a.type == NA_IPX)
	{
		if ((memcmp(a.ipx, b.ipx, 10) == 0) && a.port == b.port)
			return true;
		return false;
	}

	return false;
}

/*
===================
NET_CompareBaseAdr

Compares without the port
===================
*/
qboolean	NET_CompareBaseAdr (netadr_t a, netadr_t b)
{
	if (a.type != b.type)
		return false;

	if (a.type == NA_LOOPBACK)
		return true;

	if (a.type == NA_IP)
	{
		if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3])
			return true;
		return false;
	}
	if (a.type == NA_IPX)
	{
		if ((memcmp(a.ipx, b.ipx, 10) == 0))
			return true;
		return false;
	}

	return false;
}

char	*NET_AdrToString (netadr_t a)
{
	static	char	s[64];

	if (a.type == NA_LOOPBACK)
		Com_sprintf (s, sizeof(s), "loopback");
	else if (a.type == NA_IP)
		Com_sprintf (s, sizeof(s), "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs(a.port));
	else
		Com_sprintf (s, sizeof(s), "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%i", a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9], ntohs(a.port));

	return s;
}

/*
=============
NET_StringToAdr

localhost
idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
#define DO(src,dest)	\
	copy[0] = s[src];	\
	copy[1] = s[src + 1];	\
	sscanf (copy, "%x", &val);	\
	((struct sockaddr_ipx *)sadr)->dest = val

qboolean	NET_StringToSockaddr (char *s, struct sockaddr *sadr)
{
	struct hostent	*h;
	char	*colon;
	int		val;
	char	copy[128];

	memset (sadr, 0, sizeof(*sadr));

	if ((strlen(s) >= 23) && (s[8] == ':') && (s[21] == ':'))	// check for an IPX address
	{
		((struct sockaddr_ipx *)sadr)->sa_family = AF_IPX;
		copy[2] = 0;
		DO(0, sa_netnum[0]);
		DO(2, sa_netnum[1]);
		DO(4, sa_netnum[2]);
		DO(6, sa_netnum[3]);
		DO(9, sa_nodenum[0]);
		DO(11, sa_nodenum[1]);
		DO(13, sa_nodenum[2]);
		DO(15, sa_nodenum[3]);
		DO(17, sa_nodenum[4]);
		DO(19, sa_nodenum[5]);
		sscanf (&s[22], "%u", &val);
		((struct sockaddr_ipx *)sadr)->sa_socket = htons((unsigned short)val);
	}
	else
	{
		((struct sockaddr_in *)sadr)->sin_family = AF_INET;
		((struct sockaddr_in *)sadr)->sin_port = 0;

		strcpy (copy, s);
		// strip off a trailing :port if present
		for (colon = copy ; *colon ; colon++)
			if (*colon == ':')
			{
				*colon = 0;
				((struct sockaddr_in *)sadr)->sin_port = htons((short)atoi(colon+1));
			}
		
		if (copy[0] >= '0' && copy[0] <= '9')
		{
			((struct sockaddr_in *)sadr)->sin_addr.s_addr = inet_addr(copy);
		}
		else
		{
			if (! (h = gethostbyname(copy)) )
				return false;
			((struct sockaddr_in *)sadr)->sin_addr.s_addr = *(u_long *) h->h_addr_list[0];
		}
	}
	
	return true;
}

#undef DO

/*
=============
NET_StringToAdr

localhost
idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
qboolean	NET_StringToAdr (char *s, netadr_t *a)
{
	struct sockaddr sadr;
	
	if (!strcmp (s, "localhost"))
	{
		memset (a, 0, sizeof(*a));
		a->type = NA_LOOPBACK;
		return true;
	}

	if (!NET_StringToSockaddr (s, &sadr))
		return false;
	
	SockadrToNetadr (&sadr, a);

	return true;
}

qboolean	NET_IsLocalAddress (netadr_t adr)
{
	return adr.type == NA_LOOPBACK;
}

/*
=============================================================================

LOOPBACK BUFFERS FOR LOCAL PLAYER

=============================================================================
*/

qboolean	NET_GetLoopPacket (netsrc_t sock, netadr_t *net_from, sizebuf_t *net_message)
{
	int		i;
	loopback_t	*loop;

	loop = &loopbacks[sock];

	if (loop->send - loop->get > MAX_LOOPBACK)
		loop->get = loop->send - MAX_LOOPBACK;

	if (loop->get >= loop->send)
		return false;

	i = loop->get & (MAX_LOOPBACK-1);
	loop->get++;

	memcpy (net_message->data, loop->msgs[i].data, loop->msgs[i].datalen);
	net_message->cursize = loop->msgs[i].datalen;
	memset (net_from, 0, sizeof(*net_from));
	net_from->type = NA_LOOPBACK;
	return true;
}

void NET_SendLoopPacket (netsrc_t sock, int length, void *data, netadr_t to)
{
	int		i;
	loopback_t	*loop;

	loop = &loopbacks[sock^1];

	i = loop->send & (MAX_LOOPBACK-1);
	loop->send++;

	memcpy (loop->msgs[i].data, data, length);
	loop->msgs[i].datalen = length;
}

//=============================================================================

qboolean	NET_GetPacket (netsrc_t sock, netadr_t *net_from, sizebuf_t *net_message)
{
	int 	ret;
	struct sockaddr from;
	int		fromlen;
	SOCKET		net_socket;
	int		protocol;
	int		err;

	if (NET_GetLoopPacket (sock, net_from, net_message))
		return true;

	for (protocol = 0 ; protocol < 2 ; protocol++)
	{
		if (protocol == 0)
			net_socket = ip_sockets[sock];
		else
			net_socket = ipx_sockets[sock];
		if (!net_socket)
			continue;

		fromlen = sizeof(from);
		ret = recvfrom (net_socket, (char *)net_message->data, net_message->maxsize, 0,
				(struct sockaddr *)&from, &fromlen);

		SockadrToNetadr (&from, net_from);

		if (ret == SOCKET_ERROR)
		{
			err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
				continue;
			if (err == WSAEMSGSIZE) {
				Com_Printf ("Warning:  Oversize packet from %s\n",
						NET_AdrToString(*net_from));
				continue;
			}

			if (dedicated->intValue)	// let dedicated servers continue after errors
			{
				if(err == WSAECONNRESET) /* FS: Don't need to see this spam from connecting to ourselves */
				{
					Com_DPrintf (DEVELOPER_MSG_NET, "NET_GetPacket: \"%s\" from %s\n",
							NET_ErrorString(), NET_AdrToString(*net_from));
				}
				else
				{
					Com_Printf ("NET_GetPacket: \"%s\" from %s\n", NET_ErrorString(),
							NET_AdrToString(*net_from));
				}
			}
			else
			{
				Com_Error (ERR_DROP, "NET_GetPacket: \"%s\" from %s",
						NET_ErrorString(), NET_AdrToString(*net_from));
			}
			continue;
		}

		if (ret == net_message->maxsize)
		{
			Com_Printf ("Oversize packet from %s\n", NET_AdrToString (*net_from));
			continue;
		}

		net_message->cursize = ret;
		return true;
	}

	return false;
}

//=============================================================================

void NET_SendPacket (netsrc_t sock, int length, void *data, netadr_t to)
{
	int		ret;
	struct sockaddr	addr;
	SOCKET		net_socket;

	if ( to.type == NA_LOOPBACK )
	{
		NET_SendLoopPacket (sock, length, data, to);
		return;
	}

	if (to.type == NA_BROADCAST)
	{
		net_socket = ip_sockets[sock];
		if (!net_socket)
			return;
	}
	else if (to.type == NA_IP)
	{
		net_socket = ip_sockets[sock];
		if (!net_socket)
			return;
	}
	else if (to.type == NA_IPX)
	{
		net_socket = ipx_sockets[sock];
		if (!net_socket)
			return;
	}
	else if (to.type == NA_BROADCAST_IPX)
	{
		net_socket = ipx_sockets[sock];
		if (!net_socket)
			return;
	}
	else
	{
		net_socket = INVALID_SOCKET; /* silence compiler */
		Com_Error (ERR_FATAL, "NET_SendPacket: bad address type");
	}

	NetadrToSockadr (&to, &addr);

	ret = sendto (net_socket, data, length, 0, &addr, sizeof(addr) );
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		// wouldblock is silent
		if (err == WSAEWOULDBLOCK)
			return;

		// some PPP links dont allow broadcasts
		if ((err == WSAEADDRNOTAVAIL) && ((to.type == NA_BROADCAST) || (to.type == NA_BROADCAST_IPX)))
			return;

		if (dedicated->intValue)	// let dedicated servers continue after errors
		{
			Com_Printf ("NET_SendPacket ERROR: \"%s\" to %s\n", NET_ErrorString(),
				NET_AdrToString (to));
		}
		else
		{
			if (err == WSAEADDRNOTAVAIL)
			{
				Com_DPrintf (DEVELOPER_MSG_NET, "NET_SendPacket Warning: %s : %s\n",
						NET_ErrorString(), NET_AdrToString (to));
			}
			else
			{
				Com_Error (ERR_DROP, "NET_SendPacket ERROR: \"%s\" to %s\n",
						NET_ErrorString(), NET_AdrToString (to));
			}
		}
	}
}

//=============================================================================


/*
====================
NET_Socket
====================
*/
SOCKET NET_IPSocket (char *net_interface, int port)
{
	SOCKET			newsocket;
	struct sockaddr_in	address;
	u_long			_true = true;
	int					i = 1;
	int					err;

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		err = WSAGetLastError();
		if (err != WSAEAFNOSUPPORT)
			Com_Printf ("WARNING: UDP_OpenSocket: socket: %s\n", NET_ErrorString());
		return 0;
	}

	// make it non-blocking
	if (ioctlsocket (newsocket, FIONBIO, &_true) == SOCKET_ERROR)
	{
		Com_Printf ("WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", NET_ErrorString());
		return 0;
	}

	// make it broadcast capable
	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) == SOCKET_ERROR)
	{
		Com_Printf ("WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString());
		return 0;
	}

	if (!net_interface || !net_interface[0] || !stricmp(net_interface, "localhost"))
		address.sin_addr.s_addr = INADDR_ANY;
	else
		NET_StringToSockaddr (net_interface, (struct sockaddr *)&address);

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	address.sin_family = AF_INET;

	if ( bind (newsocket, (void *)&address, sizeof(address)) == SOCKET_ERROR)
	{
		Com_Printf ("WARNING: UDP_OpenSocket: bind: %s\n", NET_ErrorString());
		closesocket (newsocket);
		return 0;
	}

	return newsocket;
}

/*
====================
NET_OpenIP
====================
*/
void NET_OpenIP (void)
{
	cvar_t	*ip;
	int		port;
	int		dedicated;

	ip = Cvar_Get ("ip", "localhost", CVAR_NOSET);

	dedicated = Cvar_VariableValue ("dedicated");

	if (!ip_sockets[NS_SERVER])
	{
		port = Cvar_Get("ip_hostport", "0", CVAR_NOSET)->value;
		if (!port)
		{
			port = Cvar_Get("hostport", "0", CVAR_NOSET)->value;
			if (!port)
			{
				port = Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->value;
			}
		}
		ip_sockets[NS_SERVER] = NET_IPSocket (ip->string, port);
		if (!ip_sockets[NS_SERVER] && dedicated)
			Com_Error (ERR_FATAL, "Couldn't allocate dedicated server IP port");
	}


	// dedicated servers don't need client ports
	if (dedicated)
		return;

	if (!ip_sockets[NS_CLIENT])
	{
		port = Cvar_Get("ip_clientport", "0", CVAR_NOSET)->value;
		if (!port)
		{
			port = Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->value;
			if (!port)
				port = PORT_ANY;
		}
		ip_sockets[NS_CLIENT] = NET_IPSocket (ip->string, port);
		if (!ip_sockets[NS_CLIENT])
			ip_sockets[NS_CLIENT] = NET_IPSocket (ip->string, PORT_ANY);
	}
}

/*
====================
IPX_Socket
====================
*/
SOCKET NET_IPXSocket (int port)
{
	SOCKET			newsocket;
	struct sockaddr_ipx	address;
	u_long					_true = 1;
	int					err;

	if ((newsocket = socket (PF_IPX, SOCK_DGRAM, NSPROTO_IPX)) == INVALID_SOCKET)
	{
		err = WSAGetLastError();
		if (err != WSAEAFNOSUPPORT)
			Com_Printf ("WARNING: IPX_Socket: socket: %s\n", NET_ErrorString());
		return 0;
	}

	// make it non-blocking
	if (ioctlsocket (newsocket, FIONBIO, &_true) == SOCKET_ERROR)
	{
		Com_Printf ("WARNING: IPX_Socket: ioctl FIONBIO: %s\n", NET_ErrorString());
		return 0;
	}

	// make it broadcast capable
	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&_true, sizeof(_true)) == SOCKET_ERROR)
	{
		Com_Printf ("WARNING: IPX_Socket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString());
		return 0;
	}

	address.sa_family = AF_IPX;
	memset (address.sa_netnum, 0, 4);
	memset (address.sa_nodenum, 0, 6);
	if (port == PORT_ANY)
		address.sa_socket = 0;
	else
		address.sa_socket = htons((short)port);

	if( bind (newsocket, (void *)&address, sizeof(address)) == SOCKET_ERROR)
	{
		Com_Printf ("WARNING: IPX_Socket: bind: %s\n", NET_ErrorString());
		closesocket (newsocket);
		return 0;
	}

	return newsocket;
}

/*
====================
NET_OpenIPX
====================
*/
void NET_OpenIPX (void)
{
	int		port;
	int		dedicated;

	dedicated = Cvar_VariableValue ("dedicated");

	if (!ipx_sockets[NS_SERVER])
	{
		port = Cvar_Get("ipx_hostport", "0", CVAR_NOSET)->value;
		if (!port)
		{
			port = Cvar_Get("hostport", "0", CVAR_NOSET)->value;
			if (!port)
			{
				port = Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->value;
			}
		}
		ipx_sockets[NS_SERVER] = NET_IPXSocket (port);
	}

	// dedicated servers don't need client ports
	if (dedicated)
		return;

	if (!ipx_sockets[NS_CLIENT])
	{
		port = Cvar_Get("ipx_clientport", "0", CVAR_NOSET)->value;
		if (!port)
		{
			port = Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->value;
			if (!port)
				port = PORT_ANY;
		}
		ipx_sockets[NS_CLIENT] = NET_IPXSocket (port);
		if (!ipx_sockets[NS_CLIENT])
			ipx_sockets[NS_CLIENT] = NET_IPXSocket (PORT_ANY);
	}
}

/*
====================
NET_Config

A single player game will only use the loopback code
====================
*/
void	NET_Config (qboolean multiplayer)
{
	int		i;
	static	qboolean	old_config;

	if (old_config == multiplayer)
		return;

	old_config = multiplayer;

	if (!multiplayer)
	{	// shut down any existing sockets
		for (i=0 ; i<2 ; i++)
		{
			if (ip_sockets[i])
			{
				closesocket (ip_sockets[i]);
				ip_sockets[i] = 0;
			}
			if (ipx_sockets[i])
			{
				closesocket (ipx_sockets[i]);
				ipx_sockets[i] = 0;
			}
		}
	}
	else
	{	// open sockets
		if (!noudp->intValue)
			NET_OpenIP ();
		if (!noipx->intValue)
			NET_OpenIPX ();
	}
}

// sleeps msec or until net socket is ready
void NET_Sleep(int msec)
{
    struct timeval timeout;
	fd_set	fdset;
	extern cvar_t *dedicated;
	SOCKET i;

	if (!dedicated || !dedicated->value)
		return; // we're not a server, just run full speed

	FD_ZERO(&fdset);
	i = 0;
	if (ip_sockets[NS_SERVER]) {
		FD_SET(ip_sockets[NS_SERVER], &fdset); // network socket
		i = ip_sockets[NS_SERVER];
	}
	if (ipx_sockets[NS_SERVER]) {
		FD_SET(ipx_sockets[NS_SERVER], &fdset); // network socket
		if (ipx_sockets[NS_SERVER] > i)
			i = ipx_sockets[NS_SERVER];
	}
	timeout.tv_sec = msec/1000;
	timeout.tv_usec = (msec%1000)*1000;
	select(i+1, &fdset, NULL, NULL, &timeout);
}

//===================================================================


static WSADATA		winsockdata;

/*
====================
NET_Init
====================
*/
void NET_Init (void)
{
	int err = WSAStartup (MAKEWORD(1,1), &winsockdata);
	if (err)
		Com_Error (ERR_FATAL,"Winsock initialization failed.");

	Com_Printf("Winsock Initialized\n");

	noudp = Cvar_Get ("noudp", "0", CVAR_NOSET);
	noipx = Cvar_Get ("noipx", "0", CVAR_NOSET);

	net_shownet = Cvar_Get ("net_shownet", "0", 0);
}


/*
====================
NET_Shutdown
====================
*/
void	NET_Shutdown (void)
{
	NET_Config (false);	// close sockets

	WSACleanup ();
}


/*
====================
NET_ErrorString
====================
*/
char *NET_ErrorString (void)
{
	int code = WSAGetLastError ();
	switch (code)
	{
	case 0:			return "No error";
	case WSAEINTR:		return "Interrupted system call";		/* 10004 */
	case WSAEBADF:		return "Bad file number";			/* 10009 */
	case WSAEACCES:		return "Permission denied";			/* 10013 */
	case WSAEFAULT:		return "Bad address";				/* 10014 */
	case WSAEINVAL:		return "Invalid argument (not bind)";		/* 10022 */
	case WSAEMFILE:		return "Too many open files";			/* 10024 */
	case WSAEWOULDBLOCK:	return "Operation would block";			/* 10035 */
	case WSAEINPROGRESS:	return "Operation now in progress";		/* 10036 */
	case WSAEALREADY:	return "Operation already in progress";		/* 10037 */
	case WSAENOTSOCK:	return "Socket operation on non-socket";	/* 10038 */
	case WSAEDESTADDRREQ:	return "Destination address required";		/* 10039 */
	case WSAEMSGSIZE:	return "Message too long";			/* 10040 */
	case WSAEPROTOTYPE:	return "Protocol wrong type for socket";	/* 10041 */
	case WSAENOPROTOOPT:	return "Bad protocol option";			/* 10042 */
	case WSAEPROTONOSUPPORT: return "Protocol not supported";		/* 10043 */
	case WSAESOCKTNOSUPPORT: return "Socket type not supported";		/* 10044 */
	case WSAEOPNOTSUPP:	return "Operation not supported on socket";	/* 10045 */
	case WSAEPFNOSUPPORT:	return "Protocol family not supported";		/* 10046 */
	case WSAEAFNOSUPPORT:	return "Address family not supported by protocol family"; /* 10047 */
	case WSAEADDRINUSE:	return "Address already in use";		/* 10048 */
	case WSAEADDRNOTAVAIL:	return "Can't assign requested address";	/* 10049 */
	case WSAENETDOWN:	return "Network is down";			/* 10050 */
	case WSAENETUNREACH:	return "Network is unreachable";		/* 10051 */
	case WSAENETRESET:	return "Net dropped connection or reset";	/* 10052 */
	case WSAECONNABORTED:	return "Software caused connection abort";	/* 10053 */
	case WSAECONNRESET:	return "Connection reset by peer";		/* 10054 */
	case WSAENOBUFS:	return "No buffer space available";		/* 10055 */
	case WSAEISCONN:	return "Socket is already connected";		/* 10056 */
	case WSAENOTCONN:	return "Socket is not connected";		/* 10057 */
	case WSAESHUTDOWN:	return "Can't send after socket shutdown";	/* 10058 */
	case WSAETOOMANYREFS:	return "Too many references, can't splice";	/* 10059 */
	case WSAETIMEDOUT:	return "Connection timed out";			/* 10060 */
	case WSAECONNREFUSED:	return "Connection refused";			/* 10061 */
	case WSAELOOP:		return "Too many levels of symbolic links";	/* 10062 */
	case WSAENAMETOOLONG:	return "File name too long";			/* 10063 */
	case WSAEHOSTDOWN:	return "Host is down";				/* 10064 */
	case WSAEHOSTUNREACH:	return "No Route to Host";			/* 10065 */
	case WSAENOTEMPTY:	return "Directory not empty";			/* 10066 */
	case WSAEPROCLIM:	return "Too many processes";			/* 10067 */
	case WSAEUSERS:		return "Too many users";			/* 10068 */
	case WSAEDQUOT:		return "Disc Quota Exceeded";			/* 10069 */
	case WSAESTALE:		return "Stale NFS file handle";			/* 10070 */
	case WSAEREMOTE:	return "Too many levels of remote in path";	/* 10071 */
	case WSAEDISCON:	return "Graceful shutdown in progress";		/* 10101 */

	case WSASYSNOTREADY:	return "Network SubSystem is unavailable";			/* 10091 */
	case WSAVERNOTSUPPORTED: return "WINSOCK DLL Version out of range";			/* 10092 */
	case WSANOTINITIALISED:	return "Successful WSASTARTUP not yet performed";		/* 10093 */
	case WSAHOST_NOT_FOUND:	return "Authoritative answer: Host not found";			/* 11001 */
	case WSATRY_AGAIN:	return "Non-Authoritative: Host not found or SERVERFAIL";	/* 11002 */
	case WSANO_RECOVERY:	return "Non-Recoverable errors, FORMERR, REFUSED, NOTIMP";	/* 11003 */
	case WSANO_DATA:	return "Valid name, no data record of requested type";		/* 11004 */

	case WSAENOMORE:		return "10102: No more results";			/* 10102 */
	case WSAECANCELLED:		return "10103: Call has been canceled";			/* 10103 */
	case WSAEINVALIDPROCTABLE:	return "Procedure call table is invalid";		/* 10104 */
	case WSAEINVALIDPROVIDER:	return "Service provider is invalid";			/* 10105 */
	case WSAEPROVIDERFAILEDINIT:	return "Service provider failed to initialize";		/* 10106 */
	case WSASYSCALLFAILURE:		return "System call failure";				/* 10107 */
	case WSASERVICE_NOT_FOUND:	return "Service not found";				/* 10108 */
	case WSATYPE_NOT_FOUND:		return "Class type not found";				/* 10109 */
	case WSA_E_NO_MORE:		return "10110: No more results";			/* 10110 */
	case WSA_E_CANCELLED:		return "10111: Call was canceled";			/* 10111 */
	case WSAEREFUSED:		return "Database query was refused";			/* 10112 */

	default:
		{
			static char _err_unknown[64];
			sprintf(_err_unknown, "Unknown WSAE error (%d)", code);
			return  _err_unknown;
		}
	}
}
