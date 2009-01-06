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

namespace Path
{

#ifdef WIN32
static const char Separator = '\\';
#else
static const char Separator = '/';
#endif

bool Exists( const string& path )
{
	struct stat sbuf;
	return stat( path.c_str(), &sbuf ) == 0;
}

// This function returns false if the path does not exist, or if the path exists and
// is a file.
bool isDirectory( const string& path )
{
	struct stat sbuf;
	if( stat( path.c_str(), &sbuf ) == -1 ) return false;
	return !!(sbuf.st_mode & _S_IFDIR);
}

// This function returns false if the path does not exist, or if the path exists and
// is a directory.
bool isFile( const string& path )
{
	struct stat sbuf;
	if( stat( path.c_str(), &sbuf ) == -1 ) return false;
	return !!(sbuf.st_mode & _S_IFREG);
}

// Returns the length of the file.
// returns -1 if the file is not found.
int getFileSize( const string& path )
{
	struct stat sbuf;
	if( stat( path.c_str(), &sbuf ) == -1 ) return -1;
	return sbuf.st_size;
}

bool isRooted( const string& path )
{
	// if the first character is a backslash or period, or the second character
	// a colon, it's a safe bet we're rooted.

	if( path[0] == 0 ) return FALSE;
#ifdef WIN32
	return (path[0] == '/') || (path[0] == '\\') || (path[1] == ':');
#else
	return (path[0] == PathSeparator);
#endif
}

// Concatenates two pathnames together, inserting delimiters (backslash on win32)
// as needed! Assumes the 'dest' is allocated to at least g_MaxPath length.
void Combine( string& dest, const string& srcPath, const string& srcFile )
{
	int pathlen, guesslen;

	if( srcFile.empty() )
	{
		// No source filename?  Return the path unmodified.
		dest = srcPath;
		return;
	}

	if( isRooted( srcFile ) || srcPath.empty() )
	{
		// No source path?  Or source filename is rooted?
		// Return the filename unmodified.
		dest = srcFile;
		return;
	}

	// strip off the srcPath's trailing backslashes (if any)
	// Note: The win32 build works better if I check for both forward and backslashes.
	// This might be a problem on Linux builds or maybe it doesn't matter?

	pathlen = srcPath.length();
	while( pathlen > 0 && ((srcPath[pathlen-1] == '\\') || (srcPath[pathlen-1] == '/')) )
		--pathlen;

	// Concatenate strings:
	guesslen = pathlen + srcFile.length() + 2;

	if( guesslen >= g_MaxPath )
		throw Exception::PathTooLong();

	// Concatenate!

	dest.assign( srcPath.begin(), srcPath.begin()+pathlen );
	dest += Separator;
	dest += srcFile;
}

// Replaces the extension of the file with the one given.
void ReplaceExtension( string& dest, const string& src, const string& ext )
{
	int pos = src.find_last_of( '.' );
	if( pos == 0 )
		dest = src;
	else
		dest.assign( src.begin(), src.begin()+pos );

	if( !ext.empty() )
	{
		dest += '.';
		dest += ext;
	}
}

}