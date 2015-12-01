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
// $Log: d_clisrv.c,v $
// Revision 1.47  2004/07/27 08:19:34  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.46  2004/04/20 00:34:26  andyp
// Linux compilation fixes and string cleanups
//
// Revision 1.45  2003/11/22 00:22:08  darkwolf95
// get rid of FS hud pics on level exit and new game, also added exl's fix for clearing hub variables on new game
//
// Revision 1.44  2003/05/04 04:30:30  sburke
// Ensure that big-endian machines encode/decode network packets as little-endian.
//
// Revision 1.43  2003/03/22 22:35:59  hurdler
//
// Revision 1.42  2002/09/27 16:40:08  tonyd
// First commit of acbot
//
// Revision 1.41  2001/08/20 20:40:39  metzgermeister
// Revision 1.40  2001/06/10 21:16:01  bpereira
//
// Revision 1.39  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.38  2001/05/14 19:02:57  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.37  2001/04/27 13:32:13  bpereira
// Revision 1.36  2001/04/01 17:35:06  bpereira
// Revision 1.35  2001/03/30 17:12:49  bpereira
// Revision 1.34  2001/03/03 06:17:33  bpereira
// Revision 1.33  2001/02/24 13:35:19  bpereira
// Revision 1.32  2001/02/10 12:27:13  bpereira
//
// Revision 1.31  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.30  2000/11/11 13:59:45  bpereira
//
// Revision 1.29  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.28  2000/10/22 00:20:53  hurdler
// Updated for the latest master server code
//
// Revision 1.27  2000/10/21 23:21:56  hurdler
// Revision 1.26  2000/10/21 08:43:28  bpereira
//
// Revision 1.25  2000/10/17 10:09:27  hurdler
// Update master server code for easy connect from menu
//
// Revision 1.24  2000/10/16 20:02:28  bpereira
// Revision 1.23  2000/10/08 13:29:59  bpereira
// Revision 1.22  2000/10/01 10:18:16  bpereira
// Revision 1.21  2000/09/28 20:57:14  bpereira
// Revision 1.20  2000/09/15 19:49:21  bpereira
// Revision 1.19  2000/09/10 10:37:28  metzgermeister
// Revision 1.18  2000/09/01 19:34:37  bpereira
// Revision 1.17  2000/08/31 14:30:55  bpereira
//
// Revision 1.16  2000/08/21 11:06:43  hurdler
// Add ping and some fixes
//
// Revision 1.15  2000/08/16 15:44:18  hurdler
// update master server code
//
// Revision 1.14  2000/08/16 14:10:01  hurdler
// add master server code
//
// Revision 1.13  2000/08/11 19:10:13  metzgermeister
//
// Revision 1.12  2000/08/11 12:25:23  hurdler
// latest changes for v1.30
//
// Revision 1.11  2000/08/03 17:57:41  bpereira
// Revision 1.10  2000/04/30 10:30:10  bpereira
// Revision 1.9  2000/04/24 20:24:38  bpereira
// Revision 1.8  2000/04/16 18:38:06  bpereira
//
// Revision 1.7  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
//
// Revision 1.5  2000/03/08 17:02:42  hurdler
// fix the joiningame problem under Linux
//
// Revision 1.4  2000/03/06 16:51:08  hurdler
// hack for OpenGL / Open Entry problem
//
// Revision 1.3  2000/02/27 16:30:28  hurdler
// dead player bug fix + add allowmlook <yes|no>
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      DOOM Network game communication and protocol,
//      High Level Client / Server communications and functions.
//
//-----------------------------------------------------------------------------


#include <time.h>
#include <unistd.h>

#include "doomincl.h"
#include "d_clisrv.h"
#include "command.h"
#include "i_net.h"
#include "i_system.h"
#include "i_video.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "d_main.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "keys.h"
#include "doomstat.h"
#include "m_argv.h"
#include "m_menu.h"
#include "console.h"
#include "d_netfil.h"
#include "byteptr.h"

#include "p_saveg.h"
#include "p_setup.h"
#include "z_zone.h"
#include "p_tick.h"
#include "p_local.h"
#include "m_misc.h"
#include "am_map.h"
#include "m_random.h"
#include "mserv.h"
#include "t_vari.h"

#include "b_game.h"	//added by AC for acbot

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// server:
//   nettics is the tic for each node
//   next_tic_send is the lowest value of nettics
// client:
//   cl_need_tic is the tic needed by the client for run the game
//   next_tic_send is used to optimize a condition
// normaly maketic>=gametic>0,

// The addition of wait messages should be transparent to previous network
// versions.
#if 1
const int  NETWORK_VERSION = 21; // separate version number for network protocol (obsolete)
#else
const int  NETWORK_VERSION = 22; // separate version number for network protocol (obsolete)
#endif

typedef enum {
   NS_idle,
   NS_searching_server,
   NS_waiting,
   NS_active,
   NS_shutdown
} network_state_e;

static network_state_e  network_state = NS_idle;

#define PREDICTIONQUEUE         BACKUPTICS
#define PREDICTIONMASK          (PREDICTIONQUEUE-1)

// Server state
boolean  server = true; // false when Client connected to other server
boolean  serverrunning = false;
byte     serverplayer;  // 255= no server player (same as -1)

// Server specific vars.
// player=255 when unused
// nodeingame[]=false when net node is unused
static byte     player_to_nnode[MAXPLAYERS];

#if 0
static tic_t    cl_maketic[MAXNETNODES];  // unused
#endif

// Server net node state
static byte     nnode_to_player[MAXNETNODES];  // 255= unused
static byte     nnode_to_player2[MAXNETNODES]; // splitscreen player, 255= unused
static byte     playerpernode[MAXNETNODES]; // used specialy for splitscreen
static byte     nodewaiting[MAXNETNODES];
static boolean  nodeingame[MAXNETNODES];  // set false as nodes leave game
static tic_t    nettics[MAXNETNODES];     // what tic the client have received
static tic_t    nextsend_tic[MAXNETNODES]; // what server sent to client

static tic_t    next_tic_send;     // min of the nettics
static tic_t    next_tic_clear=0;  // clear next_tic_clear to next_tic_send
static tic_t    maketic;
static int16_t  consistency[BACKUPTICS];
#ifdef CLIENTPREDICTION2
tic_t localgametic;
#endif

// Client specific.
boolean         cl_drone; // client displays, no commands
static byte     cl_nnode; // net node for this client, pointofview server
static boolean  cl_packetmissed;
static tic_t    cl_need_tic;

byte            servernode = 255; // server net node, 255=none

// Index for netcmds and textcmds
#define BTIC_INDEX( tic )  ((tic)%BACKUPTICS)
// Text buffer for textcmds.
typedef struct {
   byte  buff[MAXTEXTCMD];
} textbuf_t;
  // The first byte of buff is the length.

static ticcmd_t localcmds;
static textbuf_t  localtextcmd;
static ticcmd_t localcmds2;  // player 2
static textbuf_t  localtextcmd2; // splitscreen player2

// engine
ticcmd_t        netcmds[BACKUPTICS][MAXPLAYERS];
static textbuf_t  textcmds[BACKUPTICS][MAXPLAYERS];
  // The first byte of textcmd is the length.

consvar_t cv_playdemospeed  = {"playdemospeed","0",0,CV_Unsigned};

consvar_t cv_server1 = { "server1", "192.168.1.255", CV_SAVE, NULL };
consvar_t cv_server2 = { "server2", "", CV_SAVE, NULL };
consvar_t cv_server3 = { "server3", "", CV_SAVE, NULL };

CV_PossibleValue_t downloadfiles_cons_t[] = {{0,"Allowed"}
                                           ,{1,"No Download"}
                                           ,{0,NULL}};


consvar_t cv_downloadfiles = {"downloadfiles"  ,"0", CV_SAVE, downloadfiles_cons_t};


// some software don't support largest packet
// (original sersetup, not exactly, but the probability of sending a packet
// of 512 octet is like 0.1)
uint16_t  software_MAXPACKETLENGTH;

// By Client, Server
int ExpandTics (int low)
{
    int delta;

    delta = low - (maketic&0xff);

    if (delta >= -64 && delta <= 64)
        return (maketic&~0xff) + low;
    if (delta > 64)
        return (maketic&~0xff) - 256 + low;
    if (delta < -64)
        return (maketic&~0xff) + 256 + low;
#ifdef PARANOIA
    I_SoftError ("ExpandTics: strange value %i at maketic %i\n", low, maketic);
#endif
    return 0;
}

// -----------------------------------------------------------------
//  Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

// NetXCmd indirection.
static void (*netxcmd_func[MAXNETXCMD])(char **p,int playernum);

void Register_NetXCmd(netxcmd_t cmd_id, void (*cmd_f) (char **p,int playernum))
{
#ifdef PARANOIA
   if(cmd_id >= MAXNETXCMD)
      I_Error("NetXCmd id %d exceeds defined range", cmd_id);
   if(netxcmd_func[cmd_id]!=0)
      I_Error("NetXCmd id %d already registered", cmd_id);
#endif
   netxcmd_func[cmd_id] = cmd_f;
}

void Send_NetXCmd(byte cmd_id, void *param, int nparam)
{
   if(demoplayback)
       return;

   int textlen = localtextcmd.buff[0];
   if( (textlen + 1 + nparam) > MAXTEXTCMD)
   {
#ifdef PARANOIA
       I_Error("Net command exceeds buffer size: netcmd %d\n", cmd_id);
#else
       CONS_Printf("\2Net Command exceeds buffer\n");
#endif
       return;
   }
   // Append
   textlen++;
   localtextcmd.buff[textlen] = cmd_id;
   if(param && nparam)
   {
       memcpy(&localtextcmd.buff[textlen+1], param, nparam);
       textlen += nparam;
   }
   localtextcmd.buff[0] = textlen;
}

// splitscreen player
void Send_NetXCmd2(byte cmd_id, void *param, int nparam)
{
   if(demoplayback)
       return;

   int textlen = localtextcmd2.buff[0];
   if( (textlen + 1 + nparam) > MAXTEXTCMD)
   {
#ifdef PARANOIA
       I_Error("Net command exceeds buffer size: netcmd %d\n", cmd_id);
#else
       CONS_Printf("\2Net Command fail\n");
#endif
       return;
   }
   // Append
   textlen++;
   localtextcmd2.buff[textlen] = cmd_id;
   if(param && nparam)
   {
       memcpy(&localtextcmd2.buff[textlen+1], param, nparam);
       textlen += nparam;
   }
   localtextcmd2.buff[0] = textlen;
}


// NetXCmd with 2 parameters.
void Send_NetXCmd_p2(byte cmd_id, byte param1, byte param2)
{
    byte buf[3];

    buf[0] = param1;
    buf[1] = param2;
    Send_NetXCmd(cmd_id, &buf, 2);
}


static void D_Clear_ticcmd(int tic)
{
    int i;
    int btic = BTIC_INDEX( tic );

    for(i=0;i<MAXPLAYERS;i++)
    {
        textcmds[btic][i].buff[0]=0;  // textlen
        netcmds[btic][i].angleturn = 0; //&= ~TICCMD_RECEIVED;
    }
    DEBFILE(va("Clear tic %5d [%2d]\n", tic, btic));
}


