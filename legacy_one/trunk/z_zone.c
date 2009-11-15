// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2009 by DooM Legacy Team.
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
// $Log: z_zone.c,v $
// Revision 1.17  2002/07/29 21:52:25  hurdler
// Someone want to have a look at this bugs
//
// Revision 1.16  2001/06/30 15:06:01  bpereira
// fixed wronf next level name in intermission
//
// Revision 1.15  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.14  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.13  2000/11/06 20:52:16  bpereira
// no message
//
// Revision 1.12  2000/11/03 13:15:13  hurdler
// Some debug comments, please verify this and change what is needed!
//
// Revision 1.11  2000/11/02 17:50:10  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.10  2000/10/14 18:33:34  hurdler
// sorry, I forgot to put an #ifdef for hw memory report
//
// Revision 1.9  2000/10/14 18:32:16  hurdler
// sorry, I forgot to put an #ifdef for hw memory report
//
// Revision 1.8  2000/10/04 16:33:54  hurdler
// Implement hardware texture memory stats
//
// Revision 1.7  2000/10/02 18:25:45  bpereira
// no message
//
// Revision 1.6  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.5  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.4  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.3  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Zone Memory Allocation. Neat.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "z_zone.h"
#include "i_system.h"
#include "command.h"
#include "m_argv.h"
#include "i_video.h"
#include "doomstat.h"
#ifdef HWRENDER
#include "hardware/hw_drv.h" // for hardware memory stats
#endif

// [WDJ] 1/24/2009 Memory control defines.
// Amount of main memory to allocate, in MB
// Original values
//#define MIN_MAIN_MEM_MB		6
//#define MAX_MAIN_MEM_MB		20

// Larger values, FreeDoom ran out of space
// FreeDoom MAP10 uses 23MB, but with fragmentation requires 30MB
#define MIN_MAIN_MEM_MB		24
#define MAX_MAIN_MEM_MB		80

// use malloc so when go over malloced region do a sigsegv
// [WDJ] This works fine for systems with large memory; reference performance.
//#define USE_MALLOC

// [smite] New lightweight zone memory allocation algorithm, does not purge blocks unless it's necessary.
#define NEW_ZMALLOC


// =========================================================================
//                        ZONE MEMORY ALLOCATION
// =========================================================================
//
// There is never any space between memblocks,
//  and there will never be two contiguous free memblocks.
// The rover can be left pointing at a non-empty block.
//
// It is of no value to free a cachable block,
//  because it will get overwritten automatically if needed.
//

void Command_Memfree_f();


#define ZONEID  0x1d4a11


typedef struct
{
  int         size;      ///< total bytes malloced, including header
  memblock_t  blocklist; ///< start / end cap for linked list
  memblock_t* rover;
} memzone_t;


static memzone_t* mainzone;
static int mb_used = 0;

#ifdef USE_MALLOC
static long  mem_in_use = 0;
#endif


// used to insert a new block between p and n
// user must make sure the blocks are logically contiguous
static void Z_LinkBlock(memblock_t *block, memblock_t *p, memblock_t *n)
{
  block->prev = p;
  block->next = n;
  n->prev = p->next = block;
}


// mark a block free, try to combine it with neighboring free blocks
static void Z_CreateFreeBlock(memblock_t *block)
{
  block->tag = PU_FREE; // Unused free block
  block->id = 0;
  block->user = NULL;

  // see if previous block is free, if so, merge blocks
  memblock_t *other = block->prev;
  if (other->tag == PU_FREE)
    {
      other->size += block->size;
      other->next = block->next;
      other->next->prev = other;

      if (block == mainzone->rover)
	mainzone->rover = other;

      block = other;
    }

  // see if next block is free, if so, merge blocks
  other = block->next;
  if (other->tag == PU_FREE)
    {
      block->size += other->size;
      block->next = other->next;
      block->next->prev = block;

      if (other == mainzone->rover)
	mainzone->rover = block;
    }
}

