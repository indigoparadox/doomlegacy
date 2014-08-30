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

void I_FinishUpdate (void);

// --------------------------------------------------------------------------
// This is global data for planes rendering
// --------------------------------------------------------------------------

poly_subsector_t*   poly_subsectors = NULL;

// newsubsectors are subsectors without segs, added for the plane polygons
#define NEWSUBSECTORS       50
int  totsubsectors;
int  addsubsector;

typedef struct { 
    float x;
    float y;
    float dx;
    float dy;
    polyvertex_t  divpt;
    float divfrac;
} fdivline_t;

// ==========================================================================
//                                    FLOOR & CEILING CONVEX POLYS GENERATION
// ==========================================================================

//debug counters
static int nobackpoly=0;
static int skipcut=0;
static int totalsubsecpolys=0;

// --------------------------------------------------------------------------
// Polygon fast alloc / free
// --------------------------------------------------------------------------
//hurdler: quick fix for those who wants to play with larger wad
#include "../m_argv.h"

#define ZPLANALLOC
#ifndef ZPLANALLOC
//#define POLYPOOLSIZE   1024000    // may be much over what is needed
//                                  //TODO: check out how much is used
static int POLYPOOLSIZE=1024000;

static byte*    gr_polypool=NULL;
static byte*    gr_ppcurrent;
static int      gr_ppfree;
#endif

// only between levels, clear poly pool
static void HWR_ClearPolys (void)
{
#ifndef ZPLANALLOC
    gr_ppcurrent = gr_polypool;
    gr_ppfree = POLYPOOLSIZE;
#endif
}


// allocate  pool for fast alloc of polys
void HWR_InitPolyPool (void)
{
#ifndef ZPLANALLOC
    int pnum;

    //hurdler: quick fix for those who wants to play with larger wad
    if ( (pnum=M_CheckParm("-polypoolsize")) )
        POLYPOOLSIZE = atoi(myargv[pnum+1])*1024; // (in kb)

    GenPrintf( EMSG_info, "HWR_InitPolyPool() : allocating %d bytes\n", POLYPOOLSIZE);
    gr_polypool = (byte*) malloc (POLYPOOLSIZE);
    if (!gr_polypool)
        I_Error ("HWR_InitPolyPool() : couldn't malloc polypool\n");
    HWR_ClearPolys ();
#endif
}

void HWR_FreePolyPool (void)
{
#ifndef ZPLANALLOC
    if (gr_polypool)
        free (gr_polypool);
    gr_polypool = NULL;
#endif
}

static poly_t* HWR_AllocPoly (int numpts)
{
    poly_t*     p;
    int         size;

    size = sizeof(poly_t) + sizeof(polyvertex_t) * numpts;
#ifdef ZPLANALLOC
    p = Z_Malloc(size, PU_HWRPLANE, NULL);
#else
#ifdef PARANOIA
    if(!gr_polypool)
        I_Error("Used gr_polypool without init !\n");
    if(!gr_ppcurrent)
        I_Error("gr_ppcurrent == NULL !!!\n");
#endif

    if (gr_ppfree < size)
    {
        I_Error ("allocpoly() : no more memory %d bytes left, %d bytes needed\n\n%s\n", 
                  gr_ppfree, size, "You can try the param -polypoolsize 2048 (or higher if needed)");
    }

    p = (poly_t*)gr_ppcurrent;
    gr_ppcurrent += size;
    gr_ppfree -= size;
#endif
    p->numpts = numpts;    
    return p;
}

static polyvertex_t* HWR_AllocVertex (void)
{
    polyvertex_t* p;
    int  size;

    size =  sizeof(polyvertex_t);
#ifdef ZPLANALLOC
    p = Z_Malloc(size, PU_HWRPLANE, NULL);
#else
    if (gr_ppfree < size)
        I_Error ("allocvertex() : no more memory %d bytes left, %d bytes needed\n\n%s\n", 
                  gr_ppfree, size, "You can try the param -polypoolsize 2048 (or higher if needed)");

    p = (polyvertex_t*)gr_ppcurrent;
    gr_ppcurrent += size;
    gr_ppfree -= size;
#endif
    return p;
}


