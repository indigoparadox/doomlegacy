// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2010 by DooM Legacy Team.
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
// $Log: p_saveg.c,v $
// Revision 1.29  2004/07/27 08:19:37  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.28  2004/04/20 00:34:26  andyp
// Linux compilation fixes and string cleanups
//
// Revision 1.27  2003/08/11 13:50:01  hurdler
// go final + translucent HUD + fix spawn in net game
//
// Revision 1.26  2003/07/21 11:33:57  hurdler
// go RC1
//
// Revision 1.25  2003/05/04 07:22:55  sburke
// Overlooked a LONG() swap when restoring the door->line of a door thinker.
//
// Revision 1.24  2003/05/04 02:36:18  sburke
// Make thorough use of READSHORT, READLONG macros to avoid endian, alignment faults.
//
// Revision 1.23  2002/09/09 20:41:53  uid22974
// Fix a bug with save/load game and FS
//
// Revision 1.22  2002/06/23 17:38:21  ssntails
// Fix for savegames with 3dfloors!
//
// Revision 1.21  2002/01/21 23:14:28  judgecutor
// Frag's Weapon Falling fixes
//
// Revision 1.20  2001/06/16 08:07:55  bpereira
// no message
//
// Revision 1.19  2001/06/10 21:16:01  bpereira
// no message
//
// Revision 1.18  2001/03/03 06:17:33  bpereira
// no message
//
// Revision 1.17  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.16  2001/02/10 12:27:14  bpereira
// no message
//
// Revision 1.15  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.14  2000/11/11 13:59:45  bpereira
// no message
//
// Revision 1.13  2000/11/04 16:23:43  bpereira
// no message
//
// Revision 1.12  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.11  2000/09/28 20:57:16  bpereira
// no message
//
// Revision 1.10  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.9  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.8  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.7  2000/04/15 22:12:57  stroggonmeth
// Minor bug fixes
//
// Revision 1.6  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.5  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
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
//      Archiving: SaveGame I/O.
//
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
  // offsetof

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_setup.h"
#include "byteptr.h"
#include "t_array.h"
#include "t_vari.h"
#include "t_script.h"
#include "t_func.h"
#include "m_random.h"
#include "d_items.h"
  // NUMWEAPONS, NUMAMMO, NUMPOWERS
#include "m_argv.h"
  // to save command line
#include "m_misc.h"
  // FIL_Filename_of

#include "p_saveg.h"

byte * save_p;
boolean  save_game_abort = 0;

// Pads save_p to a 4-byte boundary
//  so that the load/save works on SGI&Gecko.
#ifdef SGI
// BP: this stuff isn't be removed but i think it will no more work
//     anyway what processor can't read/write unaligned data ?
#define PADSAVEP()      save_p += (4 - ((int) save_p & 3)) & 3
const byte sg_padded = 1;
#else
#define PADSAVEP()
const byte sg_padded = 0;
#endif

// [WDJ] Sync byte with section identifier, so sections can be conditional
// Do not alter the values of these. They must have the same value over
// all save game versions they were used in.
typedef enum {
  // do not use 0, 1, 2, 254, 255
  SYNC_net = 3,
  SYNC_misc,
  SYNC_players,
  SYNC_world,
  SYNC_thinkers,
  SYNC_specials,	// 8
  // optionals that game may use
  SYNC_fragglescript = 70,
  // optional controls that may vary per game
  SYNC_gamma = 200,
  SYNC_slowdoor,
  // sync
  SYNC_end = 252,
  SYNC_sync = 253
} save_game_section_e;

void SG_SaveSync( save_game_section_e sgs )
{
    WRITEBYTE( save_p, SYNC_sync );	// validity check
    WRITEBYTE( save_p, sgs );	// section id
}

// required or conditional section
boolean SG_ReadSync( save_game_section_e sgs, boolean cond )
{
   if( save_game_abort )   return 0;	// all sync reads repeat the abort
   if( READBYTE( save_p ) != SYNC_sync )  goto invalid;
   if( READBYTE( save_p ) != sgs )
   {
      if( ! cond )  goto invalid;
      save_p -= 2;	// backup for re-reading the sync
      return 0;		// not the wanted sync
   }
   return 1;
   
 invalid:
   I_SoftError( "LoadGame: Invalid sync\n" );
   save_game_abort = 1;
   return 0;
}


// write null term string
void SG_write_string( const char * sp )
{
#if 1
    strcpy((char *)save_p, sp);
    save_p += strlen(sp) + 1;
#else   
    int i=0;
    do {
        WRITECHAR(save_p, sp[i]);
    } while (sp[i++]);	// until null
#endif   
}

// return string allocated using Z_Strdup, PU_LEVEL
char * SG_read_string( void )
{
    char * spdest;
    // Use strnlen (GNU) if you have it, otherwise have to use strlen
    // Do not define strnlen as strlen somewhere else, because that hides the
    // vulnerability.
#ifdef __USE_GNU
    // better protection against buffer overrun
    int len = strnlen( (char*)save_p, 258 ) + 1;	// incl term 0
#else   
    int len = strlen( (char*)save_p ) + 1;	// incl term 0
#endif
    // Protect against unterm string in file
    if( len > 256 )  return NULL;  // error
    spdest = Z_Strdup((char *)save_p, PU_LEVEL, NULL);
//    spdest = Z_Malloc( len, PU_LEVEL, 0 );
//    strcpy( spdest, save_p );
    save_p += len;
    return spdest;
}

// write fixed length string
void SG_write_nstring( const char * sp, int field_length )
{
#if 1
    strncpy((char *)save_p, sp, field_length);  // padded with nulls
    save_p += field_length;
#else   
    int i;
    for( i=0; i<field_length; i++ ) {
        if( sp[i] == 0 ) break;	// end of string
        WRITECHAR(save_p, sp[i]);
    }
    for( ; i<field_length; i++ ) {
        WRITECHAR(save_p, 0 );  // padding
    }
#endif   
}

// return string allocated using Z_Strdup, PU_LEVEL
char * SG_read_nstring( int field_length )
{
    char * spdest = Z_Malloc( field_length, PU_LEVEL, NULL );
    strncpy( spdest, save_p, field_length );
    save_p += field_length;
    return spdest;
}


int num_thinkers;       // number of thinkers in level being archived


typedef enum
{
    // weapons   = 0x01ff,
    BACKPACK = 0x0200,
    ORIGNWEAP = 0x0400,
    AUTOAIM = 0x0800,
    ATTACKDWN = 0x1000,
    USEDWN = 0x2000,
    JMPDWN = 0x4000,
    DIDSECRET = 0x8000,
} player_saveflags;

typedef enum
{
    // powers      = 0x00ff
    PD_REFIRE = 0x0100,
    PD_KILLCOUNT = 0x0200,
    PD_ITEMCOUNT = 0x0400,
    PD_SECRETCOUNT = 0x0800,
    PD_DAMAGECOUNT = 0x1000,
    PD_BONUSCOUNT = 0x2000,
    PD_CHICKENTICS = 0x4000,
    PD_CHICKENPECK = 0x8000,
    PD_FLAMECOUNT = 0x10000,
    PD_FLYHEIGHT = 0x20000,
} player_diff;

//
// P_ArchivePlayers
//
void P_ArchivePlayers(void)
{
    int i, j;
    int flags;
    uint32_t diff;
    player_t * ply;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (!playeringame[i])
            continue;

        ply = &players[i];

        PADSAVEP();
       
        flags = 0;
        diff = 0;
        for (j = 0; j < NUMPOWERS; j++)
            if (ply->powers[j])
                diff |= 1 << j;
        if (ply->refire)
            diff |= PD_REFIRE;
        if (ply->killcount)
            diff |= PD_KILLCOUNT;
        if (ply->itemcount)
            diff |= PD_ITEMCOUNT;
        if (ply->secretcount)
            diff |= PD_SECRETCOUNT;
        if (ply->damagecount)
            diff |= PD_DAMAGECOUNT;
        if (ply->bonuscount)
            diff |= PD_BONUSCOUNT;
        if (ply->chickenTics)
            diff |= PD_CHICKENTICS;
        if (ply->chickenPeck)
            diff |= PD_CHICKENPECK;
        if (ply->flamecount)
            diff |= PD_FLAMECOUNT;
        if (ply->flyheight)
            diff |= PD_FLYHEIGHT;

        WRITEULONG(save_p, diff);

        WRITEANGLE(save_p, ply->aiming);
        WRITEUSHORT(save_p, ply->health);
        WRITEUSHORT(save_p, ply->armorpoints);
        WRITEBYTE(save_p, ply->armortype);

        for (j = 0; j < NUMPOWERS; j++)
        {
            if (diff & (1 << j))
                WRITELONG(save_p, ply->powers[j]);
        }
        WRITEBYTE(save_p, ply->cards);
        WRITEBYTE(save_p, ply->readyweapon);
        WRITEBYTE(save_p, ply->pendingweapon);
        WRITEBYTE(save_p, ply->playerstate);

        WRITEUSHORT(save_p, ply->addfrags);
        for (j = 0; j < MAXPLAYERS; j++)
        {
            if (playeringame[j])	// [WDJ] was [i] which was useless
                WRITEUSHORT(save_p, ply->frags[j]);
        }

        for (j = 0; j < NUMWEAPONS; j++)
        {
            WRITEBYTE(save_p, ply->favoritweapon[j]);
            if (ply->weaponowned[j])
                flags |= 1 << j;
        }
        for (j = 0; j < NUMAMMO; j++)
        {
            WRITEUSHORT(save_p, ply->ammo[j]);
            WRITEUSHORT(save_p, ply->maxammo[j]);
        }
        if (ply->backpack)
            flags |= BACKPACK;
        if (ply->originalweaponswitch)
            flags |= ORIGNWEAP;
        if (ply->autoaim_toggle)
            flags |= AUTOAIM;
        if (ply->attackdown)
            flags |= ATTACKDWN;
        if (ply->usedown)
            flags |= USEDWN;
        if (ply->jumpdown)
            flags |= JMPDWN;
        if (ply->didsecret)
            flags |= DIDSECRET;

        if (diff & PD_REFIRE)
            WRITELONG(save_p, ply->refire);
        if (diff & PD_KILLCOUNT)
            WRITELONG(save_p, ply->killcount);
        if (diff & PD_ITEMCOUNT)
            WRITELONG(save_p, ply->itemcount);
        if (diff & PD_SECRETCOUNT)
            WRITELONG(save_p, ply->secretcount);
        if (diff & PD_DAMAGECOUNT)
            WRITELONG(save_p, ply->damagecount);
        if (diff & PD_BONUSCOUNT)
            WRITELONG(save_p, ply->bonuscount);
        if (diff & PD_CHICKENTICS)
            WRITELONG(save_p, ply->chickenTics);
        if (diff & PD_CHICKENPECK)
            WRITELONG(save_p, ply->chickenPeck);
        if (diff & PD_FLAMECOUNT)
            WRITELONG(save_p, ply->flamecount);
        if (diff & PD_FLYHEIGHT)
            WRITELONG(save_p, ply->flyheight);

        WRITEBYTE(save_p, ply->skincolor);

        for (j = 0; j < NUMPSPRITES; j++)
        {
	    pspdef_t * psp = & ply->psprites[j];
            if (psp->state)
                WRITEUSHORT(save_p, (psp->state - states) + 1);
            else
                WRITEUSHORT(save_p, 0);
            WRITELONG(save_p, psp->tics);
            WRITEFIXED(save_p, psp->sx);
            WRITEFIXED(save_p, psp->sy);
        }
        WRITEUSHORT(save_p, flags);

        if (inventory)
        {
            WRITEBYTE(save_p, ply->inventorySlotNum);
            for (j = 0; j < ply->inventorySlotNum; j++)
            {
                WRITEMEM(save_p, &ply->inventory[j], sizeof(ply->inventory[j]));
            }
        }
    }
}