//
// Z_ClearZone
//
// zone->size must be valid
static void Z_ClearZone(memzone_t* zone)
{
  // set the entire zone to one free block
  memblock_t *block = (memblock_t *)( (byte *)zone + sizeof(memzone_t) );

  zone->blocklist.user = (void *)zone; // FIXME why?
  zone->blocklist.tag = PU_STATIC; // protect head against purge, allocation
  zone->rover = block;

  block->size = zone->size - sizeof(memzone_t); // all the free space

  // init circular linking
  Z_LinkBlock(block, &zone->blocklist, &zone->blocklist);
  Z_CreateFreeBlock(block);
}




//
// Z_Init
//
void Z_Init()
{
#ifndef USE_MALLOC
    int size, mb_wanted;

    if( M_CheckParm("-mb") )
    {
        if( M_IsNextParm() )
            mb_wanted = atoi(M_GetNextParm());
        else
            I_Error("usage : -mb <number of mebibytes for the heap>");
    }
    else
    {
      // TODO check logic, limits
        ULONG  freemem, total;
        freemem = I_GetFreeMem(&total) >> 20;
	total >>= 20; // into MiB

        CONS_Printf("system memory %d MiB, free %d MiB\n", total, freemem);
        // we assume that system uses a lot of memory for disk cache
	// MIN and MAX are now defined above, [WDJ]
        if (freemem < MIN_MAIN_MEM_MB)
            freemem = total >> 1; // ask for half
        mb_wanted = min(max(freemem, MIN_MAIN_MEM_MB), MAX_MAIN_MEM_MB);
    }
    // take care of negative values etc.
    if (mb_wanted < MIN_MAIN_MEM_MB)
	mb_wanted = MIN_MAIN_MEM_MB;

    // [WDJ] mem limited to 2047 MiB by size being 32bit int
    CONS_Printf ("%d mebibytes requested for Z_Init.\n", mb_wanted);
    size = mb_wanted << 20;
    mainzone = (memzone_t *)malloc(size);
    if (!mainzone)
         I_Error("Could not allocate %d mebibytes.\n"
                 "Please use -mb parameter and specify a lower value.\n", mb_wanted);

    mb_used = mb_wanted;
   
    // touch memory to stop swapping
    //memset(mainzone, 0, size);
    // [WDJ] This did not stop swapping, just made pages dirty so they must be swapped out.
    // If user specifies large -mb value, then it would dirty a large unused memory area.
    memset(mainzone, 0, min(size, 16182));  // [WDJ] dirty only the first 16K
   
    mainzone->size = size;	// mainzone size includes itself

    // set the memory after the memzone header to be one free block
    Z_ClearZone(mainzone);
#endif

    COM_AddCommand ("memfree", Command_Memfree_f);
}


//
// Z_Free
//
// Marks the block as free, tries to combine it with the preceding and succeeding block if they are free as well.
// (This is why there never can be two free blocks in succession.)
// Never purges any blocks.
//
// Example: Z = block to be Z_Free'd, (S)tatic, (P)urgable, (F)ree, . = block data, 
// Z_Free takes S...SS.F...Z..P...F.S..
// into         S...SS.F......P...F.S..
//
// Z_Free takes P..F.S..P.Z...P..F.....
// into         P..F.S..P.F...P..F.....
//
#ifdef ZDEBUG
void Z_Free2(void* ptr, char *file, int line)
#else
void Z_Free(void* ptr)
#endif
{
  memblock_t *block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));

  if (block->id != ZONEID)
    I_Error("Z_Free: freed a pointer without ZONEID");

  if (block->tag == PU_FREE)
    I_Error("Z_Free: freed a block that was already marked as free");

  if (block->user)
    {
      // clear the user's pointer to this memory
      *block->user = NULL;
    }

#ifdef USE_MALLOC
  mem_in_use -= block->size;
  free(block);
  return;
#endif

