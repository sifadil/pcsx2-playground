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

//all tables for R5900 are define here..

#include "InterTables.h"
#include "R5900.h"

#include "x86/iR5900AritImm.h"
#include "x86/iR5900Arit.h"
#include "x86/iR5900MultDiv.h"
#include "x86/iR5900Shift.h"
#include "x86/iR5900Branch.h"
#include "x86/iR5900Jump.h"
#include "x86/iR5900LoadStore.h"
#include "x86/iR5900Move.h"
#include "x86/iMMI.h"
#include "x86/iCP0.h"
#include "x86/iFPU.h"

namespace R5900
{
	namespace Opcodes
	{
		// Generates an entry for the given opcode name.
		// Assumes the default function naming schemes for interpreter and recompiler  functions.
	#	define MakeOpcode( name, cycles ) \
		static const OPCODE name = { \
			#name, \
			cycles, \
			NULL, \
			R5900::Interpreter::OpcodeImpl::name, \
			Dynarec::R5900::OpcodeImpl::rec##name, \
			R5900::OpcodeDisasm::name \
		}

	#	define MakeOpcodeClass( name ) \
		static const OPCODE name = { \
			#name, \
			0, \
			R5900::Opcodes::Class_##name, \
			NULL, \
			NULL, \
			NULL \
		}

		// We're working on new hopefully better cycle ratios, but they're still a WIP.
		// And yes this whole thing is an ugly hack.  I'll clean it up once we have
		// a better idea how exactly the cycle ratios will work best.

		namespace Cycles
		{
			static const int Default = 9;
			static const int Branch = 11;
			static const int CopDefault = 7;

			static const int Mult = 2*8;
			static const int Div = 14*8;
			static const int MMI_Mult = 3*8;
			static const int MMI_Div = 22*8;

			static const int FPU_Mult = 12;

			static const int Store = 21;
			static const int Load = 11;
		}

		using namespace Cycles;

		MakeOpcode( Unknown, Default );
		MakeOpcode( MMI_Unknown, Default );
		MakeOpcode( COP0_Unknown, Default );
		MakeOpcode( COP1_Unknown, Default );

		// Class Subset Opcodes
		// (not really opcodes, but rather entire subsets of other opcode classes)

		MakeOpcodeClass( SPECIAL );
		MakeOpcodeClass( REGIMM );
		//MakeOpcodeClass( COP2 );
		MakeOpcodeClass( MMI );
		MakeOpcodeClass( MMI0 );
		MakeOpcodeClass( MMI2 );
		MakeOpcodeClass( MMI1 );
		MakeOpcodeClass( MMI3 );

		MakeOpcodeClass( COP0 );
		MakeOpcodeClass( COP1 );

		// Misc Junk

		MakeOpcode( COP2, Default );

		MakeOpcode( CACHE, Default );
		MakeOpcode( PREF, Default );
		MakeOpcode( SYSCALL, Default );
		MakeOpcode( BREAK, Default );
		MakeOpcode( SYNC, Default );

		// Branch/Jump Opcodes

		MakeOpcode( J , Default );
		MakeOpcode( JAL, Default );
		MakeOpcode( JR, Default );
		MakeOpcode( JALR, Default );

		MakeOpcode( BEQ, Branch );
		MakeOpcode( BNE, Branch );
		MakeOpcode( BLEZ, Branch );
		MakeOpcode( BGTZ, Branch );
		MakeOpcode( BEQL, Branch );
		MakeOpcode( BNEL, Branch );
		MakeOpcode( BLEZL, Branch );
		MakeOpcode( BGTZL, Branch );
		MakeOpcode( BLTZ, Branch );
		MakeOpcode( BGEZ, Branch );
		MakeOpcode( BLTZL, Branch );
		MakeOpcode( BGEZL, Branch );
		MakeOpcode( BLTZAL, Branch );
		MakeOpcode( BGEZAL, Branch );
		MakeOpcode( BLTZALL, Branch );
		MakeOpcode( BGEZALL, Branch );

		MakeOpcode( TGEI, Branch );
		MakeOpcode( TGEIU, Branch );
		MakeOpcode( TLTI, Branch );
		MakeOpcode( TLTIU, Branch );
		MakeOpcode( TEQI, Branch );
		MakeOpcode( TNEI, Branch );
		MakeOpcode( TGE, Branch );
		MakeOpcode( TGEU, Branch );
		MakeOpcode( TLT, Branch );
		MakeOpcode( TLTU, Branch );
		MakeOpcode( TEQ, Branch );
		MakeOpcode( TNE, Branch );