//
// P_UnArchivePlayers
//
void P_UnArchivePlayers(void)
{
    int i, j;
    int flags;
    uint32_t diff;
    player_t * ply;
       

    for (i = 0; i < MAXPLAYERS; i++)
    {
        memset(&players[i], 0, sizeof(player_t));
        if (!playeringame[i])
            continue;

        ply = &players[i];

        PADSAVEP();
        diff = READULONG(save_p);

        ply->aiming = READANGLE(save_p);
        ply->health = READUSHORT(save_p);
        ply->armorpoints = READUSHORT(save_p);
        ply->armortype = READBYTE(save_p);

        for (j = 0; j < NUMPOWERS; j++)
        {
            if (diff & (1 << j))
                ply->powers[j] = READLONG(save_p);
        }

        ply->cards = READBYTE(save_p);
        ply->readyweapon = READBYTE(save_p);
        ply->pendingweapon = READBYTE(save_p);
        ply->playerstate = READBYTE(save_p);

        ply->addfrags = READUSHORT(save_p);
        for (j = 0; j < MAXPLAYERS; j++)
        {
            if (playeringame[j])	// [WDJ] was [i] which was useless
                ply->frags[j] = READUSHORT(save_p);
        }

        for (j = 0; j < NUMWEAPONS; j++)
            ply->favoritweapon[j] = READBYTE(save_p);
        for (j = 0; j < NUMAMMO; j++)
        {
            ply->ammo[j] = READUSHORT(save_p);
            ply->maxammo[j] = READUSHORT(save_p);
        }
        if (diff & PD_REFIRE)
            ply->refire = READLONG(save_p);
        if (diff & PD_KILLCOUNT)
            ply->killcount = READLONG(save_p);
        if (diff & PD_ITEMCOUNT)
            ply->itemcount = READLONG(save_p);
        if (diff & PD_SECRETCOUNT)
            ply->secretcount = READLONG(save_p);
        if (diff & PD_DAMAGECOUNT)
            ply->damagecount = READLONG(save_p);
        if (diff & PD_BONUSCOUNT)
            ply->bonuscount = READLONG(save_p);
        if (diff & PD_CHICKENTICS)
            ply->chickenTics = READLONG(save_p);
        if (diff & PD_CHICKENPECK)
            ply->chickenPeck = READLONG(save_p);
        if (diff & PD_FLAMECOUNT)
            ply->flamecount = READLONG(save_p);
        if (diff & PD_FLYHEIGHT)
            ply->flyheight = READLONG(save_p);

        ply->skincolor = READBYTE(save_p);

        for (j = 0; j < NUMPSPRITES; j++)
        {
	    pspdef_t * psp = & ply->psprites[j];
            flags = READUSHORT(save_p);
            if (flags)
                psp->state = &states[flags - 1];

            psp->tics = READLONG(save_p);
            psp->sx = READFIXED(save_p);
            psp->sy = READFIXED(save_p);
        }

        flags = READUSHORT(save_p);

        if (inventory)
        {
            ply->inventorySlotNum = READBYTE(save_p);
            for (j = 0; j < ply->inventorySlotNum; j++)
            {
                READMEM(save_p, &ply->inventory[j], sizeof(ply->inventory[j]));
            }
        }

        for (j = 0; j < NUMWEAPONS; j++)
            ply->weaponowned[j] = (flags & (1 << j)) != 0;

        ply->backpack = (flags & BACKPACK) != 0;
        ply->originalweaponswitch = (flags & ORIGNWEAP) != 0;
        ply->autoaim_toggle = (flags & AUTOAIM) != 0;
        ply->attackdown = (flags & ATTACKDWN) != 0;
        ply->usedown = (flags & USEDWN) != 0;
        ply->jumpdown = (flags & JMPDWN) != 0;
        ply->didsecret = (flags & DIDSECRET) != 0;

        ply->viewheight = cv_viewheight.value << FRACBITS;
        if (gamemode == heretic)
        {
            if (ply->powers[pw_weaponlevel2])
                ply->weaponinfo = wpnlev2info;
            else
                ply->weaponinfo = wpnlev1info;
        }
        else
            ply->weaponinfo = doomweaponinfo;
    }
}

#define SD_FLOORHT     0x01
#define SD_CEILHT      0x02
#define SD_FLOORPIC    0x04
#define SD_CEILPIC     0x08
#define SD_LIGHT       0x10
#define SD_SPECIAL     0x20
#define SD_DIFF2       0x40

//SoM: 4/10/2000: Fix sector related savegame bugs
// diff2 flags
#define SD_FXOFFS     0x01
#define SD_FYOFFS     0x02
#define SD_CXOFFS     0x04
#define SD_CYOFFS     0x08
#define SD_STAIRLOCK  0x10
#define SD_PREVSEC    0x20
#define SD_NEXTSEC    0x40

#define LD_FLAG     0x01
#define LD_SPECIAL  0x02
//#define LD_TAG      0x04
#define LD_S1TEXOFF 0x08
#define LD_S1TOPTEX 0x10
#define LD_S1BOTTEX 0x20
#define LD_S1MIDTEX 0x40
#define LD_DIFF2    0x80

// diff2 flags
#define LD_S2TEXOFF 0x01
#define LD_S2TOPTEX 0x02
#define LD_S2BOTTEX 0x04
#define LD_S2MIDTEX 0x08

//
// P_ArchiveWorld
//
void P_ArchiveWorld(void)
{
    int i;
    int statsec = 0, statline = 0;
    line_t *li;
    side_t *si;
    byte *put;	// local copy of save_p, apparently for no reason
      // [WDJ] using a local var instead of global costs 800 bytes in obj
      // but saves 64 bytes in executable.
//#define put save_p

    // reload the map just to see difference
    mapsector_t *ms;
    mapsidedef_t *msd;
    maplinedef_t *mld;
    sector_t *ss;
    byte diff;
    byte diff2;

    // [WDJ] protect lump during this function
    ms = W_CacheLumpNum(lastloadedmaplumpnum + ML_SECTORS, PU_IN_USE);	// mapsectors temp
    // [WDJ] Fix endian as compare temp to internal.
    // 
    ss = sectors;
    put = save_p;

    for (i = 0; i < numsectors; i++, ss++, ms++)
    {
        // Save only how the sector differs from the wad.
        diff = 0;
        diff2 = 0;
        if (ss->floorheight != LE_SWAP16(ms->floorheight) << FRACBITS)
            diff |= SD_FLOORHT;
        if (ss->ceilingheight != LE_SWAP16(ms->ceilingheight) << FRACBITS)
            diff |= SD_CEILHT;
        //
        //  flats
        //
        // P_AddLevelFlat should not add but just return the number
        if (ss->floorpic != P_AddLevelFlat(ms->floorpic, levelflats))
            diff |= SD_FLOORPIC;
        if (ss->ceilingpic != P_AddLevelFlat(ms->ceilingpic, levelflats))
            diff |= SD_CEILPIC;

        if (ss->lightlevel != LE_SWAP16(ms->lightlevel))
            diff |= SD_LIGHT;
        if (ss->special != LE_SWAP16(ms->special))
            diff |= SD_SPECIAL;

        if (ss->floor_xoffs != 0)
            diff2 |= SD_FXOFFS;
        if (ss->floor_yoffs != 0)
            diff2 |= SD_FYOFFS;
        if (ss->ceiling_xoffs != 0)
            diff2 |= SD_CXOFFS;
        if (ss->ceiling_yoffs != 0)
            diff2 |= SD_CYOFFS;
        if (ss->stairlock < 0)
            diff2 |= SD_STAIRLOCK;
        if (ss->nextsec != -1)
            diff2 |= SD_NEXTSEC;
        if (ss->prevsec != -1)
            diff2 |= SD_PREVSEC;
        if (diff2)
            diff |= SD_DIFF2;

        if (diff)
        {
            statsec++;

            WRITESHORT(put, i);
            WRITEBYTE(put, diff);
            if (diff & SD_DIFF2)
                WRITEBYTE(put, diff2);
            if (diff & SD_FLOORHT)
                WRITEFIXED(put, ss->floorheight);
            if (diff & SD_CEILHT)
                WRITEFIXED(put, ss->ceilingheight);
            if (diff & SD_FLOORPIC)
            {
                memcpy(put, levelflats[ss->floorpic].name, 8);
                put += 8;
            }
            if (diff & SD_CEILPIC)
            {
                memcpy(put, levelflats[ss->ceilingpic].name, 8);
                put += 8;
            }
            if (diff & SD_LIGHT)
                WRITESHORT(put, (short) ss->lightlevel);
            if (diff & SD_SPECIAL)
                WRITESHORT(put, (short) ss->special);

            if (diff2 & SD_FXOFFS)
                WRITEFIXED(put, ss->floor_xoffs);
            if (diff2 & SD_FYOFFS)
                WRITEFIXED(put, ss->floor_yoffs);
            if (diff2 & SD_CXOFFS)
                WRITEFIXED(put, ss->ceiling_xoffs);
            if (diff2 & SD_CYOFFS)
                WRITEFIXED(put, ss->ceiling_yoffs);
            if (diff2 & SD_STAIRLOCK)
                WRITELONG(put, ss->stairlock);
            if (diff2 & SD_NEXTSEC)
                WRITELONG(put, ss->nextsec);
            if (diff2 & SD_PREVSEC)
                WRITELONG(put, ss->prevsec);
        }
    }
    WRITEUSHORT(put, 0xffff);  // mark end of world sector section

    mld = W_CacheLumpNum(lastloadedmaplumpnum + ML_LINEDEFS, PU_IN_USE); // linedefs temp
    msd = W_CacheLumpNum(lastloadedmaplumpnum + ML_SIDEDEFS, PU_IN_USE); // sidedefs temp
    // [WDJ] Fix endian as compare temp to internal.
    li = lines;
    // do lines
    for (i = 0; i < numlines; i++, mld++, li++)
    {
        diff = 0;
        diff2 = 0;

        // we don't care of map in deathmatch !
        if (((cv_deathmatch.value == 0) && (li->flags != LE_SWAP16(mld->flags))) || ((cv_deathmatch.value != 0) && ((li->flags & ~ML_MAPPED) != LE_SWAP16(mld->flags))))
            diff |= LD_FLAG;
        if (li->special != LE_SWAP16(mld->special))
            diff |= LD_SPECIAL;

        if (li->sidenum[0] != -1)
        {
	    mapsidedef_t * msd0 = &msd[li->sidenum[0]];
            si = &sides[li->sidenum[0]];
            if (si->textureoffset != LE_SWAP16(msd0->textureoffset) << FRACBITS)
                diff |= LD_S1TEXOFF;
            //SoM: 4/1/2000: Some textures are colormaps. Don't worry about invalid textures.
            if (R_CheckTextureNumForName(msd0->toptexture) != -1)
                if (si->toptexture != R_TextureNumForName(msd0->toptexture))
                    diff |= LD_S1TOPTEX;
            if (R_CheckTextureNumForName(msd0->bottomtexture) != -1)
                if (si->bottomtexture != R_TextureNumForName(msd0->bottomtexture))
                    diff |= LD_S1BOTTEX;
            if (R_CheckTextureNumForName(msd0->midtexture) != -1)
                if (si->midtexture != R_TextureNumForName(msd0->midtexture))
                    diff |= LD_S1MIDTEX;
        }
        if (li->sidenum[1] != -1)
        {
	    mapsidedef_t * msd1 = &msd[li->sidenum[1]];
            si = &sides[li->sidenum[1]];
            if (si->textureoffset != LE_SWAP16(msd1->textureoffset) << FRACBITS)
                diff2 |= LD_S2TEXOFF;
            if (R_CheckTextureNumForName(msd1->toptexture) != -1)
                if (si->toptexture != R_TextureNumForName(msd1->toptexture))
                    diff2 |= LD_S2TOPTEX;
            if (R_CheckTextureNumForName(msd1->bottomtexture) != -1)
                if (si->bottomtexture != R_TextureNumForName(msd1->bottomtexture))
                    diff2 |= LD_S2BOTTEX;
            if (R_CheckTextureNumForName(msd1->midtexture) != -1)
                if (si->midtexture != R_TextureNumForName(msd1->midtexture))
                    diff2 |= LD_S2MIDTEX;
            if (diff2)
                diff |= LD_DIFF2;

        }

        if (diff)
        {
            statline++;
            WRITESHORT(put, (short) i);
            WRITEBYTE(put, diff);
            if (diff & LD_DIFF2)
                WRITEBYTE(put, diff2);
            if (diff & LD_FLAG)
                WRITESHORT(put, li->flags);
            if (diff & LD_SPECIAL)
                WRITESHORT(put, li->special);

            si = &sides[li->sidenum[0]];
            if (diff & LD_S1TEXOFF)
                WRITEFIXED(put, si->textureoffset);
            if (diff & LD_S1TOPTEX)
                WRITESHORT(put, si->toptexture);
            if (diff & LD_S1BOTTEX)
                WRITESHORT(put, si->bottomtexture);
            if (diff & LD_S1MIDTEX)
                WRITESHORT(put, si->midtexture);

            si = &sides[li->sidenum[1]];
            if (diff2 & LD_S2TEXOFF)
                WRITEFIXED(put, si->textureoffset);
            if (diff2 & LD_S2TOPTEX)
                WRITESHORT(put, si->toptexture);
            if (diff2 & LD_S2BOTTEX)
                WRITESHORT(put, si->bottomtexture);
            if (diff2 & LD_S2MIDTEX)
                WRITESHORT(put, si->midtexture);
        }
    }
    WRITEUSHORT(put, 0xffff);  // mark end of world linedef section

    //CONS_Printf("sector saved %d/%d, line saved %d/%d\n",statsec,numsectors,statline,numlines);
    save_p = put;
    Z_ChangeTags_To( PU_IN_USE, PU_CACHE ); // now can free
}

