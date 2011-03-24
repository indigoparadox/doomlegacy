// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2010 by DooM Legacy Team.
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
// $Log: g_game.c,v $
// Revision 1.50  2004/09/12 19:40:05  darkwolf95
// additional chex quest 1 support
//
// Revision 1.49  2004/07/27 08:19:35  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.48  2003/11/22 00:49:33  darkwolf95
// ooops, quick fix
//
// Revision 1.47  2003/11/22 00:22:09  darkwolf95
// get rid of FS hud pics on level exit and new game, also added exl's fix for clearing hub variables on new game
//
// Revision 1.46  2003/03/22 22:35:59  hurdler
// Fix CR+LF issue
//
// Revision 1.45  2002/09/27 16:40:08  tonyd
// First commit of acbot
//
// Revision 1.44  2002/08/24 22:42:02  hurdler
// Apply Robert Hogberg patches
//
// Revision 1.43  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.42  2001/12/15 18:41:35  hurdler
// small commit, mainly splitscreen fix
//
// Revision 1.41  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.40  2001/08/20 18:34:18  bpereira
// glide ligthing and map30 bug
//
// Revision 1.39  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.38  2001/08/02 19:15:59  bpereira
// fix player reset in secret level of doom2
//
// Revision 1.37  2001/07/16 22:35:40  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.36  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.35  2001/05/03 21:22:25  hurdler
// remove some warnings
//
// Revision 1.34  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.33  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.32  2001/03/03 06:17:33  bpereira
// no message
//
// Revision 1.31  2001/02/24 13:35:19  bpereira
// no message
//
// Revision 1.30  2001/02/10 12:27:13  bpereira
// no message
//
// Revision 1.29  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.28  2000/11/26 20:36:14  hurdler
// Adding autorun2
//
// Revision 1.27  2000/11/11 13:59:45  bpereira
// no message
//
// Revision 1.26  2000/11/06 20:52:15  bpereira
// no message
//
// Revision 1.25  2000/11/04 16:23:42  bpereira
// no message
//
// Revision 1.24  2000/11/02 19:49:35  bpereira
// no message
//
// Revision 1.23  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.22  2000/10/21 08:43:28  bpereira
// no message
//
// Revision 1.21  2000/10/09 14:03:31  crashrl
// *** empty log message ***
//
// Revision 1.20  2000/10/08 13:30:00  bpereira
// no message
//
// Revision 1.19  2000/10/07 20:36:13  crashrl
// Added deathmatch team-start-sectors via sector/line-tag and linedef-type 1000-1031
//
// Revision 1.18  2000/10/01 10:18:17  bpereira
// no message
//
// Revision 1.17  2000/09/28 20:57:14  bpereira
// no message
//
// Revision 1.16  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.15  2000/08/10 14:08:48  hurdler
// no message
//
// Revision 1.14  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.13  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.12  2000/04/19 10:56:51  hurdler
// commited for exe release and tag only
//
// Revision 1.11  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.10  2000/04/11 19:07:23  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.9  2000/04/07 23:11:17  metzgermeister
// added mouse move
//
// Revision 1.8  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.7  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.5  2000/03/23 22:54:00  metzgermeister
// added support for HOME/.legacy under Linux
//
// Revision 1.4  2000/02/27 16:30:28  hurdler
// dead player bug fix + add allowmlook <yes|no>
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      game loop functions, events handling
//
//-----------------------------------------------------------------------------


// [WDJ] To show the demo version on the console
#define SHOW_DEMOVERSION   
#define DEBUG_DEMO
#define CURRENT_DEMOVERSION 143

#include "doomdef.h"
#include "command.h"
#include "console.h"
#include "dstrings.h"

#include "d_main.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "f_finale.h"
#include "p_setup.h"
#include "p_saveg.h"

#include "i_system.h"

#include "wi_stuff.h"
#include "am_map.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"

// SKY handling - still the wrong place.
#include "r_data.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_sky.h"

#include "s_sound.h"

#include "g_game.h"
#include "g_state.h"
#include "g_input.h"

//added:16-01-98:quick hack test of rocket trails
#include "p_fab.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_argv.h"

#include "hu_stuff.h"

#include "st_stuff.h"

#include "keys.h"
#include "i_joy.h"
#include "w_wad.h"
#include "z_zone.h"

#include "i_video.h"
#include "p_inter.h"
#include "p_info.h"
#include "byteptr.h"

#include "b_game.h"	//added by AC for acbot



extern boolean gamekeytapped[NUMINPUTS];


boolean G_CheckDemoStatus (void);
void    G_ReadDemoTiccmd (ticcmd_t* cmd,int playernum);
void    G_WriteDemoTiccmd (ticcmd_t* cmd,int playernum);
void    G_InitNew (skill_t skill, char* mapname, boolean resetplayer);

void    G_DoCompleted (void);
void    G_DoVictory (void);
void    G_DoWorldDone (void);


// demoversion the 'dynamic' version number, this should be == game VERSION
// when playing back demos, 'demoversion' receives the version number of the
// demo. At each change to the game play, demoversion is compared to
// the game version, if it's older, the changes are not done, and the older
// code is used for compatibility.
//
byte            demoversion;

byte            gameepisode;
byte            gamemap;
char            gamemapname[MAX_WADPATH];      // an external wad filename


gamemode_e      gamemode = indetermined;       // Game Mode - identify IWAD as shareware, retail etc.
gamemission_t   gamemission = doom;
boolean         raven = false;
language_t      language = english;          // Language.
boolean         modifiedgame;                  // Set if homebrew PWAD stuff has been added.


boolean         paused;

boolean         timingdemo;             // if true, exit with report on completion
boolean         nodrawers;              // for comparative timing purposes
boolean         noblit;                 // for comparative timing purposes
tic_t           demostarttime;              // for comparative timing purposes

boolean         netgame;                // only true if packets are broadcast
boolean         multiplayer;
boolean         playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];

// [WDJ] Whenever assign to these must update the _ptr too.
// They are not changed anywhere as often as players[] appears in IF stmts.
int             consoleplayer;          // player taking events and displaying
int             displayplayer;          // view being displayed
int             displayplayer2 = -1;    // for splitscreen, -1 when not in use
int             statusbarplayer;        // player who's statusbar is displayed
                                        // (for spying with F12)

// [WDJ] Simplify every test against a player ptr, and splitscreen
player_t *      consoleplayer_ptr = &players[0];
player_t *      displayplayer_ptr = &players[0];
player_t *      displayplayer2_ptr = NULL;  // NULL when not in use

tic_t           gametic;
tic_t           levelstarttic;          // gametic at level start
int             totalkills, totalitems, totalsecret;    // for intermission

char            demoname[32];
boolean         demorecording;
boolean         demoplayback;
byte*           demobuffer;
byte*           demo_p;
byte*           demoend;
boolean         singledemo;             // quit after playing a demo from cmdline

boolean         precache = true;        // if true, load all graphics at start

wbstartstruct_t wminfo;                 // parms for world map / intermission


// Background color fades for FS
unsigned long fadecolor;
int fadealpha;


void ShowMessage_OnChange(void);
void AllowTurbo_OnChange(void);

CV_PossibleValue_t showmessages_cons_t[]={{0,"Off"},{1,"On"},{2,"Not All"},{0,NULL}};
CV_PossibleValue_t crosshair_cons_t[]   ={{0,"Off"},{1,"Cross"},{2,"Angle"},{3,"Point"},{0,NULL}};

