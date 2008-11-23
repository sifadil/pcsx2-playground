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

#include <string.h>
#include <math.h>
#include "PsxCommon.h"

psxCounter psxCounters[8];
s32 psxNextCounter;
u32 psxNextsCounter;
u8 psxhblankgate = 0;
u8 psxvblankgate = 0;
u8 psxcntmask = 0;

// flags when the gate is off or counter disabled. (do not count)
#define IOPCNT_STOPPED	(0x10000000ul)

// used to disable targets until after an overflow
#define IOPCNT_FUTURE_TARGET	(0x1000000000ULL)

#define IOPCNT_ENABLE_GATE  (1<<0)	// enables gate-based counters
#define IOPCNT_INT_TARGET   (1<<4)	// 0x10  triggers an interrupt on targets
#define IOPCNT_INT_OVERFLOW (1<<5)	// 0x20  triggers an interrupt on overflows
#define IOPCNT_ALT_SOURCE   (1<<8)	// 0x100 uses hblank on counters 1 and 3, and PSXCLOCK on counter 0

// Use an arbitrary value to flag HBLANK counters.
// These counters will be counted by the hblank gates coming from the EE,
// which ensures they stay 100% in sync with the EE's hblank counters.
#define PSXHBLANK 0x2001

static void psxRcntReset(int index)
{
	psxCounters[index].count = 0;
	psxCounters[index].mode&= ~0x18301C00;
	psxCounters[index].sCycleT = psxRegs.cycle;
}

__forceinline static void _rcntSet( int i )
{
	// thanks to being forceinline'd, this conditional will always be optimized
	// to a constant by VC (confirmed via disassembler):

	u64 overflowCap = (i>=3) ? 0x100000000ULL : 0x10000;
	u64 c;

	// psxNextCounter is relative to the psxRegs.cycle when rcntUpdate() was last called.
	// However, the current _rcntSet could be called at any cycle count, so we need to take
	// that into account.  Adding the difference from that cycle count to the current one
	// will do the trick!

	if( psxCounters[i].mode & IOPCNT_STOPPED || psxCounters[i].rate == PSXHBLANK) return;

	c = (u64)((overflowCap - psxCounters[i].count) * psxCounters[i].rate) - (psxRegs.cycle - psxCounters[i].sCycleT);
	c += psxRegs.cycle - psxNextsCounter;		// adjust for time passed since last rcntUpdate();
	if((u64)c < (u64)psxNextCounter) psxNextCounter = (u32)c;

	//if((psxCounters[i].mode & 0x10) == 0 || psxCounters[i].target > 0xffff) continue;
	if( psxCounters[i].target & IOPCNT_FUTURE_TARGET ) return;

	c = (u64)((psxCounters[i].target - psxCounters[i].count) * psxCounters[i].rate) - (psxRegs.cycle - psxCounters[i].sCycleT);
	c += psxRegs.cycle - psxNextsCounter;		// adjust for time passed since last rcntUpdate();
	if((u64)c < (u64)psxNextCounter) psxNextCounter = (u32)c;
}


void psxRcntInit() {
	int i;

	memset(psxCounters, 0, sizeof(psxCounters));

	for (i=0; i<3; i++) {
		psxCounters[i].rate = 1;
		psxCounters[i].mode|= 0x0400;
		psxCounters[i].target = 0x0;
	}
	for (i=3; i<6; i++) {
		psxCounters[i].rate = 1;
		psxCounters[i].mode|= 0x0400;
		psxCounters[i].target = 0x0;
	}

	psxCounters[0].interrupt = 0x10;
	psxCounters[1].interrupt = 0x20;
	psxCounters[2].interrupt = 0x40;

	psxCounters[3].interrupt = 0x04000;
	psxCounters[4].interrupt = 0x08000;
	psxCounters[5].interrupt = 0x10000;

	if (SPU2async != NULL)
	{
		psxCounters[6].rate = ((Config.Hacks & 0x4) ? 768 : (768*8));
		psxCounters[6].CycleT = psxCounters[6].rate;
		psxCounters[6].mode = 0x8;
	}

	if (USBasync != NULL)
	{
		psxCounters[7].rate = PSXCLK/1000;
		psxCounters[7].CycleT = psxCounters[7].rate;
		psxCounters[7].mode = 0x8;
	}

	for (i=0; i<8; i++)
		psxCounters[i].sCycleT = psxRegs.cycle;

	// Tell the IOP to branch ASAP, so that timers can get
	// configured properly.
	psxNextCounter = 1;
	psxNextsCounter = psxRegs.cycle;
}