static void ExtraDataTicker(void)
{
    int  i;
    byte *curpos, *bufferend;
    int btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf;
    int textlen;

    for(i=0;i<MAXPLAYERS;i++)
    {
        if((playeringame[i]) || (i==0))
        {
	    textbuf = & textcmds[btic][i];
	    textlen = textbuf->buff[0];
            curpos=(byte *)&(textbuf->buff[1]);  // start of text
	    // [WDJ] need check for buffer overrun here !!
            bufferend = & textbuf->buff[textlen+1];  // end of text
            while(curpos<bufferend)
            {
                if(*curpos < MAXNETXCMD && netxcmd_func[*curpos])
                {
		    // Execute a NetXCmd.
                    byte cmd_id=*curpos;
                    curpos++;
                    DEBFILE(va("Executing x_cmd %d ply %d ", cmd_id,i));
                    (netxcmd_func[cmd_id])((char **)&curpos,i);
                    DEBFILE("Execute done\n");
                }
                else
	        {
		    // [WDJ] Why should a bad demo command byte be fatal.
                    I_SoftError("Got unknown net/demo command [%d]=%d (max %d)\n",
                           (curpos - &(textbuf->buff[0])),
                           *curpos, textlen);
		    D_Clear_ticcmd(btic);
		    return;
		}
            }
        }
    }
}


// -----------------------------------------------------------------
//  end of extra data function
// -----------------------------------------------------------------

// -----------------------------------------------------------------
//  extra data function for lmps
// -----------------------------------------------------------------

// desciption of extradate byte of LEGACY 1.12 not the same of the 1.20
// 1.20 don't have the extradata bits fields but a byte for each command
// see XD_xxx in d_netcmd.h
//
// if extradatabit is set, after the ziped tic you find this :
//
//   type   |  description
// ---------+--------------
//   byte   | size of the extradata
//   byte   | the extradata (xd) bits : see XD_...
//            with this byte you know what parameter follow
// if(xd & XDNAMEANDCOLOR)
//   byte   | color
//   char[MAXPLAYERNAME] | name of the player
// endif
// if(xd & XD_WEAPON_PREF)
//   byte   | original weapon switch : boolean, true if use the old
//          | weapon switch methode
//   char[NUMWEAPONS] | the weapon switch priority
//   byte   | autoaim : true if use the old autoaim system
// endif
boolean AddLmpExtradata(byte **demo_point, int playernum)
{
    int  btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf = & textcmds[btic][playernum];
    int  textlen = textbuf->buff[0];

    if(textlen == 0)  // anything in the buffer
        return false;

    memcpy(*demo_point, textbuf->buff, textlen+1);
    *demo_point += textlen+1;
    return true;
}

void ReadLmpExtraData(byte **demo_pointer, int playernum)
{
    unsigned char nextra,ex;
    int btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf = & textcmds[btic][playernum];

    if(!demo_pointer)
    {
        textbuf->buff[0] = 0;  // text len
        return;
    }
    nextra=**demo_pointer;
    if(demoversion==112) // support old demos v1.12
    {
        int  textlen = 0;
        byte  *p = *demo_pointer+1; // skip nextra

        ex=*p++;
        if(ex & 1)
        {
            textbuf->buff[textlen+1] = XD_NAMEANDCOLOR;
            memcpy(&textbuf->buff[textlen+2],
                   p,
                   MAXPLAYERNAME+1);
            p+=MAXPLAYERNAME+1;
	    textlen += MAXPLAYERNAME+1+1;
        }
        if(ex & 2)
        {
            textbuf->buff[textlen+1] = XD_WEAPONPREF;
            memcpy(&textbuf->buff[textlen+2],
                   p,
                   NUMWEAPONS+2);
            p+=NUMWEAPONS+2;
            textlen += NUMWEAPONS+2+1;
        }
        textbuf->buff[0] = textlen;
        nextra--;
    }
    else
        memcpy(textbuf->buff, *demo_pointer, nextra+1);
    // increment demo pointer
    *demo_pointer +=nextra+1;
}

// -----------------------------------------------------------------
//  end extra data function for lmps
// -----------------------------------------------------------------

// ----- Server/Client Responses

// Client state
typedef enum {
   CLM_searching,
   CLM_download_req,
   CLM_downloadfiles,
   CLM_askjoin,
   CLM_waitjoinresponse,
   CLM_downloadsavegame,
   CLM_connected
} cl_mode_t;

static cl_mode_t  cl_mode = CLM_searching;

static int16_t  Consistency(void);
static void Net_Packet_Handler(void);


// By Client.
// Send a request to join game.
// Called by CL_ConnectToServer.
static boolean  CL_Send_Join( void )
{
    CONS_Printf("Send join request...\n");
    netbuffer->packettype=PT_CLIENTJOIN;

    // Declare how many players at this node.
    if (cl_drone)
        netbuffer->u.clientcfg.localplayers=0;
    else
    if (cv_splitscreen.value)
        netbuffer->u.clientcfg.localplayers=2;
    else
        netbuffer->u.clientcfg.localplayers=1;

    netbuffer->u.clientcfg.version = VERSION;
    netbuffer->u.clientcfg.subversion = LE_SWAP32_FAST(NETWORK_VERSION);

    return HSendPacket(servernode,true,0,sizeof(clientconfig_pak));
}


// By Server.
// Reply to request for server info.
//   reqtime : the send time of the request
static void SV_Send_ServerInfo(int to_node, tic_t reqtime)
{
    byte  *p;

    netbuffer->packettype=PT_SERVERINFO;
    netbuffer->u.serverinfo.version = VERSION;
    netbuffer->u.serverinfo.subversion = LE_SWAP32_FAST(NETWORK_VERSION);
    // return back the time value so client can compute there ping
    netbuffer->u.serverinfo.trip_time = LE_SWAP32_FAST(reqtime);
    netbuffer->u.serverinfo.numberofplayer = doomcom->numplayers;
    netbuffer->u.serverinfo.maxplayer = cv_maxplayers.value;
    netbuffer->u.serverinfo.load = 0;        // unused for the moment
    netbuffer->u.serverinfo.deathmatch = cv_deathmatch.value;
    strncpy(netbuffer->u.serverinfo.servername, cv_servername.string, MAXSERVERNAME);
    if(game_map_filename[0])
    {
        // Map command external wad file.
        strcpy(netbuffer->u.serverinfo.mapname,game_map_filename);
    }
    else
    {
        // existing map       
        strcpy(netbuffer->u.serverinfo.mapname,G_BuildMapName(gameepisode,gamemap));
    }

    p=Put_Server_FileNeed();

    HSendPacket(to_node, false, 0, p-((byte *)&netbuffer->u));
}


// By Server.
// Accept player joining the game.
static boolean SV_Send_ServerConfig(int to_node)
{
    int   i,playermask=0;
    byte  *p;

    netbuffer->packettype=PT_SERVERCFG;
    for(i=0;i<MAXPLAYERS;i++)
    {
         if(playeringame[i])
              playermask|=1<<i;
    }

    netbuffer->u.servercfg.version         = VERSION;
    netbuffer->u.servercfg.subversion      = LE_SWAP32_FAST(NETWORK_VERSION);

    netbuffer->u.servercfg.serverplayer    = serverplayer;
    netbuffer->u.servercfg.totalplayernum  = doomcom->numplayers;
    netbuffer->u.servercfg.playerdetected  = LE_SWAP32_FAST(playermask);
    netbuffer->u.servercfg.gametic         = LE_SWAP32_FAST(gametic);
    netbuffer->u.servercfg.clientnode      = to_node;
    netbuffer->u.servercfg.gamestate       = gamestate;
    p = netbuffer->u.servercfg.netcvarstates;
    CV_SaveNetVars((char**)&p);
    // p is 1 past last cvar (if none then is at netcvarstates)

    return HSendPacket(to_node, true, 0, p-((byte *)&netbuffer->u));
}

#define JOININGAME
#ifdef JOININGAME

// By Server.
// Send a save game to the client.
static void SV_Send_SaveGame(int to_node)
{
    size_t  length;

    P_Alloc_savebuffer( 1 );	// large buffer, but no header
    if(! savebuffer)   return;
    // No savegame header
   
    P_SaveGame();  // fill buffer with game data
    // buffer will automatically grow as needed.

    length = P_Savegame_length();
    if( length < 0 )   return;	// overrun buffer
   
    // then send it !
    SV_SendData(to_node, savebuffer, length, TAH_MALLOC_FREE, 0);
    // SendData frees the savebuffer using free() after it is sent.
    // This is the only use of TAH_MALLOC_FREE.
}

static const char *tmpsave="$$$.sav";

// By Client.
// Act upon the received save game from server.
static void CL_Load_Received_Savegame(void)
{
    // Use savebuffer and save_p from p_saveg.c.
    // There cannot be another savegame in progress when this occurs.
    // [WDJ] Changed to use new load savegame file, with smaller buffer.
    if( P_Savegame_Readfile( tmpsave ) < 0 )  goto cannot_read_file;
    // file is open and savebuffer allocated
    // No Header on network sent savegame

    CONS_Printf("loading savegame\n");

    G_Downgrade (VERSION);

    paused        = false;
    demoplayback  = false;
    automapactive = false;

    // load a base level
    playerdeadview = false;

    P_LoadGame(); // read game data in savebuffer, defer error test
    if( P_Savegame_Closefile( 0 ) < 0 )  goto load_failed;
    // savegame buffer deallocated, and file closed

    // done
    unlink(tmpsave);  // delete file
    consistency[ BTIC_INDEX( gametic ) ] = Consistency();
    CON_ToggleOff ();
    return;

cannot_read_file:
    I_SoftError ("Can't read savegame sent\n");
    goto failed_exit; // must deallocate savebuffer

load_failed:
    CONS_Printf("Can't load the level !!!\n");
failed_exit:
    // needed when there are error tests before Closefile.
    P_Savegame_Error_Closefile();  // deallocate savebuffer
    return;
}


#endif

// ----- Wait for Server to start net game.
//#define WAITPLAYER_DEBUG

static byte  num_netnodes;
static byte  wait_nodes = 0;
static tic_t wait_tics  = 0;
static tic_t prev_tic = 0;

static void SV_Send_NetWait( void )
{
    int nn;

    netbuffer->packettype = PT_NETWAIT;
    netbuffer->u.netwait.num_netnodes = num_netnodes;
    netbuffer->u.netwait.wait_nodes = wait_nodes;
    netbuffer->u.netwait.wait_tics = LE_SWAP16( wait_tics );
    netbuffer->u.netwait.p_rand_index = P_GetRandIndex(); // to sync P_Random
#ifdef WAITPLAYER_DEBUG
    GenPrintf( EMSG_debug, "WaitPlayer update: wait_nodes=%d  num_netnodes=%d  wait_tics=%d\n",
	       num_netnodes, wait_nodes, wait_tics );
#endif
    for(nn=1; nn<MAXNETNODES; nn++)
    {
        if(nodeingame[nn])
        {
	    HSendPacket(nn, false, 0, sizeof(netwait_pak));
#ifdef WAITPLAYER_DEBUG
	    GenPrintf( EMSG_debug, "  sent to player[ %d ]\n", nn );
#endif
	}
    }
}

void D_WaitPlayer_Drawer( void )
{
    WI_draw_wait( num_netnodes, wait_nodes, wait_tics );
}

void D_WaitPlayer_Setup( void )
{
    if( netgame )
    {
        if( server )
        {
	    // Wait for all player nodes, during netgame.
	    wait_nodes = cv_wait_players.value;
	    wait_tics = cv_wait_timeout.value * TICRATE;
	}
        else
        {
	    // Wait indefinite, until server updates the wait.
	    wait_nodes = 99;
	    wait_tics = 0;
        }
    }
    else
    {
        // Single player and local games.
        wait_nodes = 0;
        wait_tics = 0;
    }
    gamestate = wipegamestate = GS_WAITINGPLAYERS;
}