		// Arithmetic

		MakeOpcode( MULT, Mult );
		MakeOpcode( MULTU, Mult );
		MakeOpcode( MULT1, Mult );
		MakeOpcode( MULTU1, Mult );
		MakeOpcode( MADD, Mult );
		MakeOpcode( MADDU, Mult );
		MakeOpcode( MADD1, Mult );
		MakeOpcode( MADDU1, Mult );
		MakeOpcode( DIV, Div );
		MakeOpcode( DIVU, Div );
		MakeOpcode( DIV1, Div );
		MakeOpcode( DIVU1, Div );

		MakeOpcode( ADDI, Default );
		MakeOpcode( ADDIU, Default );
		MakeOpcode( DADDI, Default );
		MakeOpcode( DADDIU, Default );
		MakeOpcode( DADD, Default );
		MakeOpcode( DADDU, Default );
		MakeOpcode( DSUB, Default );
		MakeOpcode( DSUBU, Default );
		MakeOpcode( ADD, Default );
		MakeOpcode( ADDU, Default );
		MakeOpcode( SUB, Default );
		MakeOpcode( SUBU, Default );

		MakeOpcode( ANDI, Default );
		MakeOpcode( ORI, Default );
		MakeOpcode( XORI, Default );
		MakeOpcode( AND, Default );
		MakeOpcode( OR, Default );
		MakeOpcode( XOR, Default );
		MakeOpcode( NOR, Default );
		MakeOpcode( SLTI, Default );
		MakeOpcode( SLTIU, Default );
		MakeOpcode( SLT, Default );
		MakeOpcode( SLTU, Default );
		MakeOpcode( LUI, Default );
		MakeOpcode( SLL, Default );
		MakeOpcode( SRL, Default );
		MakeOpcode( SRA, Default );
		MakeOpcode( SLLV, Default );
		MakeOpcode( SRLV, Default );
		MakeOpcode( SRAV, Default );
		MakeOpcode( MOVZ, Default );
		MakeOpcode( MOVN, Default );
		MakeOpcode( DSLLV, Default );
		MakeOpcode( DSRLV, Default );
		MakeOpcode( DSRAV, Default );
		MakeOpcode( DSLL, Default );
		MakeOpcode( DSRL, Default );
		MakeOpcode( DSRA, Default );
		MakeOpcode( DSLL32, Default );
		MakeOpcode( DSRL32, Default );
		MakeOpcode( DSRA32, Default );

		MakeOpcode( MFHI, Default );
		MakeOpcode( MTHI, Default );
		MakeOpcode( MFLO, Default );
		MakeOpcode( MTLO, Default );
		MakeOpcode( MFSA, Default );
		MakeOpcode( MTSA, Default );
		MakeOpcode( MTSAB, Default );
		MakeOpcode( MTSAH, Default );
		MakeOpcode( MFHI1, Default );
		MakeOpcode( MTHI1, Default );
		MakeOpcode( MFLO1, Default );
		MakeOpcode( MTLO1, Default );

		// Loads!

		MakeOpcode( LDL, Load );
		MakeOpcode( LDR, Load );
		MakeOpcode( LQ, Load );
		MakeOpcode( LB, Load );
		MakeOpcode( LH, Load );
		MakeOpcode( LWL, Load );
		MakeOpcode( LW, Load );
		MakeOpcode( LBU, Load );
		MakeOpcode( LHU, Load );
		MakeOpcode( LWR, Load );
		MakeOpcode( LWU, Load );
		MakeOpcode( LWC1, Load );
		MakeOpcode( LQC2, Load );
		MakeOpcode( LD, Load );

		// Stores!

		MakeOpcode( SQ, Store );
		MakeOpcode( SB, Store );
		MakeOpcode( SH, Store );
		MakeOpcode( SWL, Store );
		MakeOpcode( SW, Store );
		MakeOpcode( SDL, Store );
		MakeOpcode( SDR, Store );
		MakeOpcode( SWR, Store );
		MakeOpcode( SWC1, Store );
		MakeOpcode( SQC2, Store );
		MakeOpcode( SD, Store );


		// Multimedia Instructions!

