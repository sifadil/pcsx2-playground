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
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __MISC_H__
#define __MISC_H__

#include "PS2Etypes.h"
#include "System.h"
#include "SaveState.h"
#include "assert.h"

// --->> GNU GetText / NLS

#ifdef ENABLE_NLS

#ifdef _WIN32
#include "libintlmsc.h"
#else
#include <locale.h>
#include <libintl.h>
#endif

#undef _
#define _(String) dgettext (PACKAGE, String)
#ifdef gettext_noop
#  define N_(String) gettext_noop (String)
#else
#  define N_(String) (String)
#endif

#else

#define _(msgid) msgid
#define N_(msgid) msgid

#endif		// ENABLE_NLS

// <<--- End GNU GetText / NLS 


// --->> Path Utilities [PathUtil.c]

#define g_MaxPath 255			// 255 is safer with antiquitated Win32 ASCII APIs.

namespace Path
{
	void Combine( string& dest, const string& srcPath, const string& srcFile );
	bool isRooted( const string& path );
	bool isDirectory( const string& path );
	bool isFile( const string& path );
	bool Exists( const string& path );
	int getFileSize( const string& path );

	void ReplaceExtension( string& dest, const string& src, const string& ext );
}

// <<--- END Path Utilities [PathUtil.c]

// [TODO] : Move the config options mess from Misc.h into "config.h" or someting more sensible.

/////////////////////////////////////////////////////////////////////////
// Session Configuration Override Flags
//
// a handful of flags that can override user configurations for the current application session
// only.  This allows us to do things like force-disable recompilers if the memory allocations
// for them fail.
struct SessionOverrideFlags
{
	bool ForceDisableEErec:1;
	bool ForceDisableVU0rec:1;
	bool ForceDisableVU1rec:1;
};

extern SessionOverrideFlags g_Session;

//////////////////////////////////////////////////////////////////////////
// Pcsx2 User Configuration Options!

#define PCSX2_GSMULTITHREAD 1 // uses multithreaded gs
#define PCSX2_EEREC 0x10
#define PCSX2_VU0REC 0x20
#define PCSX2_VU1REC 0x40
#define PCSX2_COP2REC 0x80
#define PCSX2_FRAMELIMIT_MASK 0xc00
#define PCSX2_FRAMELIMIT_NORMAL 0x000
#define PCSX2_FRAMELIMIT_LIMIT 0x400
#define PCSX2_FRAMELIMIT_SKIP 0x800
#define PCSX2_FRAMELIMIT_VUSKIP 0xc00

#define CHECK_MULTIGS (Config.Options&PCSX2_GSMULTITHREAD)
#define CHECK_EEREC (!g_Session.ForceDisableEErec && Config.Options&PCSX2_EEREC)
//------------ SPEED/MISC HACKS!!! ---------------
#define CHECK_EE_CYCLERATE (Config.Hacks & 0x03)
#define CHECK_IOP_CYCLERATE (Config.Hacks & (1<<3))
#define CHECK_WAITCYCLE_HACK (Config.Hacks & (1<<4))
#define CHECK_ESCAPE_HACK	(Config.Hacks & 0x400)
//------------ SPECIAL GAME FIXES!!! ---------------
#define CHECK_FPUCLAMPHACK	(Config.GameFixes & 0x4) // Special Fix for Tekken 5, different clamping for FPU (sets NaN to zero; doesn't clamp infinities)
#define CHECK_VUCLIPHACK	(Config.GameFixes & 0x2) // Special Fix for GoW, updates the clipflag differently in recVUMI_CLIP() (note: turning this hack on, breaks Rockstar games)
#define CHECK_VUBRANCHHACK	(Config.GameFixes & 0x8) // Special Fix for Magna Carta (note: Breaks Crash Bandicoot)
//------------ Advanced Options!!! ---------------
#define CHECK_VU_OVERFLOW		(Config.vuOptions & 0x1)
#define CHECK_VU_EXTRA_OVERFLOW	(Config.vuOptions & 0x2) // If enabled, Operands are clamped before being used in the VU recs
#define CHECK_VU_SIGN_OVERFLOW	(Config.vuOptions & 0x4)
#define CHECK_VU_UNDERFLOW		(Config.vuOptions & 0x8)
#define CHECK_VU_EXTRA_FLAGS 0	// Always disabled now, doesn't seem to affect games positively. // Sets correct flags in the VU recs
#define CHECK_FPU_OVERFLOW			(Config.eeOptions & 0x1)
#define CHECK_FPU_EXTRA_OVERFLOW	(Config.eeOptions & 0x2) // If enabled, Operands are checked for infinities before being used in the FPU recs
#define CHECK_FPU_EXTRA_FLAGS 0	// Always disabled now, doesn't seem to affect games positively. // Sets correct flags in the FPU recs
#define DEFAULT_eeOptions	0x01
#define DEFAULT_vuOptions	0x01
//------------ DEFAULT sseMXCSR VALUES!!! ---------------
#define DEFAULT_sseMXCSR	0x9fc0 //disable all exception, round to 0, flush to 0
#define DEFAULT_sseVUMXCSR	0x7f80 //disable all exception