// Return true when start game.
static boolean  D_WaitPlayer_Ticker()
{
    int  nn;

    if( server )
    {
        // Count the net nodes.
        num_netnodes=0;
        for(nn=0; nn<MAXNETNODES; nn++)
        {
	    // Only counting nodes with players.
	    if(nodeingame[nn])
	    {
	        if( playerpernode[nn] > 0 )
		    num_netnodes++;
	    }
	}

        if( wait_tics > 0 || wait_nodes > 0 )
        {
	    // Service the wait tics.
	    if( wait_tics > 1 )
	    {
	        wait_tics--;  // count down to 1
	    }

	    static  byte net_update_cnt = 0;
	    if( ++net_update_cnt > 4 )
	    {
	        net_update_cnt = 0;
	        // Update all net nodes
	        SV_Send_NetWait();
	    }
	}
    }

    // Clients and Server do same tests.
    if( wait_nodes )
    {
        // Waiting on player net nodes, with or without timeout.
        if( num_netnodes < wait_nodes )
        {
	    // Waiting for player net nodes.
	    if( wait_tics != 1 )  // timeout at 1
	       goto wait_ret;  // waiting only for number of players
	}
    }
    else if( wait_tics > 1 )
        goto wait_ret;  // waiting for players by timeout

    if( server )
    {
        // All nodes need to get info to stop waiting.
        SV_Send_NetWait();
#ifdef WAITPLAYER_DEBUG
        GenPrintf( EMSG_debug, "Start game sent to players at tic=%d\n", gametic   );
#endif
    }
    return true;  // start game
    

wait_ret:
    return false;  // keep waiting
}

boolean  D_WaitPlayer_Response( int key )
{
    // User response handler
    switch( key )
    {
     case 'q':
     case KEY_ESCAPE:
        if( ! dedicated )
        {
	    D_Quit_NetGame();
	    SV_StopServer();
	    SV_ResetServer();
	    D_StartTitle();
	    netgame = multiplayer = false;
	    return true;
	}
        break;
     case 's':
        if( server )
        {
	    // Start game, stop waiting for player nodes.
	    wait_nodes = 0;
	    wait_tics = 0;
	    SV_Send_NetWait();
#ifdef WAITPLAYER_DEBUG
	    GenPrintf( EMSG_debug, "Start game (key) sent at tic=%d\n", gametic );
#endif
	    return true;
	}
        break;
    }
    return false;
}


// ----- Connect to Server

// By Client.
// Ask the server for some info.
//   to_node : when BROADCASTADDR then all servers will respond
static void CL_Send_AskInfo( int to_node )
{
    netbuffer->packettype = PT_ASKINFO;
    netbuffer->u.askinfo.version = VERSION;
    netbuffer->u.askinfo.send_time = LE_SWAP32_FAST(I_GetTime());
    HSendPacket(to_node, false, 0, sizeof(askinfo_pak));
}


// By Client.
// Broadcast to find some servers.
//   addrstr: broadcast addr string
static void CL_Broadcast_AskInfo( char * addrstr )
{
    // Modifies the broadcast address.
    if( addrstr
        && Bind_Node_str( BROADCASTADDR, addrstr ) )
    {
        CL_Send_AskInfo( BROADCASTADDR );
    }
}


// --- ServerList

server_info_t serverlist[MAXSERVERLIST];
int serverlistcount=0;

// Clear the serverlist, closing connections.
//  connectedserver: except this server
static void SL_Clear_ServerList( int connectedserver )
{
    int i;
    for( i=0; i<serverlistcount; i++ )
    {
        if( connectedserver != serverlist[i].server_node )
        {
            Net_CloseConnection(serverlist[i].server_node);
            serverlist[i].server_node = 0;
        }
    }
    serverlistcount = 0;
}

// Find the server in the serverlist.
static int SL_Find_Server( byte nnode )
{
    int i;
    for( i=0; i<serverlistcount; i++ )
    {
        if( serverlist[i].server_node == nnode )
            return i;
    }

    return -1;
}

// Insert the server into the serverlist.
static void SL_InsertServer( serverinfo_pak *info, byte nnode)
{
    tic_t  test_time;
    server_info_t se;
    int i, i2;

    // search if not already on it
    i = SL_Find_Server( nnode );
    if( i==-1 )
    {
        // not found add it
        if( serverlistcount >= MAXSERVERLIST )
            return; // list full
        i=serverlistcount++;  // last entry
    }

    // Update info
    serverlist[i].info = *info;
    serverlist[i].server_node = nnode;

    // List is sorted by trip_time (has been converted to ping time)
    // so move the entry until it is sorted (shortest time to [0]).
    se = serverlist[i];  // this is always the updated entry
    test_time = info->trip_time;
    for(;;) {
        i2 = i;  // prev position of updated entry
        if( i>0
	    && test_time < serverlist[i-1].info.trip_time )
        {
            i--;
        }
        else
        if( (i+1)<serverlistcount
	    && test_time > serverlist[i+1].info.trip_time )
        {
            i++;
        }
        else
	    break;  // done
        serverlist[i2] = serverlist[i];  // move other to prev position
        serverlist[i] = se;  // new position
    }
}

// By user, future Client.
// Called by M_Connect.
void CL_Update_ServerList( boolean internetsearch )
{
    int  i;

    SL_Clear_ServerList(0);

    if( !netgame )
    {
        I_NetOpenSocket();
        netgame = true;
        multiplayer = true;
        network_state = NS_searching_server;
    }

    // Search for local servers.
    CL_Broadcast_AskInfo( cv_server1.string );
    CL_Broadcast_AskInfo( cv_server2.string );
    CL_Broadcast_AskInfo( cv_server3.string );

    if( internetsearch )
    {
        msg_server_t *server_list;

        server_list = MS_Get_ShortServersList();
        if( server_list )
        {
	    // Poll the servers on the list to get ping time.
            for (i=0; server_list[i].header[0]; i++)
            {
                int  node;
                char addr_str[24];

                // insert ip (and optionaly port) in node list
                sprintf(addr_str, "%s:%s", server_list[i].ip, server_list[i].port);
                node = I_NetMakeNode(addr_str);
                if( node < 0 )
                    break; // no more node free
                CL_Send_AskInfo( node );
            }
        }
    }
}


// ----- Connect to Server

// By User, future Client, and by server not dedicated.
// Use adaptive send using net_bandwidth and stat.sendbytes.
// Called by Command_connect, SV_SpawnServer
//  servernode: if set then reconnect, else search
static void CL_ConnectToServer( void )
{
    int  i;
    tic_t   askinfo_tic;  // to repeat askinfo

    cl_mode = CLM_searching;
    D_WaitPlayer_Setup();

    CONS_Printf("Press Q or ESC to abort\n");
    if( servernode >= MAXNETNODES )
    {
        // init value and BROADCASTADDR
        CONS_Printf("Searching the server...\n");
    }
    else
        CONS_Printf("Contacting the server...\n");

    DEBFILE(va("Waiting %d nodes\n", wait_nodes));

    askinfo_tic = 0;
    SL_Clear_ServerList(servernode);  // clear all except the current server
    // Every player goes through here to connect to game, including a
    // single player on the server.
    // Loop until connected or user escapes.
    // Because of the combination above, this loop must include code for
    // server responding.
    do {
        switch(cl_mode) {
            case CLM_searching :
                // serverlist is updated by GetPacket function
                if( serverlistcount <= 0 )
	        {
		    // Don't have a serverlist.
		    // Poll the server (askinfo packet).
		    if( askinfo_tic <= I_GetTime() )
		    {
		        // Don't be noxious on the network.
		        // Every 2 seconds is often enough.
		        askinfo_tic = I_GetTime() + (TICRATE*2);
		        if( servernode < MAXNETNODES )
		        {
			    // Specific server.
			    CL_Send_AskInfo(servernode);
			}
		        else
		        {
			    // Any
			    CL_Update_ServerList( false );
			}
		    }
                }
		else
                {
		    // Have a serverlist, serverlistcount > 0.
                    // This can be a response to our broadcast request
                    if( servernode >= MAXNETNODES )
                    {
		        // Invalid servernode, get best server from serverlist.
                        i = 0;
                        servernode = serverlist[i].server_node;
                        CONS_Printf("Found, ");
                    }
                    else
                    {
		        // Have a current server.  Find it in the serverlist.
                        i = SL_Find_Server(servernode);
		        // Check if it shutdown, or is missing for some reason.
                        if (i<0)
                            break; // the case
                    }
		    // Check server for files needed.
                    CL_Got_Fileneed(serverlist[i].info.num_fileneed,
				    serverlist[i].info.fileneed    );
                    CONS_Printf("Checking files...\n");
                    switch( CL_CheckFiles() )
		    {
		     case CFR_no_files:
		     case CFR_all_found:
                        cl_mode = CLM_askjoin;
		        break;
                     case CFR_download_needed:
		        cl_mode = CLM_download_req;
		        break;
		     case CFR_iwad_error: // cannot join for some reason
		     case CFR_insufficient_space:
		     default:
		        goto reset_to_title_exit;
		    }
                    break;
                }
                break;
	    case CLM_download_req:
	        // Must download something.
	        // Check -nodownload switch, or request downloads.
	        switch( Send_RequestFile() )
	        {
		 case RFR_success:
		    cl_mode = CLM_downloadfiles;
		    break;
		 case RFR_send_fail:
		    break;  // retry later
		 case RFR_insufficient_space: // Does not seem to be used.
		 case RFR_nodownload:
		 default:
		    // Due to -nodownload switch, or other fatal error.
		    goto  reset_to_title_exit;
		}
	        break;
            case CLM_downloadfiles :
                if( CL_waiting_on_fileneed() )
                    break; // continue looping
	        // Have all downloaded files.
	        cl_mode = CLM_askjoin;
	        // continue into next case
            case CLM_askjoin :
                if( ! CL_Load_ServerFiles() )
	        {
		    // Cannot load some file.
		    goto  reset_to_title_exit;
		}
#ifdef JOININGAME
                // prepare structures to save the file
                // WARNING: this can be useless in case of server not in GS_LEVEL
                // but since the network layer don't provide ordered packet ...
		// This can be repeated, if CL_Send_Join fails.
                CL_Prepare_Download_SaveGame(tmpsave);
#endif
                if( CL_Send_Join() )  // join game
                    cl_mode = CLM_waitjoinresponse;
                break;
            case CLM_waitjoinresponse :
	        // see server_cfg_handler()
	        break;
#ifdef JOININGAME
            case CLM_downloadsavegame :
                if( cl_fileneed[0].status != FS_FOUND )
	            break; // continue loop

	        // Have received the savegame from the server.
	        CL_Load_Received_Savegame();
	        gamestate = GS_LEVEL;  // game loaded
	        cl_mode = CLM_connected;
                // don't break case continue to CLM_connected
#endif
            case CLM_connected :
                break;
        }

        Net_Packet_Handler();
        if( !server && !netgame )
	    goto reset_to_searching;  // connection closed by cancel or timeout

        Net_AckTicker();

        // Operations performed only once every tic.
        if( prev_tic != I_GetTime() )
        {
	    prev_tic = I_GetTime();

	    // User response handler
            I_OsPolling();
	    switch( I_GetKey() )
	    {
	      case 'q':
	      case KEY_ESCAPE:
	         goto quit_ret;
	    }

	    if( Filetx_file_cnt )  // File download in progress.
	        Filetx_Ticker();
	   
#if 0
	    // Supporting the wait during connect, like it was in the previous
	    // code, has marginal value.  Seems to cause more problems.
	    D_WaitPlayer_Ticker( 0 );
	    if( wait_tics > 0 || wait_nodes > 0 )
	        D_WaitPlayer_Drawer();
#endif
            CON_Drawer ();
            I_FinishUpdate ();              // page flip or blit buffer
        }
    } while ( cl_mode != CLM_connected );

    DEBFILE(va("Synchronization Finished\n"));

    consoleplayer&= ~DRONE;
    displayplayer = consoleplayer;
    consoleplayer_ptr = displayplayer_ptr = &players[consoleplayer];
    return;

reset_to_searching:
    cl_mode = CLM_searching;
    return;

quit_ret:
    M_SimpleMessage ("Network game synchronization aborted.\n\nPress ESC\n");
    goto reset_to_title_exit;

reset_to_title_exit:
    CL_Reset();
    D_StartTitle();
    return;
}