//TODO: polygons should be freed in reverse order for efficiency,
// for now don't free because it doenst' free in reverse order
static void HWR_FreePoly (poly_t* poly)
{
#ifdef ZPLANALLOC
    Z_Free(poly);
#else
    int  size;
    
    size = sizeof(poly_t) + sizeof(polyvertex_t) * poly->numpts;
    memset(poly,0,size);
    //mempoly -= polysize;
#endif
}


// Return interception along bsp line (partline),
// with the polygon segment

// Return the division in partline div fields.
// divfrac = how far along partline vector is crossing pt
static boolean fracdivline (fdivline_t* partline, polyvertex_t* v1, polyvertex_t* v2)
{
    double      frac;
    double      num; // numerator
    double      den; // denominator
    double      v1x,v1y,v1dx,v1dy;  // polygon side vector, v1->v2
    double      v3x,v3y,v3dx,v3dy;  // partline vector

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
        return false;  // partline and polygon side are effectively parallel

    // first check the frac along the polygon segment,
    // (do not accept hit with the extensions)
    num = (v3x - v1x)*v3dy + (v1y - v3y)*v3dx;
    frac = num / den;
    if (frac<0.0 || frac>1.0)
        return false;  // not within the polygon side

    // now get the frac along the BSP line
    // which is useful to determine what is left, what is right
    num = (v3x - v1x)*v1dy + (v1y - v3y)*v1dx;
    frac = num / den;
    partline->divfrac = frac;  // how far along partline vector

    // find the interception point along the partition line
    partline->divpt.x = v3x + v3dx*frac;
    partline->divpt.y = v3y + v3dy*frac;

    return true;
}

#if 0
//Hurdler: it's not used anymore
static boolean NearVertice (polyvertex_t* p1, polyvertex_t* p2)
{
#if 1
    float diff;
    diff = p2->x - p1->x;
    if (diff < -1.5f || diff > 1.5f)
       return false;
    diff = p2->y - p1->y;
    if (diff < -1.5f || diff > 1.5f)
       return false;
#else       
    if (p1->x != p2->x)
        return false;
    if (p1->y != p2->y)
        return false;
#endif
    // p1 and p2 are considered the same vertex
    return true;
}
#endif

// if two vertice coords have a x and/or y difference
// of less or equal than 1 FRACUNIT, they are considered the same
// point. Note: hardcoded value, 1.0f could be anything else.
static boolean SameVertice (polyvertex_t* p1, polyvertex_t* p2)
{
#if 0
    float diff;
    diff = p2->x - p1->x;
    if (diff < -1.5f || diff > 1.5f)
       return false;
    diff = p2->y - p1->y;
    if (diff < -1.5f || diff > 1.5f)
       return false;
#else
#if 1
    // cures HOM in Freedoom map09
    if (fabsf( p2->x - p1->x ) > 0.4999f)
       return false;
    if (fabsf( p2->y - p1->y ) > 0.4999f)
       return false;
#else
    if (p1->x != p2->x)
        return false;
    if (p1->y != p2->y)
        return false;
#endif
#endif
    // p1 and p2 are considered the same vertex
    return true;
}