static void __fastcall _rcntTestTarget( int i )
{
	if( psxCounters[i].count < psxCounters[i].target ) return;

	PSXCNT_LOG("IOP Counter[%d] target %x >= %x (mode: %x)\n",
		i, psxCounters[i].count, psxCounters[i].target, psxCounters[i].mode);

	if (psxCounters[i].mode & IOPCNT_INT_TARGET)
	{
		// Target interrupt

		if(psxCounters[i].mode & 0x80)
			psxCounters[i].mode &= ~0x0400; // Interrupt flag
		psxCounters[i].mode |= 0x0800; // Target flag

		psxHu32(0x1070) |= psxCounters[i].interrupt;
	}
	
	if (psxCounters[i].mode & 0x08)
	{
		// Reset on target
		psxCounters[i].count -= psxCounters[i].target;
		if(!(psxCounters[i].mode & 0x40))
		{
			SysPrintf("Counter %x repeat intr not set on zero ret, ignoring target\n", i);
			psxCounters[i].target |= IOPCNT_FUTURE_TARGET;
		}
	} else psxCounters[i].target |= IOPCNT_FUTURE_TARGET;
}


static __forceinline void _rcntTestOverflow( int i )
{
	u64 maxTarget = ( i < 3 ) ? 0xffff : 0xfffffffful;
	if( psxCounters[i].count <= maxTarget ) return;

	PSXCNT_LOG("IOP Counter[%d] overflow 0x%x >= 0x%x (mode: %x)\n",
		i, psxCounters[i].count, maxTarget, psxCounters[i].mode );

	if(psxCounters[i].mode & IOPCNT_INT_OVERFLOW)
	{
		// Overflow interrupt
		psxHu32(0x1070) |= psxCounters[i].interrupt;
		psxCounters[i].mode |= 0x1000; // Overflow flag
		if(psxCounters[i].mode & 0x80)
			psxCounters[i].mode &= ~0x0400; // Interrupt flag
	}
	
	// Update count and target.
	// Count wraps around back to zero, while the target is restored (if needed).
	// (high bit of the target gets set by rcntWtarget when the target is behind
	// the counter value, and thus should not be flagged until after an overflow)
	
	psxCounters[i].count &= maxTarget;
	psxCounters[i].target &= maxTarget;
}

/*
Gate:
   TM_NO_GATE                   000
   TM_GATE_ON_Count             001
   TM_GATE_ON_ClearStart        011
   TM_GATE_ON_Clear_OFF_Start   101
   TM_GATE_ON_Start             111

   V-blank  ----+    +----------------------------+    +------
                |    |                            |    |
                |    |                            |    |
                +----+                            +----+
 TM_NO_GATE:

                0================================>============

 TM_GATE_ON_Count:

                <---->0==========================><---->0=====

 TM_GATE_ON_ClearStart:

                0====>0================================>0=====

 TM_GATE_ON_Clear_OFF_Start:

                0====><-------------------------->0====><-----

 TM_GATE_ON_Start:

                <---->0==========================>============
*/

static void _psxCheckStartGate( int i )
{
	if(!(psxCounters[i].mode & IOPCNT_ENABLE_GATE)) return; //Ignore Gate

	switch((psxCounters[i].mode & 0x6) >> 1)
	{
		case 0x0: //GATE_ON_count - stop count on gate start:

			// get the current count at the time of stoppage:
			psxCounters[i].count = ( i < 3 ) ? 
				psxRcntRcount16( i ) : psxRcntRcount32( i );
			psxCounters[i].mode |= IOPCNT_STOPPED;
		return;

		case 0x1: //GATE_ON_ClearStart - count normally with resets after every end gate
			// do nothing - All counting will be done on a need-to-count basis.
		return;

		case 0x2: //GATE_ON_Clear_OFF_Start - start counting on gate start, stop on gate end
			psxCounters[i].count = 0;
			psxCounters[i].sCycleT = psxRegs.cycle;
			psxCounters[i].mode &= ~IOPCNT_STOPPED;
		break;

		case 0x3: //GATE_ON_Start - start and count normally on gate end (no restarts or stops or clears)
			// do nothing!
		return;
	}
	_rcntSet( i );
}

