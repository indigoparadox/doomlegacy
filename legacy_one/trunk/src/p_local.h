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
// $Log: p_local.h,v $
// Revision 1.24  2004/07/27 08:19:36  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.23  2003/06/11 00:28:50  ssntails
// Big Blockmap Support (128kb+ ?)
//
// Revision 1.22  2003/03/22 22:35:59  hurdler
// Revision 1.21  2002/09/27 16:40:09  tonyd
// First commit of acbot
//
// Revision 1.20  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.19  2001/07/28 16:18:37  bpereira
//
// Revision 1.18  2001/07/16 22:35:41  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.17  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.16  2001/03/21 18:24:38  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.15  2001/02/10 12:27:14  bpereira
//
// Revision 1.14  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.13  2000/11/03 02:37:36  stroggonmeth
// Fix a few warnings when compiling.
//
// Revision 1.12  2000/11/02 19:49:35  bpereira
//
// Revision 1.11  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.10  2000/10/21 08:43:30  bpereira
// Revision 1.9  2000/08/31 14:30:55  bpereira
// Revision 1.8  2000/04/16 18:38:07  bpereira
//
// Revision 1.7  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.6  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.5  2000/04/08 17:29:24  stroggonmeth
//
// Revision 1.4  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.3  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Play functions, animation, global header.
//
//-----------------------------------------------------------------------------


#ifndef P_LOCAL_H
#define P_LOCAL_H

#include "doomdef.h"
  // VOODOO_DOLL, DOORDELAY_CONTROL
#include "doomtype.h"
  // tic_t
#include "command.h"
  // consvar_t
#include "d_player.h"
#include "d_think.h"
#include "m_fixed.h"
#include "m_bbox.h"
#include "p_tick.h"
#include "p_mobj.h"
#include "r_defs.h"
#include "p_maputl.h"


#define FLOATSPEED              (FRACUNIT*4)

// added by Boris : for dehacked patches, replaced #define by int
extern int MAXHEALTH;   // 100

#define VIEWHEIGHT               41
#define VIEWHEIGHTS             "41"

// default viewheight is changeable at console
extern consvar_t cv_viewheight; // p_mobj.c

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS   128
#define MAPBLOCKSIZE    (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT   (FRACBITS+7)
#define MAPBMASK        (MAPBLOCKSIZE-1)
#define MAPBTOFRAC      (MAPBLOCKSHIFT-FRACBITS)


// player radius used only in am_map.c
#define PLAYERRADIUS    (16*FRACUNIT)

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
#define MAXRADIUS       (32*FRACUNIT)

#define MAXMOVE         (30*FRACUNIT/NEWTICRATERATIO)

//added:26-02-98: max Z move up or down without jumping
//      above this, a heigth difference is considered as a 'dropoff'
#define MAXSTEPMOVE     (24*FRACUNIT)

#define USERANGE        (64*FRACUNIT)
#define MELEERANGE      (64*FRACUNIT)
#define MISSILERANGE    (32*64*FRACUNIT)

// follow a player exlusively for 3 seconds
#define BASETHRESHOLD   100

//#define AIMINGTOSLOPE(aiming)   finetangent[(2048+(aiming>>ANGLETOFINESHIFT)) & FINEMASK]
#define AIMINGTOSLOPE(aiming)   finesine[(aiming>>ANGLETOFINESHIFT) & FINEMASK]

//26-07-98: p_mobj.c
extern  consvar_t cv_gravity;

#ifdef DOORDELAY_CONTROL
// [WDJ] 1/15/2009 support control of door and event delay. see p_doors.c
// init in r_main.c
extern  int  adj_ticks_per_sec;
extern  consvar_t  cv_doordelay;
#else
// standard ticks per second
#define adj_ticks_per_sec  35
#endif
#ifdef VOODOO_DOLL
// [WDJ] 2/7/2011 Voodoo doll controls and support
extern consvar_t  cv_instadeath;
extern consvar_t  cv_voodoo_mode;
typedef enum {
   VM_vanilla, VM_multispawn, VM_target, VM_auto
} voodoo_mode_e;
extern voodoo_mode_e  voodoo_mode;
extern player_t *  spechit_player; // last player to trigger switch or linedef
#endif