// split a _CONVEX_ polygon in two convex polygons
// outputs:
//   frontpoly : polygon on right side of bsp line
//   backpoly  : polygon on left side
//
// Called from: WalkBSPNode
static void SplitPoly (fdivline_t* dlnp,        //splitting parametric line
                poly_t* poly,                   //the convex poly we split
                poly_t** frontpoly,             //return one poly here
                poly_t** backpoly)              //return the other here
{
    polyvertex_t *pv;

    int          ps, pe;  // poly start, end
    int          ps_online, pe_online;
    float        ps_frac = 0.0, pe_frac = 0.0;
      // which poly is on the front side of the bsp partition line
    int          nptfront, nptback;
    polyvertex_t vs = {0.0,0.0};
    polyvertex_t ve = {0.0,0.0};
    polyvertex_t lastpv = {0.0,0.0};
    int     i,j;

    ps = pe = -1;
    ps_online = pe_online = 0;

    for (i=0; i<poly->numpts; i++)
    {
        // i, j are one side of the poly
        j=i+1;
        if (j==poly->numpts) j=0;  // wrap poly

        // start & end points
        if ( fracdivline (dlnp, &poly->pts[i], &poly->pts[j]) )
        {
	    // have dividing pt
            if (ps<0) {
                // first point
                ps = i;
                vs = dlnp->divpt;
                ps_frac = dlnp->divfrac;
            }
            else
	    {
                // the partition line can traverse a junction between two segments
                // or the two points are so close, they can be considered as one
                // thus, don't accept, since split 2 must be another vertex
                if (SameVertice(&dlnp->divpt, &lastpv))
                {
                    if (pe<0) {
                        ps = i;
                        ps_online = 1;
                    }
                    else {
                        pe = i;
                        pe_online = 1;
                    }
                }else{
                    if (pe<0) {
                        pe = i;
                        ve = dlnp->divpt;
                        pe_frac = dlnp->divfrac;
                    }
                    else
		    {
                    // a frac, not same vertice as last one
                    // we already got pt2 so pt 2 is not on the line,
                    // so we probably got back to the start point
                    // which is on the line
                        if (SameVertice(&dlnp->divpt, &vs))
                            ps_online = 1;
                        break;
                    }
                }
            }

            // remember last point intercept to detect identical points
            lastpv = dlnp->divpt;
        }
    }

    // no split : the partition line is either parallel and
    // aligned with one of the poly segments, or the line is totally
    // out of the polygon and doesn't traverse it (happens if the bsp
    // is fooled by some trick where the sidedefs don't point to
    // the right sectors)
    if (ps<0)
    {
#if 0       
        GenPrintf( EMSG_debug,
		   "SplitPoly: did not split polygon (%d %d)\n" ,ps,pe);
#endif

        // this eventually happens with 'broken' BSP's that accept
        // linedefs where each side point the same sector, that is:
        // the deep water effect with the original Doom

        //TODO: make sure front poly is to front of partition line?

        *frontpoly = poly;
        *backpoly  = NULL;
        return;
    }

    if (ps>=0 && pe<0)
    {
#if 0       
        GenPrintf( EMSG_debug,
		   "SplitPoly: only one point for split line (%d %d)",ps,pe);
#endif
        *frontpoly = poly;
        *backpoly  = NULL;
        return;
    }
    if (pe<=ps)
    {
#if 0       
        GenPrintf( EMSG_debug, "SplitPoly: invalid splitting line (%d %d)",ps,pe);
#endif
        *frontpoly = poly;
        *backpoly  = NULL;
        return;
    }

    // Number of points on each side, _not_ counting those
    // that may lie just on the line.
    nptback  = pe - ps - pe_online;
    nptfront = poly->numpts - pe_online - ps_online - nptback;

    if (nptback>0)
       *backpoly = HWR_AllocPoly (2 + nptback);
    else
       *backpoly = NULL;
    if (nptfront)
       *frontpoly = HWR_AllocPoly (2 + nptfront);
    else
       *frontpoly = NULL;

    // generate FRONT poly
    if (*frontpoly)
    {
        pv = (*frontpoly)->pts;
        *pv++ = vs;
        *pv++ = ve;
        i = pe;
        do {
            if (++i == poly->numpts)
               i=0;
            *pv++ = poly->pts[i];
        } while (i!=ps && --nptfront);
    }

    // generate BACK poly
    if (*backpoly)
    {
        pv = (*backpoly)->pts;
        *pv++ = ve;
        *pv++ = vs;
        i = ps;
        do {
            if (++i == poly->numpts)
               i=0;
            *pv++ = poly->pts[i];
        } while (i!=pe && --nptback);
    }

    // make sure frontpoly is the one on the 'right' side
    // of the partition line
    if (ps_frac>pe_frac)
    {
        poly_t*     swappoly;
        swappoly = *backpoly;
        *backpoly= *frontpoly;
        *frontpoly = swappoly;
    }

    HWR_FreePoly (poly);
}



