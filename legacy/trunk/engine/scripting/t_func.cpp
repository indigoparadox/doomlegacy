// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 2000 Simon Howard
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $Log$
// Revision 1.1  2002/11/16 14:18:18  hurdler
// Initial revision
//
// Revision 1.8  2002/09/25 15:17:39  vberghol
// Intermission fixed?
//
// Revision 1.7  2002/09/06 17:18:35  vberghol
// added most of the changes up to RC2
//
// Revision 1.6  2002/08/17 21:21:54  vberghol
// Only scripting to be fixed in engine!
//
// Revision 1.5  2002/07/23 19:21:45  vberghol
// fixed up to p_enemy.cpp
//
// Revision 1.4  2002/07/18 19:16:38  vberghol
// renamed a few files
//
// Revision 1.3  2002/07/01 21:00:39  jpakkane
// Fixed cr+lf to UNIX form.
//
// Revision 1.2  2002/06/28 10:57:19  vberghol
// Version 133 Experimental!
//
// Revision 1.13  2002/01/05 00:58:10  hurdler
// fix compiling problem when not using hwrender
//
// Revision 1.12  2001/12/31 14:44:50  hurdler
// Last fix for beta 4
//
// Revision 1.11  2001/12/31 13:47:46  hurdler
// Add setcorona FS command and prepare the code for beta 4
//
// Revision 1.10  2001/12/28 16:57:30  hurdler
// Add setcorona command to FS
//
// Revision 1.9  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.8  2001/08/14 00:36:26  hurdler
// Small update
//
// Revision 1.7  2001/08/06 23:57:10  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.6  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.5  2001/03/21 18:24:56  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.4  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.3  2000/11/09 17:56:20  stroggonmeth
// Hopefully fixed a few bugs and did a few optimizations.
//
// Revision 1.2  2000/11/04 16:23:44  bpereira
// no message
//
// Revision 1.1  2000/11/02 17:57:28  stroggonmeth
// FraggleScript files...
//
//
//--------------------------------------------------------------------------
//
// Functions
//
// functions are stored as variables(see variable.c), the
// value being a pointer to a 'handler' function for the
// function. Arguments are stored in an argc/argv-style list
//
// this module contains all the handler functions for the
// basic FraggleScript Functions.
//
// By Simon Howard
//
//---------------------------------------------------------------------------

/* includes ************************/

#include <stdio.h>

#include "doomtype.h"
#include "command.h"

#include "g_actor.h"
#include "g_pawn.h"
#include "g_player.h"

#include "d_main.h"
#include "d_netcmd.h"
#include "g_game.h"
#include "hu_stuff.h"

#include "info.h"
#include "m_random.h"

#include "p_spec.h"
#include "p_inter.h"

#include "r_data.h"
#include "r_main.h"
#include "r_state.h"
#include "r_segs.h"

#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

#include "p_camera.h"
#include "p_setup.h"

#include "i_video.h"

#include "t_parse.h"
#include "t_spec.h"
#include "t_script.h"
#include "t_oper.h"
#include "t_vari.h"
#include "t_func.h"

//extern int firstcolormaplump, lastcolormaplump;      // r_data.c

svalue_t evaluate_expression(int start, int stop);
int find_operator(int start, int stop, char *value);

// functions. SF_ means Script Function not, well.. heh, me

        /////////// actually running a function /////////////


/*******************
  FUNCTIONS
 *******************/

// the actual handler functions for the
// functions themselves

// arguments are evaluated and passed to the
// handler functions using 't_argc' and 't_argv'
// in a similar way to the way C does with command
// line options.

// values can be returned from the functions using
// the variable 't_return'

void SF_Print()
{
  int i;

  if(!t_argc) return;

  for(i=0; i<t_argc; i++)
    {
      CONS_Printf("%s", stringvalue(t_argv[i]));
    }
}

// return a random number from 0 to 255
void SF_Rnd()
{
  t_return.type = svt_int;
  t_return.value.i = rand() % 256;
}

// return a P_Random number from 0 to 255
void SF_PRnd()
{
  t_return.type = svt_int;
  t_return.value.i = P_Random();
}

// looping section. using the rover, find the highest level
// loop we are currently in and return the section_t for it.

section_t *looping_section()
{
  section_t *best = NULL;         // highest level loop we're in
                                  // that has been found so far
  int n;

  // check thru all the hashchains

  for(n=0; n<SECTIONSLOTS; n++)
    {
      section_t *current = current_script->sections[n];

      // check all the sections in this hashchain
      while(current)
        {
          // a loop?

          if(current->type == st_loop)
            // check to see if it's a loop that we're inside
            if(rover >= current->start && rover <= current->end)
              {
                // a higher nesting level than the best one so far?
                if(!best || (current->start > best->start))
                  best = current;     // save it
              }
          current = current->next;
        }
    }

  return best;    // return the best one found
}

        // "continue;" in FraggleScript is a function
void SF_Continue()
{
  section_t *section;

  if(!(section = looping_section()) )       // no loop found
    {
      script_error("continue() not in loop\n");
      return;
    }

  rover = section->end;      // jump to the closing brace
}

void SF_Break()
{
  section_t *section;

  if(!(section = looping_section()) )
    {
      script_error("break() not in loop\n");
      return;
    }

  rover = section->end+1;   // jump out of the loop
}

void SF_Goto()
{
  if(t_argc < 1)
    {
      script_error("incorrect arguments to goto\n");
      return;
    }

  // check argument is a labelptr

  if(t_argv[0].type != svt_label)
    {
      script_error("goto argument not a label\n");
      return;
    }

  // go there then if everythings fine

  rover = t_argv[0].value.labelptr;
}

void SF_Return()
{
  killscript = true;      // kill the script
}

void SF_Include()
{
  char tempstr[9];

  if(t_argc < 1)
    {
      script_error("incorrect arguments to include()");
      return;
    }

  memset(tempstr, 0, 9);

  if(t_argv[0].type == svt_string)
    strncpy(tempstr, t_argv[0].value.s, 8);
  else
    sprintf(tempstr, "%s", stringvalue(t_argv[0]));

  parse_include(tempstr);
}

void SF_Input()
{
/*        static char inputstr[128];

                gets(inputstr);

        t_return.type = svt_string;
        t_return.value.s = inputstr;
*/
  CONS_Printf("input() function not available in doom\a\n");
}

void SF_Beep()
{
  CONS_Printf("\3");
}

void SF_Clock()
{
  t_return.type = svt_int;
  t_return.value.i = (gametic*100)/35;
}

    /**************** doom stuff ****************/

void SF_ExitLevel()
{
  game.ExitLevel(0); // FIXME! lots of ways to exit!
}

        // centremsg
void SF_Tip()
{
  int     i;
  char    *tempstr;
  int     strsize = 0;

  // FIXME add player field in script_t
  /*
  if (current_script->trigger->Type() != Thinker::tt_ppawn)
    return;
  PlayerPawn *p = (PlayerPawn *)current_script->trigger;
  if (p->player != displayplayer)
    return;
  */
  // TODO: displayplayer2, tips to lower half of screen...
  if (current_script->player != displayplayer)
    return;

  for(i = 0; i < t_argc; i++)
    strsize += strlen(stringvalue(t_argv[i]));

  tempstr = (char *)Z_Malloc(strsize + 1, PU_STATIC, 0);
  tempstr[0] = '\0';

  for(i=0; i<t_argc; i++)
    sprintf(tempstr, "%s%s", tempstr, stringvalue(t_argv[i]));


  HU_SetTip(tempstr, 53);
  Z_Free(tempstr);
}


// SoM: Timed tip!
void SF_TimedTip()
{
  int     i;
  char    *tempstr;
  int     strsize = 0;

  if(t_argc < 2)
  {
    script_error("Missing parameters.\n");
    return;
  }

  if(current_script->player != displayplayer)
    return;

  int tiptime = (intvalue(t_argv[0]) * 35) / 100;

  for(i = 0; i < t_argc; i++)
    strsize += strlen(stringvalue(t_argv[i]));

  tempstr = (char *)Z_Malloc(strsize + 1, PU_STATIC, 0);
  tempstr[0] = '\0';

  for(i=1; i<t_argc; i++)
    sprintf(tempstr, "%s%s", tempstr, stringvalue(t_argv[i]));

  //CONS_Printf("%s\n", tempstr);
  HU_SetTip(tempstr, tiptime);
  Z_Free(tempstr);
}


