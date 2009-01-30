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

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "PS2Etypes.h"
#include "Exceptions.h"
#include "Paths.h"

void SysDetect();						// Detects cpu type and fills cpuInfo structs.
bool SysInit();							// Init logfiles, directories, critical memory resources, and other OS-specific one-time
void SysReset();						// Resets the various PS2 cpus, sub-systems, and recompilers.
void SysUpdate();						// Called on VBlank (to update i.e. pads)
void SysClose();						// Close mem and plugins

bool SysAllocateMem();			// allocates memory for all PS2 systems; returns FALSe on critical error.
void SysAllocateDynarecs();		// allocates memory for all dynarecs, and force-disables any failures.
void SysShutdownDynarecs();
void SysShutdownMem();
void SysResetExecutionState();

void *SysLoadLibrary(const char *lib);	// Loads Library
void *SysLoadSym(void *lib, const char *sym);	// Loads Symbol from Library
const char *SysLibError();				// Gets previous error loading sysbols
void SysCloseLibrary(void *lib);		// Closes Library

// Maps a block of memory for use as a recompiled code buffer.
// The allocated block has code execution privliges.
// Returns NULL on allocation failure.
void *SysMmap(uptr base, u32 size);

// Unmaps a block allocated by SysMmap
void SysMunmap(uptr base, u32 size);

// Writes text to the console.
// *DEPRECIATED* Use Console namespace methods instead.
void SysPrintf(const char *fmt, ...);	// *DEPRECIATED* 

static __forceinline void SysMunmap( void* base, u32 size )
{
	SysMunmap( (uptr)base, size );
}


#ifdef _MSC_VER
#	define PCSX2_MEM_PROTECT_BEGIN() __try {
#	define PCSX2_MEM_PROTECT_END() } __except(SysPageFaultExceptionFilter(GetExceptionInformation())) {}
#else
#	define PCSX2_MEM_PROTECT_BEGIN() InstallLinuxExceptionHandler()
#	define PCSX2_MEM_PROTECT_END() ReleaseLinuxExceptionHandler()
#endif


// Console Namespace -- Replacements for SysPrintf.
// SysPrintf is depreciated -- We should phase these in over time.
namespace Console
{
	enum Colors
	{
		Color_Black = 0,
		Color_Red,
		Color_Green,
		Color_Yellow,
		Color_Blue,
		Color_Magenta,
		Color_Cyan,
		Color_White
	};

	// va_args version of WriteLn, mostly for internal use only.
	extern void __fastcall _WriteLn( Colors color, const std::string& fmt, va_list args );

	extern void Open();
	extern void Close();
	extern void SetTitle( const std::string& title );

	// Changes the active console color.
	// This color will be unset by calls to colored text methods
	// such as ErrorMsg and Notice.
	extern void __fastcall SetColor( Colors color );

	// Restores the console color to default (usually low-intensity white on Win32)
	extern void ClearColor();

	// The following Write functions return bool so that we can use macros to exclude
	// them from different buildypes.  The return values are always zero.

	// Writes a newline to the console.
	extern bool __fastcall Newline();

	// Writes an unformatted string of text to the console (fast!)
	// No newline is appended.
	extern bool __fastcall Write( const std::string& fmt );

	// Writes an unformatted string of text to the console (fast!)
	// A newline is automatically appended.
	extern bool __fastcall WriteLn( const std::string& fmt );

	// Writes an unformatted string of text to the console (fast!)
	// A newline is automatically appended, and the console color reset to default.
	extern bool __fastcall WriteLn( Colors color, const std::string& fmt );

	// Writes a line of colored text to the console, with automatic newline appendage.
	// The console color is reset to default when the operation is complete.
	extern bool WriteLn( Colors color, const std::string& fmt, VARG_PARAM dummy, ... );

	// Writes a line of colored text to the console (no newline).
	// The console color is reset to default when the operation is complete.
	extern bool Write( Colors color, const std::string& fmt, VARG_PARAM dummy, ... );
	extern bool Write( Colors color, const std::string& fmt );

