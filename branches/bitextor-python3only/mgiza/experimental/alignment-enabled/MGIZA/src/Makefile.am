## Process this file with automake to produce Makefile.in

## Created by Anjuta

INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" 

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = mgiza \
	snt2cooc\
	snt2plain\
	plain2snt \
	symal \
	hmmnorm \
	d4norm

d4norm_SOURCES = \
	d4norm.cxx

d4norm_LDADD = \
	-lgiza \
	-lpthread

d4norm_LDFLAGS = \
	-L.

d4norm_DEPENDENCIES = \
	libgiza.a

d4norm_CXXFLAGS = \
	-MT \
	-MD \
	-MP \
	-MF \
	-O6

d4norm_CPPFLAGS = \
	-DNDEBUG \
	-DWORDINDEX_WITH_4_BYTE \
	-DBINARY_SEARCH_FOR_TTABLE \
	-DDEBUG

hmmnorm_SOURCES = \
	hmmnorm.cxx

hmmnorm_LDADD = \
	-lgiza \
	-lpthread

hmmnorm_LDFLAGS = \
	-L.

hmmnorm_DEPENDENCIES = \
	libgiza.a

hmmnorm_CXXFLAGS = \
	-MT \
	-MD \
	-MP \
	-MF \
	-O6

hmmnorm_CPPFLAGS = \
	-DNDEBUG \
	-DWORDINDEX_WITH_4_BYTE \
	-DBINARY_SEARCH_FOR_TTABLE \
	-DDEBUG

symal_SOURCES = \
	cmd.c  \
	cmd.h  \
	symal.cpp

plain2snt_SOURCES = \
	plain2snt.cpp

snt2plain_SOURCES = \
	snt2plain.cpp

snt2cooc_SOURCES = \
	snt2cooc.cpp

snt2cooc_CPPFLAGS = \
	-DNDEBUG \
	-DWORDINDEX_WITH_4_BYTE \
	-DBINARY_SEARCH_FOR_TTABLE \
	-DDEBUG

mgiza_SOURCES = \
	main.cpp

mgiza_DEPENDENCIES = \
	libgiza.a

mgiza_CXXFLAGS = \
	-MT \
	-MD \
	-MP \
	-MF \
	-O6

mgiza_CPPFLAGS = \
	-DNDEBUG \
	-DWORDINDEX_WITH_4_BYTE \
	-DBINARY_SEARCH_FOR_TTABLE \
	-DDEBUG

mgiza_LDFLAGS = \
	-L.

mgiza_LDADD = \
	-lgiza \
	-lpthread

lib_LIBRARIES = \
	libgiza.a

libgiza_a_SOURCES = \
	alignment.cpp\
	alignment.h \
	AlignTables.cpp \
	AlignTables.h \
	Array.h \
	Array2.h \
	Array4.h \
	ATables.cpp \
	ATables.h \
	collCounts.cpp \
	collCounts.h \
	common.h \
	D4Tables.h \
	D5Tables.h \
	defs.h \
	Dictionary.cpp \
	Dictionary.h \
	file_spec.h \
	FlexArray.h \
	ForwardBackward.cpp \
	ForwardBackward.h \
	getSentence.cpp \
	getSentence.h \
	Globals.h \
	hmm.cpp \
	hmm.h \
	HMMTables.cpp \
	HMMTables.h \
	logprob.cpp \
	logprob.h \
	model1.cpp \
	model1.h \
	model2.cpp \
	model2.h \
	model2to3.cpp \
	model3.cpp \
	model3.h \
	model3_viterbi.cpp \
	model3_viterbi_with_tricks.cpp \
	model345-peg.cpp \
	MoveSwapMatrix.cpp \
	MoveSwapMatrix.h \
	myassert.cpp \
	myassert.h \
	mymath.h \
	mystl.h \
	NTables.cpp \
	NTables.h \
	Parameter.cpp \
	Parameter.h \
	parse.cpp \
	Perplexity.cpp \
	Perplexity.h \
	Pointer.h \
	reports.cpp \
	SetArray.cpp \
	SetArray.h \
	syncObj.h \
	transpair_model1.h \
	transpair_model2.h \
	transpair_model3.cpp \
	transpair_model3.h \
	transpair_model4.cpp \
	transpair_model4.h \
	transpair_model5.cpp \
	transpair_model5.h \
	transpair_modelhmm.h \
	ttableDiff.hpp \
	TTables.cpp \
	TTables.h \
	types.h \
	utility.cpp \
	utility.h \
	Vector.h \
	vocab.cpp \
	vocab.h \
	WordClasses.h

libgiza_a_CXXFLAGS = \
	-MD \
	-MP \
	-MF \
	-MT \
	-O6

libgiza_a_CPPFLAGS = \
	-DNDEBUG \
	-DWORDINDEX_WITH_4_BYTE \
	-DBINARY_SEARCH_FOR_TTABLE \
	-DDEBUG

SUBDIRS = \
	mkcls

