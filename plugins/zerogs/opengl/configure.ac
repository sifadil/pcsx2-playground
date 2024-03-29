AC_INIT(ZeroGSogl,0.96.2,zerofrog@gmail.com)

AM_INIT_AUTOMAKE(ZeroGSogl,0.96.2)

AC_PROG_CC([gcc g++ cl KCC CC cxx cc++ xlC aCC c++])
AC_PROG_CXX([gcc g++ cl KCC CC cxx cc++ xlC aCC c++])
AC_PROG_CPP([gcc g++ cl KCC CC cxx cc++ xlC aCC c++])
AM_PROG_CC_C_O

AC_PROG_INSTALL
AC_PROG_RANLIB

dnl necessary for compiling assembly
AM_PROG_AS

ZEROGS_CURRENT=0
ZEROGS_REVISION=96
ZEROGS_AGE=2
ZEROGS_SONAME=libZeroGSogl.so.[$ZEROGS_CURRENT].[$ZEROGS_REVISION].[$ZEROGS_AGE]
ZEROGS_RELEASE=[$ZEROGS_CURRENT].[$ZEROGS_REVISION].[$ZEROGS_AGE]

AC_SUBST(ZEROGS_CURRENT)
AC_SUBST(ZEROGS_REVISION)
AC_SUBST(ZEROGS_AGE)
AC_SUBST(ZEROGS_RELEASE)
AC_SUBST(ZEROGS_SONAME)

CFLAGS=" -I/opt/cg/include -L/opt/cg/lib "
CPPFLAGS=" -I/opt/cg/include -L/opt/cg/lib "
CXXFLAGS=" -I/opt/cg/include -L/opt/cg/lib "

dnl Check for debug build
AC_MSG_CHECKING(debug build)
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [debug build]),
 debug=$enableval,debug=no)
if test "x$debug" == xyes
then
 AC_DEFINE(_DEBUG,1,[_DEBUG])
 CFLAGS+="-g "
 CPPFLAGS+="-g -fpermissive "
 CXXFLAGS+="-g- fpermissive "
else
 AC_DEFINE(NDEBUG,1,[NDEBUG])
 CFLAGS+="-O3 -fomit-frame-pointer "
 CPPFLAGS+="-O3 -fomit-frame-pointer "
 CXXFLAGS+="-O3 -fomit-frame-pointer -fpermissive "
fi
AC_MSG_RESULT($debug)

AC_CHECK_FUNCS([ _aligned_malloc _aligned_free ], AC_DEFINE(HAVE_ALIGNED_MALLOC))

dnl Check for dev build
AC_MSG_CHECKING(for development build...)
AC_ARG_ENABLE(devbuild, AC_HELP_STRING([--enable-devbuild], [Special Build for developers that simplifies testing and adds extra checks]),
 devbuild=$enableval,devbuild=no)
if test "x$devbuild" == xno
then
 AC_DEFINE(RELEASE_TO_PUBLIC,1,[RELEASE_TO_PUBLIC])
fi
AC_MSG_RESULT($devbuild)
AM_CONDITIONAL(RELEASE_TO_PUBLIC, test x$devbuild = xno)

dnl Check for recompilation
AC_MSG_CHECKING(check for sse2...)
AC_ARG_ENABLE(sse2, AC_HELP_STRING([--enable-sse2], [Enable sse2 instructions]),
 sse2=$enableval,sse2=no)
if test "x$sse2" == xyes
then
 AC_DEFINE(ZEROGS_SSE2,1,[ZEROGS_SSE2])
fi
AC_MSG_RESULT($sse2)
AM_CONDITIONAL(SSE2, test x$sse2 = xyes)

dnl Check for 64bit CPU
AC_MSG_CHECKING(for a x86-64 CPU)
dnl if test "$build_os" == "target_os"
dnl then
AC_TRY_RUN([
int main()
{
int a = 0;
int*pa = &a;
asm(".intel_syntax\n"
	"mov %%rax, %0\n"
    "mov %%eax, [%%rax]\n"
    ".att_syntax\n"
    : : "r"(pa) : "%rax");
return 0;
}
],cpu64=yes,cpu64=no,)
dnl else
dnl cpu64=no
dnl fi
if test "x$cpu64" == xyes
then
AC_DEFINE(__x86_64__,1,[__x86_64__])
fi
AC_MSG_RESULT($cpu64)
AM_CONDITIONAL(X86_64, test x$cpu64 = xyes)

dnl gtk
AC_MSG_CHECKING(gtk2+)
AC_CHECK_PROG(GTK_CONFIG, pkg-config, pkg-config)
LIBS+=$(pkg-config --libs gtk+-2.0)

dnl bindir = pcsx2exe

dnl assuming linux environment
so_ext=".so.$ZEROGS_RELEASE"
SHARED_LDFLAGS="-shared"
AC_SUBST(so_ext)
AC_SUBST(SHARED_LDFLAGS)

AC_MSG_CHECKING(OpenGL)
AC_CHECK_HEADERS([GL/gl.h GL/glu.h GL/glext.h],,,
      [[#if HAVE_GL_GL_H
         #include <GL/gl.h>
         #endif
  	   #if HAVE_GL_GLU_H
         #include <GL/glu.h>
         #endif
       ]])
AC_CHECK_LIB(GL,main,[LIBS="$LIBS -lGL"])
AC_CHECK_LIB(GLU,main,[LIBS="$LIBS -lGLU"])
AC_CHECK_LIB(GLEW,main,[LIBS="$LIBS -lGLEW"])
AC_MSG_CHECKING(Cg)
#AC_CHECK_HEADERS([Cg/cg.h Cg/cgGL.h])

AC_CHECK_LIB(jpeg,main,[LIBS="$LIBS -ljpeg"])
AC_CHECK_LIB(pthread,main,[LIBS="$LIBS -lpthread"])
AC_CHECK_LIB(stdc++,main,[LIBS="$LIBS -lstdc++"])
AC_CHECK_LIB(z,main,[LIBS="$LIBS -lz"])
AC_CHECK_LIB(dl,main,[LIBS="$LIBS -ldl"])
AC_CHECK_LIB(Xxf86vm,main,[LIBS="$LIBS -lXxf86vm"])

dnl AC_CHECK_LIB(Cg,cgCreateContext,[LIBS="$LIBS -lCg"])
dnl AC_CHECK_LIB(CgGL,cgGLEnableProfile,[LIBS="$LIBS -lCgGL"])
LIBS+=" -lCg -lCgGL"

AC_OUTPUT([
 Makefile
 Linux/Makefile
 ])

echo "Configuration:"
echo "  Target system type:    $target"
echo "  x86-64 build?	       $cpu64"
echo "  Debug build?	       $debug"
echo "  Dev build?	           $devbuild"
echo "  SSE2 enabled?		   $sse2"