// tip to a particular player
void SF_PlayerTip()
{
  int     i, plnum;
  char    *tempstr;
  int     strsize = 0;

  if(!t_argc)
    { script_error("player not specified\n"); return;}

  plnum = intvalue(t_argv[0]);

  if ((consoleplayer->number - 1) != plnum) return;

  for(i = 0; i < t_argc; i++)
    strsize += strlen(stringvalue(t_argv[i]));

  tempstr = (char *)Z_Malloc(strsize + 1, PU_STATIC, 0);
  tempstr[0] = '\0';

  for(i=1; i<t_argc; i++)
    sprintf(tempstr, "%s%s", tempstr, stringvalue(t_argv[i]));

  //CONS_Printf("%s\n", tempstr);
  HU_SetTip(tempstr, 53);
  Z_Free(tempstr);
}


// message player
void SF_Message()
{
  int     i;
  char    *tempstr;
  int     strsize = 0;

  if(current_script->player != displayplayer)
    return;

  for(i = 0; i < t_argc; i++)
    strsize += strlen(stringvalue(t_argv[i]));

  tempstr = (char *)Z_Malloc(strsize + 1, PU_STATIC, 0);
  tempstr[0] = '\0';

  for(i=0; i<t_argc; i++)
    sprintf(tempstr, "%s%s", tempstr, stringvalue(t_argv[i]));

  CONS_Printf("%s\n", tempstr);
  Z_Free(tempstr);
}

// Returns what type of game is going on - Deathmatch, CoOp, or Single Player.
// Feature Requested by SoM! SSNTails 06-13-2002
void SF_GameMode()
{
  t_return.type = svt_int;
  if(cv_deathmatch.value) // Deathmatch!
    t_return.value.i = 2;
  else if(game.netgame || game.multiplayer) // Cooperative
    t_return.value.i = 1;
  else // Single Player
    t_return.value.i = 0;
  return;
}


// message to a particular player
void SF_PlayerMsg()
{
  int     i, plnum;
  char    *tempstr;
  int     strsize = 0;

  if(!t_argc)
    { script_error("player not specified\n"); return;}

  plnum = intvalue(t_argv[0]);

  if(displayplayer->number != plnum) return;

  for(i = 0; i < t_argc; i++)
    strsize += strlen(stringvalue(t_argv[i]));

  tempstr = (char *)Z_Malloc(strsize + 1, PU_STATIC, 0);
  tempstr[0] = '\0';

  for(i=1; i<t_argc; i++)
    sprintf(tempstr, "%s%s", tempstr, stringvalue(t_argv[i]));

  CONS_Printf("%s\n", tempstr);
  Z_Free(tempstr);
}




void SF_PlayerInGame()
{
  if(!t_argc)
    { script_error("player not specified\n"); return;}

  t_return.type = svt_int;
  //t_return.value.i = playeringame[intvalue(t_argv[0])];
  // FIXME! this doesn't make sense anymore!
  t_return.value.i = (intvalue(t_argv[0]) < game.players.size()) ? true : false;
}




void SF_PlayerName()
{
  int plnum;
  PlayerInfo *pl;

  t_return.type = svt_string;

  if(!t_argc)
    {
      pl = current_script->player;
      if (pl)
	t_return.value.s = pl->name.c_str();
      else
        {
          script_error("script not started by player\n"); 
        }
      return;
    }
  else
    plnum = intvalue(t_argv[0]);

  t_return.type = svt_string;

  if (plnum < 0 || plnum >= game.maxplayers)
    pl = NULL;
  else
    pl = game.present[plnum];

  if (pl)
    t_return.value.s = pl->name.c_str();
}

        // object being controlled by player
void SF_PlayerObj()
{
  int plnum;
  PlayerInfo *pl;

  if(!t_argc)
    {
      pl = current_script->player;
      if (pl) //plnum = pl - players;
	t_return.value.mobj = pl->pawn;
      else
        {
          script_error("script not started by player\n");
        }
      return;
    }
  else
    plnum = intvalue(t_argv[0]);

  t_return.type = svt_mobj;

  if (plnum < 0 || plnum >= game.maxplayers)
    pl = NULL;
  else
    pl = game.present[plnum];

  if (pl)
    t_return.value.mobj = pl->pawn;
}


void SF_MobjIsPlayer()
{
  if (t_argc == 0)
    {
      t_return.type = svt_int;
      t_return.value.i = current_script->player ? 1 : 0;
      return;
    }

  Actor* mobj = MobjForSvalue(t_argv[0]);
  t_return.type = svt_int;
  if(!mobj)
    t_return.value.i = 0;
  else
    t_return.value.i = (mobj->Type() == Thinker::tt_ppawn) ? 1 : 0;
  return;
}


void SF_PlayerKeys()
{
  int  playernum;
  int  keynum;
  int  givetake;

  if(t_argc < 2)
  {
    script_error("missing parameters for playerkeys\n");
    return;
  }

  if(t_argc == 2)
  {
    if(t_argv[0].type == svt_mobj)
    {
      if(!t_argv[0].value.mobj->player)
      {
        script_error("mobj not a player!\n");
        return;
      }
      playernum = t_argv[0].value.mobj->player - players;
    }
    else
      playernum = intvalue(t_argv[0]);

    keynum = intvalue(t_argv[1]);
    if(!game.present[playernum])
    {
      script_error("player %i not in game\n", playernum);
      return;
    }
    if(keynum > 5)
    {
      script_error("keynum out of range! %s\n", keynum);
      return;
    }
    t_return.type = svt_int;
    t_return.value.i = players[playernum].cards & (1 << keynum);
    return;
  }
  else
  {
    if(t_argv[0].type == svt_mobj)
    {
      if(!t_argv[0].value.mobj->player)
      {
        script_error("mobj not a player!\n");
        return;
      }
      playernum = t_argv[0].value.mobj->player - players;
    }
    else
      playernum = intvalue(t_argv[0]);

    keynum = intvalue(t_argv[1]);
    if(!playeringame[playernum])
    {
      script_error("player %i not in game\n", playernum);
      return;
    }
    if(keynum > 6)
    {
      script_error("keynum out of range! %s\n", keynum);
      return;
    }
    givetake = intvalue(t_argv[2]);
    t_return.type = svt_int;
    if(givetake)
      players[playernum].cards |= (1 << keynum);
    else
      players[playernum].cards &= ~(1 << keynum);
    t_return.value.i = 0;
    return;
  }
}



void SF_PlayerAmmo()
{
  int  playernum;
  int  ammonum;
  int  newammo;

  if(t_argc < 2)
  {
    script_error("missing parameters for playerammo\n");
    return;
  }

  if(t_argc == 2)
  {
    if(t_argv[0].type == svt_mobj)
    {
      if(!t_argv[0].value.mobj->player)
      {
        script_error("mobj not a player!\n");
        return;
      }
      playernum = t_argv[0].value.mobj->player - players;
    }
    else
      playernum = intvalue(t_argv[0]);

    ammonum = intvalue(t_argv[1]);
    if(!playeringame[playernum])
    {
      script_error("player %i not in game\n", playernum);
      return;
    }
    if(ammonum >= NUMAMMO)
    {
      script_error("ammonum out of range! %s\n", ammonum);
      return;
    }
    t_return.type = svt_int;
    t_return.value.i = players[playernum].ammo[ammonum];
    return;
  }
  else
  {
    if(t_argv[0].type == svt_mobj)
    {
      if(!t_argv[0].value.mobj->player)
      {
        script_error("mobj not a player!\n");
        return;
      }
      playernum = t_argv[0].value.mobj->player - players;
    }
    else
      playernum = intvalue(t_argv[0]);

    ammonum = intvalue(t_argv[1]);
    if(!playeringame[playernum])
    {
      script_error("player %i not in game\n", playernum);
      return;
    }
    if(ammonum > NUMAMMO)
    {
      script_error("ammonum out of range! %s\n", ammonum);
      return;
    }
    newammo = intvalue(t_argv[2]);
    newammo = newammo > players[playernum].maxammo[ammonum] ? players[playernum].maxammo[ammonum] : newammo;
    t_return.type = svt_int;
    t_return.value.i = players[playernum].ammo[ammonum] = newammo;
    return;
  }
}



