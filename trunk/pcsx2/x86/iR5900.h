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

#ifndef __IR5900_H__
#define __IR5900_H__

#include "VU.h"
#include "iCore.h"

// these might not work anymore
#define ARITHMETICIMM_RECOMPILE
#define ARITHMETIC_RECOMPILE
#define MULTDIV_RECOMPILE
#define SHIFT_RECOMPILE
#define BRANCH_RECOMPILE
#define JUMP_RECOMPILE
#define LOADSTORE_RECOMPILE
#define MOVE_RECOMPILE
#define MMI_RECOMPILE
#define MMI0_RECOMPILE
#define MMI1_RECOMPILE
#define MMI2_RECOMPILE
#define MMI3_RECOMPILE
#define FPU_RECOMPILE
#define CP0_RECOMPILE
#define CP2_RECOMPILE

// We're working on new hopefully better cycle ratios, but they're still a WIP.
// And yes this whole thing is an ugly hack.  I'll clean it up once we have
// a better idea how exactly the cycle ratios will work best.

//#define EXPERIMENTAL_CYCLE_TIMINGS

#ifdef EXPERIMENTAL_CYCLE_TIMINGS

// Number of cycles for basic instructions (add/sub, etc).
// On the EE these normally run in less than 1 cycle thanks to
// the superscalar nature of the CPU.
static const int InstCycles_Default = 7;			// 0.75 cycles for base instructions.
static const int _ic_basemod = 4 - InstCycles_Default;	// don't change me unless you're changing the fixed point accuracy of cycle counting.

// Cycle penalties for particuarly slow instructions.
static const int InstCycles_Mult = _ic_basemod + 2*8;
static const int InstCycles_Div = _ic_basemod + 13*8;
static const int InstCycles_FPU_Sqrt = _ic_basemod + 4*8;
static const int InstCycles_MMI_Mult = _ic_basemod + 4*8;
static const int InstCycles_MMI_Div = _ic_basemod + 22*8;

static const int InstCycles_Peephole_Store = _ic_basemod + 22;
static const int InstCycles_Peephole_Load = _ic_basemod + 7;
static const int InstCycles_Store = _ic_basemod + 22;
static const int InstCycles_Load = _ic_basemod + 7;

#else

static const int InstCycles_Default = 9;
static const int _ic_basemod = 8 - InstCycles_Default;	// don't change me unless you're changing the fixed point accuracy of cycle counting.

static const int InstCycles_Mult = _ic_basemod + 1*8;
static const int InstCycles_Div = _ic_basemod + 13*8;
static const int InstCycles_FPU_Sqrt = _ic_basemod + 3*8;
static const int InstCycles_MMI_Mult = _ic_basemod + 2*8;
static const int InstCycles_MMI_Div = _ic_basemod + 22*8;

static const int InstCycles_Peephole_Store = _ic_basemod + 10; //_ic_basemod + 12 for snes emu
static const int InstCycles_Peephole_Load = _ic_basemod + 2;   //_ic_basemod + 4 for snes emu
static const int InstCycles_Store = _ic_basemod + 10;		   //_ic_basemod + 12 for snes emu
static const int InstCycles_Load = _ic_basemod + 2;			   //_ic_basemod + 4 for snes emu

#endif

#define EE_CONST_PROP // rec2 - enables constant propagation (faster)
//#define EE_FPU_REGCACHING 1 // Not used anymore, its always on!

#define PC_GETBLOCK(x) PC_GETBLOCK_(x, recLUT)

void recClearMem(BASEBLOCK* p);
#define REC_CLEARM(mem) { \
	if ((mem) < maxrecmem && recLUT[(mem) >> 16]) { \
		BASEBLOCK* p = PC_GETBLOCK(mem); \
		if( *(u32*)p ) recClearMem(p); \
	} \
} \

extern u32 pc;
extern int branch;
extern uptr* recLUT;

extern u32 pc;			         // recompiler pc
extern int branch;		         // set for branch
extern u32 target;		         // branch target
extern u16 x86FpuState;
extern u16 iCWstate;
extern u32 s_nBlockCycles;		// cycles of current block recompiling
extern u32 g_eeCyclePenalty;

void recBranchCall( void (*func)() );

#define REC_FUNC_INLINE( f, delreg ) \
	MOV32ItoM( (uptr)&cpuRegs.code, (u32)cpuRegs.code ); \
	MOV32ItoM( (uptr)&cpuRegs.pc, (u32)pc ); \
	iFlushCall(FLUSH_EVERYTHING); \
	if( (delreg) > 0 ) _deleteEEreg(delreg, 0); \
	CALLFunc( (uptr)f ); 

