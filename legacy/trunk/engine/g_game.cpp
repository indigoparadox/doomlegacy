// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2003 by DooM Legacy Team.
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
// $Log$
// Revision 1.20  2004/01/02 14:22:58  smite-meister
// items work
//
// Revision 1.19  2003/12/31 18:32:49  smite-meister
// Last commit of the year? Sound works.
//
// Revision 1.18  2003/12/13 23:51:03  smite-meister
// Hexen update
//
// Revision 1.17  2003/12/09 01:02:00  smite-meister
// Hexen mapchange works, keycodes fixed
//
// Revision 1.16  2003/12/06 23:57:47  smite-meister
// save-related bugfixes
//
// Revision 1.15  2003/11/23 00:41:54  smite-meister
// bugfixes
//
// Revision 1.14  2003/11/12 11:07:17  smite-meister
// Serialization done. Map progression.
//
// Revision 1.13  2003/06/01 18:56:29  smite-meister
// zlib compression, partial polyobj fix
//
// Revision 1.12  2003/05/30 13:34:42  smite-meister
// Cleanup, HUD improved, serialization
//
// Revision 1.11  2003/05/11 21:23:49  smite-meister
// Hexen fixes
//
// Revision 1.10  2003/04/26 12:01:12  smite-meister
// Bugfixes. Hexen maps work again.
//
// Revision 1.9  2003/04/04 00:01:53  smite-meister
// bugfixes, Hexen HUD
//
// Revision 1.8  2003/03/08 16:07:00  smite-meister
// Lots of stuff. Sprite cache. Movement+friction fix.
//
// Revision 1.7  2003/02/23 22:49:30  smite-meister
// FS is back! L2 cache works.
//
// Revision 1.6  2003/02/16 16:54:50  smite-meister
// L2 sound cache done
//
// Revision 1.5  2003/01/18 20:17:41  smite-meister
// HUD fixed, levelchange crash fixed.
//
// Revision 1.4  2002/12/29 18:57:03  smite-meister
// MAPINFO implemented, Actor deaths handled better
//
// Revision 1.3  2002/12/23 23:15:41  smite-meister
// Weapon groups, MAPINFO parser added!
//
// Revision 1.2  2002/12/16 22:11:00  smite-meister
// Actor/DActor separation done!
//
// Revision 1.1.1.1  2002/11/16 14:18:08  hurdler
// Initial C++ version of Doom Legacy
//
//
// DESCRIPTION:
//  Part of GameInfo class implementation
//      game loop functions, events handling
//
//-----------------------------------------------------------------------------

#include "doomdef.h"

#include "command.h"
#include "console.h"

#include "dstrings.h"
#include "g_game.h"
#include "g_player.h"
#include "g_map.h"
#include "g_pawn.h"

#include "d_items.h"
#include "g_input.h"
#include "d_main.h"

#include "d_clisrv.h"

#include "p_setup.h"
#include "p_saveg.h"

#include "i_system.h"
#include "m_random.h"

#include "r_render.h"
#include "r_state.h"
#include "r_draw.h"
#include "r_main.h"

#include "sounds.h"

#include "m_misc.h" // File handling
#include "m_menu.h"
#include "m_argv.h" // remove this!

#include "am_map.h"
#include "hu_stuff.h"
#include "wi_stuff.h"
#include "f_finale.h"

#include "keys.h"
#include "w_wad.h"
#include "z_zone.h"

#include "i_video.h" // rendermode! fix!

#include "i_joy.h" // move input processing somewhere else

#ifdef HWRENDER 
# include "hardware/hw_main.h"
#endif


tic_t   gametic;


void ShowMessage_OnChange();

CV_PossibleValue_t showmessages_cons_t[]={{0,"Off"},{1,"On"},{2,"Not All"},{0,NULL}};
CV_PossibleValue_t crosshair_cons_t[]   ={{0,"Off"},{1,"Cross"},{2,"Angle"},{3,"Point"},{0,NULL}};