// Use each seg of the poly as a partition line, keep only the
// part of the convex poly to the front of the seg (that is,
// the part inside the sector). The part behind the seg, is
// the void space and is cut out.
//
// Called from: HWR_SubsecPoly
static poly_t* CutOutSubsecPoly (seg_t* lseg, int segcount, poly_t* poly)
{
    poly_t* temppoly;
    polyvertex_t *pv;
    vertex_t *v1, *v2;
    
    int          poly_num_pts=0,ps,pe;
    polyvertex_t vs={0.0,0.0}, ve={0.0,0.0}, p1, p2;
    float        ps_frac=0.0;
    
    int          i,j;
    fdivline_t   cutseg;     //x,y,dx,dy as start of node_t struct
    
    
    // for each seg of the subsector
    for(;segcount--;lseg++)
    {
        //x,y,dx,dy (like a divline)
        line_t *line = lseg->linedef;
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
        
        // see if it cuts the convex poly
        ps = -1;
        pe = -1;
        for (i=0; i<poly->numpts; i++)
        {
	    // i, j are one side of the poly
            j=i+1;
            if (j==poly->numpts)
                j=0;
            
            if( fracdivline (&cutseg, &poly->pts[i], &poly->pts[j]) )
            {
	        // have dividing pt
                if (ps<0) {
                    ps = i;
                    vs = cutseg.divpt;
                    ps_frac = cutseg.divfrac;
                }
                else {
                    //frac 1 on previous segment,
                    //     0 on the next,
                    //the split line goes through one of the convex poly
                    // vertices, happens quite often since the convex
                    // poly is already adjacent to the subsector segs
                    // on most borders
                    if (SameVertice(&cutseg.divpt, &vs))
                        continue;
                    
                    if (ps_frac <= cutseg.divfrac) {
                        poly_num_pts = 2 + poly->numpts - (i-ps);
                        pe = ps;
                        ps = i;
                        ve = cutseg.divpt;
                    }
                    else {
                        poly_num_pts = 2 + (i-ps);
                        pe = i;
                        ve = vs;
                        vs = cutseg.divpt;
                    }
                    //found 2nd point
                    break;
                }
            }
        }
        
        // there was a split
        if (ps>=0)
        {
            //need 2 points
            if (pe>=0)
            {
                // generate FRONT poly
                temppoly = HWR_AllocPoly (poly_num_pts);
                pv = temppoly->pts;
                *pv++ = vs;
                *pv++ = ve;
                do {
                    if (++ps == poly->numpts)  // poly wrap
                        ps=0;
                    *pv++ = poly->pts[ps];
                } while (ps!=pe);
                HWR_FreePoly(poly);
                poly = temppoly;
            }
            else
	    {
	        //hmmm... maybe we should NOT accept this, but this happens
	        // only when the cut is not needed it seems (when the cut
	        // line is aligned to one of the borders of the poly, and
	        // only some times..)
                skipcut++;
#if 0	   
	        GenPrintf( EMSG_error,
		      "CutOutPoly: only one point for split line (%d %d)",ps,pe);
#endif
	    }
        }
    }
    return poly;
}


// At this point, the poly should be convex and the exact
// layout of the subsector.  It is not always the case,
// so continue to cut off the poly into smaller parts with
// each seg of the subsector.
//
static void HWR_SubsecPoly (int ssindex, poly_t* poly)
{
    int          segcount;
    subsector_t* sub;
    seg_t*       lseg;

    sscount++;

    sub = &subsectors[ssindex];
    segcount = sub->numlines;
    lseg = &segs[sub->firstline];

    if (poly) {
        poly = CutOutSubsecPoly (lseg,segcount,poly);
        totalsubsecpolys++;
        //extra data for this subsector
        poly_subsectors[ssindex].planepoly = poly;
    }
}

