// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
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
// Revision 1.1  2002/11/16 14:17:59  hurdler
// Initial revision
//
// Revision 1.16  2002/09/20 22:41:31  vberghol
// Sound system rewritten! And it workscvs update
//
// Revision 1.14  2002/09/06 17:18:33  vberghol
// added most of the changes up to RC2
//
// Revision 1.13  2002/08/25 18:21:59  vberghol
// little fixes
//
// Revision 1.12  2002/08/20 13:56:58  vberghol
// sdfgsd
//
// Revision 1.11  2002/08/19 18:06:39  vberghol
// renderer somewhat fixed
//
// Revision 1.10  2002/08/13 19:47:42  vberghol
// p_inter.cpp done
//
// Revision 1.9  2002/08/11 17:16:49  vberghol
// ...
//
// Revision 1.8  2002/08/06 13:14:23  vberghol
// ...
//
// Revision 1.7  2002/07/26 19:23:04  vberghol
// a little something
//
// Revision 1.6  2002/07/18 19:16:38  vberghol
// renamed a few files
//
// Revision 1.5  2002/07/12 19:21:38  vberghol
// hop
//
// Revision 1.4  2002/07/08 20:46:33  vberghol
// More files compile!
//
// Revision 1.3  2002/07/01 21:00:18  jpakkane
// Fixed cr+lf to UNIX form.
//
// Revision 1.2  2002/06/28 10:57:14  vberghol
// Version 133 Experimental!
//
// Revision 1.23  2001/12/26 22:46:01  hurdler
// revert to beta 3 until it's fixed (there is at least a problem with saved game)
//
// Revision 1.22  2001/12/26 22:42:52  hurdler
// revert to beta 3 until it's fixed (there is at least a problem with saved game)
//
// Revision 1.18  2001/06/10 21:16:01  bpereira
// no message
//
// Revision 1.17  2001/05/27 13:42:47  bpereira
// no message
//
// Revision 1.16  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.15  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.14  2001/04/19 05:51:47  metzgermeister
// fixed 10 shells instead of 4 - bug
//
// Revision 1.13  2001/03/30 17:12:50  bpereira
// no message
//
// Revision 1.12  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.11  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.10  2000/11/02 17:50:07  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.9  2000/10/02 18:25:45  bpereira
// no message
//
// Revision 1.8  2000/10/01 10:18:17  bpereira
// no message
//
// Revision 1.7  2000/09/28 20:57:16  bpereira
// no message
//
// Revision 1.6  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.5  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.4  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      Handling interactions (i.e., collisions).
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "d_netcmd.h" // cvars
#include "i_system.h"   //I_Tactile currently has no effect
#include "am_map.h"
#include "dstrings.h"

#include "g_game.h"
#include "g_player.h"
#include "g_map.h"
#include "g_actor.h"
#include "g_pawn.h"

#include "m_random.h"
#include "p_enemy.h"
#include "s_sound.h"
#include "sounds.h"
#include "r_main.h"
#include "r_state.h"

#include "hu_stuff.h" // HUD

#define BONUSADD        6


// a weapon is found with two clip loads,
// a big item has five clip loads
int     maxammo[NUMAMMO] = {200, 50, 300, 50};
int     clipammo[NUMAMMO] = {10, 4, 20, 1};

consvar_t cv_fragsweaponfalling = {"fragsweaponfalling"   ,"0",CV_SAVE,CV_OnOff};

// added 4-2-98 (Boris) for dehacked patch
// (i don't like that but do you see another solution ?)
int max_health = 100;

//--------------------------------------------------------------------------
// was P_SetMessage
// this function is crap, FIXME sometime....

bool ultimatemsg;

void PlayerPawn::SetMessage(const char *msg, bool ultmsg = true)
{
  if ((ultimatemsg || !cv_showmessages.value) && !ultmsg)
    return;
    
  message = msg;
  //player->messageTics = MESSAGETICS;
  //BorderTopRefresh = true;
  if (ultmsg)
    ultimatemsg = true;
}

//
// GET STUFF
//


static const weapontype_t GetAmmoChange[] =
{
  wp_goldwand,
  wp_crossbow,
  wp_blaster,
  wp_skullrod,
  wp_phoenixrod,
  wp_mace
};

//
// was P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

bool PlayerPawn::GiveAmmo(ammotype_t at, int count)
{
  if (at == am_noammo)
    return false;

  if (at < 0 || at > NUMAMMO)
    {
      CONS_Printf ("\2P_GiveAmmo: bad type %i", at);
      return false;
    }

  if (ammo[at] >= maxammo[at])
    return false;
/*
  if (num)
  num *= clipammo[at];
  else
  num = clipammo[at]/2;
*/
  if (game.skill == sk_baby
      || game.skill == sk_nightmare)
    {
      if (game.mode == heretic)
	count += count>>1;
      else
	// give double ammo in trainer mode,
	// you'll need it in nightmare
	count <<= 1;
    }
  int oldammo = ammo[at];
  ammo[at] += count;

  if (ammo[at] > maxammo[at])
    ammo[at] = maxammo[at];

  // If non zero ammo,
  // don't change up weapons,
  // player was lower on purpose.
  if (oldammo)
    return true;

  // We were down to zero,
  // so select a new weapon.
  // Preferences are not user selectable.

  // Boris hack for preferred weapons order...
  if (!player->originalweaponswitch)
    {
      if (ammo[weaponinfo[readyweapon].ammo]
	  < weaponinfo[readyweapon].ammopershoot)
	UseFavoriteWeapon();
      return true;
    }
  else if (game.mode == heretic)
    {
      if ((readyweapon == wp_staff || readyweapon == wp_gauntlets) 
	  && weaponowned[GetAmmoChange[at]])
	pendingweapon = GetAmmoChange[at];
    }
  else switch (at)
    {
    case am_clip:
      if (readyweapon == wp_fist)
        {
	  if (weaponowned[wp_chaingun])
	    pendingweapon = wp_chaingun;
	  else
	    pendingweapon = wp_pistol;
        }
      break;

    case am_shell:
      if (readyweapon == wp_fist
	  || readyweapon == wp_pistol)
        {
	  if (weaponowned[wp_shotgun])
	    pendingweapon = wp_shotgun;
        }
      break;

    case am_cell:
      if (readyweapon == wp_fist
	  || readyweapon == wp_pistol)
        {
	  if (weaponowned[wp_plasma])
	    pendingweapon = wp_plasma;
        }
      break;

    case am_misl:
      if (readyweapon == wp_fist)
        {
	  if (weaponowned[wp_missile])
	    pendingweapon = wp_missile;
        }
    default:
      break;
    }

  return true;
}

// ammo get with the weapon
int GetWeaponAmmo[NUMWEAPONS] =
{
    0,  // staff       fist
   20,  // gold wand   pistol
    8,  // crossbow    shotgun
   20,  // blaster     chaingun
    2,  // skull rod   missile    
   40,  // phoenix rod plasma     
   40,  // mace        bfg        
    0,  // gauntlets   chainsaw   
    8,  // beak        supershotgun
};

