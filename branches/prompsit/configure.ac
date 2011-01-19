#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)

#m4_define([required_lttoolbox_version], [3.2.0])
#m4_define([required_libxml_version], [2.6.17])
#m4_define([required_libpcre_version], [6.4])
#m4_define([required_pkg_config_version], [0.15])

AC_INIT([bitextor], [4.0.0], [jorgeferrandez@gmail.com])
AC_CONFIG_HEADER([bitextor_config.h])
#AC_CONFIG_HEADER([])
AC_CANONICAL_SYSTEM

PACKAGE=bitextor
VERSION=4.0.0

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

# Checks for programs.

AC_MSG_CHECKING([Compilation architecture: PPC, i686, x86_64, Other])
if test x$(which arch) = x
then ARCH=$($(which uname) -m)
else ARCH=$($(which arch))
fi

case "${target_os}" in
  mingw*)
    MinGW=yes
  ;;
  cygwin*)
    Cygwin=yes
  ;;
esac

if test x$ARCH = xppc
then
  AC_MSG_RESULT([PowerPC])
  CFLAGS="$CFLAGS -Wall -ansi -fomit-frame-pointer"
  CXXFLAGS="$CXXFLAGS -Wall -ansi -fomit-frame-pointer"
else
  if test x$MinGW = xyes
  then 
    AC_MSG_RESULT([MinGW])
    CFLAGS="$CFLAGS -Wall -march=i686 -O3 -DMINGW -fomit-frame-pointer \
	        -funroll-loops -I/include -L/lib"
    CXXFLAGS="$CXXFLAGS -Wall -march=i686 -O3 \
            -fomit-frame-pointer -funroll-loops -I/include -L/lib"
    CPPFLAGS="$CPPFLAGS -I/include"
  else
    if test x$Cygwin = xyes
    then 
      AC_MSG_RESULT([Cygwin])
      CFLAGS="$CFLAGS -Wall -march=i686 -O3 -fomit-frame-pointer \
           -funroll-loops"
      CXXFLAGS="$CXXFLAGS -Wall -march=i686 -O3 \
              -fomit-frame-pointer -funroll-loops"
      CPPFLAGS="$CPPFLAGS"
    else
      if test x$ARCH = xi686
      then
        AC_MSG_RESULT([i686])
        CFLAGS="$CFLAGS -Wall -ansi -march=i686 -O3 -fomit-frame-pointer -funroll-loops"
        CXXFLAGS="$CXXFLAGS -Wall -ansi -march=i686 -O3 \
                -fomit-frame-pointer -funroll-loops"
      else
        if test x$ARCH = xx86_64
        then
          AC_MSG_RESULT([x86_64])
          CFLAGS="-Wall -ansi -O3 -mtune=nocona -fomit-frame-pointer -funroll-loops $CFLAGS"
          CXXFLAGS="-Wall -ansi -O3 -mtune=nocona \
                  -fomit-frame-pointer -funroll-loops $CXXFLAGS"
        else
          AC_MSG_RESULT([Other])
          CFLAGS="-Wall -ansi -O3 $CFLAGS"
          CXXFLAGS="-Wall -ansi -O3 $CXXFLAGS"
        fi
      fi
    fi    
  fi
fi

AC_PROG_CXX
AC_PROG_CC
AM_SANITY_CHECK
AC_LANG_CPLUSPLUS

AC_ARG_ENABLE(debug,
              [  --enable-debug    Enable "-g -Wall" compiler options], 
              [CXXFLAGS="-g -Wall"; CFLAGS="-g -Wall"])

AC_ARG_ENABLE(profile,
              [  --enable-profile  Enable "-pg -g -Wall" compiler options],
              [CXXFLAGS="-pg -g -Wall"; CFLAGS="-pg -g -Wall"; LDFLAGS="-pg"])

#AC_PATH_PROG(XMLLINT, xmllint, no)
#if test x$ac_cv_path_XMLLINT = x
#then
#  AC_MSG_ERROR([You don't have xmllint installed.])
#fi
#if test x$ac_cv_path_XMLLINT = xno
#then
#  AC_MSG_ERROR([You don't have xmllint installed.])
#fi
 
#  AC_PATH_PROG(XSLTPROC, xsltproc, no)
#  if test x$ac_cv_path_XSLTPROC = x
#  then
#    AC_MSG_ERROR([You don't have xsltproc installed.])
#  fi
#  if test x$ac_cv_path_XSLTPROC = xno
#  then
#    AC_MSG_ERROR([You don't have xsltproc installed.])
#  fi