consvar_t cv_crosshair        = {"crosshair"   ,"0",CV_SAVE,crosshair_cons_t};
//consvar_t cv_crosshairscale   = {"crosshairscale","0",CV_SAVE,CV_YesNo};
consvar_t cv_autorun          = {"autorun"     ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_autorun2         = {"autorun2"    ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_mouse_invert     = {"invertmouse" ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_mouse_move       = {"mousemove"   ,"1",CV_SAVE,CV_OnOff};
consvar_t cv_alwaysfreelook   = {"alwaysmlook" ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_mouse2_invert    = {"invertmouse2","0",CV_SAVE,CV_OnOff};
consvar_t cv_mouse2_move      = {"mousemove2"  ,"1",CV_SAVE,CV_OnOff};
consvar_t cv_alwaysfreelook2  = {"alwaysmlook2","0",CV_SAVE,CV_OnOff};

consvar_t cv_showmessages     = {"showmessages","1",CV_SAVE | CV_CALL | CV_NOINIT,showmessages_cons_t,ShowMessage_OnChange};
consvar_t cv_allowturbo       = {"allowturbo"  ,"0",CV_NETVAR | CV_CALL, CV_YesNo, AllowTurbo_OnChange};

#if MAXPLAYERS>32
#error please update "player_name" table using the new value for MAXPLAYERS
#endif
#if MAXPLAYERNAME!=21
#error please update "player_name" table using the new value for MAXPLAYERNAME
#endif
// changed to 2d array 19990220 by Kin
char    player_names[MAXPLAYERS][MAXPLAYERNAME] =
{
    // THESE SHOULD BE AT LEAST MAXPLAYERNAME CHARS
    "Player 1\0a123456789a\0",
    "Player 2\0a123456789a\0",
    "Player 3\0a123456789a\0",
    "Player 4\0a123456789a\0",
    "Player 5\0a123456789a\0",        // added 14-1-98 for support 8 players
    "Player 6\0a123456789a\0",        // added 14-1-98 for support 8 players
    "Player 7\0a123456789a\0",        // added 14-1-98 for support 8 players
    "Player 8\0a123456789a\0",        // added 14-1-98 for support 8 players
    "Player 9\0a123456789a\0",
    "Player 10\0a123456789\0",
    "Player 11\0a123456789\0",
    "Player 12\0a123456789\0",
    "Player 13\0a123456789\0",
    "Player 14\0a123456789\0",
    "Player 15\0a123456789\0",
    "Player 16\0a123456789\0",
    "Player 17\0a123456789\0",
    "Player 18\0a123456789\0",
    "Player 19\0a123456789\0",
    "Player 20\0a123456789\0",
    "Player 21\0a123456789\0",
    "Player 22\0a123456789\0",
    "Player 23\0a123456789\0",
    "Player 24\0a123456789\0",
    "Player 25\0a123456789\0",
    "Player 26\0a123456789\0",
    "Player 27\0a123456789\0",
    "Player 28\0a123456789\0",
    "Player 29\0a123456789\0",
    "Player 30\0a123456789\0",
    "Player 31\0a123456789\0",
    "Player 32\0a123456789\0"
};

char *team_names[MAXPLAYERS] =
{
    "Team 1\0a890123456789b\0",
    "Team 2\0a890123456789b\0",
    "Team 3\0a890123456789b\0",
    "Team 4\0a890123456789b\0",
    "Team 5\0a890123456789b\0",
    "Team 6\0a890123456789b\0",
    "Team 7\0a890123456789b\0",
    "Team 8\0a890123456789b\0",
    "Team 9\0a890123456789b\0",
    "Team 10\0a90123456789b\0",
    "Team 11\0a90123456789b\0",
    "Team 12\0a90123456789b\0",      // the other name hare not used because no colors
    "Team 13\0a90123456789b\0",      // but who know ?
    "Team 14\0a90123456789b\0",
    "Team 15\0a90123456789b\0",
    "Team 16\0a90123456789b\0",
    "Team 17\0a90123456789b\0",
    "Team 18\0a90123456789b\0",
    "Team 19\0a90123456789b\0",
    "Team 20\0a90123456789b\0",
    "Team 21\0a90123456789b\0",
    "Team 22\0a90123456789b\0",
    "Team 23\0a90123456789b\0",
    "Team 24\0a90123456789b\0",
    "Team 25\0a90123456789b\0",
    "Team 26\0a90123456789b\0",
    "Team 27\0a90123456789b\0",
    "Team 28\0a90123456789b\0",
    "Team 29\0a90123456789b\0",
    "Team 30\0a90123456789b\0",
    "Team 31\0a90123456789b\0",
    "Team 32\0a90123456789b\0"
};

mobj_t*   bodyque[BODYQUESIZE];
int       bodyqueslot;

void*     statcopy;                      // for statistics driver

void ShowMessage_OnChange(void)
{
    if (!cv_showmessages.value)
        CONS_Printf("%s\n",MSGOFF);
    else
        CONS_Printf("%s\n",MSGON);
}


//  Build an original game map name from episode and map number,
//  based on the game mode (doom1, doom2...)
//
char* G_BuildMapName (int episode, int map)
{
    static char  mapname[9];    // internal map name (wad resource name)

    if (gamemode==doom2_commercial)
        strcpy (mapname, va("MAP%#02d",map));
    else
    {
        mapname[0] = 'E';
        mapname[1] = '0' + episode;
        mapname[2] = 'M';
        mapname[3] = '0' + map;
        mapname[4] = 0;
    }
    return mapname;
}


//
//  Clip the console player mouse aiming to the current view,
//  also returns a signed char for the player ticcmd if needed.
//  Used whenever the player view pitch is changed manually
//
//added:22-02-98:
//changed:3-3-98: do a angle limitation now
angle_t G_ClipAimingPitch(angle_t aiming)
{
  int32_t limitangle;

  //note: the current software mode implementation doesn't have true perspective
  if (rendermode == render_soft)
    limitangle = 732<<ANGLETOFINESHIFT;
  else
    limitangle = ANG90 - 1;

  int32_t p = aiming; // into signed to make comparisions simpler

  if (p > limitangle)
    p = limitangle;
  else if (p < -limitangle)
    p = -limitangle;
  
  return p; // back into angle_t (unsigned)
}


//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
// set displayplayer2_ptr to build player 2's ticcmd in splitscreen mode
//
angle_t localaiming, localaiming2; // player1 and player2
angle_t localangle, localangle2;

//added:06-02-98: mouseaiming (looking up/down with the mouse or keyboard)
#define KB_LOOKSPEED    (1<<25)
#define MAXPLMOVE		(forwardmove[1])
#define TURBOTHRESHOLD  0x32
#define SLOWTURNTICS    (6*NEWTICRATERATIO)

static fixed_t forwardmove[2] = {25/NEWTICRATERATIO, 50/NEWTICRATERATIO};
static fixed_t sidemove[2]    = {24/NEWTICRATERATIO, 40/NEWTICRATERATIO};
static fixed_t angleturn[3]   = {640, 1280, 320};        // + slow turn


// for change this table change also nextweapon func in g_game and P_PlayerThink
char extraweapons[8]={wp_chainsaw,-1,wp_supershotgun,-1,-1,-1,-1,-1};
byte nextweaponorder[NUMWEAPONS]={wp_fist,wp_chainsaw,wp_pistol,
     wp_shotgun,wp_supershotgun,wp_chaingun,wp_missile,wp_plasma,wp_bfg};

byte NextWeapon(player_t *player,int step)
{
    byte   w;
    int    i;
    for (i=0;i<NUMWEAPONS;i++)
        if( player->readyweapon == nextweaponorder[i] )
        {
            i = (i+NUMWEAPONS+step)%NUMWEAPONS;
            break;
        }
    for (;nextweaponorder[i]!=player->readyweapon; i=(i+NUMWEAPONS+step)%NUMWEAPONS)
    {
        w = nextweaponorder[i];
        
        // skip super shotgun for non-Doom2
        if (gamemode!=doom2_commercial && w==wp_supershotgun)
            continue;

        // skip plasma-bfg in sharware
        if (gamemode==doom_shareware && (w==wp_plasma || w==wp_bfg))
            continue;

        if ( player->weaponowned[w] &&
             player->ammo[player->weaponinfo[w].ammo] >= player->weaponinfo[w].ammopershoot )
        {
            if(w==wp_chainsaw)
                return (BT_CHANGE | BT_EXTRAWEAPON | (wp_fist<<BT_WEAPONSHIFT));
            if(w==wp_supershotgun)
                return (BT_CHANGE | BT_EXTRAWEAPON | (wp_shotgun<<BT_WEAPONSHIFT));
            return (BT_CHANGE | (w<<BT_WEAPONSHIFT));
        }
    }
    return 0;
}

byte BestWeapon(player_t *player)
{
    int newweapon = FindBestWeapon(player);

    if (newweapon == player->readyweapon)
        return 0;

    if (newweapon == wp_chainsaw)
        return (BT_CHANGE | BT_EXTRAWEAPON | (wp_fist<<BT_WEAPONSHIFT));

    if (newweapon == wp_supershotgun)
        return (BT_CHANGE | BT_EXTRAWEAPON | (wp_shotgun<<BT_WEAPONSHIFT));

    return (BT_CHANGE | (newweapon<<BT_WEAPONSHIFT));
}

boolean G_InventoryResponder(player_t *ply, int gc[num_gamecontrols][2], event_t *ev)
{
  // [WDJ] 1/9/2009 Do not get to process any keyup events, unless also saw
  // the keydown event.  Now other Responders intercepting
  // the keydown event work correctly.  Specifically heretic will no longer
  // use up an inventory item when game saving.
  static boolean keyup_armed = false;
   
  if (!inventory)
    return false;

  switch (ev->type)
    {
    case ev_keydown:
      keyup_armed = false;  // [WDJ] blanket disable of keyup events
      if( ev->data1 == gc[gc_invprev][0] || ev->data1 == gc[gc_invprev][1] )
            {
                if( ply->st_inventoryTics )
                {
                    ply->inv_ptr--;
                    if( ply->inv_ptr < 0 )
                        ply->inv_ptr = 0;
                    else
                    {
                        ply->st_curpos--;
                        if( ply->st_curpos < 0 )
                            ply->st_curpos = 0;
                    }
                }
                ply->st_inventoryTics = 5*TICRATE;
                return true;
            }
      else if( ev->data1 == gc[gc_invnext][0] || ev->data1 == gc[gc_invnext][1] )
            {
                if( ply->st_inventoryTics )
                {
                    ply->inv_ptr++;
                    if( ply->inv_ptr >= ply->inventorySlotNum )
                    {
                        ply->inv_ptr--;
                        if( ply->inv_ptr < 0)
                            ply->inv_ptr = 0;
                    }
                    else
                    {
                        ply->st_curpos++;
                        if( ply->st_curpos > 6 )
                            ply->st_curpos = 6;
                    }
                }
                ply->st_inventoryTics = 5*TICRATE;
                return true;
            }
      else if( ev->data1 == gc[gc_invuse ][0] || ev->data1 == gc[gc_invuse ][1] ){
 		keyup_armed = true;  // [WDJ] enable keyup event
                return true;
	    }

      break;

    case ev_keyup:
      if( ev->data1 == gc[gc_invuse ][0] || ev->data1 == gc[gc_invuse ][1] )
            {
	      if( keyup_armed ) { // [WDJ] Only if the keydown was not intercepted by some other responder
                if( ply->st_inventoryTics )
                    ply->st_inventoryTics = 0;
                else if( ply->inventory[ply->inv_ptr].count>0 )
                    {
                        if( ply == consoleplayer_ptr )
                            SendNetXCmd(XD_USEARTEFACT, &ply->inventory[ply->inv_ptr].type, 1);
                        else
                            SendNetXCmd2(XD_USEARTEFACT, &ply->inventory[ply->inv_ptr].type, 1);
                    }
	        return true;	// [WDJ] same as other event intercepts
	      }
            }
      else if( ev->data1 == gc[gc_invprev][0] || ev->data1 == gc[gc_invprev][1] ||
               ev->data1 == gc[gc_invnext][0] || ev->data1 == gc[gc_invnext][1] )
	return true;
      break;

    default:
      break; // shut up compiler
    }

  return false;
}



void G_BuildTiccmd(ticcmd_t* cmd, int realtics, int which_player)
{
    int         i;
    
    ticcmd_t *base = I_BaseTiccmd ();             // empty, or external driver
    memcpy (cmd,base,sizeof(*cmd));

    
    player_t *this_player;
    int (*gcc)[2];

#define G_KEY_DOWN(k) (gamekeydown[gcc[(k)][0]] || gamekeydown[gcc[(k)][1]])
#define G_KEY_PRESSED(k) (G_KEY_DOWN(k) || gamekeytapped[gcc[(k)][0]] || gamekeytapped[gcc[(k)][1]])

    angle_t pitch;

    if (which_player == 0)
    {
      this_player = consoleplayer_ptr;
      gcc = gamecontrol;
      pitch = localaiming;
    } else {
      this_player = displayplayer2_ptr;
      gcc = gamecontrol2;
      pitch = localaiming2;
    }

    // Exit now if locked
    if (this_player->locked)
      goto done;

    // a little clumsy, but then the g_input.c became a lot simpler!
    boolean strafe = G_KEY_DOWN(gc_strafe);
    int speed  = G_KEY_DOWN(gc_speed) ^ (which_player == 0 ? cv_autorun.value : cv_autorun2.value);

    boolean turnright = G_KEY_DOWN(gc_turnright);
    boolean turnleft  = G_KEY_DOWN(gc_turnleft);
    boolean mouseaiming = G_KEY_DOWN(gc_mouseaiming) ^ (which_player == 0 ? cv_alwaysfreelook.value : cv_alwaysfreelook2.value);


    int forward = 0, side = 0; // these must not wrap around, so we need bigger ranges than chars

    // strafing and yaw
    if (strafe)
    {
        if (turnright)
            side += sidemove[speed];
        if (turnleft)
            side -= sidemove[speed];
    }
    else
    {
      // use two stage accelerative turning
      // on the keyboard and joystick
      static int  turnheld[2];   // for accelerative turning

      if (turnleft || turnright)
        turnheld[which_player] += realtics;
      else
        turnheld[which_player] = 0;
      
      int tspeed = (turnheld[which_player] < SLOWTURNTICS) ? 2 : speed;

      if (turnright)
	cmd->angleturn -= angleturn[tspeed];
      if (turnleft)
	cmd->angleturn += angleturn[tspeed];
    }

    // forwards/backwards, strafing
    if (G_KEY_DOWN(gc_forward))
        forward += forwardmove[speed];
    if (G_KEY_DOWN(gc_backward))
        forward -= forwardmove[speed];
    //added:07-02-98: some people strafe left & right with mouse buttons
    if (G_KEY_DOWN(gc_straferight))
        side += sidemove[speed];
    if (G_KEY_DOWN(gc_strafeleft))
        side -= sidemove[speed];

    //added:07-02-98: fire with any button/key
    if (G_KEY_DOWN(gc_fire))
        cmd->buttons |= BT_ATTACK;

    //added:07-02-98: use with any button/key
    if (G_KEY_DOWN(gc_use))
        cmd->buttons |= BT_USE;

    //added:22-02-98: jump button
    if (cv_allowjump.value && G_KEY_DOWN(gc_jump))
        cmd->buttons |= BT_JUMP;


    //added:07-02-98: any key / button can trigger a weapon
    // chainsaw overrides
    if (G_KEY_PRESSED(gc_nextweapon))
      cmd->buttons |= NextWeapon(this_player,1);
    else if (G_KEY_PRESSED(gc_prevweapon))
      cmd->buttons |= NextWeapon(this_player,-1);
    else if (G_KEY_PRESSED(gc_bestweapon))
      cmd->buttons |= BestWeapon(this_player);
    else
    for (i=gc_weapon1; i<gc_weapon1+NUMWEAPONS-1; i++)
      if (G_KEY_PRESSED(i))
        {
            cmd->buttons |= BT_CHANGE | BT_EXTRAWEAPON; // extra by default
            cmd->buttons |= (i-gc_weapon1)<<BT_WEAPONSHIFT;
            // already have extraweapon in hand switch to the normal one
            if (this_player->readyweapon == extraweapons[i-gc_weapon1])
                cmd->buttons &= ~BT_EXTRAWEAPON;
            break;
        }


    // pitch
    static boolean keyboard_look[2]; // true if lookup/down using keyboard


    // spring back if not using keyboard neither mouselookin'
    if (!keyboard_look[which_player] && !mouseaiming)
        pitch = 0;

    if (G_KEY_DOWN(gc_lookup))
    {
        pitch += KB_LOOKSPEED;
        keyboard_look[which_player] = true;
    }
    else
    if (G_KEY_DOWN(gc_lookdown))
    {
        pitch -= KB_LOOKSPEED;
        keyboard_look[which_player] = true;
    }
    else
    if (G_KEY_PRESSED(gc_centerview))
      {
        pitch = 0;
        keyboard_look[which_player] = false;
      }

    // mice

    // mouse look stuff (mouse look is not the same as mouse aim)
    if (which_player == 0)
    {
      if (mouseaiming)
      {
        keyboard_look[which_player] = false;

        // looking up/down
        if (cv_mouse_invert.value)
            pitch -= mousey<<19;
        else
            pitch += mousey<<19;
      }
      else if (cv_mouse_move.value)
	forward += mousey;

      if (strafe)
        side += mousex*2;
      else
        cmd->angleturn -= mousex*8;

      mousex = mousey = 0;
    }
    else
    {
      if (mouseaiming)
      {
	keyboard_look[which_player] = false;

        // looking up/down
        if (cv_mouse2_invert.value)
	  pitch -= mouse2y<<19;
        else
          pitch += mouse2y<<19;
      }
      else if (cv_mouse2_move.value)
	forward += mouse2y;

      if (strafe)
        side += mouse2x*2;
      else
        cmd->angleturn -= mouse2x*8;

      mouse2x = mouse2y = 0;
    }

    // Finally the joysticks.
    for (i=0; i < num_joybindings; i++)
    {
      joybinding_t j = joybindings[i];

      if (j.playnum != which_player)
	continue;

      int value = (int)(j.scale * I_JoystickGetAxis(j.joynum, j.axisnum));
      switch (j.action)
	{
	case ja_pitch  : pitch = value << 16; break;
	case ja_move   : forward += value; break;
	case ja_turn   : cmd->angleturn += value; break;
	case ja_strafe : side += value; break;
	default: break;
	}
    }


    // Do not go faster than max. speed
    if (forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;
    if (side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if (side < -MAXPLMOVE)
        side = -MAXPLMOVE;

    cmd->forwardmove += forward;
    cmd->sidemove += side;

    //26/02/2000: added by Hurdler: accept no mlook for network games
    if (!cv_allowmlook.value)
        pitch = 0;

    pitch = G_ClipAimingPitch(pitch); // clip pitch to a reasonable sector
    cmd->aiming = pitch >> 16; // to short

    if (which_player == 0)
    {
#ifdef ABSOLUTEANGLE
    localangle += (cmd->angleturn<<16);
    cmd->angleturn = localangle >> 16;
#endif
      localaiming = pitch;
    } else {
#ifdef ABSOLUTEANGLE
    localangle2 += (cmd->angleturn<<16);
    cmd->angleturn = localangle2 >> 16;
#endif
      localaiming2 = pitch;
    }

    if( gamemode == heretic )
    {
        if (G_KEY_DOWN(gc_flydown))
            cmd->angleturn |= BT_FLYDOWN;
        else
            cmd->angleturn &= ~BT_FLYDOWN;
    }

 done:
    memset(gamekeytapped, 0, sizeof(gamekeytapped)); // we're done, reset key-tapping status
}


static fixed_t  originalforwardmove[2] = {0x19, 0x32};
static fixed_t  originalsidemove[2] = {0x18, 0x28};

void AllowTurbo_OnChange(void)
{
    if(!cv_allowturbo.value && netgame)
    {
        // like turbo 100
        forwardmove[0] = originalforwardmove[0];
        forwardmove[1] = originalforwardmove[1];
        sidemove[0] = originalsidemove[0];
        sidemove[1] = originalsidemove[1];
    }
}

//  turbo <10-255>
//
void Command_Turbo_f (void)
{
    int     scale = 200;

    if(!cv_allowturbo.value && netgame)
    {
        CONS_Printf("This server don't allow turbo\n");
        return;
    }

    if (COM_Argc()!=2)
    {
        CONS_Printf("turbo <10-255> : set turbo");
        return;
    }

    scale = atoi (COM_Argv(1));

    if (scale < 10)
        scale = 10;
    if (scale > 255)
        scale = 255;

    CONS_Printf ("turbo scale: %i%%\n",scale);

    forwardmove[0] = originalforwardmove[0]*scale/100;
    forwardmove[1] = originalforwardmove[1]*scale/100;
    sidemove[0] = originalsidemove[0]*scale/100;
    sidemove[1] = originalsidemove[1]*scale/100;
}


//
// G_DoLoadLevel
//
void G_DoLoadLevel (boolean resetplayer)
{
    int             i;

    levelstarttic = gametic;        // for time calculation
	
    // Reset certain attributes
    // (should be in resetplayer 'if'?)
    fadealpha = 0;
    extramovefactor = 0;
    JUMPGRAVITY = (6*FRACUNIT/NEWTICRATERATIO);
    consoleplayer_ptr->locked = false;

    if (wipegamestate == GS_LEVEL)
        wipegamestate = -1;             // force a wipe

    gamestate = GS_LEVEL;
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if( resetplayer || (playeringame[i] && players[i].playerstate == PST_DEAD))
            players[i].playerstate = PST_REBORN;
        memset (players[i].frags,0,sizeof(players[i].frags));
        players[i].addfrags = 0;
    }

    if (!P_SetupLevel (gameepisode, gamemap, gameskill, gamemapname[0] ? gamemapname:NULL) )
    {
        // fail so reset game stuff
        Command_ExitGame_f();
        return;
    }

    //BOT_InitLevelBots ();

    displayplayer = consoleplayer;          // view the guy you are playing
    displayplayer_ptr = consoleplayer_ptr;

    if(!cv_splitscreen.value)
    {
        // [WDJ] Changed to a testable off for player 2
        displayplayer2 = -1;
        displayplayer2_ptr = NULL;  // use as test for player2 active
    }

    gameaction = ga_nothing;
#ifdef PARANOIA
    Z_CheckHeap (-2);
#endif

    if (camera.chase)
        P_ResetCamera ( displayplayer_ptr );

    // clear cmd building stuff
    memset(gamekeydown, 0, sizeof(gamekeydown));
    memset(gamekeytapped, 0, sizeof(gamekeytapped));
    mousex = mousey = mouse2x = mouse2y = 0;

    // clear hud messages remains (usually from game startup)
    HU_ClearFSPics();
    CON_ClearHUD ();
}

//
// G_Responder
//  Get info needed to make ticcmd_ts for the players.
//
boolean G_Responder (event_t* ev)
{
    // allow spy mode changes even during the demo
    if (gamestate == GS_LEVEL && ev->type == ev_keydown
        && ev->data1 == KEY_F12 && (singledemo || !cv_deathmatch.value) )
    {
        // spy mode
        do
        {
            displayplayer++;
            if (displayplayer == MAXPLAYERS)
                displayplayer = 0;
        } while (!playeringame[displayplayer] && displayplayer != consoleplayer);
        displayplayer_ptr = &players[displayplayer];

        //added:16-01-98:change statusbar also if playingback demo
        if( singledemo )
            ST_changeDemoView ();

        //added:11-04-98: tell who's the view
        CONS_Printf("Viewpoint : %s\n", player_names[displayplayer]);

        return true;
    }

    // any other key pops up menu if in demos
    if (gameaction == ga_nothing && !singledemo &&
        (demoplayback || gamestate == GS_DEMOSCREEN) )
    {
        if (ev->type == ev_keydown)
        {
            M_StartControlPanel ();
            return true;
        }
        return false;
    }

    if (gamestate == GS_LEVEL)
    {
#if 0
        if (devparm && ev->type == ev_keydown && ev->data1 == ';')
        {
            // added Boris : test different player colors
            consoleplayer_ptr->skincolor = (consoleplayer_ptr->skincolor+1) %MAXSKINCOLORS;
            consoleplayer_ptr->mo->flags |= (consoleplayer_ptr->skincolor)<<MF_TRANSSHIFT;
            G_DeathMatchSpawnPlayer (0);
            return true;
        }
#endif
        if(!multiplayer)
           if( cht_Responder (ev))
               return true;
        if (HU_Responder (ev))
            return true;        // chat ate the event
        if (ST_Responder (ev))
            return true;        // status window ate it
        if (AM_Responder (ev))
            return true;        // automap ate it
        if (G_InventoryResponder (consoleplayer_ptr, gamecontrol, ev))
            return true;
        if (displayplayer2_ptr && G_InventoryResponder (displayplayer2_ptr, gamecontrol2, ev))
            return true;
        //added:07-02-98: map the event (key/mouse/joy) to a gamecontrol
    }

    if (gamestate == GS_FINALE)
    {
        if (F_Responder (ev))
            return true;        // finale ate the event
    }


    // update keys current state
    G_MapEventsToControls (ev);

    switch (ev->type)
    {
      case ev_keydown:
        if (ev->data1 == KEY_PAUSE)
        {
            COM_BufAddText("pause\n");
            return true;
        }
        return true;

      case ev_keyup:
        return false;   // always let key up events filter down

      case ev_mouse:
        return true;    // eat events

      default:
        break;
    }

    return false;
}


//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker (void)
{
    ULONG       i;
    int         buf;
    ticcmd_t*   cmd;

    // do player reborns if needed
    if( gamestate == GS_LEVEL )
    {
        for (i=0 ; i<MAXPLAYERS ; i++)
            if (playeringame[i])
            {
                if( players[i].playerstate == PST_REBORN )
                    G_DoReborn (i);
                if( players[i].st_inventoryTics )
                    players[i].st_inventoryTics--;
            }
    }

    // do things to change the game state
    while (gameaction != ga_nothing)
        switch (gameaction)
        {
            case ga_completed :  G_DoCompleted (); break;
            case ga_worlddone :  G_DoWorldDone (); break;
            case ga_nothing   :  break;
            default : I_Error("gameaction = %d\n", gameaction);
        }

    buf = gametic%BACKUPTICS;

    // read/write demo and check turbo cheat
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        // BP: i==0 for playback of demos 1.29 now new players is added with xcmd
        if ((playeringame[i] || i==0) && !dedicated)
        {
            cmd = &players[i].cmd;

            if (players[i].bot)	//added by AC for acbot
                B_BuildTiccmd(&players[i], &netcmds[buf][i]);

            if (demoplayback)
                G_ReadDemoTiccmd (cmd,i);
            else
                memcpy (cmd, &netcmds[buf][i], sizeof(ticcmd_t));

            if (demorecording)
                G_WriteDemoTiccmd (cmd,i);

            // check for turbo cheats
            if (cmd->forwardmove > TURBOTHRESHOLD
                && !(gametic % (32*NEWTICRATERATIO)) && ((gametic / (32*NEWTICRATERATIO))&3) == i )
            {
                static char turbomessage[80];
                sprintf (turbomessage, "%s is turbo!",player_names[i]);
                consoleplayer_ptr->message = turbomessage;
            }
        }
    }

    // do main actions
    switch (gamestate)
    {
      case GS_LEVEL:
        //IO_Color(0,255,0,0);
        P_Ticker ();             // tic the game
        //IO_Color(0,0,255,0);
        ST_Ticker ();
        AM_Ticker ();
        HU_Ticker ();
        break;

      case GS_INTERMISSION:
        WI_Ticker ();
        break;

      case GS_FINALE:
        F_Ticker ();
        break;

      case GS_DEMOSCREEN:
        D_PageTicker ();
        break;

      case GS_WAITINGPLAYERS:
      case GS_DEDICATEDSERVER:
      case GS_NULL:
      // do nothing
        break;
    }
}


//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_InitPlayer
// Called at the start.
// Called by the game initialization functions.
//
/* BP:UNUSED !
void G_InitPlayer (int player)
{
    player_t*   p;

    // set up the saved info
    p = &players[player];

    // clear everything else to defaults
    G_PlayerReborn (player);
}
*/


//
// G_PlayerFinishLevel
//  Can when a player completes a level.
//
void G_PlayerFinishLevel (int player)
{
    player_t*  p;
    int        i;

    p = &players[player];
    for(i=0; i<p->inventorySlotNum; i++)
        if( p->inventory[i].count>1) 
            p->inventory[i].count = 1;
    if(!cv_deathmatch.value)
        for(i = 0; i < MAXARTECONT; i++)
            P_PlayerUseArtifact(p, arti_fly);
    memset (p->powers, 0, sizeof (p->powers));
    if( gamemode == heretic )
        p->weaponinfo = wpnlev1info;    // cancel power weapons
    else
        p->weaponinfo = doomweaponinfo;
    p->cards = 0;
    p->mo->flags &= ~MF_SHADOW;         // cancel invisibility
    p->extralight = 0;                  // cancel gun flashes
    p->fixedcolormap = 0;               // cancel ir gogles
    p->damagecount = 0;                 // no palette changes
    p->bonuscount = 0;

    if(p->chickenTics)
    {
        p->readyweapon = p->mo->special1; // Restore weapon
        p->chickenTics = 0;
    }
    p->rain1 = NULL;
    p->rain2 = NULL;
}


// added 2-2-98 for hacking with dehacked patch
int initial_health=100; //MAXHEALTH;
int initial_bullets=50;

void VerifFavoritWeapon (player_t *player);

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//
void G_PlayerReborn (int player)
{
    player_t*   p;
    int         i;
    USHORT      frags[MAXPLAYERS];
    int         killcount;
    int         itemcount;
    int         secretcount;
    USHORT      addfrags;

    //from Boris
    int         skincolor;
    char        favoritweapon[NUMWEAPONS];
    boolean     originalweaponswitch;
    boolean     autoaim;
    int         skin;                           //Fab: keep same skin
#ifdef CLIENTPREDICTION2
    mobj_t      *spirit;
#endif
    bot_t*		bot;	//added by AC for acbot

    memcpy (frags,players[player].frags,sizeof(frags));
    addfrags = players[player].addfrags;
    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;

    //from Boris
    skincolor = players[player].skincolor;
    originalweaponswitch = players[player].originalweaponswitch;
    memcpy (favoritweapon,players[player].favoritweapon,NUMWEAPONS);
    autoaim   = players[player].autoaim_toggle;
    skin = players[player].skin;
#ifdef CLIENTPREDICTION2
    spirit = players[player].spirit;
#endif
    bot = players[player].bot;	//added by AC for acbot

    p = &players[player];
    memset (p, 0, sizeof(*p));

    memcpy (players[player].frags, frags, sizeof(players[player].frags));
    players[player].addfrags=addfrags;
    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;

    // save player config truth reborn
    players[player].skincolor = skincolor;
    players[player].originalweaponswitch = originalweaponswitch;
    memcpy (players[player].favoritweapon,favoritweapon,NUMWEAPONS);
    players[player].autoaim_toggle = autoaim;
    players[player].skin = skin;
#ifdef CLIENTPREDICTION2
    players[player].spirit = spirit;
#endif
    players[player].bot = bot;	//added by AC for acbot

    p->usedown = p->attackdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = initial_health;
    if( gamemode == heretic )
    {
        p->weaponinfo = wpnlev1info;
        p->readyweapon = p->pendingweapon = wp_goldwand;
        p->weaponowned[wp_staff] = true;
        p->weaponowned[wp_goldwand] = true;
        p->ammo[am_goldwand] = 50;
    }
    else
    {
        p->weaponinfo = doomweaponinfo;
        p->readyweapon = p->pendingweapon = wp_pistol;
        p->weaponowned[wp_fist] = true;
        p->weaponowned[wp_pistol] = true;
        p->ammo[am_clip] = initial_bullets;
    }

    // Boris stuff
    if(!p->originalweaponswitch)
        VerifFavoritWeapon(p);
    //eof Boris

    for (i=0 ; i<NUMAMMO ; i++)
        p->maxammo[i] = maxammo[i];
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot because something is occupying it
//
boolean G_CheckSpot ( int           playernum,
                      mapthing_t*   mthing )
{
    fixed_t             x;
    fixed_t             y;
    subsector_t*        ss;
    unsigned            an;
    mobj_t*             mo;
    int                 i;

    // added 25-4-98 : maybe there is no player start
    if(!mthing || mthing->type<0)
        return false;

    if (!players[playernum].mo)
    {
        // first spawn of level, before corpses
        for (i=0 ; i<playernum ; i++)
        {
            // added 15-1-98 check if player is in game (mistake from id)
            if (playeringame[i]
                && players[i].mo->x == mthing->x << FRACBITS
                && players[i].mo->y == mthing->y << FRACBITS)
                return false;
	}
        return true;
    }

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    ss = R_PointInSubsector (x,y);

    // check for respawn in team-sector
    if(ss->sector->teamstartsec)
    {
        if(cv_teamplay.value==1)
        {
            // color
            if(players[playernum].skincolor!=(ss->sector->teamstartsec-1)) // -1 because wanted to know when it is set
                return false;
        }
        else
        if(cv_teamplay.value==2)
        {
            // skins
            if(players[playernum].skin!=(ss->sector->teamstartsec-1)) // -1 because wanted to know when it is set
                return false;
        }
    }
    
    if (!P_CheckPosition (players[playernum].mo, x, y) )
        return false;

    // flush an old corpse if needed
    if (bodyqueslot >= BODYQUESIZE)
        P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE]);
    bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo;
    bodyqueslot++;

    // spawn a teleport fog
    an = ( ANG45 * (mthing->angle/45) ) >> ANGLETOFINESHIFT;

    mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an]
                      , ss->sector->floorheight
                      , MT_TFOG);

    //added:16-01-98:consoleplayer -> displayplayer (hear snds from viewpt)
    // removed 9-12-98: why not ????
    if ( displayplayer_ptr->viewz != 1 )
        S_StartSound (mo, sfx_telept);  // don't start sound on first frame

    return true;
}


//
// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//
boolean G_DeathMatchSpawnPlayer (int playernum)
{
    int             i,j,n;

    if( !numdmstarts )
        I_Error("No deathmatch start in this map !");

    if(demoversion<123)
        n=20;
    else
        n=64;

    for (j=0 ; j<n ; j++)
    {
        i = P_Random() % numdmstarts;
        if (G_CheckSpot (playernum, deathmatchstarts[i]) )
        {
            P_SpawnPlayer (deathmatchstarts[i], playernum);
            return true;
        }
    }

    if(demoversion<113)
    {
        // no good spot, so the player will probably get stuck
        P_SpawnPlayer (playerstarts[playernum], playernum);
        return true;
    }
    return false;
}

void G_CoopSpawnPlayer (int playernum)
{
    int i;

    // no deathmatch use the spot
    if (G_CheckSpot (playernum, playerstarts[playernum]) )
    {
        P_SpawnPlayer (playerstarts[playernum], playernum);
        return;
    }

    // try to spawn at one of the other players spots
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (G_CheckSpot (playernum, playerstarts[i]) )
        {
            P_SpawnPlayer (playerstarts[i], playernum);
            return;
        }
        // he's going to be inside something.  Too bad.
    }

    if(demoversion<113)
        P_SpawnPlayer (playerstarts[playernum], playernum);
    else
    {
        int  selections;

        if( !numdmstarts)
            I_Error("No deathmatch start in this map !");
        selections = P_Random() % numdmstarts;
        P_SpawnPlayer (deathmatchstarts[selections], playernum);
    }
}