	// Writes a formatted message to the console (no newline)
	extern bool Write( const std::string& fmt, VARG_PARAM dummy, ... );

	// Writes a formatted message to the console, with appended newline.
	extern bool WriteLn( const std::string& fmt, VARG_PARAM dummy, ... );

	// Displays a message in the console with red emphasis.
	// Newline is automatically appended.
	extern bool Error( const std::string& fmt, VARG_PARAM dummy, ... );
	extern bool Error( const std::string& fmt );

	// Displays a message in the console with yellow emphasis.
	// Newline is automatically appended.
	extern bool Notice( const std::string& fmt, VARG_PARAM dummy, ... );
	extern bool Notice( const std::string& fmt );

	// Displays a message in the console with yellow emphasis.
	// Newline is automatically appended.
	extern bool Status( const std::string& fmt, VARG_PARAM dummy, ... );
	extern bool Status( const std::string& fmt );
}

// Different types of message boxes that the emulator can employ from the friendly confines
// of it's blissful unawareness of whatever GUI it runs under. :)  All message boxes exhibit
// blocking behavior -- they promt the user for action and only return after the user has
// responded to the prompt.
namespace Msgbox
{
	// Pops up an alert Dialog Box with a singular "OK" button.
	// Always returns false.  Replacement for SysMessage.
	extern bool Alert( const std::string& fmt, VARG_PARAM dummy, ... );
	extern bool Alert( const std::string& fmt );

	// Pops up a dialog box with Ok/Cancel buttons.  Returns the result of the inquiry,
	// true if OK, false if cancel.
	extern bool OkCancel( const std::string& fmt, VARG_PARAM dummy, ... );
}

using Console::Color_Red;
using Console::Color_Green;
using Console::Color_Blue;
using Console::Color_Magenta;
using Console::Color_Cyan;
using Console::Color_Yellow;
using Console::Color_White;

//////////////////////////////////////////////////////////////
// Safe deallocation macros -- always check pointer validity (non-null)
// and set pointer to null on deallocation.

#define safe_delete( ptr ) \
	if( ptr != NULL ) { \
		delete ptr; \
		ptr = NULL; \
	}

#define safe_delete_array( ptr ) \
	if( ptr != NULL ) { \
		delete[] ptr; \
		ptr = NULL; \
	}

#define safe_free( ptr ) \
	if( ptr != NULL ) { \
		free( ptr ); \
		ptr = NULL; \
	}

#define safe_aligned_free( ptr ) \
	if( ptr != NULL ) { \
		_aligned_free( ptr ); \
		ptr = NULL; \
	}

#define SafeSysMunmap( ptr, size ) \
	if( ptr != NULL ) { \
		SysMunmap( (uptr)ptr, size ); \
		ptr = NULL; \
	}

//////////////////////////////////////////////////////////////
// Dev / Debug conditionals --
//   Consts for using if() statements instead of uglier #ifdef macros.
//   Abbrivated macros for dev/debug only consoles and mesgboxes.

#ifdef PCSX2_DEVBUILD

#	define DevCon Console
#	define DevMsg MsgBox
	static const bool IsDevBuild = true;

#else

#	define DevCon 0&&Console
#	define DevMsg 
	static const bool IsDevBuild = false;

#endif

#ifdef _DEBUG

#	define DbgCon Console
	static const bool IsDebugBuild = true;

#else

#	define DbgCon 0&&Console
	static const bool IsDebugBuild = false;

#endif

#ifdef PCSX2_VIRTUAL_MEM

struct PSMEMORYBLOCK
{
#ifdef _WIN32
    int NumberPages;
	uptr* aPFNs;
	uptr* aVFNs; // virtual pages that own the physical pages
#else
    int fd; // file descriptor
    char* pname; // given name
    int size; // size of allocated region
#endif
};

