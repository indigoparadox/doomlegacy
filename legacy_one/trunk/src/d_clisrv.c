// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2016 by DooM Legacy Team.
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
#include "doomstat.h"
#include "d_clisrv.h"
#include "command.h"
#include "i_net.h"
#include "i_tcp.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "d_netfil.h"
#include "d_main.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "keys.h"
#include "m_argv.h"
#include "m_menu.h"
#include "console.h"
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
#include "t_script.h"

#include "b_game.h"	//added by AC for acbot
#include "r_things.h"
  // skins
#include "g_input.h"
  // gamecontrol
#include "s_sound.h"
  // StartSound


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
const int  NETWORK_VERSION = 25; // separate version number for network protocol (obsolete)


#define JOININGAME


#if NUM_SERVERTIC_CMD < BACKUPTICS
# error Not enough NUM_SERVERTIC_CMD
#endif

// Used to signal network errors to higher level functions.
// Beware that server may also have a client.
typedef enum {
   NETS_idle,
   NETS_fatal,
   NETS_shutdown,
// Have network.
   NETS_open,      // Normal network.
   NETS_internal,  // Self server network.
// Client
   NETS_no_server, // Client lost the server.
   NETS_connecting,// Client connecting to the server
   NETS_active     // Client has server connection.
} network_state_e;

static network_state_e  network_state = NETS_idle;
static byte  quit_netgame_status = 0;  // to avoid repeating shutdown
static byte  wait_netplayer = 0;

#define PREDICTIONQUEUE         BACKUPTICS
#define PREDICTIONMASK          (PREDICTIONQUEUE-1)

// Server state
boolean  server = true; // false when Client connected to other server
boolean  serverrunning = false;
byte     serverplayer = 255;  // 255= no server player (same as -1)
byte     num_player_slots = 0;  // for messages with slots

// Server specific vars.
// player=255 when unused
// nnode =255 when unused
// nnode_state[] = NOS_idle, when net node is unused
// The netnodes are counted, 0..31
static byte  player_to_nnode[MAXPLAYERS];
static byte  player_pind[MAXPLAYERS];
static byte  num_player_used = 0;
static byte  num_join_waiting_players = 0;
static byte  num_wait_game_start = 0;  // waiting until next game


// Server net node state for
// tracking client nodes.
typedef enum {
  NOS_idle,  // node is unused
  NOS_fatal, // something bad
  NOS_shutdown,  // node is shutting down
  NOS_invalid,   // invalid node
// Recognized nodes.
  NOS_recognized,
// node during join procedure
  NOS_join,         // node join procedure
  NOS_join_file,    // downloading file
  NOS_join_savegame,// downloading savegame
  NOS_join_sg_loaded,  // downloading done
  NOS_join_timeout,
// node with a player
  NOS_client,
// waiting players
  NOS_wait_game_start, // waiting for next game start
// repair a client
  NOS_repair,          // node is being repaired
  NOS_repair_player,   // update player
  NOS_repair_pl_done,  // player update done
  NOS_repair_savegame, // downloading savegame
  NOS_repair_sg_loaded,   // downloading done
  NOS_repair_timeout,
// node normal play
  NOS_active,
// FOR CLIENT USE ONLY.  Server must not use for client state.
  NOS_server,   // client sees server at cl_servernode
  NOS_internal  // client sees server on self
} nnode_state_e;

// [WDJ] Used to be OR with nnode_to_player to indicate DRONE node.
// No longer done that way to keep player number clean, as it is used for indexing
// and the DRONE bit conflicted with 255=idle player.
// #define DRONE               0x80    // bit set in consoleplayer
// DRONE has playerpernode=0

// Server: net node state of clients.
// Node numbers seen by server are different than those seen by clients (determined by connection order).
// Index by server space nnode numbers.
static byte     nnode_state[MAXNETNODES];  // nnode_state_e
// Index by pind, [0]=main player [1]=splitscreen player
static byte     nnode_to_player[2][MAXNETNODES];  // 255= unused
static byte     playerpernode[MAXNETNODES]; // used specialy for splitscreen
static byte     join_waiting[MAXNETNODES];  // num of players waiting to join
static byte     consistency_faults[MAXNETNODES];
static tic_t    nettics[MAXNETNODES];     // what tic the client have received
static tic_t    nextsend_tic[MAXNETNODES]; // what server sent to client

static tic_t    next_tic_send;     // min of the nettics
static tic_t    next_tic_clear=0;  // clear next_tic_clear to next_tic_send
static tic_t    maketic;
#ifdef CLIENTPREDICTION2
tic_t localgametic;
#endif

// Client specific.
boolean         cl_drone; // client displays, no commands
static byte     cl_nnode; // net node for this client, assigned by server (server nnode space)
static byte     cl_error_status = 0;  // repair
static boolean  cl_packetmissed;
static tic_t    cl_need_tic;
static tic_t    cl_prev_tic = 0;  // client tests once per tic

// Client view of server :
//   Node numbers on client are not same as those on server, each has own independent nnode space.
//   Server net node, 251=none (to not match invalid node)
byte            cl_servernode = 251;  // in client nnode space, never let server use this
static byte     cl_server_state = NOS_idle; // nnode_state_e, client view of server

// Text buffer for textcmds.
// One extra byte at end for 0 termination, to protect against malicious use.
// Use textbuf_t from textcmdpak

// Client maketic
// [0]=main player [1]=splitscreen player
static ticcmd_t   localcmds[2];
static textbuf_t  localtextcmd[2];

// engine
// Server packet state
// Index for netcmds and textcmds
#define BTIC_INDEX( tic )  ((tic)%BACKUPTICS)
// NetCmd and TextCmd store
// Index using BTIC_INDEX
ticcmd_t        netcmds[BACKUPTICS][MAXPLAYERS];
static textbuf_t  textcmds[BACKUPTICS][MAXPLAYERS];

static int16_t    consistency[BACKUPTICS];



consvar_t cv_playdemospeed  = {"playdemospeed","0",CV_VALUE,CV_Unsigned};

consvar_t cv_server1 = { "server1", "192.168.1.255", CV_STRING|CV_SAVE, NULL };
consvar_t cv_server2 = { "server2", "", CV_STRING|CV_SAVE, NULL };
consvar_t cv_server3 = { "server3", "", CV_STRING|CV_SAVE, NULL };

CV_PossibleValue_t download_cons_t[] = {{0,"No Download"},{1,"Allowed"},{0,NULL}};
consvar_t cv_download_files = {"download_files"  ,"1", CV_SAVE, download_cons_t};
consvar_t cv_SV_download_files = {"sv_download_files"  ,"1", CV_SAVE, download_cons_t};
consvar_t cv_download_savegame = {"download_savegame"  ,"1", CV_SAVE, download_cons_t};
consvar_t cv_SV_download_savegame = {"sv_download_savegame"  ,"1", CV_SAVE, download_cons_t};

CV_PossibleValue_t netrepair_cons_t[] = {{0,"None"},{1,"Minimal"},{2,"Medium"},{3,"Aggressive"},{0,NULL}};
consvar_t cv_netrepair = {"netrepair","2", CV_SAVE, netrepair_cons_t};
consvar_t cv_SV_netrepair = {"sv_netrepair","2", CV_SAVE, netrepair_cons_t};

// consistency check, index by cv_SV_netrepair
static byte consistency_limit_fatal[4] = { 1, 2, 6, 9 };
static byte consistency_sg_bit[4]    = { 0, 0x02, 0x14, 0xAA };  // bit on when should req savegame



// some software don't support largest packet
// (original sersetup, not exactly, but the probability of sending a packet
// of 512 octet is like 0.1)
uint16_t  software_MAXPACKETLENGTH;

// Handle errors from HSendPacket consistently.
static void  generic_network_error_handler( byte errcode, const char * who )
{
    if( errcode == NE_not_netgame )  return;  // known problem during join
    if( errcode >= NE_fail )
        network_error_print( errcode, who );     
}


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

static void D_Clear_ticcmd(int tic)
{
    int i;
    int btic = BTIC_INDEX( tic );

    for(i=0;i<MAXPLAYERS;i++)
    {
        textcmds[btic][i].len = 0;  // textlen
        netcmds[btic][i].angleturn = 0; //&= ~TICCMD_RECEIVED;
    }
    DEBFILE(va("Clear tic %5d [%2d]\n", tic, btic));
}


// -----------------------------------------------------------------
//  Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

// NetXCmd indirection.
static void (*netxcmd_func[MAXNETXCMD]) (xcmd_t * xc);

void Register_NetXCmd(netxcmd_e cmd_id, void (*cmd_f) (xcmd_t * xc))
{
#ifdef PARANOIA
   if(cmd_id >= MAXNETXCMD)
      I_Error("NetXCmd id %d exceeds defined range", cmd_id);
   if(netxcmd_func[cmd_id]!=0)
      I_Error("NetXCmd id %d already registered", cmd_id);
#endif
   netxcmd_func[cmd_id] = cmd_f;
}

// [WDJ] Server NetXCmd use client text cmd channel [0], thus client_pn=0.
// This is because those text buffers already exist, and are sent anyway.
// The server NetXCmd do not use the client_pn.
// Only the client NetXCmd need to indicate the player performing the action.
// Server NetXCmd need to transit the network logic, even when there
// is no server player. This used to be done by unexplained kluges.
// Version 1.48 logic makes sending server NetXCmd over
// text cmd channel [0] less mysterious and much less fragile.

// The NetXCmd must be executed in all clients during a specific gametic.
// The current system has all movement commands and NetXCmd for one tick
// in the servertick message.  Once it is received the client game tick can advance.
// The alternative of a separate server command path would require considerable
// effort to get those NetXCmd executed during a specific gametic.
// It would be necessary to inform clients that a separate text command message was
// expected for that gametic.  The separate text commands would have to be collected
// before the client could execute that gametic.
// Variations in message arrival order would also be a serious problem.

// Command sent over text cmd channel.  Default as main player.
//  cmd_id :  X command, XD_
//  param : parameter strings
//  param_len : number of parameter strings
void Send_NetXCmd(byte cmd_id, void *param, int nparam)
{
    Send_NetXCmd_pind( cmd_id, param, nparam, 0 );  // main player
}

// Command sent over text cmd channel.
//  cmd_id :  X command, XD_
//  param : parameter strings
//  param_len : number of parameter strings
//  pind : player index, [0]=main player, [1]=splitscreen player
//         Server NetXCmd will use the default, pind=0.
void Send_NetXCmd_pind(byte cmd_id, void *param, int param_len, byte pind)
{
   if(demoplayback)
       return;

   // Save the NetXCmd in a localtextcmd buffer.
   textbuf_t * ltcbp = &localtextcmd[pind];
   int textlen = ltcbp->len;
   if( (textlen + 1 + param_len) > MAXTEXTCMD)  // with XD_ and param
   {
#ifdef PARANOIA
       I_SoftError("Net command exceeds buffer: pind=%d  netcmd=%d  netcmdlen=%d  total=%d\n",
                   pind, cmd_id, param_len, (textlen + 1 + param_len));
#else
       GenPrintf(EMSG_warn, "\2Net Command exceeds buffer: pind=%d  netcmd %d  netcmdlen=%d  total=%d\n",
                   pind, cmd_id, param_len, (textlen + 1 + param_len));
#endif
       return;
   }

   // Append to player1 text commands.
   // First byte is the cmd, followed by its parameters (binary or string).
   ltcbp->text[textlen++] = cmd_id; // XD_
   if(param && param_len)
   {
       memcpy(&ltcbp->text[textlen], param, param_len);
       textlen += param_len;
   }
   ltcbp->len = textlen;
}

// NetXCmd with 2 parameters.
void Send_NetXCmd_p2(byte cmd_id, byte param1, byte param2)
{
    byte buf[3];

    buf[0] = param1;
    buf[1] = param2;
    Send_NetXCmd(cmd_id, &buf, 2);  // always main player
}


