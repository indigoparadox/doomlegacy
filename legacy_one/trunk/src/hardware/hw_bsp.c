// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: hw_bsp.c,v $
// Revision 1.22  2004/07/27 08:19:38  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.21  2002/07/20 03:41:21  mrousseau
// Removed old/unused code
// Changed CutOutSubsecPoly to use the original LINEDEF's points rather than
// the SEGS for clipping to eliminate round-off errors introduced by the
// BSP builder
// Modified WalkBSPNode bounding box calculations
//
// Revision 1.20  2001/08/14 00:36:26  hurdler
// Revision 1.19  2001/08/13 17:23:17  hurdler
//
// Revision 1.18  2001/08/13 16:27:45  hurdler
// Added translucency to linedef 300 and colormap to 3d-floors
//
// Revision 1.17  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.16  2001/08/09 21:35:23  hurdler
// Add translucent 3D water in hw mode
//
// Revision 1.15  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.14  2001/05/01 20:38:34  hurdler
// Revision 1.13  2001/04/16 15:16:26  hurdler
// Revision 1.12  2000/10/04 16:21:57  hurdler
// Revision 1.11  2000/10/02 18:25:46  bpereira
// Revision 1.10  2000/08/11 19:11:57  metzgermeister
// Revision 1.9  2000/08/10 14:16:25  hurdler
// Revision 1.8  2000/08/03 17:57:42  bpereira
// Revision 1.7  2000/08/03 17:32:31  metzgermeister
// Revision 1.6  2000/03/13 21:41:40  linuxcub
// Revision 1.5  2000/03/12 23:01:29  linuxcub
//
// Revision 1.4  2000/03/06 18:44:00  hurdler
// hack for the polypoolsize problem
//
// Revision 1.3  2000/03/06 15:24:24  hurdler
// remove polypoolsize limit
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      convert Doom map
//
//-----------------------------------------------------------------------------

#include <math.h>

#include "../doomincl.h"
#include "hw_glob.h"
#include "../r_local.h"
#include "../z_zone.h"
#include "../console.h"
#include "../v_video.h"
#include "../m_menu.h"
#include "../i_system.h"
#include "../i_video.h"



//#define DEBUG_HWBSP

// Allocate poly from ZAlloc.
#define ZPLANALLOC


// --------------------------------------------------------------------------
// This is global data for planes rendering
// --------------------------------------------------------------------------

// ---- Polygon vertexes
// Separate allocations for level map vertexes, and BSP vertexes.

// Floating poly for level map vertexes.
// Can be indexed by level map vertex number.
polyvertex_t*      poly_vert = NULL;


// Create float poly vert from level map vertexes.
// These are freed by Z_Free( PU_HWRPLANE ).
static
void  create_poly_vert( void )
{
    polyvertex_t * pv;
    size_t size = sizeof(polyvertex_t) * numvertexes;
    int i;

    poly_vert = Z_Malloc(size, PU_HWRPLANE, NULL);
    pv = &poly_vert[0];
    for( i=0; i<numvertexes; i++)
    {
        pv->x = FIXED_TO_FLOAT( vertexes[i].x );
        pv->y = FIXED_TO_FLOAT( vertexes[i].y );
        pv++;
    }
}

// Return true if p1 is a level map polyvertex in the poly_vert structure.
static inline
boolean  in_poly_vert( polyvertex_t * p1 )
{
    return( (p1 >= poly_vert) && (p1 < &poly_vert[numvertexes]) );
}



#define POLYSTORE_NUM_VERT  256
typedef struct polyvertex_store_s {
   struct polyvertex_store_s *  next;  // linked list
   int    num_vert_used;
   polyvertex_t  pv[POLYSTORE_NUM_VERT];
} polyvertex_store_t;

// Extra poly vertex for BSP splits, segs, and divlines.
polyvertex_store_t *   polyvert_store = NULL;


// --- Same vertex

#if 0
//Hurdler: it's not used anymore
static
boolean NearVertex (polyvertex_t* p1, polyvertex_t* p2)
{
    if (fabsf( p2->x - p1->x ) > 1.5f)
       return false;
    if (fabsf( p2->y - p1->y ) > 1.5f)
       return false;
    // p1 and p2 are considered the same vertex
    return true;
}
#endif

// If two vertex coords have a x and y difference of less than 1 FRACUNIT,
// they could be considered the same point.
// Note: hardcoded value, 1.0f could be anything else.
//#define SAME_DIST   1.5f
// Dist 0.4999 cures HOM in Freedoom map09
#define SAME_DIST   0.4999f
//  ep : the max difference in x or y.
static
boolean SameVertex (polyvertex_t* p1, polyvertex_t* p2, float ep )
{
    if (fabsf( p2->x - p1->x ) > ep )
       return false;
    if (fabsf( p2->y - p1->y ) > ep )
       return false;
    // p1 and p2 are considered the same vertex
    return true;
}


// Get a polyvertex from the BSP polyvertex store.
// These are freed by Z_Free( PU_HWRPLANE ).
static
polyvertex_t *  new_polyvertex( void )
{
    polyvertex_store_t * psp = polyvert_store;

    if( ! polyvert_store
        || ( polyvert_store->num_vert_used >= POLYSTORE_NUM_VERT ) )
    {
        // Need another storage unit.
        polyvert_store = Z_Malloc(sizeof(polyvertex_store_t), PU_HWRPLANE, NULL);
        polyvert_store->next = psp;  // link for search
        polyvert_store->num_vert_used = 0;
    }

    // Return the next polyvertex in the storage unit.
    return & polyvert_store->pv[ polyvert_store->num_vert_used++ ];
}

// Search both the vertex lists for a close vertex.
//  ep: how close they must be to be the same ( 0.001 to 1.5 )
static
polyvertex_t *  find_close_polyvertex( float x, float y, float ep )
{
    polyvertex_store_t *   psv;
    int i;

    // Search level map vertexes.
    polyvertex_t * pv = poly_vert;
    for( i=numvertexes; i>0; i--)
    {
        if( ( fabsf( pv->x - x ) < ep )
            && ( fabsf( pv->y - y ) < ep ) )
            return pv;  // close enough to be the same vertex
        pv++;
    }
    // Search extra BSP vertexes.
    psv = polyvert_store;
    while( psv )
    {
        // Search all vertex in a polyvertex_store_t
        pv = psv->pv;
        for( i=psv->num_vert_used; i>0; i--)
        {
            if( ( fabsf( pv->x - x ) < ep )
                && ( fabsf( pv->y - y ) < ep ) )
                return pv;  // close enough to be the same vertex
            pv++;
        }
        psv = psv->next;
    }
    return NULL;  // none found
}

// Store a new polyvertex.
// Search for an existing vertex that is within ep.
// Otherwise make a new extra vertex.
//  ep: how close an existing vertex must be to be the same ( 0.001 to 1.5 )
static
polyvertex_t *  store_polyvertex( polyvertex_t * vert, float ep )
{
    polyvertex_t * vp = find_close_polyvertex( vert->x, vert->y, ep );
    if( ! vp )
    {
        vp = new_polyvertex();   // new BSP polyvertex
        vp->x = vert->x;
        vp->y = vert->y;
    }
    return vp;
}

// Create a polyvertex for the vertex.
//  v1 : fixed point vertex
//  ep: how close an existing vertex must be to be the same ( 0.001 to 1.5 )
static
polyvertex_t *  store_vertex( vertex_t * v1, float ep )
{
    float fx = FIXED_TO_FLOAT( v1->x );
    float fy = FIXED_TO_FLOAT( v1->y );
    polyvertex_t * vp = find_close_polyvertex( fx, fy, ep );
    if( ! vp )
    {
        vp = new_polyvertex();   // new BSP polyvertex
        vp->x = fx;
        vp->y = fy;
    }
    return vp;
}



// ---- Working polygons
// Have ptr to vertex instead of copy, to make handling same vertex easier.
// Polygons are stored in clockwise vertex order.

// Working poly
// A convex 'plane' polygon, clockwise order
typedef struct {
    int          num_alloc, numpts;  // allocation size and how many used
    polyvertex_t * * ppts;  // ptr to array of ptrs
                            // Allocate with Z_Malloc, PU_HWRPLANE
} wpoly_t;

// Most basic initialize.
static
void wpoly_init_0( wpoly_t * wpoly )
{
    wpoly->numpts = 0;
    wpoly->num_alloc = 0;
    wpoly->ppts = NULL;
}

// Initialize at an initial size.
//  num_alloc : num vertex, greater than 0
static
void wpoly_init_alloc( int num_alloc, wpoly_t * wpoly )
{
    wpoly->numpts = 0;
    wpoly->num_alloc = num_alloc;
    wpoly->ppts = Z_Malloc((sizeof(void*) * num_alloc), PU_HWRPLANE, NULL);
}

// Frees the allocation used by the wpoly.
static
void wpoly_free( wpoly_t * wpoly )
{
    wpoly->num_alloc = 0;
    wpoly->numpts = 0;
    if( wpoly->ppts )
    {
        Z_Free( wpoly->ppts );
        wpoly->ppts = NULL;
    }
}

#if 0
// Unused
// Will free current content, and allocate a new size, empty.
//  num_alloc : num vertex, greater than 0
static
void wpoly_free_alloc( int num_alloc, /*INOUT*/ wpoly_t * wpoly )
{
    wpoly->numpts = 0;
    if( wpoly->ppts )
        Z_Free( wpoly->ppts );

    // New array allocation within the wpoly
    wpoly->num_alloc = num_alloc;
    if( num_alloc <= 0 )
    {
        wpoly->ppts = NULL;        
        return;
    }
    wpoly->ppts = Z_Malloc((sizeof(void*) * num_alloc), PU_HWRPLANE, NULL);
}
#endif


