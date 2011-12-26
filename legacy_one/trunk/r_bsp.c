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
// $Log: r_bsp.c,v $
// Revision 1.24  2004/05/16 20:25:47  hurdler
// change version to 1.43
//
// Revision 1.23  2002/01/12 02:21:36  stroggonmeth
// Big commit
//
// Revision 1.22  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.21  2001/05/30 04:00:52  stroggonmeth
// Fixed crashing bugs in software with 3D floors.
//
// Revision 1.20  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.19  2001/03/21 18:24:39  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.18  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.17  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.16  2000/11/21 21:13:17  stroggonmeth
// Optimised 3D floors and fixed crashing bug in high resolutions.
//
// Revision 1.15  2000/11/09 17:56:20  stroggonmeth
// Hopefully fixed a few bugs and did a few optimizations.
//
// Revision 1.14  2000/11/02 19:49:36  bpereira
// no message
//
// Revision 1.13  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.12  2000/05/23 15:22:34  stroggonmeth
// Not much. A graphic bug fixed.
//
// Revision 1.11  2000/05/03 23:51:01  stroggonmeth
// A few, quick, changes.
//
// Revision 1.10  2000/04/20 21:47:24  stroggonmeth
// no message
//
// Revision 1.9  2000/04/18 17:39:39  stroggonmeth
// Bug fixes and performance tuning.
//
// Revision 1.8  2000/04/15 22:12:58  stroggonmeth
// Minor bug fixes
//
// Revision 1.7  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.6  2000/04/11 19:07:25  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.5  2000/04/06 21:06:19  stroggonmeth
// Optimized extra_colormap code...
// Added #ifdefs for older water code.
//
// Revision 1.4  2000/04/04 19:28:43  stroggonmeth
// Global colormaps working. Added a new linedef type 272.
//
// Revision 1.3  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      BSP traversal, handling of LineSegs for rendering.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "r_local.h"
#include "r_state.h"

#include "r_splats.h"
#include "p_local.h"  //SoM: 4/10/2000: camera
#include "z_zone.h"   //SoM: Check R_Prep3DFloors

seg_t*          curline;
side_t*         sidedef;
line_t*         linedef;
sector_t*       frontsector;
sector_t*       backsector;


#ifdef BSPVIEWER
// viewer setup for BSP render
extern mobj_t*  viewmobj;
int      viewer_modelsec;
boolean  viewer_has_model;
boolean  viewer_underwater;  // only set when viewer_has_model
boolean  viewer_overceiling; // only set when viewer_has_model

// [WDJ] Setup globals in common to all calls of R_FakeFlat
// Call before R_RenderBSPNode or R_DrawMasked
void R_SetupBSPRender( void )
{
    // Setup already done by R_SetupFrame: viewmobj, viewz
    // Can be camera.mo, viewplayer->mo, or fragglescript script_camera.mo
//    viewmobj = camera.chase ? camera.mo : viewplayer->mo;  // orig FakeFlat line
    viewer_modelsec = viewmobj->subsector->sector->modelsec;
    // [WDJ] modelsec used for more than water, do proper test
    // Use of modelsec is protected by model field, do not test for -1.
    viewer_has_model  = viewmobj->subsector->sector->model > SM_fluid;
    viewer_underwater = viewer_has_model && (viewz <= sectors[viewer_modelsec].floorheight);
    viewer_overceiling = viewer_has_model && (viewz >= sectors[viewer_modelsec].ceilingheight);
}
#endif

//faB:  very ugly realloc() of drawsegs at run-time, I upped it to 512
//      instead of 256.. and someone managed to send me a level with
//      896 drawsegs! So too bad here's a limit removal �-la-Boom
//Hurdler: with Legacy 1.43, drawseg_t is 6780 bytes and thus if having 512 segs, it will take 3.3 Mb of memory
//         default is 128 segs, so it means nearly 1Mb allocated
//drawseg_t     drawsegs[MAXDRAWSEGS];
drawseg_t*      drawsegs=NULL;
unsigned        maxdrawsegs;
drawseg_t*      ds_p = NULL;
drawseg_t*      firstnewseg = NULL;


//SoM:3/25/2000: indicates doors closed wrt automap bugfix:
int      doorclosed;
  // used r_segs.c

//
// R_ClearDrawSegs
//
void R_ClearDrawSegs (void)
{
    ds_p = drawsegs;
}



//
// ClipWallSegment
// Clips the given range of columns
// and includes it in the new clip list.
//
typedef struct
{
    int first;
    int last;

} cliprange_t;


//SoM: 3/28/2000: Fix from boom.
#define MAXSEGS         MAXVIDWIDTH/2+1

// new_seg_end is one past the last valid seg
static cliprange_t*    new_seg_end;
static cliprange_t     solidsegs[MAXSEGS];