//
// G_DoReborn
//
void G_DoReborn (int playernum)
{
    player_t*  player = &players[playernum];

    // boris comment : this test is like 'single player game'
    //                 all this kind of hiden variable must be removed
    if (!multiplayer && !cv_deathmatch.value)
    {
        // reload the level from scratch
        G_DoLoadLevel (true);
    }
    else
    {
        // respawn at the start

        // first dissasociate the corpse
        if(player->mo)
        {
            player->mo->player = NULL;
            player->mo->flags2 &= ~MF2_DONTDRAW;
        }
        // spawn at random spot if in death match
        if (cv_deathmatch.value)
        {
            if(G_DeathMatchSpawnPlayer (playernum))
               return;
	    // use coop spots too if deathmatch spots occupied
        }

        G_CoopSpawnPlayer (playernum);
    }
}

void G_AddPlayer( int playernum )
{
    player_t *p=&players[playernum];

    p->playerstate = PST_REBORN;
    memset(p->inventory, 0, sizeof( p->inventory ));
    p->inventorySlotNum = 0;
    p->inv_ptr = 0;
    p->st_curpos = 0;
    p->st_inventoryTics = 0;

    if( gamemode == heretic )
        p->weaponinfo = wpnlev1info;
    else
        p->weaponinfo = doomweaponinfo;
}