#if 0
// Not Used
// Resize, keeping current content.
static
void wpoly_resize( int num_points, /*INOUT*/ wpoly_t * wpoly )
{
    size_t  size;
    polyvertex_t * * old_pts;  // array of ptr

    if( num_points <= wpoly->num_alloc )   return;

    old_pts = wpoly->ppts;
    // New array allocation within the wpoly
    // Due to the cost of allocation, alloc some extra.
    wpoly->num_alloc = num_points + 3;
    size = sizeof( void* ) * wpoly->num_alloc;
    wpoly->ppts = Z_Malloc(size, PU_HWRPLANE, NULL);
    if( old_pts )
    {
        // Copy old array to new, and release old array
        memcpy( wpoly->ppts, old_pts, sizeof( void* ) * wpoly->numpts );
        Z_Free( old_pts );
    }
}
#endif


// Move all vertex from one poly to another.
//   from_poly :  source, is left empty
//   to_poly : previous content is lost
static
void wpoly_move( wpoly_t * from_poly, wpoly_t * to_poly )
{
    if( to_poly->ppts )
        wpoly_free( to_poly );

    *to_poly = *from_poly;  // copy ptrs and sizes
    // Content moved, cannot free.
    from_poly->ppts = NULL;
    from_poly->num_alloc = 0;
    from_poly->numpts = 0;
    // from_poly is empty.
}

// Append a range from one poly to another poly.
// Does not alloc more, will trunctate the append instead.
static
void  wpoly_append( wpoly_t * src_poly, int copy_from, int copy_cnt,
                  /*OUT*/ wpoly_t * dest_poly )
{
    polyvertex_t ** pvp;
    int n;

#ifdef DEBUG_HWBSP
    if( copy_cnt > src_poly->numpts )
    {
        GenPrintf( EMSG_error, "wpoly_append, exceeds src bounds, copy_from= %i, copy_cnt= %i, src numpts= %i\n",
                   copy_cnt, src_poly->numpts );
    }
#endif   

    // Prevent writes beyond our allocation.
#ifdef DEBUG_HWBSP
    if( copy_cnt > dest_poly->num_alloc - dest_poly->numpts )
    {
        GenPrintf( EMSG_error, "wpoly_append, exceeds dst allocation, copy_cnt= %i, copy_to= %i, dest numpts= %i\n",
                   copy_cnt, dest_poly->numpts+1, dest_poly->numpts );
    }
#endif
    if( copy_cnt > dest_poly->num_alloc - dest_poly->numpts )
        copy_cnt = dest_poly->num_alloc - dest_poly->numpts; // limit the append
#ifdef DEBUG_HWBSP
    if( copy_cnt <= 0 )
    {
        GenPrintf( EMSG_error, "wpoly_append, zero copy cnt, copy_cnt= %i\n",
                   copy_cnt );
    }
#endif
    if( copy_cnt <= 0 )  return;
   
    pvp = & dest_poly->ppts[ dest_poly->numpts ];  // append
    dest_poly->numpts += copy_cnt;  // before copy_cnt gets decremented

    n = src_poly->numpts - copy_from;  // vertexes to end of poly
    if( copy_cnt > n )  // too many, must rollover
    {
        // Partial copy, up to end of poly
        memcpy( pvp, &(src_poly->ppts[copy_from]), n*sizeof(void*) );
        pvp += n;
        // Rollover to start of src_poly
        copy_cnt -= n;
        copy_from = 0;
    }
#ifdef DEBUG_HWBSP
    if( copy_from + copy_cnt > src_poly->numpts )
    {
        GenPrintf( EMSG_error, "wpoly_append, exceeds src bounds, copy_from= %i, copy_cnt= %i, src numpts= %i\n",
                   copy_from, copy_cnt, src_poly->numpts );
    }
#endif   
#ifdef DEBUG_HWBSP
    if( (pvp - dest_poly->ppts) + copy_cnt > dest_poly->numpts )
    {
        GenPrintf( EMSG_error, "wpoly_append, exceeds dst bounds, copy_to= %i, copy_cnt= %i, numpts= %i\n",
                   (pvp - dest_poly->ppts), copy_cnt, dest_poly->numpts );
    }
#endif
    if( copy_cnt > 0 )
    {
        memcpy( pvp, &(src_poly->ppts[copy_from]), copy_cnt*sizeof(void*) );
    }
}


// Insert some new vertex, and then,
// copy some of another poly to the destination poly.
//  v1, v2 : polyvertex to be inserted as first vertex of poly, in this order
//  src_poly : copy from src_poly
//  copy_from, copy_cnt : the indexes of the vertexes to copy
//  dest_poly : the destination poly
static
void  wpoly_split_copy( polyvertex_t * v1, polyvertex_t * v2,
                         wpoly_t * src_poly, int copy_from, int copy_cnt,
                         /*OUT*/ wpoly_t * dest_poly )
{
    polyvertex_t ** pvp;
    int n = 0;

    // Count the dest vertexes.
    if( v1 )  n++;
    if( v2 )  n++;
    // Free old content, new allocation.
    wpoly_free( dest_poly );
    wpoly_init_alloc( n + copy_cnt, dest_poly );

    pvp = dest_poly->ppts;

    // First two points of the dest_poly are the dividing seg.
    if( v1 )
        *pvp++ = v1;
    if( v2 )
        *pvp++ = v2;

    dest_poly->numpts = n;  // v1 and v2
    wpoly_append( src_poly, copy_from, copy_cnt, /*OUT*/ dest_poly );
 }

// Insert vertexes into the destination poly, cutout some vertexes, save some.
//  v1, v2 : polyvertex to be inserted as first vertex of poly, in this order
//  v_from, v_cnt : the indexes of the vertexes to save
//  xpoly : the source and destination poly
static
void  wpoly_insert_cut( polyvertex_t * v1, polyvertex_t * v2,
                        int v_from, int v_cnt,
                        /*INOUT*/ wpoly_t * xpoly )
{
    wpoly_t  tmp_poly;

    tmp_poly = *xpoly;  // save ptrs and sizes
    xpoly->ppts = NULL;  // so does not get freed
    wpoly_split_copy( v1, v2, &tmp_poly, v_from, v_cnt, /*OUT*/ xpoly );
    wpoly_free( &tmp_poly );  // release saved poly content
}


// Insert a vertex into the destination poly, at a position.
//  v1 : polyvertex to be inserted
//  v_at : the index where v1 is inserted
//  xpoly : the source and destination poly
static
void  wpoly_insert_vert( polyvertex_t * v1, int v_at,
                        /*INOUT*/ wpoly_t * xpoly )
{
    wpoly_t  tmp_poly;
    int numpts = xpoly->numpts;

    if( v_at > numpts )  return;

    tmp_poly = *xpoly;  // save ptrs and sizes
    // Copy back from tmp_poly to xpoly
    wpoly_init_alloc( numpts + 1, xpoly );
    xpoly->numpts = numpts + 1;
    if( v_at > 0 )
    {
        memcpy( &(xpoly->ppts[0]), &(tmp_poly.ppts[0]), sizeof(void*) * v_at );
    }
    xpoly->ppts[v_at] = v1;  // insert
    if( v_at < numpts )
    {
        memcpy( &(xpoly->ppts[v_at + 1]), &(tmp_poly.ppts[v_at]),
                sizeof(void*) * (numpts - v_at) );
    }
    wpoly_free( &tmp_poly );  // release saved poly content
}




// ---- Subsectors

// Array of poly_subsector_t,
// Index by bsp subsector num,  0.. num_poly_subsector-1
poly_subsector_t*   poly_subsectors = NULL;
wpoly_t *   wpoly_subsectors = NULL;  // working subsectors


// extra subsectors are subsectors without segs, added for the plane polygons
#define NUM_EXTRA_SUBSECTORS       50
unsigned int  num_poly_subsector;
unsigned int  num_alloc_poly_subsector;


// ==========================================================================
//                                    FLOOR & CEILING CONVEX POLYS GENERATION
// ==========================================================================

#ifdef DEBUG_HWBSP
//debug counters
static int nobackpoly_cnt=0;
static int skipcut_cnt=0;
static int total_subsecpoly_cnt=0;
#endif

// --------------------------------------------------------------------------
// Polygon fast alloc / free
// --------------------------------------------------------------------------

#define ZPLANALLOC

#ifndef ZPLANALLOC
#define POLY_ALLOCINC  4096
#define POLY_VERTINC    256
static byte*    gr_polypool = NULL;
static unsigned int  gr_polypool_free = 0;
#endif

// only between levels, clear poly pool
static void HWR_ClearPolys (void)
{
    Z_FreeTags( PU_HWRPLANE, PU_HWRPLANE );
#ifndef ZPLANALLOC
    gr_polypool = NULL;
    gr_polypool_free = 0;
#endif
    poly_vert = NULL;
    polyvert_store = NULL;    
}

static void HWR_Free_poly_subsectors ( void );

// allocate  pool for fast alloc of polys
void HWR_InitPolyPool (void)
{
    HWR_ClearPolys ();
}

void HWR_FreePolyPool (void)
{
    HWR_Free_poly_subsectors ();
    HWR_ClearPolys();
}