//
// P_TICK
//

// both the head and tail of the thinker list
extern  thinker_t       thinkercap;


void P_InitThinkers (void);
void P_AddThinker (thinker_t* thinker);
void P_RemoveThinker (thinker_t* thinker);


//
// P_PSPR
//
void P_SetupPsprites (player_t* curplayer);
void P_MovePsprites (player_t* curplayer);
void P_DropWeapon (player_t* player);


//
// P_USER
//
typedef struct camera_s
{
    player_t *  chase;  // player the camera chases, NULL when off
    mobj_t*     mo;     // the camera object
    angle_t     aiming;
    int         fixedcolormap;

    //SoM: Things used by FS cameras.
    fixed_t     viewheight;
    angle_t     startangle;
} camera_t;

extern camera_t camera;

extern consvar_t cv_cam_dist;  
extern consvar_t cv_cam_height;
extern consvar_t cv_cam_speed;

extern fixed_t jumpgravity;  // variable by fragglescipt


void   P_ResetCamera (player_t* player);
void   P_PlayerThink (player_t* player);

// client prediction
void   CL_ResetSpiritPosition (mobj_t *mobj);
void   P_MoveSpirit (player_t* p,ticcmd_t *cmd, int realtics);

//
// P_MOBJ
//
#define ONFLOORZ        FIXED_MIN
#define ONCEILINGZ      FIXED_MAX

// Time interval for item respawning.
// WARING MUST be a power of 2
#define ITEMQUESIZE     128

extern mapthing_t     *itemrespawnque[ITEMQUESIZE];
extern tic_t          itemrespawntime[ITEMQUESIZE];
extern int              iquehead;
extern int              iquetail;


void P_RespawnSpecials (void);
void P_RespawnWeapons(void);

mobj_t*  P_SpawnMobj ( fixed_t x, fixed_t y, fixed_t z, mobjtype_t type );

// Morph control flags
typedef enum {
   MM_testsize = 0x01,
   MM_telefog  = 0x02
} morphmobj_e;
// Change the type and info, but keep the location and player.
boolean P_MorphMobj( mobj_t * mo, mobjtype_t type, int mmflags, int keepflags );

void    P_RemoveMobj (mobj_t* th);
boolean P_SetMobjState (mobj_t* mobj, statenum_t state);
void    P_MobjThinker (mobj_t* mobj);

//spawn splash at surface of water in sector where the mobj resides
void    P_SpawnSplash (mobj_t* mo, fixed_t z);
//Fab: when fried in in lava/slime, spawn some smoke
void    P_SpawnSmoke (fixed_t x, fixed_t y, fixed_t z);

void    P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z);
void    P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage);
void    P_SpawnBloodSplats (fixed_t x, fixed_t y, fixed_t z, int damage, fixed_t momx, fixed_t momy);
mobj_t *P_SpawnMissile (mobj_t* source, mobj_t* dest, mobjtype_t type);

mobj_t *P_SPMAngle ( mobj_t* source, mobjtype_t type, angle_t angle );
#define P_SpawnPlayerMissile(s,t) P_SPMAngle(s,t,s->angle)

//
// P_ENEMY
//
extern  byte EN_mon_infight;   // monster to monster fighting
extern  byte EN_mon_coop;  // monster coop

extern  consvar_t cv_monbehavior;
extern  consvar_t cv_monsterfriction;
extern  consvar_t cv_doorstuck;

// when pushing a line 
//#define MAXSPECIALCROSS 16

extern  int     *spechit;                //SoM: 3/15/2000: Limit removal
extern  int     numspechit;

void P_NoiseAlert (mobj_t* target, mobj_t* emmiter);

void P_UnsetThingPosition (mobj_t* thing);
void P_SetThingPosition (mobj_t* thing);