#ifdef ZDEBUG
  // SoM: HARDERCORE debuging
  // Write all Z_Free's to a debug file
  if (debugfile)
    fprintf(debugfile, "ZFREE@File: %s, line: %d\n", file, line);
  //BP: hardcore debuging
  // check if there is not a user in this zone
  for (other = mainzone->blocklist.next;
       other->next != &mainzone->blocklist;
       other = other->next)
    {
      if (other != block &&  // not this block
	  other->tag != PU_FREE &&  // in use
	  other->user &&  // has a user
	  other->user >= (void **)block &&
	  other->user <= (void **)((byte *)block + block->size)) // which is within the block to be freed
	{
	  //I_Error("Z_Free: Pointer in zone\n");
	  I_Error("Z_Free: Pointer %s:%d in zone at %s:%d", other->ownerfile, other->ownerline, file, line);
	}
    }

#ifdef PARANOIA
    // TODO overkill
    // get direct a segv when using a pointer that isn't right
    memset(ptr, 0, block->size - sizeof(memblock_t));
#endif
#endif

  Z_CreateFreeBlock(block);
}



//
// Z_Malloc
// Purgable blocks must have a user.
// You can pass a NULL user if the tag is < PU_PURGELEVEL.
//
// zone->rover points to the first block after the previously Z_Malloc'ed one.
// Z_Free's may since have created a (single) free block just before the rover.
// To avoid fragmentation, we move the rover one step back in this case.
// Purgable blocks can be purged only downstream of the rover to avoid immediate purging.
//
#define MINFRAGMENT sizeof(memblock_t)
#define NONPURGABLE(tag) (tag != PU_FREE && tag < PU_PURGELEVEL)