		MakeOpcode( PLZCW, Default );
		MakeOpcode( PMFHL, Default );
		MakeOpcode( PMTHL, Default );
		MakeOpcode( PSLLH, Default );
		MakeOpcode( PSRLH, Default );
		MakeOpcode( PSRAH, Default );
		MakeOpcode( PSLLW, Default );
		MakeOpcode( PSRLW, Default );
		MakeOpcode( PSRAW, Default );

		MakeOpcode( PADDW, Default );
		MakeOpcode( PADDH, Default );
		MakeOpcode( PADDB, Default );
		MakeOpcode( PADDSW, Default );
		MakeOpcode( PADDSH, Default );
		MakeOpcode( PADDSB, Default );
		MakeOpcode( PADDUW, Default );
		MakeOpcode( PADDUH, Default );
		MakeOpcode( PADDUB, Default );
		MakeOpcode( PSUBW, Default );
		MakeOpcode( PSUBH, Default );
		MakeOpcode( PSUBB, Default );
		MakeOpcode( PSUBSW, Default );
		MakeOpcode( PSUBSH, Default );
		MakeOpcode( PSUBSB, Default );
		MakeOpcode( PSUBUW, Default );
		MakeOpcode( PSUBUH, Default );
		MakeOpcode( PSUBUB, Default );

		MakeOpcode( PCGTW, Default );
		MakeOpcode( PMAXW, Default );
		MakeOpcode( PMAXH, Default );
		MakeOpcode( PCGTH, Default );
		MakeOpcode( PCGTB, Default );
		MakeOpcode( PEXTLW, Default );
		MakeOpcode( PEXTLH, Default );
		MakeOpcode( PEXTLB, Default );
		MakeOpcode( PEXT5, Default );
		MakeOpcode( PPACW, Default );
		MakeOpcode( PPACH, Default );
		MakeOpcode( PPACB, Default );
		MakeOpcode( PPAC5, Default );

		MakeOpcode( PABSW, Default );
		MakeOpcode( PABSH, Default );
		MakeOpcode( PCEQW, Default );
		MakeOpcode( PMINW, Default );
		MakeOpcode( PMINH, Default );
		MakeOpcode( PADSBH, Default );
		MakeOpcode( PCEQH, Default );
		MakeOpcode( PCEQB, Default );
		MakeOpcode( PEXTUW, Default );
		MakeOpcode( PEXTUH, Default );
		MakeOpcode( PEXTUB, Default );
		MakeOpcode( PSLLVW, Default );
		MakeOpcode( PSRLVW, Default );

		MakeOpcode( QFSRV, Default );

		MakeOpcode( PMADDH, MMI_Mult );
		MakeOpcode( PHMADH, MMI_Mult );
		MakeOpcode( PMSUBH, MMI_Mult );
		MakeOpcode( PHMSBH, MMI_Mult );
		MakeOpcode( PMULTH, MMI_Mult );
		MakeOpcode( PMADDW, MMI_Mult );
		MakeOpcode( PMSUBW, MMI_Mult );
		MakeOpcode( PMFHI, MMI_Mult );
		MakeOpcode( PMFLO, MMI_Mult );
		MakeOpcode( PMULTW, MMI_Mult );
		MakeOpcode( PMADDUW, MMI_Mult );
		MakeOpcode( PMULTUW, MMI_Mult );
		MakeOpcode( PDIVUW, MMI_Div );
		MakeOpcode( PDIVW, MMI_Div );
		MakeOpcode( PDIVBW, MMI_Div );

		MakeOpcode( PINTH, Default );
		MakeOpcode( PCPYLD, Default );
		MakeOpcode( PAND, Default );
		MakeOpcode( PXOR, Default );
		MakeOpcode( PEXEH, Default );
		MakeOpcode( PREVH, Default );
		MakeOpcode( PEXEW, Default );
		MakeOpcode( PROT3W, Default );

		MakeOpcode( PSRAVW, Default ); 
		MakeOpcode( PMTHI, Default );
		MakeOpcode( PMTLO, Default );
		MakeOpcode( PINTEH, Default );
		MakeOpcode( PCPYUD, Default );
		MakeOpcode( POR, Default );
		MakeOpcode( PNOR, Default );
		MakeOpcode( PEXCH, Default );
		MakeOpcode( PCPYH, Default );
		MakeOpcode( PEXCW, Default );

