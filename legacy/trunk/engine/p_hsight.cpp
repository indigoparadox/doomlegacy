// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by Raven Software, Corp.
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
// Revision 1.1  2002/11/16 14:17:58  hurdler
// Initial revision
//
// Revision 1.7  2002/08/21 16:58:33  vberghol
// Version 1.41 Experimental compiles and links!
//
// Revision 1.6  2002/08/20 13:56:58  vberghol
// sdfgsd
//
// Revision 1.5  2002/08/06 13:14:22  vberghol
// ...
//
// Revision 1.4  2002/07/23 19:21:41  vberghol
// fixed up to p_enemy.cpp
//
// Revision 1.3  2002/07/01 21:00:18  jpakkane
// Fixed cr+lf to UNIX form.
//
// Revision 1.2  2002/06/28 10:57:13  vberghol
// Version 133 Experimental!
//
// Revision 1.4  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.3  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.2  2001/02/10 13:20:55  hurdler
// update license
//
//
//
// DESCRIPTION:
// Heretic sight routines. Mostly unused?
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "p_maputl.h"
#include "r_main.h"
#include "r_state.h"


//=============================================================================
// CheckSight
//
//This uses specialized forms of the maputils routines for optimized performance
//=============================================================================
/*
static fixed_t  hsightzstart;                    // eye z of looker
static fixed_t  htopslope, hbottomslope;  // slopes to top and bottom of target
static int      hsightcounts[3];

//=============
//
// PTR_SightTraverse
//
//=============

static bool PTR_SightTraverse (intercept_t *in)
{
    line_t  *li;
    fixed_t slope;
    
    li = in->d.line;
    
    //
    // crosses a two sided line
    //
    P_LineOpening (li);
    
    if (openbottom >= opentop)      // quick test for totally closed doors
        return false;   // stop
    
    if (li->frontsector->floorheight != li->backsector->floorheight)
    {
        slope = FixedDiv (openbottom - hsightzstart , in->frac);
        if (slope > hbottomslope)
            hbottomslope = slope;
    }
    
    if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
    {
        slope = FixedDiv (opentop - hsightzstart , in->frac);
        if (slope < htopslope)
            htopslope = slope;
    }
    
    if (htopslope <= hbottomslope)
        return false;   // stop
    
    return true;    // keep going
}



//=================
//
// was P_SightBlockLinesIterator
//
//==================

bool Map::SightBlockLinesIterator (int x, int y )
{
    int                     offset;
    short           *list;
    line_t          *ld;
    int                     s1, s2;
    divline_t       dl;
    
    offset = y*bmapwidth+x;
    
    offset = *(blockmap+offset);
    
    for ( list = blockmaplump+offset ; *list != -1 ; list++)
    {
        ld = &lines[*list];
        if (ld->validcount == validcount)
            continue;               // line has already been checked
        ld->validcount = validcount;
        
        s1 = P_PointOnDivlineSide (ld->v1->x, ld->v1->y, &trace);
        s2 = P_PointOnDivlineSide (ld->v2->x, ld->v2->y, &trace);
        if (s1 == s2)
            continue;               // line isn't crossed
        P_MakeDivline (ld, &dl);
        s1 = P_PointOnDivlineSide (trace.x, trace.y, &dl);
        s2 = P_PointOnDivlineSide (trace.x+trace.dx, trace.y+trace.dy, &dl);
        if (s1 == s2)
            continue;               // line isn't crossed
        
        // try to early out the check
        if (!ld->backsector)
            return false;   // stop checking
        
        P_CheckIntercepts();
        // store the line for later intersection testing
        intercept_p->d.line = ld;
        intercept_p++;
        
    }
    
    return true;            // everything was checked
}

//===================
//
// P_SightTraverseIntercepts
//
// Returns true if the traverser function returns true for all lines
//===================

static bool P_SightTraverseIntercepts()
{
    int             count;
    fixed_t         dist;
    intercept_t     *scan, *in;
    divline_t       dl;
    
    count = intercept_p - intercepts;
    //
    // calculate intercept distance
    //
    for (scan = intercepts ; scan<intercept_p ; scan++)
    {
        P_MakeDivline (scan->d.line, &dl);
        scan->frac = P_InterceptVector (&trace, &dl);           
    }
    
    //
    // go through in order
    //      
    in = 0;                 // shut up compiler warning
    
    while (count--)
    {
        dist = MAXINT;
        for (scan = intercepts ; scan<intercept_p ; scan++)
            if (scan->frac < dist)
            {
                dist = scan->frac;
                in = scan;
            }
            
            if ( !PTR_SightTraverse (in) )
                return false;                   // don't bother going farther
            in->frac = MAXINT;
    }
    
    return true;            // everything was traversed
}




//==================
//P_SightPathTraverse
//
// Traces a line from x1,y1 to x2,y2, calling the traverser function for each
// Returns true if the traverser function returns true for all lines
//==================


bool P_SightPathTraverse (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
  fixed_t xt1,yt1,xt2,yt2;
  fixed_t xstep,ystep;
  fixed_t partial;
  fixed_t xintercept, yintercept;
  int     mapx, mapy, mapxstep, mapystep;
  int     count;
    
  validcount++;
  intercept_p = intercepts;
    
  if (((x1 - bmaporgx) & (MAPBLOCKSIZE-1)) == 0)
    x1 += FRACUNIT;                         // don't side exactly on a line
  if (((y1 - bmaporgy) & (MAPBLOCKSIZE-1)) == 0)
    y1 += FRACUNIT;                         // don't side exactly on a line
  trace.x = x1;
  trace.y = y1;
  trace.dx = x2 - x1;
  trace.dy = y2 - y1;
    
  x1 -= bmaporgx;
  y1 -= bmaporgy;
  xt1 = x1>>MAPBLOCKSHIFT;
  yt1 = y1>>MAPBLOCKSHIFT;
    
  x2 -= bmaporgx;
  y2 -= bmaporgy;
  xt2 = x2>>MAPBLOCKSHIFT;
  yt2 = y2>>MAPBLOCKSHIFT;
    
  // points should never be out of bounds, but check once instead of
  // each block
  if (xt1<0 || yt1<0 || xt1>=bmapwidth || yt1>=bmapheight
      ||  xt2<0 || yt2<0 || xt2>=bmapwidth || yt2>=bmapheight)
    return false;
    
  if (xt2 > xt1)
    {
      mapxstep = 1;
      partial = FRACUNIT - ((x1>>MAPBTOFRAC)&(FRACUNIT-1));
      ystep = FixedDiv (y2-y1,abs(x2-x1));
    }
  else if (xt2 < xt1)
    {
      mapxstep = -1;
      partial = (x1>>MAPBTOFRAC)&(FRACUNIT-1);
      ystep = FixedDiv (y2-y1,abs(x2-x1));
    }
  else
    {
      mapxstep = 0;
      partial = FRACUNIT;
      ystep = 256*FRACUNIT;
    }       
  yintercept = (y1>>MAPBTOFRAC) + FixedMul (partial, ystep);
    
    
  if (yt2 > yt1)
    {
      mapystep = 1;
      partial = FRACUNIT - ((y1>>MAPBTOFRAC)&(FRACUNIT-1));
      xstep = FixedDiv (x2-x1,abs(y2-y1));
    }
  else if (yt2 < yt1)
    {
      mapystep = -1;
      partial = (y1>>MAPBTOFRAC)&(FRACUNIT-1);
      xstep = FixedDiv (x2-x1,abs(y2-y1));
    }
  else
    {
      mapystep = 0;
      partial = FRACUNIT;
      xstep = 256*FRACUNIT;
    }       
  xintercept = (x1>>MAPBTOFRAC) + FixedMul (partial, xstep);
    
  //
  // step through map blocks
  // Count is present to prevent a round off error from skipping the break
  mapx = xt1;
  mapy = yt1;
    
    
  for (count = 0 ; count < 64 ; count++)
    {
      if (!P_SightBlockLinesIterator (mapx, mapy))
        {
	  hsightcounts[1]++;
	  return false;   // early out
        }
        
      if (mapx == xt2 && mapy == yt2)
	break;
        
      if ( (yintercept >> FRACBITS) == mapy)
        {
	  yintercept += ystep;
	  mapx += mapxstep;
        }
      else if ( (xintercept >> FRACBITS) == mapx)
        {
	  xintercept += xstep;
	  mapy += mapystep;
        }
        
    }
    
    
  //
  // couldn't early out, so go through the sorted list
  //
  hsightcounts[2]++;
    
  return P_SightTraverseIntercepts();
}
*/

//====================
//
// was P_CheckSight from Heretic?
//
// Returns true if a straight line between t1 and t2 is unobstructed
// look from eyes of t1 to any part of t2
//
//====================
/*
bool Map::CheckSightH(Actor *t1, Actor *t2)
{
// not used?
  int             s1, s2;
  int             pnum, bytenum, bitnum;
    
  //
  // check for trivial rejection
  //
  s1 = (t1->subsector->sector - sectors);
  s2 = (t2->subsector->sector - sectors);
  pnum = s1*numsectors + s2;
  bytenum = pnum>>3;
  bitnum = 1 << (pnum&7);
    
  if (rejectmatrix[bytenum] & bitnum)
    {
      hsightcounts[0]++;
      return false;           // can't possibly be connected
    }
    
  //
  // check precisely
  //              
  hsightzstart = t1->z + t1->height - (t1->height>>2);
  htopslope = (t2->z+t2->height) - hsightzstart;
  hbottomslope = (t2->z) - hsightzstart;
    
  return P_SightPathTraverse(t1->x, t1->y, t2->x, t2->y);
}
*/