#ifdef ZDEBUG
void*     Z_Malloc2(int size, memtag_t tag, void **user, int alignbits, char *file, int line)
#else
void* Z_MallocAlign(int size, memtag_t tag, void **user, int alignbits)
#endif
{
  if (tag == PU_FREE)
    {
      fprintf(stderr, "Z_Malloc called with PU_FREE tag, conflict with FREE BLOCK\n");
      tag = PU_DAVE; // not used for anything else
    }
  else if (tag >= PU_PURGELEVEL && !user)
    {
      I_Error("Z_Malloc: a user is required for purgable blocks");
      return NULL;
    }

  size = (size + 3) & ~3; // granularity of 4 bytes

  // account for size of block header
  int block_size = size + sizeof(memblock_t);

  // base points at the start of the free (or purgable) sequence of blocks
  memblock_t* base;


#ifdef USE_MALLOC
  {
    base = malloc(block_size);
    if (!base)
      I_Error("Z_Malloc: malloc failed on allocation of %d bytes\n", block_size);

    base->size = block_size;
    base->tag = tag;
    base->id = ZONEID;
    base->user = user;

    mem_in_use += block_size;

    void *data = (byte *)base + sizeof(memblock_t);
    if (user)
      *user = data;

    return data;
  }
#endif


#if 0
  // DISABLED ***
  // [WDJ]  1/22/2009  This should have worked, and reduced fragmentation.
  // This hangs FreeDoom MAP10 on the second or third reload.
  // A sure test is to start with -mb 35, then "MAP MAP10", "MAP MAP11",
  // "MAP MAP10", which will then hang in B_BuildNodes,
  // where it normally takes 12 seconds to load.
  // Either it returns a NULL pointer, or some other way it purges blocks
  // before the user is done with them.  I suspect that the user code is marking
  // blocks as PU_CACHE before it is done with them, which only works if
  // Z_ALLOC takes a long time to cycle back around to them.  But when memory
  // gets tight and it gets back to them quicker, then we get mysterious
  // failures.
  // This code just brings the user code error out much sooner.
  if (size < 2048)
    base = mainzone->blocklist.next; // try to find freed block first
  else
    base = mainzone->rover;
#else
  // Older method of allocate all first, then cycle back through,
  // which tends to fragment memory and does not leave any large blocks free
  base = mainzone->rover; // start at the oldest part of the memory
#endif

  // if there is a free block preceding base (there can be at most one), move base one block back
  if (base->prev->tag == PU_FREE)
    base = base->prev;

  if (base->id && base->id != ZONEID) //Hurdler: this shouldn't happen
    I_Error("Z_Malloc: base block without ZONEID.\n");


#ifdef NEW_ZMALLOC
  // [smite] New lightweight memory allocation algorithm, does not purge blocks unless it's necessary.

  //memblock_t* start = base;
  memblock_t* stop  = base->prev;
  // the algorithm will advance through the block ring. when we hit stop, we've made a full round
  
  boolean finish = false; // should we stop searching?

 set_base:
  // find a free or purgable block to use as base
  for ( ; ; base = base->next)
    {
      if (base == stop)
	I_Error("Completely out of memory (wow, extremely unlikely!)\n"); // TODO actually base == start is the strict error criterion, but this is easier

      if (!NONPURGABLE(base->tag))
	break; // found one
    }

  // starting from base, count contiguous free or purgable space
  int space = base->size;
  memblock_t* rover;
  for (rover = base->next; space < block_size; rover = rover->next)
    {
      if (rover == stop)
	finish = true; // passed starting point, if this area is not enough, give up

      //if (rover == base) // cannot make a full round, because the blocklist cap is always nonpurgable

      if (NONPURGABLE(rover->tag))
	{
	  // hit a nonpurgable block before accumulating enough space, try again after it
	  base = rover->next;
	  if (finish)
	    I_Error("Could not find %d bytes of contiguous memory to allocate.\n", block_size);
	  else
	    goto set_base;
	}

      // found a free or purgable block, add it to total
      space += rover->size;
    }

  // only way to end up here is to have found enough space
  // starting from base, free and combine blocks until we have enough space
  while (base->size < block_size)
    {
      if (base->next->tag < PU_PURGELEVEL)
	I_Error("cannot happen");

      // free the next block, combine it with base
      Z_Free((byte *)base->next + sizeof(memblock_t));
    }

  // TODO we don't give a damn about alignment in this algorithm

#else // !NEW_ZMALLOC
  // [WDJ] 1/22/2009  MODIFIED ZALLOC
  // This also has experimental code blocks, which are currently disabled.

  int   basedata;
  int   misalign;	// align mismatch
  memblock_t* newbase;
  ULONG alignmask = (1 << alignbits) - 1;
#define ALIGN(a) (((ULONG)a+alignmask) & ~alignmask)

  // [WDJ] TODO: could compact memory after first try
  // 1. Call owners of memory to reallocate, and clean up ptrs.
  // 2. Let tag give permission to move blocks and update user ptr.

  memblock_t* rover = base;
  memblock_t* stop  = base->prev;
  // the algorithm will advance through the block ring. when we hit stop, we've made a full round


    int tries = 0; // [WDJ] Try multiple passes before bombing out
    // scan through the block list,
    // looking for the first free block
    // of sufficient size,
    // throwing out any purgable blocks along the way.

    for (;;)
    {
        if (rover == stop)
        {
	    if( tries++ == 0 ) continue; // FIXME this does nothing, since on next iteration we end up here again
            // scanned all the way around the list
            //faB: debug to see if problems of memory fragmentation..
            Command_Memfree_f();
	    if( tries < 1 ) {
	       fprintf(stderr, "Z_Malloc: Retry %d on allocation of %d bytes\n",
		       tries, block_size );
	    }else{
	       I_Error("Z_Malloc: failed on allocation of %d bytes\n"
                     "Try to increase heap size using -mb parameter (actual heap size : %d MiB)\n", block_size, mb_used);
	    }
        }

        if (rover->tag != PU_FREE) // being used
        {
            if (rover->tag < PU_PURGELEVEL)
            {
                // hit a block that can't be purged,
                //  so move base past it

                //Hurdler: FIXME: this is where the crashing problem seem to come from
	        // [WDJ] 1/20/2009 Found other bugs that it probably interacted with.
                base = rover = rover->next;
	        // base is unknown
	        continue;	// [WDJ] have base test, has no effect
            }
            else
            {
//      fprintf(stderr,"ZMALLOC   free block   base=%lx prev=%lx next=%lx\n", (unsigned long)base, (unsigned long)base->prev, (unsigned long)base->next); //  [WDJ] debug
                // free the rover block (adding the size to base)

                // the rover can be the base block
//                base = base->prev;	// back away from block that can disappear
                base = rover->prev;	// back away from block that can disappear
                Z_Free ((byte *)rover+sizeof(memblock_t));
	          // freed memory can be put in prev, next, or same block
//                base = base->next;  // [WDJ] No, skips over possible free block
                rover = base->next;
	        // base is unknown
            }
        }
        else
	{
	    base = rover;	// base is known free, so use it
            rover = rover->next;
//	    if( ! rover->user ) {
//	       fprintf(stderr,"ZALLOC: Free follows free\n");	// [WDJ] debug
//	    }
	}
        // [WDJ] The above does not ensure that base is free.
	// Set base in every case above, so the same base is not repeatedly tested.
        if (base->tag != PU_FREE) {
//	   base = base->next;	// [WDJ] seems logical, but has no effect
	   continue;
	}
        // base is free, so test it against size


       
	// trial data alignment
        basedata = ALIGN((ULONG)base + sizeof(memblock_t));
	//Hurdler: huh? it crashed on my system :( (with 777.wad and 777.deh, only software mode)
	//         same problem with MR.ROCKET's wad -> it's probably not a problem with the wad !?
	//         this is because base doesn't point to something valid (and it's not NULL)
	// Check addr of end of blocks for fit
	// 	if( ((ULONG)base)+base->size >= basedata+block_size-sizeof(memblock_t) ) break;
	if( (((ULONG)base) + base->size) >= (basedata + size) ) {
	   // fits
#if 0
	   // [WDJ] Attempt at better allocation, does not have any effect
	   if( tries == 0 ) {
	      // Try harder to not fragment memory
	      extra = base->size - size;
	      if( (extra > 32) && (extra < block_size) ) continue;
	   }
#endif
	   // [WDJ] 1/20/2009 * CRITICAL ERROR FIX *
	   if( alignbits ) {  // consider alignment problems
	      // [WDJ] More complete checking to avoid misalign problems later.
	      // If misalignment is not large enough for a MINFRAGMENT, then
	      // cannot always find a place to put it, and it will cause errors.
	      // Eliminate problem by selecting a different block.
	      newbase = ((memblock_t*)basedata) - 1;
	      misalign = (byte*)newbase - (byte*)base;	// extra before
	      if( misalign <= MINFRAGMENT ) {   // have a problem
		 // require room for MINFRAGMENT to hold misalign memory.
		 // with at least 1 byte in the fragment, to avoid a strange case
		 basedata = ALIGN( (ULONG)base + sizeof(memblock_t) + MINFRAGMENT + 1 );
		 if( (((ULONG)base) + base->size) >= (basedata + size) )  break;  // OK
		 continue;	// too small for misalign, try another block
	      }
	   }
	   break;
	}
    }

//fprintf(stderr,"ZMALLOC found block large enough  base=%lx  base->size=%i  base->next=%lx\n",(unsigned long)base, base->size, (unsigned long)base->next); //  [WDJ] debug

    // aligning can leave free space in current block so make it really free
    if( alignbits )
    {
        // The new, aligned, block.
	// Sub 1 from memblock ptr is same as sub of header size.
        newbase = ((memblock_t*)basedata) - 1;
        misalign = (byte*)newbase - (byte*)base;	// extra before

//      fprintf(stderr,"ZMALLOC   align  misalign=%i\n", misalign); //  [WDJ] debug
	// [WDJ] 1/20/2009 loop ensures misalign is 0, or >= MINFRAGMENT.
        if( misalign > MINFRAGMENT )
        {
//      fprintf(stderr,"ZMALLOC   make fragment\n"); //  [WDJ] debug
	    // MINFRAGEMENT >= sizeof( memblock_t )
	    // so base header does not overlap newbase header
	    // Link in newbase after base, and change base size.
            newbase->prev = base;
            newbase->next = base->next;
            base->next->prev = newbase;
            base->next = newbase;

            newbase->size = base->size - misalign;
            base->size = misalign;
        }
        else
#if 0
// [WDJ] 1/20/2009 Unneeded now, because of better check in loop.
        {
	   // [WDJ] 1/18/2009 * CRITICAL ERROR FIX *
	    // [WDJ] This previously would add to the zone blocklist head,
	    // which is also in the blocklist, but never inits size.
	    // [WDJ] 1/20/2009: do not add to PU_STATIC blocks, such memory
	    // will not be recovered.
	    // [WDJ] If is left as lost between the blocks, then Z_CheckHeap
	    // will notice and give an error messsage.

	   // adjust size of previous block if adjacent (not cycling)
	   memblock_t * prev=base->prev;
//fprintf(stderr,"ZMALLOC   adjust    base->prev->tag=%i\n", prev->tag); //  [WDJ] debug
            if( prev < base && prev->tag != PU_STATIC )
                base->prev->size += misalign;
            base->prev->next = newbase;
            base->next->prev = newbase;
            base->size -= misalign;
	   // [WDJ] 1/18/2009 * CRITICAL ERROR FIX *
	    // [WDJ] These overlap, and memcpy is not overlap safe.
	    // It caused segfaults later. 
	    // Must use memmove, as it is overlap safe.
            memmove(newbase,base,sizeof(memblock_t));
        }
#else
       {
	  fprintf(stderr,"Z_ALLOC: misalign < MINFRAGMENT error\n" );
       }
#endif
        base = newbase;
    }
    // [WDJ] 1/22/2009  end of  MODIFIED ZALLOC

#endif // !NEW_ZMALLOC


  // found a block big enough
  base->tag = tag;
  base->id = ZONEID;
  base->user = user;

  // pointer to the data area after header
  void *data = (byte *)base + sizeof(memblock_t);

  if (user)
    *user = data;

#ifdef ZDEBUG
  base->ownerfile = file;
  base->ownerline = line;
#endif

  // only give the required amount of memory
  int extra = base->size - block_size;
  //fprintf(stderr,"    base->size=%i  size=%i align=%i extra=%i\n", base->size, block_size, alignbits, extra); //  [WDJ] debug

  // if there's space, create a new free block after the allocated block
  if (extra > MINFRAGMENT)
    {
      base->size = block_size;

      memblock_t *exblock = (memblock_t *)((byte *)base + block_size);
      exblock->size = extra;
      Z_LinkBlock(exblock, base, base->next);
      Z_CreateFreeBlock(exblock); // base must not have a PU_FREE tag so that it's not combined with exblock!
    }

  // next allocation will start looking here
  mainzone->rover = base->next;

  return data;
}