		//////////////////////////////////////////////////////////
		// COP0 Instructions

		MakeOpcodeClass( COP0_C0 );
		MakeOpcodeClass( COP0_BC0 );

		MakeOpcode( MFC0, CopDefault );
		MakeOpcode( MTC0, CopDefault );

		MakeOpcode( BC0F, Branch );
		MakeOpcode( BC0T, Branch );
		MakeOpcode( BC0FL, Branch );
		MakeOpcode( BC0TL, Branch );

		MakeOpcode( TLBR, CopDefault );
		MakeOpcode( TLBWI, CopDefault );
		MakeOpcode( TLBWR, CopDefault );
		MakeOpcode( TLBP, CopDefault );
		MakeOpcode( ERET, CopDefault );
		MakeOpcode( EI, CopDefault );
		MakeOpcode( DI, CopDefault );

		//////////////////////////////////////////////////////////
		// COP1 Instructions!

		MakeOpcodeClass( COP1_BC1 );
		MakeOpcodeClass( COP1_S );
		MakeOpcodeClass( COP1_W );		// contains CVT_S instruction *only*

		MakeOpcode( MFC1, CopDefault );
		MakeOpcode( CFC1, CopDefault );
		MakeOpcode( MTC1, CopDefault );
		MakeOpcode( CTC1, CopDefault );

		MakeOpcode( BC1F, Branch );
		MakeOpcode( BC1T, Branch );
		MakeOpcode( BC1FL, Branch );
		MakeOpcode( BC1TL, Branch );

		MakeOpcode( ADD_S, CopDefault );
		MakeOpcode( ADDA_S, CopDefault );
		MakeOpcode( SUB_S, CopDefault );
		MakeOpcode( SUBA_S, CopDefault );

		MakeOpcode( ABS_S, CopDefault );
		MakeOpcode( MOV_S, CopDefault );
		MakeOpcode( NEG_S, CopDefault );
		MakeOpcode( MAX_S, CopDefault );
		MakeOpcode( MIN_S, CopDefault );

		MakeOpcode( MUL_S, FPU_Mult );
		MakeOpcode( DIV_S, 3*8 );
		MakeOpcode( SQRT_S, 3*8 );
		MakeOpcode( RSQRT_S, 4*8 );
		MakeOpcode( MULA_S, FPU_Mult );
		MakeOpcode( MADD_S, FPU_Mult );
		MakeOpcode( MSUB_S, FPU_Mult );
		MakeOpcode( MADDA_S, FPU_Mult );
		MakeOpcode( MSUBA_S, FPU_Mult );

		MakeOpcode( C_F, CopDefault );
		MakeOpcode( C_EQ, CopDefault );
		MakeOpcode( C_LT, CopDefault );
		MakeOpcode( C_LE, CopDefault );

		MakeOpcode( CVT_S, CopDefault );
		MakeOpcode( CVT_W, CopDefault );
	}

	namespace OpcodeTables
	{
		using namespace Opcodes;

		const OPCODE tbl_Standard[64] = 
		{
			SPECIAL,       REGIMM,        J,             JAL,     BEQ,           BNE,     BLEZ,  BGTZ,
			ADDI,          ADDIU,         SLTI,          SLTIU,   ANDI,          ORI,     XORI,  LUI,
			COP0,          COP1,          COP2,          Unknown, BEQL,          BNEL,    BLEZL, BGTZL,
			DADDI,         DADDIU,        LDL,           LDR,     MMI,           Unknown, LQ,    SQ,
			LB,            LH,            LWL,           LW,      LBU,           LHU,     LWR,   LWU,
			SB,            SH,            SWL,           SW,      SDL,           SDR,     SWR,   CACHE,
			Unknown,       LWC1,          Unknown,       PREF,    Unknown,       Unknown, LQC2,  LD,
			Unknown,       SWC1,          Unknown,       Unknown, Unknown,       Unknown, SQC2,  SD
		};