//
// R_ClipSolidWallSegment
// Does handle solid walls,
//  e.g. single sided LineDefs (middle texture)
//  that entirely block the view.
//
// Called by R_AddLine
void R_ClipSolidWallSegment( int first, int last )
{
    cliprange_t*        next;
    cliprange_t*        start;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    while (start->last < first-1)
        start++;

    if (first < start->first)
    {
        if (last < start->first-1)
        {
            // Post is entirely visible (above start),
            //  so insert a new clippost.
            R_StoreWallRange (first, last);
            next = new_seg_end;
            new_seg_end++;
            //SoM: 3/28/2000: NO MORE CRASHING!
            if(new_seg_end - solidsegs > MAXSEGS)
              I_Error("R_ClipSolidWallSegment: Solid Segs overflow!\n");

            while (next != start)
            {
                *next = *(next-1);
                next--;
            }
            next->first = first;
            next->last = last;
            return;
        }

        // There is a fragment above *start.
        R_StoreWallRange (first, start->first - 1);
        // Now adjust the clip size.
        start->first = first;
    }

    // Bottom contained in start?
    if (last <= start->last)
        return;

    next = start;
    while (last >= (next+1)->first-1)
    {
        // There is a fragment between two posts.
        R_StoreWallRange (next->last + 1, (next+1)->first - 1);
        next++;

        if (last <= next->last)
        {
            // Bottom is contained in next.
            // Adjust the clip size.
            start->last = next->last;
            goto crunch;
        }
    }

    // There is a fragment after *next.
    R_StoreWallRange (next->last + 1, last);
    // Adjust the clip size.
    start->last = last;

    // Remove start+1 to next from the clip list,
    // because start now covers their area.
  crunch:
    if (next == start)
    {
        // Post just extended past the bottom of one post.
        return;
    }


    while (next++ != new_seg_end)
    {
        // Remove a post.
        *++start = *next;
    }

    new_seg_end = start+1;

    //SoM: 3/28/2000: NO MORE CRASHING!
    if(new_seg_end - solidsegs > MAXSEGS)
      I_Error("R_ClipSolidWallSegment: Solid Segs overflow!\n");
}


//
// R_ClipPassWallSegment
// Clips the given range of columns,
//  but does not includes it in the clip list.
// Does handle windows,
//  e.g. LineDefs with upper and lower texture.
//
void R_ClipPassWallSegment ( int first, int last )
{
    cliprange_t*        start;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    while (start->last < first-1)
        start++;

    if (first < start->first)
    {
        if (last < start->first-1)
        {
            // Post is entirely visible (above start).
            R_StoreWallRange (first, last);
            return;
        }

        // There is a fragment above *start.
        R_StoreWallRange (first, start->first - 1);
    }

    // Bottom contained in start?
    if (last <= start->last)
        return;

    while (last >= (start+1)->first-1)
    {
        // There is a fragment between two posts.
        R_StoreWallRange (start->last + 1, (start+1)->first - 1);
        start++;

        if (last <= start->last)
            return;
    }

    // There is a fragment after *next.
    R_StoreWallRange (start->last + 1, last);
}



//
// R_ClearClipSegs
//
void R_ClearClipSegs (void)
{
    solidsegs[0].first = -0x7fffffff;
    solidsegs[0].last = -1;
    solidsegs[1].first = rdraw_viewwidth;
    solidsegs[1].last = 0x7fffffff;
    new_seg_end = solidsegs+2;
}


//SoM: 3/25/2000
// This function is used to fix the automap bug which
// showed lines behind closed doors simply because the door had a dropoff.
//
// It assumes that Doom has already ruled out a door being closed because
// of front-back closure (e.g. front floor is taller than back ceiling).

int R_DoorClosed(void)
{
  return

    // if door is closed because back is shut:
    backsector->ceilingheight <= backsector->floorheight

    // preserve a kind of transparent door/lift special effect:
    && (backsector->ceilingheight >= frontsector->ceilingheight
	|| curline->sidedef->toptexture)  // 0=no-texture

    && (backsector->floorheight <= frontsector->floorheight
	|| curline->sidedef->bottomtexture) // 0=no-texture

    // properly render skies (consider door "open" if both ceilings are sky):
    && (backsector->ceilingpic != skyflatnum
	|| frontsector->ceilingpic != skyflatnum);
}

//
// If player's view height is underneath fake floor, lower the
// drawn ceiling to be just under the floor height, and replace
// the drawn floor and ceiling textures, and light level, with
// the control sector's.
//
// Similar for ceiling, only reflected.
//
//