//
// Z_FreeTags
void Z_FreeTags(memtag_t lowtag, memtag_t hightag)
{
#ifdef USE_MALLOC
  return; // FIXME does not free anything
#endif

  memblock_t *block, *next;

  for (block = mainzone->blocklist.next;
       block != &mainzone->blocklist;
       block = next)
    {
      // get link before freeing
      next = block->next; // if next is free, it may be merged to the Z_Free'd block (but this is harmless since the tag and links remain intact)

      // free block?
      if (block->tag == PU_FREE)
	continue;

      if (block->tag >= lowtag && block->tag <= hightag)
	Z_Free((byte *)block + sizeof(memblock_t));
    }
}



//
// Z_DumpHeap
//
void Z_DumpHeap(memtag_t lowtag, memtag_t hightag)
{
  memblock_t* block;
  int i=0;

  CONS_Printf("zone size: %d,  location: %p\n", mainzone->size, mainzone);
  CONS_Printf("tag range: %d to %d\n", lowtag, hightag);

  for (block = mainzone->blocklist.next;
       ;
       block = block->next)
    {
      i++;

      if (block->tag >= lowtag && block->tag <= hightag)
	CONS_Printf("block:%p  size:%7d  user:%p  tag:%3d  prev:%p  next:%p\n",
		    block, block->size, block->user, block->tag, block->next, block->prev);

      if (block->next->prev != block)
	CONS_Printf("ERROR: next block doesn't have proper back link\n");

      if (block->tag == PU_FREE && block->next->tag == PU_FREE)
	CONS_Printf("ERROR: two consecutive free blocks\n");

      if (block->user && *block->user != (byte *)block +sizeof(memblock_t))
	CONS_Printf("ERROR: block doesn't have a proper user\n");

      if (block->next == &mainzone->blocklist)
	break; // last block, not physically contiguous with first

      if ((byte *)block +block->size != (byte *)block->next)
	CONS_Printf("ERROR: block size does not touch the next block\n");
    }

  CONS_Printf("Total : %d blocks\n", i);
}