static poly_t* HWR_AllocPoly (int numpts)
{
    poly_t*     p;
    int         size;

    size = sizeof(poly_t) + sizeof(polyvertex_t) * numpts;
#ifdef ZPLANALLOC
    p = Z_Malloc(size, PU_HWRPLANE, NULL);
#else
    if(gr_polypool_free < size)
    {
        // Allocate another pool.
        // Z_FreeTags reclaims the leftover memory of previous pool.
        gr_polypool_free = POLY_ALLOCINC;
        gr_polypool = Z_Malloc(gr_polypool_free, PU_HWRPLANE, NULL);
    }

    p = (poly_t*) gr_polypool;
    gr_polypool += size;
    gr_polypool_free -= size;
#endif
    p->numpts = numpts;    
    return p;
}


#if 0
// Unused
// Adding a vertex to a poly requires a new poly allocation with larger size.
// Free the old poly memory.
static void HWR_FreePoly (poly_t* poly)
{
#ifdef ZPLANALLOC
    Z_Free(poly);
#else
    // No free list, cannot reclaim memory.
    // Each poly is a different size.
    unsigned int  size = sizeof(poly_t) + sizeof(polyvertex_t) * poly->numpts;
    memset(poly,0,size);
#endif
}
#endif

#ifdef DEBUG_HWBSP
// print poly for debugging
void pwpoly( wpoly_t * poly )
{
    int i;
    for( i=0; i<poly->numpts; i++ )
       if( poly->ppts[i] )
           printf( "(%6.2f,%6.2f)", poly->ppts[i]->x, poly->ppts[i]->y );
    printf("\n");
}
// print poly for debugging
void ppoly( poly_t * poly )
{
    int i;
    for( i=0; i<poly->numpts; i++ )
       printf( "(%6.2f,%6.2f)", poly->pts[i].x, poly->pts[i].y );
    printf("\n");
}
#endif



// The BSP has the partition lines that define the subsectors.  They do not
// exist anywhere else.

// The subsectors of the BSP only have segs that are parts of linedefs.
// The subsector segs are not in any special order.
// Subsectors with 0 segs are skipped in building the BSP, so those are missing.
// Such subsectors are defined only by the dividing lines.
// The subsector of the BSP often encloses some adjoining void space.

// Deep water in BSP: The deep water sector uses a linedef referencing a
// remote sector.  The BSP will have extra dividing line polygon splits that
// are useless.  The BSP builder got confused by the linedefs with remote
// sector references.
// This will result in an attempted polygon split that misses entirely.


// --- Divide line

typedef enum {
   DVL_none,  // no divide
   DVL_v1,    // divide at v1 end of segment
   DVL_mid,   // divide between v1 and v2
   DVL_v2,    // divide at v2 end of segment
} divline_e;

typedef struct { 
    float x, y;
    float dx, dy;
} fdivline_t;

typedef struct { 
    polyvertex_t  divpt;
    polyvertex_t * vertex;  // when same as segment endpoint
    float divfrac; // how far along the partline vector is the crossing point
    int before, after;  // index modifiers for hitting a vertex
    boolean     at_vert;  // crossing point is at a vertex
} div_result_t;

// Return interception along bsp line (partline),
// with the polygon segment

// BOOMEDIT.WAD has a vertex error of .21
#define  DIVLINE_VERTEX_DIFF   0.45f

//  partline : the dividing line
//  p1, p2 : the polygon segment
//  result : the result of the division
static
divline_e
  fracdivline (fdivline_t* partline, polyvertex_t* v1, polyvertex_t* v2,
               /*OUT*/ div_result_t * result )
{
    double  frac;
    double  num, den; // numerator, denominator
    double  v1x,v1y,v1dx,v1dy;  // polygon side vector, v1->v2
    double  v3x,v3y,v3dx,v3dy;  // partline vector

    // a segment of a polygon
    v1x  = v1->x;
    v1y  = v1->y;
    v1dx = v2->x - v1->x;
    v1dy = v2->y - v1->y;

    // the bsp partition line
    v3x  = partline->x;
    v3y  = partline->y;
    v3dx = partline->dx;
    v3dy = partline->dy;

    den = v3dy*v1dx - v3dx*v1dy;
    if (fabs(den) < 1.0E-36) // avoid check of float for exact 0
        return DVL_none;  // partline and polygon side are effectively parallel

    // first check the frac along the polygon segment,
    // (do not accept hit with the extensions)
    num = (v3x - v1x)*v3dy + (v1y - v3y)*v3dx;
    frac = num / den;
    // 0= cross at v1, 1.0= cross at v2
    if (frac<0.0 || frac>1.0)
        return DVL_none;  // not within the polygon side

    // now get the frac along the BSP line
    // which is useful to determine what is left, what is right
    num = (v3x - v1x)*v1dy + (v1y - v3y)*v1dx;
#if 1
    result->divfrac = num / den;  // how far along partline vector

    // [WDJ] find the interception point along the segment.
    // It should be slightly more accurate because it is always closer to the
    // crossing point than arbitrary positions on the partition line.
    result->divpt.x = v1x + v1dx*frac;
    result->divpt.y = v1y + v1dy*frac;
#else
    double frac2 = num / den;
    partline->divfrac = frac2;  // how far along partline vector

    // find the interception point along the partition line
    result->divpt.x = v3x + v3dx*frac2;
    result->divpt.y = v3y + v3dy*frac2;
#endif

    // Determine if dividing point is one of the end vertex.
    // Set before and after indexes, relative to v1 index.
    if( frac < 0.05
        && SameVertex( &result->divpt, v1, DIVLINE_VERTEX_DIFF ) )
    {
        result->vertex = v1;
        result->before = -1;  // before v1
        result->after = 1;   // at v2
        result->at_vert = true;
        return DVL_v1;
    }
    if( frac > 0.95
        && SameVertex( &result->divpt, v2, DIVLINE_VERTEX_DIFF ) )
    {
        result->vertex = v2;
        result->before = 0; // at v1
        result->after = 2;  // after v2
        result->at_vert = true;
        return DVL_v2;
    }
   
    // Middle split
    result->vertex = NULL;
    result->before = 0; // at v1
    result->after = 1;  // at v2
    result->at_vert = false;
    return DVL_mid;
}


// Return true when vertex is on right side of divline.
// On the divline is allowed to be rightside.
// Adapted from function in prboom.
static
boolean  point_rightside( fdivline_t * dl, polyvertex_t * v4 )
{
    // Cross product of dl and vector dl->(x,y) to v4,
    // is > 0 when v4 is to right side of divline.
    // Viewed along divline from vertex, looking towards positive dx,dy.
    // If divline is rotated until dy>0 and dx=0, then true when rotated
    // vertex position is to the right of the divline (v4->x > dl->x).
    return
       ( (((double)(v4->x) - (double)(dl->x)) * (double)(dl->dy))
       - (((double)(v4->y) - (double)(dl->y)) * (double)(dl->dx))
       >= 0 );
}

// Return the cross product of the vector p1->p2, and p1->v4.
// The cross product is > 0 when v4 is to the right side of the vector.
// If the coordinates are rotated until the vector dy>0 and dx=0, then the
// cross product is > 0 when v4 is to the right of the vector.
static
double  cross_product( polyvertex_t * p1, polyvertex_t * p2, polyvertex_t * v4 )
{
    return
    ( ((double)(v4->x) - (double)(p1->x)) * ((double)(p2->y) - (double)(p1->y))
    - ((double)(v4->y) - (double)(p1->y)) * ((double)(p2->x) - (double)(p1->x))
    );
}

