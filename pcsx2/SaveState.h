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

#ifndef _SAVESTATE_H_
#define _SAVESTATE_H_

#include <zlib.h>

// This shouldn't break Win compiles, but it does.
#ifdef __LINUX__
#include "PS2Edefs.h"
#endif

// Savestate Versioning!
//  If you make changes to the savestate version, please increment the value below.

#ifdef PCSX2_VIRTUAL_MEM
static const u32 g_SaveVersion = 0x7a300010;
#else
static const u32 g_SaveVersion = 0x8b400000;
#endif

// This class provides the base API for both loading and saving savestates.
// Normally you'll want to use one of the four "functional" derrived classes rather
// than this class directly: gzLoadingState, gzSavingState (gzipped disk-saved
// states), and memLoadingState, memSavingState (uncompressed memory states).
class SaveState
{
protected:
	u32 m_version;		// version of the savestate being loaded.

public:
	SaveState( const char* msg, const char* destination );
	virtual ~SaveState() { }

	static void GetFilename( char* dest, int slot );

	// Gets the version of savestate that this object is acting on.
	u32 GetVersion() const { return m_version; }

	// Loads or saves the entire emulation state.
	// Note: The Cpu state must be reset, and plugins *open*, prior to Defrosting
	// (loading) a state!
	void FreezeAll();

	// Loads or saves an arbitrary data type.  Usable on atomic types, structs, and arrays.  
	// For dynamically allocated pointers use FreezeMem instead.
	template<typename T>
	void Freeze( T& data )
	{
		FreezeMem( &data, sizeof( T ) );
	}

	// Loads or saves a plugin.  Plugin name is for console logging purposes.
	virtual void FreezePlugin( const char* name,s32(CALLBACK* freezer)(int mode, freezeData *data) )=0; 

	// Loads or saves a memory block.
	virtual void FreezeMem( void* data, int size )=0;

	// Returns true if this object is a StateSaving type object.
	virtual bool IsSaving() const=0;

	// Returns true if this object is a StateLoading type object.
	bool IsLoading() const { return !IsSaving(); }

	// note: gsFreeze() needs to be public because of the GSState recorder.

public:
	void gsFreeze();

protected:

	// Load/Save functions for the various components of our glorious emulator!

	void rcntFreeze();
	void vu0Freeze();
	void vu1Freeze();
	void vif0Freeze();
	void vif1Freeze();
	void sifFreeze();
	void ipuFreeze();

	void sioFreeze();
	void cdrFreeze();
	void cdvdFreeze();
	void psxRcntFreeze();
	void sio2Freeze();

};

/////////////////////////////////////////////////////////////////////////////////
// Class Declarations for Savestates using zlib

class gzBaseStateInfo : public SaveState
{
protected:
	const char* m_filename;
	gzFile m_file;		// used for reading/writing disk saves

public:
	gzBaseStateInfo( const char* msg, const char* filename );

	virtual ~gzBaseStateInfo();
};

class gzSavingState : public gzBaseStateInfo
{
public:
	virtual ~gzSavingState() {}
	gzSavingState( const char* filename ) ;
	void FreezePlugin( const char* name, s32(CALLBACK *freezer)(int mode, freezeData *data) );
	void FreezeMem( void* data, int size );
	bool IsSaving() const { return true; }
};

class gzLoadingState : public gzBaseStateInfo
{
public:
	virtual ~gzLoadingState();
	gzLoadingState( const char* filename ); 

	void FreezePlugin( const char* name, s32(CALLBACK *freezer)(int mode, freezeData *data) );
	void FreezeMem( void* data, int size );
	bool IsSaving() const { return false; }
	bool Finished() const { return !!gzeof( m_file ); }
};

//////////////////////////////////////////////////////////////////////////////////

class memBaseStateInfo : public SaveState
{
protected:
	MemoryAlloc& m_memory;
	int m_idx;		// current read/write index of the allocation

public:
	virtual ~memBaseStateInfo() { }
	memBaseStateInfo( MemoryAlloc& memblock, const char* msg );
};

class memSavingState : public memBaseStateInfo
{
protected:
	static const int ReallocThreshold = 0x200000;	// 256k reallocation block size.
	static const int MemoryBaseAllocSize = 0x02a00000;  // 42 meg base alloc

public:
	virtual ~memSavingState() { }
	memSavingState( MemoryAlloc& save_to );
	
	void FreezePlugin( const char* name, s32(CALLBACK *freezer)(int mode, freezeData *data) );
	// Saving of state data to a memory buffer
	void FreezeMem( void* data, int size );
	bool IsSaving() const { return true; }
};

class memLoadingState : public memBaseStateInfo
{
public:
	virtual ~memLoadingState();
	memLoadingState(MemoryAlloc& load_from );

	void FreezePlugin( const char* name, s32(CALLBACK *freezer)(int mode, freezeData *data) );
	// Loading of state data from a memory buffer...
	void FreezeMem( void* data, int size );
	bool IsSaving() const { return false; }
};

#endif