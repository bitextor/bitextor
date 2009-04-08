h_sources_unit = ../src/Bitext.h ../src/DownloadMod.h \
            ../src/FilePreprocess.h ../src/GlobalParams.h ../src/Heuristics.h \
            ../src/WebFile.h ../src/WebSite.h \
            test_GlobalParams.h test_WebFile.h test_Heuristics.h test_Bitext.h

cc_sources_unit = ../src/Bitext.cpp ../src/DownloadMod.cpp \
            ../src/FilePreprocess.cpp ../src/GlobalParams.cpp ../src/Heuristics.cpp \
            ../src/WebFile.cpp ../src/WebSite.cpp \
            test_GlobalParams.cpp test_WebFile.cpp test_Heuristics.cpp test_Bitext.cpp main.cpp


check_PROGRAMS = bitextor_test_unit
bitextor_testdir = $(prefix)/test

bitextor_test_unit_SOURCES = $(h_sources_unit) $(cc_sources_unit)
bitextor_test_unit_LDFLAGS = $(BITEXTOR_LIBS) -lcppunit

INCLUDES = -I/usr/include/cppunit -I$(top_srcdir) $(BITEXTOR_CFLAGS) -I/usr/include/tidy -I/usr/include/libtextcat -I../src/
CLEANFILES = *~