#define CHECK_FRAMELIMIT (Config.Options&PCSX2_FRAMELIMIT_MASK)

#define CHECK_VU0REC (!g_Session.ForceDisableVU0rec && Config.Options&PCSX2_VU0REC)
#define CHECK_VU1REC (!g_Session.ForceDisableVU1rec && (Config.Options&PCSX2_VU1REC))


struct PcsxConfig
{
	char Bios[g_MaxPath];
	char GS[g_MaxPath];
	char PAD1[g_MaxPath];
	char PAD2[g_MaxPath];
	char SPU2[g_MaxPath];
	char CDVD[g_MaxPath];
	char DEV9[g_MaxPath];
	char USB[g_MaxPath];
	char FW[g_MaxPath];
	char Mcd1[g_MaxPath];
	char Mcd2[g_MaxPath];
	char PluginsDir[g_MaxPath];
	char BiosDir[g_MaxPath];
	char Lang[g_MaxPath];

	u32 Options; // PCSX2_X options

	bool PsxOut;
	bool Profiler; // Displays profiling info to console
	bool cdvdPrint; // Prints cdvd reads to console 
	bool closeGSonEsc; // closes the GS (and saves its state) on escape automatically.

	int PsxType;
	int Cdda;
	int Mdec;
	int Patch;
	int ThPriority;
	int CustomFps;
	int Hacks;
	int GameFixes;
	int CustomFrameSkip;
	int CustomConsecutiveFrames;
	int CustomConsecutiveSkip;
	u32 sseMXCSR;
	u32 sseVUMXCSR;
	u32 eeOptions;
	u32 vuOptions;
};

extern PcsxConfig Config;
extern u32 BiosVersion;
extern char CdromId[12];
extern uptr pDsp;		// what the hell is this unused piece of crap passed to every plugin for? (air)

int LoadCdrom();
int CheckCdrom();
int GetPS2ElfName(char*);

extern const char *LabelAuthors;
extern const char *LabelGreets;

void SaveGSState(const string& file);
void LoadGSState(const string& file);

char *ParseLang(char *id);
void ProcessFKeys(int fkey, int shift); // processes fkey related commands value 1-12

#define DIRENTRY_SIZE 16

#if defined(_MSC_VER)
#pragma pack(1)
#endif

struct romdir{
	char fileName[10];
	u16 extInfoSize;
	u32 fileSize;
#if defined(_MSC_VER)
};						//+22
#else
} __attribute__((packed));
#endif

u32 GetBiosVersion();
int IsBIOS(char *filename, char *description);

extern u32 g_sseVUMXCSR, g_sseMXCSR;

void SetCPUState(u32 sseMXCSR, u32 sseVUMXCSR);

// when using mmx/xmm regs, use; 0 is load
// freezes no matter the state
extern void FreezeXMMRegs_(int save);
extern void FreezeMMXRegs_(int save);
extern bool g_EEFreezeRegs;
extern u8 g_globalMMXSaved;
extern u8 g_globalXMMSaved;

// these macros check to see if needs freezing
#define FreezeXMMRegs(save) if( g_EEFreezeRegs ) { FreezeXMMRegs_(save); }
#define FreezeMMXRegs(save) if( g_EEFreezeRegs ) { FreezeMMXRegs_(save); }


#if defined(_WIN32) && !defined(__x86_64__)
	// faster memcpy
	extern void __fastcall memcpy_raz_u(void *dest, const void *src, size_t bytes);
	extern void __fastcall memcpy_raz_(void *dest, const void *src, size_t qwc);
	extern void * memcpy_amd_(void *dest, const void *src, size_t n);
#	include "windows/memzero.h"

#define memcpy_fast memcpy_amd_
	//#define memcpy_fast memcpy //Dont use normal memcpy, it has sse in 2k5!
#else
	// for now disable linux fast memcpy
	#define memcpy_fast memcpy
	#define memcpy_raz_ memcpy
	#define memcpy_raz_u memcpy
#endif


u8 memcmp_mmx(const void* src1, const void* src2, int cmpsize);
void memxor_mmx(void* dst, const void* src1, int cmpsize);

#ifdef	_MSC_VER
#pragma pack()
#endif

void injectIRX(const char *filename);

// aligned_malloc: Implement/declare linux equivalents here!
#if !defined(_MSC_VER) && !defined(HAVE_ALIGNED_MALLOC)

static  __forceinline void* pcsx2_aligned_malloc(size_t size, size_t align)
{
	assert( align < 0x10000 );
	char* p = (char*)malloc(size+align);
	int off = 2+align - ((int)(uptr)(p+2) % align);

	p += off;
	*(u16*)(p-2) = off;

	return p;
}

static __forceinline void pcsx2_aligned_free(void* pmem)
{
	if( pmem != NULL ) {
		char* p = (char*)pmem;
		free(p - (int)*(u16*)(p-2));
	}
}

#define _aligned_malloc pcsx2_aligned_malloc
#define _aligned_free pcsx2_aligned_free

#endif

extern void InitCPUTicks();
extern u64 GetTickFrequency();
extern u64 GetCPUTicks();


#endif /* __MISC_H__ */