		static const OPCODE tbl_Special[64] = 
		{
			SLL,      Unknown,  SRL,      SRA,      SLLV,    Unknown, SRLV,    SRAV,
			JR,       JALR,     MOVZ,     MOVN,     SYSCALL, BREAK,   Unknown, SYNC,
			MFHI,     MTHI,     MFLO,     MTLO,     DSLLV,   Unknown, DSRLV,   DSRAV,
			MULT,     MULTU,    DIV,      DIVU,     Unknown, Unknown, Unknown, Unknown,
			ADD,      ADDU,     SUB,      SUBU,     AND,     OR,      XOR,     NOR,
			MFSA,     MTSA,     SLT,      SLTU,     DADD,    DADDU,   DSUB,    DSUBU,
			TGE,      TGEU,     TLT,      TLTU,     TEQ,     Unknown, TNE,     Unknown,
			DSLL,     Unknown,  DSRL,     DSRA,     DSLL32,  Unknown, DSRL32,  DSRA32
		};

		static const OPCODE tbl_RegImm[32] = {
			BLTZ,   BGEZ,   BLTZL,      BGEZL,   Unknown, Unknown, Unknown, Unknown,
			TGEI,   TGEIU,  TLTI,       TLTIU,   TEQI,    Unknown, TNEI,    Unknown,
			BLTZAL, BGEZAL, BLTZALL,    BGEZALL, Unknown, Unknown, Unknown, Unknown,
			MTSAB,  MTSAH , Unknown,    Unknown, Unknown, Unknown, Unknown, Unknown,
		};

		static const OPCODE tbl_MMI[64] = 
		{
			MADD,               MADDU,           MMI_Unknown,          MMI_Unknown,          PLZCW,            MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
			MMI0,      MMI2,   MMI_Unknown,          MMI_Unknown,          MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
			MFHI1,              MTHI1,           MFLO1,                MTLO1,                MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
			MULT1,              MULTU1,          DIV1,                 DIVU1,                MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
			MADD1,              MADDU1,          MMI_Unknown,          MMI_Unknown,          MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
			MMI1,      MMI3,   MMI_Unknown,          MMI_Unknown,          MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
			PMFHL,              PMTHL,           MMI_Unknown,          MMI_Unknown,          PSLLH,            MMI_Unknown,       PSRLH,                PSRAH,
			MMI_Unknown,        MMI_Unknown,     MMI_Unknown,          MMI_Unknown,          PSLLW,            MMI_Unknown,       PSRLW,                PSRAW,
		};

		static const OPCODE tbl_MMI0[32] = 
		{ 
			PADDW,         PSUBW,         PCGTW,          PMAXW,       
			PADDH,         PSUBH,         PCGTH,          PMAXH,        
			PADDB,         PSUBB,         PCGTB,          MMI_Unknown,
			MMI_Unknown,   MMI_Unknown,   MMI_Unknown,    MMI_Unknown,
			PADDSW,        PSUBSW,        PEXTLW,         PPACW,        
			PADDSH,        PSUBSH,        PEXTLH,         PPACH,        
			PADDSB,        PSUBSB,        PEXTLB,         PPACB,        
			MMI_Unknown,   MMI_Unknown,   PEXT5,          PPAC5,        
		};

		static const OPCODE tbl_MMI1[32] =
		{ 
			MMI_Unknown,   PABSW,         PCEQW,         PMINW, 
			PADSBH,        PABSH,         PCEQH,         PMINH, 
			MMI_Unknown,   MMI_Unknown,   PCEQB,         MMI_Unknown, 
			MMI_Unknown,   MMI_Unknown,   MMI_Unknown,   MMI_Unknown, 
			PADDUW,        PSUBUW,        PEXTUW,        MMI_Unknown,  
			PADDUH,        PSUBUH,        PEXTUH,        MMI_Unknown, 
			PADDUB,        PSUBUB,        PEXTUB,        QFSRV, 
			MMI_Unknown,   MMI_Unknown,   MMI_Unknown,   MMI_Unknown, 
		};


		static const OPCODE tbl_MMI2[32] = 
		{ 
			PMADDW,        MMI_Unknown,   PSLLVW,        PSRLVW, 
			PMSUBW,        MMI_Unknown,   MMI_Unknown,   MMI_Unknown,
			PMFHI,         PMFLO,         PINTH,         MMI_Unknown,
			PMULTW,        PDIVW,         PCPYLD,        MMI_Unknown,
			PMADDH,        PHMADH,        PAND,          PXOR, 
			PMSUBH,        PHMSBH,        MMI_Unknown,   MMI_Unknown, 
			MMI_Unknown,   MMI_Unknown,   PEXEH,         PREVH, 
			PMULTH,        PDIVBW,        PEXEW,         PROT3W, 
		};

