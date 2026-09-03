// net_bsd.c
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>

#include "qcommon.h"


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
int			ip_sockets[2];
int			ipx_sockets[2];

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
}

qboolean	NET_CompareAdr (netadr_t a, netadr_t b)
{
	if (a.type != b.type)
		return false;

	if (a.type == NA_LOOPBACK)
		return true;

	if (a.type == NA_IP)
	{
		if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port)
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
qboolean	NET_StringToSockaddr (const char *s, struct sockaddr *sadr)
{
	struct hostent	*h;
	char	*colon;
	char	copy[128];

	memset (sadr, 0, sizeof(*sadr));
	((struct sockaddr_in *)sadr)->sin_family = AF_INET;
	((struct sockaddr_in *)sadr)->sin_port = 0;

	strcpy (copy, s);
	// strip off a trailing :port if present
	for (colon = copy ; *colon ; colon++)
	{
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
			if (! (h = gethostbyname(copy)))
				return false;
			((struct sockaddr_in *)sadr)->sin_addr.s_addr = *(in_addr_t *) h->h_addr_list[0];
		}
	}

	return true;
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
	socklen_t	fromlen;
	int		net_socket;

	if (NET_GetLoopPacket (sock, net_from, net_message))
		return true;
	net_socket = ip_sockets[sock];
	if (!net_socket)
		return false;
	fromlen = sizeof(from);
	ret = recvfrom (net_socket, (char *)net_message->data, net_message->maxsize, 0,
			(struct sockaddr *)&from, &fromlen);
	if (ret == -1)
	{
		if (errno == EWOULDBLOCK)
			return false;
		if (dedicated->intValue)	// let dedicated servers continue after errors
			Com_Printf ("NET_GetPacket: %s", NET_ErrorString());
		else
			Com_Error (ERR_DROP, "NET_GetPacket: %s", NET_ErrorString());
		return false;
	}
	SockadrToNetadr (&from, net_from);

	if (ret == net_message->maxsize)
	{
		Com_Printf ("Oversize packet from %s\n", NET_AdrToString (*net_from));
		return false;
	}
	net_message->cursize = ret;
	return true;
}

//=============================================================================

void NET_SendPacket (netsrc_t sock, int length, void *data, netadr_t to)
{
	int		ret;
	struct sockaddr	addr;
	int		net_socket;

	if (to.type == NA_LOOPBACK)
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
		Com_Error (ERR_FATAL, "NET_SendPacket: bad address type");
		return; /* silence compiler. */
	}
	NetadrToSockadr (&to, &addr);

	ret = sendto (net_socket, (const char *)data, length, 0, &addr, sizeof(addr));
	if (ret == -1)
	{
		int err = errno;

		// wouldblock is silent
		if (err == EWOULDBLOCK)
			return;

		// some PPP links dont allow broadcasts
		if ((err == EADDRNOTAVAIL) && ((to.type == NA_BROADCAST) || (to.type == NA_BROADCAST_IPX)))
			return;
		if (dedicated->intValue)	// let dedicated servers continue after errors
		{
			Com_Printf ("NET_SendPacket ERROR: %s\n", NET_ErrorString());
		}
		else
		{
			if (err == EADDRNOTAVAIL)
			{
				Com_DPrintf (DEVELOPER_MSG_NET, "NET_SendPacket Warning: %s : %s\n", NET_ErrorString(), NET_AdrToString (to));
			}
			else
			{
				Com_Error (ERR_DROP, "NET_SendPacket ERROR: %s\n", NET_ErrorString());
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
int NET_IPSocket (char *net_interface, int port)
{
	int					newsocket;
	struct sockaddr_in	address;
	int					_true = 1;
	int					i = 1;

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		if (errno != EAFNOSUPPORT)
			Com_Printf ("WARNING: UDP_OpenSocket: socket: %s", NET_ErrorString());
		return 0;
	}

	// make it non-blocking
	if (ioctl (newsocket, FIONBIO, &_true) == -1)
	{
		Com_Printf ("WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", NET_ErrorString());
		return 0;
	}

	// make it broadcast capable
	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) == -1)
	{
		Com_Printf ("WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString());
		return 0;
	}

	if (!net_interface || !net_interface[0] || !strcasecmp(net_interface, "localhost"))
		address.sin_addr.s_addr = INADDR_ANY;
	else
		NET_StringToSockaddr (net_interface, (struct sockaddr *)&address);

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	address.sin_family = AF_INET;

	if (bind (newsocket, (const struct sockaddr *)&address, sizeof(address)) == -1)
	{
		Com_Printf ("WARNING: UDP_OpenSocket: bind: %s\n", NET_ErrorString());
		close (newsocket);
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
		port = Cvar_Get("ip_hostport", "0", CVAR_NOSET)->intValue;
		if (!port)
		{
			port = Cvar_Get("hostport", "0", CVAR_NOSET)->intValue;
			if (!port)
			{
				port = Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->intValue;
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
		port = Cvar_Get("ip_clientport", "0", CVAR_NOSET)->intValue;
		if (!port)
		{
			port = Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->intValue;
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
int NET_IPXSocket (int port)
{
	return 0;
}

/*
====================
NET_OpenIPX
====================
*/
void NET_OpenIPX (void)
{
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
				close (ip_sockets[i]);
				ip_sockets[i] = 0;
			}
		}
	}
	else
	{	// open sockets
		if (! noudp->intValue)
			NET_OpenIP ();
	}
}

//===================================================================

/*
====================
NET_Init
====================
*/
void NET_Init (void)
{
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
}

/*
====================
NET_ErrorString
====================
*/
char * NET_ErrorString (void)
{
	return strerror(errno);
}

// sleeps msec or until net socket is ready
void NET_Sleep(int msec)
{
	struct timeval timeout;
	fd_set	fdset;
	extern cvar_t *dedicated;

	if (dedicated && !dedicated->intValue) {
		return; // we're not a server, just run full speed
	}

	FD_ZERO(&fdset);
	FD_SET(ip_sockets[NS_SERVER], &fdset); // network socket
	timeout.tv_sec = (long)msec/1000;
	timeout.tv_usec = ((long)msec%1000)*1000;
	select(ip_sockets[NS_SERVER]+1, &fdset, NULL, NULL, &timeout);
}
