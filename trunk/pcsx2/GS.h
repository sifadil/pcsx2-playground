/*  Pcsx2 - Pc Ps2 Emulator
 *  Copyright (C) 2002-2008  Pcsx2 Team
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __GS_H__
#define __GS_H__

// GCC needs these includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "Threading.h"
#include "zlib.h"

#define GSPATH3FIX

#ifdef PCSX2_VIRTUAL_MEM
#define GSCSRr *((u64*)(PS2MEM_GS+0x1000))
#define GSIMR *((u32*)(PS2MEM_GS+0x1010))
#define GSSIGLBLID ((GSRegSIGBLID*)(PS2MEM_GS+0x1080))
#else
PCSX2_ALIGNED16( extern u8 g_RealGSMem[0x2000] );
#define GSCSRr *((u64*)(g_RealGSMem+0x1000))
#define GSIMR *((u32*)(g_RealGSMem+0x1010))
#define GSSIGLBLID ((GSRegSIGBLID*)(g_RealGSMem+0x1080))
#endif

/////////////////////////////////////////////////////////////////////////////
// MTGS GIFtag Parser - Declaration
//
// The MTGS needs a dummy "GS plugin" for processing SIGNAL, FINISH, and LABEL
// commands.  These commands trigger gsIRQs, which need to be handled accurately
// in synch with the EE (which can be running several frames ahead of the MTGS)
//
// Yeah, it's a lot of work, but the performance gains are huge, even on HT cpus.

struct GSRegSIGBLID
{
	u32 SIGID;
	u32 LBLID;
};

enum GIF_FLG
{
	GIF_FLG_PACKED	= 0,
	GIF_FLG_REGLIST	= 1,
	GIF_FLG_IMAGE	= 2,
	GIF_FLG_IMAGE2	= 3
};

enum GIF_REG
{
	GIF_REG_PRIM	= 0x00,
	GIF_REG_RGBA	= 0x01,
	GIF_REG_STQ		= 0x02,
	GIF_REG_UV		= 0x03,
	GIF_REG_XYZF2	= 0x04,
	GIF_REG_XYZ2	= 0x05,
	GIF_REG_TEX0_1	= 0x06,
	GIF_REG_TEX0_2	= 0x07,
	GIF_REG_CLAMP_1	= 0x08,
	GIF_REG_CLAMP_2	= 0x09,
	GIF_REG_FOG		= 0x0a,
	GIF_REG_XYZF3	= 0x0c,
	GIF_REG_XYZ3	= 0x0d,
	GIF_REG_A_D		= 0x0e,
	GIF_REG_NOP		= 0x0f,
};

struct GIFTAG
{
	u32 nloop : 15;
	u32 eop : 1;
	u32 dummy0 : 16;
	u32 dummy1 : 14;
	u32 pre : 1;
	u32 prim : 11;
	u32 flg : 2;
	u32 nreg : 4;
	u32 regs[2];
};

struct GIFPath
{
	GIFTAG tag; 
	u32 curreg;
	u32 _pad[3];
	u8 regs[16];

	__forceinline void PrepRegs();
	void SetTag(const void* mem);
	u32 GetReg();
};


/////////////////////////////////////////////////////////////////////////////
// MTGS Threaded Class Declaration

#define MTGS_RINGBUFFERSIZE	0x00300000 // 3Mb

enum GIF_PATH
{
	GIF_PATH_1 = 0,
	GIF_PATH_2,
	GIF_PATH_3,
};


enum GS_RINGTYPE
{
	GS_RINGTYPE_RESTART = 0
,	GS_RINGTYPE_P1
,	GS_RINGTYPE_P2
,	GS_RINGTYPE_P3
,	GS_RINGTYPE_VSYNC
,	GS_RINGTYPE_FRAMESKIP
,	GS_RINGTYPE_MEMWRITE8
,	GS_RINGTYPE_MEMWRITE16
,	GS_RINGTYPE_MEMWRITE32
,	GS_RINGTYPE_MEMWRITE64
,	GS_RINGTYPE_SAVE
,	GS_RINGTYPE_LOAD
,	GS_RINGTYPE_RECORD
,	GS_RINGTYPE_RESET		// issues a GSreset() command.
,	GS_RINGTYPE_SOFTRESET	// issues a soft reset for the GIF
,	GS_RINGTYPE_WRITECSR
,	GS_RINGTYPE_MODECHANGE	// for issued mode changes.
,	GS_RINGTYPE_STARTTIME	// special case for min==max fps frameskip settings
};


class mtgsThreadObject : public Threading::Thread
{
	friend class SaveState;

protected:
	// note: when g_pGSRingPos == g_pGSWritePos, the fifo is empty
	const u8* m_RingPos;		// cur pos gs is reading from
	u8* m_WritePos;				// cur pos ee thread is writing to
	const u8* const m_RingBufferEnd;	// pointer to the end of the ringbuffer (used to detect buffer wraps)

	Threading::WaitEvent m_wait_InitDone;	// used to regulate thread startup and gsInit
	Threading::MutexLock m_lock_RingRestart;

	// Used to delay the sending of events.  Performance is better if the ringbuffer
	// has more than one command in it when the thread is kicked.
	int m_CopyCommandTally;
	int m_CopyDataTally;
	volatile u32 m_RingBufferIsBusy;

	// These vars maintain instance data for sending Data Packets.
	// Only one data packet can be constructed and uploaded at a time.

	uint m_packet_size;		// size of the packet (data only, ie. not including the 16 byte command!)
	u8* m_packet_data;		// pointer to the data location in the ringbuffer.

#ifdef RINGBUF_DEBUG_STACK
	MutexLock m_lock_Stack;
#endif

	// the MTGS "dummy" GIFtag info!
	PCSX2_ALIGNED16( GIFPath m_path[3] );

	// mtgs needs its own memory space separate from the PS2.  The PS2 memory is in
	// synch with the EE while this stays in sync with the GS (ie, it lags behind)
	PCSX2_ALIGNED16( u8 m_gsMem[0x2000] );

	PCSX2_ALIGNED( 4096, u8 m_RingBuffer[MTGS_RINGBUFFERSIZE] );

public:
	mtgsThreadObject();
	virtual ~mtgsThreadObject();

	void Reset();
	void GIFSoftReset( int mask );

	// Waits for the GS to empty out the entire ring buffer contents.
	// Used primarily for plugin startup/shutdown.
	void WaitGS();

	int PrepDataPacket( GIF_PATH pathidx, const u8* srcdata, u32 size );
	int	PrepDataPacket( GIF_PATH pathidx, const u32* srcdata, u32 size );
	int	PrepDataPacket( GIF_PATH pathidx, const u64* srcdata, u32 size );
	void SendDataPacket();

	void SendSimplePacket( GS_RINGTYPE type, int data0, int data1, int data2 );
	void SendPointerPacket( GS_RINGTYPE type, u32 data0, void* data1 );

	u8* GetDataPacketPtr() const;
	void Freeze( SaveState& state );
	void SetEvent();

	uptr FnPtr_SimplePacket() const
	{
#ifndef __LINUX__
		__asm mov eax, SendSimplePacket
#else
		__asm ("mov %eax, SendSimplePacket");
#endif
		//return (uptr)&SendSimplePacket;
	}
protected:
	// Sets the gsEvent flag and releases a timeslice.
	// For use in loops that wait on the GS thread to do certain things.
	void SetEventWait();

	// Processes a GIFtag & packet, and throws out some gsIRQs as needed.
	// Used to keep interrupts in sync with the EE, while the GS itself
	// runs potentially several frames behind.
	u32 _gifTransferDummy( GIF_PATH pathidx, const u8 *pMem, u32 size );

	// Used internally by SendSimplePacket type functions
	void _PrepForSimplePacket();
	void _FinishSimplePacket();

	int Callback();
};

extern mtgsThreadObject* mtgsThread;

void mtgsWaitGS();
bool mtgsOpen();
void mtgsRingBufSimplePacket( s32 command, u32 data0, u32 data1, u32 data2 );

/////////////////////////////////////////////////////////////////////////////
// Generalized GS Functions and Stuff

extern void gsInit();
extern s32 gsOpen();
extern void gsClose();
extern void gsReset();
extern void gsSetVideoRegionType( u32 isPal );
extern void gsResetFrameSkip();
extern void gsSyncLimiterLostTime( s32 deltaTime );
extern void gsDynamicSkipEnable();
extern void gsPostVsyncEnd( bool updategs );
extern void gsFrameSkip( bool forceskip );

// Some functions shared by both the GS and MTGS
extern void _gs_ResetFrameskip();
extern void _gs_ChangeTimings( u32 framerate, u32 iTicks );


// used for resetting GIF fifo
void gsGIFReset();
void gsCSRwrite(u32 value);

void gsWrite8(u32 mem, u8 value);
void gsWrite16(u32 mem, u16 value);
void gsWrite32(u32 mem, u32 value);
void gsWrite64(u32 mem, u64 value);

void gsConstWrite8(u32 mem, int mmreg);
void gsConstWrite16(u32 mem, int mmreg);
void gsConstWrite32(u32 mem, int mmreg);
void gsConstWrite64(u32 mem, int mmreg);
void gsConstWrite128(u32 mem, int mmreg);

u8   gsRead8(u32 mem);
u16  gsRead16(u32 mem);
u32  gsRead32(u32 mem);
u64  gsRead64(u32 mem);

int gsConstRead8(u32 x86reg, u32 mem, u32 sign);
int gsConstRead16(u32 x86reg, u32 mem, u32 sign);
int gsConstRead32(u32 x86reg, u32 mem);
void  gsConstRead64(u32 mem, int mmreg);
void  gsConstRead128(u32 mem, int xmmreg);

void gsIrq();
extern void gsInterrupt();
void dmaGIF();
void GIFdma();
void mfifoGIFtransfer(int qwc);
int _GIFchain();
void  gifMFIFOInterrupt();

extern u32 g_vu1SkipCount;
extern u32 CSRw;
extern u64 m_iSlowStart;

// GS Playback
#define GSRUN_TRANS1 1
#define GSRUN_TRANS2 2
#define GSRUN_TRANS3 3
#define GSRUN_VSYNC 4

#ifdef PCSX2_DEVBUILD

extern int g_SaveGSStream;
extern int g_nLeftGSFrames;
extern gzSavingState* g_fGSSave;

#endif

void RunGSState(gzLoadingState& f);

extern void GSGIFTRANSFER1(u32 *pMem, u32 addr); 
extern void GSGIFTRANSFER2(u32 *pMem, u32 addr); 
extern void GSGIFTRANSFER3(u32 *pMem, u32 addr);
extern void GSVSYNC();

#endif
