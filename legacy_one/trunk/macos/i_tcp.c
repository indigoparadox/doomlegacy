// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: i_tcp.c,v $
// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// Revision 1.1  2001/04/17 22:23:38  calumr
// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// Initial add
// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// Revision 1.30  2000/11/26 00:46:31  hurdler
// small bug fixes
//
// Revision 1.29  2000/10/21 08:43:29  bpereira
// no message
//
// Revision 1.28  2000/10/16 20:02:29  bpereira
// no message
//
// Revision 1.27  2000/10/08 13:30:00  bpereira
// no message
//
// Revision 1.26  2000/10/01 15:20:23  hurdler
// Add private server
//
// Revision 1.25  2000/09/28 20:57:15  bpereira
// no message
//
// Revision 1.24  2000/09/15 19:49:22  bpereira
// no message
//
// Revision 1.23  2000/09/10 10:43:21  metzgermeister
// *** empty log message ***
//
// Revision 1.22  2000/09/08 22:28:30  hurdler
// merge masterserver_ip/port in one cvar, add -private
//
// Revision 1.21  2000/09/01 18:23:42  hurdler
// fix some issues with latest network code changes
//
// Revision 1.20  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.19  2000/08/29 15:53:47  hurdler
// Remove master server connect timeout on LAN (not connected to Internet)
//
// Revision 1.18  2000/08/21 11:06:44  hurdler
// Add ping and some fixes
//
// Revision 1.17  2000/08/17 23:18:05  hurdler
// fix bad port sent to master server when using -udpport
//
// Revision 1.16  2000/08/16 23:39:41  hurdler
// fix a bug with windows sockets
//
// Revision 1.15  2000/08/16 17:21:50  hurdler
// update master server code (bis)
//
// Revision 1.14  2000/08/16 15:44:18  hurdler
// update master server code
//
// Revision 1.13  2000/08/16 14:10:01  hurdler
// add master server code
//
// Revision 1.12  2000/08/10 14:55:56  ydario
// OS/2 port
//
// Revision 1.11  2000/08/10 14:08:48  hurdler
// no message
//
// Revision 1.10  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.9  2000/04/21 13:03:27  hurdler
// apply Robert's patch for SOCK_Get error. Boris, can you verify this?
//
// Revision 1.8  2000/04/21 00:01:45  hurdler
// apply Robert's patch for SOCK_Get error. Boris, can you verify this?
//
// Revision 1.7  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.5  2000/03/08 14:44:52  hurdler
// fix "select" problem under linux
//
// Revision 1.4  2000/03/07 03:32:24  hurdler
// fix linux compilation
//
// Revision 1.3  2000/03/06 15:46:43  hurdler
// compiler warning removed
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
// NOTE:    This is not realy Os dependant because all Os have the same Socket api
//          Just use '#ifdef' for Os dependant stuffs
//
//-----------------------------------------------------------------------------


#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>

#define STD_STRING_LEN 256 // Just some standard length for a char string




#include "doomdef.h"
#include "i_system.h"
#include "i_net.h"
#include "d_net.h"
#include "m_argv.h"
#include "command.h"

#include "doomstat.h"
#include "mserv.h" //Hurdler: support master server

#define  SOCKET int
#define  INVALID_SOCKET -1

typedef union {
        struct sockaddr_in  ip;
}  mysockaddr_t;

static mysockaddr_t clientaddress[MAXNETNODES+1];
        
static SOCKET   mysocket = -1;
static boolean  nodeconnected[MAXNETNODES+1];
int sock_port = (IPPORT_USERRESERVED +0x1d );  // 5029

char *SOCK_AddrToStr(mysockaddr_t *sk)
{
    static char s[50];

    if( sk->ip.sin_family==AF_INET)
    {
        sprintf(s,"%d.%d.%d.%d:%d",((byte *)(&(sk->ip.sin_addr.s_addr)))[0],
                                   ((byte *)(&(sk->ip.sin_addr.s_addr)))[1],
                                   ((byte *)(&(sk->ip.sin_addr.s_addr)))[2],
                                   ((byte *)(&(sk->ip.sin_addr.s_addr)))[3],
                                   ntohs(sk->ip.sin_port));
    }
    else
        sprintf(s,"Unknown type");
    return s;
}