consvar_t cv_crosshair        = {"crosshair"   ,"0",CV_SAVE,crosshair_cons_t};
consvar_t cv_autorun          = {"autorun"     ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_invertmouse      = {"invertmouse" ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_alwaysfreelook   = {"alwaysmlook" ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_mousemove        = {"mousemove"   ,"1",CV_SAVE,CV_OnOff};
consvar_t cv_showmessages     = {"showmessages","1",CV_SAVE | CV_CALL | CV_NOINIT,showmessages_cons_t,ShowMessage_OnChange};

consvar_t cv_crosshair2       = {"crosshair2"  ,"0",CV_SAVE,crosshair_cons_t};
consvar_t cv_autorun2         = {"autorun2"    ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_invertmouse2     = {"invertmouse2","0",CV_SAVE,CV_OnOff};
consvar_t cv_alwaysfreelook2  = {"alwaysmlook2","0",CV_SAVE,CV_OnOff};
consvar_t cv_mousemove2       = {"mousemove2"  ,"1",CV_SAVE,CV_OnOff};
consvar_t cv_showmessages2    = {"showmessages2","1",CV_SAVE | CV_CALL | CV_NOINIT,showmessages_cons_t,ShowMessage_OnChange};

//consvar_t cv_crosshairscale   = {"crosshairscale","0",CV_SAVE,CV_YesNo};

//consvar_t cv_allowturbo       = {"allowturbo"  ,"0",CV_NETVAR | CV_CALL, CV_YesNo, AllowTurbo_OnChange};

consvar_t cv_joystickfreelook = {"joystickfreelook" ,"0",CV_SAVE,CV_OnOff};


void ShowMessage_OnChange()
{
  if (!cv_showmessages.value)
    CONS_Printf("%s\n",MSGOFF);
  else
    CONS_Printf("%s\n",MSGON);
}

/*
static fixed_t originalforwardmove[2] = {0x19, 0x32};
static fixed_t originalsidemove[2]    = {0x18, 0x28};
static fixed_t forwardmove[2] = {25/NEWTICRATERATIO, 50/NEWTICRATERATIO};
static fixed_t sidemove[2]    = {24/NEWTICRATERATIO, 40/NEWTICRATERATIO};
*/

// stored in a signed char, but min = -100, max = 100, mmmkay?
// There is no more turbo cheat, you should modify the pawn's max speed instead
static char forwardmove[2] = {50, 100};
static char sidemove[2]    = {48, 80};


// fixed_t MaxPlayerMove[NUMCLASSES] = { 60, 50, 45, 49 }; // implemented as pawn speeds in info_m.cpp
// Otherwise OK, but fighter should also run sideways almost as fast as forward,
// (59/60) instead of (80/100). Weird. Affects straferunning.


static fixed_t angleturn[3]   = {640, 1280, 320};  // + slow turn
#define MAXPLMOVE (forwardmove[1])


/*
void AllowTurbo_OnChange()
{
  if(!cv_allowturbo.value && game.netgame)
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
void Command_Turbo_f()
{
  int scale = 200;

  if(!cv_allowturbo.value && game.netgame)
    {
      CONS_Printf("This server doesn't allow turbo\n");
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
*/


//  Clip the console player mouse aiming to the current view,
//  also returns a signed char for the player ticcmd if needed.
//  Used whenever the player view pitch is changed manually
//
//changed:3-3-98: do a angle limitation now
short G_ClipAimingPitch(int *aiming)
{
  int limitangle;

  //note: the current software mode implementation doesn't have true perspective
  if ( rendermode == render_soft)
    limitangle = 732<<ANGLETOFINESHIFT;
  else
    limitangle = ANG90 - 1;

  if (*aiming > limitangle)
    *aiming = limitangle;
  else if (*aiming < -limitangle)
    *aiming = -limitangle;

  return (*aiming)>>16;
}


static byte NextWeapon(PlayerPawn *p, int step)
{
  // Kludge. TODO when netcode is redone, fix me.
  int w = p->readyweapon;
  do
    {
      w = (w + step) % NUMWEAPONS;
      if (p->weaponowned[w] && p->ammo[p->weaponinfo[w].ammo] >= p->weaponinfo[w].ammopershoot)
	return BT_CHANGE | (weapondata[w].group << BT_WEAPONSHIFT);
    } while (w != p->readyweapon);

  return 0;
}


//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
// set secondaryplayer true to build player 2's ticcmd in splitscreen mode
//
int     localaiming,localaiming2;
angle_t localangle,localangle2;

//added:06-02-98: mouseaiming (looking up/down with the mouse or keyboard)
#define KB_LOOKSPEED    (1<<25)

#define SLOWTURNTICS    (6*NEWTICRATERATIO)

void G_BuildTiccmd(ticcmd_t* cmd, bool primary, int realtics)
{
  int         i, j, k;
  int         forward;
  int         side;
  ticcmd_t*   base;

  //added:14-02-98: these ones used for multiple conditions
  bool     turnleft,turnright,mouseaiming,analogjoystickmove,gamepadjoystickmove;

  int (*gc)[2]; //pointer to array[num_gamecontrols][2]
  PlayerPawn *p = NULL;

  if (primary)
    {
      gc = gamecontrol;
      i = cv_autorun.value;
      j = cv_alwaysfreelook.value;
      k = 0;
      if (consoleplayer)
	{
	  // FIXME this is a hack to ease debugging until the new netcode is done
	  // Here the netcode is bypassed. See also Game::Ticker() !
	  cmd = &consoleplayer->cmd;
	  p = consoleplayer->pawn;
	}
    }
  else
    {
      gc = gamecontrol2;
      i = cv_autorun2.value;
      j = cv_alwaysfreelook2.value;
      k = 1;
      if (consoleplayer2)
	p = consoleplayer2->pawn;
    }
  // FIXME no pawn nor player should be needed here. Fix doom network protocol!

  base = I_BaseTiccmd();             // empty, or external driver
  memcpy(cmd, base, sizeof(*cmd));

  // a little clumsy, but then the g_input.c became a lot simpler!
  bool strafe = gamekeydown[gc[gc_strafe][0]] || gamekeydown[gc[gc_strafe][1]];

  int speed = (gamekeydown[gc[gc_speed][0]] || gamekeydown[gc[gc_speed][1]]) ^ i;

  turnright = gamekeydown[gc[gc_turnright][0]]
    ||gamekeydown[gc[gc_turnright][1]];
  turnleft  = gamekeydown[gc[gc_turnleft][0]]
    ||gamekeydown[gc[gc_turnleft][1]];

  mouseaiming = (gamekeydown[gc[gc_mouseaiming][0]]
		 ||gamekeydown[gc[gc_mouseaiming][1]]) ^ j;

  if (primary) {
    analogjoystickmove  = cv_usejoystick.value && !Joystick.bGamepadStyle && !cv_splitscreen.value;
    gamepadjoystickmove = cv_usejoystick.value &&  Joystick.bGamepadStyle && !cv_splitscreen.value;
  } else {
    analogjoystickmove  = cv_usejoystick.value && !Joystick.bGamepadStyle;
    gamepadjoystickmove = cv_usejoystick.value &&  Joystick.bGamepadStyle;
  }

  if (gamepadjoystickmove)
    {
      turnright = turnright || (joyxmove > 0);
      turnleft  = turnleft  || (joyxmove < 0);
    }
  forward = side = 0;

  // use two stage accelerative turning
  // on the keyboard and joystick
  static int turnheld[2]; // for accelerative turning

  if (turnleft || turnright)
    turnheld[k] += realtics;
  else
    turnheld[k] = 0;

  int tspeed;
  if (turnheld[k] < SLOWTURNTICS)
    tspeed = 2;             // slow turn
  else
    tspeed = speed;

  // let movement keys cancel each other out
  if (strafe)
    {
      if (turnright)
	side += sidemove[speed];
      if (turnleft)
	side -= sidemove[speed];

      if (analogjoystickmove)
        {
	  //faB: JOYAXISRANGE is supposed to be 1023 ( divide by 1024)
	  side += ( (joyxmove * sidemove[1]) >> 10 );
        }
    }
  else
    {
      if (turnright)
	cmd->angleturn -= angleturn[tspeed];
      //else
      if (turnleft)
	cmd->angleturn += angleturn[tspeed];
      if ( joyxmove && analogjoystickmove) {
	//faB: JOYAXISRANGE should be 1023 ( divide by 1024)
	cmd->angleturn -= ( (joyxmove * angleturn[1]) >> 10 );        // ANALOG!
	//CONS_Printf ("joyxmove %d  angleturn %d\n", joyxmove, cmd->angleturn);
      }
    }

  //added:07-02-98: forward with key or button
  if (gamekeydown[gc[gc_forward][0]] || gamekeydown[gc[gc_forward][1]] ||
      ( joyymove < 0 && gamepadjoystickmove && !cv_joystickfreelook.value))
    {
      forward += forwardmove[speed];
    }
  if (gamekeydown[gc[gc_backward][0]] || gamekeydown[gc[gc_backward][1]] ||
      (joyymove > 0 && gamepadjoystickmove && !cv_joystickfreelook.value))
    {
      forward -= forwardmove[speed];
    }
        
  if (joyymove && analogjoystickmove && !cv_joystickfreelook.value) 
    forward -= ( (joyymove * forwardmove[1]) >> 10 );               // ANALOG!

  //added:07-02-98: some people strafe left & right with mouse buttons
  if (gamekeydown[gc[gc_straferight][0]] || gamekeydown[gc[gc_straferight][1]])
    side += sidemove[speed];
  if (gamekeydown[gc[gc_strafeleft][0]] || gamekeydown[gc[gc_strafeleft][1]])
    side -= sidemove[speed];

  if (gamekeydown[gc[gc_fire][0]] || gamekeydown[gc[gc_fire][1]])
    cmd->buttons |= BT_ATTACK;

  if (gamekeydown[gc[gc_use][0]] || gamekeydown[gc[gc_use][1]])
    cmd->buttons |= BT_USE;

  if (cv_allowjump.value &&
      (gamekeydown[gc[gc_jump][0]] || gamekeydown[gc[gc_jump][1]]))
    cmd->buttons |= BT_JUMP;

  if (p == NULL)
    return;
  //added:07-02-98: any key / button can trigger a weapon
  // chainsaw overrides
  if (gamekeydown[gc[gc_nextweapon][0]] || gamekeydown[gc[gc_nextweapon][1]])
    cmd->buttons |= NextWeapon(p, 1);
  else if (gamekeydown[gc[gc_prevweapon][0]] || gamekeydown[gc[gc_prevweapon][1]])
    cmd->buttons |= NextWeapon(p, -1);
  else for (i=gc_weapon1; i<=gc_weapon8; i++)
    if (gamekeydown[gc[i][0]] || gamekeydown[gc[i][1]])
      {
	cmd->buttons |= BT_CHANGE;
	cmd->buttons |= (i-gc_weapon1)<<BT_WEAPONSHIFT; // 8 keys = three bits
	break;
      }

  static bool keyboard_look[2];      // true if lookup/down using keyboard

  if (primary) {
    // mouse look stuff (mouse look is not the same as mouse aim)
    if (mouseaiming)
      {
        keyboard_look[0] = false;

        // looking up/down
        if (cv_invertmouse.value)
	  localaiming -= mlooky<<19;
        else
	  localaiming += mlooky<<19;
      }
    if (cv_usejoystick.value && analogjoystickmove && cv_joystickfreelook.value)
      localaiming += joyymove<<16;

    // spring back if not using keyboard neither mouselookin'
    if (!keyboard_look && !cv_joystickfreelook.value && !mouseaiming)
      localaiming = 0;

    if (gamekeydown[gc[gc_lookup][0]] || gamekeydown[gc[gc_lookup][1]])
      {
        localaiming += KB_LOOKSPEED;
        keyboard_look[0] = true;
      }
    else if (gamekeydown[gc[gc_lookdown][0]] || gamekeydown[gc[gc_lookdown][1]])
      {
        localaiming -= KB_LOOKSPEED;
        keyboard_look[0] = true;
      }
    else if (gamekeydown[gc[gc_centerview][0]] || gamekeydown[gc[gc_centerview][1]])
      localaiming = 0;

    //26/02/2000: added by Hurdler: accept no mlook for network games
    if (!cv_allowmlook.value)
      localaiming = 0;

    cmd->aiming = G_ClipAimingPitch (&localaiming);

    if (!mouseaiming && cv_mousemove.value)
      forward += mousey;

    if (strafe)
      side += mousex*2;
    else
      cmd->angleturn -= mousex*8;

    mousex = mousey = mlooky = 0;

  } else {

    // mouse look stuff (mouse look is not the same as mouse aim)
    if (mouseaiming)
      {
        keyboard_look[1] = false;
	
        // looking up/down
        if (cv_invertmouse2.value)
	  localaiming2 -= mlook2y<<19;
        else
	  localaiming2 += mlook2y<<19;
      }

    if (analogjoystickmove && cv_joystickfreelook.value)
      localaiming2 += joyymove<<16;
    // spring back if not using keyboard neither mouselookin'
    if (!keyboard_look && !cv_joystickfreelook.value && !mouseaiming)
      localaiming2 = 0;

    if (gamekeydown[gamecontrol2[gc_lookup][0]] ||
        gamekeydown[gamecontrol2[gc_lookup][1]])
      {
        localaiming2 += KB_LOOKSPEED;
        keyboard_look[1] = true;
      }
    else if (gamekeydown[gamecontrol2[gc_lookdown][0]] ||
	     gamekeydown[gamecontrol2[gc_lookdown][1]])
      {
        localaiming2 -= KB_LOOKSPEED;
        keyboard_look[1] = true;
      }
    else if (gamekeydown[gamecontrol2[gc_centerview][0]] ||
	     gamekeydown[gamecontrol2[gc_centerview][1]])
      localaiming2 = 0;

    //26/02/2000: added by Hurdler: accept no mlook for network games
    if (!cv_allowmlook.value)
      localaiming2 = 0;

    // look up max (viewheight/2) look down min -(viewheight/2)
    cmd->aiming = G_ClipAimingPitch (&localaiming2);;

    if (!mouseaiming && cv_mousemove2.value)
      forward += mouse2y;

    if (strafe)
      side += mouse2x*2;
    else
      cmd->angleturn -= mouse2x*8;

    mouse2x = mouse2y = mlook2y = 0;
  }

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

  //CONS_Printf("Move: %d, %d, %d\n", cmd->buttons, cmd->forwardmove, cmd->sidemove);
   
#ifdef ABSOLUTEANGLE
  if (primary) {
    localangle += (cmd->angleturn<<16);
    cmd->angleturn = localangle >> 16;
  } else {
    localangle2 += (cmd->angleturn<<16);
    cmd->angleturn = localangle2 >> 16;
  }
#endif

 if (game.mode == gm_heretic)
   {
     if (gamekeydown[gc[gc_flydown][0]] ||
	 gamekeydown[gc[gc_flydown][1]])
       cmd->angleturn |= BT_FLYDOWN;
     else
       cmd->angleturn &= ~BT_FLYDOWN;
   }
}


//--------------------------------------------
// was D_StartTitle
//
void GameInfo::StartIntro()
{
  extern int demosequence;
  action = ga_nothing;

  displayplayer = consoleplayer = NULL;
  demosequence = -1;
  paused = false;
  D_AdvanceDemo();
  CON_ToggleOff();
}


void GameInfo::Drawer()
{
  // draw the view directly
  //CONS_Printf("GI::Draw: %p, %p\n", displayplayer,displayplayer2);
  if (displayplayer && displayplayer->pawn && displayplayer->pawn->mp)
    {
      R.SetMap(displayplayer->pawn->mp);
#ifdef HWRENDER 
      if (rendermode != render_soft)
	R.HWR_RenderPlayerView(0, displayplayer);
      else //if (rendermode == render_soft)
#endif
	R.R_RenderPlayerView(displayplayer);
    }

  // added 16-6-98: render the second screen
  if (displayplayer2 && displayplayer2->pawn && displayplayer2->pawn->mp)
    {
      R.SetMap(displayplayer2->pawn->mp);
#ifdef HWRENDER 
      if (rendermode != render_soft)
	R.HWR_RenderPlayerView(1, displayplayer2);
      else 
#endif
	{
	  //faB: Boris hack :P !!
	  viewwindowy = vid.height/2;
	  memcpy(ylookup,ylookup2,viewheight*sizeof(ylookup[0]));
		  
	  R.R_RenderPlayerView(displayplayer2);
		  
	  viewwindowy = 0;
	  memcpy(ylookup,ylookup1,viewheight*sizeof(ylookup[0]));
	}
    }

  //CONS_Printf("GI::Draw done\n");
}

// was G_InventoryResponder
//
bool PlayerInfo::InventoryResponder(int (*gc)[2], event_t *ev)
{
  //gc is a pointer to array[num_gamecontrols][2]
  extern int st_curpos; // TODO: what about splitscreenplayer??

  if (!game.inventory)
    return false;

  if (!pawn)
    return false;

  switch (ev->type)
    {
    case ev_keydown :
      if (ev->data1 == gc[gc_invprev][0] || ev->data1 == gc[gc_invprev][1])
	{
	  if (pawn->invTics)
	    {
	      if (--(pawn->invSlot) < 0)
		pawn->invSlot = 0;
	      else if (--st_curpos < 0)
		st_curpos = 0;
	    }
	  pawn->invTics = 5*TICRATE;
	  return true;
	}
      else if (ev->data1 == gc[gc_invnext][0] || ev->data1 == gc[gc_invnext][1])
	{
	  int n = pawn->inventory.size();

	  if (pawn->invTics)
	    {
	      if (++(pawn->invSlot) >= n)
		pawn->invSlot = n-1;
	      else if (++st_curpos > 6)
		st_curpos = 6;
	    }
	  pawn->invTics = 5*TICRATE;
	  return true;
	}
      else if (ev->data1 == gc[gc_invuse ][0] || ev->data1 == gc[gc_invuse ][1])
	{
	  if (pawn->invTics)
	    pawn->invTics = 0;
	  else if (pawn->inventory[pawn->invSlot].count > 0)
	    {
	      // FIXME HACK bypassing netcode
	      pawn->UseArtifact(artitype_t(pawn->inventory[pawn->invSlot].type));
	      /*
	      if (1) // FIXME send playernum in the message...
		SendNetXCmd(XD_USEARTEFACT, &pawn->inventory[pawn->invSlot].type, 1);
	      else
		SendNetXCmd2(XD_USEARTEFACT, &pawn->inventory[pawn->invSlot].type, 1);
	      */
	    }
	  return true;
	}
      break;

    case ev_keyup:
      if (ev->data1 == gc[gc_invuse ][0] || ev->data1 == gc[gc_invuse ][1] ||
	  ev->data1 == gc[gc_invprev][0] || ev->data1 == gc[gc_invprev][1] ||
	  ev->data1 == gc[gc_invnext][0] || ev->data1 == gc[gc_invnext][1])
	return true;
      break;

    default:
      break; // shut up compiler
    }
  return false;
}



//
// G_Responder
//  Get info needed to make ticcmd_ts for the players.
//
bool GameInfo::Responder(event_t* ev)
{
  // allow spy mode changes even during the demo
  if (state == GS_LEVEL && ev->type == ev_keydown
      && ev->data1 == KEY_F12 && (singledemo || !cv_deathmatch.value))
    {
      // spy mode
      map<int, PlayerInfo *>::iterator i;
      if (displayplayer == NULL)
	i = Players.begin();
      else
	{
	  i = Players.upper_bound(displayplayer->number);
	  if (i == Players.end())
	    i = Players.begin();
	}

      if (i == Players.end())
	displayplayer = NULL;
      else
	displayplayer = (*i).second;

      if (displayplayer)
	{
	  //added:16-01-98:change statusbar also if playingback demo
	  if (singledemo)
	    hud.ST_Start(displayplayer->pawn);

	  //added:11-04-98: tell who's the view
	  CONS_Printf("Viewpoint : %s\n", displayplayer->name.c_str());
	}
      return true;
    }

  // any other key pops up menu if in demos
  if (action == ga_nothing && !singledemo &&
      (demoplayback || state == GS_DEMOSCREEN))
    {
      if (ev->type == ev_keydown)
        {
	  Menu::Open();
	  return true;
        }
      return false;
    }

  switch (state)
    {
    case GS_LEVEL:
      if (!multiplayer) //FIXME! The _server_ CAN cheat in multiplayer (maybe using console only?)
	if (cht_Responder (ev))
	  return true;
      if (hud.Responder(ev))
	return true;        // HUD ate the event
      if (automap.Responder(ev))
	return true;        // automap ate it

      if (consoleplayer->InventoryResponder(gamecontrol, ev))
	return true;

      if (cv_splitscreen.value && consoleplayer2->InventoryResponder(gamecontrol2, ev))
	return true;
      break;

    case GS_INTERMISSION:
      if (wi.Responder(ev))
	return true;
      break;

    case GS_FINALE:
      if (F_Responder (ev))
	return true;        // finale ate the event
      break;

    default:
      break;
    }

  // update keys current state
  G_MapEventsToControls(ev);

  // FIXME move these to Menu::Responder?
  switch (ev->type)
    {
    case ev_keydown:
      switch (ev->data1)
	{
	case KEY_PAUSE:
	  COM_BufAddText("pause\n");
	  return true;
	  
	case KEY_MINUS:     // Screen size down
	  CV_SetValue (&cv_viewsize, cv_viewsize.value-1);
	  S_StartAmbSound(sfx_menu_adjust);
	  return true;

	case KEY_EQUALS:    // Screen size up
	  CV_SetValue (&cv_viewsize, cv_viewsize.value+1);
	  S_StartAmbSound(sfx_menu_adjust);
	  return true;
	}

      return true;

    case ev_keyup:
      return false;   // always let key up events filter down

    case ev_mouse:
      return true;    // eat events

    case ev_joystick:
      return true;    // eat events

    default:
      break;
    }

  return false;
}


// returns player number 'num' if he is in the game, otherwise NULL
PlayerInfo *GameInfo::FindPlayer(int num)
{
  player_iter_t i = Players.find(num);
  if (i != Players.end())
    return (*i).second;

  return NULL;
}