// By User, future Client.
void Command_connect(void)
{
    if( COM_Argc()<2 )
    {
        CONS_Printf ("connect <serveraddress> : connect to a server\n"
                     "connect ANY : connect to the first lan server found\n"
                     "connect SELF: connect to self server\n");
        return;
    }
    server = false;

    if( strcasecmp(COM_Argv(1),"self")==0 )
    {
        servernode = 0;  // server is self
        server = true;
        // should be but...
        //SV_SpawnServer();
    }
    else
    {
        // used in menu to connect to a server in the list
        if( netgame && strcasecmp(COM_Argv(1),"node")==0 )
        {
	    // example: "connect node 4"
            servernode = atoi(COM_Argv(2));
	}
        else
        if( netgame )
        {
            CONS_Printf("You cannot connect while in netgame\n"
                        "Leave this game first\n");
            return;
        }
        else
        {
            I_NetOpenSocket();
            netgame = true;
            multiplayer = true;
        
            if( strcasecmp(COM_Argv(1),"any")==0 )
	    {
	        // Connect to first lan server found.
                servernode = BROADCASTADDR;
	    }
            else
            if( I_NetMakeNode )
	    {
	        // Connect to server at IP addr.
                servernode = I_NetMakeNode(COM_Argv(1));
	    }
            else
            {
                CONS_Printf("There is no server identification with this network driver\n");
                D_CloseConnection();
                return;
            }
        }
    }
    CL_ConnectToServer();
}

static void Reset_NetNode(byte nnode);

// Called by Kick cmd.
static void CL_RemovePlayer(int playernum)
{
    int i;
    if( server && !demoplayback )
    {
        byte nnode = player_to_nnode[playernum];
        if( playerpernode[nnode] )
	    playerpernode[nnode]--;
        if( playerpernode[nnode] == 0 )
        {
            nodeingame[player_to_nnode[playernum]] = false;
            Net_CloseConnection(player_to_nnode[playernum]);
            Reset_NetNode(nnode);
        }
    }

    // we should use a reset player but there is not such function
    for(i=0;i<MAXPLAYERS;i++)
    {
        players[i].addfrags += players[i].frags[playernum];
        players[i].frags[playernum] = 0;
        players[playernum].frags[i] = 0;
    }
    players[playernum].addfrags = 0;

    // remove avatar of player
    if( players[playernum].mo )
    {
        players[playernum].mo->player = NULL;
        P_RemoveMobj (players[playernum].mo);
    }
    players[playernum].mo = NULL;
    playeringame[playernum] = false;
    player_to_nnode[playernum] = -1;
    while(playeringame[doomcom->numplayers-1]==0
	  && doomcom->numplayers>1)
    {
        doomcom->numplayers--;
    }
}

// By Client and non-specific code, to reset client connect.
void CL_Reset (void)
{
    if (demorecording)
        G_CheckDemoStatus ();

    // reset client/server code
    DEBFILE(va("==== Client reset ====\n"));

    if( servernode < MAXNETNODES )
    {
        // Close connection to server
        nodeingame[servernode]=false;
        Net_CloseConnection(servernode);
    }
    D_CloseConnection();         // netgame=false
    multiplayer = false;
    servernode=0;  // server to self
    server=true;
    doomcom->num_player_netnodes=1;
    doomcom->numplayers=1;
    SV_StopServer();
    SV_ResetServer();

    T_ClearHubScript();	//DarkWolf95: Originally implemented by Exl
    fadealpha = 0;
    HU_ClearFSPics();

    // reset game engine
    //D_StartTitle ();
}

void Command_PlayerInfo(void)
{
    int i;

    for(i=0;i<MAXPLAYERS;i++)
    {
        if(playeringame[i])
        {
            if(serverplayer==i)
	    {
                CONS_Printf("\2num:%2d  node:%2d  %s\n",
			    i, player_to_nnode[i], player_names[i]);
	    }
            else
	    {
                CONS_Printf("num:%2d  node:%2d  %s\n",
			    i, player_to_nnode[i], player_names[i]);
	    }
        }
    }
}

// Name can be player number, or player name.
// Players 0..(MAXPLAYERS-1) are known as Player 1 .. to the user.
// Return player number, 0..(MAXPLAYERS-1).
// Return 255, and put msg to console, when name not found.
byte player_name_to_num(char *name)
{
    // Player num can be 0..250 (limited to MAXPLAYERS).
    int playernum, i;

    playernum=atoi(name);   // test as player number 1..MAXPLAYERS
    if((playernum > 0) && (playernum <= MAXPLAYERS))
    {
        playernum --;  // convert to 0..MAXPLAYERS
        if(playeringame[playernum])
            return playernum;
        goto no_player;
    }

    // Search for player by name.
    for(i=0;i<MAXPLAYERS;i++)
    {
        if(playeringame[i] && strcasecmp(player_names[i],name)==0)
            return i;
    }
    
no_player:   
    CONS_Printf("There is no player named\"%s\"\n",name);
    return 255;
}

// network kick message codes
typedef enum {
  KICK_MSG_GO_AWAY     = 1,
  KICK_MSG_CON_FAIL    = 2,
  KICK_MSG_PLAYER_QUIT = 3,
  KICK_MSG_TIMEOUT     = 4,
} kick_msg_e;

void Command_Kick(void)
{
    if (COM_Argc() != 2)
    {
        CONS_Printf ("kick <playername> or <playernum> : kick a player\n");
        return;
    }

    if(server)
    {
        int pn = player_name_to_num(COM_Argv(1));
        if(pn < MAXPLAYERS)
	   Send_NetXCmd_p2(XD_KICK, pn, KICK_MSG_GO_AWAY);
    }
    else
    {
        CONS_Printf("You are not the server\n");
    }
}

void Got_NetXCmd_KickCmd(char **p, int playernum)
{
    int pnum=READBYTE(*p);  // unsigned player num
    int msg =READBYTE(*p);  // unsigned kick message

    CONS_Printf("\2%s ",player_names[pnum]);

    switch(msg)
    {
       case KICK_MSG_GO_AWAY:
               CONS_Printf("has been kicked (Go away)\n");
               break;
       case KICK_MSG_CON_FAIL:
               CONS_Printf("has been kicked (Consistency failure)\n");
               break;
       case KICK_MSG_TIMEOUT:
               CONS_Printf("left the game (Connection timeout)\n");
               break;
       case KICK_MSG_PLAYER_QUIT:
               CONS_Printf("left the game\n");
               break;
    }
    if( pnum==consoleplayer )
    {
        CL_Reset();
        D_StartTitle();
        M_SimpleMessage("You have been kicked by the server\n\nPress ESC\n");
    }
    else
    {
        CL_RemovePlayer(pnum);
    }
}

CV_PossibleValue_t maxplayers_cons_t[]={{1,"MIN"},{32,"MAX"},{0,NULL}};

consvar_t cv_allownewplayer = {"sv_allownewplayers","1",0,CV_OnOff};
consvar_t cv_maxplayers     = {"sv_maxplayers","32",CV_NETVAR,maxplayers_cons_t,NULL,32};

void Got_NetXCmd_AddPlayer(char **p, int playernum);
void Got_NetXCmd_AddBot(char **p,int playernum);	//added by AC for acbot

// Called one time at init, by D_Startup_NetGame.
void D_Init_ClientServer (void)
{
  DEBFILE(va("==== %s debugfile ====\n", VERSION_BANNER));

    cl_drone = false;

    // drone server generating the left view of three screen view
    if(M_CheckParm("-left"))
    {
        cl_drone = true;
        viewangleoffset = ANG90;
    }
    // drone server generating the right view of three screen view
    if(M_CheckParm("-right"))
    {
        cl_drone = true;
        viewangleoffset = -ANG90;
    }
    // [WDJ] specify secondary screen angle in degrees (general case of left/right)
    if(M_CheckParm("-screendeg"))
    {
        cl_drone = true;
        if( M_IsNextParm() )
        {
	    // does not accept negative numbers, use 270, 315, etc
	    viewangleoffset = atoi(M_GetNextParm()) * (ANG90 / 90);
	    // it is cheating to have screen looking behind player
	    if( viewangleoffset < -ANG90 )  viewangleoffset = -ANG90;
	    if( viewangleoffset > ANG90 )  viewangleoffset = ANG90;
	}
    }
//    GenPrintf(EMSG_debug, "viewangleoffset=%i\n", viewangleoffset );

    COM_AddCommand("playerinfo",Command_PlayerInfo);
    COM_AddCommand("kick",Command_Kick);
    COM_AddCommand("connect",Command_connect);

    Register_NetXCmd(XD_KICK, Got_NetXCmd_KickCmd);
    Register_NetXCmd(XD_ADDPLAYER, Got_NetXCmd_AddPlayer);
    Register_NetXCmd(XD_ADDBOT, Got_NetXCmd_AddBot);	//added by AC for acbot
    CV_RegisterVar (&cv_allownewplayer);
    CV_RegisterVar (&cv_maxplayers);

    gametic = 0;
    localgametic = 0;

    // do not send anything before the real begin
    SV_StopServer();  // as an Init
}

// nnode: 0..(MAXNETNODES-1)
static void Reset_NetNode(byte nnode)
{
    nodeingame[nnode] = false;
    nnode_to_player[nnode] = 255;
    nnode_to_player2[nnode] = 255;
    nettics[nnode]=gametic;
    nextsend_tic[nnode]=gametic;
#if 0
    cl_maketic[nnode]=maketic;
#endif
    nodewaiting[nnode]=0;
    playerpernode[nnode]=0;
}

// Called by D_Init_ClientServer, SV_SpawnServer, CL_Reset, D_WaitPlayer_Response
void SV_ResetServer( void )
{
    int    i;

    // +1 because this command will be executed in com_executebuffer in
    // tryruntic so gametic will be incremented, anyway maketic > gametic 
    // is not a issue

    maketic=gametic+1;
    cl_need_tic=maketic;
#ifdef CLIENTPREDICTION2
    localgametic = gametic;
#endif
    next_tic_clear=maketic;

    for (i=0 ; i<MAXNETNODES ; i++)
        Reset_NetNode(i);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        playeringame[i]=false;
        player_to_nnode[i]=-1;
    }

    cl_nnode=0;
    cl_packetmissed=false;
    viewangleoffset=0;

    if( dedicated )
    {
        nodeingame[0]=true;
        serverplayer = 255;  // no server player
    }
    else
        serverplayer=consoleplayer;

    if(server)
        servernode=0;  // server to self

    doomcom->numplayers=0;

    DEBFILE(va("==== Server Reset ====\n"));
}