		static const OPCODE tbl_MMI3[32] = 
		{ 
			PMADDUW,       MMI_Unknown,   MMI_Unknown,   PSRAVW, 
			MMI_Unknown,   MMI_Unknown,   MMI_Unknown,   MMI_Unknown,
			PMTHI,         PMTLO,         PINTEH,        MMI_Unknown,
			PMULTUW,       PDIVUW,        PCPYUD,        MMI_Unknown,
			MMI_Unknown,   MMI_Unknown,   POR,           PNOR,  
			MMI_Unknown,   MMI_Unknown,   MMI_Unknown,   MMI_Unknown,
			MMI_Unknown,   MMI_Unknown,   PEXCH,         PCPYH, 
			MMI_Unknown,   MMI_Unknown,   PEXCW,         MMI_Unknown,
		};

		static const OPCODE tbl_COP0[32] = 
		{
			MFC0,         COP0_Unknown, COP0_Unknown, COP0_Unknown, MTC0,         COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_BC0, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_C0,  COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
		};

		static const OPCODE tbl_COP0_BC0[32] = 
		{
			BC0F,         BC0T,         BC0FL,        BC0TL,        COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
		};

		static const OPCODE tbl_COP0_C0[64] =
		{
			COP0_Unknown, TLBR,         TLBWI,        COP0_Unknown, COP0_Unknown, COP0_Unknown, TLBWR,        COP0_Unknown,
			TLBP,         COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			ERET,         COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown,
			EI,           DI,           COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown, COP0_Unknown
		};

		static const OPCODE tbl_COP1[32] =
		{
			MFC1,         COP1_Unknown, CFC1,         COP1_Unknown, MTC1,         COP1_Unknown, CTC1,         COP1_Unknown,
			COP1_BC1, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown,
			COP1_S,   COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_W, COP1_Unknown, COP1_Unknown, COP1_Unknown,
			COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown,
		};

		static const OPCODE tbl_COP1_BC1[32] =
		{
			BC1F,         BC1T,         BC1FL,        BC1TL,        COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown,
			COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown,
			COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown,
			COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown, COP1_Unknown,
		};

		static const OPCODE tbl_COP1_S[64] =
		{
			ADD_S,       SUB_S,       MUL_S,       DIV_S,       SQRT_S,      ABS_S,       MOV_S,       NEG_S, 
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,   
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,RSQRT_S,     COP1_Unknown,  
			ADDA_S,      SUBA_S,      MULA_S,      COP1_Unknown,MADD_S,      MSUB_S,      MADDA_S,     MSUBA_S,
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,CVT_W,       COP1_Unknown,COP1_Unknown,COP1_Unknown, 
			MAX_S,       MIN_S,       COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown, 
			C_F,         COP1_Unknown,C_EQ,        COP1_Unknown,C_LT,        COP1_Unknown,C_LE,        COP1_Unknown, 
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown, 
		};

		static const OPCODE tbl_COP1_W[64] = 
		{
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,   	
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,   
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,   
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,   
			CVT_S,       COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,   
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,   
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,   
			COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,COP1_Unknown,   
		};

	}	// end namespace R5900::OpcodeTables

	namespace Opcodes
	{
		using namespace OpcodeTables;

		const OPCODE& Class_SPECIAL() { return tbl_Special[_Funct_]; }
		const OPCODE& Class_REGIMM()  { return tbl_RegImm[_Rt_]; }

		const OPCODE& Class_MMI()  { return tbl_MMI[_Funct_]; }
		const OPCODE& Class_MMI0() { return tbl_MMI0[_Sa_]; }
		const OPCODE& Class_MMI1() { return tbl_MMI1[_Sa_]; }
		const OPCODE& Class_MMI2() { return tbl_MMI2[_Sa_]; }
		const OPCODE& Class_MMI3() { return tbl_MMI3[_Sa_]; }

		const OPCODE& Class_COP0() { return tbl_COP0[_Rs_]; }
		const OPCODE& Class_COP0_BC0() { return tbl_COP0_BC0[(cpuRegs.code >> 16) & 0x03]; }
		const OPCODE& Class_COP0_C0() { return tbl_COP0_C0[_Funct_]; }