//
// Z_FileDumpHeap
//
void Z_FileDumpHeap(FILE* f)
{
  memblock_t* block;
  int i=0;

  fprintf(f, "zone size: %d,  location: %p\n", mainzone->size, mainzone);

  for (block = mainzone->blocklist.next;
       ;
       block = block->next)
    {
      i++;

      fprintf(f, "block:%p  size:%7d  user:%p  tag:%3d  prev:%p  next:%p\n",
	      block, block->size, block->user, block->tag, block->next, block->prev);

      if (block->next->prev != block)
	fprintf(f, "ERROR: next block doesn't have proper back link\n");

      if (block->tag == PU_FREE && block->next->tag == PU_FREE)
	fprintf(f, "ERROR: two consecutive free blocks\n");

      if (block->user && *block->user != (byte *)block +sizeof(memblock_t))
	fprintf(f, "ERROR: block doesn't have a proper user\n");

      if (block->next == &mainzone->blocklist)
	break; // last block, not physically contiguous with first

      if ((byte *)block +block->size != (byte *)block->next)
	fprintf(f, "ERROR: block size does not touch the next block\n");
    }

  fprintf(f, "Total : %d blocks\n", i);
  fprintf(f, "===============================================================================\n\n");
}



//
// Z_CheckHeap
//
void Z_CheckHeap(int i)
{
#ifdef USE_MALLOC
  return;
#endif

  memblock_t* block;
  for (block = mainzone->blocklist.next;
       ;
       block = block->next)
    {
      if (block->next->prev != block)
	I_Error("Z_CheckHeap: next block doesn't have proper back link %d\n",i);

      if (block->tag == PU_FREE && block->next->tag == PU_FREE)
	I_Error("Z_CheckHeap: two consecutive free blocks %d\n",i);

      if (block->user && *block->user != (byte *)block +sizeof(memblock_t))
	I_Error("Z_CheckHeap: block doesn't have a proper user %d\n", i);

      if (block->next == &mainzone->blocklist)
	break; // last block, not physically contiguous with first

      if ((byte *)block +block->size != (byte *)block->next)
	I_Error("Z_CheckHeap: block size does not touch the next block %d\n",i);
    }
}




