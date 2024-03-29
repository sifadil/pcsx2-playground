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

#ifndef PCSX2_COROUTINE_LIB
#define PCSX2_COROUTINE_LIB

// low level coroutine library
typedef void *coroutine_t;

coroutine_t so_create(void (*func)(void *), void *data, void *stack, int size);
void so_delete(coroutine_t coro);

#ifdef __LINUX__
extern "C" {
#endif
void so_call(coroutine_t coro);
void so_resume(void);
void so_exit(void);
#ifdef __LINUX__
}
#endif
#endif
