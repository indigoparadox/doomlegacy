// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by Raven Software, Corp.
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
// Revision 1.20  2004/01/06 14:37:45  smite-meister
// six bugfixes, cleanup
//
// Revision 1.19  2004/01/02 14:25:01  smite-meister
// cleanup
//
// Revision 1.18  2003/12/31 18:32:50  smite-meister
// Last commit of the year? Sound works.
//
// Revision 1.17  2003/12/06 23:57:47  smite-meister
// save-related bugfixes
//
// Revision 1.16  2003/12/03 10:49:49  smite-meister
// Save/load bugfix, text strings updated
//
// Revision 1.15  2003/11/23 00:41:55  smite-meister
// bugfixes
//
// Revision 1.14  2003/11/12 11:07:21  smite-meister
// Serialization done. Map progression.
//
// Revision 1.13  2003/05/11 21:23:50  smite-meister
// Hexen fixes
//
// Revision 1.12  2003/05/05 00:24:49  smite-meister
// Hexen linedef system. Pickups.
//
// Revision 1.11  2003/04/19 17:38:47  smite-meister
// SNDSEQ support, tools, linedef system...
//
// Revision 1.10  2003/04/14 08:58:26  smite-meister
// Hexen maps load.
//
// Revision 1.9  2003/04/04 00:01:56  smite-meister
// bugfixes, Hexen HUD
//
// Revision 1.8  2003/03/23 14:24:13  smite-meister
// Polyobjects, MD3 models
//
// Revision 1.7  2003/03/15 20:07:16  smite-meister
// Initial Hexen compatibility!
//
// Revision 1.6  2003/03/08 16:07:07  smite-meister
// Lots of stuff. Sprite cache. Movement+friction fix.
//
// Revision 1.5  2003/02/16 16:54:51  smite-meister
// L2 sound cache done
//
// Revision 1.4  2003/01/12 12:56:40  smite-meister
// Texture bug finally fixed! Pickup, chasecam and sw renderer bugs fixed.
//
// Revision 1.3  2002/12/23 23:15:41  smite-meister
// Weapon groups, MAPINFO parser added!
//
// Revision 1.2  2002/12/16 22:11:35  smite-meister
// Actor/DActor separation done!
//
// Revision 1.1.1.1  2002/11/16 14:17:58  hurdler
// Initial C++ version of Doom Legacy
//
//
// DESCRIPTION:
//   Heretic/Hexen specific extra game routines, gametype patching
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "g_actor.h"
#include "g_pawn.h"
#include "g_map.h"
#include "g_player.h"

#include "p_spec.h"
#include "p_enemy.h"
#include "p_maputl.h"
#include "r_main.h"
#include "sounds.h"
#include "m_random.h"
#include "dstrings.h"
#include "p_heretic.h"
#include "wi_stuff.h"
#include "tables.h"

//---------------------------------------------------------------------------
// P_MinotaurSlam

void P_MinotaurSlam(Actor *source, Actor *target)
{
  angle_t angle;
  fixed_t thrust;
    
  angle = R_PointToAngle2(source->x, source->y, target->x, target->y);
  angle >>= ANGLETOFINESHIFT;
  thrust = 16*FRACUNIT+(P_Random()<<10);
  target->px += FixedMul(thrust, finecosine[angle]);
  target->py += FixedMul(thrust, finesine[angle]);
  target->Damage(NULL, NULL, HITDICE(6));

  /*
  if(target->player)
    {
      target->reactiontime = 14+(P_Random()&7);
    }
  */
}

//---------------------------------------------------------------------------
// P_TouchWhirlwind

bool P_TouchWhirlwind(Actor *target)
{
  int randVal;
    
  target->angle += P_SignedRandom()<<20;
  target->px += P_SignedRandom()<<10;
  target->py += P_SignedRandom()<<10;
  if (target->mp->maptic & 16 && !(target->flags2 & MF2_BOSS))
    {
      randVal = P_Random();
      if(randVal > 160)
        {
	  randVal = 160;
        }
      target->pz += randVal<<10;
      if(target->pz > 12*FRACUNIT)
        {
	  target->pz = 12*FRACUNIT;
        }
    }
  if(!(target->mp->maptic & 7))
    {
      return target->Damage(NULL, NULL, 3);
    }
  return false;
}


//----------------------------------------------------------------------------
//
// was P_FaceMobj
//
// Returns 1 if 'source' needs to turn clockwise, or 0 if 'source' needs
// to turn counter clockwise.  'delta' is set to the amount 'source'
// needs to turn.
//
//----------------------------------------------------------------------------
int P_FaceMobj(Actor *source, Actor *target, angle_t *delta)
{
  angle_t diff;
  angle_t angle1;
  angle_t angle2;

  angle1 = source->angle;
  angle2 = R_PointToAngle2(source->x, source->y, target->x, target->y);
  if(angle2 > angle1)
    {
      diff = angle2-angle1;
      if(diff > ANG180)
	{
	  *delta = ANGLE_MAX-diff;
	  return(0);
	}
      else
	{
	  *delta = diff;
	  return(1);
	}
    }
  else
    {
      diff = angle1-angle2;
      if(diff > ANG180)
	{
	  *delta = ANGLE_MAX-diff;
	  return(1);
	}
      else
	{
	  *delta = diff;
	  return(0);
	}
    }
}

