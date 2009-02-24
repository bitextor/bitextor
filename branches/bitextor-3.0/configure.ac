#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)

m4_define([required_libtagaligner_version], [3.0.0])
m4_define([required_libxml_version], [2.6.17])
m4_define([required_libenca_version], [1.9])
AC_INIT([bitextor], [2.1rc1], [miquelespla@gmail.com])
AC_CONFIG_SRCDIR([src/Bitext.h])
AC_CONFIG_HEADER([config.h])

GENERIC_LIBRARY_NAME=bitextor

# Release versioning
GENERIC_MAJOR_VERSION=2
GENERIC_MINOR_VERSION=1
GENERIC_MICRO_VERSION=rc1

# API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_API_VERSION)

# Shared library versioning
GENERIC_LIBRARY_VERSION=2:0:0
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

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION$GENERIC_MICRO_VERSION
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

VERSION=$GENERIC_VERSION

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_CHECK_TOOL(HTTRACK, [httrack], [false])
if test "${WGET}" = false; then
	AC_MSG_ERROR([
 
	Cannot find the website mirroring application httrack.
 
	Install httrack and then re-run this configuration script.
	Try ``./configure --help'' for a help.
 
	])
fi


dnl  
dnl check libxml-2.0 and libtagaligner-3.0
dnl
PKG_CHECK_MODULES(BITEXTOR, [dnl
                  libtagaligner-3.0 >= required_libtagaligner_version dnl
                  enca >= required_libenca_version dnl
                  libxml-2.0 >= required_libxml_version])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-ltextcat':
AC_CHECK_LIB([textcat], [main], [], [AC_MSG_ERROR([
 
Cannot find the text langage guesser LibTextCat library.
 
Install the text langage guesser LibTextCat library and then re-run this configuration script.
Try ``./configure --help'' for a help.
 
])])

# FIXME: Replace `main' with a function in `-ltidy':
AC_CHECK_LIB([tidy], [main], [], [AC_MSG_ERROR([
 
Cannot find the HTML files cleaner LibTidyHTML library.
 
Install the HTML files cleaner TidyHTML library and then re-run this configuration script.
Try ``./configure --help'' for a help.
 
])])


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stdio.h stdlib.h string.h stddef.h ctype.h time.h])

AC_CHECK_HEADER(tagaligner/Aligner.h)


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([memset mkdir strndup])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
