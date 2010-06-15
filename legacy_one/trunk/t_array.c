// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 2000 James Haley
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
//
// $Log: t_array.c,v $
// Revision 1.1  2004/07/27 08:22:01  exl
// Add fs arrys files
//
//
//
//--------------------------------------------------------------------------
//
// DESCRIPTION:
//
//   Array support for FraggleScript
//
//   By James Haley, with special thanks to SoM
//
//--------------------------------------------------------------------------

#include "z_zone.h"
#include "t_array.h"
#include "t_vari.h"
//#include "p_enemy.h"

// Save list for arrays -- put ALL arrays spawned into this list,
// and they'll live until the end of the level. This is very lazy
// garbage collection, but its the only real solution given the
// state of the FS source (the word "haphazard" comes to mind...)
#ifdef SAVELIST_STRUCTHEAD
array_t sfsavelist =
{
   NULL,
   0,
   0,
   NULL,
};
#else
sfarray_t * sfsavelist = NULL;
#endif

// add an array into the save list
void T_AddArray(sfarray_t *array)
{
   sfarray_t *temp;

   // always insert at head of list
#ifdef SAVELIST_STRUCTHEAD
   temp = sfsavelist.next;
   sfsavelist.next = array;
#else
   temp = sfsavelist;
   sfsavelist = array;
#endif
   array->next = temp;
}

static void * initsave_levelclear = NULL;  // indicates when PU_LEVEL cleared
  
// call from P_SetupLevel and P_SaveGame
// Clears added array values but not base of sfsavelist
void T_InitSaveList(void)
{
   // Z_Malloc of arrays is PU_LEVEL, but does not pass a user ptr, so level
   // clear will have released all this memory without informing the owners.
   if( initsave_levelclear )	// level not cleared, as in load saved game
   {
       sfarray_t * sfap, * sfap_nxt;
       // enable to test if this is happening
//       fprintf(stderr, "T_InitSaveList: clearing array list\n" );
#ifdef SAVELIST_STRUCTHEAD
       sfap = sfsavelist.next;
#else      
       sfap = sfsavelist;
#endif
       for( ; sfap; sfap=sfap_nxt )
       {
	  if( sfap->values )   Z_Free( sfap->values );
	  sfap_nxt = sfap->next;  // get next before deallocate
	  Z_Free( sfap );
       }
   }
   else
   {
       // will trip when level is cleared
       Z_Malloc( 1, PU_LEVEL, &initsave_levelclear );
   }
#ifdef SAVELIST_STRUCTHEAD
   sfsavelist.next = NULL;
#else      
   sfsavelist = NULL;
#endif
}

#if 0
// Clears all array values including base of sfsavelist
void T_InitSaveArrays(void)
{
   // Z_Malloc of arrays does not pass a user ptr, so level clear
   // will not have destroyed these arrays.
   T_InitSaveList();	// clear sfsavelist.next
   if( sfsavelist.values )  Z_Free( sfsavelist.values );
   sfsavelist.values = NULL;
   sfsavelist.saveindex = 0;
   sfsavelist.length = 0;
}
#endif

// SF Handler functions for calling from scripts

//
// SF_NewArray
// 
//  Create a new sfarray_t and initialize it with values
//
//  Implements: array newArray(...)
//
// array functions in t_array.c
void SF_NewArray(void)
{
   int i;
   sfarray_t *newArray;

   if(!t_argc) // empty, do nothing
      return;

   // allocate a sfarray_t
   newArray = Z_Malloc(sizeof(sfarray_t), PU_LEVEL, NULL);

   // init all fields to zero
   memset(newArray, 0, sizeof(sfarray_t));
   
   // allocate t_argc number of values, set length
   newArray->values = Z_Malloc(t_argc*sizeof(svalue_t), PU_LEVEL, NULL);
   memset(newArray->values, 0, t_argc*sizeof(svalue_t));
   
   newArray->length = t_argc;

   for(i=0; i<t_argc; i++)
   {
      // strings, arrays are ignored
      if(t_argv[i].type == svt_string || t_argv[i].type == svt_array)
	 continue;

      // copy all the argument values into the local array
      memcpy(&(newArray->values[i]), &t_argv[i], sizeof(svalue_t));
   }

   T_AddArray(newArray); // add the new array to the save list
   
   t_return.type = svt_array;
   // t_return is an internal value which may not be captured in
   // an svariable_t, so we don't count it as a reference --
   // in the cases of immediate value usage, the garbage collector
   // won't have a chance to free it until it has been used
   t_return.value.a = newArray;
}