// DOOM Par Times
static const int pars[4][10] =
{
    {0},
    {0,30,75,120,90,165,180,180,30,165},
    {0,90,90,90,120,90,360,240,30,170},
    {0,90,45,90,150,90,90,165,30,135}
};

// DOOM II Par Times
static const int cpars[32] =
{
    30,90,120,120,90,150,120,120,270,90,        //  1-10
    210,150,150,150,210,150,420,150,210,150,    // 11-20
    240,150,180,150,150,300,330,420,300,180,    // 21-30
    120,30                                      // 31-32
};


//
// G_DoCompleted
//
boolean         secretexit;

void G_ExitLevel (void)
{
    if( gamestate==GS_LEVEL )
    {
        secretexit = false;
        gameaction = ga_completed;
    }
}

// Here's for the german edition.
void G_SecretExitLevel (void)
{
    // IF NO WOLF3D LEVELS, NO SECRET EXIT!
    if ( (gamemode == doom2_commercial)
      && (W_CheckNumForName("map31")<0))
        secretexit = false;
    else
        secretexit = true;
    gameaction = ga_completed;
}

void G_DoCompleted (void)
{
    int             i;

    gameaction = ga_nothing;

    for (i=0 ; i<MAXPLAYERS ; i++)
        if (playeringame[i])
            G_PlayerFinishLevel (i);        // take away cards and stuff

    if (automapactive)
        AM_Stop ();

    if ( gamemode != doom2_commercial)
    {
        switch(gamemap)
        {
          case 8:
            //BP add comment : no intermistion screen
            if(cv_deathmatch.value)
                wminfo.next = 0;
            else
            {
                // also for heretic
                // disconnect from network
                CL_Reset();
                F_StartFinale();
                return;
            }
          case 9:
            for (i=0 ; i<MAXPLAYERS ; i++)
                players[i].didsecret = true;
            break;
        }
    }
    //DarkWolf95: September 11, 2004: More chex stuff
    if (gamemode == chexquest1)
    {
        if( !modifiedgame && gamemap == 5 )  // original chexquest ends at E1M5
        {
		if(cv_deathmatch.value)
			wminfo.next=0;
		else
		{
			CL_Reset();
			F_StartFinale();
			return;
		}
	}
    }

    if(!dedicated)
	wminfo.didsecret = consoleplayer_ptr->didsecret;
    wminfo.epsd = gameepisode -1;
    wminfo.last = gamemap -1;

    // go to next level
    // wminfo.next is 0 biased, unlike gamemap
    wminfo.next = gamemap;
    
    // overwrite next level in some cases
    if ( gamemode == doom2_commercial)
    {
        if (secretexit)
            switch(gamemap)
            {
              case 15 : wminfo.next = 30; break;
              case 31 : wminfo.next = 31; break;
              default : wminfo.next = 15;break;
            }
        else
            switch(gamemap)
            {
              case 31:
              case 32: wminfo.next = 15; break;
              default: wminfo.next = gamemap;
            }
    }
    else
    if( gamemode == heretic )
    {
        static const int afterSecret[5] = { 7, 5, 5, 5, 4 };
        if (secretexit)
            wminfo.next = 8;    // go to secret level
        else if (gamemap == 9)
            wminfo.next = afterSecret[gameepisode-1]-1;
    }
    else
    {
        if (secretexit)
            wminfo.next = 8;    // go to secret level
        else if (gamemap == 9)
        {
            // returning from secret level
            switch (gameepisode)
            {
              case 1 :  wminfo.next = 3; break;
              case 2 :  wminfo.next = 5; break;
              case 3 :  wminfo.next = 6; break;
              case 4 :  wminfo.next = 2; break;
              default : wminfo.next = 0; break;
            }
        }
        else
            if (gamemap == 8)
                wminfo.next = 0; // wrape around in deathmatch
    }

    wminfo.maxkills = totalkills;
    wminfo.maxitems = totalitems;
    wminfo.maxsecret = totalsecret;
    wminfo.maxfrags = 0;
    if( info_partime != -1)
        wminfo.partime = TICRATE*info_partime;
    else if ( gamemode == doom2_commercial )
        wminfo.partime = TICRATE*cpars[gamemap-1];
    else
        wminfo.partime = TICRATE*pars[gameepisode][gamemap];
    wminfo.pnum = consoleplayer;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        wminfo.plyr[i].in = playeringame[i];
        wminfo.plyr[i].skills = players[i].killcount;
        wminfo.plyr[i].sitems = players[i].itemcount;
        wminfo.plyr[i].ssecret = players[i].secretcount;
        wminfo.plyr[i].stime = leveltime;
        memcpy (wminfo.plyr[i].frags, players[i].frags
                , sizeof(wminfo.plyr[i].frags));
        wminfo.plyr[i].addfrags = players[i].addfrags;
    }

    gamestate = GS_INTERMISSION;
    automapactive = false;

    if (statcopy)
        memcpy (statcopy, &wminfo, sizeof(wminfo));

    WI_Start (&wminfo);
}


