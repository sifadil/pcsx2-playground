AUTOMAKE_OPTIONS = foreign
INCLUDES = $(shell pkg-config --cflags gtk+-2.0) -I@srcdir@/../   -I@srcdir@/../common/

bin_PROGRAMS = pcsx2

# the application source, library search path, and link libraries
pcsx2_SOURCES = Pref.cpp  interface.c  ConfigDlg.cpp DebugDlg.cpp GtkGui.cpp  LnxMain.cpp LnxConsole.cpp LnxThreads.cpp support.c

pcsx2_LDFLAGS =

pcsx2_DEPENDENCIES = ../libpcsx2.a ../IPU/libIPU.a ../IPU/mpeg2lib/libmpeg2IPU.a ../RDebug/libRDebug.a ../tinyxml/libtinyxml.a
pcsx2_DEPENDENCIES += ../x86/libx86recomp.a ../x86/ix86/libix86.a
pcsx2_DEPENDENCIES += ../DebugTools/libDebugTools.a

pcsx2_LDADD = ../libpcsx2.a ../IPU/libIPU.a ../IPU/mpeg2lib/libmpeg2IPU.a ../RDebug/libRDebug.a ../tinyxml/libtinyxml.a
pcsx2_LDADD += ../x86/libx86recomp.a ../x86/ix86/libix86.a
pcsx2_LDADD += ../DebugTools/libDebugTools.a