#define REC_FUNC( f, delreg ) \
   void f( void ); \
   void rec##f( void ) \
   { \
	   MOV32ItoM( (uptr)&cpuRegs.code, (u32)cpuRegs.code ); \
	   MOV32ItoM( (uptr)&cpuRegs.pc, (u32)pc ); \
	   iFlushCall(FLUSH_EVERYTHING); \
	   if( (delreg) > 0 ) _deleteEEreg(delreg, 0); \
	   CALLFunc( (uptr)f ); \
   }

#define REC_SYS( f ) \
   void f( void ); \
   void rec##f( void ) \
   { \
	   MOV32ItoM( (uptr)&cpuRegs.code, (u32)cpuRegs.code ); \
	   MOV32ItoM( (uptr)&cpuRegs.pc, (u32)pc ); \
	   iFlushCall(FLUSH_EVERYTHING); \
	   CALLFunc( (uptr)f ); \
	   branch = 2; \
   }

// used when processing branches
void SaveBranchState();
void LoadBranchState();

void recompileNextInstruction(int delayslot);
void SetBranchReg( u32 reg );
void SetBranchImm( u32 imm );

void iFlushCall(int flushtype);
void SaveCW();
void LoadCW();

extern void (*recBSC[64])();
extern void (*recSPC[64])();
extern void (*recREG[32])();
extern void (*recCP0[32])();
extern void (*recCP0BC0[32])();
extern void (*recCP0C0[64])();
extern void (*recCP1[32])();
extern void (*recCP1BC1[32])();
extern void (*recCP1S[64])();
extern void (*recCP1W[64])();
extern void (*recMMIt[64])();
extern void (*recMMI0t[32])();
extern void (*recMMI1t[32])();
extern void (*recMMI2t[32])();
extern void (*recMMI3t[32])();

u32* _eeGetConstReg(int reg); // gets a memory pointer to the constant reg

void _eeFlushAllUnused();
void _eeOnWriteReg(int reg, int signext);

// totally deletes from const, xmm, and mmx entries
// if flush is 1, also flushes to memory
// if 0, only flushes if not an xmm reg (used when overwriting lower 64bits of reg)
void _deleteEEreg(int reg, int flush);

// allocates memory on the instruction size and returns the pointer
u32* recAllocStackMem(int size, int align);

//////////////////////////////////////
// Templates for code recompilation //
//////////////////////////////////////
typedef void (*R5900FNPTR)();
typedef void (*R5900FNPTR_INFO)(int info);