static void _psxCheckEndGate(int i)
{
	if(!(psxCounters[i].mode & IOPCNT_ENABLE_GATE)) return; //Ignore Gate

	switch((psxCounters[i].mode & 0x6) >> 1)
	{
		case 0x0: //GATE_ON_count - reset and start counting
		case 0x1: //GATE_ON_ClearStart - count normally with resets after every end gate
			psxCounters[i].count = 0;
			psxCounters[i].sCycleT = psxRegs.cycle;
			psxCounters[i].mode &= ~IOPCNT_STOPPED;
		break;

		case 0x2: //GATE_ON_Clear_OFF_Start - start counting on gate start, stop on gate end
			psxCounters[i].count = ( i < 3 ) ? 
				psxRcntRcount16( i ) : psxRcntRcount32( i );
			psxCounters[i].mode |= IOPCNT_STOPPED;
		return;	// do not set the counter

		case 0x3: //GATE_ON_Start - start and count normally (no restarts or stops or clears)
			if( psxCounters[i].mode & IOPCNT_STOPPED )
			{
				psxCounters[i].count = 0;
				psxCounters[i].sCycleT = psxRegs.cycle;
				psxCounters[i].mode &= ~IOPCNT_STOPPED;
			}
		break;
	}
	_rcntSet( i );
}

void psxCheckStartGate16(int i)
{
	assert( i < 3 );

	//Check Gate events when Vsync Starts

	if(i == 0)
	{
		// Alternate/scanline counters for Gates 1 and 3.
		// We count them here so that they stay nicely synced with the EE's hsync.

		const u32 altSourceCheck = IOPCNT_ALT_SOURCE | IOPCNT_ENABLE_GATE;
		const u32 stoppedGateCheck = (IOPCNT_STOPPED | altSourceCheck );

		// count if alt source is enabled and either:
		//  * the gate is enabled and not stopped.
		//  * the gate is disabled.

		if( (psxCounters[1].mode & altSourceCheck) == IOPCNT_ALT_SOURCE ||
			(psxCounters[1].mode & stoppedGateCheck ) == altSourceCheck )
		{
			psxCounters[1].count++;
			_rcntTestTarget( 1 );
			_rcntTestOverflow( 1 );
		}

		if( (psxCounters[3].mode & altSourceCheck) == IOPCNT_ALT_SOURCE ||
			(psxCounters[3].mode & stoppedGateCheck ) == altSourceCheck )
		{
			psxCounters[3].count++;
			_rcntTestTarget( 3 );
			_rcntTestOverflow( 3 );
		}
	}

	_psxCheckStartGate( i );
}

void psxCheckEndGate16(int i)
{
	assert(i < 3);
	_psxCheckEndGate( i );
}

static void psxCheckStartGate32(int i)
{
	// 32 it gate is called for gate 3 only.  Ever.
	assert(i == 3);

	_psxCheckStartGate( i );
}

static void psxCheckEndGate32(int i)
{
	assert(i == 3);
	_psxCheckEndGate( i );
}


void psxVBlankStart()
{
	cdvdVsync();
	psxHu32(0x1070) |= 1;
	if(psxvblankgate & 1) psxCheckStartGate16(1);
	if(psxvblankgate & (1 << 3)) psxCheckStartGate32(3);
}

void psxVBlankEnd()
{
	psxHu32(0x1070) |= 0x800;
	if(psxvblankgate & 1) psxCheckEndGate16(1);
	if(psxvblankgate & (1 << 3)) psxCheckEndGate32(3);
}