void SF_MaxPlayerAmmo()
{
  int  playernum;
  int  ammonum;
  int  newmax;

  if(t_argc < 2)
  {
    script_error("missing parameters for maxplayerammo\n");
    return;
  }

  if(t_argc == 2)
  {
    if(t_argv[0].type == svt_mobj)
    {
      if(!t_argv[0].value.mobj->player)
      {
        script_error("mobj not a player!\n");
        return;
      }
      playernum = t_argv[0].value.mobj->player - players;
    }
    else
      playernum = intvalue(t_argv[0]);

    ammonum = intvalue(t_argv[1]);
    if(!playeringame[playernum])
    {
      script_error("player %i not in game\n", playernum);
      return;
    }
    if(ammonum >= NUMAMMO || ammonum < 0)
    {
      script_error("maxammonum out of range! %i\n", ammonum);
      return;
    }
    t_return.type = svt_int;
    t_return.value.i = players[playernum].maxammo[ammonum];
    return;
  }
  else
  {
    if(t_argv[0].type == svt_mobj)
    {
      if(!t_argv[0].value.mobj->player)
      {
        script_error("mobj not a player!\n");
        return;
      }
      playernum = t_argv[0].value.mobj->player - players;
    }
    else
      playernum = intvalue(t_argv[0]);

    ammonum = intvalue(t_argv[1]);
    if(!playeringame[playernum])
    {
      script_error("player %i not in game\n", playernum);
      return;
    }
    if(ammonum > NUMAMMO)
    {
      script_error("ammonum out of range! %s\n", ammonum);
      return;
    }
    newmax = intvalue(t_argv[2]);
    t_return.type = svt_int;
    t_return.value.i = players[playernum].maxammo[ammonum] = newmax;
    return;
  }
}



extern void SF_StartScript();      // in t_script.c
extern void SF_ScriptRunning();
extern void SF_Wait();
extern void SF_TagWait();
extern void SF_ScriptWait();

/*********** Mobj code ***************/

void SF_Player()
{
  Actor *mo = t_argc ? MobjForSvalue(t_argv[0]) :
    current_script->trigger;

  t_return.type = svt_int;

  if(mo)
    {
      t_return.value.i = (int)(mo->player - players);
    }
  else
    {
      t_return.value.i = -1;
    }
}

        // spawn an object: type, x, y, [angle]
void SF_Spawn()
{
  int x, y, z, objtype;
  angle_t angle = 0;

  if(t_argc < 3)
    { script_error("insufficient arguments to function\n"); return; }

  objtype = intvalue(t_argv[0]);
  x = intvalue(t_argv[1]) << FRACBITS;
  y = intvalue(t_argv[2]) << FRACBITS;
  if(t_argc >= 5)
    z = intvalue(t_argv[4]) << FRACBITS;
  else
  {
    // SoM: Check thing flags for spawn-on-ceiling types...
    z = R_PointInSubsector(x, y)->sector->floorheight;
  }

  if(t_argc >= 4)
    angle = intvalue(t_argv[3]) * (ANG45 / 45);

  // invalid object to spawn
  if(objtype < 0 || objtype >= NUMMOBJTYPES)
    { script_error("unknown object type: %i\n", objtype); return; }

  t_return.type = svt_mobj;
  t_return.value.mobj = P_SpawnMobj(x,y,z, (mobjtype_t)objtype);
  t_return.value.mobj->angle = angle;
}


void SF_SpawnExplosion()
{
  int       type;
  fixed_t   x, y, z;
  Actor*   spawn;


  if(t_argc < 3)
  {
    script_error("SpawnExplosion: Missing arguments\n");
    return;
  }

  type = intvalue(t_argv[0]);
  if(type < 0 || type >= NUMMOBJTYPES)
  {
    script_error("SpawnExplosion: Invalud type number\n");
    return;
  }

  x = fixedvalue(t_argv[1]);
  y = fixedvalue(t_argv[2]);
  if(t_argc > 3)
    z = fixedvalue(t_argv[3]);
  else
    z = R_PointInSubsector(x, y)->sector->floorheight;

  spawn = P_SpawnMobj(x, y, z, (mobjtype_t)type);
  t_return.type = svt_int;
  t_return.value.i = P_SetMobjState(spawn, (statenum_t)spawn->info->deathstate);
  if(spawn->info->deathsound)
    S_StartSound(spawn, spawn->info->deathsound);
}



void SF_RemoveObj()
{
  Actor *mo;

  if(!t_argc)
    { script_error("insufficient arguments to function\n"); return; }

  mo = MobjForSvalue(t_argv[0]);
  if(mo)  // nullptr check
    P_RemoveMobj(mo);
}

void SF_KillObj()
{
  Actor *mo;

  if(t_argc)
    mo = MobjForSvalue(t_argv[0]);
  else
    mo = current_script->trigger;  // default to trigger object

  if(mo)  // nullptr check
    P_KillMobj(mo, NULL, current_script->trigger);         // kill it
}

        // mobj x, y, z
void SF_ObjX()
{
  Actor *mo = t_argc ? MobjForSvalue(t_argv[0]) : current_script->trigger;

  t_return.type = svt_fixed;
  t_return.value.f = mo ? mo->x : 0;   // null ptr check
}

void SF_ObjY()
{
  Actor *mo = t_argc ? MobjForSvalue(t_argv[0]) : current_script->trigger;

  t_return.type = svt_fixed;
  t_return.value.f = mo ? mo->y : 0; // null ptr check
}

void SF_ObjZ()
{
  Actor *mo = t_argc ? MobjForSvalue(t_argv[0]) : current_script->trigger;

  t_return.type = svt_fixed;
  t_return.value.f = mo ? mo->z : 0; // null ptr check
}

        // mobj angle
void SF_ObjAngle()
{
  Actor *mo = t_argc ? MobjForSvalue(t_argv[0]) : current_script->trigger;

  t_return.type = svt_fixed;
  t_return.value.f = mo ? AngleToFixed(mo->angle) : 0;   // null ptr check
}


        // teleport: object, sector_tag
void SF_Teleport()
{
  line_t line;    // dummy line for teleport function
  Actor *mo;

  if(t_argc==0)   // no arguments
    {
      script_error("insufficient arguments to function\n");
      return;
    }
  else if(t_argc == 1)    // 1 argument: sector tag
    {
      mo = current_script->trigger;   // default to trigger
      line.tag = intvalue(t_argv[0]);
    }
  else    // 2 or more
    {                       // teleport a given object
      mo = MobjForSvalue(t_argv[0]);
      line.tag = intvalue(t_argv[1]);
    }

  if(mo)
    EV_Teleport(&line, 0, mo);
}



void SF_SilentTeleport()
{
  line_t line;    // dummy line for teleport function
  Actor *mo;

  if(t_argc==0)   // no arguments
    {
      script_error("insufficient arguments to function\n");
      return;
    }
  else if(t_argc == 1)    // 1 argument: sector tag
    {
      mo = current_script->trigger;   // default to trigger
      line.tag = intvalue(t_argv[0]);
    }
  else    // 2 or more
    {                       // teleport a given object
      mo = MobjForSvalue(t_argv[0]);
      line.tag = intvalue(t_argv[1]);
    }

  if(mo)
    EV_SilentTeleport(&line, 0, mo);
}



void SF_DamageObj()
{
  Actor *mo;
  int damageamount;

  if(t_argc==0)   // no arguments
    {
      script_error("insufficient arguments to function\n");
      return;
    }
  else if(t_argc == 1)    // 1 argument: damage trigger by amount
    {
      mo = current_script->trigger;   // default to trigger
      damageamount = intvalue(t_argv[0]);
    }
  else    // 2 or more
    {                       // teleport a given object
      mo = MobjForSvalue(t_argv[0]);
      damageamount = intvalue(t_argv[1]);
    }

  if(mo)
    P_DamageMobj(mo, NULL, current_script->trigger, damageamount);
}

        // the tag number of the sector the thing is in
void SF_ObjSector()
{
  // use trigger object if not specified
  Actor *mo = t_argc ? MobjForSvalue(t_argv[0]) : current_script->trigger;

  t_return.type = svt_int;
  t_return.value.i = mo ? mo->subsector->sector->tag : 0; // nullptr check
}

        // the health number of an object
void SF_ObjHealth()
{
  // use trigger object if not specified
  Actor *mo = t_argc ? MobjForSvalue(t_argv[0]) : current_script->trigger;

  t_return.type = svt_int;
  t_return.value.i = mo ? mo->health : 0;
}


void SF_ObjDead()
{
  Actor *mo = t_argc ? MobjForSvalue(t_argv[0]) : current_script->trigger;

  t_return.type = svt_int;
  if(mo && (mo->health <= 0 || mo->flags & MF_CORPSE))
    t_return.value.i = 1;
  else
    t_return.value.i = 0;
}


void SF_ObjFlag()
{
  Actor *mo;
  int flagnum;

  if(t_argc==0)   // no arguments
    {
      script_error("no arguments for function\n");
      return;
    }
  else
    if(t_argc == 1)         // use trigger, 1st is flag
      {
        // use trigger:
        mo = current_script->trigger;
        flagnum = intvalue(t_argv[0]);
      }
    else
      if(t_argc == 2)
        {
          // specified object
          mo = MobjForSvalue(t_argv[0]);
          flagnum = intvalue(t_argv[1]);
        }
      else                     // >= 3 : SET flags
        {
          mo = MobjForSvalue(t_argv[0]);
          flagnum = intvalue(t_argv[1]);

          if(mo)          // nullptr check
            {
              long newflag;
              // remove old bit
              mo->flags = mo->flags & ~(1 << flagnum);

              // make the new flag
              newflag = (!!intvalue(t_argv[2])) << flagnum;
              mo->flags |= newflag;   // add new flag to mobj flags
            }

          //P_UpdateThinker(&mo->thinker);     // update thinker

        }

  t_return.type = svt_int;
  // nullptr check:
  t_return.value.i = mo ? !!(mo->flags & (1 << flagnum)) : 0;
}



        // apply momentum to a thing
