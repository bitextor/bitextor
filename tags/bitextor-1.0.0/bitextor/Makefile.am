h_sources = collector/collector.h urlhash/urlhash.h webpage/webpage.h \
            languageguesser/trigramguesser.h languageguesser/wordguesser.h \
            languageguesser/urlguesser.h languageguesser/langguesser.h \
            util/genericutils.h util/memoryutils.h \
            util/languageutils.h urlgenerator/urlgenerator.h \
            orderedlist/orderedlist.h downloadmanager/downloadmanager.h \
            heavyheuristics/heavyheuristics.h util/sentencesplitter.h \
            util/xhtmltags.h tagaligner/tagaligner2-1.h
            
cc_sources = collector/collector.cpp urlhash/urlhash.cpp \
             webpage/webpage.cpp languageguesser/trigramguesser.cpp \
             languageguesser/wordguesser.cpp languageguesser/urlguesser.cpp \
             util/memoryutils.cpp \
             util/genericutils.cpp util/languageutils.cpp \
             urlgenerator/urlgenerator.cpp orderedlist/orderedlist.cpp \
             downloadmanager/downloadmanager.cpp heavyheuristics/heavyheuristics.cpp \
             util/sentencesplitter.cpp util/xhtmltags.cpp \
             tagaligner/tagaligner2-1.cpp

bin_PROGRAMS = bt-parcollector bt-collection2tmx bt-unicode2ascii bt-psbrtagaligner bt-langguesser
instdir = bitextor
bt_parcollectordir = $(prefix)/bitextor
bt_collection2tmxdir = $(prefix)/bitextor
bt_unicode2asciidir = $(prefix)/bitextor
bt_psbrtagalignerdir = $(prefix)/bitextor
bt_langguesserdir = $(prefix)/bitextor

bt_parcollector_SOURCES = $(h_sources) $(cc_sources) collectormain.cpp
bt_parcollector_DATA = ../resources/bitextor.cfg ../resources/langguesser/trigramlgparam ../resources/langguesser/wordlgparam ../resources/langguesser/urllgparam ../resources/urlgeneration/urlpatterns ../resources/elperiodico.txt
bt_parcollector_SCRIPTS = ../scripts/downloadpage.sh ../scripts/downloadandcachepage.sh ../scripts/downloadandcacheserverpage.sh
bt_parcollector_CFLAGS=-DCONF_FILE=\"$(prefix)/bitextor\"
bt_parcollector_CXXFLAGS=-DCONF_FILE=\"$(prefix)/bitextor\"

bt_collection2tmx_SOURCES = $(h_sources) $(cc_sources) corpus2tmx.cpp
bt_collection2tmx_CFLAGS=-DCONF_FILE=\"$(prefix)/bitextor\"
bt_collection2tmx_CXXFLAGS=-DCONF_FILE=\"$(prefix)/bitextor\"

bt_unicode2ascii_SOURCES = util/unicode2ascii.cpp

bt_psbrtagaligner_SOURCES = tagaligner/tagaligner2-1.h util/genericutils.h webpage/webpage.h util/sentencesplitter.h util/xhtmltags.h \
                         util/languageutils.h tagaligner/psbrtagalignermain.cpp tagaligner/tagaligner2-1.cpp util/genericutils.cpp \
                         webpage/webpage.cpp util/sentencesplitter.cpp util/xhtmltags.cpp util/languageutils.cpp

bt_langguesser_SOURCES = $(h_sources) $(cc_sources) languageguesser/langguessermain.cpp

INCLUDES = -I$(top_srcdir) $(PARALLELCOLLECTOR_CFLAGS)
CLEANFILES = *~