//
// P_UnArchiveWorld
//
void P_UnArchiveWorld(void)
{
    int i;
    line_t *li;
    side_t *si;
    sector_t *secp;
    byte *get;
      // [WDJ] using a local var instead of global costs 736 bytes in obj
      // but saves 32 bytes in executable.
//#define get save_p
    byte diff, diff2;

    get = save_p;

    while (1)
    {
        i = READUSHORT(get);

        if (i == 0xffff) // end of world sector section
            break;

        diff = READBYTE(get);
        if (diff & SD_DIFF2)
            diff2 = READBYTE(get);
        else
            diff2 = 0;
        secp = & sectors[i];
        if (diff & SD_FLOORHT)
            secp->floorheight = READFIXED(get);
        if (diff & SD_CEILHT)
            secp->ceilingheight = READFIXED(get);
        if (diff & SD_FLOORPIC)
        {
	    secp->floorpic = P_AddLevelFlat((char *)get, levelflats);
            get += 8;
        }
        if (diff & SD_CEILPIC)
        {
            secp->ceilingpic = P_AddLevelFlat((char *)get, levelflats);
            get += 8;
        }
        if (diff & SD_LIGHT)
            secp->lightlevel = READSHORT(get);
        if (diff & SD_SPECIAL)
            secp->special = READSHORT(get);

        if (diff2 & SD_FXOFFS)
            secp->floor_xoffs = READFIXED(get);
        if (diff2 & SD_FYOFFS)
            secp->floor_yoffs = READFIXED(get);
        if (diff2 & SD_CXOFFS)
            secp->ceiling_xoffs = READFIXED(get);
        if (diff2 & SD_CYOFFS)
            secp->ceiling_yoffs = READFIXED(get);
        if (diff2 & SD_STAIRLOCK)
            secp->stairlock = READLONG(get);
        else
            secp->stairlock = 0;
        if (diff2 & SD_NEXTSEC)
            secp->nextsec = READLONG(get);
        else
            secp->nextsec = -1;
        if (diff2 & SD_PREVSEC)
            secp->prevsec = READLONG(get);
        else
            secp->prevsec = -1;
    }

    while (1)
    {
        i = READUSHORT(get);

        if (i == 0xffff)  // end of world linedef section
            break;
        diff = READBYTE(get);
        li = &lines[i];

        if (diff & LD_DIFF2)
            diff2 = READBYTE(get);
        else
            diff2 = 0;
        if (diff & LD_FLAG)
            li->flags = READSHORT(get);
        if (diff & LD_SPECIAL)
            li->special = READSHORT(get);

        si = &sides[li->sidenum[0]];
        if (diff & LD_S1TEXOFF)
            si->textureoffset = READFIXED(get);
        if (diff & LD_S1TOPTEX)
            si->toptexture = READSHORT(get);
        if (diff & LD_S1BOTTEX)
            si->bottomtexture = READSHORT(get);
        if (diff & LD_S1MIDTEX)
            si->midtexture = READSHORT(get);

        si = &sides[li->sidenum[1]];
        if (diff2 & LD_S2TEXOFF)
            si->textureoffset = READFIXED(get);
        if (diff2 & LD_S2TOPTEX)
            si->toptexture = READSHORT(get);
        if (diff2 & LD_S2BOTTEX)
            si->bottomtexture = READSHORT(get);
        if (diff2 & LD_S2MIDTEX)
            si->midtexture = READSHORT(get);
    }

    save_p = get;
}

//
// Thinkers
//


// [smite] A simple std::vector -style pointer-to-id mapping.
typedef struct
{
  mobj_t   *pointer;
} pointermap_cell_t;

typedef struct
{
  pointermap_cell_t *map; // array of cells
  unsigned int       len; // number of used cells in the array
  unsigned int alloc_len; // number of allocated cells
} pointermap_t;

static pointermap_t pointermap = {NULL,0,0};

// Safe to call without knowing if was allocated or not.
static void ClearPointermap()
{
  // Clean release of memory, setup for next call of Alloc_Pointermap 
  pointermap.len = 0;
  pointermap.alloc_len = 0;
  if( pointermap.map )  free(pointermap.map);
  pointermap.map = NULL;
}

// Allocate or reallocate
static boolean  Alloc_Pointermap( int num )
{
    unsigned int fi = pointermap.len; // first uninitialized cell
   
    pointermap.map = realloc(pointermap.map, num * sizeof(pointermap_cell_t));
    if( pointermap.map == NULL )
    {
      I_SoftError("LoadGame: Pointermap alloc failed.\n");
      save_game_abort = 1;  // will be detected by ReadSync
      return 0;
    }
    pointermap.alloc_len = num;
    // num is one past the last new cell
    memset(&pointermap.map[fi], 0, (num-fi) * sizeof(pointermap_cell_t));
    // All mapping has ID==0 map to NULL ptr.
    // This is less expensive than special tests.
    pointermap.map[0].pointer = NULL;	// Map id==0 to NULL
    pointermap.len = 1;
    return 1;
}

static void InitPointermap_Save(unsigned int size)
{
  pointermap.len = 1;  // all will be free, initialized to 0
  Alloc_Pointermap( size );
}

static void InitPointermap_Load(unsigned int size)
{
  InitPointermap_Save(size);
  // mark everything as initialized (this condition holds all the time during loading)
  // Does not affect anything, yet.
  pointermap.len = pointermap.alloc_len;
}



// Saving: Returns the ID number corresponding to a pointer.
// [WDJ] see  WritePtr( mobj_t *p );
static uint32_t GetID(mobj_t *p)
{
  uint32_t id;
   
  // All NULL ptrs are mapped to ID==0
  if (!p)
    return 0; // NULL ptr has id == 0

  // see if pointer is already there
  for (id=0; id < pointermap.len; id++)
    if (pointermap.map[id].pointer == p)  // use existing mapping
      return id;

  // okay, not there, we must add it

  // is there still space or should we enlarge the mapping table?
  if (pointermap.len == pointermap.alloc_len)
  {
    if( ! Alloc_Pointermap( pointermap.alloc_len * 2 ) )
       return 0; // alloc fail
  }

  // add the new pointer mapping
  id = pointermap.len++;
  pointermap.map[id].pointer = p;
  return id;
}


// Loading, first phase: Upon read of Mobj and the ID from the save game file.
// Sets the Mobj ID to pointer mapping.
static void MapMobjID(uint32_t id, mobj_t *p)
{
  // Cannot have mobj ptr p == NULL, that would be Z_Malloc failure.
  // Cannot have ID==0, that would be failure in mobj save, or bad file.
  // map[0] is preset to NULL
#if 1
  if (id == 0 || id > 500000)  goto bad_id_err;  // bad id, probably corrupt or wrong file.
#else
  if (!p)
    return; // NULL ptr has id == 0

  if (!id)
  {
    I_SoftError("LoadGame: Object with ID number 0.\n");
    goto failed;
  }
  if (id == 0 || id > 500000)	// bad id
  {
    I_SoftError("LoadGame: Object ID sanity check failed.\n");
    goto failed;
  }
#endif

  // is id in the initialized/allocated region?
  while (id >= pointermap.alloc_len)
  {
    // no, enlarge the container
    if( ! Alloc_Pointermap( pointermap.alloc_len * 2 ) )  goto failed;
    pointermap.len = pointermap.alloc_len; // all initialized
  }

  if (pointermap.map[id].pointer)  goto duplicate_err;  // already exists
  pointermap.map[id].pointer = p;  // save the mapping
  return;

bad_id_err:
  I_SoftError("LoadGame: Mobj read has bad object ID.\n");
  goto failed;

duplicate_err:
  I_SoftError("LoadGame: Same ID number found for several Mobj.\n");
  goto failed;

failed:
  save_game_abort = 1;
  return;
}


// Loading, second phase: Returns the pointer corresponding to the ID number.
// Only call after all the saved objects have been created, and are in map.
static mobj_t * GetMobjPointer(uint32_t id)
{
  // Less expensive to have map[0]==NULL than have special tests.
  // Is id in the initialized/allocated region? has it been assigned?
//  if (id >= pointermap.alloc_len || !pointermap.map[id].pointer)
  if ( id >= pointermap.alloc_len )   goto bad_ptr;
  mobj_t * mp = pointermap.map[id].pointer;	// [0] is NULL
  if( (mp == NULL) && (id > 0) )   goto bad_ptr;
  return mp;
   
bad_ptr:
  // on error, let user load a different save game
#if 1
  // Assume some mobj ptrs saved might not be valid, such as killed target.
  // This has been observed in a fresh saved game. 
  // Not fatal, return NULL ptr and continue;
  I_SoftError("LoadGame: Ptr to non-existant Mobj, make NULL.\n");
#else   
  // Assume a bad mobj ptr is a corrupt savegame.
  I_SoftError("LoadGame: Unknown Mobj ID number.\n");
  save_game_abort = 1;
#endif  
  return NULL;
}

#define WRITE_MobjPointerID(p,mobj)  WRITEULONG((p), GetID(mobj));
#define READ_MobjPointerID(p)   GetMobjPointer( READULONG(p) )
// No difference in executable
//#define READ_MobjPointerID_S(p,mobj)   (mobj) = GetMobjPointer( READULONG(p) )

