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
// $Log: d_netfil.c,v $
// Revision 1.26  2004/04/20 00:34:26  andyp
// Linux compilation fixes and string cleanups
//
// Revision 1.25  2003/05/04 04:28:33  sburke
// Use SHORT to convert network data between big- and little-endian format.
//
// Revision 1.24  2001/07/28 16:18:37  bpereira
// Revision 1.23  2001/05/21 16:23:32  crashrl
//
// Revision 1.22  2001/05/21 14:57:05  crashrl
// Readded directory crawling file search function
//
// Revision 1.21  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.20  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.19  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.18  2001/03/30 17:12:49  bpereira
// Revision 1.17  2001/02/24 13:35:19  bpereira
// Revision 1.16  2001/02/13 20:37:27  metzgermeister
// Revision 1.15  2001/02/10 12:27:13  bpereira
//
// Revision 1.14  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.13  2000/10/08 13:30:00  bpereira
// Revision 1.12  2000/10/02 18:25:44  bpereira
// Revision 1.11  2000/09/28 20:57:14  bpereira
// Revision 1.10  2000/09/10 10:39:06  metzgermeister
// Revision 1.9  2000/08/31 14:30:55  bpereira
// Revision 1.8  2000/08/11 19:10:13  metzgermeister
//
// Revision 1.7  2000/08/10 14:52:38  ydario
// OS/2 port
//
// Revision 1.6  2000/04/16 18:38:07  bpereira
//
// Revision 1.5  2000/03/07 03:32:24  hurdler
// fix linux compilation
//
// Revision 1.4  2000/03/05 17:10:56  bpereira
// Revision 1.3  2000/02/27 00:42:10  hurdler
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      Transfer a file using HSendPacket
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <fcntl.h>
#ifdef __OS2__
#include <sys/types.h>
#endif // __OS2__
#include <sys/stat.h>

#include <time.h>

#if defined( WIN32) || defined( __DJGPP__ ) 
#include <io.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <utime.h>
#endif

#ifndef __WIN32__
#include <unistd.h>
#else
#include <sys/utime.h>
#endif
#ifdef __DJGPP__
#include <dir.h>
#include <utime.h>
#endif

#include "doomincl.h"
#include "doomstat.h"
#include "d_clisrv.h"
#include "g_game.h"
#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_net.h"
#include "w_wad.h"
#include "d_netfil.h"
#include "z_zone.h"
#include "byteptr.h"
#include "p_setup.h"
#include "m_misc.h"
#include "m_menu.h"
#include "md5.h"
#include "filesrch.h"

// sender structure
typedef struct filetx_s {
    TAH_e    release_tah; // release, access method
    char     *filename;   // name of the file, or ptr to the data
    uint32_t data_size;   // size of data transfer
    char     fileid;
    int      node;        // destination
    struct filetx_s *next; // a queue
} filetx_t;

// current transfers (one for eatch node)
typedef struct {
   filetx_t  *txlist; 
   uint32_t   position;  // file and data transfer position
   FILE*      currentfile;
} transfer_t;

static transfer_t transfer[MAXNETNODES];

// read time of file : stat _stmtime
// write time of file : utime

// Client receiver structure
int cl_num_fileneed;
fileneed_t cl_fileneed[MAX_WADFILES];

char * downloaddir="DOWNLOAD";


// By server.
// Fill the serverinfo packet with wad files loaded by the game on the server.
byte * Put_Server_FileNeed(void)
{
    int   i;
    byte *p;  // macros want byte*
    char  wadfilename[MAX_WADPATH];

    p=(byte *)&netbuffer->u.serverinfo.fileneed;
    for(i=0;i<numwadfiles;i++)
    {
        WRITEU32(p,wadfiles[i]->filesize);
        strcpy(wadfilename,wadfiles[i]->filename);
        nameonly(wadfilename);
        WRITESTRING(p,wadfilename);
        WRITEMEM(p,wadfiles[i]->md5sum,16);
    }
    netbuffer->u.serverinfo.num_fileneed = i;
    return p;
}