boolean UDP_cmpaddr(mysockaddr_t *a,mysockaddr_t *b)
{
    return (a->ip.sin_addr.s_addr == b->ip.sin_addr.s_addr);
}

boolean (*SOCK_cmpaddr) (mysockaddr_t *a,mysockaddr_t *b);


static int getfreenode( void )
{
    int j;

    for(j=0;j<MAXNETNODES;j++)
        if( !nodeconnected[j] )
        {
            nodeconnected[j]=true;
            return j;
        }
    return -1;
}

void SOCK_Get(void)
{
    int           i,j,c;
    size_t        fromlen;
    mysockaddr_t  fromaddress;
    
    fromlen = sizeof(fromaddress);
    c = recvfrom (mysocket, (char *)&doomcom->data, MAXPACKETLENGTH, 0,
                  (struct sockaddr *)&fromaddress, &fromlen );
    if (c == -1 )
    {
        if ( (errno==EWOULDBLOCK) || 
             (errno==EMSGSIZE)    || 
             (errno==ECONNREFUSED) )

        {
             doomcom->remotenode = (-1);      // no packet
             return;
        }
        I_Error ("SOCK_Get: %s",strerror(errno));
    }
    
//    DEBFILE(va("Get from %s\n",SOCK_AddrToStr(&fromaddress)));

    // find remote node number
    for (i=0 ; i<MAXNETNODES ; i++)
        if ( SOCK_cmpaddr(&fromaddress,&(clientaddress[i])) )
        {
            doomcom->remotenode = (i);      // good packet from a game player
            doomcom->datalength = (c);
            return;
        }

    // not found
    // find a free slot
    j=getfreenode();
    if(j>0)
    {
        memcpy(&clientaddress[j],&fromaddress,fromlen);
#ifdef DEBUGFILE
        if( debugfile )
            fprintf(debugfile,"New node detected : node:%d address:%s\n",j,SOCK_AddrToStr(&clientaddress[j]));
#endif
        doomcom->remotenode = (j); // good packet from a game player
        doomcom->datalength = (c);
        return;
    }

    // node table full
    if( debugfile )
        fprintf(debugfile,"New node detected : No more free slote\n");

    doomcom->remotenode = (-1);               // no packet
}

struct timeval timeval_for_select={0,0};
fd_set set;

// check if we can send (do not go over the buffer)
boolean SOCK_CanSend(void)
{
    // huh Boris, are you sure about the 1th argument:
    // it is the highest-numbered descriptor in any of the three
    // sets, plus 1 (I suppose mysocket + 1).
    return select(1,NULL,&set,NULL,&timeval_for_select);
}

void SOCK_Send(void)
{
    int         c;
                         
    if( !nodeconnected[(doomcom->remotenode)] )
        return;
        
    c = sendto (mysocket , (char *)&doomcom->data, (doomcom->datalength)
                ,0,(struct sockaddr *)&clientaddress[(doomcom->remotenode)]
                ,sizeof(struct sockaddr));

//    DEBFILE(va("send to %s\n",SOCK_AddrToStr(&clientaddress[doomcom->remotenode])));
    // ECONNREFUSED was send by linux port
    if (c == -1 && errno!=ECONNREFUSED && errno!=EWOULDBLOCK)
        I_Error ("SOCK_Send sending to node %d (%s): %s\n",(doomcom->remotenode),SOCK_AddrToStr(&clientaddress[(doomcom->remotenode)]),strerror(errno));
}