// Save sector and line ptrs as index into their arrays
#define WRITE_SECTOR_PTR( secp )   WRITELONG(save_p, (secp) - sectors)
#define READ_SECTOR_PTR( secp )   (secp) = &sectors[READLONG(save_p)]
#define WRITE_LINE_PTR( linp )   WRITELONG(save_p, (linp) - lines)
#define READ_LINE_PTR( linp )   (linp) = &lines[READLONG(save_p)]
#define WRITE_MAPTHING_PTR( mtp )   WRITELONG(save_p, (mtp) - mapthings)
#define READ_MAPTHING_PTR( mtp )   (mtp) = &mapthings[READLONG(save_p)]
// another read of mapthing in P_UnArchiveSpecials





typedef enum
{
    MD_SPAWNPOINT = 0x000001,
    MD_POS = 0x000002,
    MD_TYPE = 0x000004,
// Eliminated MD_Z to prevent 3dfloor hiccups SSNTails 03-17-2002
    MD_MOM = 0x000008,
    MD_RADIUS = 0x000010,
    MD_HEIGHT = 0x000020,
    MD_FLAGS = 0x000040,
    MD_HEALTH = 0x000080,
    MD_RTIME = 0x000100,
    MD_STATE = 0x000200,
    MD_TICS = 0x000400,
    MD_SPRITE = 0x000800,
    MD_FRAME = 0x001000,
    MD_EFLAGS = 0x002000,
    MD_PLAYER = 0x004000,
    MD_MOVEDIR = 0x008000,
    MD_MOVECOUNT = 0x010000,
    MD_THRESHOLD = 0x020000,
    MD_LASTLOOK = 0x040000,
    MD_TARGET = 0x080000,
    MD_TRACER = 0x100000,
    MD_FRICTION = 0x200000,
    MD_MOVEFACTOR = 0x400000,
    MD_FLAGS2 = 0x800000,
    MD_SPECIAL1 = 0x1000000,
    MD_SPECIAL2 = 0x2000000,
    MD_AMMO = 0x4000000,
} mobj_diff_t;

enum
{
    tc_end = 1,	// reserved type mark to end section
    // Changing order will invalidate all previous save games.
    tc_mobj,
    tc_ceiling,
    tc_door,
    tc_floor,
    tc_plat,
    tc_flash,
    tc_strobe,
    tc_glow,
    tc_fireflicker,
    tc_lightfade,
    tc_elevator,                //SoM: 3/15/2000: Add extra boom types.
    tc_scroll,
    tc_friction,
    tc_pusher
     // add new values only at end
} specials_e;

//
// P_ArchiveThinkers
//
//
// Things to handle:
//
// P_MobjsThinker (all mobj)
// T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
// T_VerticalDoor, (vldoor_t: sector_t * swizzle),
// T_MoveFloor, (floormove_t: sector_t * swizzle),
// T_LightFlash, (lightflash_t: sector_t * swizzle),
// T_StrobeFlash, (strobe_t: sector_t *),
// T_Glow, (glow_t: sector_t *),
// T_LightFade, (lightlevel_t: sector_t *),
// T_PlatRaise, (plat_t: sector_t *), - active list
// BP: added missing : T_FireFlicker
//

#if 0
// [smite] Safe sectoreffect saving and loading using a macro hack.
// [WDJ] comment: This is fragile. It depends upon the struct definitions
// of all the sector effects having the thinker_t links and sector ptr as
// the first fields.
// There will be no compiler errors when this fails !!
// It does not save the thinker_t fields (links) of the structure, nor
// the sector ptr.  It writes a sector index before raw writing the
// structure fields that are after the thinker_t and sector ptr.
// It does not handle the ceilinglist ptr.
#define SE_HEADER_SIZE (sizeof(thinker_t) + sizeof(sector_t*))
#define SAVE_SE(th) \
  { int s = sizeof(*th) - SE_HEADER_SIZE;	      \
    WRITELONG(save_p, (th)->sector - sectors);	      \
    WRITEMEM(save_p, ((byte *)(th))+SE_HEADER_SIZE, s); }

#define LOAD_SE(th) \
  { int s = sizeof(*th) - SE_HEADER_SIZE;	      \
    (th)->sector = &sectors[READLONG(save_p)];	      \
    READMEM(save_p, ((byte *)(th))+SE_HEADER_SIZE, s);\
    P_AddThinker(&(th)->thinker); }

#define WRITE_THINKER(th) { WRITEMEM(save_p, ((byte *)(th))+sizeof(thinker_t), sizeof(*th)-sizeof(thinker_t)); }
#define READ_THINKER(th) {  READMEM(save_p, ((byte *)(th))+sizeof(thinker_t), sizeof(*th)-sizeof(thinker_t)); P_AddThinker(&(th)->thinker); }

#endif // smite


// [WDJ] comment: This is fragile. It depends upon the struct definitions
// of all the sector effects having the thinker_t links and sector ptr as
// the first fields.
// There will be no compiler errors when this fails !!
// It writes the thinker fields raw, from the start field, to the end
// of the structure.

// Include writing and reading the sector ptr.
#define WRITE_SECTOR_THINKER(th, typ, field) \
  { int offset = offsetof( typ, field );\
    WRITE_SECTOR_PTR( (th)->sector );\
    WRITEMEM(save_p, ((byte *)(th))+offset, (sizeof(*th)-offset)); }

#define READ_SECTOR_THINKER(th, typ, field) \
  { int offset = offsetof( typ, field );\
    READ_SECTOR_PTR( (th)->sector );\
    READMEM(save_p, ((byte *)(th))+offset, (sizeof(*th)-offset));\
    P_AddThinker(&(th)->thinker); }

// No extra fields
#define WRITE_THINKER(th, typ, field) \
  { int offset = offsetof( typ, field );\
    WRITEMEM(save_p, ((byte *)(th))+offset, (sizeof(*th)-offset)); }

#define READ_THINKER(th, typ, field) \
  { int offset = offsetof( typ, field );\
    READMEM(save_p, ((byte *)(th))+offset, (sizeof(*th)-offset));\
    P_AddThinker(&(th)->thinker); }



// Called for stopped ceiling or active ceiling.
// Must be consistent, there is one reader for both.
void  WRITE_ceiling( ceiling_t* ceilp, byte active )
{
    WRITEBYTE(save_p, tc_ceiling); // ceiling marker
    PADSAVEP();
    WRITE_SECTOR_THINKER( ceilp, ceiling_t, type );
    // ceilinglist* does not need to be saved
    WRITEBYTE(save_p, active); // active or stopped ceiling
}


// Called for stopped platform or active platform.
// Must be consistent, there is one reader for both.
void  WRITE_plat( plat_t* platp, byte active )
{
    WRITEBYTE(save_p, tc_plat);  // platform marker
    PADSAVEP();
    WRITE_SECTOR_THINKER( platp, plat_t, type );
    // platlist* does not need to be saved
    WRITEBYTE(save_p, active); // active or stopped plat
}