//
// SF_NewEmptyArray
// 
//  Create a new sfarray_t and initialize it with a standard value
//
void SF_NewEmptyArray(void)
{
   int i;
   sfarray_t *newArray;
   svalue_t	newval;

   if(t_argc < 2) // empty, do nothing
      return;

   // bad types
   if(t_argv[0].type != svt_int || t_argv[1].type != svt_int)
   {
	   script_error("newemptyarray: expected integer\n");
	   return;
   }

   // Type out of bounds
   if(t_argv[1].value.i < 0 || t_argv[1].value.i > 2)
   {
	   script_error("newemptyarray: invalid type\n");
	   return;
   }



   // allocate a sfarray_t
   newArray = Z_Malloc(sizeof(sfarray_t), PU_LEVEL, NULL);

   // init all fields to zero
   memset(newArray, 0, sizeof(sfarray_t));
   
   // allocate t_argc number of values, set length
   newArray->values = Z_Malloc(t_argv[0].value.i*sizeof(svalue_t), PU_LEVEL, NULL);
   memset(newArray->values, 0, t_argv[0].value.i*sizeof(svalue_t));
   
   
   newArray->length = t_argv[0].value.i;

   // initialize each value
   switch(t_argv[1].value.i)
   {
		case 0:
			newval.type = svt_int;
			newval.value.i = 0;
			break;
		case 1:
			newval.type = svt_fixed;
			newval.value.f = 0;
			break;
		case 2:
			newval.type = svt_mobj;
			newval.value.mobj = NULL;
			break;
	}


   for(i=0; i<t_argv[0].value.i; i++)
   {
      
      // Copy the new element into the array
	  memcpy(&(newArray->values[i]), &newval, sizeof(svalue_t));

   }

   T_AddArray(newArray); // add the new array to the save list
   
   t_return.type = svt_array;
   // t_return is an internal value which may not be captured in
   // an svariable_t, so we don't count it as a reference --
   // in the cases of immediate value usage, the garbage collector
   // won't have a chance to free it until it has been used
   t_return.value.a = newArray;
}


//
// SF_ArrayCopyInto
//
// Copies the values from one array into the values of another.
// Arrays must be non-empty and must be of equal length.
//
// Implements: void copyInto(array source, array target)
//
void SF_ArrayCopyInto(void)
{
   unsigned int i;
   sfarray_t *source, *target;
   
   if(t_argc != 2)
   {
      script_error("insufficient arguments to function\n");
      return;
   }

   if(t_argv[0].type != svt_array || t_argv[1].type != svt_array)
   {
      script_error("copyinto must be called on arrays\n");
      return;
   }

   source = t_argv[0].value.a;
   target = t_argv[1].value.a;

   if(!source || !target)
   {
      script_error("copyinto cannot function on empty arrays\n");
      return;
   }

   if(source->length != target->length)
   {
      script_error("copyinto must be passed arrays of equal length\n");
      return;
   }

   for(i=0; i<source->length; i++)
   {
      memcpy(&(target->values[i]), &(source->values[i]), 
	     sizeof(svalue_t));
   }
}

//
// SF_ArrayElementAt
//
// Retrieves a value at a specific index
//
// Implements: 'a elementAt(array x, int i)
//
// This function is somewhat unique at it has a polymorphic
// return type :)
//
void SF_ArrayElementAt(void)
{
   unsigned int index;
   
   if(t_argc != 2)
   {
      script_error("incorrect arguments to function");
      return;
   }

   if(t_argv[0].type != svt_array || !t_argv[0].value.a)
   {
      script_error("elementat must be called on a non-empty array\n");
      return;
   }

   // get index from second arg
   index = intvalue(t_argv[1]);

   if(index < 0 || index >= t_argv[0].value.a->length)
   {
      script_error("array index out of bounds\n");
      return;
   }

   // copy full svalue_t to t_return
   memcpy(&t_return, &(t_argv[0].value.a->values[index]), 
          sizeof(svalue_t));
}

//
// SF_ArraySetElementAt
//
// Sets a specific value in an array
//
// Implements: void setElementAt(array x, 'a val, int i)
//
void SF_ArraySetElementAt(void)
{
   unsigned int index;
   
   if(t_argc != 3)
   {
      script_error("incorrect arguments to function");
      return;
   }

   if(t_argv[0].type != svt_array || !t_argv[0].value.a)
   {
      script_error("setelementat must be called on a non-empty array\n");
      return;
   }

   // get index from third arg this time...
   index = intvalue(t_argv[2]);

   if(index < 0 || index >= t_argv[0].value.a->length)
   {
      script_error("array index out of bounds\n");
      return;
   }

   // type checking on second arg: restricted types
   if(t_argv[1].type == svt_array || t_argv[1].type == svt_string)
   {
      script_error("%s cannot be an array element\n",
	 t_argv[1].type == svt_array ? "an array" : "a string");
      return;
   }

   // copy full svalue_t into array at given index
   memcpy(&(t_argv[0].value.a->values[index]), &t_argv[1],
          sizeof(svalue_t));
}

//
// SF_ArrayLength
//
// Retrieves the length of an array
//
// Implements: int length(array x)
//
void SF_ArrayLength(void)
{
   if(!t_argc)
   {
      script_error("insufficient arguments to function\n");
      return;
   }

   if(t_argv[0].type != svt_array)
   {
      script_error("length must be called on an array\n");
      return;
   }

   t_return.type = svt_int;

   if(!t_argv[0].value.a)
      t_return.value.i = 0;
   else
      t_return.value.i = t_argv[0].value.a->length;
}