#if 0
// Unused
static
double  distance( polyvertex_t * p1, polyvertex_t * p2 )
{
    double dx = (double)(p2->x) - (double)(p1->x);
    double dy = (double)(p2->y) - (double)(p1->y);
    return  sqrt( dx*dx + dy*dy );
}
#endif



  
// Split a _CONVEX_ polygon in two convex polygons.
//   poly : polygon to be split by divline
// outputs:
//   frontpoly : polygon on right side of bsp line
//   backpoly  : polygon on left side
//
// Called from: WalkBSPNode
static void SplitPoly (fdivline_t* dlnp, wpoly_t* poly,
       /*OUT*/  wpoly_t* frontpoly, wpoly_t* backpoly)
{
    // Split poly at A and B.
    wpoly_t * polyA;  // the poly from A to B, clockwise
    wpoly_t * polyB;  // the poly from B to A, clockwise
    int  n,i,j;
    divline_e     dle;
    div_result_t  A, B;  // dividing points
    div_result_t  * result;

    result = &A; // Setup to get crossing point A
    for (i=0; i<poly->numpts; i++)
    {
        // i, j are one side of the poly
        j=i+1;
        if (j==poly->numpts) j=0;  // wrap poly

        // Find A and B points
        dle = fracdivline (dlnp, poly->ppts[i], poly->ppts[j], result);
        if ( dle == DVL_none )  continue;
       
        // have dividing pt
        if (result == &A)
        {
            // Split at A
            // Dependent upon dle, setup in fracdivline.
            A.before += i;
            A.after += i;
            result = &B;  // Setup to get crossing point B
            continue;
        }
        // The partition line can cross at a vertex, between two segments,
        // or the two points are so close, they can be considered as one.
        // Crossing point B must be another vertex.

        // When ( dle == DVL_v1 || dle == DVL_v2 ) then test for same vertex.
        // It is NULL for DVL_mid.
        // When dividing point is at a vertex, it is found at next segment too.
        if( B.vertex  // ( dle == DVL_v1 || dle == DVL_v2 )
            && B.vertex == A.vertex )
               continue;

        // Split at B
        // Dependent upon dle, setup in fracdivline.
        B.before += i;
        B.after += i;  // linear, no rollover
        goto split_poly;  // got 2 points
    }
    goto no_split;

split_poly:
    // Less aggressive same vertex, to avoid kinking line.
    if( A.vertex == NULL )
    {
        A.vertex = store_polyvertex( & A.divpt, 0.01 );
    }
    if( B.vertex == NULL )
    {
        B.vertex = store_polyvertex( & B.divpt, 0.01 );
    }

    // The frontpoly is the one on the 'right' side
    // of the partition line.
    if (A.divfrac > B.divfrac)
    {
        polyA = frontpoly;
        polyB = backpoly;
    }else{
        polyA = backpoly;
        polyB = frontpoly;
    }

    // Form PolyA
    // Number of points from A to B clockwise.
    n = B.before - A.after + 1;
    if (n>0)
    {
        // B, A, poly from A to B clockwise
        wpoly_split_copy( B.vertex, A.vertex, poly, A.after, n, /*OUT*/ polyA );
    }
    else
        polyA->numpts = 0;

    // Form PolyB
    // Number of points from B to A clockwise.
    n = A.before + poly->numpts - B.after + 1;
    if (n>0)
    {
        // A, B, poly from B to A clockwise
        wpoly_split_copy( A.vertex, B.vertex, poly,
            ((B.after < poly->numpts)? B.after : (B.after - poly->numpts)),
            n, /*OUT*/ polyB );
    }
    else
        polyB->numpts = 0;

#ifdef DEBUG_HWBSP
    // Test that frontpoly is to the right
    if( frontpoly->numpts >= 2
        && ! point_rightside( dlnp, frontpoly->ppts[2] ) )
       GenPrintf( EMSG_warn, "SplitPoly: frontpoly on left side\n" );
    // Test that backpoly is to the left
    if( backpoly->numpts >= 2
        && point_rightside( dlnp, backpoly->ppts[2] ) )
       GenPrintf( EMSG_warn, "SplitPoly: backpoly on right side\n" );
#endif

    return;

no_split:
    // no split : the partition line is either parallel and
    // aligned with one of the poly segments, or the line is totally
    // out of the polygon and doesn't traverse it (happens if the bsp
    // is fooled by some trick where the sidedefs don't point to
    // the right sectors)
    if (result == &A)
    {
#ifdef DEBUG_HWBSP
        GenPrintf( EMSG_debug, "DEBUG: SplitPoly: divline missed entirely\n");
#endif

        // this eventually happens with 'broken' BSP's that accept
        // linedefs where each side point the same sector, that is:
        // the deep water effect with the original Doom
    }
    else if( A.vertex == NULL )
    {
#ifdef DEBUG_HWBSP
        GenPrintf( EMSG_debug,
            "DEBUG: SplitPoly: one new divide point %d (%6.2f,%6.2f) %d\n",
            A.before, A.divpt.x, A.divpt.y, A.after);
#endif
    }
    else
    {
#ifdef DEBUG_HWBSP
        GenPrintf( EMSG_debug,
            "DEBUG: SplitPoly: intersect at one vertex, %d\n",
            A.before + 1 );
#endif
    }

    // Make sure front poly is to right of partition line
    if( point_rightside( dlnp, poly->ppts[0] ) )
    {
        wpoly_move( poly, frontpoly );
        backpoly->numpts = 0;
    }
    else
    {
        wpoly_move( poly, backpoly );
        frontpoly->numpts = 0;
    }
    return;
}


// The BSP creates convex polygons, up to the point where it presents
// the subsector.  The subsector segs may adjoin void space, and if that
// is cut out of the subsector polygon it might not be convex.
// Where the cutting seg does not entirely traverse the polygon,
// it should only cut over its length, not entirely to the other side
// of the polygon.  Another seg, and maybe more, must be present to
// finish the cut to the other side of the polygon.  There is no
// assurance that these segs will keep the polygon convex.
// Example: BOOMEDIT.WAD, subsector 206 is not convex after cutting segs.
// It is cut by seg linedef 104, and seg linedef 110.

#define CUTOUT_NON_CONVEX   1

#ifdef CUTOUT_NON_CONVEX
// Force convex solution.
static
void  enforce_convex( wpoly_t * poly )
{
    polyvertex_t * rv1, * rv2, * rv3;
    int i1, i2, i3;
    int numpts = poly->numpts;

    for (i2=0; i2<numpts; )
    {
        // Check that angle at i2 is less than 180.
        i3 = i2 + 1;
        if( i3 == numpts )
            i3 = 0;
        rv3  = poly->ppts[i3];
        i1 = i2 - 1;
        if( i1 < 0 )
            i1 += numpts;
        rv1 = poly->ppts[i1];
        rv2 = poly->ppts[i2];
        // cross product to check angle
        if( cross_product( rv1, rv2, rv3 ) < 0 )
        {
            // Remove the i2 vertex, to make the polygon more convex.
            if( (i2+1) < numpts)
            {   // Shuffle down by 1
                memmove( &poly->ppts[i2], &poly->ppts[i2+1],
                         sizeof(void*)*(numpts - (i2+1)) );
            }
            numpts --;
            if( numpts <= 3 )   break;
            // Have to repeat from i1 vertex, because that vertex angle
            // changed too.
            i2 = (i1 > 0)? i1 : 0;
            continue;
        }
        i2++;
    }
    poly->numpts = numpts;
}
#endif

#ifdef CUTOUT_NON_CONVEX
// Some of the segs will only partially cross the polygon, but connect with
// another seg, and together will cross it, or form a corner.
// Trying to cut with these leads to an incomplete or non-convex polygon.
// Then latter seg cuts will have 3 or 4 crossings.
// Deal with linking the segs together separate from the seg cutting operation.

typedef struct loose_seg_s {
   struct loose_seg_s  *  next;
   seg_t *  seg;  // the seg
   polyvertex_t * p1, * p2;  // clockwise order
} loose_seg_t;

typedef struct seg_chain_s {
   struct seg_chain_s  *  next;
   loose_seg_t *  first_seg;  // head of seg-chain
   loose_seg_t *  last_seg;   // tail of seg-chain
   polyvertex_t * p1, * p2;  // ends in clockwise order
   boolean   loose1, loose2;  // loose ends of the seg-chain
   int       num_seg;
} seg_chain_t;

// Nothing to gain by making this a parameter, this saves param passing.
// Easier to deal with releasing memory.
static seg_chain_t *  seg_chain = NULL;

static
void free_first_seg_chain( void )
{
    seg_chain_t * sctp;
    loose_seg_t * lsp, * lsp2;
   
    sctp = seg_chain;
    if( sctp )
    {
        seg_chain = sctp->next;
        // Free the list of loose segs
        lsp = sctp->first_seg;
        while( lsp )
        {
           lsp2 = lsp->next;
           Z_Free( lsp );
           lsp = lsp2;
        }
        Z_Free( sctp );
    }
}

// Clear out the seg_chain structure
static
void clear_seg_chains( void )
{
    while( seg_chain )
       free_first_seg_chain();
}

// The seg-chains may be in portions, due to entry order.
static
void condense_seg_chains( void )
{
    seg_chain_t * sctp;
    seg_chain_t * sctp2;
    seg_chain_t * sctp2_prev;
    polyvertex_t * pv2;
   
    // Search seg-chains for combinations.
    sctp = seg_chain;
    while( sctp )
    {
        if( sctp->loose2 )  // p2 is loose
        {
            // Search for pv2 as loose first in another seg-chain
            pv2 = sctp->p2;
            sctp2_prev = NULL;
            sctp2 = seg_chain;
            while( sctp2 )
            {
                if( sctp2 != sctp
                    && sctp2->loose1
                    && sctp2->p1 == pv2 )   goto combine;
                sctp2_prev = sctp2;
                sctp2 = sctp2->next;
            }
        }
        sctp = sctp->next;
        continue;

  combine:
        // Append sctp2 to last of sctp seg-chain.
        sctp->last_seg->next = sctp2->first_seg;
        sctp->last_seg = sctp2->last_seg;
        sctp->p2 = sctp2->p2;
        sctp->loose2 = sctp2->loose2;
        sctp->num_seg += sctp2->num_seg;
        // Remove sctp2
        if( sctp2_prev )
           sctp2_prev->next = sctp2->next;
        else
           seg_chain = sctp2->next;
        Z_Free( sctp2 );
        continue;  // with same sctp
    }
}