AC_PATH_PROG(BASH, bash, no)
if test x$ac_cv_path_BASH = x
then
  AC_MSG_ERROR([You don't have bash installed.])
fi
if test x$ac_cv_path_BASH = xno
then
  AC_MSG_ERROR([You don't have bash installed.])
fi

#AC_PATH_PROG(FLEX, flex, no)
#if test x$ac_cv_path_FLEX = x
#then
#  AC_MSG_ERROR([You don't have flex installed.])
#fi
#if test x$ac_cv_path_FLEX = xno
#then
#  AC_MSG_ERROR([You don't have flex installed.])
#fi

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test x$ac_cv_path_PKG_CONFIG = x
then
  AC_MSG_ERROR([You don't have pkg-config installed.])
fi
if test x$ac_cv_path_PKG_CONFIG = xno
then
  AC_MSG_ERROR([You don't have pkg-config installed.])
fi

AC_PATH_PROG(HTTRACK, httrack, no)
if test x$ac_cv_path_HTTRACK = x
then
  AC_MSG_ERROR([You don't have httrack installed.])
fi
if test x$ac_cv_path_HTTRACK = xno
then
  AC_MSG_ERROR([You don't have httrack installed.])
fi

AC_PATH_PROG(GAWK, gawk, no)
if test x$ac_cv_path_GAWK = x
then
  AC_MSG_ERROR([You don't have gawk installed.])
fi
if test x$ac_cv_path_GAWK = xno
then
  AC_MSG_ERROR([You don't have gawk installed.])
fi

AC_DEFUN([AC_PROG_PERL_VERSION],[dnl
# Make sure we have perl
if test -z "$PERL"; then
AC_CHECK_PROG(PERL,perl,perl)
fi

# Check if version of Perl is sufficient
ac_perl_version="$1"

if test "x$PERL" != "x"; then
  AC_MSG_CHECKING(for perl version greater than or equal to $ac_perl_version)
  # NB: It would be nice to log the error if there is one, but we cannot rely
  # on autoconf internals
  $PERL -e "use $ac_perl_version;" 2> /dev/null
  if test $? -ne 0; then
    AC_MSG_RESULT(no);
    $3
  else
    AC_MSG_RESULT(yes);
    $2
  fi
else
  AC_MSG_ERROR([You don't have Perl 5.0 or later installed.])
fi
])dnl

#AC_PROG_PERL_VERSION(VERSION, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
AC_PROG_PERL_VERSION([5.0], , [AC_MSG_ERROR([You don't have Perl 5.0 or later installed.])])


#AM_PATH_PYTHON ([VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
AM_PATH_PYTHON([2.6], , [AC_MSG_ERROR([You don't have Python 2.6 or later installed.])])

  AC_MSG_CHECKING([for Levenshtein for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import Levenshtein; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- python-Levenshtein:   Please, install v0.2 or later and try again. http://pypi.python.org/pypi/python-Levenshtein/
])
  fi

  AC_MSG_CHECKING([for utidylib for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import tidy; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- python-utidylib:   Please, install v0.2 or later and try again. http://utidylib.berlios.de/
])
  fi

  AC_MSG_CHECKING([for itemgetter for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
from operator import itemgetter; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- itemgetter:           installed with Python 2.2 or later.
])
  fi

  AC_MSG_CHECKING([for hashlib for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import hashlib; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- hashlib:           installed with Python 2.5 or later.
])
  fi

  AC_MSG_CHECKING([for HTMLParser for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import HTMLParser; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:.
- HTMLParser:        installed with Python 2.2 or later.
])
  fi

AC_PATH_PROG(ENV, env, no)
if test x$ac_cv_path_ENV = x
then
  AC_MSG_ERROR([You don't have env installed.])
fi
if test x$ac_cv_path_ENV = xno
then
  AC_MSG_ERROR([You don't have env installed.])
fi

AC_CHECK_FUNCS(strcasecmp)

if test x$(uname) != xDarwin;
then 
AC_CHECK_HEADER(pcreposix.h,
  AC_CHECK_LIB(pcre, pcre_info,[
  LIBS="$LIBS -lpcreposix -lpcre"
  no_comp_check=yes],
  AC_MSG_ERROR([*** unable to locate pcre library ***])),
  AC_MSG_ERROR([*** unable to locate pcreposix.h include file ***]))

AC_CHECK_HEADER(pcrecpp.h,
  AC_CHECK_LIB(pcrecpp,pcre_compile,[
  LIBS="$LIBS -lpcrecpp"
  no_comp_check=yes],
  AC_MSG_ERROR([*** unable to locate pcrecpp library ***])),
  AC_MSG_ERROR([*** unable to locate pcrecpp.h include file ***])) 
fi


#PKG_CHECK_MODULES(APERTIUM, [dnl
#  lttoolbox-3.2 >= required_lttoolbox_version dnl
#  libxml-2.0 >= required_libxml_version dnl
#  libpcre >= required_libpcre_version], CPPFLAGS="$CPPFLAGS $APERTIUM_CFLAGS"; LIBS="$LIBS $APERTIUM_LIBS")

# Check for wide strings
AC_DEFUN([AC_CXX_WSTRING],[
  AC_CACHE_CHECK(whether the compiler supports wide strings,
  ac_cv_cxx_wstring,
  [AC_LANG_SAVE
   AC_LANG_CPLUSPLUS
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <string>]],[[
std::wstring test = L"test";
   ]])],
   [ac_cv_cxx_wstring=yes], [ac_cv_cxx_wstring=no])
   AC_LANG_RESTORE
  ])
])

AC_CXX_WSTRING
AC_C_BIGENDIAN

if test "$ac_cv_cxx_wstring" = no 
then
  AC_MSG_ERROR([Missing wide string support])
fi

  
# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h stddef.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC

AC_CHECK_DECLS([fread_unlocked, fwrite_unlocked, fgetc_unlocked, fputc_unlocked, fputs_unlocked, getopt_long, fgetwc_unlocked, fputwc_unlocked, fgetws_unlocked, fputws_unlocked])

AC_CHECK_FUNCS([setlocale strdup getopt_long snprintf])

AC_OUTPUT([Makefile bitextor-4.0.pc hunalign-src/Makefile hunalign-src/src/hunalign/Makefile simpleHTMLreader/Makefile splitsentences/nonbreaking_prefixes/Makefile splitsentences/Makefile textcat/Makefile])