void SF_PushThing()
{
  Actor *mo;
  angle_t angle;
  fixed_t force;

  if(t_argc<3)   // missing arguments
    {
      script_error("insufficient arguments for function\n");
      return;
    }

  mo = MobjForSvalue(t_argv[0]);

  if(!mo) return;

  angle = FixedToAngle(fixedvalue(t_argv[1]));
  force = fixedvalue(t_argv[2]);

  mo->px += FixedMul(finecosine[angle >> ANGLETOFINESHIFT], force);
  mo->py += FixedMul(finesine[angle >> ANGLETOFINESHIFT], force);
}



void SF_ReactionTime()
{
  Actor *mo;

  if(t_argc < 1)
  {
    script_error("no arguments for function\n");
    return;
  }

  mo = MobjForSvalue(t_argv[0]);
  if(!mo) return;

  if(t_argc > 1)
  {
    mo->reactiontime = (intvalue(t_argv[1]) * 35) / 100;
  }

  t_return.type = svt_int;
  t_return.value.i = mo->reactiontime;
}


// Sets a mobj's Target! >:)
void SF_MobjTarget()
{
  Actor*  mo;
  Actor*  target;

  if(t_argc < 1)
  {
    script_error("Missing parameters!\n");
    return;
  }

  mo = MobjForSvalue(t_argv[0]);
  if(!mo)
    return;

  if(t_argc >= 2)
  {
    if(t_argv[1].type != svt_mobj && intvalue(t_argv[1]) == -1)
    {
      // Set target to NULL
      mo->target = NULL;
      P_SetMobjState(mo, (statenum_t)mo->info->spawnstate);
    }
    else
    {
      target = MobjForSvalue(t_argv[1]);
      mo->target = target;
      P_SetMobjState(mo, (statenum_t)mo->info->seestate);
    }
  }

  t_return.type = svt_mobj;
  t_return.value.mobj = mo->target;
}


void SF_MobjMomx()
{
  Actor*   mo;

  if(t_argc < 1)
  {
    script_error("missing parameters\n");
    return;
  }

  mo = MobjForSvalue(t_argv[0]);
  if(t_argc > 1)
  {
    if(!mo)
      return;
    mo->px = fixedvalue(t_argv[1]);
  }

  t_return.type = svt_fixed;
  t_return.value.f = mo ? mo->px : 0;
}


void SF_MobjMomy()
{
  Actor*   mo;

  if(t_argc < 1)
  {
    script_error("missing parameters\n");
    return;
  }

  mo = MobjForSvalue(t_argv[0]);
  if(t_argc > 1)
  {
    if(!mo)
      return;
    mo->py = fixedvalue(t_argv[1]);
  }

  t_return.type = svt_fixed;
  t_return.value.f = mo ? mo->py : 0;
}


void SF_MobjMomz()
{
  Actor*   mo;

  if(t_argc < 1)
  {
    script_error("missing parameters\n");
    return;
  }

  mo = MobjForSvalue(t_argv[0]);
  if(t_argc > 1)
  {
    if(!mo)
      return;
    mo->pz = fixedvalue(t_argv[1]);
  }

  t_return.type = svt_fixed;
  t_return.value.f = mo ? mo->pz : 0;
}

/****************** Trig *********************/

void SF_PointToAngle()
{
  angle_t angle;
  int x1, y1, x2, y2;

  if(t_argc<4)
    {
      script_error("insufficient arguments to function\n");
      return;
    }

  x1 = intvalue(t_argv[0]) << FRACBITS;
  y1 = intvalue(t_argv[1]) << FRACBITS;
  x2 = intvalue(t_argv[2]) << FRACBITS;
  y2 = intvalue(t_argv[3]) << FRACBITS;

  angle = R_PointToAngle2(x1, y1, x2, y2);

  t_return.type = svt_fixed;
  t_return.value.f = AngleToFixed(angle);
}



void SF_PointToDist()
{
  int dist;
  int x1, x2, y1, y2;

  if(t_argc<4)
    {
      script_error("insufficient arguments to function\n");
      return;
    }

  x1 = intvalue(t_argv[0]) << FRACBITS;
  y1 = intvalue(t_argv[1]) << FRACBITS;
  x2 = intvalue(t_argv[2]) << FRACBITS;
  y2 = intvalue(t_argv[3]) << FRACBITS;

  dist = R_PointToDist2(x1, y1, x2, y2);
  t_return.type = svt_fixed;
  t_return.value.f = dist;
}




/************* Camera functions ***************/

camera_t script_camera = {false, 0, 0, 0, 0, NULL};
bool  script_camera_on;


// setcamera(obj, [angle], [viewheight], [aiming])
void SF_SetCamera()
{
  Actor     *mo;
  angle_t    angle;
  fixed_t    aiming;


  if(t_argc < 1)
    {
      script_error("insufficient arguments to function\n");
      return;
    }

  mo = MobjForSvalue(t_argv[0]);
  if(!mo) return;         // nullptr check

  if(script_camera.mo != mo)
  {
    if(script_camera.mo)
      script_camera.mo->angle = script_camera.startangle;

    script_camera.startangle = mo->angle;
  }

  angle = t_argc < 2 ? mo->angle : FixedToAngle(fixedvalue(t_argv[1]));

  script_camera.mo = mo;
  script_camera.mo->angle = angle;
  script_camera.mo->z = t_argc < 3 ? (mo->subsector->sector->floorheight + (41 << FRACBITS)) : fixedvalue(t_argv[2]);
  aiming = t_argc < 4 ? 0 : fixedvalue(t_argv[3]);
  script_camera.aiming = G_ClipAimingPitch(&aiming);
  G_ClipAimingPitch((int *)&script_camera.aiming);
  script_camera_on = true;
}




void SF_ClearCamera()
{
  script_camera_on = false;
  if(!script_camera.mo)
  {
    script_error("Clearcamera: called without setcamera.\n");
    return;
  }

  script_camera.mo->angle = script_camera.startangle;
  script_camera.mo = NULL;
}

