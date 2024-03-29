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

#include "PrecompiledHeader.h"

#include "Common.h"
#include "Hw.h"
#include "GS.h"

#include "Vif.h"
#include "VifDma.h"

//////////////////////////////////////////////////////////////////////////
/////////////////////////// Quick & dirty FIFO :D ////////////////////////
//////////////////////////////////////////////////////////////////////////
extern int FIFOto_write(u32* pMem, int size);
extern void FIFOfrom_readsingle(void *value);

extern int g_nIPU0Data;
extern u8* g_pIPU0Pointer;
extern int FOreadpos;
// NOTE: cannot use XMM/MMX regs
void ReadFIFO(u32 mem, u64 *out) {
	if ((mem >= 0x10004000) && (mem < 0x10005000)) {
		VIF_LOG("ReadFIFO VIF0 0x%08X\n", mem);
		out[0] = psHu64(mem  );
		out[1] = psHu64(mem+8);
		return;
	} else
	if ((mem >= 0x10005000) && (mem < 0x10006000)) {

#ifdef PCSX2_DEVBUILD
		VIF_LOG("ReadFIFO VIF1 0x%08X\n", mem);

		if( vif1Regs->stat & (VIF1_STAT_INT|VIF1_STAT_VSS|VIF1_STAT_VIS|VIF1_STAT_VFS) ) {
			SysPrintf("reading from vif1 fifo when stalled\n");
		}
#endif

		if (vif1Regs->stat & 0x800000) {
			if (--psHu32(D1_QWC) == 0) {
				vif1Regs->stat&= ~0x1f000000;
			} else {
			}
		}
		out[0] = psHu64(mem  );
		out[1] = psHu64(mem+8);
		return;
	} else if( (mem&0xfffff010) == 0x10007000) {
		
		if( g_nIPU0Data > 0 ) {
			out[0] = *(u64*)(g_pIPU0Pointer);
			out[1] = *(u64*)(g_pIPU0Pointer+8);
			FOreadpos = (FOreadpos + 4) & 31;
			g_nIPU0Data--;
			g_pIPU0Pointer += 16;
		}
		return;
	}else if ( (mem&0xfffff010) == 0x10007010) {
		FIFOfrom_readsingle((void*)out);
		return;
	}
	SysPrintf("ReadFIFO Unknown %x\n", mem);
}

void ConstReadFIFO(u32 mem)
{
	// not done
}

void WriteFIFO(u32 mem, const u64 *value) {
	int ret;

	if ((mem >= 0x10004000) && (mem < 0x10005000)) {
		VIF_LOG("WriteFIFO VIF0 0x%08X\n", mem);
		
		psHu64(mem  ) = value[0];
		psHu64(mem+8) = value[1];
		vif0ch->qwc += 1;
		ret = VIF0transfer((u32*)value, 4, 0);
		assert(ret == 0 ); // vif stall code not implemented
	}
	else if ((mem >= 0x10005000) && (mem < 0x10006000)) {
		VIF_LOG("WriteFIFO VIF1 0x%08X\n", mem);
		
		psHu64(mem  ) = value[0];
		psHu64(mem+8) = value[1];

#ifdef PCSX2_DEVBUILD
		if(vif1Regs->stat & VIF1_STAT_FDR)
			SysPrintf("writing to fifo when fdr is set!\n");
		if( vif1Regs->stat & (VIF1_STAT_INT|VIF1_STAT_VSS|VIF1_STAT_VIS|VIF1_STAT_VFS) ) {
			SysPrintf("writing to vif1 fifo when stalled\n");
		}
#endif
		vif1ch->qwc += 1;
		ret = VIF1transfer((u32*)value, 4, 0);
		assert(ret == 0 ); // vif stall code not implemented
	}
	else if ((mem >= 0x10006000) && (mem < 0x10007000)) {
		GIF_LOG("WriteFIFO GIF 0x%08X\n", mem);

		psHu64(mem  ) = value[0];
		psHu64(mem+8) = value[1];

		if( mtgsThread != NULL )
		{
			const uint count = mtgsThread->PrepDataPacket( GIF_PATH_3, value, 1 );
			jASSUME( count == 1 );
			u64* data = (u64*)mtgsThread->GetDataPacketPtr();
			data[0] = value[0];
			data[1] = value[1];
			mtgsThread->SendDataPacket();
		}
		else
		{
			FreezeXMMRegs(1);
			GSGIFTRANSFER3((u32*)value, 1);
			FreezeXMMRegs(0);
		}

	} else
	if ((mem&0xfffff010) == 0x10007010) {
		IPU_LOG("WriteFIFO IPU_in[%d] <- %8.8X_%8.8X_%8.8X_%8.8X\n", (mem - 0x10007010)/8, ((u32*)value)[3], ((u32*)value)[2], ((u32*)value)[1], ((u32*)value)[0]);

		//commiting every 16 bytes
		while( FIFOto_write((u32*)value, 1) == 0 ) {
			Console::WriteLn("IPU sleeping");
			Threading::Timeslice();
		}
	} else {
		Console::Notice("WriteFIFO Unknown %x", params mem);
	}
}

void ConstWriteFIFO(u32 mem) {
}