int SysPhysicalAlloc(u32 size, PSMEMORYBLOCK* pblock);
void SysPhysicalFree(PSMEMORYBLOCK* pblock);
int SysVirtualPhyAlloc(void* base, u32 size, PSMEMORYBLOCK* pblock);
void SysVirtualFree(void* lpMemReserved, u32 size);

// returns 1 if successful, 0 otherwise
int SysMapUserPhysicalPages(void* Addr, uptr NumPages, uptr* pblock, int pageoffset);

// call to enable physical page allocation
//BOOL SysLoggedSetLockPagesPrivilege ( HANDLE hProcess, BOOL bEnable);

#endif

//////////////////////////////////////////////////////////////////
// Handy little class for allocating a resizable memory block, complete with
// exception-based error handling and automatic cleanup.

template< typename T >
class MemoryAlloc : public NoncopyableObject
{
public:
	static const int DefaultChunkSize = 0x1000 * sizeof(T);

public: 
	const std::string Name;		// user-assigned block name
	int ChunkSize;

protected:
	T* m_ptr;
	int m_size;	// size of the allocation of memory

public:
	virtual ~MemoryAlloc()
	{
		safe_free( m_ptr );
	}

	explicit MemoryAlloc( const std::string& name="Unnamed" ) : 
	  Name( name )
	, ChunkSize( DefaultChunkSize )
	, m_ptr( NULL )
	, m_size( 0 )
	{
	}

	explicit MemoryAlloc( int initialSize, const std::string& name="Unnamed" ) : 
	  Name( name )
	, ChunkSize( DefaultChunkSize )
	, m_ptr( (T*)malloc( initialSize * sizeof(T) ) )
	, m_size( initialSize )
	{
		if( m_ptr == NULL )
			throw Exception::OutOfMemory();
	}

	// Returns the size of the memory allocation, as according to the array type.
	int GetLength() const { return m_size; }
	// Returns the size of the memory allocation in bytes.
	int GetSizeInBytes() const { return m_size * sizeof(T); }

	// Ensures that the allocation is large enough to fit data of the
	// amount requested.  The memory allocation is not resized smaller.
	void MakeRoomFor( int blockSize )
	{
		std::string temp;
		
		if( blockSize > m_size )
		{
			const uint newalloc = blockSize + ChunkSize;
			m_ptr = (T*)realloc( m_ptr, newalloc * sizeof(T) );
			if( m_ptr == NULL )
			{
				throw Exception::OutOfMemory(
					"Out-of-memory on block re-allocation. "
					"Old size: " + to_string( m_size ) + " bytes, "
					"New size: " + to_string( newalloc ) + " bytes"
				);
			}
			m_size = newalloc;
		}
	}

	// Gets a pointer to the requested allocation index.
	// DevBuilds : Throws std::out_of_range() if the index is invalid.
	T *GetPtr( uint idx=0 ) { return _getPtr( idx ); }
	const T *GetPtr( uint idx=0 ) const { return _getPtr( idx ); }

	// Gets an element of this memory allocation much as if it were an array.
	// DevBuilds : Throws std::out_of_range() if the index is invalid.
	T& operator[]( int idx ) { return *_getPtr( (uint)idx ); }
	const T& operator[]( int idx ) const { return *_getPtr( (uint)idx ); }

	virtual MemoryAlloc<T>& Clone() const
	{
		MemoryAlloc<T>* retval = new MemoryAlloc<T>( m_size );
		memcpy( retval->GetPtr(), m_ptr, sizeof(T) * m_size );
		return *retval;
	}

protected:
	T* _getPtr( uint i ) const
	{
		if( i >= (uint)m_size )
		{
			throw std::out_of_range(
				"Index out of bounds on MemoryAlloc: " + Name + 
				" (index=" + to_string(i) + 
				", size=" + to_string(m_size) + ")"
			);
		}
		return &m_ptr[i];
	}

};

#endif /* __SYSTEM_H__ */