// By Client
// Handle the received serverinfo packet and fill client fileneed table.
void CL_Got_Fileneed(int num_fileneed_parm, byte *fileneed_str)
{
    int i;

    cl_num_fileneed = num_fileneed_parm;
    byte *p = fileneed_str;
    for(i=0; i<cl_num_fileneed; i++)
    {
        fileneed_t * fnp = & cl_fileneed[i];  // client fileneed
        fnp->status = FS_NOTFOUND;
        fnp->totalsize = READU32(p);
        fnp->phandle = NULL;
        //READSTRING(p,fnp->filename); // overflow unsafe
	// [WDJ] String overflow safe
        {
	    int fn_len = strlen( (char*)p ) + 1;
            int read_len = min( fn_len, MAX_WADPATH-1 );  // length safe
	    memcpy(fnp->filename, p, read_len);
	    fnp->filename[MAX_WADPATH-1] = '\0';
            p += fn_len;  // whole
	}
        READMEM(p,fnp->md5sum,16);
    }
}

// By Client.
// First step in join game, fileneed and savegame.
void CL_Prepare_Download_SaveGame(const char *tmpsave)
{
    cl_num_fileneed = 1;
    cl_fileneed[0].status = FS_REQUESTED;
    cl_fileneed[0].totalsize = -1;
    cl_fileneed[0].phandle = NULL;
    memset(cl_fileneed[0].md5sum, 0, 16);
    strcpy(cl_fileneed[0].filename, tmpsave);
}


// By Client.
// Send to the server the names of requested files.
// Files who status is FS_NOTFOUND in the fileneed table are sent.
// Return false when there is a failure.
boolean Send_RequestFile(void)
{
    int   i;
    uint32_t  totalfreespaceneeded=0;
    fileneed_t * fnp;

    if( M_CheckParm("-nodownload") )
    {
        char s[1024]="";

        // Check for missing files.
        for(i=0; i<cl_num_fileneed; i++)
        {
	    fnp = & cl_fileneed[i];
            if( fnp->status!=FS_FOUND )
            {
                strcat(s,"  \"");
                strcat(s,fnp->filename);
                strcat(s,"\"");
                if(fnp->status==FS_NOTFOUND)
                {
                    strcat(s," not found");
                } 
                else if(fnp->status==FS_MD5SUMBAD)
                {
                    int j;
                    int strl;

                    strcat(s," has wrong md5sum, needs: ");
                    strl = strlen(s);

                    for(j=0; j<16; j++)
                    {
                        sprintf(&s[strl+2*j],"%02x", fnp->md5sum[j]);
                    }
                    s[strl+32]='\0';
                }
                else if(fnp->status==FS_OPEN)
                    strcat(s," found, ok");
                strcat(s,"\n");
            }
	}
        I_Error("To play with this server you should have these files:\n%s\n"
                "remove -nodownload if you want to download the files!\n",s);
    }

    netbuffer->packettype = PT_REQUESTFILE;
    byte *p = netbuffer->u.textcmd;
    for(i=0; i<cl_num_fileneed; i++)
    {
        fnp = & cl_fileneed[i];
        if( fnp->status==FS_NOTFOUND || fnp->status == FS_MD5SUMBAD)
        {
	    char filetmp[ MAX_WADPATH ];
            if( fnp->status==FS_NOTFOUND )
                totalfreespaceneeded += fnp->totalsize;
	    strcpy( filetmp, fnp->filename );
            nameonly(filetmp);
            WRITECHAR(p,i);  // fileid
            WRITESTRING(p,filetmp);
            // put it in download dir 
	    cat_filename( fnp->filename, downloaddir, filetmp );
            fnp->status = FS_REQUESTED;
        }
    }
    WRITECHAR(p,-1);
    uint64_t availablefreespace = I_GetDiskFreeSpace();
    // CONS_Printf("free byte %d\n",availablefreespace);
    if(totalfreespaceneeded>availablefreespace)
    {
        I_Error("To play on this server you should download %dKb\n"
                "but you have only %dKb freespace on this drive\n",
                totalfreespaceneeded,availablefreespace);
    }

    // prepare to download
    I_mkdir(downloaddir,0755);
    return HSendPacket(servernode,true,0,p-netbuffer->u.textcmd);
}