// Execute NetXCmd for this gametic.
void ExtraDataTicker(void)
{
    int btic = BTIC_INDEX( gametic );
    int pn;
    textbuf_t * textbuf;
    byte * endbuf;
    byte * endtxt;  // independent of changes by called xfunc
    unsigned int textlen;
    xcmd_t  xcmd;

    // There is a textcmd buffer [MAXTEXTCMD+1] for each active player.
    for(pn=0; pn<MAXPLAYERS; pn++)
    {
        // Execute commands of any player in the game, and always for pn=0.
        if( ! playeringame[pn] && (pn>0) )  continue;

        xcmd.playernum = pn;
        textbuf = & textcmds[btic][pn];
        textlen = textbuf->len;  // 0..255
        if( textlen == 0 )  continue;  // empty text
#if MAXTEXTCMD < 255
        if( textlen > MAXTEXTCMD )   textlen = MAXTEXTCMD;  // bad length
#endif       
        // set extra termination byte at end of buffer (text[MAXTEXTCMD+1])
        endbuf = & textbuf->text[MAXTEXTCMD]; // last char
        endbuf[0] = 0;  // Protect against malicious strings.

        // Commands can have 0 strings, 1 string, or 2 strings.
        // Inventory has just a byte number.
        xcmd.curpos = (byte*)&(textbuf->text[0]);  // start of command
        endtxt = &(textbuf->text[textlen]);  // after last char of text
        if( endtxt >= endbuf )  endtxt = endbuf;
        endtxt[0] = 0;  // Protect against malicious strings.
        xcmd.endpos = endtxt;  // end of text + 1
        // One or more commands are within curpos..endpos-1
        while(xcmd.curpos < endtxt)
        {
            xcmd.cmd = *(xcmd.curpos++);  // XD_ command
            if(xcmd.cmd < MAXNETXCMD && netxcmd_func[xcmd.cmd])
            {
                // Execute a NetXCmd.
                // The NetXCmd must update xcmd.curpos.
                DEBFILE(va("Executing xcmd %d player %d ", xcmd.cmd, pn));
                (netxcmd_func[xcmd.cmd])(&xcmd);
                // nextcmd_func updates curpos, without knowing textlen
                DEBFILE("Execute done\n");
            }
            else
            {
                // [WDJ] Why should a bad demo command byte be fatal.
                I_SoftError("Got unknown net/demo command [%d]=%d len=%d\n",
                           (xcmd.curpos - &(textbuf->text[0])),
                           xcmd.cmd, textlen);
                D_Clear_ticcmd(btic);
                break;
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
//   byte   | LEGACY 1.13:  XDNAMEANDCOLOR, XD_WEAPON_PREF bits
//            LEGACY 1.20:  XD_ codes
//            Determines what parameters follow.
//
// LEGACY 1.12 XD bits
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

// Save textcmd to demo using textbuf_t format.
boolean AddLmpExtradata(byte **demo_point, int playernum)
{
    int  btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf = & textcmds[btic][playernum];
    int  textlen = textbuf->len;

    if(textlen == 0)  // anything in the buffer
        return false;

    // Demo format matches textbuf_t, length limited
    memcpy(*demo_point, textbuf, textlen+1);  // len,text
    *demo_point += textlen+1;
    return true;
}

void ReadLmpExtraData(byte **demo_pointer, int playernum)
{
    unsigned int extra_len;
    int btic = BTIC_INDEX( gametic );
    textbuf_t * textbuf = & textcmds[btic][playernum];
    byte  * dp;

    if(!demo_pointer)  goto no_text_cmd;

    dp = *demo_pointer;
    if( dp == NULL )  goto no_text_cmd;

    extra_len = dp[0];   // textbuf->len
    // [WDJ] Clean separation of old and new formating.
    if(demoversion==112) // support old demos v1.12
    {
        // Different, limited, XCmd format.
        byte  * p = dp;
        int  textlen = 0;
        byte  ex;

        // extra_len is length of extra data, incl length.
        ex = p[1];  // XCmd bits
        p += 2;  // skip extra_len and XCmd
        if(ex & 1)
        {
            textbuf->text[textlen++] = XD_NAMEANDCOLOR;
            memcpy(&textbuf->text[textlen],
                   p,
                   MAXPLAYERNAME+1);
            p+=MAXPLAYERNAME+1;
            textlen += MAXPLAYERNAME+1;
        }
        if(ex & 2)
        {
            textbuf->text[textlen++] = XD_WEAPONPREF;
            memcpy(&textbuf->text[textlen],
                   p,
                   NUMWEAPONS+2);
            p+=NUMWEAPONS+2;
            textlen += NUMWEAPONS+2;
        }
        textbuf->len = textlen;
    }
    else
    {
        // Demo format matches textbuf_t, length limited.
        // extra_len is textbuf->len, excluding len field.
        extra_len ++;
        memcpy(textbuf, dp, extra_len);  // len,text
    }
    // update demo pointer
    *demo_pointer = dp + extra_len;
    return;

no_text_cmd:
    textbuf->len = 0;  // empty text cmd
    return;
}


// -----------------------------------------------------------------
//  end extra data function for lmps
// -----------------------------------------------------------------

// ----- Server/Client Responses

// Client state
typedef enum {
   CLM_idle,
   CLM_fatal,
   CLM_searching,
   CLM_server_files,
   CLM_download_req,
   CLM_download_files,
   CLM_askjoin,
   CLM_wait_join_response,
   CLM_download_savegame,
   CLM_download_done,
   CLM_wait_game_start,  // ready but must wait for next game start
   CLM_connected
} cl_mode_t;

static cl_mode_t  cl_mode = CLM_idle;

static void CL_ConnectToServer(void);
static int16_t  Consistency(void);
static void Net_Packet_Handler(void);
static void SV_Reset_NetNode(byte nnode);
static boolean SV_Add_Join_Waiting(void);
static byte SV_update_player_counts(void);


// By Server
// Broadcast to all connected nodes
//   flags: SP_ for HSendPacket
static void SV_SendPacket_All( sendpacket_flag_e flags, size_t size_packet, const char * msg )
{
    int nn;
    for(nn=1; nn<MAXNETNODES; nn++)
    {
        if( nnode_state[nn] >= NOS_recognized )
        {
            HSendPacket( nn, flags, 0, size_packet );  // ignore failures
            if( msg )	   
                debug_Printf( "%s[ %d ]\n", msg, nn );
        }
    }
}

// Server, Client
// send a simple packet
//   nnode : client node, cl_servernode, BROADCAST_NODE
// Return  NE_xx
static byte  SendPacket( byte nnode, byte packettype )
{
    netbuffer->packettype = packettype;

    if( nnode == BROADCAST_NODE )  // only by server
    {
        SV_SendPacket_All( SP_reliable|SP_queue|SP_error_handler, 0, NULL );
        return NE_success;
    }

    if( nnode < MAXNETNODES )
        return HSendPacket( nnode, SP_reliable|SP_queue|SP_error_handler, 0, 0 );

    return NE_fail;
}


// Flags used by Join msg.
typedef enum {
  NF_drone = 0x80,
  NF_big_endian = 0x40,
  NF_download_savegame = 0x10,
} join_flags_e;

// By Client.
// Send a request to join game.
// Called by CL_ConnectToServer.
static boolean  CL_Send_Join( void )
{
    byte flg = 0;
    if( cv_download_savegame.EV )  flg |= NF_download_savegame;
    if( cl_drone )  flg |= NF_drone;
#ifdef __BIG_ENDIAN__
    flg |= NF_big_endian;
#endif

    GenPrintf(EMSG_hud, "Send join request...\n");
    netbuffer->packettype=PT_CLIENTJOIN;
    netbuffer->u.clientcfg.version = VERSION;
    netbuffer->u.clientcfg.subversion = LE_SWAP32(NETWORK_VERSION);
    netbuffer->u.clientcfg.mode = 0;
    netbuffer->u.clientcfg.flags = flg;

    // Declare how many players at this node.
    netbuffer->u.clientcfg.num_node_players =
        (cl_drone)? 0
      : ( (cv_splitscreen.value)? 2 : 1 );

    byte errcode = HSendPacket( cl_servernode, SP_reliable|SP_queue|SP_error_handler, 0, sizeof(clientconfig_pak_t) );
    return  (errcode < NE_fail);
}


// ----- Server Info

// By Server.
// Reply to request for server info.
//   reqtime : the send time of the request
static void SV_Send_ServerInfo(int to_node, tic_t reqtime)
{
    byte * p;

    netbuffer->packettype=PT_SERVERINFO;
    netbuffer->u.serverinfo.version = VERSION;
    netbuffer->u.serverinfo.subversion = LE_SWAP32(NETWORK_VERSION);
    // return back the time value so client can compute their ping
    netbuffer->u.serverinfo.trip_time = LE_SWAP32(reqtime);
    netbuffer->u.serverinfo.num_active_players = num_game_players;
    netbuffer->u.serverinfo.maxplayer = cv_maxplayers.value;
    netbuffer->u.serverinfo.load = 0;        // unused for the moment
    netbuffer->u.serverinfo.deathmatch = cv_deathmatch.EV;  // menu setting
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

    HSendPacket( to_node, 0, 0, p - ((byte *)&netbuffer->u) );  // msg lost when too busy
}


// By Server.
// Accept player joining the game.
// Send the initial command with the config to avoid sending a second message,
// which could arrive out-of-order.
//   command : CTRL_ command to send with config
static boolean SV_Send_ServerConfig( byte to_node, byte command )
{
    int   i,playermask=0;
    xcmd_t xc;

    netbuffer->packettype=PT_SERVERCFG;
    for(i=0;i<MAXPLAYERS;i++)
    {
         if(playeringame[i])
              playermask|=1<<i;
    }

    netbuffer->u.servercfg.version         = VERSION;
    netbuffer->u.servercfg.subversion      = LE_SWAP32(NETWORK_VERSION);

    netbuffer->u.servercfg.serverplayer    = serverplayer;
    netbuffer->u.servercfg.num_player_slots= num_player_slots;
    netbuffer->u.servercfg.playerdetected  = LE_SWAP32(playermask);
    netbuffer->u.servercfg.gametic         = LE_SWAP32(gametic);
    netbuffer->u.servercfg.clientnode      = to_node;  // client node (in server space)
    netbuffer->u.servercfg.gamestate       = gamestate;
    netbuffer->u.servercfg.command         = command;

    xc.playernum = 0;
    xc.curpos = netbuffer->u.servercfg.netvar_buf;
    xc.endpos = xc.curpos + NETVAR_BUFF_LEN - 1;
    CV_SaveNetVars( &xc );
    // curpos is 1 past last cvar (if none then is at netvar_buf)

    byte errcode = HSendPacket( to_node, SP_reliable|SP_queue|SP_error_handler, 0, xc.curpos - ((byte *)&netbuffer->u) );
    return  (errcode < NE_fail);
}


// ----- Control Message

// Network commands
typedef enum {
    CTRL_state,  // no node command
    CTRL_normal,
    CTRL_download_savegame,
    CTRL_wait_game_start,
    CTRL_game_start,
    CTRL_wait_timer,
} net_control_command_e;

// By Server
//   ctrl_command : net_control_e
//   player_num : player num (may be 255)
//   data16 : timer value or other data specific to the command
static void SV_Send_control( byte nnode, byte ctrl_command, byte player_num, uint16_t data16 )
{
    netbuffer->packettype = PT_CONTROL;
    netbuffer->u.control.command = ctrl_command;
    netbuffer->u.control.player_num = player_num;
    netbuffer->u.control.player_state = (player_num < MAXPLAYERS)? player_state[player_num] : PS_unused;
    netbuffer->u.control.gametic = LE_SWAP32(gametic);
    netbuffer->u.control.gamemap = gamemap;
    netbuffer->u.control.gameepisode = gameepisode;
    netbuffer->u.control.data = LE_SWAP16( data16 );

    if( nnode == BROADCAST_NODE )
    {
        // Broadcast it
        SV_SendPacket_All( true, sizeof(control_pak_t), NULL );
    }
    else if( nnode < MAXNETNODES )
    {
        byte errcode = HSendPacket( nnode, SP_reliable|SP_queue, 0, sizeof(control_pak_t) );
        if( errcode >= NE_fail )
        {
            generic_network_error_handler( errcode, "Control msg" );  // should not happen
        }
    }
}

// By Client
static void control_msg_handler( void )
{
    // Command from server to client.
    byte pn = netbuffer->u.control.player_num;
    byte ps = netbuffer->u.control.player_state;

    if( !server )
    {
        if( pn < MAXPLAYERS )
            player_state[pn] = ps;
   
        gametic = LE_SWAP32(netbuffer->u.control.gametic);
        cl_need_tic = gametic;
        gamemap = netbuffer->u.control.gamemap;
        gameepisode = netbuffer->u.control.gameepisode;
    }

    // Situation specific
    switch( netbuffer->u.control.command )
    {
     case CTRL_normal:
        if( cl_mode < CLM_connected )
            cl_mode = CLM_connected;
        break;	 
     case CTRL_wait_game_start:
        if( (cl_mode < CLM_connected) && (ps == PS_join_wait_game_start) )
        {
            cl_mode = CLM_wait_game_start;
            wait_netplayer = 0;  // turn off wait display

            if( server )  break;  // protection, should not happen

            gamestate = GS_INTERMISSION;
        }
        break;	 
     case CTRL_game_start:
        if( cl_mode == CLM_wait_game_start )
        {
            cl_mode = CLM_connected;
            // Warn the player, who probably has dozed off by now.	   
            S_StartSound(sfx_menuop);  // always present

            if( server )  break;  // protection, should not happen

            G_Start_Intermission();  // setup intermission

            // update from server
            wait_game_start_timer = LE_SWAP16( netbuffer->u.control.data );
//            wait_netplayer = 0;  // only want the timer display
            S_StartSound(sfx_telept);  // longer sound, likely to be in all games
        }
        break;
     case CTRL_wait_timer:
        if( server )  break;  // protection, should not happen

        wait_game_start_timer = LE_SWAP16( netbuffer->u.control.data );
        break;
     default:
        break;	 
    }
}


// ----- Random State Update

// Send: Fill in the fields
static void get_random_state( random_state_t * rs )
{
    uint32_t rand2;
    rs->p_rand_index = P_Rand_GetIndex(); // to sync P_Random
    rs->b_rand_index = B_Rand_GetIndex(); // to sync B_Random
    rs->e_rand1 = LE_SWAP32_FAST( E_Rand_Get( & rand2 ) ); // to sync E_Random
    rs->e_rand2 = LE_SWAP32_FAST( rand2 );
}

#define SET_RANDOM    1
// Client
// Receive: Update client random state.
//   set_enable : 1=enable set,  0=check only
static void random_state_checkset( random_state_t * rs, const char * msg, byte set_enable )
{
    uint32_t  o_ernd1, o_ernd2, rs_ernd1, rs_ernd2;
    byte  o_pr, o_br, rs_pr, rs_br;

    if( ! msg )
    {
        msg = "Client random state repair";
    }

    o_pr = P_Rand_GetIndex();
    rs_pr = rs->p_rand_index;
    if( o_pr != rs_pr )
    {
        // Warn when different P_Random index.
        GenPrintf( EMSG_warn, "%s: gametic %i, update P_random index %i to %i\n",
             msg, gametic, o_pr, rs_pr );
        if( set_enable )
            P_Rand_SetIndex( rs_pr ); // to sync P_Random
    }
   
    o_br = B_Rand_GetIndex();
    rs_br = rs->b_rand_index;
    if( o_br != rs_br )
    {
        GenPrintf( EMSG_warn, "%s: update B_Random index %i to %i\n", msg, o_br, rs_br );
        if( set_enable )
            B_Rand_SetIndex( rs_br ); // to sync B_Random
    }

    o_ernd1 = E_Rand_Get( & o_ernd2 );
    rs_ernd1 = LE_SWAP32_FAST(rs->e_rand1);
    rs_ernd2 = LE_SWAP32_FAST(rs->e_rand2);
    if( o_ernd1 != rs_ernd1 || o_ernd2 != rs_ernd2 )
    {
        GenPrintf( EMSG_warn, "%s: update E_Random (%08X,%08X) to (%08X,%08X)\n",
                   msg, o_ernd1, o_ernd2, rs_ernd1, rs_ernd2 );
        if( set_enable )
            E_Rand_Set( rs_ernd1, rs_ernd2 ); // to sync E_Random
    }
}


// ----- Gametic and Pause State

// [WDJ] Update state by sever.
// By Server
void SV_Send_State( byte server_pause )
{
    netbuffer->packettype=PT_STATE;
    netbuffer->u.state.gametic = LE_SWAP32_FAST(gametic);
    get_random_state( & netbuffer->u.state.rs ); // to sync P_Random
    netbuffer->u.state.server_pause = server_pause;

    SV_SendPacket_All( true, sizeof(state_pak_t), NULL );
}

// By Client
static void state_handler( void )
{
    byte nnode = doomcom->remotenode;

    // Message is PT_STATE
    tic_t serv_gametic = LE_SWAP32_FAST(netbuffer->u.state.gametic);
    if( serv_gametic != gametic )
    {
        if( verbose > 1 )
            GenPrintf( EMSG_ver, "PT_STATE: node %i, client gametic %i, server gametic %i\n", nnode, gametic, serv_gametic );
        // Gametic cannot be fixed directly, need game commands.        
        // It may just be behind by a tic or two.
    }
    else
    {
        // Update the random generators.
        random_state_checkset( & netbuffer->u.state.rs, "PT_STATE", SET_RANDOM ); // to sync P_Random
    }
    paused = netbuffer->u.state.server_pause;

#if 0
    debug_Printf( "STATE: gametic %i, P_Random [%i], paused %i\n",
          gametic, netbuffer->u.state.rs.p_rand_index, netbuffer->u.state.server_pause );
#endif
}


// ----- Wait for Client Update to Finish

// NETWORK_WAIT_ACTIVE_FLAG is carefully choosen to avoid boolean values 0/1
#define NETWORK_WAIT_ACTIVE_FLAG   0x40
static uint16_t  network_wait_timer = 0;
static byte      network_wait_pause = 0;

// Server
//  wait_timeout : wait timeout in ticks
void  SV_network_wait_timer( uint16_t wait_timeout )
{
    if( wait_timeout > network_wait_timer )   network_wait_timer = wait_timeout;

    if( ! network_wait_pause )
    {
        // save pause state, and network pause state
        network_wait_pause = paused | NETWORK_WAIT_ACTIVE_FLAG;
        if( ! paused )
        {
            // All clients except the server.
            SV_Send_State( 1 );  // pause game
        }
        paused = network_wait_pause;
    }
}

// Server
// Wait when netgame is forced to pause.
static
void  SV_network_wait_handler( void )
{
    byte nn, ns;
   
    // Is everybody ready.
    for( nn=0; nn<MAXNETNODES; nn++ )
    {
        ns = nnode_state[nn];
        if((ns >= NOS_join) && (ns <= NOS_join_sg_loaded))  goto keep_waiting;
        if((ns >= NOS_repair) && (ns <= NOS_repair_sg_loaded))  goto keep_waiting;
    }
    goto unpause_game;
    
keep_waiting:
    if( network_wait_timer > 1 )
    {
        network_wait_timer--;
        return;
    }
        
    // Timeout the wait
    network_wait_timer = 0;
    for( nn=0; nn<MAXNETNODES; nn++ )
    {
        byte ns = nnode_state[nn];
        if( ns == NOS_idle ) continue;
       
        GenPrintf(EMSG_warn, "Network timeout: node=%i  nnode_state=%i\n", nn, ns );
        if((ns >= NOS_join) && (ns < NOS_join_timeout))
        {
            nnode_state[nn] = NOS_join_timeout;
            network_wait_timer = 2;
        }
        else if((ns >= NOS_repair) && (ns < NOS_repair_timeout))
        {
            nnode_state[nn] = NOS_repair_timeout;
            network_wait_timer = 2;
        }
        else if( ns == NOS_repair_timeout || ns == NOS_join_timeout || ns == NOS_fatal )
        {
            Net_CloseConnection(nn, 1); // force close
            SV_Reset_NetNode(nn);
        }
    }

    if( network_wait_timer == 0 )  goto unpause_game;  // let other players continue
    return;

unpause_game:
    if( network_wait_pause == NETWORK_WAIT_ACTIVE_FLAG )  // originally was not paused
    {
        // All clients except the server.
        SV_Send_State( 0 );  // unpause the game
    }
    paused &= ~NETWORK_WAIT_ACTIVE_FLAG;
    network_wait_pause = 0;
    network_wait_timer = 0;
    return;
}


// By Server
static void  ready_handler( byte nnode )
{
    byte  ns = nnode_state[nnode];

    // Client is ready
    if( ns == NOS_join_savegame )
    {
        nnode_state[nnode] = NOS_join_sg_loaded;  // signal to SV_Add_Join_Waiting()
        SV_Add_Join_Waiting();
    }
}


// ---- Client Update Savegame

#ifdef JOININGAME

// [WDJ] For little-endian and big-endian machines to play together, must fix savegames.
// Each machine writes savegames in its native format, and must be able
// to read its own older savegames, so cannot just change to LE savegames.
// A big-endian machine needs to be able to read either endian savegames.
// Other possibilities require also changing the write of savegames, at least for big-endian servers.

// By Server.
// Send a save game to the client.
static void SV_Send_SaveGame(int to_node)
{
    size_t  length;

    SV_network_wait_timer( 90 );  // pause game during download

    P_Alloc_savebuffer( 1 );	// large buffer, but no header
    if(! savebuffer)   goto buffer_err;

    P_Savegame_Write_header( NULL, 1 );  // Netgame header
    P_Savegame_Save_game();  // fill buffer with game data
    // buffer will automatically grow as needed.

    length = P_Savegame_length();
    if( length < 0 )   goto buffer_err;	// overrun buffer

    // then send it !
    SV_SendData(to_node, "SAVEGAME", savebuffer, length, TAH_MALLOC_FREE, SAVEGAME_FILEID);
    // SendData frees the savebuffer using free() after it is sent.
    // This is the only use of TAH_MALLOC_FREE.

    // Wait for reply. Let wait timer do unpause.
    return;

buffer_err:
    GenPrintf(EMSG_error, "Send_savegame: cannot allocate large enough buffer\n" );
    return;   
}

// Dummy name for a savegame file.
static const char *tmpsave="$$$.sav";

// By Client.
// Act upon the received save game from server.
// Success: cl_mode = CLM_download_done
// Fail:    cl_mode = CLM_fatal
static void CL_Load_Received_Savegame(void)
{
    savegame_info_t   sginfo;  // read header info

    // Use savebuffer and save_p from p_saveg.c.
    // There cannot be another savegame in progress when this occurs.
    // [WDJ] Changed to use new load savegame file, with smaller buffer.
    if( P_Savegame_Readfile( tmpsave ) < 0 )  goto cannot_read_file;
    // file is open and savebuffer allocated

    // Read netgame header.
    sginfo.msg[0] = 0;
    if( ! P_Savegame_Read_header( & sginfo, 1 ) )  goto load_failed;

    GenPrintf(EMSG_hud, "Loading savegame\n");

    G_setup_VERSION();

    // Sever will control pause during download.
    demoplayback  = false;
    automapactive = false;

    // load a base level
    playerdeadview = false;

    P_Savegame_Load_game(); // read game data in savebuffer, defer error test
    if( P_Savegame_Closefile( 0 ) < 0 )  goto load_failed;
    // savegame buffer deallocated, and file closed

    // done
    unlink(tmpsave);  // delete file (posix)
    consistency[ BTIC_INDEX( gametic ) ] = Consistency();
    CON_ToggleOff ();

    cl_mode = CLM_download_done;
    return;

cannot_read_file:
    I_SoftError ("Can't read savegame sent\n");
    goto failed_exit; // must deallocate savebuffer

load_failed:
    GenPrintf(EMSG_error, "Can't load the level !!!\n%s", sginfo.msg);
failed_exit:
    // needed when there are error tests before Closefile.
    P_Savegame_Error_Closefile();  // deallocate savebuffer
    cl_mode = CLM_fatal;
    return;
}

#endif



// ----- Consistency fail, repair position.

// By Server.
// Send a player repair message.
//   pind : player index, 0=main player, 1=splitscreen player
//   to_node : the net node
static void SV_Send_player_repair( byte pind, byte to_node )
{
    const char * fail_msg;
    mobj_t * mo;

    byte pn = nnode_to_player[pind][ to_node ];
    if( pn >= MAXPLAYERS )  return;

    if( ! playeringame[pn] )
    {
        fail_msg = "not in game";
        goto fail;
    }

    mo = players[pn].mo;
    if( ! mo )
    {
        fail_msg = "no mobj";
        goto fail;
    }

    // Enough to fix a small difference in player position.
    netbuffer->u.repair.repair_type = RQ_PLAYER;
    netbuffer->u.repair.pos.id_num = LE_SWAP16( pn );
    netbuffer->u.repair.pos.angle = LE_SWAP32( mo->angle );
    netbuffer->u.repair.pos.x = LE_SWAP32( mo->x );
    netbuffer->u.repair.pos.y = LE_SWAP32( mo->y );
    netbuffer->u.repair.pos.z = LE_SWAP32( mo->z );
    netbuffer->u.repair.pos.momx = LE_SWAP32( mo->momx );
    netbuffer->u.repair.pos.momy = LE_SWAP32( mo->momy );
    netbuffer->u.repair.pos.momz = LE_SWAP32( mo->momz );

    byte errcode = HSendPacket( to_node, SP_reliable, 0, sizeof(repair_pak_t) );
    if( errcode >= NE_fail )
    {
        nnode_state[to_node] = NOS_fatal;
        fail_msg = "Send fail";       
        goto fail;
    }

    GenPrintf( EMSG_warn, "Server Send_player_repair: player %i\n", pn );
    return;

fail:
    GenPrintf( EMSG_warn, "Server Send_player_repair: player %i, %s\n", pn, fail_msg );
    return;
}


// By Client.
// Repair the player from the repair message in the netbuffer.
static void CL_player_repair( void )
{
    const char * fail_msg;
    mobj_t * mo;
    byte pn;

    pn = (uint16_t) LE_SWAP16( netbuffer->u.repair.pos.id_num );
    if( ! playeringame[pn] )
    {
        fail_msg = "not in game";
        goto fail;
    }

    mo = players[pn].mo;
    if( ! mo )
    {
        fail_msg = "no mobj";
        goto fail;
    }

    angle_t  r_angle = LE_SWAP32( netbuffer->u.repair.pos.angle );
    fixed_t  r_x = LE_SWAP32( netbuffer->u.repair.pos.x );
    fixed_t  r_y = LE_SWAP32( netbuffer->u.repair.pos.y );
    fixed_t  r_z = LE_SWAP32( netbuffer->u.repair.pos.z );
    fixed_t  r_momx = LE_SWAP32( netbuffer->u.repair.pos.momx );
    fixed_t  r_momy = LE_SWAP32( netbuffer->u.repair.pos.momy );
    fixed_t  r_momz = LE_SWAP32( netbuffer->u.repair.pos.momz );

    if( mo->angle != r_angle )
    {
        GenPrintf( EMSG_warn, "Client player_repair: ANGLE  client %x  server %x\n", mo->angle, r_angle );
        mo->angle = r_angle;
    }

    if(  mo->x != r_x || mo->y != r_y || mo->z != r_z )
    {
        GenPrintf( EMSG_warn, "Client player_repair: POS  client (%x.%x, %x.%x, %x.%x)  server (%x.%x, %x.%x, %x.%x)\n",
                   mo->x>>16, mo->x&0xFFFF, mo->y>>16, mo->y&0xFFFF, mo->z>>16, mo->z&0xFFFF,
                   r_x>>16, r_x&0xFFFF, r_y>>16, r_y&0xFFFF, r_z>>16, r_z&0xFFFF  );
        mo->x = r_x;
        mo->y = r_y;
        mo->z = r_z;
    }

    if(  mo->momx != r_momx || mo->momy != r_momy || mo->momz != r_momz )
    {
        GenPrintf( EMSG_warn, "Client player_repair: MOM  client (%x.%x, %x.%x, %x.%x)  server (%x.%x, %x.%x, %x.%x)\n",
                   mo->momx>>16, mo->momx&0xFFFF, mo->momy>>16, mo->momy&0xFFFF, mo->momz>>16, mo->momz&0xFFFF,
                   r_momx>>16, r_momx&0xFFFF, r_momy>>16, r_momy&0xFFFF, r_momz>>16, r_momz&0xFFFF  );
        mo->momx = r_momx;
        mo->momy = r_momy;
        mo->momz = r_momz;
    }
    return;

fail:
    GenPrintf( EMSG_warn, "Server Send_player_repair: player %i, %s\n", pn, fail_msg );
    return;
}

// By Server.
//  to_node : to the player node
//  repair_type : RQ_PLAYER, RQ_SUG_SAVEGAME
static void SV_Send_repair( byte repair_type, byte to_node )
{
    netbuffer->packettype = PT_REPAIR;
    netbuffer->u.repair.gametic = LE_SWAP32(gametic);
    get_random_state( & netbuffer->u.repair.rs ); // to sync P_Random

    netbuffer->u.repair.repair_type = repair_type;
    if( repair_type == RQ_PLAYER )
    {
        SV_Send_player_repair( 0, to_node );  // main player
        SV_Send_player_repair( 1, to_node );  // splitscreen
        return;
    }

    // simple messages
    byte errcode = HSendPacket( to_node, SP_reliable|SP_queue, 0, sizeof(repair_pak_t) );
    if( errcode >= NE_fail )
    {
        nnode_state[to_node] = NOS_fatal;
        generic_network_error_handler( errcode, "Send Repair" );
    }
    return;
}


// By Client.
//  repair_type : RQ_REQ_SAVEGAME, RQ_REQ_PLAYER, RQ_CLOSE_ACK
static void CL_Send_Req_repair( repair_type_e repair_type )
{
    netbuffer->packettype = PT_REPAIR;
    netbuffer->u.repair.repair_type = repair_type;

    // The fields are there, so fill them.  Easier to fill them everytime.
    netbuffer->u.repair.gametic = LE_SWAP32(gametic);
    get_random_state( & netbuffer->u.repair.rs ); // to sync P_Random

    byte errcode = HSendPacket( cl_servernode, SP_reliable|SP_queue, 0, sizeof(repair_pak_t) );
    if( errcode >= NE_fail )
    {
        network_state = NETS_fatal;
        generic_network_error_handler( errcode, "Send Repair" );
    }
}


// [WDJ] Attempt to fix consistency errors.
// PT_REPAIR
// By Client
static void repair_handler_client( byte nnode )
{
    // Message is PT_REPAIR
    byte msg_type = netbuffer->u.repair.repair_type;

    if( server )
        return;  // Ignore attempts to corrupt server.

    if( msg_type < RQ_REQ_TO_SERVER )
    {
        // Server repairs client.
        uint32_t net_gametic = LE_SWAP32_FAST(netbuffer->u.repair.gametic);
        if( gametic != net_gametic )
        {
            GenPrintf( EMSG_warn, "Client repair: gametic client %u  server %u\n", gametic, net_gametic );
            gametic = net_gametic;
        }
        random_state_checkset( & netbuffer->u.repair.rs, NULL, SET_RANDOM ); // to sync P_Random
    }

    switch( msg_type )
    {
     case RQ_SUG_SAVEGAME:
#ifdef JOININGAME
        // CLIENT: Server suggests downloading a savegame from the server.
        if(( cv_download_savegame.EV == 0 ) || ( cv_netrepair.EV < 2 ))
        {
            CL_Send_Req_repair( RQ_REQ_PLAYER ); // to server
            break;
        }

        // Client: prepare for savegame download.
        cl_mode = CLM_download_savegame;
        CL_Prepare_download_savegame(tmpsave);
        CL_Send_Req_repair( RQ_REQ_SAVEGAME ); // to server
        // Loop here while savegame is downloaded.
        while( netfile_download )
        {
            Net_Packet_Handler();
            if( network_state < NETS_active )  // Need client to server connection
                goto reset_to_title_exit;  // connection closed by cancel or timeout
#if 1
            if( !server && !netgame )
                goto reset_to_title_exit;  // connection closed by cancel or timeout
#endif

            Net_AckTicker();

            // Operations performed only once every tic.
            if( cl_prev_tic != I_GetTime() )
            {
                cl_prev_tic = I_GetTime();

                // User response handler
                I_OsPolling();
                switch( I_GetKey() )
                {
                  case 'q':
                  case KEY_ESCAPE:
                     goto reset_to_title_exit;
                }

                // Server upkeep
                if( Filetx_file_cnt )  // File download in progress.
                    Filetx_Ticker();
            }
        }

        // Have received the savegame from the server.
        CL_Load_Received_Savegame();  // CLM_download_done, or CLM_fatal
        if( cl_mode != CLM_download_done )
        {
            GenPrintf( EMSG_error, "Client player_repair: savegame failed\n" );
            if( cv_netrepair.EV < 3 )  goto reset_to_title_exit;
            // aggressive
            if( cl_error_status > 2 )  goto reset_to_title_exit;
            cl_error_status = 3;  // retry once
            // Try again
            CL_Send_Req_repair( RQ_REQ_SAVEGAME ); // to server
            break;
        }
        // Download done
        CL_Send_Req_repair( RQ_CLOSE_ACK ); // to server
        cl_mode = CLM_connected;
        goto close_repair;
#else
        CL_Send_Req_repair( RQ_REQ_PLAYER ); // to server
#endif
        break;

     case RQ_SAVEGAME_REJ:
        // Server rejects savegame.
        CL_Send_Req_repair( RQ_REQ_PLAYER ); // to server
        break;

     case RQ_PLAYER:
        // Server repairs player position.
        CL_player_repair();
        cl_mode = CLM_connected;
        goto close_repair;

     default:
        break;
    }
    return;

close_repair:
    // Client closes the repair state. 
    if( cl_mode != CLM_fatal )
        CL_Send_Req_repair( RQ_CLOSE_ACK ); // to server
    cl_error_status = 1;
    return;

reset_to_title_exit:
    if( cl_mode != CLM_fatal )
        D_Quit_NetGame(); // to server
    CL_Reset();
    D_StartTitle();
    return;
}

// PT_REPAIR
// By Server only.
static void repair_handler_server( byte nnode )
{
    // Message is PT_REPAIR
    random_state_checkset( & netbuffer->u.repair.rs, "Repair", 0 ); // to check P_Random
    byte msg_type = netbuffer->u.repair.repair_type;
    switch( msg_type )
    {
     case RQ_REQ_SAVEGAME:
        // Client has requested a savegame repair
#ifdef JOININGAME
        if( (cv_SV_download_savegame.EV == 0) || (cv_SV_netrepair.EV < 2) )
        {
            GenPrintf(EMSG_info, "Repair: Send savegame not allowed\n" );
        }
        else if(gamestate == GS_LEVEL)
        {
            nnode_state[nnode] = NOS_repair_savegame;
            SV_Send_SaveGame( nnode ); // send game data
            // netwait timer is running
            GenPrintf(EMSG_info, "Repair: Send savegame\n");
            // Client will return  RQ_REQ_SAVEGAME, RQ_REQ_PLAYER, RQ_CLOSE_ACK, or PT_CLIENTQUIT
            break;	    
        }
#else
        GenPrintf(EMSG_info, "Repair: Send savegame not allowed, no Join-in-game.\n" );
#endif
        SV_Send_repair( RQ_SAVEGAME_REJ, nnode );
        break;
       
     case RQ_REQ_PLAYER:
        // Client has requested a player repair
        SV_network_wait_timer( 18 );  // keep alive
        nnode_state[nnode] = NOS_repair_player;
        SV_Send_repair( RQ_PLAYER, nnode );  // includes player1, player2 position
        break;

     case RQ_CLOSE_ACK:
        // Client is ending the repair.
//        random_state_checkset( & netbuffer->u.repair.rs, "Repair Close", 0 ); // to check P_Random
        nnode_state[nnode] = NOS_active;
        GenPrintf(EMSG_warn, "Repair: closed.\n" );
        SV_network_wait_handler();
        break;

     default:
        break;
    }
}


// ----- Wait for Server to start net game.
//#define WAITPLAYER_DEBUG

static byte  num_netnodes;
static byte  num_netplayer;  // wait for netplayer, some nodes are 2 players
static tic_t wait_tics  = 0;

// By Server
static void SV_Send_NetWait( void )
{
    netbuffer->packettype = PT_NETWAIT;
    netbuffer->u.netwait.num_netplayer = num_netplayer;
    netbuffer->u.netwait.wait_netplayer = wait_netplayer;
    netbuffer->u.netwait.wait_tics = LE_SWAP16( wait_tics );
    get_random_state( & netbuffer->u.netwait.rs ); // to sync P_Random
#ifdef WAITPLAYER_DEBUG
    debug_Printf( "WaitPlayer update: num_netnodes=%d num_netplayer=%d  wait_netplayer=%d  wait_tics=%d\n",
               num_netnodes, num_netplayer, wait_netplayer, wait_tics );
    SV_SendPacket_All( false, sizeof(netwait_pak_t), "  sent to player" );
#else
    SV_SendPacket_All( false, sizeof(netwait_pak_t), NULL );
#endif
}

// By Client
static void netwait_handler( void )
{
    // Updates of wait for players, from the server.
    num_netplayer = netbuffer->u.netwait.num_netplayer;
    wait_netplayer = netbuffer->u.netwait.wait_netplayer;
    wait_tics = LE_SWAP16( netbuffer->u.netwait.wait_tics );
    random_state_checkset( & netbuffer->u.netwait.rs, NULL, SET_RANDOM ); // to sync P_Random
}

// Called from D_Display when GS_WAITINGPLAYERS, CL_ConnectToServer?
void D_WaitPlayer_Drawer( void )
{
    WI_Draw_wait( num_netnodes, num_netplayer, wait_netplayer, wait_tics );
}

// Called by Menu M_StartServer
void D_WaitPlayer_Setup( void )
{
    if( netgame )
    {
        if( server )
        {
            // Wait for all player nodes, during netgame.
            wait_netplayer = cv_wait_players.value;
            wait_tics = cv_wait_timeout.value * TICRATE;
        }
        else
        {
            // Wait indefinite, until server updates the wait.
            wait_netplayer = 99;
            wait_tics = 0;
        }
    }
    else
    {
        // Single player and local games.
        wait_netplayer = 0;
        wait_tics = 0;
    }
    gamestate = wipegamestate = GS_WAITINGPLAYERS;
}

// Return true when start game.
static boolean  D_WaitPlayer_Ticker(void)
{
    int  nn;

    if( server )
    {
        // Count the net nodes.
        num_netnodes=0;
        num_netplayer=0;
        for(nn=0; nn<MAXNETNODES; nn++)
        {
            // Only counting nodes with players.
            if( nnode_state[nn] >= NOS_client )
            {
                if( playerpernode[nn] > 0 )
                {
                    num_netnodes ++;
                    num_netplayer += playerpernode[nn];
                }
            }
        }

        if( wait_tics > 0 || wait_netplayer > 0 )
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
    if( wait_netplayer > 0 )
    {
        // Waiting on player net nodes, with or without timeout.
        if( num_netplayer < wait_netplayer )
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
        debug_Printf( "Start game sent to players at tic=%d\n", gametic   );
#endif
    }
    return true;  // start game
    

wait_ret:
    return false;  // keep waiting
}

boolean  D_WaitPlayer_Response( int key )
{
    // [WDJ] This should not respond to the ENTER key.  Users will be pressing ENTER quickly and often.
    // This message will stop them, and only the correct response will start the unreversible action.

    // Translate for joystick.  The PAUSE is not otherwise used during WAITPLAYER and does not conflict.
    if( key == gamecontrol[gc_pause][0] || key == gamecontrol[gc_pause][1]
        || (key >= KEY_JOY0BUT2 && key <= KEY_JOY0BUT5 ) )
        key = 's';
   
    // User response handler
    switch( key )
    {
     // Also gc_menuesc (translated to KEY_ESCAPE)
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
            wait_netplayer = 0;
            wait_tics = 0;
            SV_Send_NetWait();
#ifdef WAITPLAYER_DEBUG
            debug_Printf( "Start game (key) sent at tic=%d\n", gametic );
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
//   to_node : when BROADCAST_NODE then all servers will respond
// Called by: CL_Broadcast_AskInfo, CL_Update_ServerList, CL_ConnectToServer
static void CL_Send_AskInfo( byte to_node )
{
    netbuffer->packettype = PT_ASKINFO;
    netbuffer->u.askinfo.version = VERSION;
    netbuffer->u.askinfo.send_time = LE_SWAP32(I_GetTime());
    HSendPacket( to_node, 0, 0, sizeof(askinfo_pak_t) );  // msg lost when too busy
}


// By Client.
// Broadcast to find some servers.
//   addrstr: broadcast addr string
// Called by: CL_Update_ServerList
static void CL_Broadcast_AskInfo( char * addrstr )
{
    // Modifies the broadcast address.
    if( addrstr
        && Bind_Node_str( BROADCAST_NODE, addrstr, server_sock_port ) )
    {
//        debugPrintf( "CL_Broadcast_AskInfo  server_sock_port = %d\n", server_sock_port );       
       
        CL_Send_AskInfo( BROADCAST_NODE );
    }
}


// --- ServerList

server_info_t serverlist[MAXSERVERLIST];
int serverlistcount=0;

// Clear the serverlist, closing connections.
//  keep_node: except this server node
static void SL_Clear_ServerList( int keep_node )
{
    int i;
    for( i=0; i<serverlistcount; i++ )
    {
        if( serverlist[i].server_node != keep_node )
        {
            Net_CloseConnection(serverlist[i].server_node, 0);
            serverlist[i].server_node = 0;
        }
    }
    serverlistcount = 0;
}

// Find the server in the serverlist.
// Called by: SL_InsertServer, CL_ConnectToServer
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
static void SL_InsertServer( serverinfo_pak_t * info, byte nnode)
{
    tic_t  test_time;
    server_info_t se;
    int i, i2;

    // search if not already on it
    i = SL_Find_Server( nnode );
    if( i < 0 )
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
        server = false;  // To get correct port
        if( ! I_NetOpenSocket() )  return;  // failed to get socket

        netgame = true;
        multiplayer = true;
        network_state = NETS_open;
        quit_netgame_status = 0;
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
//  cl_servernode: if set then reconnect, else search
static void CL_ConnectToServer( void )
{
    int  i;
    tic_t   askinfo_tic = 0;  // to repeat askinfo

    if( network_state < NETS_open )
         goto reset_to_title_exit;  // connection closed by cancel or timeout

    network_state = NETS_connecting;
   
    cl_mode = CLM_searching;
    D_WaitPlayer_Setup();

    if( cl_servernode >= MAXNETNODES )
    {
        // init value and BROADCAST_NODE
        GenPrintf(EMSG_hud, "Searching for a DoomLegacy server ...\n");
    }
    else
        GenPrintf(EMSG_hud, "Contacting the DoomLegacy server ...\n");

    DEBFILE(va("Waiting %d players\n", wait_netplayer));

    SL_Clear_ServerList(cl_servernode);  // clear all except the current server

    GenPrintf(EMSG_hud, "Press Q or ESC to abort\n");
    // Every player goes through here to connect to game, including a
    // single player on the server.
    // Loop until connected or user escapes.
    // Because of the combination above, this loop must include code for
    // server responding.
    do
    {
        // Because of console draw in the loop
        V_SetupDraw( 0 | V_SCALESTART | V_SCALEPATCH | V_CENTERHORZ );

        switch(cl_mode) {
            case CLM_searching :
                // serverlist is updated by GetPacket function
                if( serverlistcount > 0 )
                {
                    cl_mode = CLM_server_files;
                    break;
                }
                // Don't have a serverlist.
                // Poll the server (askinfo packet).
                if( askinfo_tic <= I_GetTime() )
                {
                    // Don't be noxious on the network.
                    // Every 2 seconds is often enough.
                    askinfo_tic = I_GetTime() + (TICRATE*2);
                    if( cl_servernode < MAXNETNODES )
                    {
                        // Specific server.
                        CL_Send_AskInfo(cl_servernode);
                    }
                    else
                    {
                        // Any
                        CL_Update_ServerList( false );
                    }
                }
                break;
            case CLM_server_files :
                // Have a serverlist, serverlistcount > 0.
                // This can be a response to our broadcast request
                if( cl_servernode < MAXNETNODES )
                {
                    // Have a current server.  Find it in the serverlist.
                    i = SL_Find_Server(cl_servernode);
                    // Check if it shutdown, or is missing for some reason.
                    if (i<0)
                        return;  // go back to user
                }
                else
                {
                    // Invalid cl_servernode, get best server from serverlist.
                    i = 0;
                    cl_servernode = serverlist[i].server_node;
                    GenPrintf(EMSG_hud, " Found, ");
                }
                // Check server for files needed.
                CL_Got_Fileneed(serverlist[i].info.num_fileneed,
                                serverlist[i].info.fileneed    );
                GenPrintf(EMSG_hud, " Checking files ...\n");
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
            case CLM_download_req:
                // Must download something.
                // Check -nodownload switch, cv_download_files, or request downloads.
                switch( Send_RequestFile() )
                {
                 case RFR_success:
                    Net_GetNetStat();  // init for later display
                    cl_mode = CLM_download_files;
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
            case CLM_download_files :
                // Wait test, and display loading files.
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
                CL_Prepare_download_savegame(tmpsave);
#endif
                if( CL_Send_Join() )  // join game
                    cl_mode = CLM_wait_join_response;
                if( network_state < NETS_connecting )  // Need client to server connection
                    goto reset_to_searching;
#if 1
                if( !server && !netgame )
                    goto reset_to_searching;
#endif
                break;
            case CLM_wait_join_response :
                // waiting for server response	   
                // see server_cfg_handler()
                break;
            case CLM_download_savegame :
#ifdef JOININGAME
                M_DrawTextBox( 2, NETFILE_BOX_Y, 38, 4);
                V_DrawString (30, NETFILE_BOX_Y+8, 0, "Download Savegame");
                if( netfile_download )
                    break; // continue loop

                // Have received the savegame from the server.
                CL_Load_Received_Savegame();
                if( cl_mode != CLM_download_done )
                {
                    // send client quit
                    goto reset_to_searching;
                }

                // got savegame
                if( SendPacket( cl_servernode, PT_CLIENTREADY ) >= NE_fail )
                    goto reset_to_searching;

                gamestate = GS_LEVEL;  // game loaded
                cl_mode = CLM_connected;
                break;
#else	   
                goto reset_to_searching;  // should not end up in this state
#endif

            case CLM_wait_game_start :
                // Not going to get a download.
                if( netfile_download )
                    CL_Cancel_download_savegame();

                M_DrawTextBox( 2, NETFILE_BOX_Y, 38, 4);
                V_DrawString (30, NETFILE_BOX_Y+8, 0, "Wait Game Start");
                // wait for control msg
                break;

            default:
                break;
            // CLM_connected will exit the loop
        }

        Net_Packet_Handler();
        if( network_state < NETS_connecting )  // Need client to server.
            goto reset_to_searching;  // connection closed by cancel or timeout
        if( !server && !netgame )
            goto reset_to_searching;  // connection closed by cancel or timeout

        Net_AckTicker();

        // Operations performed only once every tic.
        if( cl_prev_tic != I_GetTime() )
        {
            cl_prev_tic = I_GetTime();

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

#if 1
   if( cl_mode > CLM_searching )
   {
      // Nothing bad, just not the problem
            // Supporting the wait during connect, like it was in the previous
            // code, has marginal value.  Seems to cause more problems.
            D_WaitPlayer_Ticker();
            if( wait_tics > 0 || wait_netplayer > 0 )
                D_WaitPlayer_Drawer();
   }
#endif
            CON_Drawer ();
            I_FinishUpdate ();              // page flip or blit buffer
        }
    } while ( cl_mode != CLM_connected );

    // If this is still set, then did not get a savegame, so turn it off.
    if( netfile_download )
        CL_Cancel_download_savegame();
   
    network_state = NETS_active;

    DEBFILE(va("Synchronization Finished\n"));

    // [WDJ] consoleplayer no longer has DRONE bit
    displayplayer = consoleplayer;
    consoleplayer_ptr = displayplayer_ptr = &players[consoleplayer];
    return;

reset_to_searching:
    if( cl_mode >= CLM_download_req )
    {
        SendPacket( cl_servernode, PT_CLIENTQUIT );  // ignore failure
    }
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
    quit_netgame_status = 0;

    if( strcasecmp(COM_Argv(1),"self")==0 )
    {
        cl_servernode = 0;  // server is self
        cl_server_state = NOS_idle;
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
            cl_servernode = atoi(COM_Argv(2));
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
            network_state = NETS_open;

            if( strcasecmp(COM_Argv(1),"any")==0 )
            {
                // Connect to first lan server found.
                cl_servernode = BROADCAST_NODE;
            }
            else
            if( I_NetMakeNode )
            {
                // Connect to server at IP addr.
                cl_servernode = I_NetMakeNode(COM_Argv(1));
            }
            else
            {
                CONS_Printf("There is no server identification with this network driver\n");
                D_CloseConnection();
                network_state = NETS_idle;
                return;
            }
        }
    }
    CL_ConnectToServer();
}


// Called by Kick cmd.
static void CL_RemovePlayer( byte playernum )
{
    player_t * player;
    int i;

    // Cannot trust an index from a network message.
    if( playernum >= MAXPLAYERS )  return;
    player = & players[playernum];
   
    if( server && !demoplayback )
    {
        byte nnode = player_to_nnode[playernum];
        if( playerpernode[nnode] )
            playerpernode[nnode]--;
        if( playerpernode[nnode] == 0 )
        {
            // No more players at this node.	   
            Net_CloseConnection(nnode, 0);
            SV_Reset_NetNode(nnode);  // node_state
        }
    }

    playeringame[playernum] = false;
    player_state[playernum] = 0;
    player_to_nnode[playernum] = 255;
   
    // we should use a reset player but there is not such function
    // Reduce the player slots in the messages.  Count the players.
    num_player_slots = 1;
    num_game_players = 0;
    for(i=0;i<MAXPLAYERS;i++)
    {
        players[i].addfrags += players[i].frags[playernum];
        players[i].frags[playernum] = 0;
        player->frags[i] = 0;

        // count remaining players
        if( playeringame[i] )
        {
            num_player_slots = i+1;
            num_game_players++;
        }
    }
    player->addfrags = 0;

    // remove avatar of player
    if( player->mo )
    {
        player->mo->player = NULL;
        P_RemoveMobj( player->mo );
    }
    player->mo = NULL;
   
    B_Destroy_Bot( player );
}

// By Client and non-specific code, to reset client connect.
void CL_Reset (void)
{
    if (demorecording)
        G_CheckDemoStatus ();

    // reset client/server code
    DEBFILE(va("==== Client reset ====\n"));

    if( cl_servernode < MAXNETNODES )
    {
        // Close connection to server
        // Client keeps nnode_state for server.	
        cl_server_state = NOS_idle;
        Net_CloseConnection(cl_servernode, 0);
    }
    D_CloseConnection();         // netgame=false
    multiplayer = false;
    cl_servernode=0;  // server to self
    server=true;
#ifdef DOSNET_SUPPORT
    doomcom->num_player_netnodes=1;
#endif
    num_player_slots = 1;
    cl_error_status = 0;
    SV_StopServer();
    SV_ResetServer();

    T_Clear_HubScript(); //DarkWolf95: Originally implemented by Exl
    fs_fadealpha = 0;
    HU_Clear_FSPics();

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

void Got_NetXCmd_KickCmd(xcmd_t * xc)
{
    byte pnum=READBYTE(xc->curpos);  // unsigned player num
    byte msg =READBYTE(xc->curpos);  // unsigned kick message

    if( pnum >= MAXPLAYERS )  return;
    GenPrintf(EMSG_hud, "\2%s ", player_names[pnum]);

    switch(msg)
    {
       case KICK_MSG_GO_AWAY:
               GenPrintf(EMSG_hud, "has been kicked (Go away)\n");
               break;
       case KICK_MSG_CON_FAIL:
               GenPrintf(EMSG_hud, "has been kicked (Consistency failure)\n");
               break;
       case KICK_MSG_TIMEOUT:
               GenPrintf(EMSG_hud, "left the game (Connection timeout)\n");
               break;
       case KICK_MSG_PLAYER_QUIT:
               GenPrintf(EMSG_hud, "left the game\n");
               break;
    }

    if( pnum == (byte)consoleplayer )
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
consvar_t cv_maxplayers     =
  {"sv_maxplayers","32",CV_NETVAR,maxplayers_cons_t,NULL,32};

static void Got_NetXCmd_AddPlayer(xcmd_t * xc);
static void Got_NetXCmd_AddBot(xcmd_t * xc);	//added by AC for acbot

// Called one time at init, by D_Startup_NetGame.
void D_Init_ClientServer (void)
{
  DEBFILE(va("==== %s debugfile ====\n", VERSION_BANNER));

    network_state = NETS_idle;
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
//    debug_Printf( "viewangleoffset=%i\n", viewangleoffset );

    COM_AddCommand("playerinfo",Command_PlayerInfo, CC_info);
    COM_AddCommand("kick",Command_Kick, CC_net);
    COM_AddCommand("connect",Command_connect, CC_net);

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

// By Server
// nnode: 0..(MAXNETNODES-1)
static void SV_Reset_NetNode(byte nnode)
{
    nnode_state[nnode] = NOS_idle;
    nnode_to_player[0][nnode] = 255;
    nnode_to_player[1][nnode] = 255;
    nettics[nnode]=gametic;
    nextsend_tic[nnode]=gametic;
    join_waiting[nnode]=0;
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
        SV_Reset_NetNode(i);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        playeringame[i]=false;
        player_state[i] = 0;
        player_to_nnode[i] = 255;
    }

    cl_nnode=0;
    cl_packetmissed=false;
    viewangleoffset=0;

    if( dedicated )
    {
        // [WDJ] FIXME, why dedicated required node_in_game[0].
        nnode_state[0] = NETS_internal;
        serverplayer = 255;  // no server player
    }
    else
        serverplayer=consoleplayer;

    if(server)
    {
        cl_servernode=0;  // server to self
        cl_server_state = NOS_idle;
    }

    SV_update_player_counts();
    num_player_slots = 0;
    quit_netgame_status = 0;

    DEBFILE(va("==== Server Reset ====\n"));
}

//
// D_Quit_NetGame
// Server or Client
// Called before quitting to leave a net game
// without hanging the other players
//
// Called by D_Quit_Save, D_WaitPlayer_Response, Command_ExitGame_f, repair_handler
void D_Quit_NetGame (void)
{
    if (!netgame)
        return;

    // [WDJ] Avoid the tight loop when the network fails.
    if( quit_netgame_status > 0 )
        return;

    quit_netgame_status = 1;
     
    DEBFILE("==== Quiting Game, closing connection ====\n" );

    // abort send/receive of files
    Close_NetFile();

    if( server )
    {
        // Server sends shutdown to all clients.
        byte nn; // net node num
        for(nn=0; nn<MAXNETNODES; nn++)
        {
            if( nnode_state[nn] >= NOS_recognized )
                SendPacket( nn, PT_SERVERSHUTDOWN );  // ignore failure
        }
        // Close registration with the Master Server.
        if ( serverrunning && cv_internetserver.value )
             MS_UnregisterServer(); 
    }
    else  // Client not server
    if( (cl_servernode < MAXNETNODES)
       && (cl_server_state == NOS_server) )  // client connected to server
    {
        // Client sends quit to server.
        SendPacket( cl_servernode, PT_CLIENTQUIT );  // ignore failure
    }

    D_CloseConnection();
    cl_server_state = NOS_idle;
    network_state = NETS_shutdown;

    DEBFILE("==== Log finish ====\n" );
#ifdef DEBUGFILE
    if (debugfile)
    {
        fclose (debugfile);
        debugfile = NULL;
    }
#endif
}

// By Server.
// Add a node to the game (player will follow at map change or at savegame....)
//  set_nnode_state : the new nnode_state
static
void SV_AddNode(byte nnode, byte set_nnode_state)
{
    nettics[nnode]       = gametic;
    nextsend_tic[nnode]  = gametic;

    // [WDJ] nnode_state is client state maintained by server.
    // This includes the client on the server.  It will not start up correctly
    // if the nnode_state is not kept correctly.
    // This used to be blocked for nnode==0 because the server was setting
    // node_in_game to get packets working.  This bypass is no longer done.
    if( nnode_state[nnode] < NOS_client )
        nnode_state[nnode] = set_nnode_state;
}

// Get a free player node.  Obey the rules.
byte  SV_get_player_num( void )
{
    // The server player_state determines availability.
    byte pn = 0;
    while( player_state[pn] )  // find free player slot
    {
       pn++;
       if( pn >= MAXPLAYERS )  return  251;
    }
    
    player_state[pn] = PS_added; // pending
    return pn;
}

// Commit to using a player num slot.
//   nnode : node of the player
//   new_state : the new player state
// Return the new player num.
// Return 255 if too many players or too many player for the node.
byte SV_commit_player( byte nnode, byte new_state )
{
    byte newplayernum, pind;
    // Search for a free playernum.
    // New players will have playeringame set as a result of XCmd AddPlayer.
    pind = playerpernode[nnode];  // next pind
    if( pind > 1 )
        return 255;
    
    newplayernum = SV_get_player_num();
            
#ifdef PARANOIA
    // Should never happen because we check the number of players
    // before accepting the join.
    if(newplayernum >= MAXPLAYERS)
    {
        I_SoftError("SV_commit_player: Reached MAXPLAYERS\n");
        return 255;
    }
#endif
    
    // Commit the server network settings.
    playerpernode[nnode]++;

    player_state[newplayernum] = new_state;
    player_pind[newplayernum] = pind;
    player_to_nnode[newplayernum] = nnode;
    nnode_to_player[pind][nnode] = newplayernum;
    return newplayernum;
}



// Server
// More accurate than purely inc and dec. There are too many odd ways to kill a node.
static byte  SV_update_player_counts( void )
{
    int pn, nn;

    num_join_waiting_players = 0;
    num_wait_game_start = 0;
    num_player_used = 0;   
    num_game_players = 0;
    num_player_slots = 1;

    for(nn=0; nn<MAXNETNODES; nn++)
    {
        num_join_waiting_players += join_waiting[nn];
    }

    for(pn=0; pn<MAXPLAYERS; pn++)
    {
        // count remaining players
        if( playeringame[pn] )
        {
            num_player_slots = pn+1;
            num_game_players++;
        }
        if( player_state[pn] )
        {
            num_player_used++;
            if( player_state[pn] == PS_join_wait_game_start )
                num_wait_game_start++;
        }
    }
   
    return num_player_used;
}


// Broadcast the XD_ADDPLAYER
//  nnode : new player is at this node
//  pn : new player num
//  flags : any flags that may be needed (future)
static
void  SV_Send_AddPlayer( byte nnode, byte pn, byte flags )
{
    byte buf[6];
   
    player_state[pn] = PS_added_commit;

    // Format: XD_ADDPLAYER   (ver 1.48)
    //  byte:  nnode
    //  byte:  player num
    //  byte:  pind
    //  byte:  flags
    buf[0] = nnode;
    buf[1] = pn;
    buf[2] = player_pind[pn];
    buf[3] = flags;

    // Message from server to everyone, to add the player.
    Send_NetXCmd(XD_ADDPLAYER, buf, 4);

    DEBFILE(va("Server added player %d net node %d\n", pn, nnode));

    if( nnode_state[nnode] < NOS_client )
        nnode_state[nnode] = NOS_active;
}


// Xcmd XD_ADDPLAYER
// Sent by server to all client.
static
void Got_NetXCmd_AddPlayer(xcmd_t * xc)
{
    static uint32_t sendconfigtic = 0xffffffff;
    byte nnode, newplayernum, pind, flags;

    // Format: XD_ADDPLAYER
    // Older, from demo
    //  byte:  nnode
    //  byte:  (player num) | (splitscreen pind at 0x80)
    // ver 1.48
    //  byte:  nnode
    //  byte:  player num
    //  byte:  pind
    //  byte:  flags

    // [WDJ] Having error due to sign extension of byte read (signed char).
    nnode = READBYTE(xc->curpos);  // unsigned
    newplayernum = READBYTE(xc->curpos);  // unsigned
    // NetXCmd are issued by demos.
    if( EV_legacy >= 148 )
    {
        pind = READBYTE(xc->curpos);
        flags = READBYTE(xc->curpos);
        if( flags )
            GenPrintf(EMSG_warn, "AddPlayer %d unsupported flags=%x\n", (newplayernum+1), flags);
    }
    else
    {
        // Old format
        pind = (newplayernum & 0x80)? 1:0;
        newplayernum &= 0x1F;
    }

    // Make play engine player data.
    // Do not set playeringame until player is created.
    G_AddPlayer(newplayernum);
    playeringame[newplayernum]=true;  // enable this player
    player_state[newplayernum]= PS_player;
    if( num_player_slots < newplayernum+1 )
        num_player_slots = newplayernum+1;
    num_game_players++;

    // [WDJ] Players are 1..MAXPLAYERS to the user.
    GenPrintf(EMSG_hud, "Player %d is in the game (node %d)\n", (newplayernum+1), nnode);

    if(nnode==cl_nnode)
    {
        // The server is creating my player.
        player_to_nnode[newplayernum]=0;  // for information only
        if( pind == 0 )
        {
            // mainplayer
            consoleplayer=newplayernum;
            displayplayer=newplayernum;
            displayplayer_ptr = consoleplayer_ptr = &players[newplayernum];
        }
        else
        {
            // splitscreen
            displayplayer2=newplayernum;
            displayplayer2_ptr = &players[displayplayer2];
        }
        DEBFILE(va("Spawning player[%i] pind=%i at this node.\n", newplayernum, pind));
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
static
void Got_NetXCmd_AddBot(xcmd_t * xc)  //added by AC for acbot
{
    bot_info_t bi;

    // [WDJ] Having error due to sign extension of byte read (signed char).
    byte newplayernum = READBYTE(xc->curpos);  // unsigned
    bi.name_index = READBYTE(xc->curpos);
    bi.colour = READBYTE(xc->curpos);
    bi.skinrand = LE_SWAP16( READU16(xc->curpos) );

    newplayernum&=0x7F;  // remove flag bit, and any sign extension

    player_t * pl = & players[newplayernum];
    char * botname = botnames[bi.name_index];

    if( playeringame[newplayernum] )
    {
        GenPrintf(EMSG_warn, "Bot %s: player slot %i already in use.\n", botname, newplayernum );
        return;
    }

    G_AddPlayer(newplayernum);

    B_Create_Bot( pl );

    strcpy(player_names[newplayernum], botname);
    pl->skincolor = bi.colour;
    if( cv_bot_skin.EV && (numskins > 1))
    {
        SetPlayerSkin_by_index( pl, (bi.skinrand % (numskins-1)) + 1 );
    }

    playeringame[newplayernum]=true;  // enable this player
    player_state[newplayernum]= PS_bot;
    if( num_player_slots < newplayernum+1 )
        num_player_slots = newplayernum+1;
    num_game_players++;

    multiplayer=1;

    GenPrintf(EMSG_hud, "Bot %s has entered the game\n", botname);
}

// By Server.
// Called by SV_SpawnServer, client_join_handler.
// Return true when a new player is added.
static
boolean SV_Add_Join_Waiting(void)
{
    boolean  newplayer_added = false;  // return
    byte nnode, ns;
    byte newplayernum;

    if( num_join_waiting_players == 0 )  return 0;

    // The code below usually clears the join_waiting queues.
    num_join_waiting_players = 0;
   
    for(nnode=0; nnode<MAXNETNODES; nnode++)
    {
        ns = nnode_state[nnode];
        if( (ns >= NOS_join_file) && (ns <= NOS_join_savegame) )
        {
            // Join download in progress
            num_join_waiting_players += join_waiting[nnode];
            continue;
        }
 
        // splitscreen can allow 2 player in one node
        for(; join_waiting[nnode]>0; join_waiting[nnode]--)
        {
            // Search for a free playernum.
            // New players will have playeringame set as a result of XCmd AddPlayer.
            newplayernum = SV_commit_player( nnode, PS_added_commit );
            if( newplayernum >= MAXPLAYERS )  continue;

            // Message from server to everyone, to add the player.
            SV_Send_AddPlayer( nnode, newplayernum, 0 );  // PS_added_commit
            // XD_ADDPLAYER commands will stay in the cmd buffers.
            // When cl_mode==CLM_connected is achieved, they will be transmitted and acted upon.

            newplayer_added = true;
        }
    }

    SV_update_player_counts();

    return newplayer_added;
}

#define GAME_START_WAIT    22

// Add players waiting for game start
void SV_Add_waiting_players( void )
{
    byte pn, nnode, cnt = 0;

    for( pn=0; pn<MAXPLAYERS; pn++)
    {
        if( player_state[pn] == PS_join_wait_game_start )
        {
            nnode = player_to_nnode[pn];
            if( nnode_state[nnode] == NOS_wait_game_start )
            {
                nnode_state[nnode] = NOS_active;
                nextsend_tic[nnode] = gametic;
            }

            SV_Send_control( nnode, CTRL_game_start, pn, TICRATE*GAME_START_WAIT );
            SV_Send_AddPlayer( nnode, pn, 0 );  // PS_added_commit
            cnt++;
        }
    }

    if( cnt == 0 )  return;

    // Update gametic and random state.
    SV_Send_State( paused | network_wait_pause );

    // Invoke the next level wait timer.
    wait_game_start_timer = TICRATE*GAME_START_WAIT;
}


void CL_Splitscreen_Player_Manager( void )
{
    if( cl_mode != CLM_connected )  return;

    if( cv_splitscreen.EV )
    {
        if( ! displayplayer2_ptr )
            CL_Send_Join();  // join game
        return;
    }

    // Remove splitscreen player
    if( displayplayer2_ptr )
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

    quit_netgame_status = 0;

    if( serverrunning == false )
    {
        GenPrintf(EMSG_hud, "Starting Server ...\n");
        serverrunning = true;
        SV_ResetServer();
        network_state = NETS_internal;  // Self server network.
        if( netgame )
        {
            I_NetOpenSocket();
            network_state = NETS_open;
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

    if( num_join_waiting_players )
        return SV_Add_Join_Waiting();

    return 0;
}

// Called by D_Init_ClientServer, G_StopDemo, CL_Reset, SV_StartSinglePlayerServer,
// D_WaitPlayer_Response.
void SV_StopServer( void )
{
    int i;

    gamestate = wipegamestate = GS_NULL;

    localtextcmd[0].len = 0;  // text len
    localtextcmd[1].len = 0; // text len

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
    cl_server_state = NOS_active; // no quit game message

    // no more tic the game with this settings !
    SV_StopServer();

    if( cv_splitscreen.value )
        multiplayer    = true;
}

// By Server.
// Called by client_join_handler.
static void SV_Send_Refuse(int to_node, char *reason)
{
    strncpy(netbuffer->u.stringpak.str, reason, MAX_STRINGPAK_LEN-1);
    netbuffer->u.stringpak.str[MAX_STRINGPAK_LEN-1] = 0;

    netbuffer->packettype = PT_SERVERREFUSE;
    HSendPacket( to_node, SP_reliable|SP_queue, 0, strlen(netbuffer->u.stringpak.str)+1 );  // ignore failure
    Net_CloseConnection(to_node, 0);
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
                           LE_SWAP32(netbuffer->u.askinfo.send_time));
        Net_CloseConnection(nnode, 0);  // a temp connection
    }
}

// By Server.
// PT_CLIENTJOIN from future client.
//   nnode: net node that is joining
static void client_join_handler( byte nnode )
{
    byte num_to_join, command;
    byte join_flags = netbuffer->u.clientcfg.flags;
    boolean newnode=false;

    if( netbuffer->u.clientcfg.version != VERSION
        || LE_SWAP32(netbuffer->u.clientcfg.subversion) != NETWORK_VERSION)
    {
        SV_Send_Refuse(nnode,
           // Text is automatically centered, must not be too long.
           va("Incompatible client\nServer %i Net %i\nLegacy %i Net %i",
               VERSION, NETWORK_VERSION, netbuffer->u.clientcfg.version, LE_SWAP32(netbuffer->u.clientcfg.subversion) )
        );
        return;
    }

    // nnode==0 is self, which is always accepted.
    if(!cv_allownewplayer.value && nnode!=0 )
    {
        SV_Send_Refuse(nnode,
          "The server is not accepting people for the moment");
        return;
    }

    num_to_join = netbuffer->u.clientcfg.num_node_players - playerpernode[nnode];
    // DRONE will have 0 players
   
    // Compute it using join and player counts.
    if( (SV_update_player_counts() + num_to_join) > cv_maxplayers.value )
    {
        SV_Send_Refuse(nnode,
           va("Maximum players reached (max:%d)", cv_maxplayers.value));
        return;
    }

    // To determine the intial command to send with the config.
    command = CTRL_normal;
    // If there is an existing player at the node, then will not be newnode.
    newnode = ( nnode_state[nnode] < NOS_join );
    if( newnode && (gamestate == GS_LEVEL) )
    {
#ifdef JOININGAME
        // If cannot download, then wait for next game start.
        if(( join_flags & NF_download_savegame )  // joiner allows savegame download
          && ( cv_SV_download_savegame.EV )    )  // server allows savegame download
            command = CTRL_download_savegame;
        else
            command = CTRL_wait_game_start;

        // For savegame downloads to work, the endian must match.
#ifdef __BIG_ENDIAN__
        // Server is big-endian
        if( (join_flags & NF_big_endian) == 0 )  // joiner is little-endian
#else
        // Server is little-endian
        if( join_flags & NF_big_endian )  // joiner is big-endian
#endif
            command = CTRL_wait_game_start;

#else
        // No join-in-game, no savegame download
        command = CTRL_wait_game_start;
#endif
    }

    // Client authorized to join.
    if( newnode )
    {
        // The nnode is new to this server.
        SV_AddNode(nnode, NOS_join);

        // Send inital command with the config to avoid sending a separate
        // command message, which could arrive out-of-order.
        if(! SV_Send_ServerConfig(nnode, command) )
            goto kill_node;

        DEBFILE("New node joined\n");
    }
   
    if( join_flags & NF_drone )
    {
        // drone only watches another player
        playerpernode[nnode] = 0;  // indicator that is DRONE node
        return;
    }

    if( num_to_join )
    {
        if( command == CTRL_wait_game_start )
            goto wait_for_game_start;

#ifdef JOININGAME
        if( command == CTRL_download_savegame )
        {
            // New node with new players joining existing game.
            // Update the nnode with game in progress.
            nnode_state[nnode] = NOS_join_savegame;
            SV_Send_SaveGame(nnode); // send game data
            // netwait timer is running
            GenPrintf(EMSG_info, "Send savegame\n");
            // Client will return  PT_CLIENTREADY or PT_CLIENTQUIT
        }
#endif

        // Add node players to join_waiting
        join_waiting[nnode] = num_to_join;
        num_join_waiting_players += num_to_join;
        SV_Add_Join_Waiting();
    }
    return;
 
wait_for_game_start:
    // These players will wait until the next game start.
    while( num_to_join-- )
    {
        byte pn = SV_commit_player( nnode, PS_join_wait_game_start );
        SV_Send_control( nnode, CTRL_wait_game_start, pn, 0 );
        DEBFILE(va("Client Join: node=%i, wait game start player=%i.\n", nnode, pn));
    }

    if( nnode_state[nnode] < NOS_client )
        nnode_state[nnode] = NOS_wait_game_start;  // release network_wait
    return;
    
kill_node:
    DEBFILE("Client Join: Failure to Send.\n");
    GenPrintf(EMSG_error, "Client Join: Failure to Send\n" );
    SV_Reset_NetNode(nnode);
    return;
}


// BY Server.
// PT_NODE_TIMEOUT, PT_CLIENTQUIT
//   nnode : the network client node quitting
//   client_pn : the client player num that sent the quit
static void client_quit_handler( byte nnode, byte client_pn )
{
    // Set nnode_state to NOS_shutdown at the end of the kick command.
    // This allows the sending of some packets to the quiting client
    // and to have them ack back.
    join_waiting[nnode]= 0;
    if( (client_pn < MAXPLAYERS) && playeringame[client_pn])
    {
        byte reason = (netbuffer->packettype == PT_NODE_TIMEOUT) ?
           KICK_MSG_TIMEOUT : KICK_MSG_PLAYER_QUIT;
        // Update other players by kicking nnode.
        Send_NetXCmd_p2(XD_KICK, client_pn, reason);  // kick player
        nnode_to_player[0][nnode] = 255;

        byte pn2 = nnode_to_player[1][nnode];  // splitscreen player at the nnode
        if( pn2 < MAXPLAYERS )
        {
            if( playeringame[pn2] )
            {
               // kick player2
               Send_NetXCmd_p2(XD_KICK, pn2, reason);
            }
            nnode_to_player[1][nnode] = 255;
        }
    }
    Net_CloseConnection(nnode, 0);
    nnode_state[nnode] = NOS_shutdown;
}



// By Client
// PT_SERVERINFO from Server.
//  nnode : remote node
static void server_info_handler( byte nnode )
{
    // Compute ping in ms.
    netbuffer->u.serverinfo.trip_time =
     (I_GetTime() - LE_SWAP32(netbuffer->u.serverinfo.trip_time))*1000/TICRATE; 
    netbuffer->u.serverinfo.servername[MAXSERVERNAME-1]=0;

    SL_InsertServer( &netbuffer->u.serverinfo, nnode);
}


// By Client
// PT_SERVERREFUSE from Server.
static void server_refuse_handler( byte nnode )
{
    if( cl_mode == CLM_wait_join_response )
    {
        M_SimpleMessage(va("Server %i refuses connection\n\nReason :\n%s",
                           nnode,
                           netbuffer->u.stringpak.str));
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
    xcmd_t xc;

    if( cl_mode != CLM_wait_join_response )
        return;

    if(!server)
    {
        // Clients not on the server, update to server time.
        maketic = gametic = cl_need_tic = LE_SWAP32(netbuffer->u.servercfg.gametic);
    }

    // Client keeps server state, even on the server.
    if( cl_servernode < MAXNETNODES )
        cl_server_state = NOS_server;  // connected

#ifdef CLIENTPREDICTION2
    localgametic = gametic;
#endif

    // Handle a player on the server.
    serverplayer = netbuffer->u.servercfg.serverplayer;
    if (serverplayer < MAXPLAYERS)  // 255 = no player
        player_to_nnode[serverplayer] = cl_servernode;

    num_player_slots = netbuffer->u.servercfg.num_player_slots;
    cl_nnode = netbuffer->u.servercfg.clientnode;  // assigned by server

    GenPrintf(EMSG_hud, "Join accepted, wait next map change ...\n");
    DEBFILE(va("Server accept join gametic=%d, client net node=%d\n",
               gametic, cl_nnode));

    // No need for the server to update itself from message from server.
    if( ! server )
    {
        // Client
        uint32_t  playerdet = LE_SWAP32(netbuffer->u.servercfg.playerdetected);
        for(j=0;j<MAXPLAYERS;j++)
        {
            playeringame[j] = (( playerdet & (1<<j) ) != 0);
            player_state[j] = (playeringame[j])? PS_player_from_server : 0;
        }

        xc.playernum = 0;
        xc.curpos = netbuffer->u.servercfg.netvar_buf;
        xc.endpos = xc.curpos + NETVAR_BUFF_LEN - 1;
        CV_LoadNetVars( &xc );
    }

    // Initial command, too avoid a separate command message.
    switch( netbuffer->u.servercfg.command )
    {
     case CTRL_normal:
        cl_mode = CLM_connected;
        break;
#ifdef JOININGAME
     case CTRL_download_savegame:
        if(netbuffer->u.servercfg.gamestate == GS_LEVEL)
            GenPrintf(EMSG_hud, "Server Config: Download savegame when NOT GS_LEVEL\n");
        cl_mode = CLM_download_savegame;
        break;
#endif
     case CTRL_wait_game_start:
        cl_mode = CLM_wait_game_start;
        break;
    }
}

// By Client
// PT_SERVERSHUTDOWN from Server.
static void server_shutdown_handler()
{
    network_state = NETS_no_server;
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
    network_state = NETS_no_server;
    if( cl_mode != CLM_searching )
    {
        M_SimpleMessage("Server Timeout\n\nPress Esc");
        CL_Reset();
        D_StartTitle();
    }
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



// --- Text command
// Client NetXCmd: XD_NAMEANDCOLOR, XD_WEAPONPREF, XD_USEARTIFACT, XD_SAY
// Server NetXCmd: XD_PAUSE, XD_KICK, XD_ADDPLAYER, XD_ADDBOT,
//    XD_MAP, XD_EXITLEVEL, XD_LOADGAME, XD_SAVEGAME

// By Server, Client.
// Send accumulated client textcmd to server.  Also server textcmd.
// Called by Send_localtextcmd
static void Send_localtextcmd_pind( byte pind )
{
    static byte PT_TEXTCMD_pind[2] = {PT_TEXTCMD, PT_TEXTCMD2};

    textbuf_t * ltcp = &localtextcmd[pind];
    int tc_len = ltcp->len+1;  // text len + cmd
    netbuffer->packettype = PT_TEXTCMD_pind[pind];
    memcpy(&netbuffer->u.textcmdpak, ltcp, tc_len);
    // all extra data have been sent
    byte errcode = HSendPacket( cl_servernode, SP_reliable|SP_queue, 0, tc_len ); // send can fail for some reasons...
    if( errcode >= NE_fail )
    {
        generic_network_error_handler( errcode, "LocalTextCmd msg" );  // should not happen
        return;
    }
    ltcp->len = 0;  // text len
}

// By Server, Client
// Called by NetUpdate
static void Send_localtextcmd( void )
{
    // [WDJ] Server also needs to send NetXCmd, even without serverplayer.
    // How this used to work is a mystery.
    if( server || ((cl_mode == CLM_connected) && (network_state >= NETS_open)) )
    {
        // send extra data if needed
        if( localtextcmd[0].len ) // text len
        {
            Send_localtextcmd_pind(0);
        }
        
        // send extra data if needed for player 2 (splitscreen)
        if( localtextcmd[1].len ) // text len
        {
            Send_localtextcmd_pind(1);
        }
    }
    else
    {
        // Clear NetXCmd that would overflow the buffers.
        localtextcmd[0].len = 0;
        localtextcmd[1].len = 0;
    }
}

// By Server
// used at txtcmds received to check packetsize bound
// Called by: SV_Send_Tics, net_textcmd_handler.
static int TotalTextCmdPerTic( int tic )
{
    int i,total=1; // num of textcmds in the tic (ntextcmd byte)

    int btic = BTIC_INDEX( tic );

    for(i=0;i<MAXPLAYERS;i++)
    {
        if( (i==0) || playeringame[i] )
        {
            int textlen = textcmds[btic][i].len;
            if( textlen )
               total += textlen + 2; // "+2" for size and playernum
        }
    }

    return total;
}

// By Server
// PT_TEXTCMD, PT_TEXTCMD2
//   nnode : the network client
//   client_pn : playernum of client, always valid
// Called by Net_Packet_Handler
static void net_textcmd_handler( byte nnode, byte client_pn )
{
    int nbtc_len = netbuffer->u.textcmdpak.len;  // incoming length
    int tc_limit;  // Max size of existing textcmd that can be included with this textcmd.
    int textlen, btic;
    tic_t tic;
    textbuf_t *  textbuf;

    // Handle NetXCmd that come from clients, and some from the server.
    // Server textcmd are sent with client_pn=0, and they must always go through.
    // The server places them into tick messages here, which are sent to all clients,
    // so even if placement is arbitrary here, all clients will execute them uniformly.

    // Move textcmd from netbuffer to a textbuf.

    // Check if tic that we are making isn't too large,
    // else we cannot send it :(
    // Note: num_player_slots+1 is "+1" because numplayers
    // can change within this time and sent time.
    tc_limit = software_MAXPACKETLENGTH
         - ( nbtc_len + 2 + SERVER_TIC_BASE_SIZE
            + ((num_player_slots+1) * sizeof(ticcmd_t)) );

    // Search for a tic that has enough space in the ticcmd.
    tic = maketic;
    while( TotalTextCmdPerTic(tic) > tc_limit )
    {
        textbuf = & textcmds[ BTIC_INDEX( tic ) ][client_pn];
        if( (nbtc_len + textbuf->len) < MAXTEXTCMD )
          break; // found one
        tic++;
        if( tic >= (next_tic_send+BACKUPTICS) )  goto drop_packet;
    }

    btic = BTIC_INDEX( tic );
    textbuf = & textcmds[btic][client_pn];
    textlen = textbuf->len;
    DEBFILE(va("Textcmd: btic %d text[%d] player %d nxttic %d maketic %d\n",
               btic, textlen, client_pn, next_tic_send, maketic));
    // Append to the selected buffer.
    memcpy(&textbuf->text[ textlen ],
           &netbuffer->u.textcmdpak.text[0],  // the text
           nbtc_len);
    textbuf->len += nbtc_len;  // text len
    return;
   
drop_packet:
    // Drop the packet, let the node resend it.
    DEBFILE(va("Textcmd too long: max %d used %d maketic %d nxttic %d node %d player %d\n",
               tc_limit, TotalTextCmdPerTic(maketic), maketic, next_tic_send,
               nnode, client_pn));
    Net_Cancel_Packet_Ack(nnode);
    return;
}


// Detected a consistency fault.
//  nnode : the client node
//  client_pn : the client player num
//  fault_tic : tick with consistency fault
//  btic : BTIC_INDEX for this network message
static void SV_consistency_fault( byte nnode, byte client_pn, tic_t fault_tic, int btic )
{
    byte confault = ++consistency_faults[nnode];  // failure count
    uint16_t sv_con = consistency[btic];
    uint16_t cl_con = LE_SWAP16(netbuffer->u.clientpak.consistency);

    // No consistency fault during intermission, because of joining players who don't have position yet.
    if( gamestate == GS_INTERMISSION )
        return;

    if( confault >= consistency_limit_fatal[cv_SV_netrepair.EV] )
    {
        // Failed the consistency check too many times
#if 1
        Send_NetXCmd_p2(XD_KICK, client_pn, KICK_MSG_CON_FAIL);
#else
        // Debug message instead.
//        GenPrintf(EMSG_warn, "Kick player %d at tic %d, consistency failure\n",
//            client_pn, start_tic);
#endif
        GenPrintf(EMSG_warn, "Kick player %d at tic %d, consistency failure ( server=%X client=%X )\n",
            client_pn, fault_tic, sv_con, cl_con );
        DEBFILE(va("Kick player %d at tic %d, consistency failure ( server=%i client=%i )\n",
            client_pn, fault_tic, sv_con, cl_con ));

    }
#ifdef JOININGAME
    else if( ( (consistency_sg_bit[cv_SV_netrepair.EV] >> (confault-1)) & 0x01)
             && ( cv_SV_download_savegame.EV ))
    {
        // try to use savegame to fix consistency
        SV_Send_repair(RQ_SUG_SAVEGAME, nnode);
    }
#endif
    else
    {
        // try to fix consistency
        SV_Send_repair(RQ_PLAYER, nnode);
    }
}


// By Server
// PT_CLIENTCMD, PT_CLIENT2CMD, PT_CLIENTMIS, PT_CLIENT2MIS,
// PT_NODEKEEPALIVE, PT_NODEKEEPALIVEMIS from Client.
//  client_pn: the player that sent the cmd
static void client_cmd_handler( byte netcmd, byte nnode, byte client_pn )
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
    if( playerpernode[nnode] == 0 )   // DRONE node indicator
       return;

    if(client_pn >= MAXPLAYERS) // invalid player num, or old message with DRONE bit
       return;

    if( netcmd==PT_NODEKEEPALIVE || netcmd==PT_NODEKEEPALIVEMIS )
       return;

    // Check consistency
    if((start_tic <= gametic)
       && (start_tic > (gametic - BACKUPTICS + 1)) )
    {
        // within previous tics
        btic = BTIC_INDEX(start_tic);
        if(consistency[btic] != LE_SWAP16_FAST(netbuffer->u.clientpak.consistency))
        {
            // Failed the consistency check.
            SV_consistency_fault( nnode, client_pn, start_tic, btic );
            return;  // packet contents lost when other messages sent
        }
        else if( consistency_faults[nnode] > 0 )
        {
            consistency_faults[nnode] -- ;
        }
    }

    // Copy the ticcmd
    btic = BTIC_INDEX( maketic );
    TicCmdCopy(&netcmds[btic][client_pn],
               &netbuffer->u.clientpak.cmd, 1);

    // PT_CLIENT2CMD has cmd for both players
    if( netcmd == PT_CLIENT2CMD )
    {
        // From player2
        byte client_pn2 = nnode_to_player[1][nnode];  // splitscreen player
        if( client_pn2 < MAXPLAYERS)
        {
            // Copy the ticcmd for player2.
            TicCmdCopy(&netcmds[btic][client_pn2], 
                   &netbuffer->u.client2pak.cmd2, 1);
        }
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
    byte * endbuffer;  // for buffer overrun tests
    byte   num_txt;
    int    btic, j, k;

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
    endbuffer = (byte*)&ticp[NUM_SERVERTIC_CMD];  // after last content

    // After the nettics are the net textcmds
    k = netbuffer->u.serverpak.numplayerslots * netbuffer->u.serverpak.numtics;
    if( k >= NUM_SERVERTIC_CMD )  goto exceed_buffer;
    txtp = (byte *)&netbuffer->u.serverpak.cmds[k];
    // txtp uses cmd space for text

    for(ti = start_tic; ti<end_tic; ti++)
    {
        // Clear first
        D_Clear_ticcmd(ti);

        // Copy the tics
        btic = BTIC_INDEX( ti );
        // btic limited to BACKUPTICS-1
        TicCmdCopy(netcmds[btic], ticp, netbuffer->u.serverpak.numplayerslots);
        ticp += netbuffer->u.serverpak.numplayerslots;

        // Copy the incoming textcmds.
        num_txt = *(txtp++);  // num_textcmd field, number of txtcmd
        for(j=0; j<num_txt; j++)
        {
            // Format:
            //  byte: playernum
            //  textbuf_t: textcmd
            int pn = *(txtp++); // playernum
            textbuf_t * tc = & textcmds[btic][pn];
            int tc_len = txtp[0]+1;  // max len of 256
              // length of whole textbuf_t 
            if( txtp + tc_len > endbuffer )  goto exceed_buffer;
#if MAXTEXTCMD < 255
            if( tc_len > MAXTEXTCMD+1 )  goto exceed_buffer;  // prevent dest overrun
#endif
            memcpy( tc, txtp, tc_len);
            // force string termination to defend against malicious packets
            tc->text[tc->len] = 0;
            tc->text[MAXTEXTCMD] = 0;
            txtp += tc_len;
        }
    }

    cl_need_tic = end_tic;
    return;

   
 exceed_buffer:
    I_SoftError("Nettics textcmd exceed buffer\n");
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
    byte client_pn;
    byte nnode, nodestate, packettype;

    while ( HGetPacket() )
    {
        nnode = doomcom->remotenode;  // 0..MAXNETNODES-1, BROADCAST_NODE
        nodestate = (nnode < MAXNETNODES)?  nnode_state[nnode] : NOS_invalid;
        packettype = netbuffer->packettype;

        // [WDJ] Run-time messages are given priority in speed of handling.

        // ---- SERVER handling packets of known clients nodes.
        if( server && (nodestate >= NOS_recognized))
        {
            // [WDJ] pn no longer has DRONE bit.
            client_pn = nnode_to_player[0][nnode];  // the player
            // Every message type must handle the invalid client_pn individually.

            // Messages handled by server, for known client nodes.
            switch(packettype)
            {
             case PT_CLIENTCMD  :
             case PT_CLIENT2CMD :
             case PT_CLIENTMIS  :
             case PT_CLIENT2MIS :
             case PT_NODEKEEPALIVE :
             case PT_NODEKEEPALIVEMIS :
                // updates nettics for invalid client_pn
                client_cmd_handler( packettype, nnode, client_pn );
                continue;
             case PT_TEXTCMD2 : // splitscreen special
                client_pn = nnode_to_player[1][nnode];
                // fall through
             case PT_TEXTCMD :
                if( client_pn >= MAXPLAYERS )  // unused = 255
                {
                    // Do not ACK the packet from a strange client_pn.
                    Net_Cancel_Packet_Ack(nnode);
                    continue;
                }
                net_textcmd_handler( nnode, client_pn );  // always valid client_pn
                continue;
             case PT_NODE_TIMEOUT:
             case PT_CLIENTQUIT:
                // still closes the connection when invalid client_pn
                client_quit_handler( nnode, client_pn );
                continue;
             default:
                break;
            }
        }
           
        // ---- CLIENT Handling Server packets.
        if( (nnode == cl_servernode) && (cl_mode >= CLM_server_files) )  // from the known server
        {
            // Messages from the server, for known server.
            switch(packettype)
            {
             case PT_SERVERTICS :
                servertic_handler( nnode );
                continue;
             case PT_FILEFRAGMENT :
                if( !server )
                    Got_Filetxpak();
                continue;
             case PT_NETWAIT:
                if( !server )
                    netwait_handler();
                continue;
             case PT_REPAIR:
                repair_handler_client( nnode );  // from server
                continue;
             default:
                break;
            } // end switch
        }

        // Infrequent messages from server.
        if( nnode == cl_servernode )
        {
            // Any Client that knows the server.
            // These messages are recognized, but application is limited.
            switch(packettype)
            {
             case PT_STATE:
                if( !server )
                    state_handler();  // to client
                continue;
             case PT_CONTROL:
                control_msg_handler();
                continue;
             case PT_SERVERSHUTDOWN:
                if( ! server )
                    server_shutdown_handler();  // only client not on server
                continue;
             case PT_NODE_TIMEOUT:  // from server
                // must be before PT_NODE_TIMEOUT from client.
                if( ! server )
                    server_timeout_handler();  // only client not on server
                continue;
            }
        }
       
        // Infrequent messages, from anybody.
        if( server )
        {
            // Packet can be from client trying to join server.
            if( (nnode != cl_servernode) && (nodestate < NOS_recognized) )
            {
                // Client trying to Join.
                DEBFILE(va("Received packet from unknown host %d\n",nnode));
            }

            // Can only be handled by a server.
            switch(packettype)
            {
             case PT_TEXTCMD :
                // Server to server use of client channel for server NetXCmd, with no players.
                // No valid player num for this nnode.
                net_textcmd_handler( nnode, 0 );
                continue;
             case PT_ASKINFO:  // client has asked server for info
                // May have been sent to BROADCAST_NODE, but nnode is sender.
                server_askinfo_handler( nnode );
                continue;
             case PT_REQUESTFILE :
                if( cv_SV_download_files.EV == 0 )
                {
                    GenPrintf(EMSG_ver, "RequestFile: Blocked, Not Allowed\n" );
                    continue;
                }
                Got_RequestFilePak(nnode);
                continue;
             case PT_CLIENTJOIN:
                client_join_handler( nnode );
                continue;
             case PT_CLIENTREADY:
                ready_handler( nnode );
                continue;
             case PT_REPAIR:
                if( nodestate >= NOS_recognized )
                    repair_handler_server( nnode );  // from client
                continue;
             case PT_NODE_TIMEOUT:  // from unknown client
             case PT_CLIENTQUIT:  // when unknown client
                Net_CloseConnection(nnode, 0);  // normal closing
                continue;
            }
        }

        // Prospective Client, Client handling of server messages.
        if( cl_mode != CLM_connected )  // Protect against rogue interference.
        {
            // Messages to client before connected. They change client state.
            switch(packettype)
            {
             case PT_SERVERINFO:
                // response from server to BROADCAST_NODE message, should have nnode same as new cl_servernode
                server_info_handler( nnode );
                continue;
             case PT_SERVERREFUSE : // negative response of client join request
                server_refuse_handler( nnode );
                continue;
             case PT_SERVERCFG :    // positive response of client join request
                server_cfg_handler( nnode );
                continue;
             case PT_FILEFRAGMENT :
                // handled in d_netfil.c
                if( !server )
                    Got_Filetxpak();
                continue;
             case PT_SERVERTICS:
                // do not remove my own server
                // (we have just to get a out of order packet)
                if( nnode == cl_servernode )  continue;
                break;  // kill it
            }
        }

        switch(packettype)
        {
         case PT_ASKINFO:  // broadcast, non-servers should ignore it
         case PT_SERVERCFG :  // server cfg at wrong time
         case PT_NODE_TIMEOUT:  // errant
            continue;  // ignore
         case PT_REQUESTFILE :
            if( ! server )  goto server_only;
            continue;
        }

        // Packet not accepted.
        if((nnode >= MAXNETNODES) || (nodestate < NOS_recognized))
        {
            DEBFILE(va("Unknown packet received (%d) from unknown host !\n", packettype));
            goto close_node;
        }

        DEBFILE(va("Unknown packet type: type %d node %d\n", packettype, nnode));
        continue;
       
    server_only:
        GenPrintf(EMSG_warn, "Recv warn: unknown node=%i, Client ignores server only packet type (%d).\n", nnode, packettype );
        DEBFILE(va("Recv warn: unknown node=%i, Client ignores server only packet type (%d).\n", nnode, packettype));
        goto close_node;

    close_node:
        Net_CloseConnection(nnode, 0);  // a temp connection
        continue;       
    } // end while

    if( server && network_wait_pause )
        SV_network_wait_handler();
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
    DEBFILE(va("pos = %d, rnd %d\n", ret, P_Rand_GetIndex()));
    ret+=P_Rand_GetIndex();

    return ret;
}



// By Client.
// Send the client packet to the server
// Called by NetUpdate, 
static void CL_Send_ClientCmd (void)
{
    // index by  [mis]
    static byte  PT_CLIENTCMD_options[2] = {PT_CLIENTCMD, PT_CLIENTMIS};
    static byte  PT_CLIENT2CMD_options[2] = {PT_CLIENT2CMD, PT_CLIENT2MIS};
    // index by  [mis]
    static byte  PT_NODEKEEPALIVE_options[2] = {PT_NODEKEEPALIVE, PT_NODEKEEPALIVEMIS};

/* oops can do that until i have implemented a real dead reckoning
    static ticcmd_t lastcmdssent;
    static int      lastsenttime=-TICRATE;

    if( memcmp(&localcmds[0],&lastcmdssent,sizeof(ticcmd_t))!=0 || lastsenttime+TICRATE/3<I_GetTime())
    {
        lastsenttime=I_GetTime();
*/

    int packetsize=0;

    byte  cmd_options = 0;  // easier to understand and maintain
    if (cl_packetmissed)
        cmd_options = 1;  // MIS bit
   
    netbuffer->packettype = PT_CLIENTCMD_options[cmd_options];
    netbuffer->u.clientpak.resendfrom = cl_need_tic;
    netbuffer->u.clientpak.client_tic = gametic;

    if( gamestate == GS_WAITINGPLAYERS )
    {
        // Server is waiting for network players before starting the game.
        // send NODEKEEPALIVE, or NODEKEEPALIVEMIS packet
        netbuffer->packettype = PT_NODEKEEPALIVE_options[cmd_options];
//        packetsize = sizeof(clientcmd_pak_t)-sizeof(ticcmd_t)-sizeof(int16_t);
        packetsize = offsetof(clientcmd_pak_t, consistency);
        HSendPacket( cl_servernode, 0, 0, packetsize );  // msg lost when too busy
    }
    else
    if( gamestate != GS_NULL )
    {
        int btic = BTIC_INDEX( gametic );
        TicCmdCopy(&netbuffer->u.clientpak.cmd, &localcmds[0], 1);
        netbuffer->u.clientpak.consistency = LE_SWAP16_FAST(consistency[btic]);

        // send a special packet with 2 cmd for splitscreen
        if (cv_splitscreen.value)
        {
            // send PT_CLIENT2CMD, or PT_CLIENT2CMDMIS packet
            netbuffer->packettype = PT_CLIENT2CMD_options[cmd_options];
            TicCmdCopy(&netbuffer->u.client2pak.cmd2, &localcmds[1], 1);
            packetsize = sizeof(client2cmd_pak_t);
        }
        else
            packetsize = sizeof(clientcmd_pak_t);
        
        HSendPacket( cl_servernode, 0, 0, packetsize );  // msg lost when too busy
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
    // Assume sender is server. Assume cl_servernode=0.
    for(nnode=1; nnode<MAXNETNODES; nnode++)
    {
        if( nnode_state[nnode] < NOS_client )  continue;
        // Need this to send the NetXCmd Add Player

        // For each node create a packet with x tics and send it.
        // x is computed using nextsend_tic[n], max packet size and maketic.

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
            packsize += sizeof(ticcmd_t) * num_player_slots;
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
                                packsize, num_player_slots, nnode);
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
        netbuffer->u.serverpak.numplayerslots = num_player_slots;
        bufpos=(char *)&netbuffer->u.serverpak.cmds;
       
        // All the ticcmd_t, start_tic..(end_tic-1)
        for(ti=start_tic; ti<end_tic; ti++)
        {
            int btic = BTIC_INDEX( ti );
            TicCmdCopy((ticcmd_t*) bufpos, netcmds[btic], num_player_slots);
            bufpos += num_player_slots * sizeof(ticcmd_t);
        }

        // All the textcmd, start_tic..(end_tic-1)
        for(ti=start_tic; ti<end_tic; ti++)
        {
            btic = BTIC_INDEX( ti );
            // There must be a num_txtcmd field for every tic, even if 0.
            num_txt_p = bufpos++; // the num_textcmd field
            num_txt = 0;
            for(pn=0; pn<MAXPLAYERS; pn++)
            {
                // Format:
                //  byte: playernum
                //  textbuf_t: textcmd
                // Force text cmd channel [0] for server NetXCmd.		
                if((pn==0) || playeringame[pn])
                {
                    int textlen = textcmds[btic][pn].len;
                    if(textlen)
                    {
                        *(bufpos++) = pn;  // playernum
                        // Send the textbuf_t, length limited.
                        memcpy(bufpos, &textcmds[btic][pn], textlen+1);
                        bufpos += textlen+1;
                        num_txt++; // inc the number of textcmd sent
                    }
                }
            }
            // Update the num_txtcmd field, even if it is 0.
            *num_txt_p = num_txt; // the number of textcmd
        }

        packsize = bufpos - (char *)&(netbuffer->u);
        HSendPacket( nnode, 0, 0, packsize );  // msg lost when too busy

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
    G_BuildTiccmd(&localcmds[0], realtics, 0);
    // [WDJ] requires splitscreen and player2 present
    if (cv_splitscreen.value && displayplayer2_ptr )
      G_BuildTiccmd(&localcmds[1], realtics, 1);

#ifdef CLIENTPREDICTION2
    if( !paused && localgametic<gametic+BACKUPTICS)
    {
        P_MoveSpirit ( &players[consoleplayer], &localcmds[0], realtics );
        localgametic+=realtics;
    }
#endif
    localcmds[0].angleturn |= TICCMD_RECEIVED;
}

void SV_SpawnPlayer( byte playernum, int x, int y, angle_t angle )
{
    // for futur copytic use the good x,y and angle!
    if( server )
    {
        ticcmd_t * tc = &netcmds[ BTIC_INDEX(maketic) ][playernum];
#ifdef CLIENTPREDICTION2
        tc->x = x;
        tc->y = y;
#endif
        tc->angleturn=(angle>>16) | TICCMD_RECEIVED;
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
        if(playerpernode[nnode] == 0)  continue;  // not in use or DRONE
       
        // Detect missing ticcmd, using only player[0] at the node.
        player = nnode_to_player[0][nnode];
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
                player = nnode_to_player[1][nnode];
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
        COM_BufExecute( CFG_none );

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
        if( network_state < NETS_open )
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
    else if( gamestate == GS_INTERMISSION )
    {
        // [WDJ] Server runs timer, even when client is not-active.
        // G_Ticker does the GS_INTERMISSION effects.
        // Timer for start of next game level.
        if(( wait_game_start_timer > 0 ) && ! paused && (realtics > 0))
        {
            wait_game_start_timer--;
            if( wait_game_start_timer == 0 )
            {
                S_StartSound(sfx_slop);

                WI_Init_NoState();  // start transition to G_NextLevel
            }

            if( (wait_game_start_timer & 0x1F) == 0x03 )
            {
                // Update the wait timer, to keep everyone in sync
                SV_Send_control( BROADCAST_NODE, CTRL_wait_timer, 255, wait_game_start_timer );
            }
        }
    }

    // Server keeps forcing (cl_need_tic = maketic) in SV_Send_Tic_Update, kluge.
    if( cl_need_tic > gametic )
    {
        if (demo_ctrl == DEMO_seq_advance)  // and not disabled
        {
            D_DoAdvanceDemo ();
            return;
        }

        // Run the count * tics
        while (cl_need_tic > gametic)
        {
            DEBFILE(va("==== Run tic %u (local %d)\n",gametic, localgametic));

            G_Ticker ();
            ExtraDataTicker();  // execute NetXCmd
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
        if( (nnode_state[nn] >= NOS_active)
            && (nettics[nn] < next_tic_send) )
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
        D_Process_Events ();
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

    Send_localtextcmd();  // includes server NetXCmd cmds
   
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

        if( num_join_waiting_players )
            SV_Add_Join_Waiting();
    }

    Net_AckTicker();

    if( ! dedicated )
    {
        M_Ticker ();
        CON_Ticker();
    }
}
