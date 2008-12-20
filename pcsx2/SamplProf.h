#ifndef _SAMPLPROF_H_
#define _SAMPLPROF_H_

#include "common.h"

// The profiler does not have a Linux version yet.
// So for now we turn it into duds for non-Win32 platforms.

#if !defined( _DEBUG ) || !defined( WIN32 )

void ProfilerInit();
void ProfilerTerm();
void ProfilerSetEnabled(bool Enabled);
void ProfilerRegisterSource(const char* Name,void* buff,u32 sz);
void ProfilerRegisterSource(const char* Name,void* function);

#else

// Disables the profiler in Debug builds.
// Profiling info in debug builds isn't much use anyway and the console
// popups are annoying when you're trying to trace debug logs and stuff.

#define ProfilerInit()
#define ProfilerTerm()
#define ProfilerSetEnabled 0&&
#define ProfilerRegisterSource 0&&
#define ProfilerRegisterSource 0&&

#endif

#endif