// Received request filepak. Put the files to the send queue.
void Got_RequestFilePak(int nnode)
{
    char *p = (char *)netbuffer->u.textcmd;

    while(*p!=-1)
    {
        SendFile(nnode, p+1, *p);
        p++; // skip fileid
        SKIPSTRING(p);
    }
}


// client check if the fileneeded arent already loaded or on the disk
int CL_CheckFiles(void)
{
    int  i,j;
    char wadfilename[MAX_WADPATH];
    int  ret=1;

    if( M_CheckParm("-nofiles") )
        return 1;

    // the first is the iwad (the main wad file)
    // do not check file date, also don't download it (copyright problem) 
    strcpy(wadfilename,wadfiles[0]->filename);
    nameonly(wadfilename);
    if( strcasecmp(wadfilename,cl_fileneed[0].filename)!=0 )
    {
        M_SimpleMessage(va("You cannot connect to this server\n"
                          "since it uses %s\n"
                          "You are using %s\n",
                          cl_fileneed[0].filename,wadfilename));
        return 2;
    }
    cl_fileneed[0].status=FS_OPEN;

    for (i=1;i<cl_num_fileneed;i++)
    {
        fileneed_t * fnp = &cl_fileneed[i];
        if(devparm)
	    GenPrintf(EMSG_dev, "searching for '%s' ", fnp->filename);
        
        // check in already loaded files
        for(j=1;wadfiles[j];j++)
        {
            strcpy(wadfilename,wadfiles[j]->filename);
            nameonly(wadfilename);
            if( strcasecmp(wadfilename, fnp->filename)==0 &&
                 !memcmp(wadfiles[j]->md5sum, fnp->md5sum, 16))
            {
                if(devparm)
		   GenPrintf(EMSG_dev, "already loaded\n");
                fnp->status=FS_OPEN;
                break;
            }
        }
        if( fnp->status!=FS_NOTFOUND )
           continue;

        fnp->status = findfile(fnp->filename, fnp->md5sum, true);
        if(devparm)
	    GenPrintf(EMSG_dev, "found %d\n", fnp->status);
        if( fnp->status != FS_FOUND )
            ret=0;
    }
    return ret;
}

// load it now
void CL_Load_ServerFiles(void)
{
    int i;
    
    for (i=1;i<cl_num_fileneed;i++)
    {
        fileneed_t * fnp = &cl_fileneed[i];
        if( fnp->status == FS_OPEN )
            // already loaded
            continue;
        else
        if( fnp->status == FS_FOUND )
        {
            P_AddWadFile(fnp->filename,NULL);
            fnp->status = FS_OPEN;
        }
        else
        if( fnp->status == FS_MD5SUMBAD) 
        {
            P_AddWadFile(fnp->filename,NULL);
            fnp->status = FS_OPEN;
            CONS_Printf("\2File %s found but with differant md5sum\n", fnp->filename);
        }
        else
            I_Error("Try to load file %s with status of %d\n", fnp->filename, fnp->status);
    }
}

// little optimization to test if there is a file in the queue
static int filetosend=0;