// When using for backsector, back=true.
sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec,
                     int *floorlightlevel, int *ceilinglightlevel,
                     boolean back)
{
  int        colormapnum = -1; //SoM: 4/4/2000
  int	     floorlightsubst, ceilinglightsubst; // light from another sector
#ifndef BSPVIEWER
  // [WDJ] partial duplicate of viewmobj setup by R_SetupFrame
  mobj_t*    viewmobj = camera.chase ? camera.mo : viewplayer->mo;
#endif

  // first light substitution, may be -1 which defaults to sec->lightlevel
  floorlightsubst = sec->floorlightsec;
  ceilinglightsubst = sec->ceilinglightsec;

  //SoM: 4/4/2000: If the sector has a midmap, it's probably from 280 type
  if(sec->model == SM_colormap && sec->midmap != -1 )
    colormapnum = sec->midmap;  // explicit colormap

//  if (sec->modelsec != -1 && sec->model == SM_Boom_deep_water)	// [WDJ] 11/14/2009
  if (sec->model == SM_Boom_deep_water)	// [WDJ] 11/14/2009
  {
      // SM_Boom_deep_water passes modelsec >= 0
      const sector_t *modsecp = &sectors[sec->modelsec];
#ifndef BSPVIEWER     
      int viewer_modelsec = viewmobj->subsector->sector->modelsec;
      // [WDJ] modelsec used for more than water, do proper test
      boolean  viewer_has_model  = viewmobj->subsector->sector->model > SM_fluid;
      boolean  viewer_underwater = viewer_has_model && (viewz <= sectors[viewer_modelsec].floorheight);
#endif

      // Replace sector being drawn, with a copy to be hacked
      *tempsec = *sec;
      colormapnum = modsecp->midmap;  // Deep-water colormap, middle-section default

      // Replace floor and ceiling height with other sector's heights.
      if( viewer_underwater )
      {
	  // under the model sector floor
      	  tempsec->floorheight = sec->floorheight;
	  tempsec->ceilingheight = modsecp->floorheight-1;
      }
      else
      {
	  // view above the model sector floor
	  tempsec->floorheight   = modsecp->floorheight;
	  tempsec->ceilingheight = modsecp->ceilingheight;
      }
     
      if ((viewer_underwater && !back) || viewz <= modsecp->floorheight)
      {                   // head-below-floor hack
	  // view under the model sector floor
          tempsec->floorpic    = modsecp->floorpic;
          tempsec->floor_xoffs = modsecp->floor_xoffs;
          tempsec->floor_yoffs = modsecp->floor_yoffs;


          if (viewer_underwater)
          {
            if (modsecp->ceilingpic == skyflatnum)
            {
	        // Boom ref: F_SKY1 as control sector ceiling gives strange effect.
		// Underwater, only the control sector floor appears
	        // and it "envelops" the player.
                tempsec->floorheight   = tempsec->ceilingheight+1;
                tempsec->ceilingpic    = tempsec->floorpic;
                tempsec->ceiling_xoffs = tempsec->floor_xoffs;
                tempsec->ceiling_yoffs = tempsec->floor_yoffs;
            }
            else
            {
                tempsec->ceilingpic    = modsecp->ceilingpic;
                tempsec->ceiling_xoffs = modsecp->ceiling_xoffs;
                tempsec->ceiling_yoffs = modsecp->ceiling_yoffs;
            }
            colormapnum = modsecp->bottommap; // Boom colormap, underwater
          }

          tempsec->lightlevel  = modsecp->lightlevel;

	  // use model substitute, or model light
	  floorlightsubst = (modsecp->floorlightsec != -1) ? modsecp->floorlightsec : sec->modelsec;
	  ceilinglightsubst = (modsecp->ceilinglightsec != -1) ? modsecp->ceilinglightsec : sec->modelsec;
      }
      else
      {
#ifdef BSPVIEWER
        if (viewer_overceiling
	    && (sec->ceilingheight > modsecp->ceilingheight))
#else	 
//        if (viewer_modelsec != -1 && viewz >= sectors[viewer_modelsec].ceilingheight &&
        if (viewer_has_model && (viewz >= sectors[viewer_modelsec].ceilingheight)
	    && (sec->ceilingheight > modsecp->ceilingheight))
#endif	   
        {   // Above-ceiling hack
	    // view over the model sector ceiling
            tempsec->ceilingheight = modsecp->ceilingheight;
            tempsec->floorheight   = modsecp->ceilingheight + 1;

	        // Boom ref: F_SKY1 as control sector floor gives strange effect.
		// Over the ceiling, only the control sector ceiling appears
	        // and it "envelops" the player.
            tempsec->floorpic    = tempsec->ceilingpic    = modsecp->ceilingpic;
            tempsec->floor_xoffs = tempsec->ceiling_xoffs = modsecp->ceiling_xoffs;
            tempsec->floor_yoffs = tempsec->ceiling_yoffs = modsecp->ceiling_yoffs;

            colormapnum = modsecp->topmap; // Boom colormap, over ceiling

            if (modsecp->floorpic != skyflatnum)
            {
	        // view over ceiling, model floor/ceiling
                tempsec->ceilingheight = sec->ceilingheight;
                tempsec->floorpic      = modsecp->floorpic;
                tempsec->floor_xoffs   = modsecp->floor_xoffs;
                tempsec->floor_yoffs   = modsecp->floor_yoffs;
            }

            tempsec->lightlevel  = modsecp->lightlevel;

	    // use model substitute, or model light
            floorlightsubst = (modsecp->floorlightsec != -1) ? modsecp->floorlightsec : sec->modelsec;
	    ceilinglightsubst = (modsecp->ceilinglightsec != -1) ? modsecp->ceilinglightsec : sec->modelsec;
        }
	// else normal view
      }
      sec = tempsec;
  }
//  else if (sec->modelsec != -1 && sec->model == SM_Legacy_water) //SoM: 3/20/2000
  else if (sec->model == SM_Legacy_water) //SoM: 3/20/2000
  {
    // SM_Legacy_water passes modelsec >= 0
    sector_t*    modsecp = &sectors[sec->modelsec];
#ifndef BSPVIEWER
    int          viewer_modelsec = viewmobj->subsector->sector->modelsec;
    // [WDJ] modelsec used for more than water, do proper test
    boolean      viewer_has_model  = viewmobj->subsector->sector->model > SM_fluid;
//    int          viewer_underwater = modelsec!=-1 && viewz<=sectors[modelsec].floorheight;
    boolean      viewer_underwater = viewer_has_model && (viewz <= sectors[viewer_modelsec].floorheight);
#endif

    *tempsec = *sec;

    if(viewer_underwater)
    {
      // view below model sector floor
      colormapnum = modsecp->bottommap; // Legacy colormap, underwater
      if(sec->floorlightsec != -1)
      {
	// use substitute light
        floorlightsubst = ceilinglightsubst = sec->floorlightsec;
	tempsec->lightlevel = sectors[sec->floorlightsec].lightlevel;
      }
      if(modsecp->floorheight < tempsec->ceilingheight)
      {
        tempsec->ceilingheight = modsecp->floorheight;
        tempsec->ceilingpic = modsecp->floorpic;
        tempsec->ceiling_xoffs = modsecp->floor_xoffs;
        tempsec->ceiling_yoffs = modsecp->floor_yoffs;
      }
    }
#ifdef BSPVIEWER
    else if(!viewer_underwater && viewer_overceiling)
#else     
//    else if(!viewer_underwater && modelsec != -1 && viewz >= sectors[modelsec].ceilingheight)
    else if(!viewer_underwater && viewer_has_model && (viewz >= sectors[viewer_modelsec].ceilingheight))
#endif
    {
      // view over model sector ceiling
      colormapnum = modsecp->topmap; // Legacy colormap, over ceiling
      if(sec->ceilinglightsec != -1)
      {
	// use substitute light
        floorlightsubst = ceilinglightsubst = sec->ceilinglightsec;
	tempsec->lightlevel = sectors[sec->ceilinglightsec].lightlevel;
      }
      if(modsecp->ceilingheight > tempsec->floorheight)
      {
        tempsec->floorheight = modsecp->ceilingheight;
        tempsec->floorpic = modsecp->ceilingpic;
        tempsec->floor_xoffs = modsecp->ceiling_xoffs;
        tempsec->floor_yoffs = modsecp->ceiling_yoffs;
      }
    }
    else
    {
      colormapnum = modsecp->midmap;  // Legacy colormap, middle section
      //SoM: Use middle normal sector's lightlevels.
      if(modsecp->floorheight > tempsec->floorheight)
      {
        tempsec->floorheight = modsecp->floorheight;
        tempsec->floorpic = modsecp->floorpic;
        tempsec->floor_xoffs = modsecp->floor_xoffs;
        tempsec->floor_yoffs = modsecp->floor_yoffs;
      }
      else
      {
	floorlightsubst = -1; // revert floor to no subst
      }
      if(modsecp->ceilingheight < tempsec->ceilingheight)
      {
        tempsec->ceilingheight = modsecp->ceilingheight;
        tempsec->ceilingpic = modsecp->ceilingpic;
        tempsec->ceiling_xoffs = modsecp->ceiling_xoffs;
        tempsec->ceiling_yoffs = modsecp->ceiling_yoffs;
      }
      else
      {
	ceilinglightsubst = -1; // revert ceiling to no subst
      }
    }
    sec = tempsec;
  }

  // colormap that this sector uses for this frame, from colormapnum.
  if(colormapnum >= 0 && colormapnum < num_extra_colormaps)
    sec->extra_colormap = &extra_colormaps[colormapnum];
  else
    sec->extra_colormap = NULL;

  // [WDJ] return light parameters in one place
  if (floorlightlevel) {
    *floorlightlevel = (floorlightsubst != -1) ?
       sectors[floorlightsubst].lightlevel : sec->lightlevel ;
  }

  if (ceilinglightlevel) {
    *ceilinglightlevel = (ceilinglightsubst != -1) ?
       sectors[ceilinglightsubst].lightlevel : sec->lightlevel ;
  }
   
  return sec;
}