void psxRcntUpdate()
{
	int i;
	//u32 change = 0;

	for (i=0; i<=5; i++)
	{
		s32 change = psxRegs.cycle - psxCounters[i].sCycleT;

		// don't count disabled, gated, or hblank counters...
		// We can't check the ALTSOURCE flag because the PSXCLOCK source *should*
		// be counted here.

		if( psxCounters[i].mode & (IOPCNT_STOPPED | IOPCNT_ENABLE_GATE) ) continue;
		if( psxCounters[i].rate == PSXHBLANK ) continue;
		if( change <= 0 ) continue;

		psxCounters[i].count += change / psxCounters[i].rate;
		if(psxCounters[i].rate != 1)
		{
			change -= (change / psxCounters[i].rate) * psxCounters[i].rate;
			psxCounters[i].sCycleT = psxRegs.cycle - change;
		}
		else
			psxCounters[i].sCycleT = psxRegs.cycle;
	}

	// Do target/overflow testing
	// Optimization Note: This approach is very sound.  Please do not try to unroll it
	// as the size of the Test functions will cause code cache clutter and slowness.
	
	for( i=0; i<6; i++ )
	{
		// don't do target/oveflow checks for hblankers.  Those
		// checks are done when the counters are updated.
		if( psxCounters[i].rate == PSXHBLANK ) continue;
		if( psxCounters[i].mode & IOPCNT_STOPPED ) continue;

		_rcntTestTarget( i );
		_rcntTestOverflow( i );
		
		// perform second target test because if we overflowed above it's possible we
		// already shot past our target if it was very near zero.

		//if( psxCounters[i].count >= psxCounters[i].target ) _rcntTestTarget( i );
	}

	psxNextCounter = 0xffffff;
	psxNextsCounter = psxRegs.cycle;

	if(SPU2async)
	{	
		const s32 difference = psxRegs.cycle - psxCounters[6].sCycleT;
		s32 c = psxCounters[6].CycleT;

		if(difference >= psxCounters[6].CycleT)
		{
			SPU2async(difference);
			psxCounters[6].sCycleT = psxRegs.cycle;
			psxCounters[6].CycleT = psxCounters[6].rate;
		}
		else c -= difference;
		psxNextCounter = c;
	}

	if(USBasync)
	{
		const s32 difference = psxRegs.cycle - psxCounters[7].sCycleT;
		s32 c = psxCounters[7].CycleT;

		if(difference >= psxCounters[7].CycleT)
		{
			USBasync(difference);
			psxCounters[7].sCycleT = psxRegs.cycle;
			psxCounters[7].CycleT = psxCounters[7].rate;
		}
		else c -= difference;
		if (c < psxNextCounter) psxNextCounter = c;
	}

	for (i=0; i<6; i++) _rcntSet( i );
}

void psxRcntWcount16(int index, u32 value)
{
	u32 change;

	assert( index < 3 );
	PSXCNT_LOG("IOP Counter[%d] > writeCount16 = %x\n", index, value);

	if(psxCounters[index].rate != PSXHBLANK)
	{
		// Re-adjust the sCycleT to match where the counter is currently
		// (remainder of the rate divided into the time passed will do the trick)

		change = psxRegs.cycle - psxCounters[index].sCycleT;
		psxCounters[index].sCycleT = psxRegs.cycle - (change % psxCounters[index].rate);
	}

	psxCounters[index].count = value & 0xffff;
	psxCounters[index].target &= 0xffff;
	_rcntSet( index );
}

void psxRcntWcount32(int index, u32 value)
{
	u32 change;

	assert( index >= 3 && index < 6 );
	PSXCNT_LOG("IOP Counter[%d] > writeCount32 = %x\n", index, value);
	
	if(psxCounters[index].rate != PSXHBLANK)
	{
		// Re-adjust the sCycleT to match where the counter is currently
		// (remainder of the rate divided into the time passed will do the trick)

		change = psxRegs.cycle - psxCounters[index].sCycleT;
		psxCounters[index].sCycleT = psxRegs.cycle - (change % psxCounters[index].rate);
	}

	psxCounters[index].count = value & 0xffffffff;
	psxCounters[index].target &= 0xffffffff;	
	_rcntSet( index );
}

void psxRcnt0Wmode(u32 value)
{
	PSXCNT_LOG("IOP Counter[0] > writeMode = %lx\n", value);

	psxCounters[0].mode = value;
	psxCounters[0].mode|= 0x0400;
	psxCounters[0].rate = 1;

	if(value & IOPCNT_ALT_SOURCE)
		psxCounters[0].rate = PSXPIXEL;
	
	if(psxCounters[0].mode & IOPCNT_ENABLE_GATE)
	{
		// gated counters are added up as per the h/vblank timers.
		PSXCNT_LOG("IOP Counter[0] > Gate Check set, value = %x\n", value);
		psxhblankgate |= 1;
	}
	else psxhblankgate &= ~1;

	psxCounters[0].count = 0;
	psxCounters[0].sCycleT = psxRegs.cycle;
	psxCounters[0].target &= 0xffff;

	_rcntSet( 0 );
}