// init braintagets position
void P_InitBrainTarget();

//
// P_MAP
//

// TryMove, thing map global vars
extern fixed_t  tm_bbox[4];	// box around the thing
extern mobj_t*  tm_thing;	// the thing itself
extern int      tm_flags;	// thing flags of tm_thing
extern fixed_t  tm_x, tm_y;	// thing map position

// TryMove, thing map response global vars
// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern boolean  tmr_floatok;  // floating thing ok to move
extern fixed_t  tmr_floorz;   // floor and ceiling of new position
extern fixed_t  tmr_ceilingz;
extern fixed_t  tmr_sectorceilingz;      //added:28-02-98: p_spawnmobj
extern mobj_t*  tmr_floorthing;   // standing on another thing
extern line_t*  tmr_ceilingline;  // line that lowers ceiling, for missile sky test
extern line_t*  tmr_blockingline; // stopping line that is solid
extern line_t*  tmr_dropoffline;  // line that is dropoff edge

extern  msecnode_t*     sector_list;

// P_CheckPosition, P_TryMove, P_CheckCrossLine
// use tm_ global vars, and return tmr_ global vars
boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y);
boolean P_TryMove (mobj_t* thing, fixed_t x, fixed_t y, boolean allowdropoff);
boolean P_CheckCrossLine (mobj_t* thing, fixed_t x, fixed_t y);

boolean P_TeleportMove (mobj_t* thing, fixed_t x, fixed_t y);
void    P_SlideMove (mobj_t* mo);

boolean P_CheckSight (mobj_t* t1, mobj_t* t2);
boolean P_CheckSight2 (mobj_t* t1, mobj_t* t2, fixed_t x, fixed_t y, fixed_t z);	//added by AC for predicting
void    P_UseLines (player_t* player);

boolean P_CheckSector(sector_t *sector, boolean crunch);
boolean P_ChangeSector (sector_t* sector, boolean crunch);

void    P_DelSeclist(msecnode_t *);
void    P_CreateSecNodeList(mobj_t*,fixed_t,fixed_t);
void    P_Initsecnode( void );

extern fixed_t  got_friction;
extern int      got_movefactor;  // return values of P_GetFriction and P_GetMoveFactor
fixed_t P_GetFriction( const mobj_t * mo );
int     P_GetMoveFactor(mobj_t* mo);

// Line Attack return global vars  lar_*
extern mobj_t*  lar_linetarget;  // who got hit (or NULL)
extern fixed_t  la_attackrange;  // max range of weapon

fixed_t P_AimLineAttack ( mobj_t* t1, angle_t angle, fixed_t distance );

void P_LineAttack ( mobj_t* t1, angle_t angle, fixed_t distance,
		    fixed_t slope, int damage );

void P_RadiusAttack ( mobj_t* spot, mobj_t* source, int damage );



//
// P_SETUP
//
extern byte*            rejectmatrix;   // for fast sight rejection
// Read wad blockmap using int16_t wadblockmaplump[].
// Expand from 16bit wad to internal 32bit blockmap.
extern uint32_t*        blockmaphead;   // offsets in blockmap are from here
extern uint32_t*        blockmapindex;  // Big blockmap, SSNTails
extern int              bmapwidth;
extern int              bmapheight;     // in mapblocks
extern fixed_t          bmaporgx;
extern fixed_t          bmaporgy;       // origin of block map
extern mobj_t**         blocklinks;     // for thing chains


//
// P_INTER
//
extern int              maxammo[NUMAMMO];
extern int              clipammo[NUMAMMO];

void P_TouchSpecialThing ( mobj_t* special, mobj_t* toucher );

boolean P_DamageMobj ( mobj_t* target, mobj_t* inflictor,
		       mobj_t* source, int damage );

//
// P_SIGHT
//

// slopes to top and bottom of target
extern fixed_t  see_topslope;
extern fixed_t  see_bottomslope;


//
// P_SPEC
//
#include "p_spec.h"