// Save loose segs in the seg_chain structure.
//  B_A_order : the vertex order is B A for clockwise
static
void save_loose_seg( seg_t * loose_seg,
                     polyvertex_t * vA, polyvertex_t * vB,
                     boolean  looseA, boolean  looseB,
                     boolean B_A_order )
{
    loose_seg_t * lsp;
    seg_chain_t * sctp;
    polyvertex_t * pv1, * pv2;
    boolean  loose1, loose2;

    // B_A_order is determined by the order of the polygon sides.
    if( B_A_order )
    {
        pv1 = vB;
        pv2 = vA;
        loose1 = looseB;
        loose2 = looseA;
    }
    else
    {
        pv1 = vA;
        pv2 = vB;
        loose1 = looseA;
        loose2 = looseB;
    }
    lsp = Z_Malloc( sizeof(loose_seg_t), PU_HWRPLANE, NULL);
    lsp->seg = loose_seg;
    lsp->p1 = pv1;
    lsp->p2 = pv2;
    lsp->next = NULL;

    // Search seg-chains
    sctp = seg_chain;
    while( sctp )
    {
        if( loose2
            && sctp->loose1
            && sctp->p1 == pv2 )
        {
            // first of seg-chain
            lsp->next = sctp->first_seg;
            sctp->first_seg = lsp;
            sctp->p1 = pv1;
            sctp->loose1 = loose1;
            sctp->num_seg++;
            return;
        }
        if( loose1
            && sctp->loose2
            && sctp->p2 == pv1 )
        {
            // last of seg-chain
            lsp->next = NULL;
            sctp->last_seg->next = lsp;
            sctp->last_seg = lsp;
            sctp->p2 = pv2;
            sctp->loose2 = loose2;
            sctp->num_seg++;
            return;
        }
        sctp = sctp->next;
    }

    // Need new seg-chain
    sctp = Z_Malloc( sizeof(seg_chain_t), PU_HWRPLANE, NULL);
    sctp->next = NULL;
    sctp->num_seg = 1;
    sctp->first_seg = lsp;
    sctp->last_seg = lsp;
    sctp->p1 = pv1;
    sctp->p2 = pv2;
    sctp->loose1 = loose1;
    sctp->loose2 = loose2;
    sctp->next = seg_chain;
    seg_chain = sctp;
}


// Apply the list of loose end seg chains.
// Return true if a possible non-convex cut is made.
static
boolean  apply_seg_chains( wpoly_t * poly )
{
    boolean check_convex = false;
    wpoly_t  comb_poly;  // combine seg-chain and poly
    loose_seg_t * lsp;
    seg_chain_t * sctp;
    polyvertex_t * rv1, * rv2;
    int n, i1, i2;
    int numpts = poly->numpts;

    // All the seg-chains are part of the polygon.  If a seg-chain is
    // outside the polygon, the polygon must be expanded to include it.
    // Otherwise there will be cracks in the floor.
    for(;;)
    {
        sctp = seg_chain;
        if( ! sctp )  break;
       
        rv1 = sctp->p1;
        rv2 = sctp->p2;
        i1 = i2 = numpts + 20;  // invalid
       
        if( sctp->loose1 || sctp->loose2 )
        {
            // FIXME: This needs to be handled.
            goto reject;
        }

        {
            // Find original crossing point.
            for( i1 = 0; i1 < numpts; i1++ )
            {
                if( poly->ppts[i1] == rv1 )  break;
            }
        }

        {
            // Find original crossing point.
            for( i2 = 0; i2 < numpts; i2++ )
            {
                if( poly->ppts[i2] == rv2 )  break;
            }
        }

        if((i1 < numpts) && (i2 < numpts))
        {
            // Insert points are still there.
            // Alloc the right size comb_poly.
            i2 ++;  // copy after i2
            if( i2 >= numpts )
               i2 -= numpts;
            n = i1 - i2;  // not inclusive of i1 or i2
            if( n < 0 )
               n += numpts;
            wpoly_init_alloc( sctp->num_seg + 1 + n, & comb_poly );

            // Copy the seg-chain into the comb_poly.
            polyvertex_t ** ppv = comb_poly.ppts;
            for( lsp = sctp->first_seg; lsp; lsp = lsp->next )
            {
                *ppv++ = lsp->p1;  // rv1 to before rv2
            }
            *ppv = rv2;
            comb_poly.numpts = sctp->num_seg + 1;

            // Save i2 to i1, which starts after rv2, to the comb_poly.
            wpoly_append( poly, i2, n, /*OUT*/ & comb_poly );
            wpoly_move( & comb_poly, poly );  // empty comb_poly
            numpts = poly->numpts;
            check_convex = true;
        }
    reject:
        free_first_seg_chain();
    }
    return check_convex;
}

#endif


// Use each seg of the poly as a partition line, keep only the
// part of the convex poly to the front of the seg (that is,
// the part inside the sector). The part behind the seg, is
// the void space and is cut out.
//
//  poly : surrounding convex polygon, non-destructive
//  ssindex : subsec index, 0..(numsubsectors-1)
// Called from: HWR_SubsecPoly
static
void  CutOutSubsecPoly ( int ssindex, /*INOUT*/ wpoly_t* poly)
{
    subsector_t* sub;
    seg_t*       lseg;  // array of seg
    int          segcount;  // number of seg in the array

    polyvertex_t * rv1, * rv2;  // A,B or B,A
    boolean   cut_at_vert;

#ifdef CUTOUT_NON_CONVEX
    boolean   check_convex = false;
    boolean   looseA, looseB;
#endif

    vertex_t *v1, *v2;
    polyvertex_t p1, p2;
    fdivline_t   cutseg;     //x,y,dx,dy as start of node_t struct
    divline_e    dle;
    
    div_result_t  A, B;  // dividing points
    div_result_t  * result;
    int  poly_num_pts, ps, n;
    int  i1, i2;
    
    poly_num_pts = poly->numpts;
    sub = &subsectors[ssindex];
    segcount = sub->numlines;
    lseg = &segs[sub->firstline];

    // For each seg of the subsector
    for(;segcount--;lseg++)
    {
        //x,y,dx,dy (like a divline)
        line_t *line = lseg->linedef;
        if( line->sidenum[1] != NULL_INDEX )
        {
            if( sides[line->sidenum[0]].sector == sides[line->sidenum[1]].sector )
            {
#ifdef DEBUG_HWBSP
                GenPrintf( EMSG_debug, "CutOutSubsecPoly: self ref line %i\n",
                           line - lines );
#endif
                continue;
            }
        }

        if( lseg->side )
        {  // side 1
            v1 = line->v2;
            v2 = line->v1;
        }
        else
        {  // side 0
            v1 = line->v1;
            v2 = line->v2;
        }
        p1.x = FIXED_TO_FLOAT( v1->x );
        p1.y = FIXED_TO_FLOAT( v1->y );
        p2.x = FIXED_TO_FLOAT( v2->x );
        p2.y = FIXED_TO_FLOAT( v2->y );

        cutseg.x = p1.x;
        cutseg.y = p1.y;
        cutseg.dx = p2.x - p1.x;
        cutseg.dy = p2.y - p1.y;
        
        // See if it cuts the convex poly
        result = &A; // Setup to get crossing point A
        for (i1=0; i1<poly_num_pts; i1++)
        {
            i2 = i1 + 1;
            if( i2 >= poly_num_pts )   i2 = 0;
            // i1, i2 are one side of the poly
            dle = fracdivline (&cutseg, poly->ppts[i1], poly->ppts[i2], result);
            if ( dle == DVL_none )  continue;
            // have dividing pt
            if (result == &A)
            {
                // Split at A
                // Dependent upon dle, setup in fracdivline.
                A.before += i1;
                A.after += i1;
                result = &B;  // Setup to get crossing point B
                continue;
            }
            // The partition line can cross at a vertex, between two segments,
            // or the two points are so close, they can be considered as one.
            // Crossing point B must be another vertex.
            
            // When ( dle == DVL_v1 || dle == DVL_v2 ) then test for
            // the same vertex.  It is NULL for DVL_mid.
            // When dividing point is at a vertex, is found at next segment too.
            if( B.vertex  // ( dle == DVL_v1 || dle == DVL_v2 )
                && B.vertex == A.vertex )  continue;

            // Split at B
            // Dependent upon dle, setup in fracdivline.
            B.before += i1;
            B.after += i1;  // linear, no rollover
            goto cut_poly;  // got 2 points
        }

        // need 2 points
        if (result == &B)
        {
                //hmmm... maybe we should NOT accept this, but this happens
                // only when the cut is not needed it seems (when the cut
                // line is aligned to one of the borders of the poly, and
                // only some times..)
                // [WDJ] This happens often (27 times in Doom2 map01).
#ifdef DEBUG_HWBSP
            skipcut_cnt++;
//	      GenPrintf( EMSG_error,
//		      "DEBUG: CutOutPoly: only one point for split line (%d %d)\n",ps,pe);
#endif
            continue;
        }
        continue;  // no split by this segment

   cut_poly:
        // Cuts that miss the polygon.
        if( (A.divfrac < 0.0) && (B.divfrac < 0.0) )  continue;
        if( (A.divfrac > 1.0) && (B.divfrac > 1.0) )  continue;

        cut_at_vert = A.at_vert && B.at_vert;  // cuts are at existing vertexes
        if( cut_at_vert )
        {
            // Skip if both crossings are on same polygon segment.
            if( A.after + 1 == B.after )  continue;
            if( A.after + poly_num_pts == B.after + 1 )  continue;
        }


#ifdef CUTOUT_NON_CONVEX
        // Cuts that may make the polygon non-convex.
        looseA = (A.divfrac < 0.0) || (A.divfrac > 1.0);
        if( looseA )
        {
            // Cannot make normal cut, A end is loose.
            if( cv_grpolyshape.value == 1 )  continue;  // fat polygons
            // Get vertex at A end of seg, v1 or v2
            A.vertex = store_polyvertex( (( A.divfrac < 0.0 )? &p1 : &p2), 0.25 );
        }
        looseB = (B.divfrac < 0.0) || (B.divfrac > 1.0);
        if( looseB )
        {
            // Cannot make normal cut, B end is loose.
            if( cv_grpolyshape.value == 1 )  continue;  // fat polygons
            // Get vertex at B end of seg, v1 or v2
            B.vertex = store_polyvertex( (( B.divfrac < 0.0 )? &p1 : &p2), 0.25 );
            if( looseA )  // from A end
            {
                // Both ends are loose.
                // All that can be done is to save the seg
                save_loose_seg( lseg, A.vertex, B.vertex, true, true,
                                ( B.divfrac < A.divfrac ) );
                continue;
            }
        }
#else
        // Reject cuts that could make the polygon non-convex.
        if( (A.divfrac < 0.0) || (A.divfrac > 1.0) )  continue;
        if( (B.divfrac < 0.0) || (B.divfrac > 1.0) )  continue;
#endif

        // Store new crossing vertex.
        if( A.vertex == NULL )
        {
            A.vertex = store_polyvertex( & A.divpt, 0.25 );
        }
        if( B.vertex == NULL )
        {
            B.vertex = store_polyvertex( & B.divpt, 0.25 );
        }

#ifdef CUTOUT_NON_CONVEX
        if( looseA || looseB )
        {
            // Cutseg does not completely cross the poly.
            // Save the seg
            save_loose_seg( lseg, A.vertex, B.vertex, looseA, looseB,
                            ( B.divfrac < A.divfrac ) );
            // Insert the crossing vertex into the poly, as a marker.
            // Must occur after the crossing vertex A, B are stored.
            if( looseA )
            {
                // A is loose, B is crossing point.
                if( B.at_vert )  continue;  // Point is at existing vertex
                rv1 = B.vertex;
                ps = B.before + 1;   // insert point
            }
            else
            {
                // B is loose, A is crossing point.
                if( A.at_vert )  continue;  // Point is at existing vertex
                rv1 = A.vertex;
                ps = A.before + 1;  // insert point
            }
            rv2 = NULL;
            n = poly_num_pts;
        }
        else
#endif
        {
            // Cutseg cuts across poly, at two points.
            // Save poly on rightside of cutting seg.	    
            if( B.divfrac < A.divfrac )
            {
                // B, A, poly from A to B clockwise
                // Number of points from A to B clockwise.
                n = B.before - A.after + 1;
                ps = A.after;
                rv1 = B.vertex;
                rv2 = A.vertex;
            }
            else
            {
                // A, B, poly from B to A clockwise
                // Number of points from B to A clockwise.
                n = A.before + poly_num_pts - B.after + 1;
                ps = B.after;
                rv1 = A.vertex;
                rv2 = B.vertex;
            }
            // If cut at existing vertexes, and it has the same number of pts,
            // then the cut is already an existing side,
            // and the polygon is not going to change.
            if( cut_at_vert && ((n+2) == poly_num_pts ) )   continue;
        }

        if( ps >= poly_num_pts )
            ps -= poly_num_pts;
        wpoly_insert_cut( rv1, rv2, ps, n, poly );
        poly_num_pts = poly->numpts;
    }

#ifdef CUTOUT_NON_CONVEX
    // After all normal seg cuts, deal with the loose end segs.
    if( seg_chain )
    {
        // Have some loose end segs.
        condense_seg_chains();
       
        check_convex = apply_seg_chains( poly );

        clear_seg_chains();
    }
   
    if( check_convex
      && ( cv_grpolyshape.value == 2 )   // Trim polygons
      && (poly_num_pts > 3) )
    {
        // Need to check convex.
        // Force convex solution, or split into convex.
        enforce_convex( poly );
    }
#endif
}


