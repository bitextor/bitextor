#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)

#m4_define([required_lttoolbox_version], [3.2.0])
#m4_define([required_libxml_version], [2.6.17])
#m4_define([required_libpcre_version], [6.4])
#m4_define([required_pkg_config_version], [0.15])

AC_INIT([bitextor], [git], [https://github.com/bitextor])
AC_CONFIG_HEADER([bitextor_config.h])
#AC_CONFIG_HEADER([])
AC_CANONICAL_SYSTEM

PACKAGE=bitextor
VERSION=git

AM_INIT_AUTOMAKE([subdir-objects])

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
  AC_MSG_ERROR([You don't have gawk installed.])
fi
if test x$ac_cv_path_GAWK = xno
then
  AC_MSG_ERROR([You don't have gawk installed.])
fi

AC_PATH_PROG(CMAKE, cmake, no)
if test x$ac_cv_path_CMAKE = x
then
  AC_MSG_ERROR([You don't have cmake installed.])
fi
if test x$ac_cv_path_CMAKE = xno
then
  AC_MSG_ERROR([You don't have cmake installed.])
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

AC_PATH_PROG(HTTRACK, httrack, no)
if test x$ac_cv_path_HTTRACK = x
then
  AC_MSG_WARN([You don't have httrack installed. You can only use Creepy crawler (default crawler and integrated in Bitextor)])
fi
if test x$ac_cv_path_HTTRACK = xno
then
  AC_MSG_WARN([You don't have httrack installed. You can only use Creepy crawler (default crawler and integrated in Bitextor)])
fi

AC_PATH_PROG(MVN, mvn, no)
if test x$ac_cv_path_MVN = x
then
  AC_MSG_ERROR([You don't have mvn (maven) installed.])
fi
if test x$ac_cv_path_MVN = xno
then
  AC_MSG_ERROR([You don't have mvn (maven) installed.])
fi

AC_PATH_PROG(NGRAM, ngram, no)
if test x$ac_cv_path_NGRAM = x
then
  AC_MSG_WARN([You don't have ngram (SRILM) installed. You can't use Zipporah])
fi
if test x$ac_cv_path_NGRAM = xno
then
  AC_MSG_WARN([You don't have ngram (SRILM) installed. You can't use Zipporah])
fi

AC_ARG_WITH([apertium],
            [AS_HELP_STRING([--with-apertium],
              [enable support for apertium; if this option is disabled, you will not be able to use apertium morphological analysers)])],
            [],
            [with_apertium=no])

AS_IF([test "x$with_apertium" != yes],
  AC_MSG_WARN([You are not using apertium as morphological analyser. In case you want to use it, install it and use with option --with-apertium])
)

          
AS_IF([test "x$with_apertium" != xno],
  [AC_PATH_PROG(APERTIUM, apertium-destxt, no)
     if test x$ac_cv_path_APERTIUM = x
     then
       AC_MSG_ERROR([You don't have apertium-destxt installed])
     fi
     if test x$ac_cv_path_APERTIUM = xno
     then
       AC_MSG_ERROR([You don't have apertium-destxt installed])
     fi
])

AS_IF([test "x$with_apertium" != xno],
  [AC_PATH_PROG(APERTIUM, apertium-retxt, no)
     if test x$ac_cv_path_APERTIUM = x
     then
       AC_MSG_ERROR([You don't have apertium-retxt installed])
     fi
     if test x$ac_cv_path_APERTIUM = xno
     then
       AC_MSG_ERROR([You don't have apertium-retxt installed])
     fi
])

#AM_PATH_PYTHON ([VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
AM_PATH_PYTHON([3.0], , [AC_MSG_ERROR([You don't have Python 3.0 or later installed.])])
  AC_MSG_CHECKING([for Levenshtein for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
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

AM_PATH_PYTHON([3.0], , [AC_MSG_ERROR([You don't have Python 3.0 or later installed.])])
  AC_MSG_CHECKING([for tensorflow for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
import tensorflow; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- tensorflow:   Please, install tensorflow and try again. http://www.tensorflow.org/
])
  fi

  AC_MSG_CHECKING([for keras for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
import keras; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    requiredversion="1.0.3"
    kerasversion=`python -c "import keras; print(keras.__version__)" 2> /dev/null`
    lowestversion=$(echo -e "$requiredversion\n$kerasversion" | sort -V | head -n 1)
    if test "$lowestversion" == "$requiredversion";
    then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([this package requires keras at version v1.0.3 or higher, while currently installed version is $kerasversion; try running:
pip install --upgrade keras
])
    fi
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- keras:   Please, install v1.0.3 or later and try again. http://www.keras.io/
])
  fi

AM_PATH_PYTHON([3.0], , [AC_MSG_ERROR([You don't have Python 3.0 or later installed.])])
  AC_MSG_CHECKING([for h5py for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
import h5py; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- h5py:   Please, install v2.6 or later and try again. http://www.h5py.org/
])
  fi
AM_PATH_PYTHON([3.0], , [AC_MSG_ERROR([You don't have Python 3.0 or later installed.])])
  AC_MSG_CHECKING([for scipy for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
import scipy; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_WARN([Paracrawl document aligner package requires scipy for Python 3. This option will not work on Bitextor until you install the library.
])
  fi

  AC_MSG_CHECKING([for sklearn for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
import sklearn; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_WARN([Paracrawl document aligner package requires sklearn for Python 3. This option will not work on Bitextor until you install the library.
])
  fi


  AC_MSG_CHECKING([for numpy for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
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
  ac_supports_python_ver=`python3 -c "import sys; \
import magic; \
m=magic.open(magic.MAGIC_NONE); \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires magic for Python. Please, install with 'sudo apt install python3-magic' and try again.
])
  fi


  AC_MSG_CHECKING([for iso-639 for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
import iso639; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires iso639 for Python. Please, install with 'sudo pip install iso-639' and try again.
])
  fi

  AC_MSG_CHECKING([for LangID for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
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

  AC_MSG_CHECKING([for NLTK for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
import nltk; \
var = True; \
print (var)" 2> /dev/null`
  if test "$ac_supports_python_ver" = "True";
  then
    requiredversion="3.3"
    nltkversion=`python -c "import nltk; print nltk.__version__" 2> /dev/null`
    lowestversion=$(echo -e "$requiredversion\n$nltkversion" | sort -V | head -n 1)
    if test "$lowestversion" == "$requiredversion";
    then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([this package requires nltk at version v3.3 or higher, while currently installed version is v$nltkversion; try running:
pip install --upgrade nltk
])
    fi
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([this package requires the following libraries for Python:
- keras:   Please, install v1.0.3 or later and try again. http://www.keras.io/
])
  fi

    AC_MSG_CHECKING([for regex for Python])
  ac_supports_python_ver=`python3 -c "import sys; \
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

AC_OUTPUT([Makefile bitextor-git.pc piped-boilerpipe/Makefile piped-tika/Makefile utils/Makefile ulysses/Makefile ulysses-data/Makefile model/Makefile zipporah/Makefile share/Makefile])