// The bsp divline does not have enough precision.
// Search for the segs source of this divline.
void SearchDivline(node_t* bsp, fdivline_t *divline)
{
#if 0
    // MAR - If you don't use the same partition line that the BSP uses,
    // the front/back polys won't match the subsectors in the BSP!
#endif
    divline->x=FIXED_TO_FLOAT( bsp->x );
    divline->y=FIXED_TO_FLOAT( bsp->y );
    divline->dx=FIXED_TO_FLOAT( bsp->dx );
    divline->dy=FIXED_TO_FLOAT( bsp->dy );
}

//Hurdler: implement a loading status
static int ls_count = 0;
static int ls_percent = 0;

// poly : the convex polygon that encloses all child subsectors
// Recursive
// Called from HWR_CreatePlanePolygons at load time.
static void WalkBSPNode (int bspnum, poly_t* poly, unsigned short* leafnode, fixed_t *bbox)
{
    node_t*     bsp;

    poly_t*     backpoly;
    poly_t*     frontpoly;
    fdivline_t  fdivline;   
    polyvertex_t*   pt;
    int     i;


    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1)
        {
            // BP: i think this code is useless and wrong because
            // - bspnum==-1 happens only when numsubsectors == 0
            // - it can't happens in bsp recursive call since bspnum is a int and children is unsigned short
            // - the BSP is complet !! (there just can have subsector without segs) (i am not sure of this point)

            // do we have a valid polygon ?
            if (poly && poly->numpts > 2)
	    {
	        if( verbose )
		    GenPrintf( EMSG_ver, "Poly: Adding a new subsector !!!\n");
                if (addsubsector == numsubsectors + NEWSUBSECTORS)
                    I_Error ("WalkBSPNode : not enough addsubsectors\n");
                else if (addsubsector > 0x7fff)
                    I_Error ("WalkBSPNode : addsubsector > 0x7fff\n");
                *leafnode = (unsigned short)addsubsector | NF_SUBSECTOR;
                poly_subsectors[addsubsector].planepoly = poly;
                addsubsector++;
            }
            
            //add subsectors without segs here?
            //HWR_SubsecPoly (0, NULL);
        }
        else
        {
            HWR_SubsecPoly (bspnum&(~NF_SUBSECTOR), poly);
            //Hurdler: implement a loading status
            if (ls_count-- <= 0)
            {
                char s[16];
                int x, y;

                I_OsPolling();
                ls_count = numsubsectors/50;
                CON_Drawer();
                sprintf(s, "%d%%", (++ls_percent)<<1);
                x = BASEVIDWIDTH/2;
                y = BASEVIDHEIGHT/2;
	        V_SetupDraw( 0 | V_SCALESTART | V_SCALEPATCH | V_CENTERSCREEN );
                M_DrawTextBox(x-58, y-8, 13, 1);
                V_DrawString(x-50, y, V_WHITEMAP, "Loading...");
                V_DrawString(x+50-V_StringWidth(s), y, V_WHITEMAP, s);

                I_FinishUpdate ();
            }
        }
        M_ClearBox(bbox);
        poly=poly_subsectors[bspnum&~NF_SUBSECTOR].planepoly;
 
        for (i=0, pt=poly->pts; i<poly->numpts; i++,pt++)
             M_AddToBox (bbox, (fixed_t)(pt->x * FRACUNIT), (fixed_t)(pt->y * FRACUNIT));

        return;
    }

    bsp = &nodes[bspnum];
    SearchDivline(bsp,&fdivline);
    SplitPoly (&fdivline, poly, &frontpoly, &backpoly);
    poly = NULL;

    //debug
    if (!backpoly)
        nobackpoly++;

    // Recursively divide front space.
    if (frontpoly)
    {
        WalkBSPNode (bsp->children[0], frontpoly, &bsp->children[0],bsp->bbox[0]);

        // copy child bbox
        memcpy(bbox, bsp->bbox[0], 4*sizeof(fixed_t));
    }
    else
        I_Error ("WalkBSPNode: no front poly ?");

    // Recursively divide back space.
    if (backpoly)
    {
        // Correct back bbox to include floor/ceiling convex polygon
        WalkBSPNode (bsp->children[1], backpoly, &bsp->children[1],bsp->bbox[1]);

        // enlarge bbox with seconde child
        M_AddToBox (bbox, bsp->bbox[1][BOXLEFT  ],
                          bsp->bbox[1][BOXTOP   ]);
        M_AddToBox (bbox, bsp->bbox[1][BOXRIGHT ],
                          bsp->bbox[1][BOXBOTTOM]);
    }
}