//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//
// Called by R_Subsector
void R_AddLine (seg_t*  line)
{
    int                 x1;
    int                 x2;
    angle_t             angle1;
    angle_t             angle2;
    angle_t             span;
    angle_t             tspan;
    static sector_t     tempsec; //SoM: ceiling/water hack

    curline = line;

    // OPTIMIZE: quickly reject orthogonal back sides.
    angle1 = R_PointToAngle (line->v1->x, line->v1->y);
    angle2 = R_PointToAngle (line->v2->x, line->v2->y);

    // Clip to view edges.
    // OPTIMIZE: make constant out of 2*clipangle (FIELDOFVIEW).
    span = angle1 - angle2;

    // Back side? I.e. backface culling?
    if (span >= ANG180)
        return;

    // Global angle needed by segcalc.
    rw_angle1 = angle1;
    angle1 -= viewangle;
    angle2 -= viewangle;

    tspan = angle1 + clipangle;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;

        angle1 = clipangle;
    }
    tspan = clipangle - angle2;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;
        angle2 = -clipangle;
    }

    // The seg is in the view range,
    // but not necessarily visible.
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    x1 = viewangle_to_x[angle1];
    x2 = viewangle_to_x[angle2];

    // Does not cross a pixel?
    if (x1 == x2)  //SoM: 3/17/2000: Killough said to change the == to >= for... "robustness"?
        return;

    backsector = line->backsector;

    // Single sided line?
    if (!backsector)
        goto clipsolid;

    backsector = R_FakeFlat(backsector, &tempsec, NULL, NULL, true);

    doorclosed = 0; //SoM: 3/25/2000

    // Closed door.
    if (backsector->ceilingheight <= frontsector->floorheight
        || backsector->floorheight >= frontsector->ceilingheight)
        goto clipsolid;

    //SoM: 3/25/2000: Check for automap fix. Store in doorclosed for r_segs.c
    if ((doorclosed = R_DoorClosed()))
      goto clipsolid;

    // Window.
    if (backsector->ceilingheight != frontsector->ceilingheight
        || backsector->floorheight != frontsector->floorheight)
        goto clippass;

    // Reject empty lines used for triggers
    //  and special events.
    // Identical floor and ceiling on both sides,
    // identical light levels on both sides,
    // and no middle texture.
    if (backsector->ceilingpic == frontsector->ceilingpic
        && backsector->floorpic == frontsector->floorpic
        && backsector->lightlevel == frontsector->lightlevel
        && curline->sidedef->midtexture == 0

        //SoM: 3/22/2000: Check offsets too!
        && backsector->floor_xoffs == frontsector->floor_xoffs
        && backsector->floor_yoffs == frontsector->floor_yoffs
        && backsector->ceiling_xoffs == frontsector->ceiling_xoffs
        && backsector->ceiling_yoffs == frontsector->ceiling_yoffs

        //SoM: 3/17/2000: consider altered lighting
        && backsector->floorlightsec == frontsector->floorlightsec
        && backsector->ceilinglightsec == frontsector->ceilinglightsec
        //SoM: 4/3/2000: Consider colormaps
        && backsector->extra_colormap == frontsector->extra_colormap
        && ((!frontsector->ffloors && !backsector->ffloors) ||
           (frontsector->tag == backsector->tag)))
    {
        return;
    }


  clippass:
    R_ClipPassWallSegment (x1, x2-1);
    return;

  clipsolid:
    R_ClipSolidWallSegment (x1, x2-1);
}