// movecamera(cameraobj, targetobj, targetheight, movespeed, targetangle, anglespeed)
void SF_MoveCamera()
{
  fixed_t    x, y, z;
  fixed_t    xdist, ydist, zdist, xydist, movespeed;
  fixed_t    xstep, ystep, zstep, targetheight;
  angle_t    anglespeed, anglestep = 0, angledist, targetangle, mobjangle, bigangle, smallangle;
  // I have to use floats for the math where angles are divided by fixed
  // values.
  double     fangledist, fanglestep, fmovestep;
  int        angledir = 0;
  Actor*    camera;
  Actor*    target;
  int        moved = 0;
  int        quad1, quad2;

  if(t_argc < 6)
  {
    script_error("movecamera: insufficient arguments to function\n");
    return;
  }

  camera = MobjForSvalue(t_argv[0]);
  target = MobjForSvalue(t_argv[1]);
  targetheight = fixedvalue(t_argv[2]);
  movespeed = fixedvalue(t_argv[3]);
  targetangle = FixedToAngle(fixedvalue(t_argv[4]));
  anglespeed = FixedToAngle(fixedvalue(t_argv[5]));

  // Figure out how big the step will be
  xdist = target->x - camera->x;
  ydist = target->y - camera->y;
  zdist = targetheight - camera->z;

  // Angle checking...
  //    90
  //   Q1|Q0
  //180--+--0
  //   Q2|Q3
  //    270
  quad1 = targetangle / ANG90;
  quad2 = camera->angle / ANG90;
  bigangle = targetangle > camera->angle ? targetangle : camera->angle;
  smallangle = targetangle < camera->angle ? targetangle : camera->angle;
  if((quad1 > quad2 && quad1 - 1 == quad2) || (quad2 > quad1 && quad2 - 1 == quad1) || quad1 == quad2)
  {
    angledist = bigangle - smallangle;
    angledir = targetangle > camera->angle ? 1 : -1;
  }
  else
  {
    if(quad2 == 3 && quad1 == 0)
    {
      angledist = (bigangle + ANG180) - (smallangle + ANG180);
      angledir = 1;
    }
    else if(quad1 == 3 && quad2 == 0)
    {
      angledist = (bigangle + ANG180) - (smallangle + ANG180);
      angledir = -1;
    }
    else
    {
      angledist = bigangle - smallangle;
      if(angledist > ANG180)
      {
        angledist = (bigangle + ANG180) - (smallangle + ANG180);
        angledir = targetangle > camera->angle ? -1 : 1;
      }
      else
        angledir = targetangle > camera->angle ? 1 : -1;
    }
  }

  //CONS_Printf("angle: cam=%i, target=%i; dir: %i; quads: 1=%i, 2=%i\n", camera->angle / ANGLE_1, targetangle / ANGLE_1, angledir, quad1, quad2);
  // set the step variables based on distance and speed...
  mobjangle = R_PointToAngle2(camera->x, camera->y, target->x, target->y);

  if(movespeed)
  {
    xydist = R_PointToDist2(camera->x, camera->y, target->x, target->y);
    xstep = FixedMul(finecosine[mobjangle >> ANGLETOFINESHIFT], movespeed);
    ystep = FixedMul(finesine[mobjangle >> ANGLETOFINESHIFT], movespeed);
    if(xydist)
      zstep = FixedDiv(zdist, FixedDiv(xydist, movespeed));
    else
      zstep = zdist > 0 ? movespeed : -movespeed;

    if(xydist && !anglespeed)
    {
      fangledist = ((double)angledist / ANGLE_1);
      fmovestep = ((double)FixedDiv(xydist, movespeed) / FRACUNIT);
      if(fmovestep)
        fanglestep = (fangledist / fmovestep);
      else
        fanglestep = 360;

      //CONS_Printf("fstep: %f, fdist: %f, fmspeed: %f, ms: %i\n", fanglestep, fangledist, fmovestep, FixedDiv(xydist, movespeed) >> FRACBITS);

      anglestep = (fanglestep * ANGLE_1);
    }
    else
      anglestep = anglespeed;

    if(abs(xstep) >= (abs(xdist) - 1))
      x = target->x;
    else
    {
      x = camera->x + xstep;
      moved = 1;
    }

    if(abs(ystep) >= (abs(ydist) - 1))
      y = target->y;
    else
    {
      y = camera->y + ystep;
      moved = 1;
    }

    if(abs(zstep) >= abs(zdist) - 1)
      z = targetheight;
    else
    {
      z = camera->z + zstep;
      moved = 1;
    }
  }
  else
  {
    x = camera->x;
    y = camera->y;
    z = camera->z;
  }

  if(anglestep >= angledist)
    camera->angle = targetangle;
  else
  {
    if(angledir == 1)
    {
      moved = 1;
      camera->angle += anglestep;
    }
    else if(angledir == -1)
    {
      moved = 1;
      camera->angle -= anglestep;
    }
  }

  if((x != camera->x || y != camera->y) && !P_TryMove(camera, x, y, true))
  {
    script_error("Illegal camera move\n");
    return;
  }
  camera->z = z;

  t_return.type = svt_int;
  t_return.value.i = moved;
}




/*********** sounds ******************/

        // start sound from thing
void SF_StartSound()
{
  Actor *mo;

  if(t_argc < 2)
    {
      script_error("insufficient arguments to function\n");
      return;
    }

  if(t_argv[1].type != svt_string)
    {
      script_error("sound lump argument not a string!\n");
      return;
    }

  mo = MobjForSvalue(t_argv[0]);
  if(!mo) return;

  S_StartSoundName(mo, t_argv[1].value.s);
}




        // start sound from sector
void SF_StartSectorSound()
{
  sector_t *sector;
  int tagnum, secnum;

  if(t_argc < 2)
    { script_error("insufficient arguments to function\n"); return; }
  if(t_argv[1].type != svt_string)
    { script_error("sound lump argument not a string!\n"); return;}

  tagnum = intvalue(t_argv[0]);
  // argv is sector tag

  secnum = P_FindSectorFromTag(tagnum, -1);

  if(secnum < 0)
    { script_error("sector not found with tagnum %i\n", tagnum); return;}

  secnum = -1;
  while ((secnum = P_FindSectorFromTag(tagnum, secnum)) >= 0)
  {
    sector = &sectors[secnum];
    S_StartSoundName((Actor *)&sector->soundorg, t_argv[1].value.s);
  }
}


void SF_AmbiantSound() // FIXME it is supposed to read Ambient. Fix everywhere.
{
  if(t_argc != 1)
    { script_error("insufficient arguments to function\n"); return; }
  if(t_argv[0].type != svt_string)
    { script_error("sound lump argument not a string!\n"); return;}

  S_StartSoundName(NULL, t_argv[0].value.s);
}


/************* Sector functions ***************/

        // floor height of sector
void SF_FloorHeight()
{
  sector_t *sector;
  int tagnum;
  int secnum;
  int returnval = 1;

  if(!t_argc)
    { script_error("insufficient arguments to function\n"); return; }

  tagnum = intvalue(t_argv[0]);

  // argv is sector tag
  secnum = P_FindSectorFromTag(tagnum, -1);

  if(secnum < 0)
    { script_error("sector not found with tagnum %i\n", tagnum); return;}

  sector = &sectors[secnum];

  if(t_argc > 1)          // > 1: set floorheight
    {
      int i = -1;
      bool crush = t_argc == 3 ? intvalue(t_argv[2]) : false;

      // set all sectors with tag
      while ((i = P_FindSectorFromTag(tagnum, i)) >= 0)
        {
          //sectors[i].floorheight = intvalue(t_argv[1]) << FRACBITS;
          if(T_MovePlane(&sectors[i], abs(fixedvalue(t_argv[1]) - sectors[i].floorheight), fixedvalue(t_argv[1]), crush, 0, fixedvalue(t_argv[1]) > sectors[i].floorheight ? 1 : -1) == crushed)
            returnval = 0;
        }
    }
  else
    returnval = sectors[secnum].floorheight >> FRACBITS;

  // return floorheight

  t_return.type = svt_int;
  t_return.value.i = returnval;
}




void SF_MoveFloor()
{
  int secnum = -1;
  sector_t *sec;
  floormove_t *floor;
  int tagnum, platspeed = 1, destheight;

  if(t_argc < 2)
    { script_error("insufficient arguments to function\n"); return; }

  tagnum = intvalue(t_argv[0]);
  destheight = intvalue(t_argv[1]) << FRACBITS;
  platspeed = FLOORSPEED * (t_argc > 2 ? intvalue(t_argv[2]) : 1);

  // move all sectors with tag

  while ((secnum = P_FindSectorFromTag(tagnum, secnum)) >= 0)
    {
      sec = &sectors[secnum];

      // Don't start a second thinker on the same floor
      if (P_SectorActive(floor_special,sec))
        continue;

      floor = (floormove_t *)Z_Malloc(sizeof(floormove_t), PU_LEVSPEC, 0);
      P_AddThinker(&floor->thinker);
      sec->floordata = floor;
      floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
      floor->type = floor_e(-1);   // not done by line
      floor->crush = false;

      floor->direction =
        destheight < sec->floorheight ? -1 : 1;
      floor->sector = sec;
      floor->speed = platspeed;
      floor->floordestheight = destheight;
    }
}



        // ceiling height of sector
void SF_CeilingHeight()
{
  sector_t *sector;
  int secnum;
  int tagnum;
  int returnval = 1;

  if(!t_argc)
    { script_error("insufficient arguments to function\n"); return; }

  tagnum = intvalue(t_argv[0]);

  // argv is sector tag
  secnum = P_FindSectorFromTag(tagnum, -1);

  if(secnum < 0)
    { script_error("sector not found with tagnum %i\n", tagnum); return;}

  sector = &sectors[secnum];

  if(t_argc > 1)          // > 1: set ceilheight
    {
      int i = -1;
      bool crush = t_argc == 3 ? intvalue(t_argv[2]) : false;

      // set all sectors with tag
      while ((i = P_FindSectorFromTag(tagnum, i)) >= 0)
        {
          //sectors[i].ceilingheight = intvalue(t_argv[1]) << FRACBITS;
          if(T_MovePlane(&sectors[i], abs(fixedvalue(t_argv[1]) - sectors[i].ceilingheight), fixedvalue(t_argv[1]), crush, 1, fixedvalue(t_argv[1]) > sectors[i].ceilingheight ? 1 : -1) == crushed)
            returnval = 0;
        }
    }
  else
    returnval = sectors[secnum].ceilingheight >> FRACBITS;

  // return floorheight
  t_return.type = svt_int;
  t_return.value.i = returnval;
}