void SendFile(int node,char *filename, char fileid)
{
    filetx_t **q,*p;
    int i;
    boolean found=0;
    char  wadfilename[MAX_WADPATH];

    q=&transfer[node].txlist;
    while(*q) q=&((*q)->next);
    *q=(filetx_t *)malloc(sizeof(filetx_t));
    if(!*q)
       I_Error("SendFile : No more ram\n");
    p=*q;
    p->filename=(char *)malloc(MAX_WADPATH);
    strncpy(p->filename, filename, MAX_WADPATH-1);
    p->filename[MAX_WADPATH-1] = '\0';
    
    // a minimum of security, can only get file in legacy directory
    nameonly(p->filename);

    // check first in wads loaded the majority of case
    for(i=0;wadfiles[i] && !found;i++)
    {
        strcpy(wadfilename,wadfiles[i]->filename);
        nameonly(wadfilename);
        if(strcasecmp(wadfilename,p->filename)==0)
        {
            found = true;
            // copy filename with full path
            strncpy(p->filename, wadfiles[i]->filename, MAX_WADPATH-1);
	    p->filename[MAX_WADPATH-1] = '\0';
        }
    }
    
    if( !found )
    {
        DEBFILE(va("%s not found in wadfiles\n", filename));
        if(findfile(p->filename,NULL,true)==0)
        {
            // not found
            // don't inform client (probably hacker)
            DEBFILE(va("Client %d request %s : not found\n", node, filename));
            free(p->filename);
            free(p);
            *q=NULL;
            return;
        }
        else
            return;
    }

    DEBFILE(va("Sending file %s to %d (id=%d)\n", filename,node, fileid));
    p->release_tah=TAH_FILE;
    // size initialized at file open 
    //p->size=size;
    p->fileid=fileid;
    p->next=NULL; // end of list

    filetosend++;
}

void SendData(int node, byte *data, uint32_t size, TAH_e tah, char fileid)
{
    filetx_t **q,*p;

    q=&transfer[node].txlist;
    while(*q) q=&((*q)->next);
    *q=(filetx_t *)malloc(sizeof(filetx_t));
    if(!*q) 
       I_Error("SendData : No more data\n");
    p=*q;
    p->release_tah=tah;
    p->filename = (char *)data;
    p->data_size=size;
    p->fileid=fileid;
    p->next=NULL; // end of list

    DEBFILE(va("Sending ram %x( size:%d) to %d (id=%d)\n",
	       p->filename, size, node, fileid));

    filetosend++;
}

void EndSend(int node)
{
    filetx_t *p=transfer[node].txlist;
    switch (p->release_tah) {
    case TAH_FILE:
        if( transfer[node].currentfile )
            fclose(transfer[node].currentfile);
        free(p->filename);
        break;
    case TAH_Z_FREE:
        Z_Free(p->filename);
        break;
    case TAH_MALLOC_FREE:
        free(p->filename);
    case TAH_NOTHING:
        break;
    }
    transfer[node].txlist = p->next;
    transfer[node].currentfile = NULL;
    free(p);                 
    filetosend--;
}

#define PACKETPERTIC net_bandwidth/(TICRATE*software_MAXPACKETLENGTH)