void psxRcnt1Wmode(u32 value)
{
	PSXCNT_LOG("IOP Counter[0] > writeMode = %lx\n", value);

	psxCounters[1].mode = value;
	psxCounters[1].mode|= 0x0400;
	psxCounters[1].rate = 1;

	if(value & IOPCNT_ALT_SOURCE)
		psxCounters[1].rate = PSXHBLANK;

	if(psxCounters[1].mode & IOPCNT_ENABLE_GATE)
	{
		PSXCNT_LOG("IOP Counter[1] > Gate Check set, value = %x\n", value);
		psxvblankgate |= 1<<1;
	}
	else psxvblankgate &= ~(1<<1);

	psxCounters[1].count = 0;
	psxCounters[1].sCycleT = psxRegs.cycle;
	psxCounters[1].target &= 0xffff;
	_rcntSet( 1 );
}

void psxRcnt2Wmode(u32 value)
{
	PSXCNT_LOG("IOP Counter[0] > writeMode = %lx\n", value);

	psxCounters[2].mode = value;
	psxCounters[2].mode|= 0x0400;

	switch(value & 0x200)
	{
		case 0x200: psxCounters[2].rate = 8; break;
		case 0x000: psxCounters[2].rate = 1; break;
	}

	if((psxCounters[2].mode & 0x7) == 0x7 || (psxCounters[2].mode & 0x7) == 0x1)
	{
		//SysPrintf("Gate set on IOP C2, disabling\n");
		psxCounters[2].mode |= IOPCNT_STOPPED;
	}
	
	psxCounters[2].count = 0;
	psxCounters[2].sCycleT = psxRegs.cycle;
	psxCounters[2].target &= 0xffff;
	_rcntSet( 2 );
}

void psxRcnt3Wmode(u32 value)
{
	PSXCNT_LOG("IOP Counter[3] > writeMode = %lx\n", value);

	psxCounters[3].mode = value;
	psxCounters[3].rate = 1;
	psxCounters[3].mode|= 0x0400;

	if(value & IOPCNT_ALT_SOURCE)
		psxCounters[3].rate = PSXHBLANK;
  
	if(psxCounters[3].mode & IOPCNT_ENABLE_GATE)
	{
		PSXCNT_LOG("IOP Counter[3] > Gate Check set, value = %x\n", value);
		psxvblankgate |= 1<<3;
	}
	else psxvblankgate &= ~(1<<3);

	psxCounters[3].count = 0;
	psxCounters[3].sCycleT = psxRegs.cycle;
	psxCounters[3].target &= 0xffffffff;
	_rcntSet( 3 );
}

void psxRcnt4Wmode(u32 value)
{
	PSXCNT_LOG("IOP Counter[4] > writeMode = %lx\n", value);

	psxCounters[4].mode = value;
	psxCounters[4].mode|= 0x0400;

	switch(value & 0x6000)
	{
		case 0x0000: psxCounters[4].rate = 1;   break;
		case 0x2000: psxCounters[4].rate = 8;   break;
		case 0x4000: psxCounters[4].rate = 16;  break;
		case 0x6000: psxCounters[4].rate = 256; break;
	}
	// Need to set a rate and target
	if((psxCounters[4].mode & 0x7) == 0x7 || (psxCounters[4].mode & 0x7) == 0x1)
	{
		SysPrintf("Gate set on IOP C4, disabling\n");
		psxCounters[4].mode |= IOPCNT_STOPPED;
	}
	
	psxCounters[4].count = 0;
	psxCounters[4].sCycleT = psxRegs.cycle;
	psxCounters[4].target &= 0xffffffff;
	_rcntSet( 4 );
}

void psxRcnt5Wmode(u32 value)
{
	PSXCNT_LOG("IOP Counter[5] > writeMode = %lx\n", value);

	psxCounters[5].mode = value;
	psxCounters[5].mode|= 0x0400;

	switch(value & 0x6000)
	{
		case 0x0000: psxCounters[5].rate = 1; break;
		case 0x2000: psxCounters[5].rate = 8; break;
		case 0x4000: psxCounters[5].rate = 16; break;
		case 0x6000: psxCounters[5].rate = 256; break;
	}
	// Need to set a rate and target
	if((psxCounters[5].mode & 0x7) == 0x7 || (psxCounters[5].mode & 0x7) == 0x1)
	{
		SysPrintf("Gate set on IOP C5, disabling\n");
		psxCounters[5].mode |= IOPCNT_STOPPED;
	}
	
	psxCounters[5].count = 0;
	psxCounters[5].sCycleT = psxRegs.cycle;
	psxCounters[5].target &= 0xffffffff;
	_rcntSet( 5 );
}