void SF_MoveCeiling()
{
  int secnum = -1;
  sector_t *sec;
  ceiling_t *ceiling;
  int tagnum, platspeed = 1, destheight;

  if(t_argc < 2)
    { script_error("insufficient arguments to function\n"); return; }

  tagnum = intvalue(t_argv[0]);
  destheight = intvalue(t_argv[1]) << FRACBITS;
  platspeed = FLOORSPEED * (t_argc > 2 ? intvalue(t_argv[2]) : 1);

  // move all sectors with tag

  while ((secnum = P_FindSectorFromTag(tagnum, secnum)) >= 0)
    {
      sec = &sectors[secnum];

      // Don't start a second thinker on the same floor
      if (P_SectorActive(ceiling_special,sec))
        continue;

      ceiling = (ceiling_t *)Z_Malloc(sizeof(*ceiling), PU_LEVSPEC, 0);
      P_AddThinker(&ceiling->thinker);
      sec->ceilingdata = ceiling;
      ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;
      ceiling->type = genCeiling;   // not done by line
      ceiling->crush = false;

      ceiling->direction =
        destheight < sec->ceilingheight ? -1 : 1;
      ceiling->sector = sec;
      ceiling->speed = platspeed;
      // just set top and bottomheight the same
      ceiling->topheight = ceiling->bottomheight = destheight;

      ceiling->tag = sec->tag;
      P_AddActiveCeiling(ceiling);
    }
}




void SF_LightLevel()
{
  sector_t *sector;
  int secnum;
  int tagnum;

  if(!t_argc)
    { script_error("insufficient arguments to function\n"); return; }

  tagnum = intvalue(t_argv[0]);

  // argv is sector tag
  secnum = P_FindSectorFromTag(tagnum, -1);

  if(secnum < 0)
    { script_error("sector not found with tagnum %i\n", tagnum); return;}

  sector = &sectors[secnum];

  if(t_argc > 1)          // > 1: set ceilheight
    {
      int i = -1;

      // set all sectors with tag
      while ((i = P_FindSectorFromTag(tagnum, i)) >= 0)
        {
          sectors[i].lightlevel = intvalue(t_argv[1]);
        }
    }

  // return lightlevel
  t_return.type = svt_int;
  t_return.value.i = sector->lightlevel;
}




void SF_FadeLight()
{
  int sectag, destlevel, speed = 1;

  if(t_argc < 2)
    { script_error("insufficient arguments to function\n"); return; }

  sectag = intvalue(t_argv[0]);
  destlevel = intvalue(t_argv[1]);
  speed = t_argc>2 ? intvalue(t_argv[2]) : 1;

  P_FadeLight(sectag, destlevel, speed);
}




void SF_FloorTexture()
{
  int tagnum, secnum;
  sector_t *sector;

  if(!t_argc)
    { script_error("insufficient arguments to function\n"); return; }

  tagnum = intvalue(t_argv[0]);

  // argv is sector tag
  secnum = P_FindSectorFromTag(tagnum, -1);

  if(secnum < 0)
    { script_error("sector not found with tagnum %i\n", tagnum); return;}

  sector = &sectors[secnum];

  if(t_argc > 1)
    {
      int i = -1;
      int picnum = R_FlatNumForName(t_argv[1].value.s);

      // set all sectors with tag
      while ((i = P_FindSectorFromTag(tagnum, i)) >= 0)
        {
          sectors[i].floorpic = picnum;
        }
    }

  t_return.type = svt_string;
  t_return.value.s = P_FlatNameForNum(sectors[secnum].floorpic);
}




void SF_SectorColormap()
{
  int tagnum, secnum;
  sector_t *sector;

  if(!t_argc)
    { script_error("insufficient arguments to function\n"); return; }

  tagnum = intvalue(t_argv[0]);

  // argv is sector tag
  secnum = P_FindSectorFromTag(tagnum, -1);

  if(secnum < 0)
    { script_error("sector not found with tagnum %i\n", tagnum); return;}

  sector = &sectors[secnum];

  if(t_argc > 1)
    {
      int i = -1;
      int mapnum = R_ColormapNumForName(t_argv[1].value.s);

      // set all sectors with tag
      while ((i = P_FindSectorFromTag(tagnum, i)) >= 0)
        {
          if(mapnum == -1)
            {
              sectors[i].midmap = 0;
              sectors[i].altheightsec = 0;
              sectors[i].heightsec = 0;
            }
          else
            {
              sectors[i].midmap = mapnum;
              sectors[i].altheightsec = 2;
              sectors[i].heightsec = 0;
            }
        }
    }

  t_return.type = svt_string;
  t_return.value.s = R_ColormapNameForNum(sector->midmap);
}




void SF_CeilingTexture()
{
  int tagnum, secnum;
  sector_t *sector;

  if(!t_argc)
    { script_error("insufficient arguments to function\n"); return; }

  tagnum = intvalue(t_argv[0]);

  // argv is sector tag
  secnum = P_FindSectorFromTag(tagnum, -1);

  if(secnum < 0)
    { script_error("sector not found with tagnum %i\n", tagnum); return;}

  sector = &sectors[secnum];

  if(t_argc > 1)
    {
      int i = -1;
      int picnum = R_FlatNumForName(t_argv[1].value.s);

      // set all sectors with tag
      while ((i = P_FindSectorFromTag(tagnum, i)) >= 0)
        {
          sectors[i].ceilingpic = picnum;
        }
    }

  t_return.type = svt_string;
  t_return.value.s = P_FlatNameForNum(sectors[secnum].ceilingpic);
}




void SF_ChangeHubLevel()
{
/*  int tagnum;

  if(!t_argc)
    {
      script_error("hub level to go to not specified!\n");
      return;
    }
  if(t_argv[0].type != svt_string)
    {
      script_error("level argument is not a string!\n");
      return;
    }

  // second argument is tag num for 'seamless' travel
  if(t_argc > 1)
    tagnum = intvalue(t_argv[1]);
  else
    tagnum = -1;

  P_SavePlayerPosition(current_script->trigger->player, tagnum);
  P_ChangeHubLevel(t_argv[0].value.s);*/
}



// for start map: start new game on a particular skill
void SF_StartSkill()
{
  skill_t skill;

  if(t_argc < 1)
    { script_error("need skill level to start on\n"); return;}

  // -1: 1-5 is how we normally see skills
  // 0-4 is how doom sees them

  skill = skill_t(intvalue(t_argv[0]) - 1);

  G_DeferedInitNew(skill, G_BuildMapName(1,1), false);
}




//////////////////////////////////////////////////////////////////////////
//
// Doors
//

// opendoor(sectag, [delay], [speed])

void SF_OpenDoor()
{
  int speed, wait_time;
  int sectag;

  if(t_argc < 1)
    {
      script_error("need sector tag for door to open\n");
      return;
    }

  // got sector tag
  sectag = intvalue(t_argv[0]);

  // door wait time

  if(t_argc > 1)    // door wait time
    wait_time = (intvalue(t_argv[1]) * 35) / 100;
  else
    wait_time = 0;  // 0= stay open

  // door speed

  if(t_argc > 2)
    speed = intvalue(t_argv[2]);
  else
    speed = 1;    // 1= normal speed

  EV_OpenDoor(sectag, speed, wait_time);
}




void SF_CloseDoor()
{
  int speed;
  int sectag;

  if(t_argc < 1)
    {
      script_error("need sector tag for door to open\n");
      return;
    }

  // got sector tag
  sectag = intvalue(t_argv[0]);

  // door speed

  if(t_argc > 1)
    speed = intvalue(t_argv[1]);
  else
    speed = 1;    // 1= normal speed

  EV_CloseDoor(sectag, speed);
}



// run console cmd

void SF_RunCommand()
{
  int     i;
  char    *tempstr;
  int     strsize = 0;

  for(i = 0; i < t_argc; i++)
    strsize += strlen(stringvalue(t_argv[i]));

  tempstr = (char *)Z_Malloc(strsize + 1, PU_STATIC, 0);
  tempstr[0] = '\0';

  for(i=0; i<t_argc; i++)
    sprintf(tempstr,"%s%s", tempstr, stringvalue(t_argv[i]));

  COM_BufAddText(tempstr);
  Z_Free(tempstr);
}




// any linedef type

void SF_LineTrigger()
{
  line_t junk;

  if(!t_argc)
    {
      script_error("need line trigger type\n");
      return;
    }

  junk.special = intvalue(t_argv[0]);
  junk.tag = t_argc < 2 ? 0 : intvalue(t_argv[1]);

  P_UseSpecialLine(t_trigger, &junk, 0);    // Try using it
  P_ActivateCrossedLine(&junk, 0, t_trigger);   // Try crossing it
}