// At this point, the poly should be convex and the exact
// layout of the subsector.  It is not always the case,
// so continue to cut off the poly into smaller parts with
// each seg of the subsector.
//
//  ssindex : subsec index, 0..(numsubsectors-1)
//  poly : surrounding convex polygon, non-destructive
// Called from WalkBSPNode
static void HWR_SubsecPoly (int ssindex, wpoly_t* poly)
{
    sscount++;

    if (poly->numpts <= 0 )  return;

    if( cv_grpolyshape.value > 0 )
    {
        // Trim the subsector with the segs.
        CutOutSubsecPoly( ssindex, /*INOUT*/ poly);
    }

#ifdef DEBUG_HWBSP
    total_subsecpoly_cnt++;
#endif
}

// The bsp divline does not have enough precision.
// Search for the segs source of this divline.
static
void set_divline(node_t* bsp, fdivline_t *divline)
{
    divline->x=FIXED_TO_FLOAT( bsp->x );
    divline->y=FIXED_TO_FLOAT( bsp->y );
    divline->dx=FIXED_TO_FLOAT( bsp->dx );
    divline->dy=FIXED_TO_FLOAT( bsp->dy );
}

//Hurdler: implement a loading status
static int ls_count = 0;
static int ls_percent = 0;

static
void loading_status( void )
{
    char s[16];
    int x, y;

    I_OsPolling();
    CON_Drawer();
    sprintf(s, "%d%%", (++ls_percent)<<1);
    x = BASEVIDWIDTH/2;
    y = BASEVIDHEIGHT/2;
    V_SetupDraw( 0 | V_SCALESTART | V_SCALEPATCH | V_CENTERHORZ );
    M_DrawTextBox(x-58, y-8, 13, 1);
    V_DrawString(x-50, y, V_WHITEMAP, "Loading...");
    V_DrawString(x+50-V_StringWidth(s), y, V_WHITEMAP, s);

    I_FinishUpdate ();
}

// poly : the convex polygon that encloses all child subsectors
// Recursive
// Called from HWR_CreatePlanePolygons at load time.
static
void WalkBSPNode (int bspnum, wpoly_t* poly, unsigned short* leafnode, fixed_t *bbox)
{
    node_t*     bsp;

    wpoly_t     backpoly;
    wpoly_t     frontpoly;
    fdivline_t  fdivline;   
    polyvertex_t*  pt;
    unsigned int  subsecnum;  // subsector index
    int     i;


    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        // Subsector leaf: bspnum is a subsector number.
        subsecnum = bspnum & ~NF_SUBSECTOR;
        if( subsecnum >= numsubsectors )  goto bad_subsector;

        HWR_SubsecPoly ( subsecnum, poly );
        M_ClearBox(bbox);
 
        // Add the poly points into the bounding box.
        for (i=0; i<poly->numpts; i++)
        {
             pt = poly->ppts[i];
             M_AddToBox (bbox, (fixed_t)(pt->x * FRACUNIT), (fixed_t)(pt->y * FRACUNIT));
        }
        
        wpoly_move( poly, /*OUT*/ & wpoly_subsectors[subsecnum] );
        // poly is empty

        //Hurdler: implement a loading status
        if (ls_count-- <= 0)
        {
           ls_count = numsubsectors/50;
           loading_status();
        }
        return;
    }

    // Node reference: bspnum is another node of the tree.
    if( bspnum >= numnodes )  goto bad_node;
    bsp = &nodes[bspnum];
    set_divline(bsp, /*OUT*/ &fdivline);
    wpoly_init_0( &frontpoly );
    wpoly_init_0( &backpoly );
    SplitPoly (&fdivline, poly, &frontpoly, &backpoly);

#ifdef DEBUG_HWBSP
    //debug
    if (backpoly.numpts == 0)
        nobackpoly_cnt++;
#endif

    // Recursively divide front space.
    if (frontpoly.numpts)
    {
        WalkBSPNode (bsp->children[0], &frontpoly, &bsp->children[0], bsp->bbox[0]);

        // copy child bbox
        memcpy(bbox, bsp->bbox[0], 4*sizeof(fixed_t));
    }
    else
    {
        // [WDJ] Having no front poly is as likely as no back poly, since
        // logic in Split Poly was changed to check poly direction.
//        I_SoftError ("WalkBSPNode: no front poly, bspnum= %d\n", bspnum);
    }

    // Recursively divide back space.
    if (backpoly.numpts)
    {
        // Correct back bbox to include floor/ceiling convex polygon
        WalkBSPNode (bsp->children[1], &backpoly, &bsp->children[1], bsp->bbox[1]);

        // enlarge bbox with second child
        M_AddToBox (bbox, bsp->bbox[1][BOXLEFT  ],
                          bsp->bbox[1][BOXTOP   ]);
        M_AddToBox (bbox, bsp->bbox[1][BOXRIGHT ],
                          bsp->bbox[1][BOXBOTTOM]);
    }
    else
    {
    }

    wpoly_free( & backpoly );
    wpoly_free( & frontpoly );
    return;

bad_subsector:
    if (bspnum == -1)
    {
#if 0
            // BP: i think this code is useless and wrong because
            // - bspnum==-1 happens only when numsubsectors == 0
            // - it can't happens in bsp recursive call since bspnum is a int and children is unsigned short
            // - the BSP is complet !! (there just can have subsector without segs) (i am not sure of this point)

            // do we have a valid polygon ?
            if (poly && poly->numpts > 2)
            {
                if( verbose )
                    GenPrintf( EMSG_ver, "Poly: Adding a new subsector !!!\n");
                if (num_poly_subsector >= num_alloc_poly_subsector)
                    I_Error ("WalkBSPNode : not enough poly_subsectors\n");
                else if (num_poly_subsector > 0x7fff)
                    I_Error ("WalkBSPNode : num_poly_subsector > 0x7fff\n");

                *leafnode = (unsigned short)num_poly_subsector | NF_SUBSECTOR;
                poly_subsectors[num_poly_subsector].planepoly = poly;
                num_poly_subsector++;

                // frontpoly and backpoly are empty, and were not init.
                return;
            }
#endif
            
            //add subsectors without segs here?
            //HWR_SubsecPoly (0, NULL);
            I_Error ("WalkBSPNode : bspnum -1\n");
    }
    I_Error ("WalkBSPNode : bad secnum %i, numsectors=%i -1\n",
              subsecnum, numsubsectors );
            
