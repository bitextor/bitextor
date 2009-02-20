bin_PROGRAMS = bitextor

textcatconfdir = /usr/local/etc/bitextor/LM
textcatconf_DATA = LM/*

bitextor_SOURCES = \
Bitext.h \
Bitext.cpp \
DownloadMod.h \
DownloadMod.cpp \
FilePreprocess.h \
FilePreprocess.cpp \
GlobalParams.h \
GlobalParams.cpp \
Heuristics.h \
Heuristics.cpp \
WebFile.h \
WebFile.cpp \
WebSite.h \
WebSite.cpp \
Bitextor.cpp

AM_CFLAGS = -g3 -Wall -Wunused \
-Wmissing-prototypes

INCLUDES = -I/usr/include/libxml2 -I/usr/include/tidy -I/usr/include/libtextcat

LDADD = -ltextcat -lxml2 -ltidy -ltagaligner -lenca -lmagic

EXTRA_DIST = LM