void SF_LineFlag()
{
  line_t*  line;
  int      linenum;
  int      flagnum;

  if(t_argc < 2)
  {
    script_error("LineFlag: missing parameters\n");
    return;
  }

  linenum = intvalue(t_argv[0]);
  if(linenum < 0 || linenum > numlines)
  {
    script_error("LineFlag: Invalid line number.\n");
    return;
  }

  line = lines + linenum;

  flagnum = intvalue(t_argv[1]);
  if(flagnum < 0 || flagnum > 32)
  {
    script_error("LineFlag: Invalid flag number.\n");
    return;
  }

  if(t_argc > 2)
  {
    line->flags &= ~(1 << flagnum);
    if(intvalue(t_argv[2]))
      line->flags |= (1 << flagnum);
  }

  t_return.type = svt_int;
  t_return.value.i = line->flags & (1 << flagnum);
}


void SF_ChangeMusic()
{
  if(!t_argc)
    {
      script_error("need new music name\n");
      return;
    }
  if(t_argv[0].type != svt_string)
    { script_error("incorrect argument to function\n"); return;}

  S_ChangeMusicName(t_argv[0].value.s, 1);
}



// SoM: Max and Min math functions.
void SF_Max()
{
  fixed_t   n1, n2;

  if(t_argc != 2)
  {
    script_error("invalid number of arguments\n");
    return;
  }

  n1 = fixedvalue(t_argv[0]);
  n2 = fixedvalue(t_argv[1]);

  t_return.type = svt_fixed;
  t_return.value.f = n1 > n2 ? n1 : n2;
}



void SF_Min()
{
  fixed_t   n1, n2;

  if(t_argc != 2)
  {
    script_error("invalid number of arguments\n");
    return;
  }

  n1 = fixedvalue(t_argv[0]);
  n2 = fixedvalue(t_argv[1]);

  t_return.type = svt_fixed;
  t_return.value.f = n1 < n2 ? n1 : n2;
}


void SF_Abs()
{
  fixed_t   n1;

  if(t_argc != 1)
  {
    script_error("invalid number of arguments\n");
    return;
  }

  n1 = fixedvalue(t_argv[0]);

  t_return.type = svt_fixed;
  t_return.value.f = n1 < 0 ? n1 * -1 : n1;
}


//Hurdler: some new math functions
fixed_t double2fixed(double t)
{
  double fl = floor(t);
  return ((int)fl << 16) | (int)((t-fl)*65536.0);
}

void SF_Sin()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = double2fixed(sin(FIXED_TO_FLOAT(n1)));
    }
}

void SF_ASin()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = double2fixed(asin(FIXED_TO_FLOAT(n1)));
    }
}

void SF_Cos()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = double2fixed(cos(FIXED_TO_FLOAT(n1)));
    }
}

void SF_ACos()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = double2fixed(acos(FIXED_TO_FLOAT(n1)));
    }
}

void SF_Tan()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = double2fixed(tan(FIXED_TO_FLOAT(n1)));
    }
}

void SF_ATan()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = double2fixed(atan(FIXED_TO_FLOAT(n1)));
    }
}

void SF_Exp()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = double2fixed(exp(FIXED_TO_FLOAT(n1)));
    }
}

void SF_Log()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = double2fixed(log(FIXED_TO_FLOAT(n1)));
    }
}

void SF_Sqrt()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = double2fixed(sqrt(FIXED_TO_FLOAT(n1)));
    }
}

void SF_Floor()
{
  if(t_argc != 1)
    {
      script_error("invalid number of arguments\n");
    }
  else
    {
      fixed_t n1 = fixedvalue(t_argv[0]);
      t_return.type = svt_fixed;
      t_return.value.f = n1 & 0xffFF0000;
    }
}

void SF_Pow()
{
  fixed_t   n1, n2;
  if(t_argc != 2)
    {
      script_error("invalid number of arguments\n");
      return;
    }
  n1 = fixedvalue(t_argv[0]);
  n2 = fixedvalue(t_argv[1]);
  t_return.type = svt_fixed;
  t_return.value.f = double2fixed(pow(FIXED_TO_FLOAT(n1), FIXED_TO_FLOAT(n2)));   
}



//////////////////////////////////////////////////////////////////////////
// FraggleScript HUD graphics
//////////////////////////////////////////////////////////////////////////
int  HU_GetFSPic(int lumpnum, int xpos, int ypos);
int  HU_DeleteFSPic(int handle);
int  HU_ModifyFSPic(int handle, int lumpnum, int xpos, int ypos);
int  HU_FSDisplay(int handle, bool newval);


void SF_NewHUPic()
{
  if(t_argc != 3)
  {
    script_error("newhupic: invalid number of arguments\n");
    return;
  }

  t_return.type = svt_int;
  t_return.value.i = HU_GetFSPic(W_GetNumForName(stringvalue(t_argv[0])), intvalue(t_argv[1]), intvalue(t_argv[2]));
  return;
}


void SF_DeleteHUPic()
{
  if(t_argc != 1)
  {
    script_error("deletehupic: Invalid number if arguments\n");
    return;
  }

  if(HU_DeleteFSPic(intvalue(t_argv[0])) == -1)
    script_error("deletehupic: Invalid sfpic handle: %i\n", intvalue(t_argv[0]));
  return;
}


void SF_ModifyHUPic()
{
  if(t_argc != 4)
  {
    script_error("modifyhupic: invalid number of arguments\n");
    return;
  }

  if(HU_ModifyFSPic(intvalue(t_argv[0]), W_GetNumForName(stringvalue(t_argv[1])), intvalue(t_argv[2]), intvalue(t_argv[3])) == -1)
    script_error("modifyhypic: invalid sfpic handle %i\n", intvalue(t_argv[0]));
  return;
}



void SF_SetHUPicDisplay()
{
  if(t_argc != 2)
  {
    script_error("sethupicdisplay: invalud number of arguments\n");
    return;
  }

  if(HU_FSDisplay(intvalue(t_argv[0]), intvalue(t_argv[1]) > 0 ? 1 : 0) == -1)
    script_error("sethupicdisplay: invalid pic handle %i\n", intvalue(t_argv[0]));
}


// Hurdler: I'm enjoying FS capability :)

#ifdef HWRENDER
extern light_t lspr[];

int String2Hex(char *s)
{
    #define HEX2INT(x) (x >= '0' && x <= '9' ? x - '0' : x >= 'a' && x <= 'f' ? x - 'a' + 10 : x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0)
    return (HEX2INT(s[0]) <<  4) + (HEX2INT(s[1]) <<  0) +
           (HEX2INT(s[2]) << 12) + (HEX2INT(s[3]) <<  8) +
           (HEX2INT(s[4]) << 20) + (HEX2INT(s[5]) << 16) + 
           (HEX2INT(s[6]) << 28) + (HEX2INT(s[7]) << 24);
    #undef HEX2INT
}

void SF_SetCorona()
{
    if (rendermode == render_soft)
        return; // do nothing in software mode
    if (t_argc != 3 && t_argc != 7)
    {
        script_error("Incorrect parameters.\n");
        return;
    }
    //this function accept 2 kinds of parameters
    if (t_argc == 3)
    {
        int    num  = t_argv[0].value.i; // which corona we want to modify
        int    what = t_argv[1].value.i; // what we want to modify (type, color, offset,...)
        int    ival = t_argv[2].value.i; // the value of what we modify
        double fval = ((double)t_argv[3].value.f / FRACUNIT);

        switch (what)
        {
        case 0: lspr[num].type = ival; break;
        case 1: lspr[num].light_xoffset = fval; break;
        case 2: lspr[num].light_yoffset = fval; break;
        case 3: 
            if (t_argv[2].type == svt_string)
                lspr[num].corona_color = String2Hex(t_argv[2].value.s);
            else
                memcpy(&lspr[num].corona_color, &ival, sizeof(int));
            break;
        case 4: lspr[num].corona_radius = fval; break;
        case 5: 
            if (t_argv[2].type == svt_string)
                lspr[num].dynamic_color = String2Hex(t_argv[2].value.s);
            else
                memcpy(&lspr[num].dynamic_color, &ival, sizeof(int));
            break;
        case 6: 
            lspr[num].dynamic_radius = fval; 
            lspr[num].dynamic_sqrradius = sqrt(lspr[num].dynamic_radius); 
            break;
        default: CONS_Printf("Error in setcorona\n"); break;
        }
    }
    else
    {
        int num = t_argv[0].value.i; // which corona we want to modify
        lspr[num].type          = t_argv[1].value.i;
        lspr[num].light_xoffset = t_argv[2].value.f;
        lspr[num].light_yoffset = t_argv[3].value.f;
        if (t_argv[4].type == svt_string)
            lspr[num].corona_color = String2Hex(t_argv[4].value.s);
        else
            memcpy(&lspr[num].corona_color, &t_argv[4].value.i, sizeof(int));
        lspr[num].corona_radius = t_argv[5].value.f;
        if (t_argv[6].type == svt_string)
            lspr[num].dynamic_color = String2Hex(t_argv[6].value.s);
        else
            memcpy(&lspr[num].dynamic_color, &t_argv[6].value.i, sizeof(int));
        lspr[num].dynamic_radius = t_argv[7].value.f;
        lspr[num].dynamic_sqrradius = sqrt(lspr[num].dynamic_radius); 
    }
}
#endif