//
// G_NextLevel (WorldDone)
//
// init next level or go to the final scene
// called by end of intermision screen (wi_stuff)
void G_NextLevel (void)
{
    gameaction = ga_worlddone;
    if (secretexit)
        consoleplayer_ptr->didsecret = true;

    if ( gamemode == doom2_commercial)
    {
        if(cv_deathmatch.value==0)
        {
            switch (gamemap)
            {
            case 15:
            case 31:
                if (!secretexit)
                    break;
            case 6:
            case 11:
            case 20:
            case 30:
                if( gamemap == 30 )
                    CL_Reset(); // end of game disconnect from server
                gameaction = ga_nothing;
                F_StartFinale ();
                break;
            }
        }
        else
            if(gamemap==30)
                wminfo.next = 0; // wrape around in deathmatch
    }
}

void G_DoWorldDone (void)
{
    if( demoversion<129 )
    {
        gamemap = wminfo.next+1;
        G_DoLoadLevel (true);
    }
    else
        // not in demo because demo have the mapcommand on it
        if(server && !demoplayback) 
        {
            if( cv_deathmatch.value==0 )
                // don't reset player between maps
                COM_BufAddText (va("map \"%s\" -noresetplayers\n",G_BuildMapName(gameepisode,wminfo.next+1)));
            else
                // resetplayer in deathmatch for more equality
                COM_BufAddText (va("map \"%s\"\n",G_BuildMapName(gameepisode,wminfo.next+1)));
        }
    
    gameaction = ga_nothing;
}