//
// D_Quit_NetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_Quit_NetGame (void)
{
    byte nn; // net node num

    if (!netgame)
        return;

    // [WDJ] This can tight loop when the network fails.
    if( network_state == NS_shutdown )
        return;
    network_state = NS_shutdown;
     
    DEBFILE("==== Quiting Game, closing connection ====\n" );

    // abort send/receive of files
    CloseNetFile();

    if( server )
    {
        // Server sends shutdown to all clients.
        netbuffer->packettype=PT_SERVERSHUTDOWN;
        for(nn=0; nn<MAXNETNODES; nn++)
        {
            if( nodeingame[nn] )
                HSendPacket(nn,true,0,0);
	}
        // Close registration with the Master Server.
        if ( serverrunning && cv_internetserver.value )
             MS_UnregisterServer(); 
    }
    else
    if( (servernode < MAXNETNODES)
       && nodeingame[servernode] )
    {
        // Client sends quit to server.
        netbuffer->packettype=PT_CLIENTQUIT;
        HSendPacket(servernode,true,0,0);
    }

    D_CloseConnection();

    DEBFILE("==== Log finish ====\n" );
#ifdef DEBUGFILE
    if (debugfile)
    {
        fclose (debugfile);
        debugfile = NULL;
    }
#endif
}

// Add a node to the game (player will follow at map change or at savegame....)
void SV_AddNode(byte nnode)
{
    nettics[nnode]       = gametic;
    nextsend_tic[nnode]  = gametic;
#if 0
    cl_maketic[nnode]    = maketic;
#endif
    // Because the server connects to itself and sets nodeingame[0],
    // do not interfere with nodeingame[0] here.
    if(nnode)
       nodeingame[nnode]=true;
}

// Xcmd XD_ADDPLAYER
void Got_NetXCmd_AddPlayer(char **p,int playernum)
{
    static uint32_t sendconfigtic = 0xffffffff;

    // [WDJ] Having error due to sign extension of byte read (signed char).
    byte nnode=READBYTE(*p);  // unsigned
    unsigned int newplayernum=READBYTE(*p);  // unsigned
    boolean splitscreenplayer = newplayernum&0x80;

    newplayernum&=0x7F;  // remove flag bit, and any sign extension

    playeringame[newplayernum]=true;
    G_AddPlayer(newplayernum);
    if( newplayernum+1 > doomcom->numplayers )
        doomcom->numplayers=newplayernum+1;
    // [WDJ] Players are 1..MAXPLAYERS to the user.
    CONS_Printf("Player %d is in the game (node %d)\n", (newplayernum+1), nnode);

    if(nnode==cl_nnode)
    {
        // The server is creating my player.
        player_to_nnode[newplayernum]=0;  // for information only
        if(!splitscreenplayer)
        {
            consoleplayer=newplayernum;
            displayplayer=newplayernum;
	    displayplayer_ptr = consoleplayer_ptr = &players[newplayernum];
            DEBFILE("Spawning me\n");
        }
        else
        {
            displayplayer2=newplayernum;
	    displayplayer2_ptr = &players[displayplayer2];
            DEBFILE("Spawning my brother\n");
        }
    }

    // the new player send there config
    // and the old player send there config to the new one
    // WARNING : this can cause a bottleneck in the txtcmd
    //           this can also produce consistency failure if packet get lost
    //           because everybody knows the actual config except the joiner
    //    TODO : fixthis

    //  Don't send config more than once per tic (more than one player join)
    if( sendconfigtic!=gametic )
    {
        sendconfigtic=gametic;
        D_Send_PlayerConfig();
    }
}

// Xcmd XD_ADDBOT
void Got_NetXCmd_AddBot(char **p, int playernum)  //added by AC for acbot
{
    // [WDJ] Having error due to sign extension of byte read (signed char).
    unsigned int newplayernum=READBYTE(*p);  // unsigned
    //int node = 0;
    //int i = 0;
    newplayernum&=0x7F;  // remove flag bit, and any sign extension
    playeringame[newplayernum]=true;
    strcpy(player_names[newplayernum], botinfo[newplayernum].name);
    players[newplayernum].skincolor = botinfo[newplayernum].colour;
    G_AddPlayer(newplayernum);
    players[newplayernum].bot = B_CreateBot();
    if( newplayernum+1>doomcom->numplayers )
        doomcom->numplayers=newplayernum+1;

    multiplayer=1;

    CONS_Printf ("Bot %s has entered the game\n", player_names[newplayernum]);
}

// By Server.
// Called by SV_SpawnServer, client_join_handler.
// Return true when a new player is added.
boolean SV_AddWaitingPlayers(void)
{
    boolean  newplayer_added = false;  // return
    byte nnode, nn;
    byte newplayernum, playernum_param;

    newplayernum=0;
    for(nnode=0; nnode<MAXNETNODES; nnode++)
    {
        // splitscreen can allow 2 player in one node
        for(; nodewaiting[nnode]>0; nodewaiting[nnode]--)
        {
            newplayer_added = true;

            // Search for a free playernum.
            // We can't use playeringame since it is not updated here.
            //while(newplayernum<MAXPLAYERS && playeringame[newplayernum])
            //    newplayernum++;
            for( ;newplayernum<MAXPLAYERS; newplayernum++)
            {
                for(nn=0; nn<MAXNETNODES; nn++)
	        {
                    if( nnode_to_player[nn]  == newplayernum
                        || nnode_to_player2[nn] == newplayernum )
                        break;
		}
                if( nn == MAXNETNODES )
		    break;  // found an unused player number
            }
            
#ifdef PARANOIA
            // Should never happen because we check the number of players
	    // before accepting the join.
            if(newplayernum==MAXPLAYERS)
                I_Error("SV_AddWaitingPlayers: Reached MAXPLAYERS\n");
#endif
            player_to_nnode[newplayernum] = nnode;

            if( playerpernode[nnode]<1 )
	    {
                nnode_to_player[nnode] = newplayernum;
	        playernum_param = newplayernum;
	    }
            else
            {
                nnode_to_player2[nnode] = newplayernum;
                playernum_param = newplayernum | 0x80;  // player 2 flag
            }
            playerpernode[nnode]++;

            Send_NetXCmd_p2(XD_ADDPLAYER, nnode, playernum_param);
            if( doomcom->numplayers==0 )
                doomcom->numplayers++;  //we must send the change to other players
            
            DEBFILE(va("Server added player %d net node %d\n",
		       newplayernum, nnode));
            // use the next free slot (we can't put playeringame[j]=true here)
            newplayernum++; 
        }
    }

    return newplayer_added;
}

void CL_AddSplitscreenPlayer( void )
{
    if( cl_mode == CLM_connected )
        CL_Send_Join();  // join game
}

void CL_RemoveSplitscreenPlayer( void )
{
    if( cl_mode != CLM_connected )
        return;

    Send_NetXCmd_p2(XD_KICK, displayplayer2, KICK_MSG_PLAYER_QUIT);  // player 2
}

// Is there a game running.
boolean Game_Playing( void )
{
    return (server && serverrunning) || (!server && cl_mode==CLM_connected);
}

// By Server and Server-only commands.
// Called by D_Startup_NetGame (dedicated server).
// Called by Command_Map_f, Command_Load_f (server).
// Return true when a new player is added.
boolean SV_SpawnServer( void )
{
    D_DisableDemo();

    if( serverrunning == false )
    {
        CONS_Printf("Starting Server....\n");
        serverrunning = true;
        SV_ResetServer();
        if( netgame )
        {
            I_NetOpenSocket();
	    // Register with the Master Server.
            if( cv_internetserver.value )
	    {
	        // MasterServer address is in cv_masterserver.
                MS_RegisterServer();
	    }
        }

        D_WaitPlayer_Setup();

        // server just connect to itself
        if( !dedicated )
            CL_ConnectToServer();
    }

    return SV_AddWaitingPlayers();
}

// Called by D_Init_ClientServer, G_StopDemo, CL_Reset, SV_StartSinglePlayerServer,
// D_WaitPlayer_Response.
void SV_StopServer( void )
{
    int i;

    gamestate = wipegamestate = GS_NULL;

    localtextcmd.buff[0]=0;  // text len
    localtextcmd2.buff[0]=0; // text len

    for(i=0; i<BACKUPTICS; i++)
        D_Clear_ticcmd(i);

    consoleplayer=0;
    cl_mode = CLM_searching;
    maketic=gametic+1;
    cl_need_tic=maketic;
    serverrunning = false;
}

// called at singleplayer start and stopdemo
void SV_StartSinglePlayerServer(void)
{
    server        = true;
    netgame       = false;
    multiplayer   = false;

    // no more tic the game with this settings !
    SV_StopServer();

    if( cv_splitscreen.value )
        multiplayer    = true;
}

// By Server.
// Called by client_join_handler.
static void SV_Send_Refuse(int to_node, char *reason)
{
    strcpy(netbuffer->u.serverrefuse.reason, reason);

    netbuffer->packettype=PT_SERVERREFUSE;
    HSendPacket(to_node, true, 0, strlen(netbuffer->u.serverrefuse.reason)+1);
    Net_CloseConnection(to_node);
}

// By Server.
// PT_ASKINFO from Client.
// Handle a client request for server info.
static void server_askinfo_handler( byte nnode )
{
    if(serverrunning)
    {
        // Make the send_time the round trip ping time.
        SV_Send_ServerInfo(nnode,
			   LE_SWAP32_FAST(netbuffer->u.askinfo.send_time));
        Net_CloseConnection(nnode);  // a temp connection
    }
}

// By Server.
// PT_CLIENTJOIN from future client.
//   nnode: net node that is joining
static void client_join_handler( byte nnode )
{
#ifdef JOININGAME
    boolean newnode=false;
#endif
    if( netbuffer->u.clientcfg.version != VERSION
	|| LE_SWAP32_FAST(netbuffer->u.clientcfg.subversion) != NETWORK_VERSION)
    {
        SV_Send_Refuse(nnode,
	   va("Different DOOM versions cannot play a net game! (server version %s)",
	       VERSION_BANNER));
        return;
    }
    // nnode==0 is self, which is always accepted.
    if(!cv_allownewplayer.value && nnode!=0 )
    {
        SV_Send_Refuse(nnode,
	  "The server is not accepting people for the moment");
        return;
    }
   
    // TODO; compute it using nodewaiting and playeringame
    if( (doomcom->numplayers + 1) > cv_maxplayers.value)
    {
        SV_Send_Refuse(nnode,
	   va("Maximum of player reached (max:%d)", cv_maxplayers.value));
        return;
    }
   
    // Client authorized to join.
    nodewaiting[nnode] = netbuffer->u.clientcfg.localplayers - playerpernode[nnode];
    if(!nodeingame[nnode])
    {
        SV_AddNode(nnode);
        if(!SV_Send_ServerConfig(nnode))
        {
	    // TODO : fix this !!!
	    CONS_Printf("Internal Error 5 : client lost\n");
	    return;
	}
        DEBFILE("New node joined\n");
#ifdef JOININGAME
        newnode = true;
#endif
    }
#ifdef JOININGAME
    if( nodewaiting[nnode] )
    {
        if( (gamestate == GS_LEVEL) && newnode)
        {
	    SV_Send_SaveGame(nnode); // send game data
	    CONS_Printf("Send savegame\n");
	}
        SV_AddWaitingPlayers();
    }
#endif
}