void psxRcntWtarget16(int index, u32 value)
{
	assert( index < 3 );
	PSXCNT_LOG("IOP Counter[%d] > writeTarget16 = %lx\n", index, value);
	psxCounters[index].target = value & 0xffff;

	// protect the target from an early arrival.
	// if the target is behind the current count, then set the target overflow
	// flag, so that the target won't be active until after the next overflow.
	
	if(psxCounters[index].target <= psxRcntCycles(index))
		psxCounters[index].target |= IOPCNT_FUTURE_TARGET;

	_rcntSet( index );
}

void psxRcntWtarget32(int index, u32 value)
{
	assert( index >= 3 && index < 6);
	PSXCNT_LOG("IOP Counter[%d] > writeTarget32 = %lx\n", index, value);

	psxCounters[index].target = value;

	// protect the target from an early arrival.
	// if the target is behind the current count, then set the target overflow
	// flag, so that the target won't be active until after the next overflow.

	if(psxCounters[index].target <= psxRcntCycles(index))
		psxCounters[index].target |= IOPCNT_FUTURE_TARGET;

	_rcntSet( index );
}

u16 psxRcntRcount16(int index)
{
	u32 retval = (u32)psxCounters[index].count;

	assert( index < 3 );

	PSXCNT_LOG("IOP Counter[%d] > readCount16 = %lx", index, (u16)retval );

	// Don't count HBLANK timers
	// Don't count stopped gates either.

	if( !( psxCounters[index].mode & IOPCNT_STOPPED ) &&
		( psxCounters[index].rate != PSXHBLANK ) )
	{
		u32 delta = (u32)((psxRegs.cycle - psxCounters[index].sCycleT) / psxCounters[index].rate);
		retval += delta;
		PSXCNT_LOG(" (delta = %lx)", delta );
	}
	PSXCNT_LOG( "\n" );

	return (u16)retval;
}

u32 psxRcntRcount32(int index)
{
	u32 retval = (u32)psxCounters[index].count;
	
	assert( index >= 3 && index < 6 );

	PSXCNT_LOG("IOP Counter[%d] > readCount32 = %lx", index, retval );

	if( !( psxCounters[index].mode & IOPCNT_STOPPED ) &&
		( psxCounters[index].rate != PSXHBLANK ) )
	{
		u32 delta = (u32)((psxRegs.cycle - psxCounters[index].sCycleT) / psxCounters[index].rate);
		retval += delta;
		PSXCNT_LOG(" (delta = %lx)", delta );
	}
	PSXCNT_LOG( "\n" );

	return retval;
}

u64 psxRcntCycles(int index)
{
	if(psxCounters[index].mode & IOPCNT_STOPPED || psxCounters[index].rate == PSXHBLANK ) return psxCounters[index].count;
	return (u64)(psxCounters[index].count + (u32)((psxRegs.cycle - psxCounters[index].sCycleT) / psxCounters[index].rate));
}

extern u32 dwCurSaveStateVer;
int psxRcntFreeze(gzFile f, int Mode)
{
    if( Mode == 0 && (dwCurSaveStateVer < 0x7a300010) )
	{
		// --- Reading Mode, Old Version ---
		// struct used to be 32bit count and target
		int i;
		u32 val;
		for(i = 0; i < ARRAYSIZE(psxCounters); ++i)
		{
			gzfreeze(&val,4); psxCounters[i].count = val;
			gzfreeze(&val,4); psxCounters[i].mode = val;
			gzfreeze(&val,4); psxCounters[i].target = val;
			gzfreeze((u8*)&psxCounters[i].rate, sizeof(psxCounters[i])-20);
		}
	}
    else
	{
	    gzfreezel(psxCounters);
	}

	if( Mode == 0 )
	{
		// This is needed to make old save states compatible.

		psxCounters[6].rate = ((Config.Hacks & 0x4) ? 768 : (768*8));
		psxCounters[6].CycleT = psxCounters[6].rate;
		psxCounters[7].rate = PSXCLK/1000;
		psxCounters[7].CycleT = psxCounters[7].rate;	
		
		// PSXHBLANK is now an arbitrary value, since it can differ based 
		// on PAL/NTSC and is timed by the EE.

		if(psxCounters[1].mode & IOPCNT_ALT_SOURCE)
			psxCounters[1].rate = PSXHBLANK;

		if(psxCounters[3].mode & IOPCNT_ALT_SOURCE)
			psxCounters[3].rate = PSXHBLANK;

	}

	return 0;
}