void SOCK_FreeNodenum(int numnode)
{
    // can't disconnect to self :)
    if(!numnode)
        return;

    if( debugfile )
        fprintf(debugfile,"Free node %d (%s)\n",numnode,SOCK_AddrToStr(&clientaddress[numnode]));

    nodeconnected[numnode]=false;

    // put invalide address
    memset(&clientaddress[numnode],0,sizeof(clientaddress[numnode]));
}

//
// UDPsocket
//
SOCKET UDP_Socket (void)
{
    SOCKET s;
    struct sockaddr_in  address;
    int    trueval = true;
    int i,j;

    // allocate a socket
    s = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s<0 || s==INVALID_SOCKET)
        I_Error ("Udp_socket: Can't create socket: %s",strerror(errno));

    memset (&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    
    //Hurdler: I'd like to put a server and a client on the same computer
    //BP: in fact for client we can use any free port we want i have read 
    //    in some doc that connect in udp can do it for us...
    if ( (i = M_CheckParm ("-clientport"))!=0 )
    {
        if( !M_IsNextParm() )
            I_Error("syntax : -clientport <portnum>");
        address.sin_port = htons(atoi(M_GetNextParm()));
    }
    else
        address.sin_port = htons(sock_port);

    if (bind (s, (struct sockaddr *)&address, sizeof(address)) == -1)
        I_Error ("UDP_Bind: %s", strerror(errno));

    // make it non blocking
    ioctl (s, FIONBIO, &trueval);

    // make it broadcastable
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *)&trueval, sizeof(trueval));

    j=4;
    getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, (size_t *)&j); // FIXME: so an int value is written to a (char *); portability!!!!!!!
    CONS_Printf("Network system buffer : %dKb\n",i>>10);

    if(i < 64<<10) // 64k
    {
        i=64<<10;
        if( setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, sizeof(i))!=0 )
            CONS_Printf("Can't set buffer lenght to 64k, file transfer will be bad\n");
        else
            CONS_Printf("Network system buffer set to : %d\n",i);
    }

    // ip + udp
    packetheaderlength=20 + 8; // for stats

    clientaddress[0].ip.sin_family      = AF_INET;
    clientaddress[0].ip.sin_port        = htons(sock_port);
    clientaddress[0].ip.sin_addr.s_addr = inet_addr("127.0.0.1");
    // setup broadcast adress to BROADCASTADDR entry
    clientaddress[BROADCASTADDR].ip.sin_family      = AF_INET;
    clientaddress[BROADCASTADDR].ip.sin_port        = htons(sock_port);
    clientaddress[BROADCASTADDR].ip.sin_addr.s_addr = INADDR_BROADCAST;

    doomcom->extratics=(1); // internet is very high ping

    SOCK_cmpaddr=UDP_cmpaddr;
    return s;
}

//Hurdler: temporary addition and changes for master server

static int init_tcp_driver = 0;

void I_InitTcpDriver(void)
{
    if (!init_tcp_driver)
    {
        init_tcp_driver = 1;
    }
}


void SOCK_CloseSocket( void )
{
    if( mysocket>=0 )
    {
    	close(mysocket);  //A.J. bug fix
        mysocket = -1;
    }
}

void I_ShutdownTcpDriver(void)
{
	if(mysocket != -1)   //A.J. possible bug fix. I_ShutdownTcpDriver never used in Mac version
	{
		SOCK_CloseSocket();
	}
    if ( init_tcp_driver )
    {
        init_tcp_driver = 0;
    }
}