//
// was P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//
bool PlayerPawn::GiveWeapon(weapontype_t wt, bool dropped)
{
  bool     gaveammo;
  bool     gaveweapon;

  if (game.multiplayer && (cv_deathmatch.value != 2) && !dropped)
    {
      // leave placed weapons forever on net games
      if (weaponowned[wt])
	return false;

      if (displayplayer == player)
        hud.bonuscount += BONUSADD;
      weaponowned[wt] = true;

      if (cv_deathmatch.value)
	GiveAmmo(weaponinfo[wt].ammo, 5*clipammo[weaponinfo[wt].ammo]);
      else
	GiveAmmo(weaponinfo[wt].ammo, GetWeaponAmmo[wt]);

      // Boris hack preferred weapons order...
      if (player->originalweaponswitch
	  || player->favoriteweapon[wt] > player->favoriteweapon[readyweapon])
	pendingweapon = wt;     // do like Doom2 original

      if (player == displayplayer || (cv_splitscreen.value && player == displayplayer2))
	S_StartAmbSound(sfx_wpnup);
      return false;
    }

  if (weaponinfo[wt].ammo != am_noammo)
    {
      // give one clip with a dropped weapon,
      // two clips with a found weapon
      if (dropped)
	gaveammo = GiveAmmo(weaponinfo[wt].ammo, clipammo[weaponinfo[wt].ammo]);
      else
	gaveammo = GiveAmmo(weaponinfo[wt].ammo, GetWeaponAmmo[wt]);
    }
  else
    gaveammo = false;

  if (weaponowned[wt])
    gaveweapon = false;
  else
    {
      gaveweapon = true;
      weaponowned[wt] = true;
      if (player->originalweaponswitch
	  || player->favoriteweapon[wt] > player->favoriteweapon[readyweapon])
	pendingweapon = wt;    // Doom2 original stuff
    }

  return (gaveweapon || gaveammo);
}




//
// was P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
bool PlayerPawn::GiveArmor(int at)
{
  int hits;

  hits = at*100;
  if (armorpoints >= hits)
    return false;   // don't pick up

  armortype = at;
  armorpoints = hits;

  return true;
}


//
// P_GiveCard
//
bool PlayerPawn::GiveCard(card_t ct)
{
  if (cards & ct)
    return false;

  if (displayplayer == player)
    hud.bonuscount = BONUSADD;
  cards |= ct;
  return true;
}


// Boris stuff : dehacked patches hack
int max_armor=200;
int green_armor_class=1;
int blue_armor_class=2;
int maxsoul=200;
int soul_health=100;
int mega_health=200;


//---------------------------------------------------------------------------
// was P_GiveArtifact
//
// Returns true if artifact accepted. FIXME turn into giveitem

bool PlayerPawn::GiveArtifact(artitype_t arti, Actor *from)
{
  vector<inventory_t>::iterator i = inventory.begin();

  // find the right slot
  while (i < inventory.end() && i->type != arti) i++;

  if (i == inventory.end())
    // give one artifact
    inventory.push_back(inventory_t(arti, 1));
  else
    {
      // player already has some of these
      if (i->count >= MAXARTECONT)
	// Player already has 16 of this item
	return false;
      
      i->count++; // one more
    }

  //if (inventory[inv_ptr].count == 0) inv_ptr = i;

  if (from && (from->flags & MF_COUNTITEM))
    player->items++;

  return true;
}


//---------------------------------------------------------------------------
//
// PROC P_SetDormantArtifact
//
// Removes the MF_SPECIAL flag, and initiates the artifact pickup
// animation.
//
//---------------------------------------------------------------------------