// compose menu message from strings
void compose_message( char * str1, char * str2 )
{
    char msgtemp[128];
    if( str2 == NULL )  str2 = "";
    sprintf( msgtemp, "%s %s\n\nPress ESC\n", str1, str2 );
    M_SimpleMessage ( msgtemp );
}


extern char  savegamedir[SAVESTRINGSIZE];

// Must be able to handle 99 savegame slots, even when
// not SAVEGAME99, so net game saves are universally accepted.
void G_Savegame_Name( /*OUT*/ char * namebuf, /*IN*/ int slot )
{
#ifdef SAVEGAMEDIR
    sprintf(namebuf, savegamename, savegamedir, slot);
#else
    sprintf(namebuf, savegamename, slot);
#endif
#ifdef PC_DOS
    if( slot > 9 )
    {
        // shorten name to 8 char
        int ln = strlen( namebuf );
        memmove( &namebuf[ln-4], &namebuf[ln-3], 4 );
    }
#endif
}

//
// G_InitFromSavegame
// Can be called by the startup code or the menu task.
//
// Called from menu M_LoadSelect from M_Responder,
// and from D_Main code for -loadgame command line switch.
void G_LoadGame (int slot)
{
    // [WDJ] will handle 99 slots
    COM_BufAddText(va("load %d\n",slot));
    // net command call to G_DoLoadGame
}

// Called from network command, sent from G_LoadGame
// Reads the save game file.
void G_DoLoadGame (int slot)
{
    char        savename[255];
    savegame_info_t   sginfo;  // read header info

    G_Savegame_Name( savename, slot );

    if( P_Savegame_Readfile( savename ) < 0 )  goto cannot_read_file;
    // file is open and savebuffer allocated

    if( ! P_Read_Savegame_Header( &sginfo ) )  goto load_header_failed;
    if( ! sginfo.have_game )  goto wrong_game;
    if( ! sginfo.have_wad )  goto wrong_wad;

    if(demoplayback)  // reset game engine
        G_StopDemo();

    //added:27-02-98: reset the game version
    G_Downgrade(VERSION);

    paused        = false;
    automapactive = false;

    // dearchive all the modifications
    P_LoadGame(); // read game data in savebuffer, defer error test
    if( P_Savegame_Closefile( 0 ) < 0 )  goto load_failed;
    // savegame buffer deallocated, and file closed

    gameaction = ga_nothing;
    gamestate = GS_LEVEL;

    displayplayer = consoleplayer;
    displayplayer_ptr = consoleplayer_ptr;

    // done
    multiplayer = playeringame[1];
    if(playeringame[1] && !netgame)
        CV_SetValue(&cv_splitscreen,1);

    if (setsizeneeded)
        R_ExecuteSetViewSize ();

    // draw the pattern into the back screen
    R_FillBackScreen ();
    CON_ToggleOff ();
    return;

cannot_read_file:
    CONS_Printf ("Couldn't read file %s", savename);
    goto failed_exit;

load_header_failed:
    compose_message( sginfo.msg, NULL );
    goto failed_exit;

wrong_game:
    compose_message( "savegame requires game:", sginfo.game );
    goto failed_exit;

wrong_wad:
    compose_message( "savegame requires wad:", sginfo.wad );
    goto failed_exit;

load_failed:
    M_SimpleMessage("savegame file corrupted\n\nPress ESC\n" );
    Command_ExitGame_f();
failed_exit:
    P_Savegame_Error_Closefile();  // to dealloate buffer
    // were not playing, but server got started by sending load message
    if( gamestate == GS_WAITINGPLAYERS )
    {
        // [WDJ] fix ALLREADYPLAYING message, so that still not playing
        Command_ExitGame_f();
    }
    return;
}

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string
//
// Called from menu M_DoSave from M_Responder.
void G_SaveGame ( int   slot, char* description )
{
    // Solo player has server, net player without server cannot save.
    if (server)
    {
        // [WDJ] will handle 99 slots
        COM_BufAddText(va("save %d \"%s\"\n",slot,description));
        // Net command call to G_DoSaveGame
    }
}

// Called from network command sent from G_SaveGame.
// Writes the save game file.
void G_DoSaveGame (int   savegameslot, char* savedescription)
{
    char        savename[256];

    gameaction = ga_nothing;

    G_Savegame_Name( savename, savegameslot );

    gameaction = ga_nothing;

    if( P_Savegame_Writefile( savename ) < 0 )  return;
    
    P_Write_Savegame_Header( savedescription );
    P_SaveGame();  // Write game data to savegame buffer.
   
    if( P_Savegame_Closefile( 1 ) < 0 )  return;

    gameaction = ga_nothing;

    consoleplayer_ptr->message = GGSAVED;

    // draw the pattern into the back screen
    R_FillBackScreen ();
    ST_Drawer( 1 );	// [WDJ] refresh status background without global flags
}


//
// G_InitNew
//  Can be called by the startup code or the menu task,
//  consoleplayer, displayplayer, playeringame[] should be set.
//
// Boris comment : single player start game
void G_DeferedInitNew (skill_t skill, char* mapname, boolean StartSplitScreenGame)
{
    G_Downgrade(VERSION);
    paused        = false;

    
    if( demoplayback )
        COM_BufAddText ("stopdemo\n");

    // this leave the actual game if needed
    SV_StartSinglePlayerServer();
    
    COM_BufAddText (va("splitscreen %d;deathmatch 0;fastmonsters 0;"
                       "respawnmonsters 0;timelimit 0;fraglimit 0\n",
                       StartSplitScreenGame));

    COM_BufAddText (va("map \"%s\" -skill %d -monsters 1\n",mapname,skill+1));
}

//
// This is the map command interpretation something like Command_Map_f
//
// called at : map cmd execution, doloadgame, doplaydemo
void G_InitNew (skill_t skill, char* mapname, boolean resetplayer)
{
    //added:27-02-98: disable selected features for compatibility with
    //                older demos, plus reset new features as default
    if(!G_Downgrade (demoversion))
    {
        CONS_Printf("Cannot Downgrade engine\n");
        CL_Reset();
        D_StartTitle();
        return;
    }

    if (paused)
    {
        paused = false;
        S_ResumeSound ();
    }

    if (skill > sk_nightmare)
        skill = sk_nightmare;

    M_ClearRandom ();

    if( server && skill == sk_nightmare )
    {
        CV_SetValue(&cv_respawnmonsters,1);
        CV_SetValue(&cv_fastmonsters,1);
    }

    // for internal maps only
    if (FIL_CheckExtension(mapname))
    {
        // external map file
        strncpy (gamemapname, mapname, MAX_WADPATH);
        gameepisode = 1;
        gamemap = 1;
    }
    else
    {
        // internal game map
        // well this  check is useless because it is done before (d_netcmd.c::command_map_f)
        // but in case of for demos....
        if (W_CheckNumForName(mapname)==-1)
        {
            CONS_Printf("\2Internal game map '%s' not found\n"
                        "(use .wad extension for external maps)\n",mapname);
            Command_ExitGame_f();
            return;
        }

        gamemapname[0] = 0;             // means not an external wad file
        if (gamemode==doom2_commercial)       //doom2
        {
            gamemap = atoi(mapname+3);  // get xx out of MAPxx
            gameepisode = 1;
        }
        else
        {
            gamemap = mapname[3]-'0';           // ExMy
            gameepisode = mapname[1]-'0';
        }
    }

    gameskill      = skill;
    playerdeadview = false;
    automapactive  = false;

    G_DoLoadLevel (resetplayer);
}


