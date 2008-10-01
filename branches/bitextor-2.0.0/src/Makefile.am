bin_PROGRAMS = bitextor

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

INCLUDES = -I/usr/include/libxml++-2.6 -I/usr/include/libxml++-2.6/libxml++ \
-I/usr/include/glibmm-2.4 -I/usr/lib/glibmm-2.4/include -I/usr/include/sigc++-2.0 \
-I/usr/lib/sigc++-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include \
-I/usr/lib/libxml++-2.6/include -I/usr/include/libxml2

LDADD = -ltextcat -lxml++-2.6 -ltidy -ltagaligner -lenca

EXTRA_DIST = LM2 LexAnalyzer config.xml LexAnalyzer.cpp