//----------------------------------------------------------------------------
//
// was P_SeekerMissile
//
// Returns true if target was tracked, false if not.
//
//----------------------------------------------------------------------------

bool DActor::SeekerMissile(angle_t thresh, angle_t turnMax)
{
  int dir;
  angle_t delta;
  angle_t ang;

  Actor *t = target;

  if (t == NULL)
    return false;
   
  if (!(t->flags & MF_SHOOTABLE))
    { // Target died
      target = NULL;
      return false;
    }
  dir = P_FaceMobj(this, t, &delta);
  if (delta > thresh)
    {
      delta >>= 1;
      if (delta > turnMax)
	delta = turnMax;
    }
  if (dir)
    { // Turn clockwise
      angle += delta;
    }
  else
    { // Turn counter clockwise
      angle -= delta;
    }
  ang = angle>>ANGLETOFINESHIFT;
  px = int(info->speed * finecosine[ang]);
  py = int(info->speed * finesine[ang]);
  if (z+height < t->z || t->z+t->height < z)
    { // Need to seek vertically
      int dist = P_AproxDistance(t->x-x, t->y-y);
      dist = dist / int(info->speed * FRACUNIT);
      if (dist < 1)
	dist = 1;
      pz = (t->z+(t->height>>1) - (z+(height>>1))) / dist;
    }
  return true;
}

//---------------------------------------------------------------------------
//
// was P_SpawnMissileAngle
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a Actor pointer to the missile.
//
//---------------------------------------------------------------------------

DActor *DActor::SpawnMissileAngle(mobjtype_t t, angle_t angle, fixed_t momz)
{
  fixed_t mz;

  switch(t)
    {
    case MT_MNTRFX1: // Minotaur swing attack missile
      mz = z+40*FRACUNIT;
      break;
    case MT_MNTRFX2: // Minotaur floor fire missile
      mz = ONFLOORZ; // +floorclip; 
      break;
    case MT_SRCRFX1: // Sorcerer Demon fireball
      mz = z+48*FRACUNIT;
      break;
    case MT_ICEGUY_FX2: // Secondary Projectiles of the Ice Guy
      mz = z+3*FRACUNIT;
      break;
    case MT_MSTAFF_FX2:
      mz = z+40*FRACUNIT;
      break;
    default:
      mz = z+32*FRACUNIT;
      break;

    }

  mz -= floorclip;
    
  DActor *mo = mp->SpawnDActor(x, y, mz, t);
  if (mo->info->seesound)
    S_StartSound(mo, mo->info->seesound);

  mo->owner = this; // Originator
  mo->angle = angle;
  angle >>= ANGLETOFINESHIFT;
  mo->px = int(mo->info->speed * finecosine[angle]);
  mo->py = int(mo->info->speed * finesine[angle]);
  mo->pz = momz;
  return (mo->CheckMissileSpawn() ? mo : NULL);
}



//----------------------------------------------------------------------------

static DActor *LavaInflictor;

void P_InitLava()
{
  LavaInflictor = new DActor(MT_PHOENIXFX2);
  LavaInflictor->flags =  MF_NOBLOCKMAP | MF_NOGRAVITY;
  LavaInflictor->flags2 = MF2_FIREDAMAGE|MF2_NODMGTHRUST;
}

//----------------------------------------------------------------------------


void DoomPatchEngine()
{
  Intermission::s_count = sfx_pistol;
  button_t::buttonsound = sfx_switchon;

  game.inventory = false;

  // hacks: teleport fog, blood, gibs
  mobjinfo[MT_TFOG].spawnstate = S_TFOG;
  sprnames[SPR_BLUD] = "BLUD";
  states[S_GIBS].sprite = SPR_POL5;
}


void HereticPatchEngine()
{
  Intermission::s_count = sfx_keyup;
  button_t::buttonsound = sfx_switchon;

  game.inventory = true;

  // hacks
  mobjinfo[MT_TFOG].spawnstate = S_HTFOG1;
  sprnames[SPR_BLUD] = "BLOD";
  states[S_GIBS].sprite = SPR_BLOD;

  // Above, good. Below, bad.
  text[TXT_PD_REDK] = "YOU NEED A GREEN KEY TO OPEN THIS DOOR";

  text[TXT_GOTBLUECARD] = "BLUE KEY";
  text[TXT_GOTYELWCARD] = "YELLOW KEY";
  text[TXT_GOTREDCARD] = "GREEN KEY";
}

void HexenPatchEngine()
{
  // FIXME sounds
  Intermission::s_count = sfx_switchon;
  button_t::buttonsound = sfx_switchon;

  game.inventory = true;

  // hacks
  mobjinfo[MT_TFOG].spawnstate = S_HTFOG1;
  sprnames[SPR_BLUD] = "BLOD";
  states[S_GIBS].sprite = SPR_GIBS;
}