//
// Z_ChangeTag
//
void Z_ChangeTag2(void* ptr, memtag_t tag)
{
#ifdef USE_MALLOC
// can't free because the most pu_cache allocated is to use just after
//    if(tag>=PU_PURGELEVEL)
//        Z_Free(ptr);
  return;
#endif

  memblock_t* block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));

  if (block->id != ZONEID)
    I_Error("Z_ChangeTag: block without ZONEID");

  if (tag == PU_FREE)
    {
      fprintf(stderr, "Z_ChangeTag changing to PU_FREE tag, conflict with FREE BLOCK\n" );
      tag = PU_DAVE; // not used for anything else
    }
  else if (tag >= PU_PURGELEVEL && !block->user)
    I_Error("Z_ChangeTag: a user is required for purgable blocks");

  block->tag = tag;
}



//
// Z_FreeMemory
//
void Z_FreeMemory (int *realfree,int *cachemem,int *usedmem,int *largefreeblock)
{
#ifdef USE_MALLOC
  *usedmem = mem_in_use;
  return;
#endif

  memblock_t*         block;
  int freeblock = 0;

  *realfree = 0;
  *cachemem = 0;
  *usedmem  = 0;
  *largefreeblock = 0;

  for (block = mainzone->blocklist.next;
       block != &mainzone->blocklist;
       block = block->next)
    {
      if (block->tag == PU_FREE) // free block
        {
	  // free memory
	  *realfree += block->size;
	  freeblock += block->size;
	  if (freeblock > *largefreeblock)
	    *largefreeblock = freeblock;
        }
      else if (block->tag >= PU_PURGELEVEL)
	{
	  // purgable memory (cache)
	  *cachemem += block->size;
	  freeblock += block->size;
	  if (freeblock > *largefreeblock)
	    *largefreeblock = freeblock;
	}
      else
	{
	  // used block
	  *usedmem += block->size;
	  freeblock = 0;
	}
    }
}