//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
//   | 0 | 1 | 2
// --+---+---+---
// 0 | 0 | 1 | 2
// 1 | 4 | 5 | 6
// 2 | 8 | 9 | A
int     checkcoord[12][4] =
{
    {3,0,2,1},
    {3,0,2,0},
    {3,1,2,0},
    {0},       // UNUSED
    {2,0,2,1},
    {0},       // UNUSED
    {3,1,3,0},
    {0},       // UNUSED
    {2,0,3,1},
    {2,1,3,1},
    {2,1,3,0}
};


boolean R_CheckBBox (fixed_t*   bspcoord)
{
    int                 boxpos;

    fixed_t             x1;
    fixed_t             y1;
    fixed_t             x2;
    fixed_t             y2;

    angle_t             angle1;
    angle_t             angle2;
    angle_t             span;
    angle_t             tspan;

    cliprange_t*        start;

    int                 sx1;
    int                 sx2;

    // Find the corners of the box
    // that define the edges from current viewpoint.
    if (viewx <= bspcoord[BOXLEFT])
        boxpos = 0;
    else if (viewx < bspcoord[BOXRIGHT])
        boxpos = 1;
    else
        boxpos = 2;

    if (viewy >= bspcoord[BOXTOP])
        boxpos |= 0;
    else if (viewy > bspcoord[BOXBOTTOM])
        boxpos |= 1<<2;
    else
        boxpos |= 2<<2;

    if (boxpos == 5)
        return true;

    x1 = bspcoord[checkcoord[boxpos][0]];
    y1 = bspcoord[checkcoord[boxpos][1]];
    x2 = bspcoord[checkcoord[boxpos][2]];
    y2 = bspcoord[checkcoord[boxpos][3]];

    // check clip list for an open space
    angle1 = R_PointToAngle (x1, y1) - viewangle;
    angle2 = R_PointToAngle (x2, y2) - viewangle;

    span = angle1 - angle2;

    // Sitting on a line?
    if (span >= ANG180)
        return true;

    tspan = angle1 + clipangle;

    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;

        angle1 = clipangle;
    }
    tspan = clipangle - angle2;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;

        angle2 = -clipangle;
    }


    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    sx1 = viewangle_to_x[angle1];
    sx2 = viewangle_to_x[angle2];

    // Does not cross a pixel.
    if (sx1 == sx2)
        return false;
    sx2--;

    start = solidsegs;
    while (start->last < sx2)
        start++;

    if (sx1 >= start->first
        && sx2 <= start->last)
    {
        // The clippost contains the new span.
        return false;
    }

    return true;
}