//////////////////////////////////////////////////////////////////////////
//
// Init Functions
//

//extern int fov; // r_main.c
int fov;

void init_functions()
{
  // add all the functions
  add_game_int("consoleplayer", &consoleplayer);
  add_game_int("displayplayer", &displayplayer);
  add_game_int("fov", &fov);
  add_game_int("zoom", &fov); //SoM: BAKWARDS COMPATABILITY!
  add_game_mobj("trigger", &trigger_obj);

  // important C-emulating stuff
  new_function("break", SF_Break);
  new_function("continue", SF_Continue);
  new_function("return", SF_Return);
  new_function("goto", SF_Goto);
  new_function("include", SF_Include);

  // standard FraggleScript functions
  new_function("print", SF_Print);
  new_function("rnd", SF_Rnd);
  new_function("prnd", SF_PRnd);
  new_function("input", SF_Input); // TODO: document
  new_function("beep", SF_Beep);
  new_function("clock", SF_Clock);
  new_function("wait", SF_Wait);
  new_function("tagwait", SF_TagWait);
  new_function("scriptwait", SF_ScriptWait);
  new_function("startscript", SF_StartScript);
  new_function("scriptrunning", SF_ScriptRunning);

  // doom stuff
  new_function("startskill", SF_StartSkill);
  new_function("exitlevel", SF_ExitLevel);
  new_function("tip", SF_Tip);
  new_function("timedtip", SF_TimedTip);
  new_function("message", SF_Message);
  new_function("gamemode", SF_GameMode); // SoM Request SSNTails 06-13-2002

  // player stuff
  new_function("playermsg", SF_PlayerMsg);
  new_function("playertip", SF_PlayerTip);
  new_function("playeringame", SF_PlayerInGame);
  new_function("playername", SF_PlayerName);
  new_function("playerobj", SF_PlayerObj);
  new_function("isobjplayer", SF_MobjIsPlayer);
  new_function("isplayerobj", SF_MobjIsPlayer); // backward compatibility
  new_function("playerkeys", SF_PlayerKeys);
  new_function("playerammo", SF_PlayerAmmo);
  new_function("maxplayerammo", SF_MaxPlayerAmmo);

  // mobj stuff
  new_function("spawn", SF_Spawn);
  new_function("spawnexplosion", SF_SpawnExplosion);
  new_function("kill", SF_KillObj);
  new_function("removeobj", SF_RemoveObj);
  new_function("objx", SF_ObjX);
  new_function("objy", SF_ObjY);
  new_function("objz", SF_ObjZ);
  new_function("teleport", SF_Teleport);
  new_function("silentteleport", SF_SilentTeleport);
  new_function("damageobj", SF_DamageObj);
  new_function("player", SF_Player);
  new_function("objsector", SF_ObjSector);
  new_function("objflag", SF_ObjFlag);
  new_function("pushobj", SF_PushThing);
  new_function("pushthing", SF_PushThing); // backward compatibility
  new_function("objangle", SF_ObjAngle);
  new_function("objhealth", SF_ObjHealth);
  new_function("objdead", SF_ObjDead);
  new_function("objreactiontime", SF_ReactionTime);
  new_function("reactiontime", SF_ReactionTime); // backward compatibility
  new_function("objtarget", SF_MobjTarget);
  new_function("objmomx", SF_MobjMomx);
  new_function("objmomy", SF_MobjMomy);
  new_function("objmomz", SF_MobjMomz);

  // sector stuff
  new_function("floorheight", SF_FloorHeight);
  new_function("floortext", SF_FloorTexture);
  new_function("floortexture", SF_FloorTexture); // b comp.
  new_function("movefloor", SF_MoveFloor);
  new_function("ceilheight", SF_CeilingHeight);
  new_function("ceilingheight", SF_CeilingHeight); // b comp.
  new_function("moveceil", SF_MoveCeiling);
  new_function("moveceiling", SF_MoveCeiling); // b comp.
  new_function("ceiltext", SF_CeilingTexture);
  new_function("ceilingtexture", SF_CeilingTexture); // b comp.
  new_function("lightlevel", SF_LightLevel);
  new_function("fadelight", SF_FadeLight);
  new_function("colormap", SF_SectorColormap);

  // cameras!
  new_function("setcamera", SF_SetCamera);
  new_function("clearcamera", SF_ClearCamera);
  new_function("movecamera", SF_MoveCamera);

  // trig functions
  new_function("pointtoangle", SF_PointToAngle);
  new_function("pointtodist", SF_PointToDist);

  // sound functions
  new_function("startsound", SF_StartSound);
  new_function("startsectorsound", SF_StartSectorSound);
  new_function("startambiantsound", SF_AmbiantSound);
  new_function("ambientsound", SF_AmbiantSound); // b comp.
  new_function("changemusic", SF_ChangeMusic);

  // hubs!
  new_function("changehublevel", SF_ChangeHubLevel); // TODO: document

  // doors
  new_function("opendoor", SF_OpenDoor);
  new_function("closedoor", SF_CloseDoor);

  new_function("runcommand", SF_RunCommand);
  new_function("linetrigger", SF_LineTrigger);
  new_function("lineflag", SF_LineFlag); // TODO: document

  new_function("max", SF_Max);
  new_function("min", SF_Min);
  new_function("abs", SF_Abs);

  //Hurdler: new math functions
  new_function("sin", SF_Sin);
  new_function("asin", SF_ASin);
  new_function("cos", SF_Cos);
  new_function("acos", SF_ACos);
  new_function("tan", SF_Tan);
  new_function("atan", SF_ATan);
  new_function("exp", SF_Exp);
  new_function("log", SF_Log);
  new_function("sqrt", SF_Sqrt);
  new_function("floor", SF_Floor);
  new_function("pow", SF_Pow);

  // HU Graphics
  new_function("newhupic", SF_NewHUPic);
  new_function("createpic", SF_NewHUPic);
  new_function("deletehupic", SF_DeleteHUPic);
  new_function("modifyhupic", SF_ModifyHUPic);
  new_function("modifypic", SF_ModifyHUPic);
  new_function("sethupicdisplay", SF_SetHUPicDisplay);
  new_function("setpicvisible", SF_SetHUPicDisplay);

  // Hurdler's stuff :)
#ifdef HWRENDER
  new_function("setcorona", SF_SetCorona);
#endif
}

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2002/11/16 14:18:18  hurdler
// Initial revision
//
// Revision 1.8  2002/09/25 15:17:39  vberghol
// Intermission fixed?
//
// Revision 1.7  2002/09/06 17:18:35  vberghol
// added most of the changes up to RC2
//
// Revision 1.6  2002/08/17 21:21:54  vberghol
// Only scripting to be fixed in engine!
//
// Revision 1.5  2002/07/23 19:21:45  vberghol
// fixed up to p_enemy.cpp
//
// Revision 1.4  2002/07/18 19:16:38  vberghol
// renamed a few files
//
// Revision 1.3  2002/07/01 21:00:39  jpakkane
// Fixed cr+lf to UNIX form.
//
// Revision 1.2  2002/06/28 10:57:19  vberghol
// Version 133 Experimental!
//
// Revision 1.13  2002/01/05 00:58:10  hurdler
// fix compiling problem when not using hwrender
//
// Revision 1.12  2001/12/31 14:44:50  hurdler
// Last fix for beta 4
//
// Revision 1.11  2001/12/31 13:47:46  hurdler
// Add setcorona FS command and prepare the code for beta 4
//
// Revision 1.10  2001/12/28 16:57:30  hurdler
// Add setcorona command to FS
//
// Revision 1.9  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.8  2001/08/14 00:36:26  hurdler
// Small update
//
// Revision 1.7  2001/08/06 23:57:10  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.6  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.5  2001/03/21 18:24:56  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.4  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.3  2000/11/09 17:56:20  stroggonmeth
// Hopefully fixed a few bugs and did a few optimizations.
//
// Revision 1.2  2000/11/04 16:23:44  bpereira
// no message
//
// Revision 1.1  2000/11/02 17:57:28  stroggonmeth
// FraggleScript files...
//
// Revision 1.1.1.1  2000/04/30 19:12:08  fraggle
// initial import
//
//
//---------------------------------------------------------------------------
