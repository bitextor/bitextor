bin_PROGRAMS = bitextor

sysconfdir = @sysconfdir@/bitextor3/conf
textcatconfdir = @sysconfdir@/bitextor3/LM
textcatconf_DATA = LM/*

bitextor_SOURCES = \
BitextCandidates.h \
BitextCandidates.cpp \
TranslationMemory.h \
TranslationMemory.cpp \
DownloadMod.h \
DownloadMod.cpp \
FilePreprocess.h \
FilePreprocess.cpp \
GlobalParams.h \
GlobalParams.cpp \
Heuristics.h \
Heuristics.cpp \
Url.h \
Url.cpp \
WebFile.h \
WebFile.cpp \
WebSite.h \
WebSite.cpp \
SaxParser.h \
SaxParser.cpp \
Bitextor.cpp

AM_CPPFLAGS = -g3 -Wall -Wunused -DBASE_CONF='"${sysconfdir}/config.xml"'

INCLUDES = $(BITEXTOR_CFLAGS) -I/usr/include/tidy -I/usr/include/libtextcat

LDADD = $(BITEXTOR_LIBS) $(LIBS)

EXTRA_DIST = LM
