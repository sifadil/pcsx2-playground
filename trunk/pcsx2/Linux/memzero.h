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

#ifndef _LNX_MEMZERO_H_
#define _LNX_MEMZERO_H_

// This header contains non-optimized implementation of memzero_ptr and memset8_obj,
// memset16_obj, etc.

template< u32 data, typename T >
static __forceinline void memset32_obj( T& obj )
{
	// this function works on 32-bit aligned lengths of data only.
	// If the data length is not a factor of 32 bits, the C++ optimizing compiler will
	// probably just generate mysteriously broken code in Release builds. ;)

	jASSUME( (sizeof(T) & 0x3) == 0 );

	u32* dest = (u32*)&obj;
	for( int i=sizeof(T)>>2; i; --i, ++dest )
		*dest = data;
}

template< uint size >
static __forceinline void memzero_ptr( void* dest )
{
	memset( dest, 0, size );
}

template< typename T >
static __forceinline void memzero_obj( T& obj )
{
	memset( &obj, 0, sizeof( T ) );
}

template< u8 data, typename T >
static __forceinline void memset8_obj( T& obj )
{
	// Aligned sizes use the optimized 32 bit inline memset.  Unaligned sizes use memset.
	if( (sizeof(T) & 0x3) != 0 )
		memset( &obj, data, sizeof( T ) );
	else
		memset32_obj<data + (data<<8) + (data<<16) + (data<<24)>( obj );
}

template< u16 data, typename T >
static __forceinline void memset16_obj( T& obj )
{
	if( (sizeof(T) & 0x3) != 0 )
		_memset_16_unaligned( &obj, data, sizeof( T ) )
	else
		memset32_obj<data + (data<<16)>( obj );
}

#endif