void P_ArchiveThinkers(void)
{
    thinker_t *th;
    mobj_t *mobj;
    uint32_t diff;

    // save the current thinkers
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1) P_MobjThinker)
        {
	    // Mobj thinker
            mobj = (mobj_t *) th;
/*
            // not a monster nor a pickable item so don't save it
            if( (((mobj->flags & (MF_COUNTKILL | MF_PICKUP | MF_SHOOTABLE )) == 0)
                 && (mobj->flags & MF_MISSILE)
                 && (mobj->info->doomednum !=-1) )
                || (mobj->type == MT_BLOOD) )
                continue;
*/
            if (mobj->spawnpoint && (!(mobj->spawnpoint->options & MTF_FS_SPAWNED)) && (mobj->info->doomednum != -1))
            {
                // spawnpoint is not modified but we must save it since it is a indentifier
                diff = MD_SPAWNPOINT;

                if ((mobj->x != mobj->spawnpoint->x << FRACBITS) || (mobj->y != mobj->spawnpoint->y << FRACBITS) || (mobj->angle != (unsigned) (ANG45 * (mobj->spawnpoint->angle / 45))))
                    diff |= MD_POS;
                if (mobj->info->doomednum != mobj->spawnpoint->type)
                    diff |= MD_TYPE;
            }
            else
            {
                // not a map spawned thing so make it from scratch
                diff = MD_POS | MD_TYPE;
            }

            // not the default but the most probable
            if ((mobj->momx != 0) || (mobj->momy != 0) || (mobj->momz != 0))
                diff |= MD_MOM;
            if (mobj->radius != mobj->info->radius)
                diff |= MD_RADIUS;
            if (mobj->height != mobj->info->height)
                diff |= MD_HEIGHT;
            if (mobj->flags != mobj->info->flags)
                diff |= MD_FLAGS;
            if (mobj->flags2 != mobj->info->flags2)
                diff |= MD_FLAGS2;
            if (mobj->health != mobj->info->spawnhealth)
                diff |= MD_HEALTH;
            if (mobj->reactiontime != mobj->info->reactiontime)
                diff |= MD_RTIME;
            if (mobj->state - states != mobj->info->spawnstate)
                diff |= MD_STATE;
            if (mobj->tics != mobj->state->tics)
                diff |= MD_TICS;
            if (mobj->sprite != mobj->state->sprite)
                diff |= MD_SPRITE;
            if (mobj->frame != mobj->state->frame)
                diff |= MD_FRAME;
            if (mobj->eflags)
                diff |= MD_EFLAGS;
            if (mobj->player)
                diff |= MD_PLAYER;

            if (mobj->movedir)
                diff |= MD_MOVEDIR;
            if (mobj->movecount)
                diff |= MD_MOVECOUNT;
            if (mobj->threshold)
                diff |= MD_THRESHOLD;
            if (mobj->lastlook != -1)
                diff |= MD_LASTLOOK;
            if (mobj->target)
                diff |= MD_TARGET;
            if (mobj->tracer)
                diff |= MD_TRACER;
            if (mobj->friction != ORIG_FRICTION)
                diff |= MD_FRICTION;
            if (mobj->movefactor != ORIG_FRICTION_FACTOR)
                diff |= MD_MOVEFACTOR;
            if (mobj->special1)
                diff |= MD_SPECIAL1;
            if (mobj->special2)
                diff |= MD_SPECIAL2;
            if (mobj->dropped_ammo_count)
                diff |= MD_AMMO;

            PADSAVEP();
            WRITEBYTE(save_p, tc_mobj);	// mark as mobj
            WRITEULONG(save_p, diff);
            // Save ID number of this Mobj so that pointers can be restored.
	    // NOTE: does not check if this mobj has been already saved, so it'd better not appear twice.
	    WRITE_MobjPointerID(save_p, mobj);
            WRITEFIXED(save_p, mobj->z);        // Force this so 3dfloor problems don't arise. SSNTails 03-17-2002
            WRITEFIXED(save_p, mobj->floorz);

            if (diff & MD_SPAWNPOINT)
                WRITE_MAPTHING_PTR( mobj->spawnpoint );
            if (diff & MD_TYPE)
                WRITEULONG(save_p, mobj->type);
            if (diff & MD_POS)
            {
                WRITEFIXED(save_p, mobj->x);
                WRITEFIXED(save_p, mobj->y);
                WRITEANGLE(save_p, mobj->angle);
            }
            if (diff & MD_MOM)
            {
                WRITEFIXED(save_p, mobj->momx);
                WRITEFIXED(save_p, mobj->momy);
                WRITEFIXED(save_p, mobj->momz);
            }
            if (diff & MD_RADIUS)
                WRITEFIXED(save_p, mobj->radius);
            if (diff & MD_HEIGHT)
                WRITEFIXED(save_p, mobj->height);
            if (diff & MD_FLAGS)
                WRITELONG(save_p, mobj->flags);
            if (diff & MD_FLAGS2)
                WRITELONG(save_p, mobj->flags2);
            if (diff & MD_HEALTH)
                WRITELONG(save_p, mobj->health);
            if (diff & MD_RTIME)
                WRITELONG(save_p, mobj->reactiontime);
            if (diff & MD_STATE)
                WRITEUSHORT(save_p, mobj->state - states);
            if (diff & MD_TICS)
                WRITELONG(save_p, mobj->tics);
            if (diff & MD_SPRITE)
                WRITEUSHORT(save_p, mobj->sprite);
            if (diff & MD_FRAME)
                WRITEULONG(save_p, mobj->frame);
            if (diff & MD_EFLAGS)
                WRITEULONG(save_p, mobj->eflags);
            if (diff & MD_PLAYER)
                WRITEBYTE(save_p, mobj->player - players);
            if (diff & MD_MOVEDIR)
                WRITELONG(save_p, mobj->movedir);
            if (diff & MD_MOVECOUNT)
                WRITELONG(save_p, mobj->movecount);
            if (diff & MD_THRESHOLD)
                WRITELONG(save_p, mobj->threshold);
            if (diff & MD_LASTLOOK)
                WRITELONG(save_p, mobj->lastlook);
            if (diff & MD_TARGET)
	        WRITE_MobjPointerID(save_p, mobj->target);
            if (diff & MD_TRACER)
	        WRITE_MobjPointerID(save_p, mobj->tracer);
            if (diff & MD_FRICTION)
                WRITELONG(save_p, mobj->friction);
            if (diff & MD_MOVEFACTOR)
                WRITELONG(save_p, mobj->movefactor);
            if (diff & MD_SPECIAL1)
                WRITELONG(save_p, mobj->special1);
            if (diff & MD_SPECIAL2)
                WRITELONG(save_p, mobj->special2);
            if (diff & MD_AMMO)
                WRITELONG(save_p, mobj->dropped_ammo_count);
        }
        // Use action as determinant of its owner.
	// acv == -1  means deallocated (see P_RemoveThinker)
        else if (th->function.acv == (actionf_v) NULL)
        {
	    // No action function.
	    // This thinker can be a stopped ceiling or platform.
	    // Each sector action has a separate thinker.

	    boolean done = false;
            //SoM: 3/15/2000: Boom stuff...
            ceilinglist_t *cl;
	   
	    // search for this thinker being a stopped active ceiling
            for (cl = activeceilings; cl; cl = cl->next)
	    {
                if (cl->ceiling == (ceiling_t *) th)  // found in ceilinglist
                {
		    WRITE_ceiling( (ceiling_t*)th, 0 ); // stopped ceiling 
		    done = true;
		    break;
                }
	    }

	    if (done)
	      continue;

	    // [smite] Added a similar search for stopped plats.
            platlist_t *pl;
	    // search for this thinker being a stopped active platform
	    for (pl = activeplats; pl; pl = pl->next)
	    {
                if (pl->plat == (plat_t *) th)  // found in platform list
                {
		    WRITE_plat( (plat_t *)th, 0 ); // stopped plat
		    break;
                }
	    }

            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_MoveCeiling)
        {
            WRITE_ceiling( (ceiling_t *)th, 1 );  // moving ceiling
            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_PlatRaise)
        {
	    WRITE_plat( (plat_t *)th, 1 ); // moving plat
            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_VerticalDoor)
        {
            WRITEBYTE(save_p, tc_door);  // door marker
            PADSAVEP();
	    vldoor_t *door = (vldoor_t *)th;
	    WRITE_SECTOR_THINKER( door, vldoor_t, type );
	    WRITE_LINE_PTR( door->line );
            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_MoveFloor)
        {
            WRITEBYTE(save_p, tc_floor);  // floor marker
            PADSAVEP();
	    floormove_t *floormv = (floormove_t *)th;
	    WRITE_SECTOR_THINKER( floormv, floormove_t, type );
            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_LightFlash)
        {
            WRITEBYTE(save_p, tc_flash);
            PADSAVEP();
            lightflash_t *flash = (lightflash_t *)th;
	    WRITE_SECTOR_THINKER( flash, lightflash_t, count );
            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_StrobeFlash)
        {
            WRITEBYTE(save_p, tc_strobe);
            PADSAVEP();
            strobe_t *strobe = (strobe_t *)th;
	    WRITE_SECTOR_THINKER( strobe, strobe_t, count );
            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_Glow)
        {
            WRITEBYTE(save_p, tc_glow);
            PADSAVEP();
            glow_t *glow = (glow_t *)th;
	    WRITE_SECTOR_THINKER( glow, glow_t, minlight );
            continue;
        }
        else
            // BP added T_FireFlicker
        if (th->function.acp1 == (actionf_p1) T_FireFlicker)
        {
            WRITEBYTE(save_p, tc_fireflicker);
            PADSAVEP();
            fireflicker_t *fireflicker = (fireflicker_t *)th;
	    WRITE_SECTOR_THINKER( fireflicker, fireflicker_t, count );
            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_LightFade)
        {
            WRITEBYTE(save_p, tc_lightfade);
            PADSAVEP();
            lightlevel_t *fade = (lightlevel_t *)th;
	    WRITE_SECTOR_THINKER( fade, lightlevel_t, destlevel );
            continue;
        }
        else
            //SoM: 3/15/2000: Added extra Boom thinker types.
        if (th->function.acp1 == (actionf_p1) T_MoveElevator)
        {
            WRITEBYTE(save_p, tc_elevator);
            PADSAVEP();
            elevator_t *elevator = (elevator_t *)th;
	    WRITE_SECTOR_THINKER( elevator, elevator_t, type );
	    continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_Scroll)
        {
            WRITEBYTE(save_p, tc_scroll);
	    scroll_t *scroll = (scroll_t *)th;
	    WRITE_THINKER( scroll, scroll_t, type );
            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_Friction)
        {
            WRITEBYTE(save_p, tc_friction);
	    friction_t *friction = (friction_t *)th;
	    WRITE_THINKER( friction, friction_t, affectee );
            continue;
        }
        else if (th->function.acp1 == (actionf_p1) T_Pusher)
        {
	    WRITEBYTE(save_p, tc_pusher);
	    pusher_t *pusher = (pusher_t *)th;
	    WRITE_THINKER( pusher, pusher_t, type );
            continue;
        }
#ifdef PARANOIA
        else if ((int) th->function.acp1 != -1) // wait garbage colection
        {
            I_SoftError("SaveGame: Unknown thinker type 0x%X", th->function.acp1);
	}
#endif

    }

    // mark the end of the save section using reserved type mark
    WRITEBYTE(save_p, tc_end);
    return;
}