// Called by NetUpdate, CL_ConnectToServer.
void Filetx_Ticker(void)
{
    static int currentnode=0;  // net node num

    filetx_pak *p;
    uint32_t   size;
    filetx_t   *f;
    TAH_e      access_tah;
    int        i, tcnt;
    int        packetsent = PACKETPERTIC;

    if( filetosend==0 )
        return;
    if(packetsent==0) packetsent++;
    // (((sendbytes-nowsentbyte)*TICRATE)/(I_GetTime()-starttime)<(ULONG)net_bandwidth)
    while( packetsent-- && filetosend!=0)
    {
        for(i=currentnode, tcnt=0; tcnt<MAXNETNODES; i=(i+1)%MAXNETNODES,tcnt++)
        {
	    if(transfer[i].txlist)
	         goto found;
	}
        // no transfer to do
        I_Error("filetosend=%d but no filetosend found\n", filetosend);

found:
        currentnode=(i+1)%MAXNETNODES;
        f=transfer[i].txlist;
        access_tah=f->release_tah;

        if(!transfer[i].currentfile) // file not already open
        {
	    if(access_tah == TAH_FILE) {
	        // open the file to transfer
	        long filesize;

	        transfer[i].currentfile = fopen(f->filename,"rb");

	        if(!transfer[i].currentfile)
		    I_Error("File %s does not exist", f->filename);

	        fseek(transfer[i].currentfile, 0, SEEK_END);
	        filesize = ftell(transfer[i].currentfile);

	        // nobody wants to transfer a file bigger than 4GB!
	        // and computers will never need more than 640kb of RAM ;-)
	        if(-1 == filesize)
	        {
		    perror("Error");
		    I_Error("Error getting filesize of %s\n", f->filename);
		}

	        f->data_size = filesize;
	        fseek(transfer[i].currentfile, 0, SEEK_SET);            
	    }
	    else
	        transfer[i].currentfile = (FILE *)1;  // faked open flag
	    transfer[i].position=0;
	}

        p=&netbuffer->u.filetxpak;
        size=software_MAXPACKETLENGTH-(FILETX_HEADER_SIZE+PACKET_BASE_SIZE);
        if((f->data_size - transfer[i].position) < size )
	    size = f->data_size - transfer[i].position;
        if(access_tah == TAH_FILE)
        {
	    if( fread(p->data,size,1,transfer[i].currentfile) != 1 )
	        I_Error("FiletxTicker : can't get %d byte on %s at %d",size,f->filename,transfer[i].position);
	}
        else
	    memcpy(p->data,&f->filename[transfer[i].position],size);
        p->position = transfer[i].position;
        // put flag so receiver know the totalsize
        if( transfer[i].position+size == f->data_size )
	    p->position |= 0x80000000;
        p->position = LE_SWAP32_FAST(p->position);
        p->size     = LE_SWAP16_FAST(size);
        p->fileid   = f->fileid;
        netbuffer->packettype=PT_FILEFRAGMENT;
        if (!HSendPacket(i,true,0,FILETX_HEADER_SIZE+size ) ) // reliable SEND
        { // not sent for some odd reason
	    // retry at next call
	    if(access_tah == TAH_FILE)
                fseek(transfer[i].currentfile,transfer[i].position,SEEK_SET);
	    // exit the while (can't send this one why should i send the next ?
	    break;
	}
        else
        { // success
	    transfer[i].position+=size;
	    if(transfer[i].position == f->data_size) //  finish ?
	        EndSend(i);
	}
    }
}

void Got_Filetxpak(void)
{
    static int stat_cnt = 0;  // steps spent receiving file
   
    int filenum=netbuffer->u.filetxpak.fileid;

    if(filenum>=cl_num_fileneed)
    {
        DEBFILE(va("filefragment not needed %d>%d\n", filenum, cl_num_fileneed));
        return;
    }

    if( cl_fileneed[filenum].status==FS_REQUESTED )
    {
        if(cl_fileneed[filenum].phandle) I_Error("Got_Filetxpak : file already open\n");
        cl_fileneed[filenum].phandle=fopen(cl_fileneed[filenum].filename,"wb");
        if(!cl_fileneed[filenum].phandle) I_Error("Can't create file %s : disk full ?",cl_fileneed[filenum].filename);
        CONS_Printf("\r%s...",cl_fileneed[filenum].filename);
        cl_fileneed[filenum].bytes_recv = 0; 
        cl_fileneed[filenum].status=FS_DOWNLOADING;
    }

    if( cl_fileneed[filenum].status==FS_DOWNLOADING )
    {
	// Swap file position and size on big_endian machines.
	netbuffer->u.filetxpak.position	= LE_SWAP32_FAST(netbuffer->u.filetxpak.position);
	netbuffer->u.filetxpak.size	= LE_SWAP16_FAST(netbuffer->u.filetxpak.size);

        // use a special trick to know when file is finished (not always used)
        // WARNING: filepak can arrive out of order so don't stop now !
        if( netbuffer->u.filetxpak.position & 0x80000000 ) 
        {
            netbuffer->u.filetxpak.position &= ~0x80000000;
            cl_fileneed[filenum].totalsize = netbuffer->u.filetxpak.position + netbuffer->u.filetxpak.size;
        }
        // we can receive packet in the wrong order, anyway all os support gaped file
        fseek(cl_fileneed[filenum].phandle,netbuffer->u.filetxpak.position,SEEK_SET);
        if( fwrite(netbuffer->u.filetxpak.data,netbuffer->u.filetxpak.size,1,cl_fileneed[filenum].phandle)!=1 )
            I_Error("Can't write %s : disk full ?\n",cl_fileneed[filenum].filename);
        cl_fileneed[filenum].bytes_recv+=netbuffer->u.filetxpak.size;
        if(stat_cnt==0)
        {
            Net_GetNetStat();
            CONS_Printf("\r%s %dK/%dK %.1fK/s",cl_fileneed[filenum].filename,
                                               cl_fileneed[filenum].bytes_recv>>10,
                                               cl_fileneed[filenum].totalsize>>10,
			((float)netstat_recv_bps)/1024);
        }

        // finish ?
        if(cl_fileneed[filenum].bytes_recv==cl_fileneed[filenum].totalsize) 
        {
            fclose(cl_fileneed[filenum].phandle);
            cl_fileneed[filenum].phandle=NULL;
            cl_fileneed[filenum].status=FS_FOUND;
            CONS_Printf("\rDownloading %s...(done)\n",cl_fileneed[filenum].filename);
        }
    }
    else
        I_Error("Received a file not requested\n");
    // send ack back quickly

    if(++stat_cnt==4)
    {
        Net_SendAcks(servernode);
        stat_cnt=0;
    }

}

