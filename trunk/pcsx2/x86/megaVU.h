/*  Pcsx2-Playground - Pc Ps2 Emulator
*  Copyright (C) 2009  Pcsx2-Playground Team
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

#pragma once
#include "Common.h"
#include "VU.h"
#include "megaVU_Tables.h"
//#include <vector>

struct megaBlock {
	u32 pipelineState; // FMACx|y|z|w | FDiv | EFU | IALU | BRANCH // Still thinking of how I'm going to do this
	u32 x86ptrStart;
	u32 x86ptrEnd;
	u32 x86ptrBranch;
	//u32 size;
};

#define mMaxBlocks 32 // Max Blocks With Different Pipeline States (For n = 1, 2, 4, 8, 16, etc...)
class megaBlockManager {
private:
	static const int MaxBlocks = mMaxBlocks - 1;
	u32 startPC;
	u32 endPC;
	int listSize; // Total Items - 1
	int callerSize; // Total Callers - 1
	megaBlock blockList[mMaxBlocks];
	megaBlock callersList[mMaxBlocks]; // Foreign Blocks that call Local Blocks

public:
	megaBlockManager()	{ init(); }
	~megaBlockManager()	{ close(); }
	void init() {
		listSize = -1;
		callerSize = -1;
		ZeroMemory(&blockList, sizeof(blockList)); // Can be Omitted?
	}
	void close() {}; // Can be Omitted?
	void add(u32 pipelineState, u32 x86ptrStart) {
		if (!search(pipelineState)) {
			listSize++;
			listSize &= MaxBlocks;
			blockList[listSize].pipelineState = pipelineState;
			blockList[listSize].x86ptrStart = x86ptrStart;
		}
	}
	megaBlock* search(u32 pipelineState) {
		for (int i = 0; i < listSize; i++) {
			if (blockList[i].pipelineState == pipelineState) return &blockList[i];
		}
	}
	void clearFast() {
		listSize = -1;
		for ( ; callerSize >= 0; callerSize--) {
			//callerList[callerSize]. // Implement Branch Link Removal Code
		}
	}
	int clear() {
		if (listSize >= 0) { clearFast(); return 1; }
		else return 0;
	}
};

template<u32 progSize>
struct microProgram {
	u8 data[progSize];
	u32 used;	// Number of times its been used
	int cached;	// Has been Cached?
	megaBlockManager* block[progSize];
};

#define mMaxProg 16 // The amount of Micro Programs Recs will 'remember' (For n = 1, 2, 4, 8, 16, etc...)
template<u32 pSize>
struct microProgManager {
	microProgram<pSize>	prog[mMaxProg];	// Store MicroPrograms in memory
	static const int	max = mMaxProg - 1; 
	int					cur;			// Index to Current MicroProgram thats running (-1 = uncached)
	int					total;			// Total Number of valid MicroPrograms minus 1
	int					cleared;		// Micro Program is Indeterminate so must be searched for (and if no matches are found then recompile a new one)
	int					finished;		// Completed MicroProgram to E-bit Termination
};

struct megaVU {
	int index;		// VU Index (VU0 or VU1)
	u32 microSize;	// VU Micro Memory Size
	u32 progSize;	// VU Micro Program Size (microSize/8)
	u32 cacheAddr;	// VU Cache Start Address
	static const u32 cacheSize = 0x400000; // VU Cache Size
	
	VURegs*	regs;	// VU Regs Struct
	u8*		cache;	// Dynarec Cache Start (where we will start writing the recompiled code to)
	u8*		ptr;	// Pointer to next place to write recompiled code to

	uptr x86callstack;
	uptr x86ebp;
	uptr x86esi;
	uptr x86edi;
	uptr x86ebx;
	uptr x86esp;

	microProgManager<0x800> prog; // Micro Program Data
};

// Opcode Tables
extern void (*mVU_UPPER_OPCODE[64])( VURegs* VU, s32 info );
extern void (*mVU_LOWER_OPCODE[128])( VURegs* VU, s32 info );

//void invalidateBlocks(u32 addr, u32 size); // Invalidates Blocks in the range [addr, addr+size)
__forceinline void mVUinit(megaVU* mVU, VURegs* vuRegsPtr, const int vuIndex);
__forceinline void mVUreset(megaVU* mVU);
__forceinline void mVUclose(megaVU* mVU);
__forceinline void mVUclear(megaVU* mVU, u32 addr, u32 size); // Clears part of a Micro Program (must use before modifying micro program!)
void* mVUexecute(megaVU* mVU, u32 startPC, u32 cycles); // Recompiles/Executes code for the number of cycles indicated (will always run for >= 'cycles' amount unless 'finished')
void* mVUexecuteF(megaVU* mVU, u32 startPC); // Recompiles/Executes code till finished

__forceinline int mVUfindLeastUsedProg(megaVU* mVU);
__forceinline int mVUsearchProg(megaVU* mVU);
__forceinline void mVUcacheProg(megaVU* mVU);