bad_node:
    // frontpoly and backpoly are empty, and were not init.
    I_Error ("WalkBSPNode : bad node num %i, numnodes=%i -1\n",
              bspnum, numnodes );
}


static
void HWR_Free_poly_subsectors (void)
{
    if (poly_subsectors)
        Z_Free(poly_subsectors);
}


#define MAXDIST   (1.5f)
// BP: can't move vertex : DON'T change polygon geometry ! (convex)
//#define MOVEVERTEX
// Is vertex va  within the seg v1, v2
boolean PointInSeg(polyvertex_t* va, polyvertex_t* v1, polyvertex_t* v2)
{
    register float ax,ay,bx,by,cx,cy,d,norm;

#if 1
    // check bbox of the seg first (without altering v1, v2)
    if( v2->x > v1->x )
    {
        // check if x within seg box  v1..v2
        if( (va->x + MAXDIST) < v1->x )  goto not_in;
        if( (va->x - MAXDIST) > v2->x )  goto not_in;
    }
    else
    {
        // check if x within seg box  v2..v1
        if( (va->x + MAXDIST) < v2->x )  goto not_in;
        if( (va->x - MAXDIST) > v1->x )  goto not_in;
    }
    if( v2->y > v1->y )
    {
        // check if x within seg box  v1..v2
        if( (va->y + MAXDIST) < v1->y )  goto not_in;
        if( (va->y - MAXDIST) > v2->y )  goto not_in;
    }
    else
    {
        // check if x within seg box  v2..v1
        if( (va->y + MAXDIST) < v2->y )  goto not_in;
        if( (va->y - MAXDIST) > v1->y )  goto not_in;
    }
#else   
    // check bbox of the seg first
    register polyvertex_t* p;

    if( v1->x > v2->x )
    {
        // swap v1, v2, so v2 > v1
        p=v1;
        v1=v2;
        v2=p;
    }
    if((va->x < v1->x-MAXDIST) || (va->x > v2->x+MAXDIST))
        goto not_in;  // x not within seg box

    if( v1->y > v2->y )
    {
        // swap v1, v2, so v2 > v1
        p=v1;
        v1=v2;
        v2=p;
    }
    if((va->y < v1->y-MAXDIST) || (va->y > v2->y+MAXDIST))
        goto not_in;  // y not within seg box
#endif

    // v1 = origin
    ax= v2->x - v1->x;
    ay= v2->y - v1->y;
    norm = sqrt(ax*ax+ay*ay);  // length of seg
    ax/=norm;
    ay/=norm;  // unit vector along seg, v1->v2
    bx= va->x - v1->x;
    by= va->y - v1->y;  // vector v1->va

    // d = (a DOT b),  (product of lengths * cosine( angle ))
    d =ax*bx+ay*by;
    // bound of the seg
    if(d<0 || d>norm)
    {
        // Also excludes some va within MAXDIST of v1 or v2
        goto not_in;
    }
    // measure the error in vector bx,by as difference squared sum
    //c= (d * unit_vector_seg) - b
    cx=ax*d-bx;
    cy=ay*d-by;
#ifdef MOVEVERTEX
    if(cx*cx+cy*cy<=MAXDIST*MAXDIST)
    {
        // adjust a little the point position
        a->x=ax*d+v1->x;
        a->y=ay*d+v1->y;
        // anyway the correction is not enough
        return true;
    }
    return false;
#else
    return cx*cx+cy*cy <= MAXDIST*MAXDIST;
#endif
 not_in: 
    return false;
}


// [WDJ] The poly forming code has attempted to improve the polygons.
// Snapping of divide points to existing vertexes is one, and it may
// contribute to cracks in the floor tiling.  Snapping to an existing
// vertex may pull the edge of a polygon away from the adjoining polygon.
// Inaccuracies in the division lines may also contribute to this.
// This code attempts to find such cracks and fix the polygons to cover them.

static int num_T_vertex_fixed;

// A structure to pass in BSP recursion, reducing it to one parameter.
typedef struct {
   fixed_t max_x, min_x, max_y, min_y;
   wpoly_t * poly;  // our poly
   polyvertex_t * pt;  // T-split vertex
   polyvertex_t * before, * after;  // shared vertex before and after it
   int our_secnum, find_secnum;  // sectors
   int pt_index;
} split_T_t;

// Dist 0.4999 cures HOM in Freedoom map09
#define SEARCHSEG_VERTEX_DIST   0.4999f

// Recursive descent in BSP.
void SearchSegInBSP(int bspnum, split_T_t * stp)
{
    wpoly_t * wq = stp->poly;
    polyvertex_t * pt = stp->pt;
    unsigned int  subsecnum;
    int  numpts, i1, i2;

    for(;;)
    {
        if (bspnum & NF_SUBSECTOR)  goto got_subsector;

        // Not a subsector, visit left and right children.
        if(    (nodes[bspnum].bbox[0][BOXBOTTOM] <= stp->max_y)
            && (nodes[bspnum].bbox[0][BOXTOP   ] >= stp->min_y)
            && (nodes[bspnum].bbox[0][BOXLEFT  ] <= stp->max_x)
            && (nodes[bspnum].bbox[0][BOXRIGHT ] >= stp->min_y)
           )
            SearchSegInBSP(nodes[bspnum].children[0], stp);

        if(! ( (nodes[bspnum].bbox[1][BOXBOTTOM] <= stp->max_y)
            && (nodes[bspnum].bbox[1][BOXTOP   ] >= stp->min_y)
            && (nodes[bspnum].bbox[1][BOXLEFT  ] <= stp->max_x)
            && (nodes[bspnum].bbox[1][BOXRIGHT ] >= stp->min_y)
           ) )  break;
        // Tail recursion within loop.
        bspnum = nodes[bspnum].children[1];
    }
    return;

got_subsector:
    subsecnum = bspnum & ~NF_SUBSECTOR;
    if( subsecnum >= numsubsectors )  return;

    // For every subsector polygon different than poly
    wq = & wpoly_subsectors[subsecnum];
    if( wq == stp->poly )  return;

    numpts = wq->numpts;
    if( numpts == 0 )  return;

    // For all the vertex.
    for( i1=0; i1<numpts; i1++)
    {
        if( wq->ppts[i1] == pt )  continue;
        i2 = i1+1;
        if( i2 == numpts )  i2=0;
        if( wq->ppts[i2] == pt )  continue;
        if( PointInSeg(pt, wq->ppts[i1], wq->ppts[i2]) )
        {
            goto add_pt;
        }
    }
    return;

add_pt:
    // Insert the vertex pt into the polygon of the bsp subsector.
    wpoly_insert_vert( pt, i2, /*INOUT*/ wq );
    num_T_vertex_fixed++;
//    stp->max_y = - 0x7ffffff0;  // exit all the way
    return;
}


// search for T-intersection problem
// BP : It can be much more faster doing this at the same time of the splitpoly
// but we must use a different structure : polygon pointing on segs 
// segs pointing on polygon and on vertex (too much complicated, well not 
// really but I am soo lazy), the method described is also better for segs precision

static
void SolveTProblem (void)
{
    split_T_t  splitt;  // parameter to search
    wpoly_t  *wp;
    int     ssnum, numpts, i, j;

    if (cv_grsolvetjoin.value == 0)
        return;

    GenPrintf( EMSG_all | EMSG_now, "Solving T-joins. This may take a while. Please wait...\n");

    num_T_vertex_fixed=0;

    // For every subsector
    for(ssnum=0; ssnum<num_poly_subsector; ssnum++ )
    {
        wp = & wpoly_subsectors[ssnum];
        if( wp->numpts == 0 )  continue;

        splitt.poly = wp;
        numpts = wp->numpts;
        // For all vertex in the subsector
        for(i=0; i<numpts; i++)
        {
#ifdef DEBUG_HWBSP
            if( wp->ppts == NULL )
            {
                GenPrintf( EMSG_debug, "DEBUG: SolveT: NULL vertex, subsector= %d\n", ssnum );
            }
#endif
            // No need to process polyvertex from the level map.
            if( in_poly_vert( wp->ppts[i] ) )  continue;

            // This is a vertex added by a split.
            splitt.pt_index = i;
            splitt.pt = wp->ppts[i];
            splitt.max_x = (fixed_t)((wp->ppts[i]->x + MAXDIST) * 0x10000);
            splitt.min_x = (fixed_t)((wp->ppts[i]->x - MAXDIST) * 0x10000);
            splitt.max_y = (fixed_t)((wp->ppts[i]->y + MAXDIST) * 0x10000);
            splitt.min_y = (fixed_t)((wp->ppts[i]->y - MAXDIST) * 0x10000);
            j = i - 1;
            if( j < 0 )  j += numpts;
            splitt.before = wp->ppts[j];
            j = i + 1;
            if( j >= numpts )  j -= numpts;
            splitt.after = wp->ppts[j];

            // Check added polyvertex due to SplitPoly.
            SearchSegInBSP(numnodes-1, & splitt);
        }
    }
#ifdef DEBUG_HWBSP
    GenPrintf( EMSG_debug, "DEBUG: SolveT: div polygon line= %d\n", num_T_vertex_fixed );
#endif
}



