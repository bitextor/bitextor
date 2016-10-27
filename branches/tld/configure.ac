#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)

#m4_define([required_lttoolbox_version], [3.2.0])
#m4_define([required_libxml_version], [2.6.17])
#m4_define([required_libpcre_version], [6.4])
#m4_define([required_pkg_config_version], [0.15])

AC_INIT([bitextor], [5.0.0], [jorgeferrandez@gmail.com])
AC_CONFIG_HEADER([bitextor_config.h])
#AC_CONFIG_HEADER([])
AC_CANONICAL_SYSTEM

PACKAGE=bitextor
VERSION=5.0.0

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
  CFLAGS="$CFLAGS -Wall -fomit-frame-pointer"
  CXXFLAGS="$CXXFLAGS -Wall -fomit-frame-pointer"
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
        CFLAGS="$CFLAGS -Wall -march=i686 -O3 -fomit-frame-pointer -funroll-loops"
        CXXFLAGS="$CXXFLAGS -Wall -march=i686 -O3 \
                -fomit-frame-pointer -funroll-loops"
      else
        if test x$ARCH = xx86_64
        then
          AC_MSG_RESULT([x86_64])
          CFLAGS="-Wall -O3 -mtune=nocona -fomit-frame-pointer -funroll-loops $CFLAGS"
          CXXFLAGS="-Wall -O3 -mtune=nocona \
                  -fomit-frame-pointer -funroll-loops $CXXFLAGS"
        else
          AC_MSG_RESULT([Other])
          CFLAGS="-Wall -O3 $CFLAGS"
          CXXFLAGS="-Wall -O3 $CXXFLAGS"
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