		const OPCODE& Class_COP1() { return tbl_COP1[_Rs_]; }
		const OPCODE& Class_COP1_BC1() { return tbl_COP1_BC1[_Rt_]; }
		const OPCODE& Class_COP1_S() { return tbl_COP1_S[_Funct_]; }
		const OPCODE& Class_COP1_W() { return tbl_COP1_W[_Funct_]; }

		// These are for future use when the COP2 tables are completed.
		//const OPCODE& Class_COP2() { return tbl_COP2[_Rs_]; }
		//const OPCODE& Class_COP2_BC2() { return tbl_COP2_BC2[_Rt_]; }
		//const OPCODE& Class_COP2_SPECIAL() { return tbl_COP2_SPECIAL[_Funct_]; }
		//const OPCODE& Class_COP2_SPECIAL2() { return tbl_COP2_SPECIAL2[(cpuRegs.code & 0x3) | ((cpuRegs.code >> 4) & 0x7c)]; }
	}
}	// end namespace R5900

void (*Int_COP2PrintTable[32])() = {
    COP2_Unknown, QMFC2,        CFC2,         COP2_Unknown, COP2_Unknown, QMTC2,        CTC2,         COP2_Unknown,
    COP2_BC2,     COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown,
    COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL,
	COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL, COP2_SPECIAL,
};

void (*Int_COP2BC2PrintTable[32])() = {
    BC2F,         BC2T,         BC2FL,        BC2TL,        COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown,
    COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown,
    COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown,
    COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown, COP2_Unknown,
}; 

void (*Int_COP2SPECIAL1PrintTable[64])() = 
{ 
 VADDx,       VADDy,       VADDz,       VADDw,       VSUBx,        VSUBy,        VSUBz,        VSUBw,  
 VMADDx,      VMADDy,      VMADDz,      VMADDw,      VMSUBx,       VMSUBy,       VMSUBz,       VMSUBw, 
 VMAXx,       VMAXy,       VMAXz,       VMAXw,       VMINIx,       VMINIy,       VMINIz,       VMINIw, 
 VMULx,       VMULy,       VMULz,       VMULw,       VMULq,        VMAXi,        VMULi,        VMINIi,
 VADDq,       VMADDq,      VADDi,       VMADDi,      VSUBq,        VMSUBq,       VSUBi,        VMSUBi, 
 VADD,        VMADD,       VMUL,        VMAX,        VSUB,         VMSUB,        VOPMSUB,      VMINI,  
 VIADD,       VISUB,       VIADDI,      COP2_Unknown,VIAND,        VIOR,         COP2_Unknown, COP2_Unknown,
 VCALLMS,     VCALLMSR,    COP2_Unknown,COP2_Unknown,COP2_SPECIAL2,COP2_SPECIAL2,COP2_SPECIAL2,COP2_SPECIAL2,  
};

void (*Int_COP2SPECIAL2PrintTable[128])() = 
{
 VADDAx      ,VADDAy      ,VADDAz      ,VADDAw      ,VSUBAx      ,VSUBAy      ,VSUBAz      ,VSUBAw,
 VMADDAx     ,VMADDAy     ,VMADDAz     ,VMADDAw     ,VMSUBAx     ,VMSUBAy     ,VMSUBAz     ,VMSUBAw,
 VITOF0      ,VITOF4      ,VITOF12     ,VITOF15     ,VFTOI0      ,VFTOI4      ,VFTOI12     ,VFTOI15,
 VMULAx      ,VMULAy      ,VMULAz      ,VMULAw      ,VMULAq      ,VABS        ,VMULAi      ,VCLIPw,
 VADDAq      ,VMADDAq     ,VADDAi      ,VMADDAi     ,VSUBAq      ,VMSUBAq     ,VSUBAi      ,VMSUBAi,
 VADDA       ,VMADDA      ,VMULA       ,COP2_Unknown,VSUBA       ,VMSUBA      ,VOPMULA     ,VNOP,   
 VMOVE       ,VMR32       ,COP2_Unknown,COP2_Unknown,VLQI        ,VSQI        ,VLQD        ,VSQD,   
 VDIV        ,VSQRT       ,VRSQRT      ,VWAITQ      ,VMTIR       ,VMFIR       ,VILWR       ,VISWR,  
 VRNEXT      ,VRGET       ,VRINIT      ,VRXOR       ,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown, 
 COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,
 COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,
 COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,
 COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,
 COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,
 COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,
 COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,COP2_Unknown,
};