void P_SetDormantArtifact(Actor *arti)
{
  arti->flags &= ~MF_SPECIAL;
  if(cv_deathmatch.value && (arti->type != MT_ARTIINVULNERABILITY)
     && (arti->type != MT_ARTIINVISIBILITY))
    {
      arti->SetState(S_DORMANTARTI1);
    }
  else
    { // Don't respawn
      arti->SetState(S_DEADARTI1);
    }
  S_StartSound(arti, sfx_artiup);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreArtifact
//
//---------------------------------------------------------------------------

void A_RestoreArtifact(Actor *arti)
{
  arti->flags |= MF_SPECIAL;
  arti->SetState(arti->info->spawnstate);
  S_StartSound(arti, sfx_itmbk);
}

//----------------------------------------------------------------------------
//
// PROC P_HideSpecialThing
//
//----------------------------------------------------------------------------

void P_HideSpecialThing(Actor *thing)
{
  thing->flags &= ~MF_SPECIAL;
  thing->flags2 |= MF2_DONTDRAW;
  thing->SetState(S_HIDESPECIAL1);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing1
//
// Make a special thing visible again.
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing1(Actor *thing)
{
  if(thing->type == MT_WMACE)
    { // Do random mace placement
      thing->mp->RepositionMace(thing);
    }
  thing->flags2 &= ~MF2_DONTDRAW;
  S_StartSound(thing, sfx_itmbk);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing2
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing2(Actor *thing)
{
  thing->flags |= MF_SPECIAL;
  thing->SetState(thing->info->spawnstate);
}


//----------------------------------------------------------------------------
//
// PROC A_HideThing
//
//----------------------------------------------------------------------------

void A_HideThing(Actor *actor)
{
  //P_UnsetThingPosition(actor);
  actor->flags2 |= MF2_DONTDRAW;
}

//----------------------------------------------------------------------------
//
// PROC A_UnHideThing
//
//----------------------------------------------------------------------------

void A_UnHideThing(Actor *actor)
{
  //P_SetThingPosition(actor);
  actor->flags2 &= ~MF2_DONTDRAW;
}


//
// was P_TouchSpecialThing
//
void PlayerPawn::TouchSpecialThing(Actor *special)
{                  
  int         i;

  fixed_t delta = special->z - z;

  //SoM: 3/27/2000: For some reason, the old code allowed the player to
  //grab items that were out of reach...
  if (delta > height || delta < -special->height)
    {
      // out of reach
      return;
    }

  // Dead thing touching.
  // Can happen with a sliding player corpse.
  if (health <= 0 || flags & MF_CORPSE)
    return;

  int sound = sfx_itemup;

  // Identify by sprite.
  switch (special->sprite)
    {
    case SPR_SHLD: // Item_Shield1
      // armor
    case SPR_ARM1:
      if (!GiveArmor(green_armor_class))
	return;
      message = GOTARMOR;
      break;

    case SPR_SHD2: // Item_Shield2
    case SPR_ARM2:
      if (!GiveArmor(blue_armor_class))
	return;
      message = GOTMEGA;
      break;

      // bonus items
    case SPR_BON1:
      health++;               // can go over 100%
      if (health > 2*max_health)
	health = 2*max_health;
      if (cv_showmessages.value==1)
	message = GOTHTHBONUS;
      break;

    case SPR_BON2:
      armorpoints++;          // can go over 100%
      if (armorpoints > max_armor)
	armorpoints = max_armor;
      if (!armortype)
	armortype = 1;
      if (cv_showmessages.value==1)
	message = GOTARMBONUS;
      break;

    case SPR_SOUL:
      health += soul_health;
      if (health > maxsoul)
	health = maxsoul;
      message = GOTSUPER;
      sound = sfx_getpow;
      break;

    case SPR_MEGA:
      health = mega_health;
      GiveArmor(2);
      message = GOTMSPHERE;
      sound = sfx_getpow;
      break;

      // cards
      // leave cards for everyone
    case SPR_BKYY: // Key_Blue
    case SPR_BKEY:
      if( GiveCard (it_bluecard) )
        {
	  message = GOTBLUECARD;
	  if( game.mode == heretic ) sound = sfx_keyup;
        }
      if (!game.multiplayer)
	break;
      return;

    case SPR_CKYY: // Key_Yellow
    case SPR_YKEY:
      if( GiveCard (it_yellowcard) )
        {
	  message = GOTYELWCARD;
	  if( game.mode == heretic ) sound = sfx_keyup;
        }
      if (!game.multiplayer)
	break;
      return;

    case SPR_AKYY: // Key_Green
    case SPR_RKEY:
      if (GiveCard (it_redcard))
        {
	  message = GOTREDCARD;
	  if( game.mode == heretic ) sound = sfx_keyup;
        }
      if (!game.multiplayer)
	break;
      return;

    case SPR_BSKU:
      if (GiveCard (it_blueskull))
        {
	  message = GOTBLUESKUL;
	  if( game.mode == heretic ) sound = sfx_keyup;
        }
      if (!game.multiplayer)
	break;
      return;

    case SPR_YSKU:
      if (GiveCard (it_yellowskull))
        {
	  message = GOTYELWSKUL;
	  if( game.mode == heretic ) sound = sfx_keyup;
        }
      if (!game.multiplayer)
	break;
      return;

    case SPR_RSKU:
      if (GiveCard (it_redskull))
        {
	  message = GOTREDSKULL;
	  if( game.mode == heretic ) sound = sfx_keyup;
        }
      if (!game.multiplayer)
	break;
      return;

      // medikits, heals
    case SPR_PTN1: // Item_HealingPotion
    case SPR_STIM:
      if (!GiveBody (10))
	return;
      if(cv_showmessages.value==1)
	message = GOTSTIM;
      break;

    case SPR_MEDI:
      if (!GiveBody (25))
	return;
      if(cv_showmessages.value==1)
        {
	  if (health < 25)
	    message = GOTMEDINEED;
	  else
	    message = GOTMEDIKIT;
        }
      break;

        // heretic Artifacts :
    case SPR_PTN2: // Arti_HealingPotion
      if(GiveArtifact(arti_health, special))
	{
	  if( cv_showmessages.value==1 )
	    SetMessage(TXT_ARTIHEALTH, false);
              
	  P_SetDormantArtifact(special);
	}
      return;
    case SPR_SOAR: // Arti_Fly
      if(GiveArtifact(arti_fly, special))
	{
	  SetMessage(TXT_ARTIFLY, false);
	  P_SetDormantArtifact(special);
	}
      return;
    case SPR_INVU: // Arti_Invulnerability
      if(GiveArtifact(arti_invulnerability, special))
	{
	  SetMessage(TXT_ARTIINVULNERABILITY, false);
	  P_SetDormantArtifact(special);
	}
      return;
    case SPR_PWBK: // Arti_TomeOfPower
      if(GiveArtifact(arti_tomeofpower, special))
	{
	  SetMessage(TXT_ARTITOMEOFPOWER, false);
	  P_SetDormantArtifact(special);
	}
      return;
    case SPR_INVS: // Arti_Invisibility
      if(GiveArtifact(arti_invisibility, special))
	{
	  SetMessage(TXT_ARTIINVISIBILITY, false);
	  P_SetDormantArtifact(special);
	}
      return;
    case SPR_EGGC: // Arti_Egg
      if(GiveArtifact(arti_egg, special))
	{
	  SetMessage(TXT_ARTIEGG, false);
	  P_SetDormantArtifact(special);
	}
      return;
    case SPR_SPHL: // Arti_SuperHealth
      if(GiveArtifact(arti_superhealth, special))
	{
	  SetMessage(TXT_ARTISUPERHEALTH, false);
	  P_SetDormantArtifact(special);
	}
      return;
    case SPR_TRCH: // Arti_Torch
      if(GiveArtifact(arti_torch, special))
	{
	  SetMessage(TXT_ARTITORCH, false);
	  P_SetDormantArtifact(special);
	}
      return;
    case SPR_FBMB: // Arti_FireBomb
      if(GiveArtifact(arti_firebomb, special))
	{
	  SetMessage(TXT_ARTIFIREBOMB, false);
	  P_SetDormantArtifact(special);
	}
      return;
    case SPR_ATLP: // Arti_Teleport
      if(GiveArtifact(arti_teleport, special))
	{
	  SetMessage(TXT_ARTITELEPORT, false);
	  P_SetDormantArtifact(special);
	}
      return;

        // power ups
    case SPR_PINV:
      if (!GivePower (pw_invulnerability))
	return;
      message = GOTINVUL;
      sound = sfx_getpow;
      break;

    case SPR_PSTR:
      if (!GivePower (pw_strength))
	return;
      message = GOTBERSERK;
      if (readyweapon != wp_fist)
	pendingweapon = wp_fist;
      sound = sfx_getpow;
      break;

    case SPR_PINS:
      if (!GivePower (pw_invisibility))
	return;
      message = GOTINVIS;
      sound = sfx_getpow;
      break;

    case SPR_SUIT:
      if (!GivePower (pw_ironfeet))
	return;
      message = GOTSUIT;
      sound = sfx_getpow;
      break;

    case SPR_SPMP: // Item_SuperMap
    case SPR_PMAP:
      if (!GivePower (pw_allmap))
	return;
      message = GOTMAP;
      if( game.mode != heretic )
	sound = sfx_getpow;
      break;

    case SPR_PVIS:
      if (!GivePower (pw_infrared))
	return;
      message = GOTVISOR;
      sound = sfx_getpow;
      break;

      // heretic Ammo
    case SPR_AMG1: // Ammo_GoldWandWimpy
      if(!GiveAmmo(am_goldwand, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOGOLDWAND1, false);
      break;
    case SPR_AMG2: // Ammo_GoldWandHefty
      if(!GiveAmmo(am_goldwand, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOGOLDWAND2, false);
      break;
    case SPR_AMM1: // Ammo_MaceWimpy
      if(!GiveAmmo(am_mace, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOMACE1, false);
      break;
    case SPR_AMM2: // Ammo_MaceHefty
      if(!GiveAmmo(am_mace, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOMACE2, false);
      break;
    case SPR_AMC1: // Ammo_CrossbowWimpy
      if(!GiveAmmo(am_crossbow, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOCROSSBOW1, false);
      break;
    case SPR_AMC2: // Ammo_CrossbowHefty
      if(!GiveAmmo(am_crossbow, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOCROSSBOW2, false);
      break;
    case SPR_AMB1: // Ammo_BlasterWimpy
      if(!GiveAmmo(am_blaster, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOBLASTER1, false);
      break;
    case SPR_AMB2: // Ammo_BlasterHefty
      if(!GiveAmmo(am_blaster, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOBLASTER2, false);
      break;
    case SPR_AMS1: // Ammo_SkullRodWimpy
      if(!GiveAmmo(am_skullrod, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOSKULLROD1, false);
      break;
    case SPR_AMS2: // Ammo_SkullRodHefty
      if(!GiveAmmo(am_skullrod, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOSKULLROD2, false);
      break;
    case SPR_AMP1: // Ammo_PhoenixRodWimpy
      if(!GiveAmmo(am_phoenixrod, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOPHOENIXROD1, false);
      break;
    case SPR_AMP2: // Ammo_PhoenixRodHefty
      if(!GiveAmmo(am_phoenixrod, special->health))
	{
	  return;
	}
      if( cv_showmessages.value==1 )
	SetMessage(TXT_AMMOPHOENIXROD2, false);
      break;

      // ammo
    case SPR_CLIP:
      if (special->flags & MF_DROPPED)
        {
	  if (!GiveAmmo(am_clip,clipammo[am_clip]/2))
	    return;
        }
      else
        {
	  if (!GiveAmmo(am_clip,clipammo[am_clip]))
	    return;
        }
      if(cv_showmessages.value==1)
	message = GOTCLIP;
      break;

    case SPR_AMMO:
      if (!GiveAmmo (am_clip,5*clipammo[am_clip]))
	return;
      if(cv_showmessages.value==1)
	message = GOTCLIPBOX;
      break;

    case SPR_ROCK:
      if (!GiveAmmo (am_misl,clipammo[am_misl]))
	return;
      if(cv_showmessages.value==1)
	message = GOTROCKET;
      break;

    case SPR_BROK:
      if (!GiveAmmo (am_misl,5*clipammo[am_misl]))
	return;
      if(cv_showmessages.value==1)
	message = GOTROCKBOX;
      break;

    case SPR_CELL:
      if (!GiveAmmo (am_cell,clipammo[am_cell]))
	return;
      if(cv_showmessages.value==1)
	message = GOTCELL;
      break;

    case SPR_CELP:
      if (!GiveAmmo (am_cell,5*clipammo[am_cell]))
	return;
      if(cv_showmessages.value==1)
	message = GOTCELLBOX;
      break;

    case SPR_SHEL:
      if (!GiveAmmo (am_shell,clipammo[am_shell]))
	return;
      if(cv_showmessages.value==1)
	message = GOTSHELLS;
      break;

    case SPR_SBOX:
      if (!GiveAmmo (am_shell,5*clipammo[am_shell]))
	return;
      if(cv_showmessages.value==1)
	message = GOTSHELLBOX;
      break;

    case SPR_BPAK:
      if (!backpack)
        {
	  for (i=0 ; i<NUMAMMO ; i++)
	    maxammo[i] *= 2;
	  backpack = true;
        }
      for (i=0 ; i<NUMAMMO ; i++)
	GiveAmmo (ammotype_t(i), clipammo[i]);
      message = GOTBACKPACK;
      break;

    case SPR_BAGH: // Item_BagOfHolding
      if(!backpack)
        {
	  for(i = 0; i < NUMAMMO; i++)
	    maxammo[i] *= 2;
	  backpack = true;
        }
      GiveAmmo(am_goldwand, AMMO_GWND_WIMPY);
      GiveAmmo(am_blaster, AMMO_BLSR_WIMPY);
      GiveAmmo(am_crossbow, AMMO_CBOW_WIMPY);
      GiveAmmo(am_skullrod, AMMO_SKRD_WIMPY);
      GiveAmmo(am_phoenixrod, AMMO_PHRD_WIMPY);
      SetMessage(TXT_ITEMBAGOFHOLDING, false);
      break;

        // weapons
    case SPR_BFUG:
      if (!GiveWeapon (wp_bfg, false) )
	return;
      message = GOTBFG9000;
      sound = sfx_wpnup;
      break;

    case SPR_MGUN:
      if (!GiveWeapon (wp_chaingun, special->flags & MF_DROPPED) )
	return;
      message = GOTCHAINGUN;
      sound = sfx_wpnup;
      break;

    case SPR_CSAW:
      if (!GiveWeapon (wp_chainsaw, false) )
	return;
      message = GOTCHAINSAW;
      sound = sfx_wpnup;
      break;

    case SPR_LAUN:
      if (!GiveWeapon (wp_missile, false) )
	return;
      message = GOTLAUNCHER;
      sound = sfx_wpnup;
      break;

    case SPR_PLAS:
      if (!GiveWeapon (wp_plasma, false) )
	return;
      message = GOTPLASMA;
      sound = sfx_wpnup;
      break;

    case SPR_SHOT:
      if (!GiveWeapon (wp_shotgun, special->flags&MF_DROPPED ) )
	return;
      message = GOTSHOTGUN;
      sound = sfx_wpnup;
      break;

    case SPR_SGN2:
      if (!GiveWeapon (wp_supershotgun, special->flags&MF_DROPPED ) )
	return;
      message = GOTSHOTGUN2;
      sound = sfx_wpnup;
      break;

      // heretic weapons
    case SPR_WMCE: // Weapon_Mace
      if(!GiveWeapon(wp_mace,false))
	{
	  return;
	}
      SetMessage(TXT_WPNMACE, false);
      sound = sfx_wpnup;
      break;
    case SPR_WBOW: // Weapon_Crossbow
      if(!GiveWeapon(wp_crossbow,false))
	{
	  return;
	}
      SetMessage(TXT_WPNCROSSBOW, false);
      sound = sfx_wpnup;
      break;
    case SPR_WBLS: // Weapon_Blaster
      if(!GiveWeapon(wp_blaster,false))
	{
	  return;
	}
      SetMessage(TXT_WPNBLASTER, false);
      sound = sfx_wpnup;
      break;
    case SPR_WSKL: // Weapon_SkullRod
      if(!GiveWeapon(wp_skullrod, false))
	{
	  return;
	}
      SetMessage(TXT_WPNSKULLROD, false);
      sound = sfx_wpnup;
      break;
    case SPR_WPHX: // Weapon_PhoenixRod
      if(!GiveWeapon(wp_phoenixrod, false))
	{
	  return;
	}
      SetMessage(TXT_WPNPHOENIXROD, false);
      sound = sfx_wpnup;
      break;
    case SPR_WGNT: // Weapon_Gauntlets
      if(!GiveWeapon(wp_gauntlets, false))
	{
	  return;
	}
      SetMessage(TXT_WPNGAUNTLETS, false);
      sound = sfx_wpnup;
      break;

    default:
      // SoM: New gettable things with FraggleScript!
      //CONS_Printf ("\2P_TouchSpecialThing: Unknown gettable thing\n");
      return;
    }

  if (special->flags & MF_COUNTITEM)
    player->items++;
  special->Remove();
  if (displayplayer == player)
    hud.bonuscount += BONUSADD;

    //added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
  if (player == displayplayer || (cv_splitscreen.value && player == displayplayer2))
    S_StartAmbSound(sound);
}



#ifdef thatsbuggycode
//
//  Tell each supported thing to check again its position,
//  because the 'base' thing has vanished or diminished,
//  the supported things might fall.
//
//added:28-02-98:
void P_CheckSupportThings (Actor* mobj)
{
  fixed_t   supportz = mobj->z + mobj->height;

  while ((mobj = mobj->supportthings))
    {
      // only for things above support thing
      if (mobj->z > supportz)
	mobj->eflags |= MF_CHECKPOS;
    }
}


//
//  If a thing moves and supportthings,
//  move the supported things along.
//
//added:28-02-98:
void P_MoveSupportThings (Actor* mobj, fixed_t xmove, fixed_t ymove, fixed_t zmove)
{
  fixed_t   supportz = mobj->z + mobj->height;
  Actor    *mo = mobj->supportthings;

  while (mo)
    {
      //added:28-02-98:debug
      if (mo==mobj)
        {
	  mobj->supportthings = NULL;
	  break;
        }
      
      // only for things above support thing
      if (mobj->z > supportz)
        {
	  mobj->eflags |= MF_CHECKPOS;
	  mobj->px += xmove;
	  mobj->py += ymove;
	  mobj->pz += zmove;
        }

      mo = mo->supportthings;
    }
}


//
//  Link a thing to it's 'base' (supporting) thing.
//  When the supporting thing will move or change size,
//  the supported will then be aware.
//
//added:28-02-98:
void P_LinkFloorThing(Actor*   mobj)
{
    Actor*     mo;
    Actor*     nmo;

    // no supporting thing
    if (!(mo = mobj->floorthing))
        return;

    // link mobj 'above' the lower mobjs, so that lower supporting
    // mobjs act upon this mobj
    while ( (nmo = mo->supportthings) &&
            (nmo->z<=mobj->z) )
    {
        // dont link multiple times
        if (nmo==mobj)
            return;

        mo = nmo;
    }
    mo->supportthings = mobj;
    mobj->supportthings = nmo;
}


//
//  Unlink a thing from it's support,
//  when it's 'floorthing' has changed,
//  before linking with the new 'floorthing'.
//
//added:28-02-98:
void P_UnlinkFloorThing(Actor*   mobj)
{
  Actor*     mo;

  if (!(mo = mobj->floorthing))      // just to be sure (may happen)
    return;

  while (mo->supportthings)
    {
      if (mo->supportthings == mobj)
        {
	  mo->supportthings = NULL;
	  break;
        }
      mo = mo->supportthings;
    }
}
#endif

// was P_DeathMessages
// Death messages relating to the target (dying) player
//
static void P_DeathMessages(PlayerPawn* t, Actor *inflictor, Actor *source)
{
  char *str = NULL;

  if (!source)
    {
      // environment kills
      int w = t->specialsector;      //see p_spec.c

      if (w==5)
	str = text[DEATHMSG_HELLSLIME];
      else if (w==7)
	str = text[DEATHMSG_NUKE];
      else if (w==16 || w==4)
	str = text[DEATHMSG_SUPHELLSLIME];
      else
	str = text[DEATHMSG_SPECUNKNOW];
      CONS_Printf(str, t->player->name.c_str());
    }
  else if (source->Type() == Thinker::tt_ppawn)
    {
      // player kill
      PlayerPawn *s = (PlayerPawn *)source;

      if (s->player == t->player)
	{
	  CONS_Printf(text[DEATHMSG_SUICIDE], t->player->name.c_str());
	  // FIXME when console is rewritten to accept << >>
	  //if (cv_splitscreen.value)
	  // console << "\4" << t->player->name << text[DEATHMSG_SUICIDE];
	}
      else
        {
	  if (t->health < -9000) // telefrag !
	    str = text[DEATHMSG_TELEFRAG];
	  else
            {
	      int w = -1;
	      if (inflictor)
                {
		  switch(inflictor->type) {
		  case MT_BARREL:
		    w = wp_barrel;
		    break;
		  case MT_ROCKET   :
		    w = wp_missile;
		    break;
		  case MT_PLASMA   :
		    w = wp_plasma;
		    break;
		  case MT_EXTRABFG :
		  case MT_BFG      :
		    w = wp_bfg;
		    break;
		  default :
		    w = s->readyweapon;
		    break;
		  }
                }

	      switch(w)
                {
                case wp_fist:
		  str = text[DEATHMSG_FIST];
		  break;
                case wp_pistol:
		  str = text[DEATHMSG_GUN];
		  break;
                case wp_shotgun:
		  str = text[DEATHMSG_SHOTGUN];
		  break;
                case wp_chaingun:
		  str = text[DEATHMSG_MACHGUN];
		  break;
                case wp_missile:
		  str = text[DEATHMSG_ROCKET];
		  if (t->health < -t->info->spawnhealth &&
		      t->info->xdeathstate)
		    str = text[DEATHMSG_GIBROCKET];
		  break;
                case wp_plasma:
		  str = text[DEATHMSG_PLASMA];
		  break;
                case wp_bfg:
		  str = text[DEATHMSG_BFGBALL];
		  break;
                case wp_chainsaw:
		  str = text[DEATHMSG_CHAINSAW];
		  break;
                case wp_supershotgun:
		  str = text[DEATHMSG_SUPSHOTGUN];
		  break;
		case wp_barrel:
		  str = text[DEATHMSG_BARRELFRAG];
		  break;
                default:
		  str = text[DEATHMSG_PLAYUNKNOW];
		  break;
                }
            }
	  CONS_Printf(str, t->player->name.c_str(),
		      s->player->name.c_str());
	  // FIXME when console is rewritten to accept << >>
	  //if (cv_splitscreen.value)
	  // console << "\4" << str...
        }
    }
  else
    {
      // monster kill
      switch (source->type)
	{
	case MT_BARREL:    str = text[DEATHMSG_BARREL]; break;
	case MT_POSSESSED: str = text[DEATHMSG_POSSESSED]; break;
	case MT_SHOTGUY:   str = text[DEATHMSG_SHOTGUY];   break;
	case MT_VILE:      str = text[DEATHMSG_VILE];      break;
	case MT_FATSO:     str = text[DEATHMSG_FATSO];     break;
	case MT_CHAINGUY:  str = text[DEATHMSG_CHAINGUY];  break;
	case MT_TROOP:     str = text[DEATHMSG_TROOP];     break;
	case MT_SERGEANT:  str = text[DEATHMSG_SERGEANT];  break;
	case MT_SHADOWS:   str = text[DEATHMSG_SHADOWS];   break;
	case MT_HEAD:      str = text[DEATHMSG_HEAD];      break;
	case MT_BRUISER:   str = text[DEATHMSG_BRUISER];   break;
	case MT_UNDEAD:    str = text[DEATHMSG_UNDEAD];    break;
	case MT_KNIGHT:    str = text[DEATHMSG_KNIGHT];    break;
	case MT_SKULL:     str = text[DEATHMSG_SKULL];     break;
	case MT_SPIDER:    str = text[DEATHMSG_SPIDER];    break;
	case MT_BABY:      str = text[DEATHMSG_BABY];      break;
	case MT_CYBORG:    str = text[DEATHMSG_CYBORG];    break;
	case MT_PAIN:      str = text[DEATHMSG_PAIN];      break;
	case MT_WOLFSS:    str = text[DEATHMSG_WOLFSS];    break;
	default:           str = text[DEATHMSG_DEAD];      break;
	}
    }
  CONS_Printf(str, t->player->name.c_str());

}


void PlayerPawn::Kill(Actor *inflictor, Actor *source)
{
  Actor::Kill(inflictor, source);

  PlayerPawn *s = NULL;
  if (source && source->Type() == Thinker::tt_ppawn)
    s = (PlayerPawn *)source;

  // show death messages, only if it concern the console player
  // (be it an attacker or a target)
  if (player == consoleplayer)
    P_DeathMessages (this, inflictor, source);
  else if (s && s->player == consoleplayer)
    P_DeathMessages (this, inflictor, source);


  // count frags if player killed player
  if (s)
    {
      game.UpdateScore(s->player, player);
      if( game.mode == heretic )
	{
	  if(s->player == displayplayer 
	     || s->player == displayplayer2 )
	    S_StartAmbSound(sfx_gfrag);

	  // Make a super chicken
	  if (s->morphTics)
	    s->GivePower(pw_weaponlevel2);
	}
    }
  // count environment kills against you (you fragged yourself!)
  if (!source)
    {
      game.UpdateScore(player, player);
    }

  // make corpse
  flags &= ~MF_SOLID;                     // does not block
  flags2 &= ~MF2_FLY;
  powers[pw_flight] = 0;
  powers[pw_weaponlevel2] = 0;
  player->playerstate = PST_DEAD;
  DropWeapon();  // put weapon away
  if (player == consoleplayer)
    {
      // don't die in auto map,
      // switch view prior to dying
      if (automap.active)
	automap.Close();

      // recenter view for next live...
      localaiming = 0;
    }
  if (player == consoleplayer2)
    {
      // recenter view for next live...
      localaiming2 = 0;
    }
  /* HERE TODO
     if(flags2&MF2_FIREDAMAGE)
     { // Player flame death
     SetState(S_PLAY_FDTH1);
     //S_StartSound(this, sfx_hedat1); // Burn sound
     return;
     }
  */  
}
// was P_KillMobj
//
//      source is the attacker,
//      target is the 'target' of the attack, target dies...
//                                          113
void Actor::Kill(Actor *inflictor, Actor *source)
{
  extern consvar_t cv_solidcorpse;

  // dead target is no more shootable
  if (!cv_solidcorpse.value)
    flags &= ~MF_SHOOTABLE;

  flags &= ~(MF_FLOAT|MF_SKULLFLY);

  if (type != MT_SKULL)
    flags &= ~MF_NOGRAVITY;

  // scream a corpse :)
  if (flags & MF_CORPSE)
    {
      // turn it to gibs
      SetState(S_GIBS);

      flags &= ~MF_SOLID;
      height = 0;
      radius<<= 1;
      //this->skin = 0;

      //added:22-02-98: lets have a neat 'crunch' sound!
      S_StartSound (this, sfx_slop);
      return;
    }

  //added:22-02-98: remember who exploded the barrel, so that the guy who
  //                shot the barrel which killed another guy, gets the frag!
  //                (source is passed from barrel to barrel also!)
  //                (only for multiplayer fun, does not remember monsters)
  if ((type == MT_BARREL || type == MT_POD) && source)
    owner = source;

  if( game.demoversion < 131 )
    {
      // in version 131 and higer this is done later in a_fall 
      // (this fix the stepping monster)
      flags   |= MF_CORPSE|MF_DROPOFF;
      height >>= 2;
      if( game.demoversion>=112 )
	radius -= (radius>>4);      //for solid corpses
    }

  // if killed by a player
  if (flags & MF_COUNTKILL)
    {
      if (source && source->Type() == Thinker::tt_ppawn)
	{
	  PlayerPawn *s = (PlayerPawn *)source;
	  // count for intermission
	  s->player->kills++;    
	}
      else if (!game.multiplayer)
	{
	  // count all monster deaths,
	  // even those caused by other monsters
	  consoleplayer->kills++;
	}
    }

  if (( (game.mode != heretic && health < -info->spawnhealth)
        ||(game.mode == heretic && health < -(info->spawnhealth>>1)))
      && info->xdeathstate)
    {
      SetState(info->xdeathstate);
    }
  else
    SetState(info->deathstate);

  tics -= P_Random()&3;

  if (tics < 1)
    tics = 1;

  mobjtype_t item;
  // Drop stuff.
  // This determines the kind of object spawned
  // during the death frame of a thing.
  switch (type)
    {
    case MT_WOLFSS:
    case MT_POSSESSED:
      item = MT_CLIP;
      break;

    case MT_SHOTGUY:
      item = MT_SHOTGUN;
      break;

    case MT_CHAINGUY:
      item = MT_CHAINGUN;
      break;

    default:
      return;
    }

  // SoM: Damnit! Why not use the target's floorz?
  Actor *mo = mp->SpawnActor(x, y, game.demoversion<132 ? ONFLOORZ : floorz, item);
  mo->flags |= MF_DROPPED;    // special versions of items
}


//---------------------------------------------------------------------------
//
// FUNC P_MinotaurSlam
//
//---------------------------------------------------------------------------

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

  //if(target->player) FIXME... if necessary
    {
      target->reactiontime = 14+(P_Random()&7);
    }
}

//---------------------------------------------------------------------------
//
// FUNC P_TouchWhirlwind
//
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// was P_ChickenMorphPlayer
// Returns true if the player gets turned into a chicken.

#define CHICKENTICS     (40*TICRATE)

bool PlayerPawn::Morph()
{
  Actor *fog;
  Actor *chicken;
  
  if (morphTics)
    {
      if ((morphTics < CHICKENTICS-TICRATE)
	  && !powers[pw_weaponlevel2])
        { // Make a super chicken
	  GivePower(pw_weaponlevel2);
        }
      return false;
    }
  if (powers[pw_invulnerability])
    { // Immune when invulnerable
      return false;
    }

  // store x,y,z, angle, flags2
  //SetState(S_FREETARGMOBJ);

  fog = mp->SpawnActor(x, y, z+TELEFOGHEIGHT, MT_TFOG);
  S_StartSound(fog, sfx_telept);

  // FIXME again this Morph/FREETARGMOBJ problem...
  chicken = mp->SpawnActor(x, y, z, MT_CHICPLAYER);
  chicken->special1 = readyweapon;
  chicken->angle = angle;
  //chicken->player = player;
  //chicken->health = MAXCHICKENHEALTH;
  //player->mo = chicken;
  armorpoints = armortype = 0;
  powers[pw_invisibility] = 0;
  powers[pw_weaponlevel2] = 0;
  weaponinfo = wpnlev1info;
  if (flags2 & MF2_FLY)
    {
      chicken->flags2 |= MF2_FLY;
    }
  morphTics = CHICKENTICS;
  ActivateBeak();
  return true;
}

//---------------------------------------------------------------------------
//
// was P_ChickenMorph
//
//---------------------------------------------------------------------------

bool Actor::Morph()
{
  Actor  *fog;
  Actor  *chicken;
  Actor  *ntarget;
  fixed_t nx, ny, nz;
  angle_t ang;
  int ghost;
    
  mobjtype_t moType = type;
  switch(moType)
    {
    case MT_POD:
    case MT_CHICKEN:
    case MT_HHEAD:
    case MT_MINOTAUR:
    case MT_SORCERER1:
    case MT_SORCERER2:
      return false;
    default:
      break;
    }
  nx = x;
  ny = y;
  nz = z;
  ang = angle;
  ghost = flags & MF_SHADOW;
  ntarget = target;
  // FIXME again this freetargmobj...
  SetState(S_FREETARGMOBJ);
  fog = mp->SpawnActor(nx, ny, nz+TELEFOGHEIGHT, MT_TFOG);
  S_StartSound(fog, sfx_telept);
  chicken = mp->SpawnActor(nx, ny, nz, MT_CHICKEN);
  chicken->special2 = moType;
  chicken->special1 = CHICKENTICS+P_Random();
  chicken->flags |= ghost;
  chicken->target = ntarget;
  chicken->angle = angle;
  return true;
}

//---------------------------------------------------------------------------
//
// FUNC P_AutoUseChaosDevice
//
//---------------------------------------------------------------------------

bool P_AutoUseChaosDevice(PlayerPawn *p)
{
  int i, n = p->inventory.size();
    
  for (i = 0; i < n; i++)
    {
      if (p->inventory[i].type == arti_teleport)
        {
	  p->UseArtifact(arti_teleport);
	  p->health = (p->health + 1) / 2;
	  return true;
        }
    }
  return false;
}

//---------------------------------------------------------------------------
//
// PROC P_AutoUseHealth
//
//---------------------------------------------------------------------------

void P_AutoUseHealth(PlayerPawn *p, int saveHealth)
{
  int i, n = p->inventory.size();
  int count;
  int normalCount;
  int normalSlot;
  int superCount;
  int superSlot;
    
  normalCount = superCount = 0;
  for(i = 0; i < n; i++)
    {
      if (p->inventory[i].type == arti_health)
        {
	  normalSlot = i;
	  normalCount = p->inventory[i].count;
        }
      else if (p->inventory[i].type == arti_superhealth)
        {
	  superSlot = i;
	  superCount = p->inventory[i].count;
        }
    }
  if((game.skill == sk_baby) && (normalCount*25 >= saveHealth))
    { // Use quartz flasks
      count = (saveHealth+24)/25;
      for(i = 0; i < count; i++)
	p->UseArtifact(arti_health);
    }
  else if(superCount*100 >= saveHealth)
    { // Use mystic urns
      count = (saveHealth+99)/100;
      for(i = 0; i < count; i++)
	p->UseArtifact(arti_superhealth);
    }
  else if((game.skill == sk_baby) && (superCount*100+normalCount*25 >= saveHealth))
    { // Use mystic urns and quartz flasks
      count = (saveHealth+24)/25;
      for(i = 0; i < count; i++)
	p->UseArtifact(arti_health);

      saveHealth -= count*25;
      count = (saveHealth+99)/100;
      for(i = 0; i < count; i++)
	p->UseArtifact(arti_superhealth);
    }
}


//
// was P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
// TODO the damage/thrust logic should be changed altogether, using functions like
// Staff::Hit(actor) {
//    actor->Damage(2,this);
//    if (actor.mass == small) actor->Thrust()...
// }

bool PlayerPawn::Damage(Actor *inflictor, Actor *source, int damage)
{
  bool takedamage = true; // false on some case in teamplay

  if (game.skill == sk_baby)
    damage >>= 1;   // take half damage in trainer mode
  
  if (inflictor)
    {
      switch (inflictor->type)
	{
	case MT_MACEFX4: // Death ball
	  // Player specific checks
	  if (powers[pw_invulnerability])
	    // Can't hurt invulnerable players
	    takedamage = false;
	    break;	  
	  if (P_AutoUseChaosDevice(this))
	    // Player was saved using chaos device
	    return false;	
	  damage = 10000; // Something's gonna die
	  break;
        case MT_PHOENIXFX2: // Flame thrower
	  if (P_Random() < 128)
            { // Freeze player for a bit
	      reactiontime += 4;
            }
	  break;
	default:
	  break;
	}
    }

  // player specific
  if (!(flags & MF_CORPSE))
    {
      // end of game hell hack
      if (subsector->sector->special == 11 && damage >= health)
        {
	  damage = health - 1;
        }

      // Below certain threshold,
      // ignore damage in GOD mode, or with INVUL power.
      if (damage < 1000 &&
	  ((cheats & CF_GODMODE) || powers[pw_invulnerability]))
        {
	  return false;
        }

      if (armortype)
        {
	  int saved;
	  if (armortype == 1)
	    saved = game.mode == heretic ? damage>>1 : damage/3;
	  else
	    saved = game.mode == heretic ? (damage>>1)+(damage>>2) : damage/2;

	  if (armorpoints <= saved)
            {
	      // armor is used up
	      saved = armorpoints;
	      armortype = 0;
            }
	  armorpoints -= saved;
	  damage -= saved;
        }

      PlayerPawn *s = NULL;
      if (source && source->Type() == Thinker::tt_ppawn)
	s = (PlayerPawn *)source;

      // added team play and teamdamage (view logboris at 13-8-98 to understand)
      if (game.demoversion < 125   || // support old demoversion
	  cv_teamdamage.value ||
	  damage>1000         || // telefrag
	  source == this || !source || !s)
	//	||  (cv_deathmatch.value && (!cv_teamplay.value || !game.SameTeam(s, p))))
        {
	  // damage is done!
	  if (damage >= health && ((game.skill == sk_baby) || cv_deathmatch.value)
	      && !morphTics)
            { // Try to use some inventory health
	      P_AutoUseHealth(this, damage-health+1);
            }

	  //health -= damage;   // mirror mobj health here for Dave

	  if (player == displayplayer)
	    hud.damagecount += damage;  // add damage after armor / invuln

	  //added:22-02-98: force feedback ??? electro-shock???
	  if (player == consoleplayer)
	    I_Tactile (40,10,40+min(damage, 100)*2);
        }
      else
	takedamage = false;

      attacker = source;
    }

  // FIXME this entire function
  if (takedamage)
    takedamage = Actor::Damage(inflictor, source, damage);
  else
    return false;

  // TODO instead use damage types?
  // kinetic, fire, drowning, acid etc.
  if (health <= 0 && inflictor && !morphTics)
    { // Check for flame death
      if ((inflictor->flags2 & MF2_FIREDAMAGE) ||
	  ((inflictor->type == MT_PHOENIXFX1)
	   && (health > -50) && (damage > 25)))
	{
	  flags2 |= MF2_FIREDAMAGE;
	}
    }

  return takedamage;
}


#define BASETHRESHOLD 100

bool Actor::Damage(Actor *inflictor, Actor *source, int damage)
{
  unsigned    ang;
  fixed_t     thrust;

  if (!(flags & MF_SHOOTABLE))
    return false; // shouldn't happen...

  if (health <= 0)
    return false;

  if (flags & MF_SKULLFLY)
    {
      // Minotaur is invulnerable during charge attack
      if (type == MT_MINOTAUR)
	return false;
      px = py = pz = 0;
    }

  // Special damage types
  if (inflictor)
    {
      switch(inflictor->type)
        {
        case MT_EGGFX:
	  Morph();
	  return false; // Always return
        case MT_WHIRLWIND:
	  return P_TouchWhirlwind(this);
        case MT_MINOTAUR:
	  if (inflictor->flags & MF_SKULLFLY)
            { // Slam only when in charge mode
	      P_MinotaurSlam(inflictor, this);
	      return true;
            }
	  break;
        case MT_MACEFX4: // Death ball
	  if ((flags2 & MF2_BOSS) || type == MT_HHEAD)
	    // Don't allow cheap boss kills
	    break;
	  damage = 10000; // Something's gonna die
	  break;
        case MT_RAINPLR1: // Rain missiles
        case MT_RAINPLR2:
        case MT_RAINPLR3:
        case MT_RAINPLR4:
	  if (flags2 & MF2_BOSS)
            { // Decrease damage for bosses
	      damage = (P_Random()&7)+1;
            }
	  break;
        case MT_HORNRODFX2:
        case MT_PHOENIXFX1:
	  if (type == MT_SORCERER2 && P_Random() < 96)
            { // D'Sparil teleports away
	      DSparilTeleport();
	      return false;
            }
	  break;
        case MT_BLASTERFX1:
        case MT_RIPPER:
	  if (type == MT_HHEAD)
            { // Less damage to Ironlich bosses
	      damage = P_Random()&1;
	      if (!damage)
		return false;
            }
	  break;
        default:
	  break;
        }
    }

  PlayerPawn *s = NULL;
  if (source && source->Type() == Thinker::tt_ppawn)
    s = (PlayerPawn *)source;
  
  // Some close combat weapons should not
  // inflict thrust and push the victim out of reach,
  // thus kick away unless using the chainsaw.
  if (inflictor && !(flags & MF_NOCLIP) && !(inflictor->flags2 & MF2_NODMGTHRUST)
      && (!source || !s || s->readyweapon != wp_chainsaw))
    {
      fixed_t            apx, apy, apz = 0;//SoM: 3/28/2000
      extern consvar_t   cv_allowrocketjump;

      ang = R_PointToAngle2(inflictor->x, inflictor->y, x, y);

      if (game.mode == heretic )
	thrust = damage*(FRACUNIT>>3)*150/info->mass;
      else
	thrust = damage*(FRACUNIT>>3)*100/info->mass;

      // sometimes a target shot down might fall off a ledge forwards
      if (damage < 40 && damage > health
	  && (z - inflictor->z) > 64*FRACUNIT && (P_Random() & 1))
        {
	  ang += ANG180;
	  thrust *= 4;
        }

      ang >>= ANGLETOFINESHIFT;

      if (game.mode == heretic && source && s && (source == inflictor)
	  && s->powers[pw_weaponlevel2]
	  && s->readyweapon == wp_staff)
        {
	  // Staff power level 2
	  px += FixedMul(10*FRACUNIT, finecosine[ang]);
	  py += FixedMul(10*FRACUNIT, finesine[ang]);
	  if(!(flags&MF_NOGRAVITY))
            {
	      pz += 5*FRACUNIT;
            }
        }
      else
        {
	  apx = FixedMul (thrust, finecosine[ang]);
	  apy = FixedMul (thrust, finesine[ang]);
	  px += apx;
	  py += apy;
            
	  // added pz (do it better for missiles explotion)
	  if (source && game.demoversion>=124 && (game.demoversion<129 || !cv_allowrocketjump.value))
            {
	      fixed_t dist, sx, sy, sz;
	      if (source == this) // rocket in yourself (suicide)
                {
		  // FIXME we should use this always, thrust cannot come from the shooter direction
		  // if an inflictor exists, right? With immediate LineAttack damage (pistol, for example)
		  // source == inflictor, so no problem there either.
		  //viewx=inflictor->x;
		  //viewy=inflictor->y;
		  sx = inflictor->x;
		  sy = inflictor->y;
		  sz = inflictor->z;
                }
	      else
                {
		  //viewx=source->x;
		  //viewy=source->y;
		  sx = source->x;
		  sy = source->y;
		  sz = source->z;
                }
	      //dist=R_PointToDist(x,y);
	      dist = R_PointToDist2(sx, sy, x, y);
                
	      //viewx=0;
	      //viewy=sz;
	      //ang = R_PointToAngle(dist,z);
	      ang = R_PointToAngle2(0, sz, dist, z);
                
	      ang >>= ANGLETOFINESHIFT;
	      apz = FixedMul (thrust, finesine[ang]);
            }
	  else //SoM: 2/28/2000: Added new function.
            if(game.demoversion >= 129 && cv_allowrocketjump.value)
	      {
                fixed_t delta1 = abs(inflictor->z - z);
                fixed_t delta2 = abs(inflictor->z - (z + height));
                apz = (abs(apx) + abs(apy))>>1;
                
                if(delta1 >= delta2 && inflictor->pz < 0)
		  apz = -apz;
	      }
	  pz += apz;
#ifdef CLIENTPREDICTION2
	  if (p && p->spirit)
	    {
	      p->spirit->px += apx;
	      p->spirit->py += apy;
	      p->spirit->pz += apz;
	    }
#endif  
        }
    }

  
  // do the damage
  health -= damage;
  if (health <= 0)
    {
      special1 = damage;

      Kill(inflictor, source);
      return true;
    }

  if ( (P_Random () < info->painchance)
       && !(flags & (MF_SKULLFLY|MF_CORPSE)) )
    {
      flags |= MF_JUSTHIT;    // fight back!
      SetState(info->painstate);
    }

  reactiontime = 0;           // we're awake now...


  if ( (!threshold || type == MT_VILE)
       && source && source != target
       && source->type != MT_VILE
       && !(source->flags2 & MF2_BOSS)
       && !(type == MT_SORCERER2 && source->type == MT_WIZARD))
    {
      // if not intent on another player,
      // chase after this one
      target = source;
      threshold = BASETHRESHOLD;
      if (state == &states[info->spawnstate]
	  && info->seestate != S_NULL)
	SetState(info->seestate);
    }

  return true;
}