//SoM: 3/6/2000: Added public "boomsupport variable"
extern int boomsupport;
extern int variable_friction;
extern int allow_pushers;
extern byte  monster_friction;  // MBF demo flag
extern byte  mbf_support;  // [WDJ] MBF enable
			   // similar to prboom mbf_features, but as a flag

typedef enum {
 // Boom values
   INFT_none,     // Boom demo value, Doom default of no infight.
   INFT_infight,  // Boom demo value, Monster to monster fighting.
 // Enhanced values
   INFT_coop = 16,  // Legacy enhancement, and ZDoom option
   INFT_infight_off,  // DEH
} infight_e;
// Values from infight_e and from Boom demo.
extern byte  monster_infight; //DarkWolf95:November 21, 2003: Monsters Infight!
extern byte  monster_infight_deh; // DEH input.

typedef enum {
  FR_orig, FR_boom, FR_mbf, FR_prboom, FR_legacy, FR_heretic, FR_hexen
} friction_model_e;
extern friction_model_e  friction_model;

extern byte  boom_detect;
extern byte  legacy_detect;

void  DemoAdapt_p_user( void );
void  DemoAdapt_p_enemy( void );
void  DemoAdapt_p_floor( void );


// heretic specific
extern int ceilmovesound;
extern int doorclosesound;
extern int ArtifactFlash;

#define TELEFOGHEIGHT  (32*FRACUNIT)
extern mobjtype_t      PuffType;
#define FOOTCLIPSIZE   (10*FRACUNIT)
#define HITDICE(a) ((1+(P_Random()&7))*a)

#define MAXCHICKENHEALTH 30

#define BLINKTHRESHOLD  (4*32)
#define WPNLEV2TICS     (40*TICRATE)
#define FLIGHTTICS      (60*TICRATE)

#define CHICKENTICS     (40*TICRATE)
#define FLOATRANDZ      (MAXINT-1)

void P_RepositionMace(mobj_t *mo);
void P_ActivateBeak(player_t *player);
void P_PlayerUseArtifact(player_t *player, artitype_t arti);
void P_DSparilTeleport(mobj_t *actor);
void P_InitMonsters(void);
boolean P_LookForMonsters(mobj_t *actor);
int P_GetThingFloorType(mobj_t *thing);
mobj_t *P_CheckOnmobj(mobj_t *thing);
void P_AddMaceSpot(mapthing_t *mthing);
boolean P_SightPathTraverse (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
void P_HerePlayerInSpecialSector(player_t *player);
void P_UpdateBeak(player_t *player, pspdef_t *psp);
boolean P_TestMobjLocation(mobj_t *mobj);
void P_PostChickenWeapon(player_t *player, weapontype_t weapon);
void P_SetPsprite ( player_t*     player,
                    int           position,
                    statenum_t    stnum );
boolean P_UseArtifact(player_t *player, artitype_t arti);
boolean P_Teleport(mobj_t *thing, fixed_t x, fixed_t y, angle_t angle);
boolean P_SeekerMissile(mobj_t *actor, angle_t thresh, angle_t turnMax);
mobj_t *P_SpawnMissileAngle(mobj_t *source, mobjtype_t type,
        angle_t angle, fixed_t momz);
boolean P_SetMobjStateNF(mobj_t *mobj, statenum_t state);
boolean P_CheckMissileSpawn (mobj_t* th);
void P_ThrustMobj(mobj_t *mo, angle_t angle, fixed_t move);
void P_Thrust(player_t *player, angle_t angle, fixed_t move);
void P_ExplodeMissile (mobj_t* mo);
void P_SetMessage(player_t *player, char *message, boolean ultmsg);
boolean P_GiveArtifact(player_t *player, artitype_t arti, mobj_t *mo);
boolean P_ChickenMorphPlayer(player_t *player);
void P_Massacre(void);
void P_AddBossSpot(fixed_t x, fixed_t y, angle_t angle);

#endif  // P_LOCAL_H
