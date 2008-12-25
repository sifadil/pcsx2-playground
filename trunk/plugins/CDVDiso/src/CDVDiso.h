/*  CDVDiso
 *  Copyright (C) 2002-2004  CDVDiso Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifdef __MSCW32__
#pragma warning(disable:4018)
#endif

//#define CDVDdefs
#include "PS2Edefs.h"
#include "libiso.h"

#ifndef __LINUX__
#define EXPORT_C(type) __declspec(dllexport) type __stdcall
#else
#define EXPORT_C(type) type
#endif

EXPORT_C(u32)   PS2EgetLibType();
EXPORT_C(u32)   PS2EgetLibVersion2(u32 type);
EXPORT_C(char*) PS2EgetLibName();


EXPORT_C(s32)  CDVDinit();
EXPORT_C(s32)  CDVDopen(const char* pTitleFilename);
EXPORT_C(void) CDVDclose();
EXPORT_C(void) CDVDshutdown();
EXPORT_C(s32)  CDVDreadTrack(u32 lsn, int mode);

// return can be NULL (for async modes)
EXPORT_C(u8*)  CDVDgetBuffer();

EXPORT_C(s32)  CDVDreadSubQ(u32 lsn, cdvdSubQ* subq);//read subq from disc (only cds have subq data)
EXPORT_C(s32)  CDVDgetTN(cdvdTN *Buffer);			//disk information
EXPORT_C(s32)  CDVDgetTD(u8 Track, cdvdTD *Buffer);	//track info: min,sec,frame,type
EXPORT_C(s32)  CDVDgetTOC(void* toc);				//gets ps2 style toc from disc
EXPORT_C(s32)  CDVDgetDiskType();					//CDVD_TYPE_xxxx
EXPORT_C(s32)  CDVDgetTrayStatus();					//CDVD_TRAY_xxxx
EXPORT_C(s32)  CDVDctrlTrayOpen();					//open disc tray
EXPORT_C(s32)  CDVDctrlTrayClose();					//close disc tray

// extended funcs

EXPORT_C(void) CDVDconfigure();
EXPORT_C(void) CDVDabout();
EXPORT_C(s32)  CDVDtest();
EXPORT_C(void) CDVDnewDiskCB(void (*callback)());

#define CDVD_LOG __Log
extern FILE *cdvdLog;

void __Log(char *fmt, ...);

#define VERBOSE 1

char IsoFile[256];
#define DEV_DEF		""
char CdDev[256];
#define CDDEV_DEF	"/dev/cdrom"
int BlockDump;
isoFile *fdump;

isoFile *iso;

typedef struct {
	int slsn;
	int elsn;
#ifdef __WIN32__
	HANDLE handle;
#else
	FILE *handle;
#endif
} _cdIso;
_cdIso cdIso[8];

#define CD_FRAMESIZE_RAW	2352
#define DATA_SIZE	(CD_FRAMESIZE_RAW-12)

#define itob(i)		((i)/10*16 + (i)%10)	/* u_char to BCD */
#define btoi(b)		((b)/16*10 + (b)%16)	/* BCD to u_char */

#define MSF2SECT(m,s,f)	(((m)*60+(s)-2)*75+(f))

extern unsigned char cdbuffer[];
unsigned char *pbuffer;
int cdblocksize;
int cdblockofs;
int cdoffset;
int cdtype;
int cdblocks;

int Zmode; // 1 Z - 2 bz2
int fmode;						// 0 - file / 1 - Zfile
char *Ztable;

extern char *methods[];

void UpdateZmode();
void CfgOpenFile();
void SysMessage(char *fmt, ...);