//
// P_UnArchiveThinkers
//
void P_UnArchiveThinkers(void)
{
    mobj_t *mobj;
    uint32_t diff;
    int i;
    byte tclass;
    char * reason; // err
   
    // remove all the current thinkers
    thinker_t *currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        thinker_t * next = currentthinker->next;  // because of unlinking

        mobj = (mobj_t *) currentthinker;
        if (currentthinker->function.acp1 == (actionf_p1) P_MobjThinker)
        {
            // since this item isn't save don't remove it
/*            if( !((((mobj->flags & (MF_COUNTKILL | MF_PICKUP | MF_SHOOTABLE )) == 0)
                   && (mobj->flags & MF_MISSILE)
                   && (mobj->info->doomednum !=-1) )
                  || (mobj->type == MT_BLOOD) ) )
*/
            P_RemoveMobj((mobj_t *) currentthinker);
	}
        else
            Z_Free(currentthinker);

        currentthinker = next;
    }
    // BP: we don't want the removed mobj come back !!!
    iquetail = iquehead = 0;
    P_InitThinkers();

    // read in saved thinkers
    while (1)
    {
        tclass = READBYTE(save_p);
        if (tclass == tc_end)	// reserved type mark to end section
            break;      // leave the while
        switch (tclass)
        {
            case tc_mobj:

                mobj = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL);
                memset(mobj, 0, sizeof(mobj_t));

                PADSAVEP();
                diff = READULONG(save_p);
	        // [WDJ] initializing the lookup for GetMobjPointer and READ_MobjPointerID(),
	        // this is the id number for the mobj being read here.
                MapMobjID(READULONG(save_p), mobj); // assign the ID to the newly created mobj

                mobj->z = READFIXED(save_p);    // Force this so 3dfloor problems don't arise. SSNTails 03-17-2002
                mobj->floorz = READFIXED(save_p);

                if (diff & MD_SPAWNPOINT)
                {
		    READ_MAPTHING_PTR( mobj->spawnpoint );
                    mobj->spawnpoint->mobj = mobj;
                }
                if (diff & MD_TYPE)
                {
                    mobj->type = READULONG(save_p);
                }
                else //if (diff & MD_SPAWNPOINT) //Hurdler: I think we must add that test ?
                {
		    if( mobj->spawnpoint == NULL )
		    {
		        reason = "No Type and No Spawnpoint";
		        goto err_report;
		    }
                    for (i = 0; i < NUMMOBJTYPES; i++)
                        if (mobj->spawnpoint->type == mobjinfo[i].doomednum)
                            break;
                    if (i == NUMMOBJTYPES)
                    {
		        reason = "Spawnpoint type invalid";
		        goto err_report;
                    }
                    mobj->type = i;
                }
                mobj->info = &mobjinfo[mobj->type];
                if (diff & MD_POS)
                {
                    mobj->x = READFIXED(save_p);
                    mobj->y = READFIXED(save_p);
                    mobj->angle = READANGLE(save_p);
                }
                else
                {
                    mobj->x = mobj->spawnpoint->x << FRACBITS;
                    mobj->y = mobj->spawnpoint->y << FRACBITS;
                    mobj->angle = ANG45 * (mobj->spawnpoint->angle / 45);
                }
                if (diff & MD_MOM)
                {
                    mobj->momx = READFIXED(save_p);
                    mobj->momy = READFIXED(save_p);
                    mobj->momz = READFIXED(save_p);
                }       // else 0 (memset)

                if (diff & MD_RADIUS)
                    mobj->radius = READFIXED(save_p);
                else
                    mobj->radius = mobj->info->radius;
                if (diff & MD_HEIGHT)
                    mobj->height = READFIXED(save_p);
                else
                    mobj->height = mobj->info->height;
                if (diff & MD_FLAGS)
                    mobj->flags = READLONG(save_p);
                else
                    mobj->flags = mobj->info->flags;
                if (diff & MD_FLAGS2)
                    mobj->flags2 = READLONG(save_p);
                else
                    mobj->flags2 = mobj->info->flags2;
                if (diff & MD_HEALTH)
                    mobj->health = READLONG(save_p);
                else
                    mobj->health = mobj->info->spawnhealth;
                if (diff & MD_RTIME)
                    mobj->reactiontime = READLONG(save_p);
                else
                    mobj->reactiontime = mobj->info->reactiontime;

                if (diff & MD_STATE)
                    mobj->state = &states[READUSHORT(save_p)];
                else
                    mobj->state = &states[mobj->info->spawnstate];
                if (diff & MD_TICS)
                    mobj->tics = READLONG(save_p);
                else
                    mobj->tics = mobj->state->tics;
                if (diff & MD_SPRITE)
                    mobj->sprite = READUSHORT(save_p);
                else
                    mobj->sprite = mobj->state->sprite;
                if (diff & MD_FRAME)
                    mobj->frame = READULONG(save_p);
                else
                    mobj->frame = mobj->state->frame;
                if (diff & MD_EFLAGS)
                    mobj->eflags = READULONG(save_p);
                if (diff & MD_PLAYER)
                {
                    i = READBYTE(save_p);
                    mobj->player = &players[i];
                    mobj->player->mo = mobj;
                    // added for angle prediction
                    if (consoleplayer == i)
                        localangle = mobj->angle;
                    if (secondarydisplayplayer == i)
                        localangle2 = mobj->angle;
                }
                if (diff & MD_MOVEDIR)
                    mobj->movedir = READLONG(save_p);
                if (diff & MD_MOVECOUNT)
                    mobj->movecount = READLONG(save_p);
                if (diff & MD_THRESHOLD)
                    mobj->threshold = READLONG(save_p);
                if (diff & MD_LASTLOOK)
                    mobj->lastlook = READLONG(save_p);
                else
                    mobj->lastlook = -1;
                if (diff & MD_TARGET)
		  mobj->target = (mobj_t *) READULONG(save_p); // HACK, fixed at the end of the function
                if (diff & MD_TRACER)
                    mobj->tracer = (mobj_t *) READULONG(save_p); // HACK, fixed at the end of the function
                if (diff & MD_FRICTION)
                    mobj->friction = READLONG(save_p);
                else
                    mobj->friction = ORIG_FRICTION;
                if (diff & MD_MOVEFACTOR)
                    mobj->movefactor = READLONG(save_p);
                else
                    mobj->movefactor = ORIG_FRICTION_FACTOR;
                if (diff & MD_SPECIAL1)
                    mobj->special1 = READLONG(save_p);
                if (diff & MD_SPECIAL2)
                    mobj->special2 = READLONG(save_p);
                if (diff & MD_AMMO)
                    mobj->dropped_ammo_count = READLONG(save_p);

                // now set deductable field
                // TODO : save this too
                mobj->skin = NULL;

                // set sprev, snext, bprev, bnext, subsector
                P_SetThingPosition(mobj);

                /*
                   mobj->floorz = mobj->subsector->sector->floorheight;
                   if( (diff & MD_Z) == 0 )
                   mobj->z = mobj->floorz;
                 */// This causes 3dfloor problems! SSNTails 03-17-2002
                if (mobj->player)
                {
                    mobj->player->viewz = mobj->player->mo->z + mobj->player->viewheight;
                    //CONS_Printf("viewz = %f\n",FIXED_TO_FLOAT(mobj->player->viewz));
                }
                mobj->ceilingz = mobj->subsector->sector->ceilingheight;
                mobj->thinker.function.acp1 = (actionf_p1) P_MobjThinker;
                P_AddThinker(&mobj->thinker);
                break;

            case tc_ceiling:
	      {
		ceiling_t *ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVEL, NULL);
                PADSAVEP();
		READ_SECTOR_THINKER( ceiling, ceiling_t, type );
                ceiling->sector->ceilingdata = ceiling;
		byte moving = READBYTE(save_p); // moving ceiling?
		ceiling->thinker.function.acp1 = moving ? (actionf_p1)T_MoveCeiling : NULL;
                P_AddActiveCeiling(ceiling);
	      }
	      break;

            case tc_door:
	      {
                vldoor_t *door = Z_Malloc(sizeof(*door), PU_LEVEL, NULL);
                PADSAVEP();
		READ_SECTOR_THINKER( door, vldoor_t, type );
 		READ_LINE_PTR( door->line );
                door->sector->ceilingdata = door;
                door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
	      }
	      break;

            case tc_floor:
	      {
		floormove_t *floormv = Z_Malloc(sizeof(*floormv), PU_LEVEL, NULL);
		PADSAVEP();
		READ_SECTOR_THINKER( floormv, floormove_t, type );
                floormv->sector->floordata = floormv;
                floormv->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
	      }
	      break;

            case tc_plat:
	      {
		plat_t *plat = Z_Malloc(sizeof(*plat), PU_LEVEL, NULL);
                PADSAVEP();
		READ_SECTOR_THINKER( plat, plat_t, type );
                plat->sector->floordata = plat;
		byte moving = READBYTE(save_p); // moving plat?
		plat->thinker.function.acp1 = moving ? (actionf_p1)T_PlatRaise : NULL;
                P_AddActivePlat(plat);
	      }
	      break;

            case tc_flash:
	      {
		lightflash_t *flash = Z_Malloc(sizeof(*flash), PU_LEVEL, NULL);
                PADSAVEP();
		READ_SECTOR_THINKER( flash, lightflash_t, count );
                flash->thinker.function.acp1 = (actionf_p1) T_LightFlash;
	      }
	      break;

            case tc_strobe:
	      {
		strobe_t *strobe = Z_Malloc(sizeof(*strobe), PU_LEVEL, NULL);
                PADSAVEP();
		READ_SECTOR_THINKER( strobe, strobe_t, count );
                strobe->thinker.function.acp1 = (actionf_p1) T_StrobeFlash;
	      }
	      break;

            case tc_glow:
	      {
		glow_t *glow = Z_Malloc(sizeof(*glow), PU_LEVEL, NULL);
                PADSAVEP();
		READ_SECTOR_THINKER( glow, glow_t, minlight );
                glow->thinker.function.acp1 = (actionf_p1) T_Glow;
	      }
	      break;
	      
            case tc_fireflicker:
	      {
		fireflicker_t *fireflicker = Z_Malloc(sizeof(*fireflicker), PU_LEVEL, NULL);
                PADSAVEP();
		READ_SECTOR_THINKER( fireflicker, fireflicker_t, count );
                fireflicker->thinker.function.acp1 = (actionf_p1) T_FireFlicker;
	      }
	      break;
	      
            case tc_lightfade:
	      {
		lightlevel_t *fade = Z_Malloc(sizeof(*fade), PU_LEVEL, NULL);
                PADSAVEP();
		READ_SECTOR_THINKER( fade, lightlevel_t, destlevel );
                fade->thinker.function.acp1 = (actionf_p1) T_LightFade;
	      }
	      break;

            case tc_elevator:
	      {
		elevator_t *elevator = Z_Malloc(sizeof(elevator_t), PU_LEVEL, NULL);
                PADSAVEP();
		READ_SECTOR_THINKER( elevator, elevator_t, type );
                elevator->sector->floordata = elevator; //jff 2/22/98
                elevator->sector->ceilingdata = elevator;       //jff 2/22/98
                elevator->thinker.function.acp1 = (actionf_p1) T_MoveElevator;
	      }
	      break;

            case tc_scroll:
	      {
		scroll_t *scroll = Z_Malloc(sizeof(scroll_t), PU_LEVEL, NULL);
		READ_THINKER( scroll, scroll_t, type );
                scroll->thinker.function.acp1 = (actionf_p1) T_Scroll;
	      }
	      break;

            case tc_friction:
	      {
		friction_t *friction = Z_Malloc(sizeof(friction_t), PU_LEVEL, NULL);
		READ_THINKER( friction, friction_t, affectee );
                friction->thinker.function.acp1 = (actionf_p1) T_Friction;
	      }
	      break;

            case tc_pusher:
	      {
		pusher_t *pusher = Z_Malloc(sizeof(pusher_t), PU_LEVEL, NULL);
		READ_THINKER( pusher, pusher_t, type );
                pusher->source = P_GetPushThing(pusher->affectee);
                pusher->thinker.function.acp1 = (actionf_p1) T_Pusher;
	      }
                break;

            default:
              I_SoftError("LoadGame: Unknown thinker type 0x%X", tclass);
	      goto err_exit;
        }
    }

    // Reversing the HACK: Convert ID numbers to proper mobj_t*:s
    for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
    {
      if (currentthinker->function.acp1 == (actionf_p1) P_MobjThinker)
      {
	mobj = (mobj_t *) currentthinker;
	if (mobj->tracer)
	  mobj->tracer = GetMobjPointer((uint32_t)mobj->tracer);

	if (mobj->target)
	  mobj->target = GetMobjPointer((uint32_t)mobj->target);
      }
    }
    return;

err_report:
    I_SoftError("LoadGame: %s\n", reason );
err_exit:
    save_game_abort = 1;
    return;
}


//
// P_ArchiveSpecials
//

// BP: added : itemrespawnqueue
//
void P_ArchiveSpecials(void)
{
    int i;

    // BP: added save itemrespawn queue for deathmatch
    i = iquetail;
    while (iquehead != i)
    {
        WRITE_MAPTHING_PTR( itemrespawnque[i] );
        WRITELONG(save_p, itemrespawntime[i]);
        i = (i + 1) & (ITEMQUESIZE - 1);
    }

    // end delimiter
    WRITELONG(save_p, 0xffffffff);
}

//
// P_UnArchiveSpecials
//
void P_UnArchiveSpecials(void)
{
    int i;
   
    // BP: added save itemrespawn queue for deathmatch
    iquetail = iquehead = 0;
    while ((i = READLONG(save_p)) != 0xffffffff)
    {
        itemrespawnque[iquehead] = &mapthings[i];
        itemrespawntime[iquehead++] = READLONG(save_p);
    }
}

/////////////////////////////////////////////////////////////////////////////
// BIG NOTE FROM SOM!
//
// SMMU/MBF use the CheckSaveGame function to dynamically expand the savegame
// buffer which would eliminate all limits on savegames... Could/Should we
// use this method?
/////////////////////////////////////////////////////////////////////////////

#ifdef FRAGGLESCRIPT


static unsigned int P_NumberFSArrays(void)
{
  unsigned int count = 0;
#ifdef SAVELIST_STRUCTHEAD
  sfarray_t *cur = sfsavelist.next; // start at first array
#else
  sfarray_t *cur = sfsavelist; // start at first array
#endif
  while (cur)
  {
    cur->saveindex = ++count;  // replaces ptr in save game
    cur = cur->next;
  }

  return count;
}


static void  WRITE_SFArrayPtr( sfarray_t * arrayptr )
{
    // write the array id (saveindex) for the ptr
#ifdef SAVELIST_STRUCTHEAD
    sfarray_t *cur = sfsavelist.next;
#else
    sfarray_t *cur = sfsavelist;
#endif
    while(cur && (cur != arrayptr))  // verify if valid ptr
	    cur = cur->next;

    // zero is unused, so use it for NULL
    // The arrays have been numbered in saveindex, see ReadSFArrayPtr
    WRITELONG(save_p, (cur ? cur->saveindex : 0) );
}
   
static sfarray_t * READ_SFArrayPtr( void )
{
    int svindx;
    sfarray_t *cur = NULL;
    // All arrays were numbered in saveindex
    svindx = READULONG(save_p);  // consistent with Write saveindex

    if(svindx)		// 0 is NULL ptr
    {
#ifdef SAVELIST_STRUCTHEAD
        cur = sfsavelist.next;  // start of all arrays
#else
        cur = sfsavelist;  // start of all arrays
#endif
	while(cur)	// search for matching saveindex
	{
            if(svindx == cur->saveindex)  break;
            cur = cur->next;
	}
        // not found is NULL ptr
    }
    return cur;   // which may be NULL
}
  
// SoM: Added FraggleScript stuff.
// Save all the neccesary FraggleScript data.
// we need to save the levelscript(all global
// variables) and the runningscripts (scripts
// currently suspended)