#define NEARDIST (0.75f) 
// Only needs to be reasonably larger than NEARDIST.
#define INITIAL_MAX    (10000000000000.0f)
#define SEG_SAME_VERT   (0.5f)

// Adds polyvertex_t references to the segs.
// [WDJ] 2013/12 Removed writes of polyvertex_t* to vertex_t*, it now has its
// own ptrs.  Fewer reverse conversions are needed.
static
void AdjustSegs(void)
{
    int ssnum, j, segcount;
    seg_t* lseg;
    wpoly_t *wp;
    int v1found=0, v2found=0;
    float nearv1, nearv2;

    // for all segs in all sectors
    for(ssnum=0; ssnum<numsubsectors; ssnum++)
    {
        wp = & wpoly_subsectors[ssnum];
        if(wp->numpts == 0)
            continue;

        segcount = subsectors[ssnum].numlines;
        lseg = &segs[subsectors[ssnum].firstline];

        for(;segcount--;lseg++)
        {
            polyvertex_t sv1, sv2;  // seg v1, v2
            float distv1,distv2,tmp;

#ifdef DEBUG_HWBSP
            if( lseg->linedef->sidenum[1] != NULL_INDEX )
            {
                if( sides[lseg->linedef->sidenum[0]].sector == sides[lseg->linedef->sidenum[1]].sector )
                {
                    GenPrintf( EMSG_debug, "AdjustSegs: self ref line %i\n",
                           lseg->linedef - lines );
                    continue;
                }
            }
#endif
            sv1.x = FIXED_TO_FLOAT( lseg->v1->x );
            sv1.y = FIXED_TO_FLOAT( lseg->v1->y );
            sv2.x = FIXED_TO_FLOAT( lseg->v2->x );
            sv2.y = FIXED_TO_FLOAT( lseg->v2->y );

            nearv1=nearv2 = INITIAL_MAX;
            // find nearest existing poly pts to seg v1, v2
            for(j=0; j<wp->numpts; j++)
            {
                distv1 = wp->ppts[j]->x - sv1.x; 
                tmp    = wp->ppts[j]->y - sv1.y;
                distv1 = distv1*distv1 + tmp*tmp;
                if( distv1 <= nearv1 )
                {
                    v1found=j;
                    nearv1 = distv1;
                }
                // the same with v2
                distv2 = wp->ppts[j]->x - sv2.x; 
                tmp    = wp->ppts[j]->y - sv2.y;
                distv2 = distv2*distv2 + tmp*tmp;
                if( distv2 <= nearv2 )
                {
                    v2found=j;
                    nearv2 = distv2;
                }
            }

            // close enough to be considered the same ?
            if( nearv1<=NEARDIST*NEARDIST )
            {
                // share vertex with segs
                lseg->pv1 = wp->ppts[v1found];
            }
            else
            {
                // BP: here we can do better, using PointInSeg and compute
                // the right point position also split a polygon side to
                // solve a T-intersection, but too much work

                lseg->pv1 = store_polyvertex( &sv1, SEG_SAME_VERT );
            }
            if( nearv2<=NEARDIST*NEARDIST )
            {
                lseg->pv2 = wp->ppts[v2found];
            }
            else
            {
                lseg->pv2 = store_polyvertex( &sv2, SEG_SAME_VERT );
            }

            // recompute length 
            {
                // [WDJ] FIXED_TO_FLOAT_MULT used to add 1/2 of lsb of fixed_t fraction.
                float x=lseg->pv2->x - lseg->pv1->x + (0.5*FIXED_TO_FLOAT_MULT);
                float y=lseg->pv2->y - lseg->pv1->y + (0.5*FIXED_TO_FLOAT_MULT);
                lseg->length = sqrt(x*x+y*y)*FRACUNIT;
                // BP: debug see this kind of segs
                //if (nearv2>NEARDIST*NEARDIST || nearv1>NEARDIST*NEARDIST)
                //    lseg->length=1;
            }
        }
    }

    // check for missed segs, not in any polygon
    for( j=0; j<numsegs; j++ )
    {
        lseg = &segs[j];
        if( verbose )
        {
            if( ! ( lseg->pv1 && lseg->pv2 ) )
            {
                GenPrintf( EMSG_ver, "Seg %i, not in any polygon.\n", j );
            }
        }
        if( ! lseg->pv1 )
        {
            lseg->pv1 = store_vertex( lseg->v1, SEG_SAME_VERT );
        }
        if( ! lseg->pv2 )
        {
            lseg->pv2 = store_vertex( lseg->v2, SEG_SAME_VERT );
        }
    }
}

// Generate drawing polygons from wpoly_t versions.
static
void  finalize_polygons( void )
{
    wpoly_t *  wpoly;
    poly_t *   dpoly;  // drawing poly
    polyvertex_t *pv;
    int ssnum, ps;

    // For all segs in all sectors.
    for(ssnum=0; ssnum<numsubsectors; ssnum++)
    {
        wpoly = & wpoly_subsectors[ssnum];
        // Generate poly in poly_t format.
        // Vertex in wpoly_t are ptr, but in poly_t they are a copy of the vertex.
        dpoly = HWR_AllocPoly (wpoly->numpts);
        poly_subsectors[ssnum].planepoly = dpoly;
        pv = dpoly->pts;
        for( ps = 0; ps<wpoly->numpts; ps++ )
        {
            *pv++ = *(wpoly->ppts[ps]);  // copy of each vertex
        }
    }
}



// Call this routine after the BSP of a Doom wad file is loaded,
// and it will generate all the convex polys for the hardware renderer.
// Called from P_SetupLevel
void HWR_CreatePlanePolygons ( void )
{
    wpoly_t       rootp;
    polyvertex_t** rootpv;

    int     i;

    fixed_t     rootbbox[4];

    GenPrintf( EMSG_all | EMSG_now, "Creating polygons, please wait...\n");
    ls_percent = ls_count = 0; // reset the loading status

    HWR_ClearPolys ();

    // Enter all vertexes into the root bounding box.
    // find min/max boundaries of map
    //CONS_Printf ("Looking for boundaries of map...\n");
    M_ClearBox(rootbbox);
    for (i=0;i<numvertexes;i++)
        M_AddToBox( rootbbox, vertexes[i].x, vertexes[i].y );

    //CONS_Printf ("Generating subsector polygons... %d subsectors\n", numsubsectors);

    HWR_Free_poly_subsectors ();
    // allocate extra data for each subsector present in map
    num_alloc_poly_subsector = numsubsectors + NUM_EXTRA_SUBSECTORS;
    poly_subsectors = Z_Malloc( sizeof(poly_subsector_t) * num_alloc_poly_subsector, PU_STATIC, NULL);
    // set all data in to 0 or NULL !!!
    memset (poly_subsectors, 0, sizeof(poly_subsector_t) * num_alloc_poly_subsector);

    wpoly_subsectors = Z_Malloc( sizeof(wpoly_t) * num_alloc_poly_subsector, PU_HWRPLANE, NULL);
    memset (wpoly_subsectors, 0, sizeof(wpoly_t) * num_alloc_poly_subsector );

    // allocate table for back to front drawing of subsectors
    /*gr_drawsubsectors = (short*)malloc (sizeof(*gr_drawsubsectors) * num_alloc_poly_subsector);
    if (!gr_drawsubsectors)
        I_Error ("couldn't malloc gr_drawsubsectors\n");*/

    // The level map polyvertexes
    create_poly_vert();

    // number of the first new subsector that might be added
    num_poly_subsector = numsubsectors;

    // construct the initial convex poly that encloses the full map
    wpoly_init_alloc( 4, &rootp );  // alloc space for 4 pts
    rootp.numpts = 4;
    rootpv = rootp.ppts;
    rootpv[0] = new_polyvertex();
    rootpv[1] = new_polyvertex();
    rootpv[2] = new_polyvertex();
    rootpv[3] = new_polyvertex();
    // clockwise polygon
    // 0=lower_left, 1=upper_left, 2=upper_right, 3=lower_right
    rootpv[0]->x = rootpv[1]->x = FIXED_TO_FLOAT( rootbbox[BOXLEFT  ] );
    rootpv[2]->x = rootpv[3]->x = FIXED_TO_FLOAT( rootbbox[BOXRIGHT ] );
    rootpv[1]->y = rootpv[2]->y = FIXED_TO_FLOAT( rootbbox[BOXTOP   ] );
    rootpv[0]->y = rootpv[3]->y = FIXED_TO_FLOAT( rootbbox[BOXBOTTOM] );

    // start at head node of bsp tree
    WalkBSPNode ( numnodes-1, &rootp, NULL, rootbbox);

    SolveTProblem ();

    AdjustSegs();

    finalize_polygons();  // wpoly_t to drawing polygons
    Z_Free( wpoly_subsectors );

#ifdef DEBUG_HWBSP
    //debug debug..
    if (nobackpoly_cnt)
    {
        // should happen only with the deep water trick
        GenPrintf( EMSG_debug, "DEBUG: no back polygon cnt= %d\n", nobackpoly_cnt);
    }

    if (skipcut_cnt)
        GenPrintf( EMSG_debug, "DEBUG: cuts skipped because of only one point= %d\n", skipcut_cnt);


    GenPrintf( EMSG_debug, "DEBUG: total subsector convex polygons= %d\n", total_subsecpoly_cnt);
#endif
}