//FIXME: use Z_MAlloc() STATIC ?
void HWR_Free_poly_subsectors (void)
{
    if (poly_subsectors)
        free(poly_subsectors);
}

#define MAXDIST   (1.5f)
// BP: can't move vertex : DON'T change polygon geometry ! (convex)
//#define MOVEVERTEX
// Is vertex va  within the seg v1, v2
boolean PointInSeg(polyvertex_t* va, polyvertex_t* v1, polyvertex_t* v2)
{
    register float ax,ay,bx,by,cx,cy,d,norm;
    register polyvertex_t* p;
    
    // check bbox of the seg first
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

int numsplitpoly;

void SearchSegInBSP(int bspnum,polyvertex_t *p,poly_t *poly)
{
    poly_t  *q;
    int     j,k;

    if (bspnum & NF_SUBSECTOR)
    {
        if( bspnum!=-1 )
        {
            bspnum&=~NF_SUBSECTOR;
            q = poly_subsectors[bspnum].planepoly;
            if( poly==q || !q)
                return;
            for(j=0;j<q->numpts;j++)
            {
                k=j+1;
                if( k==q->numpts ) k=0;
                if( !SameVertice(p,&q->pts[j]) && 
                    !SameVertice(p,&q->pts[k]) &&
                    PointInSeg(p, &q->pts[j], &q->pts[k]) )
                {
                    poly_t *newpoly=HWR_AllocPoly(q->numpts+1);
                    int n;

                    for(n=0;n<=j;n++)
                        newpoly->pts[n]=q->pts[n];
                    newpoly->pts[k]=*p;
                    for(n=k+1;n<newpoly->numpts;n++)
                        newpoly->pts[n]=q->pts[n-1];
                    numsplitpoly++;
                    poly_subsectors[bspnum].planepoly = newpoly;
                    HWR_FreePoly(q);
                    return;
                }
            }
        }
        return;
    }

    if(   (FIXED_TO_FLOAT( nodes[bspnum].bbox[0][BOXBOTTOM] )-MAXDIST <= p->y)
       && (FIXED_TO_FLOAT( nodes[bspnum].bbox[0][BOXTOP   ] )+MAXDIST >= p->y)
       && (FIXED_TO_FLOAT( nodes[bspnum].bbox[0][BOXLEFT  ] )-MAXDIST <= p->x)
       && (FIXED_TO_FLOAT( nodes[bspnum].bbox[0][BOXRIGHT ] )+MAXDIST >= p->x)
       )
        SearchSegInBSP(nodes[bspnum].children[0],p,poly);

    if(   (FIXED_TO_FLOAT( nodes[bspnum].bbox[1][BOXBOTTOM] )-MAXDIST <= p->y)
       && (FIXED_TO_FLOAT( nodes[bspnum].bbox[1][BOXTOP   ] )+MAXDIST >= p->y)
       && (FIXED_TO_FLOAT( nodes[bspnum].bbox[1][BOXLEFT  ] )-MAXDIST <= p->x)
       && (FIXED_TO_FLOAT( nodes[bspnum].bbox[1][BOXRIGHT ] )+MAXDIST >= p->x)
      )
        SearchSegInBSP(nodes[bspnum].children[1],p,poly);
}

// search for T-intersection problem
// BP : It can be much more faster doing this at the same time of the splitpoly
// but we must use a different structure : polygon pointing on segs 
// segs pointing on polygon and on vertex (too much complicated, well not 
// really but I am soo lazy), the method described is also better for segs precision
extern consvar_t cv_grsolvetjoin;

int SolveTProblem (void)
{
    poly_t  *p;
    int     i,l;

    if (cv_grsolvetjoin.value == 0)
        return 0;

    GenPrintf( EMSG_all | EMSG_now, "Solving T-joins. This may take a while. Please wait...\n");

    numsplitpoly=0;

    for(l=0;l<addsubsector;l++ )
    {
        p = poly_subsectors[l].planepoly;
        if( p )
        for(i=0;i<p->numpts;i++)
            SearchSegInBSP(numnodes-1,&p->pts[i],p);
    }
    //CONS_Printf("numsplitpoly %d\n", numsplitpoly);
    return numsplitpoly;
}

#define NEARDIST (0.75f) 
#define MYMAX    (10000000000000.0f)

// Adds polyvertex_t references to the segs.
// [WDJ] 2013/12 Removed writes of polyvertex_t* to vertex_t*, it now has its
// own ptrs.  Fewer reverse conversions are needed.
void AdjustSegs(void)
{
    int i,j,segcount;
    seg_t* lseg;
    poly_t *p;
    int v1found=0,v2found=0;
    float nearv1,nearv2;

    // for all segs in all sectors
    for(i=0;i<numsubsectors;i++)
    {
        segcount = subsectors[i].numlines;
        lseg = &segs[subsectors[i].firstline];
        p = poly_subsectors[i].planepoly;
        if(!p)
            continue;
        for(;segcount--;lseg++)
        {
	    polyvertex_t sv1, sv2;  // seg v1, v2
            float distv1,distv2,tmp;

	    sv1.x = FIXED_TO_FLOAT( lseg->v1->x );
	    sv1.y = FIXED_TO_FLOAT( lseg->v1->y );
	    sv2.x = FIXED_TO_FLOAT( lseg->v2->x );
	    sv2.y = FIXED_TO_FLOAT( lseg->v2->y );
            nearv1=nearv2=MYMAX;
	    // find nearest existing poly pts to seg v1, v2 
            for(j=0;j<p->numpts;j++)
            {
                distv1 = p->pts[j].x - sv1.x; 
                tmp    = p->pts[j].y - sv1.y;
                distv1 = distv1*distv1+tmp*tmp;
                if( distv1 <= nearv1 )
                {
                    v1found=j;
                    nearv1 = distv1;
                }
                // the same with v2
                distv2 = p->pts[j].x - sv2.x; 
                tmp    = p->pts[j].y - sv2.y;
                distv2 = distv2*distv2+tmp*tmp;
                if( distv2 <= nearv2 )
                {
                    v2found=j;
                    nearv2 = distv2;
                }
            }
	    // close enough to be considered the same ?
            if( nearv1<=NEARDIST*NEARDIST )
	    {
                // share vertice with segs
                lseg->pv1 = &(p->pts[v1found]);
	    }
            else
            {
                // BP: here we can do better, using PointInSeg and compute
                // the right point position also split a polygon side to
                // solve a T-intersection, but too much work

                polyvertex_t *p1=HWR_AllocVertex();
                p1->x=sv1.x;
                p1->y=sv1.y;
                lseg->pv1 = p1;
            }
            if( nearv2<=NEARDIST*NEARDIST )
	    {
                lseg->pv2 = &(p->pts[v2found]);
	    }
            else
            {
                polyvertex_t *p2=HWR_AllocVertex();
                p2->x=sv2.x;
                p2->y=sv2.y;
                lseg->pv2 = p2;
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
    for( i=0; i<numsegs; i++ )
    {
        lseg = &segs[i];
        if( verbose )
        {
	    if( ! ( lseg->pv1 && lseg->pv2 ) )
	    {
	        GenPrintf( EMSG_ver, "Seg %i, not in any polygon.\n", i );
	    }
        }
        if( ! lseg->pv1 )
        {
	    polyvertex_t *p1=HWR_AllocVertex();
	    p1->x=FIXED_TO_FLOAT( lseg->v1->x );
	    p1->y=FIXED_TO_FLOAT( lseg->v1->y );
	    lseg->pv1 = p1;
	}
        if( ! lseg->pv2 )
        {
	    polyvertex_t *p2=HWR_AllocVertex();
	    p2->x=FIXED_TO_FLOAT( lseg->v2->x );
	    p2->y=FIXED_TO_FLOAT( lseg->v2->y );
	    lseg->pv2 = p2;
	}
    }
}


// Call this routine after the BSP of a Doom wad file is loaded,
// and it will generate all the convex polys for the hardware renderer.
// Called from P_SetupLevel
void HWR_CreatePlanePolygons (int bspnum)
{
    poly_t*       rootp;
    polyvertex_t* rootpv;

    int     i;

    fixed_t     rootbbox[4];

    GenPrintf( EMSG_all | EMSG_now, "Creating polygons, please wait...\n");
    ls_percent = ls_count = 0; // reset the loading status

    HWR_ClearPolys ();
    
    // find min/max boundaries of map
    //CONS_Printf ("Looking for boundaries of map...\n");
    M_ClearBox(rootbbox);
    for (i=0;i<numvertexes;i++)
        M_AddToBox(rootbbox,vertexes[i].x,vertexes[i].y);

    //CONS_Printf ("Generating subsector polygons... %d subsectors\n", numsubsectors);

    HWR_Free_poly_subsectors ();
    // allocate extra data for each subsector present in map
    totsubsectors = numsubsectors + NEWSUBSECTORS;
    poly_subsectors = (poly_subsector_t*)malloc (sizeof(poly_subsector_t) * totsubsectors);
    if (!poly_subsectors)
        I_Error ("couldn't malloc poly_subsectors totsubsectors %d\n", totsubsectors);
    // set all data in to 0 or NULL !!!
    memset (poly_subsectors, 0, sizeof(poly_subsector_t) * totsubsectors);

    // allocate table for back to front drawing of subsectors
    /*gr_drawsubsectors = (short*)malloc (sizeof(*gr_drawsubsectors) * totsubsectors);
    if (!gr_drawsubsectors)
        I_Error ("couldn't malloc gr_drawsubsectors\n");*/

    // number of the first new subsector that might be added
    addsubsector = numsubsectors;

    // construct the initial convex poly that encloses the full map
    rootp  = HWR_AllocPoly (4);
    rootpv = rootp->pts;

    rootpv->x = FIXED_TO_FLOAT( rootbbox[BOXLEFT  ] );
    rootpv->y = FIXED_TO_FLOAT( rootbbox[BOXBOTTOM] );  //lr
    rootpv++;
    rootpv->x = FIXED_TO_FLOAT( rootbbox[BOXLEFT  ] );
    rootpv->y = FIXED_TO_FLOAT( rootbbox[BOXTOP   ] );  //ur
    rootpv++;
    rootpv->x = FIXED_TO_FLOAT( rootbbox[BOXRIGHT ] );
    rootpv->y = FIXED_TO_FLOAT( rootbbox[BOXTOP   ] );  //ul
    rootpv++;
    rootpv->x = FIXED_TO_FLOAT( rootbbox[BOXRIGHT ] );
    rootpv->y = FIXED_TO_FLOAT( rootbbox[BOXBOTTOM] );  //ll

    WalkBSPNode (bspnum, rootp, NULL, rootbbox);

    i=SolveTProblem ();
    //CONS_Printf("%d point div a polygon line\n",i);
    AdjustSegs();

    //debug debug..
    //if (nobackpoly)
    //    CONS_Printf ("no back polygon %d times\n",nobackpoly);
                             //"(should happen only with the deep water trick)"

    //if (skipcut)
    //    CONS_Printf ("%d cuts were skipped because of only one point\n",skipcut);


    //CONS_Printf ("done : %d total subsector convex polygons\n", totalsubsecpolys);
}