void AbortSendFiles(int node)
{
    while(transfer[node].txlist)
        EndSend(node);
}

void CloseNetFile(void)
{
    int i;
    // is sending ?
    for( i=0;i<MAXNETNODES;i++)
        AbortSendFiles(i);

    // receiving a file ?
    for( i=0;i<MAX_WADFILES;i++ )
    {
        fileneed_t * fnp = &cl_fileneed[i];
        if( fnp->status==FS_DOWNLOADING && fnp->phandle)
        {
            fclose(fnp->phandle);
            // file is not complete, delete it
            remove(fnp->filename);
        }
    }

    // remove FILEFRAGMENT from acknledge list
    Net_AbortPacketType(PT_FILEFRAGMENT);
}

// functions cut and pasted from doomatic :)

// Remove all except filename at tail
void nameonly(char *s)
{
  int j;

  for(j=strlen(s);j>=0;j--)
  {
      if( (s[j]=='\\') || (s[j]==':') || (s[j]=='/') )
      {
	  // [WDJ] DO NOT USE memcpy, these may overlap, use memmove
          memmove(s, &(s[j+1]), strlen(&(s[j+1]))+1 );
          return;
      }
  }
}


// UNUSED for now
boolean fileexist(char *filename,time_t time)
{
   int handel;
   handel=open(filename,O_RDONLY|O_BINARY);
   if( handel!=-1 )
   {
         close(handel);
         if(time!=0)
         {
            struct stat bufstat;
            stat(filename,&bufstat);
            if( time!=bufstat.st_mtime )
                return false;
         }
         return true;
   }
   else
       return false;
}

filestatus_e checkfilemd5(char *filename, unsigned char *wantedmd5sum)
{
    FILE *fhandle;
    unsigned char md5sum[16];
    filestatus_e return_val = FS_NOTFOUND;

    if((fhandle = fopen(filename,"rb")))
    {
        if(wantedmd5sum)
        {
            md5_stream(fhandle,md5sum);
            fclose(fhandle);
            if(!memcmp(wantedmd5sum, md5sum, 16))
                return_val = FS_FOUND;
            else
                return_val = FS_MD5SUMBAD; 
        }
        else
        {
            return_val = FS_FOUND;
        }
    }

    return return_val;
}

// filename must be a buffer of MAX_WADPATH
filestatus_e findfile(char *filename, unsigned char *wantedmd5sum, boolean completepath)
{
    //FIXME: implement wadpath-search
    //just for the start... recursive 10 levels from current dir should bring back old behaviour
    
    return filesearch(filename, ".", wantedmd5sum, completepath, 10);
}