//added:03-02-98:
//
//  'Downgrade' the game engine so that it is compatible with older demo
//   versions. This will probably get harder and harder with each new
//   'feature' that we add to the game. This will stay until it cannot
//   be done a 'clean' way, then we'll have to forget about old demos..
//
boolean G_Downgrade(int version)
{
    int i;

    if (version<109)
        return false;

    if( version<130 )
    {
        mobjinfo[MT_BLOOD].radius = 20*FRACUNIT;
        mobjinfo[MT_BLOOD].height = 16*FRACUNIT;
        mobjinfo[MT_BLOOD].flags  = MF_NOBLOCKMAP;
    }
    else
    {
        mobjinfo[MT_BLOOD].radius = 3*FRACUNIT;
        mobjinfo[MT_BLOOD].height = 0*FRACUNIT;
        mobjinfo[MT_BLOOD].flags  = 0;
    }

    // smoke trails for skull head attack since v1.25
    if (version<125)
    {
        states[S_ROCKET].action.acv = NULL;

        states[S_SKULL_ATK3].action.acv = NULL;
        states[S_SKULL_ATK4].action.acv = NULL;
    }
    else
    {
        //activate rocket trails by default
        states[S_ROCKET].action.acv     = A_SmokeTrailer;

        // smoke trails behind the skull heads
        states[S_SKULL_ATK3].action.acv = A_SmokeTrailer;
        states[S_SKULL_ATK4].action.acv = A_SmokeTrailer;
    }

    //hmmm.. first time I see an use to the switch without break...
    switch (version)
    {
      case 109:
        // disable rocket trails
        states[S_ROCKET].action.acv = NULL; //NULL like in Doom2 v1.9

        // Boris : for older demos, initalise the new skincolor value
        //         also disable the new preferred weapons order.
        for(i=0;i<4;i++)
        {
            players[i].skincolor = i % MAXSKINCOLORS;
            players[i].originalweaponswitch=true;
        }//eof Boris
      case 110:
      case 111:
        //added:16-02-98: make sure autoaim is used for older
        //                demos not using mouse aiming
        for(i=0;i<MAXPLAYERS;i++)
            players[i].autoaim_toggle = true;

      default:
        break;
    }


    //SoM: 3/17/2000: Demo compatability
    if(version < 129) 
    {
        boomsupport = 0;
        allow_pushers = 0;
        variable_friction = 0;
    }
    else 
    {
        boomsupport = 1;
        allow_pushers = 1;
        variable_friction = 1;
    }

    // always true now, might be false in the future, if couldn't
    // go backward and disable all the features...
    demoversion = version;
    return true;
}


//
// DEMO RECORDING
//

#define ZT_FWD          0x01
#define ZT_SIDE         0x02
#define ZT_ANGLE        0x04
#define ZT_BUTTONS      0x08
#define ZT_AIMING       0x10
#define ZT_CHAT         0x20    // no more used
#define ZT_EXTRADATA    0x40
#define DEMOMARKER      0x80    // demoend

ticcmd_t oldcmd[MAXPLAYERS];

void G_ReadDemoTiccmd (ticcmd_t* cmd,int playernum)
{
    if (*demo_p == DEMOMARKER)
    {
        // end of demo data stream
        G_CheckDemoStatus ();
        return;
    }
    if(demoversion<112)
    {
        cmd->forwardmove = READCHAR(demo_p);
        cmd->sidemove = READCHAR(demo_p);
        cmd->angleturn = READBYTE(demo_p)<<8;
        cmd->buttons = READBYTE(demo_p);
        cmd->aiming = 0;
    }
    else
    {
        char ziptic=*demo_p++;

        if(ziptic & ZT_FWD)
            oldcmd[playernum].forwardmove = READCHAR(demo_p);
        if(ziptic & ZT_SIDE)
            oldcmd[playernum].sidemove = READCHAR(demo_p);
        if(ziptic & ZT_ANGLE)
        {
            if(demoversion<125)
                oldcmd[playernum].angleturn = READBYTE(demo_p)<<8;
            else
                oldcmd[playernum].angleturn = READ16(demo_p);
        }
        if(ziptic & ZT_BUTTONS)
            oldcmd[playernum].buttons = READBYTE(demo_p);
        if(ziptic & ZT_AIMING)
        {
            if(demoversion<128)
                oldcmd[playernum].aiming = READCHAR(demo_p);
            else
                oldcmd[playernum].aiming = READ16(demo_p);
        }
        if(ziptic & ZT_CHAT)
            demo_p++;
        if(ziptic & ZT_EXTRADATA)
            ReadLmpExtraData(&demo_p,playernum);
        else
            ReadLmpExtraData(0,playernum);

        memcpy(cmd,&(oldcmd[playernum]),sizeof(ticcmd_t));
    }
}

void G_WriteDemoTiccmd (ticcmd_t* cmd,int playernum)
{
    char ziptic=0;
    byte *ziptic_p;

    ziptic_p=demo_p++;  // the ziptic
                        // write at the end of this function

    if(cmd->forwardmove != oldcmd[playernum].forwardmove)
    {
        *demo_p++ = cmd->forwardmove;
        oldcmd[playernum].forwardmove = cmd->forwardmove;
        ziptic|=ZT_FWD;
    }

    if(cmd->sidemove != oldcmd[playernum].sidemove)
    {
        *demo_p++ = cmd->sidemove;
        oldcmd[playernum].sidemove=cmd->sidemove;
        ziptic|=ZT_SIDE;
    }

    if(cmd->angleturn != oldcmd[playernum].angleturn)
    {
        *(short *)demo_p = cmd->angleturn;
        demo_p +=2;
        oldcmd[playernum].angleturn=cmd->angleturn;
        ziptic|=ZT_ANGLE;
    }

    if(cmd->buttons != oldcmd[playernum].buttons)
    {
        *demo_p++ = cmd->buttons;
        oldcmd[playernum].buttons=cmd->buttons;
        ziptic|=ZT_BUTTONS;
    }

    if(cmd->aiming != oldcmd[playernum].aiming)
    {
        *(short *)demo_p = cmd->aiming;
        demo_p+=2;
        oldcmd[playernum].aiming=cmd->aiming;
        ziptic|=ZT_AIMING;
    }

    if(AddLmpExtradata(&demo_p,playernum))
        ziptic|=ZT_EXTRADATA;

    *ziptic_p=ziptic;
    //added:16-02-98: attention here for the ticcmd size!
    // latest demos with mouse aiming byte in ticcmd
    if (ziptic_p > demoend - (5*MAXPLAYERS))
    {
        G_CheckDemoStatus ();   // no more space
        return;
    }

//  don't work in network the consistency is not copyed in the cmd
//    demo_p = ziptic_p;
//    G_ReadDemoTiccmd (cmd,playernum);         // make SURE it is exactly the same
}



//
// G_RecordDemo
//
void G_RecordDemo (char* name)
{
    int             i;
    int             maxsize;

    strcpy (demoname, name);
    strcat (demoname, ".lmp");
    maxsize = 0x20000;
    i = M_CheckParm ("-maxdemo");
    if (i && i<myargc-1)
        maxsize = atoi(myargv[i+1])*1024;
    demobuffer = Z_Malloc (maxsize,PU_STATIC,NULL);
    demoend = demobuffer + maxsize;

    demorecording = true;
}


void G_BeginRecording (void)
{
    int             i;

    demo_p = demobuffer;

    *demo_p++ = CURRENT_DEMOVERSION; // NOTE only needs to be updated when the demo format actually changes
    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = cv_deathmatch.value;     // just to be compatible with old demo (no more used)
    *demo_p++ = cv_respawnmonsters.value;// just to be compatible with old demo (no more used)
    *demo_p++ = cv_fastmonsters.value;   // just to be compatible with old demo (no more used)
    *demo_p++ = nomonsters;
    *demo_p++ = consoleplayer;
    *demo_p++ = cv_timelimit.value;      // just to be compatible with old demo (no more used)
    *demo_p++ = multiplayer;             // 1.31

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if(playeringame[i])
          *demo_p++ = 1;
        else
          *demo_p++ = 0;
    }

    memset(oldcmd,0,sizeof(oldcmd));
}


//
// G_PlayDemo
//

void G_DeferedPlayDemo (char* name)
{
    COM_BufAddText("playdemo \"");
    COM_BufAddText(name);
    COM_BufAddText("\"\n");
}


