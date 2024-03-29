AC_INIT(pcsx2,0.9.5,zerofrog@gmail.com)
AM_INIT_AUTOMAKE(pcsx2,0.9.5)

AC_PROG_CC([gcc g++ cl KCC CC cxx cc++ xlC aCC c++])
AC_PROG_CXX([gcc g++ cl KCC CC cxx cc++ xlC aCC c++])
AC_PROG_CPP([gcc g++ cl KCC CC cxx cc++ xlC aCC c++])

AC_PROG_RANLIB

dnl necessary for compiling assembly
AM_PROG_AS

svnrev="`svn info | grep Revision:`"

if test -n "${svnrev}" ; then
  AC_REVISION("${svnrev}") 
  AC_DEFINE_UNQUOTED(SVN_REV,"${svnrev}",
    [Define to be the subversion revision number])
else
  AC_REVISION([$Revision: 0 $])
  AC_DEFINE(SVN_REV,"$Revision: 0 $",
    [Define to be the subversion revision number])
fi

AC_ARG_ENABLE(customcflags, AC_HELP_STRING([--enable-customcflags], [Use custom CFLAGS]),
customcflags=$enableval,customcflags=no)

if test "x$customcflags" == xno
then
CFLAGS=
CPPFLAGS=
CXXFLAGS=
CCASFLAGS=
else
DEBUG_FLAGS=" -O0 -g "
fi

WARNING_FLAGS="-Wall -Wno-format -Wno-unused-value"
#Pcsx2 now crashes if --fomit-frame-pointer is enabled and MTGS is off 
NORMAL_FLAGS=" -pipe -O3 -fno-omit-frame-pointer -msse "
DEBUG_FLAGS+=" -g -msse ${WARNING_FLAGS} "

dnl Check for debug build
AC_MSG_CHECKING(debug build)
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [debug build]),
 debug=$enableval,debug=no)
if test "x$debug" == xyes
then
 AC_DEFINE(_DEBUG,1,[_DEBUG])
 CFLAGS+=" ${DEBUG_FLAGS}  -fpermissive -Xlinker -zmuldefs "
 CXXFLAGS+=" ${DEBUG_FLAGS} -fpermissive -Xlinker -zmuldefs "
 CCASFLAGS+=" -D_DEBUG ${DEBUG_FLAGS} "
 MYOBJDIR="Debug"
else
 AC_DEFINE(NDEBUG,1,[NDEBUG])
 CFLAGS+=" ${NORMAL_FLAGS}"
 CXXFLAGS+=" ${NORMAL_FLAGS} -fpermissive -Xlinker -zmuldefs "
 MYOBJDIR="Release"
fi
AC_MSG_RESULT($debug)

AC_CHECK_FUNCS([ _aligned_malloc _aligned_free ], AC_DEFINE(HAVE_ALIGNED_MALLOC))

dnl Check for dev build
AC_MSG_CHECKING(for development build)
AC_ARG_ENABLE(devbuild, AC_HELP_STRING([--enable-devbuild], [Special Build for developers that simplifies testing and adds extra checks]),
 devbuild=$enableval,devbuild=no)
if test "x$devbuild" == xyes
then
 AC_DEFINE(PCSX2_DEVBUILD,1,[PCSX2_DEVBUILD])
 MYOBJDIR=$MYOBJDIR"Dev"
fi
AC_MSG_RESULT($devbuild)

AC_MSG_CHECKING(force sse3 instructions)
AC_ARG_ENABLE(sse3, AC_HELP_STRING([--enable-sse3], [Forces sse3 detection on CPUs]),
 sse3=$enableval,sse3=no)
if test "x$sse3" == xyes
then
 AC_DEFINE(PCSX2_FORCESSE3,1,[PCSX2_DEVBUILD])
fi
AC_MSG_RESULT(sse3)

AC_MSG_CHECKING(force sse4 instructions)
AC_ARG_ENABLE(sse4, AC_HELP_STRING([--enable-sse4], [Forces sse4 detection on CPUs]),
 sse4=$enableval,sse4=no)
if test "x$sse4" == xyes
then
 AC_DEFINE(PCSX2_FORCESSE4,1,[PCSX2_DEVBUILD])
fi
AC_MSG_RESULT(sse4)

dnl gtk
AC_MSG_CHECKING(gtk+)
AC_CHECK_PROG(GTK_CONFIG, pkg-config, pkg-config)
LIBS+=$(pkg-config --libs gtk+-2.0 gthread-2.0)

dnl AC_CHECK_LIB(pthread,main,[LIBS="$LIBS -lpthread"])
AC_CHECK_LIB(stdc++,main,[LIBS="$LIBS -lstdc++"])
AC_CHECK_LIB(z,main,[LIBS="$LIBS -lz"])

AC_ARG_ENABLE(nls, AC_HELP_STRING([--disable-nls], [Disable nls support]),
nls=$enableval,nls=yes)
if test "x$nls" == xyes
then
AC_CHECK_HEADER([libintl.h], [AC_DEFINE(ENABLE_NLS,1,[__x86_64__])])
fi

AC_ARG_ENABLE(local-inis, AC_HELP_STRING([--enable-local-inis], [Enable local plugin inis]),
localinis=$enableval, localinis=no)
if test "x$localinis" == xyes
then
AC_DEFINE(LOCAL_PLUGIN_INIS)
fi

AC_OUTPUT([
 Makefile
 DebugTools/Makefile
 Linux/Makefile
 IPU/Makefile
 IPU/mpeg2lib/Makefile
 RDebug/Makefile
 tinyxml/Makefile
 x86/Makefile
 x86/ix86/Makefile
 3rdparty/zlib/Makefile
 ])

dnl bindir = pcsx2exe

echo "Configuration:"
echo "  Target system type:    $target"
echo "  Debug build?	       $debug"
echo "  Dev build?	           $devbuild"
echo "  Force sse3?	           $sse3"
echo " nls support?   $nls"
echo " local plugin inis? $localinis"
echo " custom cflags? $customcflags"