void P_ArchiveSValue(svalue_t *s)
{
  switch (s->type)   // store depending on type
  {
    case svt_string:
      {
	strcpy((char *)save_p, s->value.s);
	save_p += strlen(s->value.s) + 1;
	break;
      }
    case svt_int:
      {
	WRITELONG(save_p, s->value.i);
	break;
      }
    case svt_fixed:
      {
	WRITEFIXED(save_p, s->value.f);
	break;
      }
    case svt_mobj:
      {
	WRITE_MobjPointerID(save_p, s->value.mobj);
	break;
      }
    default:
      // other types do not appear in user scripts
      break;
  }
}

void P_UnArchiveSValue(svalue_t *s)
{
  switch (s->type)       // read depending on type
  {
    case svt_string:
      {
	s->value.s = Z_Strdup((char *)save_p, PU_LEVEL, 0);
	save_p += strlen(s->value.s) + 1;
	break;
      }
    case svt_int:
      {
	s->value.i = READLONG(save_p);
	break;
      }
    case svt_fixed:
      {
	s->value.f = READFIXED(save_p);
	break;
      }
    case svt_mobj:
      {
	s->value.mobj = READ_MobjPointerID(save_p);
	break;
      }
    default:
      break;
  }
}




void P_ArchiveFSVariables(svariable_t **vars)
{
  int i;

  // count number of variables
  int num_variables = 0;
  for (i = 0; i < VARIABLESLOTS; i++)
  {
    svariable_t *sv = vars[i];

    // once we get to a label there can be no more actual
    // variables in the list to store
    while (sv && sv->type != svt_label)
    {
      num_variables++;
      sv = sv->next;
    }
  }

  //CheckSaveGame(sizeof(short));
  WRITESHORT(save_p, num_variables);  // write num_variables

  // go thru hash chains, store each variable
  for (i = 0; i < VARIABLESLOTS; i++)
  {
    // go thru this hashchain
    svariable_t *sv = vars[i];

    while (sv && sv->type != svt_label)
    {
      //CheckSaveGame(strlen(sv->name)+10); // 10 for type and safety
      // write svariable: name
      strcpy((char *)save_p, sv->name);
      save_p += strlen(sv->name) + 1;     // 1 extra for ending NULL

      WRITEBYTE(save_p, sv->type); // store type;

      // Those that are not handled by P_ArchiveSValue
      if (sv->type == svt_array) // haleyjd: arrays
      {
	  WRITE_SFArrayPtr( sv->value.a );
	  break;
      }
      else
      {
	  // [smite] TODO svariable_t should simply inherit svalue_t
	  // also svalue_t should have array as a possible subtype
	  svalue_t s;
	  s.type  = sv->type;

	  s.value.mobj = sv->value.mobj; // HACK largest type in union
	  P_ArchiveSValue(&s);
      }

      sv = sv->next;
    }
  }
}



void P_UnArchiveFSVariables(svariable_t **vars)
{
  int i;

  // now read the number of variables from the savegame file
  int num_variables = READSHORT(save_p);

  for (i = 0; i < num_variables; i++)
  {
    svariable_t *sv = Z_Malloc(sizeof(svariable_t), PU_LEVEL, 0);

    // name
    sv->name = Z_Strdup((char *)save_p, PU_LEVEL, 0);
    save_p += strlen(sv->name) + 1;

    sv->type = READBYTE(save_p);

    if (sv->type == svt_array) // Exl; arrays
    {
        sv->value.a = READ_SFArrayPtr();
    }
    else
    {
	// [smite] TODO svariable_t should simply inherit svalue_t, but...
	svalue_t s;
	s.type = sv->type;

	P_UnArchiveSValue(&s);
	sv->value.mobj = s.value.mobj; // HACK largest type in union
    }

    // link in the new variable
    int hashkey = variable_hash(sv->name);
    sv->next = vars[hashkey];
    vars[hashkey] = sv;
  }
}



/***************** save the levelscript *************/
// make sure we remember all the global
// variables.

void P_ArchiveLevelScript()
{
  // all we really need to do is save the variables
  P_ArchiveFSVariables(levelscript.variables);
}

void P_UnArchiveLevelScript()
{
  int i;

  // free all the variables in the current levelscript first
  for (i = 0; i < VARIABLESLOTS; i++)
  {
    svariable_t *sv = levelscript.variables[i];

    while (sv && sv->type != svt_label)
    {
      svariable_t *next = sv->next;
      Z_Free(sv);
      sv = next;
    }
    levelscript.variables[i] = sv;  // null or label
  }


  P_UnArchiveFSVariables(levelscript.variables);
}

/**************** save the runningscripts ***************/

extern runningscript_t runningscripts;  // t_script.c
runningscript_t *new_runningscript();   // t_script.c
void clear_runningscripts();    // t_script.c

// save a given runningscript
void P_ArchiveRunningScript(runningscript_t * rs)
{
    //CheckSaveGame(sizeof(short) * 8); // room for 8 shorts
    WRITESHORT(save_p, rs->script->scriptnum);  // save scriptnum
    // char* into data, saved as index
    WRITESHORT(save_p, rs->savepoint - rs->script->data);       // offset
    WRITESHORT(save_p, rs->wait_type);
    WRITESHORT(save_p, rs->wait_data);

    // save trigger ID
    WRITE_MobjPointerID(save_p, rs->trigger);

    P_ArchiveFSVariables(rs->variables);
}

// get the next runningscript
runningscript_t *P_UnArchiveRunningScript()
{
    int i;

    // create a new runningscript
    runningscript_t *rs = new_runningscript();

    int scriptnum = READSHORT(save_p);      // get scriptnum

    // levelscript?

    if (scriptnum == -1)
        rs->script = &levelscript;
    else
        rs->script = levelscript.children[scriptnum];

    // read out offset from save, convert index into ptr = &data[index]
    rs->savepoint = rs->script->data + READSHORT(save_p);
    rs->wait_type = READSHORT(save_p);
    rs->wait_data = READSHORT(save_p);

    // read out trigger thing
    rs->trigger = READ_MobjPointerID(save_p);


    // read out the variables now (fun!)
    // start with basic script slots/labels FIXME why?
    for (i = 0; i < VARIABLESLOTS; i++)
      rs->variables[i] = rs->script->variables[i];

    P_UnArchiveFSVariables(rs->variables);

    return rs;
}

// archive all runningscripts in chain
void P_ArchiveRunningScripts()
{
    runningscript_t *rs;
    int num_runningscripts = 0;

    // count runningscripts
    for (rs = runningscripts.next; rs; rs = rs->next)
        num_runningscripts++;

    //CheckSaveGame(sizeof(long));

    // store num_runningscripts
    WRITEULONG(save_p, num_runningscripts);

    // now archive them
    rs = runningscripts.next;
    while (rs)
    {
        P_ArchiveRunningScript(rs);
        rs = rs->next;
    }
}

// restore all runningscripts from save_p
void P_UnArchiveRunningScripts()
{
    runningscript_t *rs;
    int num_runningscripts;
    int i;

    // remove all runningscripts first : may have been started
    // by levelscript on level load

    clear_runningscripts();

    // get num_runningscripts
    num_runningscripts = READULONG(save_p);

    for (i = 0; i < num_runningscripts; i++)
    {
        // get next runningscript
        rs = P_UnArchiveRunningScript();

        // hook into chain
        rs->next = runningscripts.next;
        rs->prev = &runningscripts;
        rs->prev->next = rs;
        if (rs->next)
            rs->next->prev = rs;
    }
}




//
// FS Array Saving
//
// Array variables are saved by the code above for the level and
// running scripts, but first this stuff needs to be done -- enumerate
// and archive the arrays themselves.
//


// must be called before running/level script archiving
void P_ArchiveFSArrays(void)
{
  // [smite] FIXME can we have several array variables reference the same object? 
  // Because if arrays are handled by value, this is unnecessary and they can be treated like normal variables.
  
  unsigned int num_fsarrays = P_NumberFSArrays(); // number all the arrays

  // write number of FS arrays
  WRITEULONG(save_p, num_fsarrays);
      
#ifdef SAVELIST_STRUCTHEAD
  sfarray_t *cur = sfsavelist.next; // start at first array
#else
  sfarray_t *cur = sfsavelist; // start at first array
#endif
  while(cur)
  {
    unsigned int i;

    // write the length of this array
    WRITEULONG(save_p, cur->length);

    // values[] is array of svalue_s, which is a union of possible values
    // marked with the type, each array element can be of a different type

    // write the contents of this array
    for (i=0; i<cur->length; i++)
    {
      WRITEBYTE(save_p, cur->values[i].type); // store type;
      P_ArchiveSValue(&cur->values[i]);
    }

    cur = cur->next;
  }
}

// must be called before unarchiving running/level scripts
void P_UnArchiveFSArrays(void)
{
  T_InitSaveList(); // reinitialize the save list
     // All PU_LEVEL memory already cleared by P_UnArchiveMisc()

  // read number of FS arrays
  unsigned int num_fsarrays = READULONG(save_p);

#ifdef SAVELIST_STRUCTHEAD
  sfarray_t *last = sfsavelist.next; // start at first array
#else
  sfarray_t *last = sfsavelist; // start at first array
#endif

  // read all the arrays
  unsigned int q;
  for(q=0; q<num_fsarrays; q++)
  {
    sfarray_t *newArray = Z_Malloc(sizeof(sfarray_t), PU_LEVEL, NULL);
    memset(newArray, 0, sizeof(sfarray_t));

    // read length of this array
    newArray->length = READULONG(save_p);
      
    newArray->values = Z_Malloc(newArray->length * sizeof(svalue_t), PU_LEVEL, NULL);
    CONS_Printf("%i", newArray->length);
      
    // read all archived values
    unsigned int i;
    for(i=0; i<newArray->length; i++)
    {
      newArray->values[i].type = READBYTE(save_p);
      P_UnArchiveSValue(&newArray->values[i]);
    }

    // link in the new array -- must reconstruct list in same
    // order as read (T_AddArray will not work for this)
    last->next = newArray;
    last = newArray;
  }

  // now number all the arrays
  P_NumberFSArrays();
}



void P_ArchiveScripts()
{
    // save FS arrays
    P_ArchiveFSArrays();

    // save levelscript
    P_ArchiveLevelScript();

    // save runningscripts
    P_ArchiveRunningScripts();

    WRITEBOOLEAN(save_p, script_camera_on);
    WRITE_MobjPointerID(save_p, script_camera.mo);
    WRITEANGLE(save_p, script_camera.aiming);
    WRITEFIXED(save_p, script_camera.viewheight);
    WRITEANGLE(save_p, script_camera.startangle);
}

void P_UnArchiveScripts()
{
    // restore FS arrays
    P_UnArchiveFSArrays();
    
    // restore levelscript
    P_UnArchiveLevelScript();

    // restore runningscripts
    P_UnArchiveRunningScripts();

    // Unarchive the script camera
    script_camera_on = READBOOLEAN(save_p);
    script_camera.mo = READ_MobjPointerID(save_p);
    script_camera.aiming = READANGLE(save_p);
    script_camera.viewheight = READFIXED(save_p);
    script_camera.startangle = READANGLE(save_p);
}

// [WDJ] return true if there is fragglescript state to be saved
boolean SG_fragglescript_detect( void )
{
#ifdef SAVELIST_STRUCTHEAD
    if( sfsavelist.next ) goto found_state;	// start of arrays
#else
    if( sfsavelist ) goto found_state;	// start of arrays
#endif
    if( levelscript.variables ) goto found_state;  // levelscript has vars
    if( runningscripts.next ) goto found_state;  // there is a running script
    if( script_camera_on ) goto found_state;
    if( script_camera.mo || script_camera.viewheight
	|| script_camera.aiming || script_camera.startangle )
     		goto found_state; // camera was on
 
    return 0;

 found_state:
    return 1;	// must save fragglescript
}

