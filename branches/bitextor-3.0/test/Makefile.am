unit_SOURCES = \
../src/Bitext.h \
../src/Bitext.cpp \
../src/DownloadMod.h \
../src/DownloadMod.cpp \
../src/FilePreprocess.h \
../src/FilePreprocess.cpp \
../src/GlobalParams.h \
../src/GlobalParams.cpp \
../src/Heuristics.h \
../src/Heuristics.cpp \
../src/WebFile.h \
../src/WebFile.cpp \
../src/WebSite.h \
../src/WebSite.cpp \
main.cpp \
TestGlobalParams.h \
TestGlobalParams.cpp

AM_CXXFLAGS = -g

check_PROGRAMS = bitextor_test_unit
bitextor_testdir = $(prefix)/test

bitextor_test_unit_SOURCES = $(h_sources_unit) $(cc_sources_unit)
tagaligner_test_unit_LDADD = -ltextcat -lxml2 -ltidy -ltagaligner -lenca -lmagic


INCLUDES = -I/usr/include/libxml2 -I/usr/include/tidy -I/usr/include/libtextcat
CLEANFILES = *~