#define EERECOMPILE_CODE0(fn, xmminfo) \
void rec##fn(void) \
{ \
	eeRecompileCode0(rec##fn##_const, rec##fn##_consts, rec##fn##_constt, rec##fn##_, xmminfo); \
}

#define EERECOMPILE_CODE0_PENALTY(fn, xmminfo, cycles) \
void rec##fn(void) \
{ \
	eeRecompileCode0(rec##fn##_const, rec##fn##_consts, rec##fn##_constt, rec##fn##_, xmminfo); \
	g_eeCyclePenalty = (cycles); \
}

#define EERECOMPILE_CODE0_PENALTY(fn, xmminfo, cycles) \
void rec##fn(void) \
{ \
	eeRecompileCode0(rec##fn##_const, rec##fn##_consts, rec##fn##_constt, rec##fn##_, xmminfo); \
	g_eeCyclePenalty = (cycles); \
}

#define EERECOMPILE_CODEX(codename, fn) \
void rec##fn(void) \
{ \
	codename(rec##fn##_const, rec##fn##_); \
}

//
// MMX/XMM caching helpers
//

// rd = rs op rt
void eeRecompileCode0(R5900FNPTR constcode, R5900FNPTR_INFO constscode, R5900FNPTR_INFO consttcode, R5900FNPTR_INFO noconstcode, int xmminfo);
// rt = rs op imm16
void eeRecompileCode1(R5900FNPTR constcode, R5900FNPTR_INFO noconstcode);
// rd = rt op sa
void eeRecompileCode2(R5900FNPTR constcode, R5900FNPTR_INFO noconstcode);
// rt op rs  (SPECIAL)
void eeRecompileCode3(R5900FNPTR constcode, R5900FNPTR_INFO multicode);

//
// non mmx/xmm version, slower
//
// rd = rs op rt
#define EERECOMPILE_CONSTCODE0(fn) \
void rec##fn(void) \
{ \
	eeRecompileCodeConst0(rec##fn##_const, rec##fn##_consts, rec##fn##_constt, rec##fn##_); \
} \

// rt = rs op imm16
#define EERECOMPILE_CONSTCODE1(fn) \
void rec##fn(void) \
{ \
	eeRecompileCodeConst1(rec##fn##_const, rec##fn##_); \
} \

// rd = rt op sa
#define EERECOMPILE_CONSTCODE2(fn) \
void rec##fn(void) \
{ \
	eeRecompileCodeConst2(rec##fn##_const, rec##fn##_); \
} \

// rd = rt op rs
#define EERECOMPILE_CONSTCODESPECIAL(fn, mult) \
void rec##fn(void) \
{ \
	eeRecompileCodeConstSPECIAL(rec##fn##_const, rec##fn##_, mult); \
} \

// rd = rs op rt
void eeRecompileCodeConst0(R5900FNPTR constcode, R5900FNPTR_INFO constscode, R5900FNPTR_INFO consttcode, R5900FNPTR_INFO noconstcode);
// rt = rs op imm16
void eeRecompileCodeConst1(R5900FNPTR constcode, R5900FNPTR_INFO noconstcode);
// rd = rt op sa
void eeRecompileCodeConst2(R5900FNPTR constcode, R5900FNPTR_INFO noconstcode);
// rd = rt MULT rs  (SPECIAL)
void eeRecompileCodeConstSPECIAL(R5900FNPTR constcode, R5900FNPTR_INFO multicode, int MULT);

// XMM caching helpers
#define XMMINFO_READLO	0x01
#define XMMINFO_READHI	0x02
#define XMMINFO_WRITELO	0x04
#define XMMINFO_WRITEHI	0x08
#define XMMINFO_WRITED	0x10
#define XMMINFO_READD	0x20
#define XMMINFO_READS	0x40
#define XMMINFO_READT	0x80
#define XMMINFO_READD_LO	0x100 // if set and XMMINFO_READD is set, reads only low 64 bits of D
#define XMMINFO_READACC		0x200
#define XMMINFO_WRITEACC	0x400

#define CPU_SSE_XMMCACHE_START(xmminfo) \
    { \
		int info = eeRecompileCodeXMM(xmminfo); \

#define CPU_SSE2_XMMCACHE_START(xmminfo) \
    { \
		int info = eeRecompileCodeXMM(xmminfo); \

#define CPU_SSE_XMMCACHE_END \
		_clearNeededXMMregs(); \
		return; \
	}  \

#define FPURECOMPILE_CONSTCODE(fn, xmminfo) \
void rec##fn(void) \
{ \
	eeFPURecompileCode(rec##fn##_xmm, fn, xmminfo); \
}

#define FPURECOMPILE_CONSTCODE_PENALTY(fn, xmminfo, cycles) \
void rec##fn(void) \
{ \
	eeFPURecompileCode(rec##fn##_xmm, fn, xmminfo); \
	g_eeCyclePenalty = (cycles); \
}

// rd = rs op rt (all regs need to be in xmm)
int eeRecompileCodeXMM(int xmminfo);
void eeFPURecompileCode(R5900FNPTR_INFO xmmcode, R5900FNPTR fpucode, int xmminfo);


// For propagation of BSC stuff.
// Code implementations in ir5900tables.c
class BSCPropagate
{
protected:
	EEINST& prev;
	EEINST& pinst;

public:
	BSCPropagate( EEINST& previous, EEINST& pinstance );

	void rprop();

protected:
	void rpropSPECIAL();
	void rpropREGIMM();
	void rpropCP0();
	void rpropCP1();
	void rpropCP2();
	void rpropMMI();
	void rpropMMI0();
	void rpropMMI1();
	void rpropMMI2();
	void rpropMMI3();

	void rpropSetRead( int reg, int mask );
	void rpropSetFPURead( int reg, int mask );
	void rpropSetWrite( int reg, int mask );
	void rpropSetFPUWrite( int reg, int mask );

	template< int mask >
	void rpropSetRead( int reg );

	template< int live >
	void rpropSetWrite0( int reg, int mask );

	void rpropSetFast( int write1, int read1, int read2, int mask );

	template< int low, int hi >
	void rpropSetLOHI( int write1, int read1, int read2, int mask );

	template< int live >
	void rpropSetFPUWrite0( int reg, int mask );

};

#endif // __IR5900_H__