AC_PATH_PROG(BASH, bash, no)
if test x$ac_cv_path_BASH = x
then
  AC_MSG_ERROR([You don't have bash installed.])
fi
if test x$ac_cv_path_BASH = xno
then
  AC_MSG_ERROR([You don't have bash installed.])
fi

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test x$ac_cv_path_PKG_CONFIG = x
then
  AC_MSG_ERROR([You don't have pkg-config installed.])
fi
if test x$ac_cv_path_PKG_CONFIG = xno
then
  AC_MSG_ERROR([You don't have pkg-config installed.])
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

AC_PATH_PROG(WC, wc, no)
if test x$ac_cv_path_WC = x
then
  AC_MSG_ERROR([You don't have wc installed.])
fi
if test x$ac_cv_path_WC = xno
then
  AC_MSG_ERROR([You don't have wc installed.])
fi

AC_PATH_PROG(GREP, grep, no)
if test x$ac_cv_path_GREP = x
then
  AC_MSG_ERROR([You don't have grep installed.])
fi
if test x$ac_cv_path_GREP = xno
then
  AC_MSG_ERROR([You don't have grep installed.])
fi

AC_PATH_PROG(GAWK, gawk, no)
if test x$ac_cv_path_GAWK = x
then
  AC_MSG_ERROR([You don't have netstat installed.])
fi
if test x$ac_cv_path_GAWK = xno
then
  AC_MSG_ERROR([You don't have gawk installed.])
fi

AC_PATH_PROG(JAVA, java, no)
if test x$ac_cv_path_JAVA = x
then
  AC_MSG_ERROR([You don't have java installed.])
fi
if test x$ac_cv_path_JAVA = xno
then
  AC_MSG_ERROR([You don't have java installed.])
fi

AC_PATH_PROG(JAVAC, javac, no)
if test x$ac_cv_path_JAVAC = x
then
  AC_MSG_ERROR([You don't have javac installed.])
fi
if test x$ac_cv_path_JAVAC = xno
then
  AC_MSG_ERROR([You don't have javac installed.])
fi

AC_PATH_PROG(JAR, jar, no)
if test x$ac_cv_path_JAR = x
then
  AC_MSG_ERROR([You don't have java installed.])
fi
if test x$ac_cv_path_JAR = xno
then
  AC_MSG_ERROR([You don't have jar installed.])
fi

AC_ARG_WITH([apertium],
            [AS_HELP_STRING([--without-apertium],
              [disable support for apertium; if this option is enabled, you will not be able to use apertium morphological analysers)])],
            [],
            [with_apertium=yes])

AS_IF([test "x$with_apertium" != xno],
  [AC_PATH_PROG(APERTIUM, apertium-destxt, no)
     if test x$ac_cv_path_APERTIUM = x
     then
       AC_MSG_ERROR([You don't have apertium-destxt installed: try to install it or run this script with option --without-apertium.])
     fi
     if test x$ac_cv_path_APERTIUM = xno
     then
       AC_MSG_ERROR([You don't have apertium-destxt installed: try to install it or run this script with option --without-apertium.])
     fi
])

AS_IF([test "x$with_apertium" != xno],
  [AC_PATH_PROG(APERTIUM, apertium-retxt, no)
     if test x$ac_cv_path_APERTIUM = x
     then
       AC_MSG_ERROR([You don't have apertium-retxt installed: try to install it or run this script with option --without-apertium.])
     fi
     if test x$ac_cv_path_APERTIUM = xno
     then
       AC_MSG_ERROR([You don't have apertium-retxt installed: try to install it or run this script with option --without-apertium.])
     fi
])

#AM_PATH_PYTHON ([VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
AM_PATH_PYTHON([2.7], , [AC_MSG_ERROR([You don't have Python 2.7 or later installed.])])
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

AM_PATH_PYTHON([2.7], , [AC_MSG_ERROR([You don't have Python 2.7 or later installed.])])
  AC_MSG_CHECKING([for keras for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import keras; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    requiredversion="1.0.3"
    kerasversion=`python -c "import keras; print keras.__version__" 2> /dev/null`
    lowestversion=$(echo -e "$requiredversion\n$kerasversion" | sort -V | head -n 1)
    if test "$lowestversion" == "$requiredversion";
    then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([this package requires keras at version v1.0.3 or higher, while currently installed version is v$kerasversion; try running:
pip install --upgrade keras
])
    fi
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- keras:   Please, install v1.0.3 or later and try again. http://www.keras.io/
])
  fi

AM_PATH_PYTHON([2.7], , [AC_MSG_ERROR([You don't have Python 2.7 or later installed.])])
  AC_MSG_CHECKING([for h5py for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import h5py; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- h5py:   Please, install v2.6 or later and try again. http://www.keras.io/
])
  fi

AM_PATH_PYTHON([2.7], , [AC_MSG_ERROR([You don't have Python 2.7 or later installed.])])
  AC_MSG_CHECKING([for numpy for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import numpy; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- numpy:   Please, install v1.7 or later and try again. http://www.numpy.org/
])
  fi

  AC_MSG_CHECKING([for libmagic for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import magic; \
m=magic.open(magic.MAGIC_NONE); \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires magic for Python. Please, install v1.1.4 or later and try again. https://github.com/ahupp/python-magic
])
  fi


  AC_MSG_CHECKING([for LangID for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import langid; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires langid for Python. Please, install v1.1.4 or later and try again. https://github.com/saffsd/langid.py
])
  fi


  AC_MSG_CHECKING([for pycld2 for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import pycld2; \
var = True; \
print (var)" 2> /dev/null`
if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires pycld2 for Python. Please, install v0.31 or later and try again. https://pypi.python.org/pypi/pycld2/
])
  fi

  AC_MSG_CHECKING([for NLTK for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import nltk; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires nltk for Python. Please, install v3.0 or later and try again. http://www.nltk.org
])
  fi

    AC_MSG_CHECKING([for regex for Python])
  ac_supports_python_ver=`$PYTHON -c "import sys; \
import regex; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires regex for Python. Please, install it and try again. https://code.google.com/p/mrab-regex-hg/
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
AC_CHECK_HEADERS([stdlib.h string.h unistd.h stddef.h python.h])

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

AC_OUTPUT([Makefile bitextor-5.0.pc hunalign-src/Makefile hunalign-src/src/hunalign/Makefile piped-boilerpipe/Makefile piped-tika/Makefile utils/Makefile giza-pp/GIZA++-v2/Makefile giza-pp/mkcls-v2/Makefile ulysses/Makefile ulysses-data/Makefile model/Makefile])