//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//

drawseg_t*   firstseg;

// Called by R_RenderBSPNode
void R_Subsector (int num)
{
    int                 count;
    seg_t*              line;
    subsector_t*        sub;
    static sector_t     tempsec; //SoM: 3/17/2000: Deep water hack
    int                 floorlightlevel;
    int                 ceilinglightlevel;
    extracolormap_t*    floorcolormap;
    extracolormap_t*    ceilingcolormap;
    int                 light;

#ifdef RANGECHECK
    if (num>=numsubsectors)
        I_Error ("R_Subsector: ss %i with numss = %i",
                 num,
                 numsubsectors);
#endif

    //faB: subsectors added at run-time
    if (num>=numsubsectors)
        return;

    sscount++;
    sub = &subsectors[num];
    frontsector = sub->sector;
    count = sub->numlines;
    line = &segs[sub->firstline];


    //SoM: 3/17/2000: Deep water/fake ceiling effect.
    frontsector = R_FakeFlat(frontsector, &tempsec, &floorlightlevel,
                             &ceilinglightlevel, false);

    floorcolormap = ceilingcolormap = frontsector->extra_colormap;

    // SoM: Check and prep all 3D floors. Set the sector floor/ceiling light
    // levels and colormaps.
    if(frontsector->ffloors)
    {
      if(frontsector->moved) // floor or ceiling moved, must refresh
      {
        frontsector->numlights = sub->sector->numlights = 0;
        R_Prep3DFloors(frontsector);  // refresh light lists
        sub->sector->lightlist = frontsector->lightlist;
        sub->sector->numlights = frontsector->numlights;
        sub->sector->moved = frontsector->moved = false;  // clear until next move
      }

      light = R_GetPlaneLight(frontsector, frontsector->floorheight);
      if(frontsector->floorlightsec == -1)
        floorlightlevel = *frontsector->lightlist[light].lightlevel;
      floorcolormap = frontsector->lightlist[light].extra_colormap;
      light = R_GetPlaneLight(frontsector, frontsector->ceilingheight);
      if(frontsector->ceilinglightsec == -1)
        ceilinglightlevel = *frontsector->lightlist[light].lightlevel;
      ceilingcolormap = frontsector->lightlist[light].extra_colormap;
    }

    sub->sector->extra_colormap = frontsector->extra_colormap;

    if ((frontsector->floorheight < viewz)
//      || (frontsector->modelsec != -1 &&
	|| (frontsector->model > SM_fluid &&
            sectors[frontsector->modelsec].ceilingpic == skyflatnum))
    {
        // visplane global parameter
        vsp_floorplane = R_FindPlane (frontsector->floorheight,
                                  frontsector->floorpic,
                                  floorlightlevel,
                                  frontsector->floor_xoffs,
                                  frontsector->floor_yoffs,
                                  floorcolormap,
                                  NULL);
    }
    else
        vsp_floorplane = NULL;

    if ((frontsector->ceilingheight > viewz)
        || (frontsector->ceilingpic == skyflatnum)
//	|| (frontsector->modelsec != -1 &&
        || (frontsector->model > SM_fluid &&
            sectors[frontsector->modelsec].floorpic == skyflatnum))
    {
        // visplane global parameter
        vsp_ceilingplane = R_FindPlane (frontsector->ceilingheight,
                                    frontsector->ceilingpic,
                                    ceilinglightlevel,
                                    frontsector->ceiling_xoffs,
                                    frontsector->ceiling_yoffs,
                                    ceilingcolormap,
                                    NULL);
    }
    else
        vsp_ceilingplane = NULL;


    numffloors = 0;
    ffloor[numffloors].plane = NULL;
    if(frontsector->ffloors)
    {
      ffloor_t*  rover;

      for(rover = frontsector->ffloors; rover && numffloors < MAXFFLOORS; rover = rover->next) {

      if(!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERPLANES))
        continue;

      ffloor[numffloors].plane = NULL;
      if(*rover->bottomheight <= frontsector->ceilingheight
	 && *rover->bottomheight >= frontsector->floorheight
         && ((viewz < *rover->bottomheight && !(rover->flags & FF_INVERTPLANES))
	     || (viewz > *rover->bottomheight && (rover->flags & FF_BOTHPLANES))))
	 // [WDJ] What about (viewz == *rover->bottomheight) ???
//DEBUG	 && ((viewz <= *rover->bottomheight && !(rover->flags & FF_INVERTPLANES))
//DEBUG	     || (viewz >= *rover->bottomheight && (rover->flags & FF_BOTHPLANES))))
      {
        light = R_GetPlaneLight_viewz(frontsector, *rover->bottomheight);
        ffloor[numffloors].plane = R_FindPlane(*rover->bottomheight,
                                  *rover->bottompic,
                                  *frontsector->lightlist[light].lightlevel,
                                  *rover->bottomxoffs,
                                  *rover->bottomyoffs,
                                  frontsector->lightlist[light].extra_colormap,
                                  rover);

        ffloor[numffloors].height = *rover->bottomheight;
        ffloor[numffloors].ffloor = rover;
        numffloors++;
      }
      if(numffloors >= MAXFFLOORS)
        break;
      if(*rover->topheight >= frontsector->floorheight
	 && *rover->topheight <= frontsector->ceilingheight
         && ((viewz > *rover->topheight && !(rover->flags & FF_INVERTPLANES))
	     || (viewz < *rover->topheight && (rover->flags & FF_BOTHPLANES))))
	 // [WDJ] What about (viewz == *rover->topheight) ???
//DEBUG	 && ((viewz >= *rover->topheight && !(rover->flags & FF_INVERTPLANES))
//DEBUG         (viewz <= *rover->topheight && (rover->flags & FF_BOTHPLANES))))
          {
              light = R_GetPlaneLight_viewz(frontsector, *rover->topheight);
              ffloor[numffloors].plane = R_FindPlane(*rover->topheight,
                                                     *rover->toppic,
                                                     *frontsector->lightlist[light].lightlevel,
                                                     *rover->topxoffs,
                                                     *rover->topyoffs,
                                                     frontsector->lightlist[light].extra_colormap,
                                                     rover);
              ffloor[numffloors].height = *rover->topheight;
              ffloor[numffloors].ffloor = rover;
              numffloors++;
          }
      }
    }