//
//  Start a demo from a .LMP file or from a wad resource (eg: DEMO1)
//
void G_DoPlayDemo (char *defdemoname)
{
    skill_t skill;
    int     i, episode, map;
    boolean boomdemo = 0;

//
// load demo file / resource
//

    //it's an internal demo
    if ((i=W_CheckNumForName(defdemoname)) == -1)
    {
        FIL_DefaultExtension(defdemoname,".lmp");
        if (!FIL_ReadFile (defdemoname, &demobuffer) )
        {
            CONS_Printf ("\2ERROR: couldn't open file '%s'.\n", defdemoname);
            goto no_demo;
        }
        demo_p = demobuffer;
    }
    else
        demobuffer = demo_p = W_CacheLumpNum (i, PU_STATIC);

//
// read demo header
//

    gameaction = ga_nothing;
    demoversion = READBYTE(demo_p);
    // header[0]: byte : demo version
    // 101 = Strife 1.01  (unsupported)
    // 104 = Doom 1.4 beta (unsupported)
    // 105 = Doom 1.5 beta (unsupported)
    // 106 = Doom 1.6 beta, 1.666 (unsupported)
    // 107 = Doom2 1.7, 1.7a (unsupported)
    // 108 = Doom 1.8, Doom2 1.8 (unsupported)
    // 109 = Doom 1.9, Doom2 1.9
    // 110 = Doom, published source code
    // 111..143 = Legacy
    // 200 = Boom 2.00	(supported badly, no sync)
    // 201 = Boom 2.01  (supported badly, no sync)
    // 202 = Boom 2.02  (supported badly, no sync)
    // 203 = LxDoom or MBF  (supported badly, no sync)
    // 210..214 = prboom (supported badly, no sync)
    // Do not have version: Hexen, Heretic, Doom 1.2 and before
#ifdef SHOW_DEMOVERSION
    CONS_Printf( "Demo Version %i.\n", (int)demoversion );
#endif
#ifdef DEBUG_DEMO
    fprintf( stderr, "Demo version %i.\n", (int)demoversion );
#endif

    // This works, but it also kills screen wipes    FIXME why?
//    if ( demoversion >= 200 )
    if (demoversion < 109 || demoversion >= 215)
    {
        CONS_Printf("\2ERROR: Incompatible demo (version %d). Legacy supports demo versions 109-%d.\n", demoversion, CURRENT_DEMOVERSION);
        goto kill_demo;
    }
   
    // Boom, MBF, and prboom headers
    // Used by FreeDoom
    if (demoversion >= 200 && demoversion <= 214)
    {
        // Read the "Boom" or "MBF" header line
        if( *demo_p == 0x1d )
        {
	    unsigned char header[10];
	    unsigned char compatibility;
	    demo_p ++;
	    for ( i=0; i<9; i++ )
	    {
	        header[i] = *demo_p++;
	        if( header[i] == 0xe6 )  break;
	    }
	    header[i] = 0;
	    // MBF and prboom header have compatibility level
	    compatibility = *demo_p++;
#ifdef DEBUG_DEMO
	    fprintf( stderr, " header: %s.\n", header );
	    fprintf( stderr, " compatibility %i.\n", compatibility );
#endif
	    boomdemo = 1;
	}
        else
        {
#ifdef DEBUG_DEMO
	    fprintf( stderr, " broken demo header\n" );
#endif
	    goto kill_demo;
	}
    }

    if (demoversion < VERSION)
        CONS_Printf ("\2Demo is from an older game version\n");

    // header[1]: byte: skill level 0..4
    skill       = *demo_p++;
    // header[2]: byte: Doom episode 1..3, Doom2 and above use 1
    episode     = *demo_p++;
    // header[3]: byte: map level 1..32
    map         = *demo_p++;
#ifdef DEBUG_DEMO
    fprintf( stderr, " skill %i.\n", (int)skill );
    fprintf( stderr, " episode %i.\n", (int)episode );
    fprintf( stderr, " map %i.\n", (int)map );
#endif
    // header[4]: byte: play mode 0..2
    //   0 = single player
    //   1 = deathmatch or cooperative
    //   2 = alt deathmatch
#ifdef DEBUG_DEMO
    fprintf( stderr, " play mode/deathmatch %i.\n", (int)demo_p[0] );
#endif
    if (demoversion < 127 || boomdemo)
        // push it in the console will be too late set
        cv_deathmatch.value=*demo_p++;
    else
        demo_p++;

    if( ! boomdemo )
    {
#ifdef DEBUG_DEMO
        fprintf( stderr, " respawn %i.\n", (int)demo_p[1] );
        fprintf( stderr, " fast monsters %i.\n", (int)demo_p[2] );
#endif
        // header[5]: byte: respawn boolean
        if (demoversion < 128)
	    // push it in the console will be too late set
	    cv_respawnmonsters.value=*demo_p++;
        else
            demo_p++;

        // header[6]: byte: fast boolean
        if (demoversion < 128)
        {
	    // push it in the console will be too late set
	    cv_fastmonsters.value=*demo_p++;
	    cv_fastmonsters.func();
	}
        else
	    demo_p++;

        // header[7]: byte: no monsters present boolean
        nomonsters  = *demo_p++;
#ifdef DEBUG_DEMO
        fprintf( stderr, " no monsters %i.\n", (int)nomonsters );
#endif
    }

    // header[8]: byte: viewing player 0..3, 0=player1
    //added:08-02-98: added displayplayer because the status bar links
    // to the display player when playing back a demo.
    displayplayer = consoleplayer = *demo_p++;
    displayplayer_ptr = consoleplayer_ptr = &players[consoleplayer];  // [WDJ]

#ifdef DEBUG_DEMO
    fprintf( stderr, " viewing player %i.\n",  (int)displayplayer );
#endif

     //added:11-01-98:
    //  support old v1.9 demos with ONLY 4 PLAYERS ! Man! what a shame!!!
    if (demoversion==109)
    {
       // header[9..12]: byte: player[1..4] present boolean
        for (i=0 ; i<4 ; i++) {
#ifdef DEBUG_DEMO
	    if( *demo_p )
	         fprintf( stderr, " player %i present %i.\n", i+1, (int)*demo_p );
#endif
            playeringame[i] = *demo_p++;
	}
    }
    else if( boomdemo )
    {
       	// [WDJ] according to prboom
        // [0] monsters remember
        // [1] variable friction
        // [2] weapon recoil
        // [3] allow pushers
        // [4] ??
        // [5] player bobbing
#ifdef DEBUG_DEMO
        fprintf( stderr, " respawn %i.\n", (int)demo_p[6] );
        fprintf( stderr, " fast monsters %i.\n", (int)demo_p[7] );
#endif
        cv_respawnmonsters.value = demo_p[6];  // respawn monsters, boolean
        cv_fastmonsters.value = demo_p[7]; // fast monsters, boolean
        cv_fastmonsters.func();
        nomonsters = demo_p[8];  // nomonsters, boolean
#ifdef DEBUG_DEMO
        fprintf( stderr, " no monsters %i.\n", (int)nomonsters );
#endif
        // [9] demo insurance
        // [10..13] random number gen
        if( demoversion == 203 ) // MBF
        {
	    // [14] monster infighting
	    // [15] dogs
	    // [16..17] ??
	    // [18..19] distfriend
	    // [20] monster backing
	    // [21] monster avoid hazards
	    // [22] monster friction
	    // [23] help friends
	    // [24] dog jumping
	    // [25] monkeys
	    // [26..57] comp vector x32
	    // [58] force old BSP
	}
        demo_p += (demoversion == 200)? 256 : 64;  // option area size
        // byte: player[1..32] present boolean
	// Made room for 32 players even though only supported 4
        for (i=0 ; i<32 ; i++) {
#ifdef DEBUG_DEMO
	    if( *demo_p )
	         fprintf( stderr, " player %i present %i.\n", i+1, (int)*demo_p );
#endif
            playeringame[i] = *demo_p++;
	}
    }
    else
    {
#ifdef DEBUG_DEMO
       fprintf( stderr, " time limit %i.\n", (int)*demo_p );
#endif
        if(demoversion<128)
        {
           cv_timelimit.value=*demo_p++;
           cv_timelimit.func();
        }
        else
            demo_p++;

        if (demoversion<113)
        {
	   // header[9..16]: byte: player[1..8] present boolean
            for (i=0 ; i<8 ; i++) {
#ifdef DEBUG_DEMO
	        if( *demo_p )
	            fprintf( stderr, " player %i present %i.\n", i+1, (int)*demo_p );
#endif
                playeringame[i] = *demo_p++;
	    }
        }
        else
        {
 	    // header[17]: byte: multiplayer boolean
            if( demoversion>=131 ) {
                multiplayer = *demo_p++;
#ifdef DEBUG_DEMO
 		fprintf( stderr, " multi-player %i.\n", (int)multiplayer );
#endif
	    }

 	    // header[18..50]: byte: player[1..32] present boolean
            for (i=0 ; i<32 ; i++) {
#ifdef DEBUG_DEMO
	        if( *demo_p )
	            fprintf( stderr, " player %i present %i.\n", i+1, (int)*demo_p );
#endif
                playeringame[i] = *demo_p++;
	    }
        }
#if MAXPLAYERS>32
#error Please add support for old lmps
#endif
    }

    // FIXME: do a proper test here
    if( demoversion<131 )
        multiplayer = playeringame[1];

    memset(oldcmd,0,sizeof(oldcmd));

    // don't spend a lot of time in loadlevel
    if(demoversion<127 || boomdemo)
    {
        precache = false;
        G_InitNew (skill, G_BuildMapName(episode, map),true);
        precache = true;
        CON_ToggleOff (); // will be done at the end of map command
    }
    else
        // wait map command in the demo
        gamestate = wipegamestate = GS_WAITINGPLAYERS;

    demoplayback = true;
    return;

kill_demo:
    demoversion = VERSION;
    Z_Free (demobuffer);
no_demo:
    gameaction = ga_nothing;
    return;
}

//
// G_TimeDemo
//             NOTE: name is a full filename for external demos
//
static int restorecv_vidwait;

void G_TimeDemo (char* name)
{
    nodrawers = M_CheckParm ("-nodraw");
    noblit = M_CheckParm ("-noblit");
    restorecv_vidwait = cv_vidwait.value;
    if( cv_vidwait.value )
        CV_Set( &cv_vidwait, "0");
    timingdemo = true;
    singletics = true;
    framecount = 0;
    demostarttime = I_GetTime ();
    G_DeferedPlayDemo (name);
}


void G_DoneLevelLoad(void)
{
    CONS_Printf("Load Level in %f sec\n",(float)(I_GetTime()-demostarttime)/TICRATE);
    framecount = 0;
    demostarttime = I_GetTime ();
}

/*
===================
=
= G_CheckDemoStatus
=
= Called after a death or level completion to allow demos to be cleaned up
= Returns true if a new demo loop action will take place
===================
*/

// reset engine variable set for the demos
// called from stopdemo command, map command, and g_checkdemoStatus.
void G_StopDemo(void)
{
    Z_Free (demobuffer);
    demoplayback  = false;
    timingdemo = false;
    singletics = false;

    G_Downgrade(VERSION);

    gamestate=wipegamestate=GS_NULL;
    SV_StopServer();
//    SV_StartServer();
    SV_ResetServer();
}

boolean G_CheckDemoStatus (void)
{
    if (timingdemo)
    {
        int time;
        float f1,f2;
        time = I_GetTime () - demostarttime;
        if(!time) return true;
        G_StopDemo ();
        timingdemo = false;
        f1=time;
        f2=framecount*TICRATE;
        CONS_Printf ("timed %i gametics in %i realtics\n"
                     "%f secondes, %f avg fps\n"
                     ,leveltime,time,f1/TICRATE,f2/f1);
        if( restorecv_vidwait != cv_vidwait.value )
            CV_SetValue(&cv_vidwait, restorecv_vidwait);
        D_AdvanceDemo ();
        return true;
    }

    if (demoplayback)
    {
        if (singledemo)
            I_Quit ();
        G_StopDemo();
        D_AdvanceDemo ();
        return true;
    }

    if (demorecording)
    {
        *demo_p++ = DEMOMARKER;
        FIL_WriteFile (demoname, demobuffer, demo_p - demobuffer);
        Z_Free (demobuffer);
        demorecording = false;

        CONS_Printf("\2Demo %s recorded\n",demoname);
        return true;
        //I_Quit ();
    }

    return false;
}
