#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)

m4_define([required_libxml_version], [2.6.12])
m4_define([required_tidy_version], [Oct/2004])

AC_INIT([Bitextor], [1.0], [esvillamil@dlsi.ua.es])
AC_CONFIG_SRCDIR(bitextor/collectormain.cpp)

GENERIC_LIBRARY_NAME=bitextor

# Release versioning
GENERIC_MAJOR_VERSION=1
GENERIC_MINOR_VERSION=0
GENERIC_MICRO_VERSION=0

# API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_API_VERSION)

# Shared library versioning
GENERIC_LIBRARY_VERSION=0:0:0
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                                  or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed

AC_SUBST(GENERIC_LIBRARY_VERSION)
PACKAGE=$GENERIC_LIBRARY_NAME
AC_SUBST(GENERIC_LIBRARY_NAME)

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION.$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

VERSION=$GENERIC_VERSION

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
#AM_INIT_AUTOMAKE([gnits])

# Checks for programs.

AC_MSG_CHECKING([Compilation architecture: PPC, i686, Other])
ARCH=$(/bin/arch)
if test x$ARCH = xppc
then
  AC_MSG_RESULT([PowerPC])
  CFLAGS="$CFLAGS -Wall -ansi -mpowerpc -maltivec -fno-pic -fomit-frame-pointer"
  CXXFLAGS="$CXXFLAGS -Wall -ansi -mpowerpc -maltivec -fno-pic -fomit-frame-pointer"
else
  if test x$ARCH = xi686
  then
    AC_MSG_RESULT([i686])
    CFLAGS="$CFLAGS -Wall -ansi -march=i686 -O3 -fno-pic -fomit-frame-pointer"
    CXXFLAGS="$CXXFLAGS -Wall -ansi -march=i686 -O3 -fno-pic \
              -fomit-frame-pointer"
  else
    AC_MSG_RESULT([Other])
    CFLAGS="$CFLAGS -Wall -ansi -O3"
    CXXFLAGS="$CXXFLAGS -Wall -ansi -O3"
  fi
fi

AC_PROG_CC
AC_PROG_CXX
AM_SANITY_CHECK
AC_LANG_CPLUSPLUS

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

PKG_CHECK_MODULES(PARALLELCOLLECTOR, [dnl
  libxml-2.0 >= required_libxml_version])

# Checks for libraries.
AC_CHECK_LIB(xml2, xmlReaderForFile)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h stddef.h fcntl.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_HEADER_SYS_WAIT
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_FORK
AC_FUNC_MKTIME


AC_CHECK_DECLS([fread_unlocked, fwrite_unlocked, fgetc_unlocked, fputc_unlocked, fputs_unlocked])
                
AC_CHECK_FUNCS([setlocale strdup getopt_long memmove mkdir pow sqrt strtol])

AC_OUTPUT([Makefile bitextor/Makefile])