// BY Server.
// PT_NODE_TIMEOUT, PT_CLIENTQUIT
//   nnode : the network client node quitting
//   netconsole : the player
static void client_quit_handler( byte nnode, int netconsole )
{
    // nodeingame will made false during the execution of kick command.
    // This allows the sending of some packets to the quiting client
    // and to have them ack back.
    nodewaiting[nnode]= 0;
    if(netconsole >= 0 && playeringame[netconsole])
    {
        byte reason = (netbuffer->packettype == PT_NODE_TIMEOUT) ?
	   KICK_MSG_TIMEOUT : KICK_MSG_PLAYER_QUIT;
        // Update other players by kicking nnode.
        Send_NetXCmd_p2(XD_KICK, netconsole, reason);  // kick player
        nnode_to_player[nnode] = 255;

        if( nnode_to_player2[nnode] < MAXPLAYERS )
        {
	    if( playeringame[nnode_to_player2[nnode]] )
	    {
	       // kick player2
	       Send_NetXCmd_p2(XD_KICK, nnode_to_player2[nnode], reason);
	    }
	    nnode_to_player2[nnode] = 255;
	}
    }
    Net_CloseConnection(nnode);
    nodeingame[nnode]=false;
}



// By Client
// PT_SERVERINFO from Server.
static void server_info_handler( byte nnode )
{
    // Compute ping in ms.
    netbuffer->u.serverinfo.trip_time =
     (I_GetTime() - LE_SWAP32_FAST(netbuffer->u.serverinfo.trip_time))*1000/TICRATE; 
    netbuffer->u.serverinfo.servername[MAXSERVERNAME-1]=0;

    SL_InsertServer( &netbuffer->u.serverinfo, nnode);
}


// By Client
// PT_SERVERREFUSE from Server.
static void server_refuse_handler( byte nnode )
{
    if( cl_mode == CLM_waitjoinresponse )
    {
        M_SimpleMessage(va("Server %i refuses connection\n\nReason :\n%s",
			   nnode,
			   netbuffer->u.serverrefuse.reason));
        CL_Reset();
        D_StartTitle();
    }
}


// By Client
// PT_SERVERCFG from Server.
// Received acceptance of the player node joining the game.
static void server_cfg_handler( byte nnode )
{
    int j;
    byte *p;

    if( cl_mode != CLM_waitjoinresponse )
        return;

    if(!server)
    {
        // Clients not on the server, update to server time.
        maketic = gametic = cl_need_tic = LE_SWAP32_FAST(netbuffer->u.servercfg.gametic);
    }

#ifdef CLIENTPREDICTION2
    localgametic = gametic;
#endif
    nodeingame[servernode]=true;

    // Handle a player on the server.
    serverplayer = netbuffer->u.servercfg.serverplayer;
    if (serverplayer < MAXPLAYERS)  // 255 = no player
        player_to_nnode[serverplayer] = servernode;

    doomcom->numplayers = netbuffer->u.servercfg.totalplayernum;
    cl_nnode = netbuffer->u.servercfg.clientnode;

    CONS_Printf("Join accepted, wait next map change...\n");
    DEBFILE(va("Server accept join gametic=%d, client net node=%d\n",
	       gametic, cl_nnode));

    uint32_t  playerdet = LE_SWAP32_FAST(netbuffer->u.servercfg.playerdetected);
    for(j=0;j<MAXPLAYERS;j++)
    {
        playeringame[j]=( playerdet & (1<<j) ) != 0;
    }

    p = netbuffer->u.servercfg.netcvarstates;
    CV_LoadNetVars( (char**)&p );
#ifdef JOININGAME
    cl_mode = ( netbuffer->u.servercfg.gamestate == GS_LEVEL ) ?
       CLM_downloadsavegame : CLM_connected;
#else
    cl_mode = CLM_connected;
#endif
}

// By Client
// PT_SERVERSHUTDOWN from Server.
static void server_shutdown_handler()
{
    if( cl_mode != CLM_searching )
    {
        M_SimpleMessage("Server has Shutdown\n\nPress Esc");
        CL_Reset();
        D_StartTitle();
    }
}

// By Client
// PT_NODE_TIMEOUT
static void server_timeout_handler()
{
    if( cl_mode != CLM_searching )
    {
        M_SimpleMessage("Server Timeout\n\nPress Esc");
        CL_Reset();
        D_StartTitle();
    }
}


// By Client, Server.
// Invoked by anyone trying to join !
static void unknown_host_handler( byte nnode )
{
    // Packet can be from client trying to join server,
    // or from a server responding to a join request.
    if( nnode != servernode )
    {
        // Client trying to Join.
        DEBFILE(va("Received packet from unknown host %d\n",nnode));
    }

    // The commands that are allowd by an unknown host.
    switch(netbuffer->packettype)
    {
     case PT_ASKINFO:  // client has asked server for info
        if( server )
            server_askinfo_handler( nnode );
        break;
     case PT_SERVERREFUSE : // negative response of client join request
        server_refuse_handler( nnode );
        break;
     case PT_SERVERCFG :    // positive response of client join request
        server_cfg_handler( nnode );
        break;
     // handled in d_netfil.c
     case PT_FILEFRAGMENT :
        if( !server )
	    Got_Filetxpak();
        break;
     case PT_REQUESTFILE :
        if( server )
	    Got_RequestFilePak(nnode);
        break;
     case PT_NODE_TIMEOUT:
     case PT_CLIENTQUIT:
        if( server )
	    Net_CloseConnection(nnode);
        break;
     case PT_SERVERTICS:
        if( nnode == servernode )
        {
	    // do not remove my own server
	    // (we have just to get a out of order packet)
	    break;
	}
        // Server tic from unknown source.
        // Fall through to default.
     default:
        DEBFILE(va("Unknown packet received (%d) from unknown host !\n",
		   netbuffer->packettype));
        Net_CloseConnection(nnode);  // a temp connection
        break; // ignore it
    } // switch
}


// used at txtcmds received to check packetsize bound
static int TotalTextCmdPerTic(int tic)
{
    int i,total=1; // num of textcmds in the tic (ntextcmd byte)

    int btic = BTIC_INDEX( tic );

    for(i=0;i<MAXPLAYERS;i++)
    {
        if( (i==0) || playeringame[i] )
	{
	    int textlen = textcmds[btic][i].buff[0];
	    if( textlen )
	       total += textlen + 2; // "+2" for size and playernum
	}
    }

    return total;
}

// Copy an array of ticcmd_t, swapping between host and network byte order.
//
static void TicCmdCopy(ticcmd_t * dst, ticcmd_t * src, int n)
{
    int i;
    for (i = 0; i < n; src++, dst++, i++)
    {
#ifdef CLIENTPREDICTION2
	dst->x = LE_SWAP32_FAST(src->x);
	dst->y = LE_SWAP32_FAST(src->y);
#endif
	dst->forwardmove = src->forwardmove;
	dst->sidemove    = src->sidemove;
	dst->angleturn   = LE_SWAP16_FAST(src->angleturn);
	dst->aiming      = LE_SWAP16_FAST(src->aiming);
	dst->buttons     = src->buttons;
    }
}

// By Server
// PT_TEXTCMD, PT_TEXTCMD2
//   nnode : the network client
//   netconsole : playernum
static void net_textcmd_handler( byte nnode, int netconsole )
{
    int ntextsize = netbuffer->u.textcmd[0];
    int mpsize;  // Max size of packet than can be sent with the text
    int textlen, btic;
    tic_t tic;
    textbuf_t *  textbuf;  // first byte of this buffer is the cur usage

    // Check if tic that we are making isn't too large,
    // else we cannot send it :(
    // Note: doomcom->numplayers+1 is "+1" because doomcom->numplayers
    // can change within this time and sent time.
    mpsize = software_MAXPACKETLENGTH
	 - ( ntextsize + 2 + SERVER_TIC_BASE_SIZE
	    + ((doomcom->numplayers+1) * sizeof(ticcmd_t)) );

    // Search for a tic that has enough space in the ticcmd.
    tic = maketic;
    while( TotalTextCmdPerTic(tic) > mpsize )
    {
        textbuf = & textcmds[ BTIC_INDEX( tic ) ][netconsole];
        if( (ntextsize + textbuf->buff[0]) <= MAXTEXTCMD )
	  break; // found one
        tic++;
        if( tic >= (next_tic_send+BACKUPTICS) )  goto drop_packet;
    }

    btic = BTIC_INDEX( tic );
    textbuf = & textcmds[btic][netconsole];
    textlen = textbuf->buff[0];
    DEBFILE(va("Textcmd: btic %d buff[%d] player %d nxttic %d maketic %d\n",
	       btic, textlen+1, netconsole, next_tic_send, maketic));
    // Append to the selected buffer.
    memcpy(&textbuf->buff[ textlen+1 ],
	   netbuffer->u.textcmd+1,  // the text
	   ntextsize);
    textbuf->buff[0] += ntextsize;  // text len
    return;
   
drop_packet:
    // Drop the packet, let the node resend it.
    DEBFILE(va("Textcmd too long: max %d used %d maketic %d nxttic %d node %d player %d\n",
	       mpsize, TotalTextCmdPerTic(maketic), maketic, next_tic_send,
	       nnode, netconsole));
    Net_Cancel_Packet_Ack(nnode);
    return;
}

// By Server
// PT_CLIENTCMD, PT_CLIENT2CMD, PT_CLIENTMIS, PT_CLIENT2MIS,
// PT_NODEKEEPALIVE, PT_NODEKEEPALIVEMIS from Client.
//  netconsole: a player
static void client_cmd_handler( byte netcmd, byte nnode, int netconsole )
{
    tic_t  start_tic, end_tic;
    int  btic;

    // To save bytes, only the low byte of tic numbers are sent
    // Figure out what the rest of the bytes are
    start_tic  = ExpandTics (netbuffer->u.clientpak.client_tic);
    end_tic = ExpandTics (netbuffer->u.clientpak.resendfrom);

    if(  netcmd == PT_CLIENTMIS
	 || netcmd == PT_CLIENT2MIS
	 || netcmd == PT_NODEKEEPALIVEMIS 
	 || nextsend_tic[nnode] < end_tic )
    {
        nextsend_tic[nnode] = end_tic;
    }

    // Discard out of order packet
    if( nettics[nnode] > end_tic )
    {
        DEBFILE(va("Out of order ticcmd discarded: nettics %d\n",
		   nettics[nnode]));
        return;
    }

    // Update the nettics.
    nettics[nnode] = end_tic;

    // Don't do any tic cmds for drones, just update their nettics.
    if((netconsole & DRONE) || netconsole==-1
       || netcmd==PT_NODEKEEPALIVE || netcmd==PT_NODEKEEPALIVEMIS)
       return;

    // Check consistency
    btic = BTIC_INDEX(start_tic);
    if((start_tic <= gametic)
       && (start_tic > (gametic - BACKUPTICS + 1))
       && (consistency[btic] != LE_SWAP16_FAST(netbuffer->u.clientpak.consistency)))
    {
        // Failed the consistency check.
#if 1
        Send_NetXCmd_p2(XD_KICK, netconsole, KICK_MSG_CON_FAIL);
#else
        // Debug message instead.
        GenPrintf(EMSG_warn, "Kick player %d at tic %d, consistency failure\n",
		    netconsole, start_tic);
#endif
        DEBFILE(va("Kick player %d at tic %d, consistency %d != %d\n",
		    netconsole, start_tic, consistency[btic],
		    LE_SWAP16_FAST(netbuffer->u.clientpak.consistency)));

    }

    // Copy the ticcmd
    btic = BTIC_INDEX( maketic );
    TicCmdCopy(&netcmds[btic][netconsole],
	       &netbuffer->u.clientpak.cmd, 1);

    if( netcmd==PT_CLIENT2CMD
	&& (nnode_to_player2[nnode] < MAXPLAYERS))
    {
        // Copy the ticcmd for player2.
        TicCmdCopy(&netcmds[btic][nnode_to_player2[nnode]], 
		   &netbuffer->u.client2pak.cmd2, 1);
    }
    return;
}