int SOCK_NetMakeNode (char *hostname)
{
    int newnode;
    char *localhostname = Z_StrDup(hostname);
    char *portchar;
    int portnum = htons(sock_port);
    
    // retrieve portnum from address !
    strtok(localhostname,":");
    portchar = strtok(NULL,":");
    if( portchar )
        portnum = htons(atoi(portchar));
    free(localhostname);

    {
        struct  hostent *hostentry;      // host information entry
        char            *t;

        // remove the port in the hostname as we've it already
        t = localhostname = Z_StrDup(hostname);
        while ((*t != ':') && (*t != '\0'))
            t++;
        *t = '\0';

        newnode = getfreenode();
        if( newnode == -1 )
            return -1;
        // find ip of the server
        clientaddress[newnode].ip.sin_family      = AF_INET;
        clientaddress[newnode].ip.sin_port        = portnum;
        clientaddress[newnode].ip.sin_addr.s_addr = inet_addr(localhostname);

        if(clientaddress[newnode].ip.sin_addr.s_addr==INADDR_NONE) // not a ip ask to the dns
        {
            CONS_Printf("Resolving %s\n",localhostname);
            hostentry = gethostbyname (localhostname);
            if (!hostentry)
            {
                CONS_Printf ("%s unknow\n", localhostname);
                I_NetFreeNodenum(newnode);
                free(localhostname);
                return -1;
            }
            clientaddress[newnode].ip.sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];
        }
        CONS_Printf("Resolved %s\n",inet_ntoa(*(struct in_addr *)&clientaddress[newnode].ip.sin_addr.s_addr));
        free(localhostname);

        return newnode;
    }
}

boolean SOCK_OpenSocket( void )
{
    int i;

    memset(clientaddress,0,sizeof(clientaddress));

    for(i=0;i<MAXNETNODES;i++)
        nodeconnected[i]=false;

    nodeconnected[0] = true; // always connected to self
    nodeconnected[BROADCASTADDR] = true;
    I_NetSend        = SOCK_Send;
    I_NetGet         = SOCK_Get;
    I_NetCloseSocket = SOCK_CloseSocket;
    I_NetFreeNodenum = SOCK_FreeNodenum;
    I_NetMakeNode    = SOCK_NetMakeNode;

    mysocket = UDP_Socket ();

    // for select
    FD_ZERO(&set);
    FD_SET(mysocket,&set);

    return mysocket != -1;
}


boolean I_InitTcpNetwork( void )
{
    char     serverhostname[255];
    boolean  ret=0;
    
    // initilize the driver
    I_InitTcpDriver(); 

    if ( M_CheckParm ("-udpport") )
        sock_port = atoi(M_GetNextParm());

    // parse network game options,
    if ( M_CheckParm ("-server") )
    {
        server=true;

        // if a number of clients (i.e. nodes) is specified, the server will wait for the clients to connect before starting
        // if no number is specified here, the server starts with 1 client, others can join in-game.
        // since Boris has implemented join in-game, there is no actual need for specifying a particular number here
        // FIXME: for dedicated server, numnodes needs to be set to 0 upon start
        if( M_IsNextParm() )
            doomcom->numnodes=SHORT(atoi(M_GetNextParm()));
        else
            doomcom->numnodes=SHORT(1);

        if (SHORT(doomcom->numnodes)<1)
            doomcom->numnodes=SHORT(1);
        if (SHORT(doomcom->numnodes)>MAXNETNODES)
            doomcom->numnodes=SHORT(MAXNETNODES);

        // server
        servernode = 0;
        // FIXME:
        // ??? and now ?
        // server on a big modem ??? 4*isdn
        net_bandwidth = 16000;
        hardware_MAXPACKETLENGTH = 512;

        ret = true;
    }
    else if( M_CheckParm ("-connect") )
    {
        if(M_IsNextParm())
            strcpy(serverhostname,M_GetNextParm());
        else
            serverhostname[0]=0; // assuming server in the LAN, use broadcast to detect it

        // server address only in ip
        if(serverhostname[0])
        {
            COM_BufAddText("connect \"");
            COM_BufAddText(serverhostname);
            COM_BufAddText("\"\n");

            // probably modem
            hardware_MAXPACKETLENGTH = INETPACKETLENGTH;
        }
        else
        {
            // so we're on a LAN
            COM_BufAddText("connect any\n");

            net_bandwidth = 800000;
            hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
        }
    }

    I_NetOpenSocket = SOCK_OpenSocket;

    return ret;
}