#ifdef FLOORSPLATS
    if (sub->splats && cv_splats.value )
        R_AddVisibleFloorSplats (sub);
#endif

    R_AddSprites (sub->sector, tempsec.lightlevel);

    firstseg = NULL;

    while (count--) // over all lines in the subsector
    {
      R_AddLine (line);
      line++;
    }
}



//
// R_Prep3DFloors
//
// This function creates the lightlists that the given sector uses to light
// floors/ceilings/walls according to the 3D floors.
void R_Prep3DFloors(sector_t*  sector)
{
  ffloor_t*      rover;
  ffloor_t*      best;
  fixed_t        bestheight, maxheight;
  int            count, i, mapnum;
  sector_t*      modelsec;

  // count needed lightlist entries
  count = 1;
  for(rover = sector->ffloors; rover; rover = rover->next)
  {
    if((rover->flags & FF_EXISTS) && (!(rover->flags & FF_NOSHADE) || (rover->flags & FF_CUTLEVEL) || (rover->flags & FF_CUTSPRITES)))
    {
      count++;
      if(rover->flags & FF_DOUBLESHADOW)
        count++;
    }
  }

  if(count != sector->numlights)
  {
    if(sector->lightlist)
      Z_Free(sector->lightlist);
    sector->lightlist = Z_Malloc(sizeof(ff_lightlist_t) * count, PU_LEVEL, 0);
    sector->numlights = count;
    memset(sector->lightlist, 0, sizeof(ff_lightlist_t) * count);
  }
  else
  {
    // clear existing lightlist 
    memset(sector->lightlist, 0, sizeof(ff_lightlist_t) * count);
  }

  // init [0] to sector light
  sector->lightlist[0].height = sector->ceilingheight + 1;
  sector->lightlist[0].lightlevel = &sector->lightlevel;
  sector->lightlist[0].caster = NULL;
  sector->lightlist[0].extra_colormap = sector->extra_colormap;
  sector->lightlist[0].flags = 0;

  // Work down from highest light to lowest light.
  // Determine each light in lightlist.
  maxheight = FIXED_MAX;  // down from max, previous light
  for(i = 1; i < count; i++)
  {
    bestheight = -FIXED_MAX;
    best = NULL;
    for(rover = sector->ffloors; rover; rover = rover->next)
    {
      if(!(rover->flags & FF_EXISTS) || (rover->flags & FF_NOSHADE && !(rover->flags & FF_CUTLEVEL) && !(rover->flags & FF_CUTSPRITES)))
        continue;

      // find highest topheight, lower than maxheight
      if(*rover->topheight > bestheight && *rover->topheight < maxheight)
      {
        best = rover;
        bestheight = *rover->topheight;
        continue;
      }
      // FF_DOUBLESHADOW considers bottomheight too
      if(rover->flags & FF_DOUBLESHADOW
	 && *rover->bottomheight > bestheight && *rover->bottomheight < maxheight)
      {
        best = rover;
        bestheight = *rover->bottomheight;
        continue;
      }
    }
    if(!best)  // failure escape
    {
      sector->numlights = i;
      return;
    }

    sector->lightlist[i].height = maxheight = bestheight;
    sector->lightlist[i].caster = best;
    sector->lightlist[i].flags = best->flags;
    // [WDJ] FIXME ??:
    // This is messing with the model sector, which could be used for many sectors,
    // settings are independent of this lightlist,
    // this could be done elsewhere, once.
    modelsec = &sectors[best->model_secnum];
    mapnum = modelsec->midmap;
    if(mapnum >= 0 && mapnum < num_extra_colormaps)
      modelsec->extra_colormap = &extra_colormaps[mapnum];
    else
      modelsec->extra_colormap = NULL;

    // best is highest floor less than maxheight
    if(best->flags & FF_NOSHADE)
    {
      // FF_NOSHADE, copy next higher light
      sector->lightlist[i].lightlevel = sector->lightlist[i-1].lightlevel;
      sector->lightlist[i].extra_colormap = sector->lightlist[i-1].extra_colormap;
    }
    else
    {
      // usual light
      sector->lightlist[i].lightlevel = best->toplightlevel;
      sector->lightlist[i].extra_colormap = modelsec->extra_colormap;
    }

    if(best->flags & FF_DOUBLESHADOW)
    {
      // FF_DOUBLESHADOW, consider bottomheight too.
      if(bestheight == *best->bottomheight)
      {
	// [WDJ] FIXME: segfault here in Chexquest-newmaps E2M2, best->lastlight wild value
	// Stopped segfault by init to 0, but what is this trying to do ??
	// Happens when bottom is found without finding top.
        sector->lightlist[i].lightlevel = sector->lightlist[best->lastlight].lightlevel;
        sector->lightlist[i].extra_colormap = sector->lightlist[best->lastlight].extra_colormap;
      }
      else
      {
        best->lastlight = i - 1;
      }
    }
  }
}