// By Client.
// PT_SERVERTICS from Server.
static void servertic_handler( byte nnode )
{
    tic_t  start_tic, end_tic, ti;
    ticcmd_t * ticp;  // net tics
    byte * txtp;  // net txtcmd text
    byte   num_txt;
    int    btic, j;

    start_tic = ExpandTics (netbuffer->u.serverpak.starttic);
    end_tic   = start_tic + netbuffer->u.serverpak.numtics;

    if( end_tic > (gametic + BACKUPTICS))
        end_tic = (gametic + BACKUPTICS);  // limit to backup capability

    // Check if missed any packets.
    cl_packetmissed = (start_tic > cl_need_tic);

    if((start_tic > cl_need_tic) || (end_tic <= cl_need_tic))
       goto not_in_packet;
   
    // The needed tic is within this packet.
    // Nettics
    ticp = netbuffer->u.serverpak.cmds;
    // After the nettics are the net textcmds
    txtp = (byte *)&netbuffer->u.serverpak.cmds[
	   netbuffer->u.serverpak.numplayers*netbuffer->u.serverpak.numtics];

    for(ti = start_tic; ti<end_tic; ti++)
    {
        // Clear first
        D_Clear_ticcmd(ti);

        // Copy the tics
        btic = BTIC_INDEX( ti );
        TicCmdCopy(netcmds[btic], ticp, netbuffer->u.serverpak.numplayers);
        ticp += netbuffer->u.serverpak.numplayers;

        // Copy the textcmds
        num_txt = *(txtp++);  // number of txtcmd
        for(j=0; j<num_txt; j++)
        {
	    int pn = *txtp++; // playernum
	    memcpy(textcmds[btic][pn].buff, txtp, txtp[0]+1);
	    txtp += txtp[0]+1;
	}
    }

    cl_need_tic = end_tic;
    return;

 not_in_packet:
    DEBFILE(va("Needed tic not in packet tic bounds: tic %u\n", cl_need_tic));
    return;
}


//
// Get Packets and invoke their Server and Client handlers.
//
static void Net_Packet_Handler(void)
{
    int  netconsole;
    byte nnode;

    while ( HGetPacket() )
    {
        nnode = doomcom->remotenode;
        // ---- Universal Handling ----------
        switch(netbuffer->packettype) {
	 case PT_CLIENTJOIN:
	    if( server )
	        client_join_handler( nnode );
            continue;
	 case PT_SERVERSHUTDOWN:
	    if( !server && (nnode == servernode) )
	        server_shutdown_handler();
            continue;
         case PT_NODE_TIMEOUT:
	    if( !server && (nnode == servernode) )
	    {
	        server_timeout_handler();
	        continue;
	    }
	    break; // There are other PT_NODE_TIMEOUT handler.
	 case PT_SERVERINFO:
	    server_info_handler( nnode );
            continue;
	}

        if(!nodeingame[nnode])
        {
	    unknown_host_handler( nnode );
            continue; //while
        }

        // Known nodes only.
        netconsole = nnode_to_player[nnode];  // the player
#ifdef PARANOIA
        if(!(netconsole & DRONE) && netconsole>=MAXPLAYERS)
        {
            I_Error("bad table nnode_to_player : node %d player %d",
                     doomcom->remotenode,netconsole);
	}
#endif


        switch(netbuffer->packettype)
        {
	    // ---- Server Handling Client packets ----------
            case PT_CLIENTCMD  :
            case PT_CLIENT2CMD :
            case PT_CLIENTMIS  :
            case PT_CLIENT2MIS :
            case PT_NODEKEEPALIVE :
            case PT_NODEKEEPALIVEMIS :
                if(server)
	             client_cmd_handler(netbuffer->packettype, nnode, netconsole );
                break;
            case PT_TEXTCMD2 : // splitscreen special
                netconsole=nnode_to_player2[nnode];
	        // fall through
            case PT_TEXTCMD :
                if(server)
	        {
		    if( netconsole<0 || netconsole>=MAXPLAYERS )
		    {
		        // Do not ACK the packet from a strange netconsole.
		        Net_Cancel_Packet_Ack(nnode);
		        break;
		    }
	            net_textcmd_handler( nnode, netconsole );
		}
                break;
            case PT_NODE_TIMEOUT:
            case PT_CLIENTQUIT:
                if(server)
	            client_quit_handler( nnode, netconsole );
                break;

            // ---- CLIENT Handling Server packets ----------
            case PT_SERVERTICS :
	        servertic_handler( nnode );
                break;
            case PT_SERVERCFG :
	        break;
            case PT_FILEFRAGMENT :
                if( !server )
                    Got_Filetxpak();
                break;
	    case PT_NETWAIT:
	        if( !server )
	        {
		    // Updates of wait for players, from the server.
		    num_netnodes = netbuffer->u.netwait.num_netnodes;
		    wait_nodes = netbuffer->u.netwait.wait_nodes;
		    wait_tics = LE_SWAP16( netbuffer->u.netwait.wait_tics );
		    P_SetRandIndex( netbuffer->u.netwait.p_rand_index ); // to sync P_Random
		}
	        break;
            default:
                DEBFILE(va("Unknown packet type: type %d node %d\n",
			   netbuffer->packettype, nnode));
	        break;
        } // end switch
    } // end while
}


// ----- NetUpdate
// Builds ticcmds for console player,
// sends out a packet

// no more use random generator, because at very first tic isn't yet synchronized
static int16_t Consistency(void)
{
    int16_t ret=0;
    int   pn;

    DEBFILE(va("TIC %d ",gametic));
    for(pn=0; pn<MAXPLAYERS; pn++)
    {
        if( playeringame[pn] && players[pn].mo )
        {
            DEBFILE(va("p[%d].x = %f ", pn, FIXED_TO_FLOAT(players[pn].mo->x)));
            ret += players[pn].mo->x;
        }
    }
    DEBFILE(va("pos = %d, rnd %d\n",ret,P_GetRandIndex()));
    ret+=P_GetRandIndex();

    return ret;
}

// By Server, Client.
// send the client packet to the server
// Called by NetUpdate, 
static void CL_Send_ClientCmd (void)
{
/* oops can do that until i have implemented a real dead reckoning
    static ticcmd_t lastcmdssent;
    static int      lastsenttime=-TICRATE;

    if( memcmp(&localcmds,&lastcmdssent,sizeof(ticcmd_t))!=0 || lastsenttime+TICRATE/3<I_GetTime())
    {
        lastsenttime=I_GetTime();
*/
    byte  cmd_options = 0;  // easier to understand and maintain
    int packetsize=0;

    if (cl_packetmissed)
        cmd_options |= 1;  // MIS bit
   
    netbuffer->packettype = PT_CLIENTCMD + cmd_options;
    netbuffer->u.clientpak.resendfrom = cl_need_tic;
    netbuffer->u.clientpak.client_tic = gametic;

    if( gamestate == GS_WAITINGPLAYERS )
    {
        // Server is waiting for network players before starting the game.
        // send NODEKEEPALIVE, or NODEKEEPALIVEMIS packet
        netbuffer->packettype = PT_NODEKEEPALIVE + cmd_options;
//        packetsize = sizeof(clientcmd_pak)-sizeof(ticcmd_t)-sizeof(int16_t);
        packetsize = offsetof(clientcmd_pak, consistency);
        HSendPacket (servernode,false,0,packetsize);
    }
    else
    if( gamestate != GS_NULL )
    {
        int btic = BTIC_INDEX( gametic );
        TicCmdCopy(&netbuffer->u.clientpak.cmd, &localcmds, 1);
        netbuffer->u.clientpak.consistency = LE_SWAP16_FAST(consistency[btic]);

        // send a special packet with 2 cmd for splitscreen
        if (cv_splitscreen.value)
        {
	    // send PT_CLIENT2CMD, or PT_CLIENT2CMDMIS packet
            netbuffer->packettype = PT_CLIENT2CMD + cmd_options;
            TicCmdCopy(&netbuffer->u.client2pak.cmd2, &localcmds2, 1);
            packetsize = sizeof(client2cmd_pak);
        }
        else
            packetsize = sizeof(clientcmd_pak);
        
        HSendPacket (servernode,false,0,packetsize);
    }

    if( cl_mode == CLM_connected )
    {
        // send extra data if needed
        if (localtextcmd.buff[0]) // text len
        {
	    int len = localtextcmd.buff[0]+1;  // text len + cmd
            netbuffer->packettype=PT_TEXTCMD;
            memcpy(netbuffer->u.textcmd, localtextcmd.buff, len);
            // all extra data have been sended
            if( HSendPacket(servernode, true, 0, len)) // send can fail for some reasons...
                localtextcmd.buff[0] = 0;  // text len
        }
        
        // send extra data if needed for player 2 (splitscreen)
        if (localtextcmd2.buff[0]) // text len
        {
	    int len = localtextcmd2.buff[0]+1;  // text len + cmd
            netbuffer->packettype=PT_TEXTCMD2;
            memcpy(netbuffer->u.textcmd, localtextcmd2.buff, len);
            // all extra data have been sended
            if( HSendPacket(servernode, true, 0, len)) // send can fail for some reasons...
                localtextcmd2.buff[0] = 0; // text len
        }
    }

}