#endif // FRAGGLESCRIPT

// =======================================================================
//          Misc
// =======================================================================
void P_ArchiveMisc()
{
    ULONG pig = 0;
    int i;

    WRITEBYTE(save_p, gameskill);
    WRITEBYTE(save_p, gameepisode);
    WRITEBYTE(save_p, gamemap);

    for (i = 0; i < MAXPLAYERS; i++)
        pig |= (playeringame[i] != 0) << i;

    WRITEULONG(save_p, pig);

    WRITEULONG(save_p, leveltime);
    WRITEBYTE(save_p, P_GetRandIndex());
}

boolean P_UnArchiveMisc()
{
    ULONG pig;
    int i;

    gameskill = READBYTE(save_p);
    gameepisode = READBYTE(save_p);
    gamemap = READBYTE(save_p);

    pig = READULONG(save_p);

    for (i = 0; i < MAXPLAYERS; i++)
    {
        playeringame[i] = (pig & (1 << i)) != 0;
        players[i].playerstate = PST_REBORN;
    }

    // Purge all previous PU_LEVEL memory.
    if (!P_SetupLevel(gameepisode, gamemap, gameskill, NULL))
        return false;

    // get the time
    leveltime = READULONG(save_p);
    P_SetRandIndex(READBYTE(save_p));

    return true;
}

// =======================================================================
//          Save game
// =======================================================================

// Save game is inherently variable length, this is worst case wild guess.
// added 8-3-98 increase savegame size from 0x2c000 (180kb) to 512*1024
#define SAVEGAMESIZE    (512*1024)
#define SAVEGAME_HEADERSIZE   (64 + 80 + 128 + 32 + 32)
//#define SAVESTRINGSIZE  24
int savebuffer_size = 0;
byte * savebuffer = NULL;

// Allocate malloc an appropriately sized buffer
byte *  P_Alloc_savebuffer( boolean header, boolean data )
{
    savebuffer_size = 0;
    if( header )   savebuffer_size += SAVEGAME_HEADERSIZE;
    if( data )     savebuffer_size += SAVEGAMESIZE;
   
    save_p = savebuffer = (byte *)malloc(savebuffer_size);
    if( ! savebuffer)
    {
        CONS_Printf (" free memory for savegame\n");
        return NULL;
    }
    return savebuffer;
}

// return -1 if overrun the buffer
size_t  P_Savegame_length( void )
{
    size_t length = save_p - savebuffer;
    if (length > SAVEGAMESIZE)
    {
        I_SoftError ("Savegame buffer overrun, need %i\n", length);
   	return -1;
    }
    return length;
}


// Save game header support

// Get the name of the wad containing the current level map
// Write operation for :map: line.
char * level_wad( void )
{
    char * mapwad;
    // lastloadedmaplumpnum contains index to the wad containing current map.
    int mapwadnum = WADFILENUM( lastloadedmaplumpnum );
    if( mapwadnum >= numwadfiles )  goto defname;
    mapwad = wadfiles[ mapwadnum ]->filename;
    if( mapwad == NULL )  goto defname;
    return FIL_Filename_of( mapwad );  // ignore directories

 defname:
    return gamedesc.iwad_filename;
}

// Check if the wad name is in the wadfiles
// Read operation for :map: line.
boolean  check_have_wad( char * ckwad )
{
    int i;
    // search all known wad names for a match
    for( i=0; i<numwadfiles; i++ )
    {
        char * tt = FIL_Filename_of( wadfiles[i]->filename );
        if( strcmp( tt, ckwad ) == 0 )  return true;
    }
    return false;
}


// Write the command line switches to savebuffer.
// Write operation for :cmd: line.
void WRITE_command_line( void )
{
    int i;
    SG_write_string( ":cmd:" );
    save_p --;  // No term 0 on header writes
    for( i=1; i<myargc; i++ )	// skip executable
    {
        int len = sprintf( save_p, " %s", myargv[i] );
        save_p += len;
    }
    SG_write_string( "\n" );
    save_p --;
}

// Save game header
// Langid format requires underlines.
const char * sg_head_format =
"!!Legacy_save_game.V%i\n:name:%s\n:game:%s\n:wad:%s\n:map:%s\n:time:%2i:%02i\n";
const char * sg_head_END = "::END\n";
const short idname_length = 18;  // !!<name> length

#ifdef __BIG_ENDIAN__
const byte sg_big_endian = 1;
#else
const byte sg_big_endian = 0;
#endif

// Called from menu via G_DoSaveGame via network Got_SaveGamecmd.
// Only used for savegame file.
// Write savegame header to savegame buffer.
void P_Write_Savegame_Header( const char * description )
{
    int len;
    int l_min, l_sec;
    
    // time into level
    l_sec = leveltime / TICRATE;  // seconds
    l_min = l_sec / 60;
    l_sec -= l_min * 60;
   
    save_p = savebuffer;
   
    // [WDJ] A consistent header across all save game versions.
    // Save Langid game header
    // Do not use WRITESTRING as that will put term 0 into the header.
    len = sprintf( save_p, sg_head_format,
		   VERSION, description, gamedesc.gname,
		   level_wad(), levelmapname, l_min, l_sec );
    save_p += len;  // does not include string term 0
    WRITE_command_line();
    len = sprintf( save_p, sg_head_END );
    save_p += len;  // does not include string term 0
    WRITEBYTE( save_p, 0 );  // The only 0 in the header is after the END
    // the level number is also saved in ArchiveMisc
 
    // binary header data
    WRITESHORT( save_p, VERSION );	// 16 bit game version that wrote file
    WRITEBYTE( save_p, sg_big_endian );
    WRITEBYTE( save_p, sg_padded );
    WRITEBYTE( save_p, sizeof(int) );	// word size
    WRITEBYTE( save_p, sizeof(boolean) );	// machine dependent
    // reserved
    WRITEBYTE( save_p, 0 );
    WRITEBYTE( save_p, 0 );
    WRITEBYTE( save_p, 0 );
    WRITEBYTE( save_p, 0 );
}


// Find the header line in the savebuffer
char *  read_header_line( const char * idstr )
{
    char * fnd = strstr( save_p, idstr ); // find the :name:
    if( fnd ) // NULL if not found
        fnd += strlen(idstr);  // start of line content
    return fnd;
}

// Terminate strings, this modifies the header in the savebuffer
// Must be only done after all header reads.
void  term_header_line( char * infodest )
{
    if( infodest ) // NULL if not found
    {
        // terminate strings, this modifies the header in the savebuffer
        * strpbrk( infodest, "\r\n" ) = 0;
    }
}


// Called from G_DoLoadGame, M_ReadSaveStrings
// Only used for savegame file.
// Read savegame header from savegame buffer.
// Returns header info in infop.
// Returns 1 when header is correct.
boolean P_Read_Savegame_Header( savegame_info_t * infop)
{
    char * reason;

    // Read header
    save_game_abort = 0;	// all sync reads will check this
    save_p = savebuffer;

    if( strncmp( save_p, sg_head_format, idname_length ) )  goto not_save;
    if( ! strstr( save_p, "::END" ) )  goto not_save;

    // find header strings
    infop->name = read_header_line( ":name:" );
    infop->game = read_header_line( ":game:" );
    infop->wad = read_header_line( ":wad:" );
    infop->map = read_header_line( ":map:" );
    infop->levtime = read_header_line( ":time:" );
    save_p += strlen( save_p ) + 1; // find 0, to get past Langid header;

    // terminate the strings, this modifies the header in the savebuffer
    // and prevents finding any more header lines
    term_header_line( infop->name );
    term_header_line( infop->game );
    term_header_line( infop->wad );
    term_header_line( infop->map );
    term_header_line( infop->levtime );

    // validity tests
    infop->have_game = ( strcmp( gamedesc.gname, infop->game ) == 0 );
    infop->have_wad = check_have_wad( infop->wad );

    // binary header data
    reason = "version";
    if( READSHORT( save_p ) != VERSION )  goto wrong;
    reason = "endian";
    if( READBYTE( save_p ) != sg_big_endian )  goto wrong;
    reason = "padding";
    if( READBYTE( save_p ) != sg_padded )  goto wrong;
    reason = "integer size";
    if( READBYTE( save_p ) != sizeof(int))  goto wrong;
    reason = "boolean size";
    if( READBYTE( save_p ) != sizeof(boolean))  goto wrong;
    // reserved header bytes
    READBYTE( save_p );
    READBYTE( save_p );
    READBYTE( save_p );
    READBYTE( save_p );
   
    infop->msg[0] = 0;    
    return 1;
   
 not_save:
   snprintf( infop->msg, 60, "Not Legacy savegame\n" );
   goto failed;
  
 wrong:
   snprintf( infop->msg, 60, "Invalid savegame: wrong %s\n", reason );
   goto failed;
   
 failed:
    return false;
}


// Called from menu via G_DoSaveGame via network Got_SaveGamecmd,
// and called from SV_SendSaveGame by network for JOININGAME.
// Write game data to savegame buffer.
void P_SaveGame( void )
{   
    InitPointermap_Save(1024);

    SG_SaveSync( SYNC_net );
    CV_SaveNetVars((char **) &save_p);
    SG_SaveSync( SYNC_misc );
    P_ArchiveMisc();
    SG_SaveSync( SYNC_players );
    P_ArchivePlayers();
    SG_SaveSync( SYNC_world );
    P_ArchiveWorld();
    SG_SaveSync( SYNC_thinkers );
    P_ArchiveThinkers();
    SG_SaveSync( SYNC_specials );
    P_ArchiveSpecials();
#ifdef FRAGGLESCRIPT
    // Only save fragglescript if the level uses it.
    if( SG_fragglescript_detect() )
    {
        SG_SaveSync( SYNC_fragglescript );
        P_ArchiveScripts();
    }
#endif
   
    SG_SaveSync( SYNC_end );

    ClearPointermap();
}


   
// Called from G_DoLoadGame
// Read game data in savegame buffer.
boolean P_LoadGame(void)
{
    InitPointermap_Load(1024);

    if( ! SG_ReadSync( SYNC_net, 0 ) )  goto sync_err;
    CV_LoadNetVars((char **) &save_p);
    if( ! SG_ReadSync( SYNC_misc, 0 ) )  goto sync_err;
    // Misc does level setup, and purges all previous PU_LEVEL memory.
    if (!P_UnArchiveMisc())  goto failed;
    if( ! SG_ReadSync( SYNC_players, 0 ) )  goto sync_err;
    P_UnArchivePlayers();
    if( ! SG_ReadSync( SYNC_world, 0 ) )  goto sync_err;
    P_UnArchiveWorld();
    if( ! SG_ReadSync( SYNC_thinkers, 0 ) )  goto sync_err;
    P_UnArchiveThinkers();
    if( ! SG_ReadSync( SYNC_specials, 0 ) )  goto sync_err;
    P_UnArchiveSpecials();
    // Optional fragglescript section
    if( SG_ReadSync( SYNC_fragglescript, 1 ) )
#ifdef FRAGGLESCRIPT
    {
        P_UnArchiveScripts();
    }
    else
    {
        // This is all the setup does
        T_InitSaveList();             // Setup FS array list
        // FIXME: kill any existing fragglescript
    }
#else   
    {
        I_SoftError( "Fragglescript required for this save game" );
        goto failed;
    }
#endif

    if( ! SG_ReadSync( SYNC_end, 1 ) )  goto sync_err;
   
    ClearPointermap();
    return true;
   
 sync_err:
   I_SoftError( "Legacy save game sync error\n" );

 failed:
    ClearPointermap();	// safe clear
    return false;
}