//
// Z_TagUsage
// - return number of bytes currently allocated in the heap for the given tag
int Z_TagUsage(memtag_t tag)
{
  memblock_t* block;
  int bytes = 0;

  for (block = mainzone->blocklist.next;
       block != &mainzone->blocklist;
       block = block->next)
    {
      if (block->tag == tag)
	bytes += block->size;
    }

  return bytes;
}


void Command_Memfree_f()
{
    int   memfree,cache,used,largefreeblock;
    ULONG freebytes, totalbytes;

#ifdef USE_MALLOC
    CONS_Printf("\2Memory Heap Info - Malloc\n");
    CONS_Printf("used  memory       : %7d KiB\n", mem_in_use>>10);
#else   
    Z_CheckHeap (-1);
    Z_FreeMemory(&memfree,&cache,&used,&largefreeblock);
    CONS_Printf("\2Memory Heap Info\n");
    CONS_Printf("total heap size    : %7d KiB\n", mb_used<<10);
    CONS_Printf("used  memory       : %7d KiB\n", used>>10);
    CONS_Printf("free  memory       : %7d KiB\n", memfree>>10);
    CONS_Printf("cache memory       : %7d KiB\n", cache>>10);
    CONS_Printf("largest free block : %7d KiB\n", largefreeblock>>10);
#ifdef HWRENDER
    if( rendermode != render_soft )
    {
    CONS_Printf("patch info headers : %7d KiB\n", Z_TagUsage(PU_HWRPATCHINFO)>>10);
    CONS_Printf("HW texture cache   : %7d KiB\n", Z_TagUsage(PU_HWRCACHE)>>10);
    CONS_Printf("plane polygons     : %7d KiB\n", Z_TagUsage(PU_HWRPLANE)>>10);
    CONS_Printf("HW texture used    : %7d KiB\n", HWD.pfnGetTextureUsed()>>10);
    }
#endif
#endif	// USE_MALLOC

    CONS_Printf("\2System Memory Info\n");
    freebytes = I_GetFreeMem(&totalbytes);
    CONS_Printf("Total     physical memory: %6d KiB\n", totalbytes>>10);
    CONS_Printf("Available physical memory: %6d KiB\n", freebytes>>10);
}





char *Z_Strdup(const char *s, memtag_t tag, void **user)
{
  return strcpy(Z_Malloc(strlen(s)+1, tag, user), s);
}