// By Server.
// Send PT_SERVERTICS, the server packet.
// Send tic from next_tic_send to maketic-1.
static void SV_Send_Tics (void)
{
    static byte resend_cnt = 0;  // spread resends at less cost than Get_Time

    tic_t start_tic, end_tic, ti;
    int  nnode;
    int  pn, packsize;
    int  btic;
    char *num_txt_p, num_txt;
    char *bufpos;

    // Send PT_SERVERTIC to all client, but not to myself.
    // For each node create a packet with x tics and send it.
    // x is computed using nextsend_tic[n], max packet size and maketic.
    for(nnode=1; nnode<MAXNETNODES; nnode++)
    {
        if( ! nodeingame[nnode] )  continue;

        // Send a packet to each client node in this game.
	// They may be different, with individual status.
        end_tic = maketic;
          // The last tic to send is one before maketic.

        // assert nextsend_tic[nnode]>=nettics[nnode]
        start_tic = nextsend_tic[nnode];  // last tic sent to this nnode
       
        if(start_tic >= maketic)
        {
	    // We have sent all tics, so we will use the extrabandwidth
	    // to resend packets that are supposed lost.
	    // This is necessary since lost packet detection works when we
	    // have received packet with (firsttic > cl_need_tic)
	    // (in the getpacket servertics case).
	    DEBFILE(va("Send Tics none: node %d maketic %u nxttic %u nettic %u\n", 
		       nnode, maketic, nextsend_tic[nnode], nettics[nnode]));
	    start_tic = nettics[nnode];
	    if( start_tic >= maketic )
	        continue;  // all tic are ok, do next node
	    if( (nnode + resend_cnt++)&3 )  // some kind of randomness
	        continue;  // skip it
	    DEBFILE(va("Resend from tic %d\n", start_tic));
	}

        // Limit start to first new tic we have.
        if( start_tic < next_tic_send )
	    start_tic = next_tic_send;

        // Compute the length of the packet and cut it if too large.
        packsize = SERVER_TIC_BASE_SIZE;
        for(ti=start_tic; ti<end_tic; ti++)
        {
	    // All of the ticcmd
	    packsize += sizeof(ticcmd_t) * doomcom->numplayers;
	    // All of the textcmd
	    packsize += TotalTextCmdPerTic(ti);

	    if( packsize > software_MAXPACKETLENGTH )
	    {
	        // Exceeds max packet size.
	        DEBFILE(va("Packet too large (%d) at tic %d (should be from %d to %d)\n", 
			    packsize, ti, start_tic, end_tic));
	        end_tic = ti;  // limit this packet due to size

	        // too bad :
	        // Too many players have sent extradata and there is too
	        // much data for one tic packet.
	        // Too avoid it put the data on the next tic (see getpacket 
	        // textcmd case). But when numplayer changes the computation
	        // can be different
	        if(end_tic == start_tic)
	        {
		    // No tics made it into the packet.
		    if( packsize > MAXPACKETLENGTH )
		    {
		        I_Error("Too many players: cannot send %d data for %d players to net node %d\n"
				"Well sorry nobody is perfect....\n",
				packsize, doomcom->numplayers, nnode);
		    }
		    else
		    {
		        end_tic++;  // send it anyway !
		        DEBFILE("Sending empty tic anyway\n");
		    }
		}
	        break;
	    }
	}
            
        // Send the tics, start_tic..(end_tic-1), in one packet.

        netbuffer->packettype = PT_SERVERTICS;
        netbuffer->u.serverpak.starttic = start_tic;
        netbuffer->u.serverpak.numtics = (end_tic - start_tic); // num tics
        netbuffer->u.serverpak.numplayers = LE_SWAP16_FAST(doomcom->numplayers);
        bufpos=(char *)&netbuffer->u.serverpak.cmds;
       
        // All the ticcmd_t, start_tic..(end_tic-1)
        for(ti=start_tic; ti<end_tic; ti++)
        {
	    int btic = BTIC_INDEX( ti );
	    TicCmdCopy((ticcmd_t*) bufpos, netcmds[btic], doomcom->numplayers);
	    bufpos += doomcom->numplayers * sizeof(ticcmd_t);
	}
            
        // All the textcmd, start_tic..(end_tic-1)
        for(ti=start_tic; ti<end_tic; ti++)
        {
	    btic = BTIC_INDEX( ti );
	    num_txt_p = bufpos++; // the num textcmd field
	    num_txt = 0;
	    for(pn=0; pn<MAXPLAYERS; pn++)
	    {
	        if((pn==0) || playeringame[pn])
	        {
		    int textlen = textcmds[btic][pn].buff[0];
		    if(textlen)
                    {
                        *(bufpos++) = pn;  // playernum
		        // The textcmd is textlen, text.
                        memcpy(bufpos, textcmds[btic][pn].buff, textlen+1);
                        bufpos += textlen+1;
                        num_txt++; // inc the number of textcmd
                    }
		}
	    }
	    // There must be a num txtcmd field for every tic, even if 0.
	    *num_txt_p = num_txt; // the number of textcmd
	}
        packsize = bufpos - (char *)&(netbuffer->u);

        HSendPacket(nnode, false, 0, packsize);

	// Record next tic for this net node.
	// Extratic causes redundant transmission of tics.
	ti = (end_tic - doomcom->extratics);  // normal
        // When tic is too large, only one tic is sent so don't go backward !
        if( ti <= start_tic )
	   ti = end_tic;  // num tics is too small for extratics
        if( ti < nettics[nnode] )
	   ti = nettics[nnode];
        nextsend_tic[nnode] = ti;
    }
    // node 0 is me !
    nextsend_tic[0] = maketic;
}

//
// TryRunTics
//
static void Local_Maketic(int realtics)
{
    rendergametic=gametic;
    // translate inputs (keyboard/mouse/joystick) into game controls
    G_BuildTiccmd(&localcmds, realtics, 0);
    // [WDJ] requires splitscreen and player2 present
    if (cv_splitscreen.value && displayplayer2_ptr )
      G_BuildTiccmd(&localcmds2, realtics, 1);

#ifdef CLIENTPREDICTION2
    if( !paused && localgametic<gametic+BACKUPTICS)
    {
        P_MoveSpirit ( &players[consoleplayer], &localcmds, realtics );
        localgametic+=realtics;
    }
#endif
    localcmds.angleturn |= TICCMD_RECEIVED;
}

void SV_SpawnPlayer(int playernum, int x, int y, angle_t angle)
{
    // for futur copytic use the good x,y and angle!
    if( server )
    {
        int btic = BTIC_INDEX( maketic );
#ifdef CLIENTPREDICTION2
        netcmds[btic][playernum].x=x;
        netcmds[btic][playernum].y=y;
#endif
        netcmds[btic][playernum].angleturn=(angle>>16) | TICCMD_RECEIVED;
    }
}

// create missed tic
void SV_Maketic(void)
{
    int btic = BTIC_INDEX( maketic );
    int bticprev, i, player;
    byte nnode;

    for(nnode=0; nnode<MAXNETNODES; nnode++)
    {
        if(playerpernode[nnode] == 0)  continue;
       
        player=nnode_to_player[nnode];
        if((netcmds[btic][player].angleturn & TICCMD_RECEIVED) == 0)
        {
	    // Catch startup glitch where playerpernode gets set
	    // before the player is actually setup.
	    if( ! playeringame[player] )  continue;

	    DEBFILE(va("MISS tic %4u for node %d\n", maketic, nnode));
#ifdef PARANOIA
	    if( devparm )
	        GenPrintf(EMSG_dev, "\2Client %d Miss tic %d\n", nnode, maketic);
#endif
	    // Copy the previous tic
	    bticprev = BTIC_INDEX(maketic-1);
	    for(i=0; i<playerpernode[nnode]; i++)
	    {
	        netcmds[btic][player] = netcmds[bticprev][player];
	        netcmds[btic][player].angleturn &= ~TICCMD_RECEIVED;
	        player = nnode_to_player2[nnode];
	    }
	}
    }
    // all tic are now present, make the next
    maketic++;
}

#ifdef DEBUGFILE
static  int     net_load;
#endif

//  realtics: 0..5
void TryRunTics (tic_t realtics)
{
    // the machine have laged but is not so bad
    if(realtics>TICRATE/7) // FIXME: consistency failure!!
    {
        if(server)
            realtics=1;
        else
            realtics=TICRATE/7;
    }

    if(singletics)
        realtics = 1;

    if( realtics > 0 )
        COM_BufExecute();            

    NetUpdate();

    if(demoplayback)
    {
        cl_need_tic = gametic + realtics + cv_playdemospeed.value;
        // start a game after a demo
        maketic+=realtics;
        next_tic_send=maketic;
        next_tic_clear=next_tic_send;
    }

#ifdef DEBUGFILE
    if(realtics==0)
        if(net_load) net_load--;
#endif
    Net_Packet_Handler();

#ifdef DEBUGFILE
    if (debugfile && (realtics || cl_need_tic>gametic))
    {
        //SoM: 3/30/2000: Need long int in the format string for args 4 & 5.
        fprintf (debugfile,
                 "------------ Tryruntic : REAL:%lu NEED:%lu GAME:%lu LOAD: %i\n",
                 (unsigned long)realtics, (unsigned long)cl_need_tic,
		 (unsigned long)gametic, net_load);
        net_load=100000;
    }
#endif

    if( gamestate == GS_WAITINGPLAYERS )
    {
        // Server is waiting for network players.
	// To wait, execution must not reach G_Ticker.
        if( !server && !netgame )
	    goto error_ret;  // connection closed by cancel or timeout
       
        if( realtics <= 0 )
	    return;
       
        // Once per tic
        // Wait for players before netgame starts.
        if( ! D_WaitPlayer_Ticker() )
	    return;  // Waiting

        // Start game
        if( dedicated )
	    gamestate = GS_DEDICATEDSERVER;
        // Others must wait for load game to set gamestate to GS_LEVEL.
    }

    if (cl_need_tic > gametic)
    {
        if (demo_ctrl == DEMO_seq_advance)  // and not disabled
        {
            D_DoAdvanceDemo ();
	    return;
	}

        // Run the count * tics
        while (cl_need_tic > gametic)
        {
            DEBFILE(va("==== Runing tic %u (local %d)\n",gametic, localgametic));

            G_Ticker ();
            ExtraDataTicker();
            gametic++;
            // skip paused tic in a demo
            if(demoplayback)
            {
	        if(paused)
		   cl_need_tic++;
	    }
            else
	    {
                consistency[ BTIC_INDEX( gametic ) ] = Consistency();
	    }
        }
    }
    return;

error_ret:
    D_StartTitle();
    return;
}

// By Server
// Send Tic updates
static void SV_Send_Tic_Update( int count )
{
    byte nn;

    // Find lowest tic, over all nodes.
    next_tic_send = gametic;
    for( nn=0; nn<MAXNETNODES; nn++)
    {
        // Max of gametic and nettics[].
        if(nodeingame[nn]
	   && nettics[nn]<next_tic_send )
        {
	   next_tic_send = nettics[nn];
	}
    }

    // Don't erase tics not acknowledged
    if( (maketic+count) >= (next_tic_send+BACKUPTICS) )
        count = (next_tic_send+BACKUPTICS) - maketic - 1;

    while( count-- > 0 )
        SV_Maketic();  // create missed tics and increment maketic

    // clear only when acknowledged
    for( ; next_tic_clear<next_tic_send; next_tic_clear++)
        D_Clear_ticcmd(next_tic_clear);  // clear the maketic the new tic

    SV_Send_Tics();

    cl_need_tic=maketic; // the server is a client too
}


void NetUpdate(void)
{
    static tic_t prev_netupdate_time=0;
    tic_t        nowtime;
    int          realtics;	// time is actually long [WDJ]

    nowtime  = I_GetTime();
    realtics = nowtime - prev_netupdate_time;

    if( realtics <= 0 )
    {
        if( realtics > -100000 )  // [WDJ] 1/16/2009  validity check
	    return;     // same tic as previous

        // [WDJ] 1/16/2009 something is wrong, like time has wrapped.
        // Program gets stuck waiting for this, so force it out.
        realtics = 1;
    }
    if( realtics > 5 )
    {
        realtics = ( server )? 1 : 5;
    }

    // Executed once per tic, with realtics = 1..5.
    prev_netupdate_time = nowtime;

    if( !server )
        maketic = cl_need_tic;

    if( ! dedicated )
    {
        // Local Client
        I_OsPolling();       // i_getevent
        D_ProcessEvents ();
          // menu responder ???!!!
          // Cons responder
          // game responder call :
          //    HU_responder,St_responder, Am_responder
          //    F_responder (final)
          //    and G_MapEventsToControls

        Local_Maketic (realtics);  // make local tic

        if( server && !demoplayback )
	    CL_Send_ClientCmd();     // send server tic
    }

    Net_Packet_Handler();  // get packet from client or from server

    // Client sends its commands after a receive of the server tic.
    // The server sends before because in single player is better.

    if( !server )
        CL_Send_ClientCmd();   // send tic cmd
    else
    {
        // By Server
        //Hurdler: added for acking the master server
        if( cv_internetserver.value )
            MS_SendPing_MasterServer( nowtime );

        if(!demoplayback)
	    SV_Send_Tic_Update( realtics );  // realtics > 0

        if( Filetx_file_cnt )  // Rare to have file download in progress.
	    Filetx_Ticker();
    }

    Net_AckTicker();
    if( ! dedicated )
    {
        M_Ticker ();
        CON_Ticker();
    }
}