// Find light under planeheight, plain version
int   R_GetPlaneLight(sector_t* sector, fixed_t planeheight)
{
  int   i;

  for(i = 1; i < sector->numlights; i++)
    if(sector->lightlist[i].height <= planeheight)
      return i - 1;

  return sector->numlights - 1;
}


// Find light under planeheight, slight difference according to viewz
int   R_GetPlaneLight_viewz(sector_t* sector, fixed_t  planeheight)
{
  int   i;

#if 0
  // faster
  if( viewz < planeheight )
  {
      for(i = 1; i < sector->numlights; i++)
        if(sector->lightlist[i].height < planeheight)   goto found;
  }
  else
  {
      for(i = 1; i < sector->numlights; i++)
        if(sector->lightlist[i].height <= planeheight)  goto found;
  }
#else
  // smaller
  if( viewz >= planeheight )
     return R_GetPlaneLight( sector, planeheight );
   
  for(i = 1; i < sector->numlights; i++)
        if(sector->lightlist[i].height < planeheight)   goto found;
#endif
  // not found
  return sector->numlights - 1;

found:     
  return i - 1;
}



//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
// Called by R_RenderPlayerView
#if 1
// Recursive
void R_RenderBSPNode (int bspnum)
{
    node_t*     bsp;
    int         side;

    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1)
            // BP: never happen : bspnum = int, children = unsigned short
            // except first call if numsubsectors=0 ! who care ?
            R_Subsector (0);
        else
            R_Subsector (bspnum&(~NF_SUBSECTOR));
        return;
    }

    bsp = &nodes[bspnum];

    // Decide which side the view point is on.
    side = R_PointOnSide (viewx, viewy, bsp);

    // Recursively divide front space.
    R_RenderBSPNode (bsp->children[side]);

    // Possibly divide back space.
    if (R_CheckBBox (bsp->bbox[side^1]))
        R_RenderBSPNode (bsp->children[side^1]);
}
#else



//
// RenderBSPNode : DATA RECURSION version, slower :<
//
// Denis.F. 03-April-1998 : I let this here for learning purpose
//                          but this is slower coz PGCC optimises
//                          fairly well the recursive version.
//                          (it was clocked with p5prof)
//
#define MAX_BSPNUM_PUSHED 512

// Stack based descent
void R_RenderBSPNode (int bspnum)
{
    node_t*     bsp;
    int         side;

    node_t      *bspstack[MAX_BSPNUM_PUSHED];
    node_t      **bspnum_p;

    //int         visited=0;

    bspstack[0] = NULL;
    bspstack[1] = NULL;
    bspnum_p = &bspstack[2];

    // Recursively divide front space.
    for (;;)
    {
        // Recursively divide front space.
        while (!(bspnum & NF_SUBSECTOR))
        {
            bsp = &nodes[bspnum];

            // Decide which side the view point is on.
            side = R_PointOnSide (viewx, viewy, bsp);

            *bspnum_p++ = bsp;
            *bspnum_p++ = (void*) side;
            bspnum = bsp->children[side];
        }

        // Found a subsector
        if (bspnum == -1)
            R_Subsector (0);
        else
            R_Subsector (bspnum&(~NF_SUBSECTOR));

        side = (int) *--bspnum_p;
        if ((bsp = *--bspnum_p) == NULL )
        {
            // we're done
            //CONS_Printf ("Subsectors visited: %d\n", visited);
            return;
        }

        // Possibly divide back space.
        if (R_CheckBBox (bsp->bbox[side^1]))
            // dirty optimisation here!! :) double-pop done because no push!
            bspnum = bsp->children[side^1];
    }

}
#endif
