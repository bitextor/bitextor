sources = Parameter.cpp myassert.cpp Perplexity.cpp model1.cpp model2.cpp \
	model3.cpp getSentence.cpp TTables.cpp ATables.cpp AlignTables.cpp \
	main.cpp NTables.cpp model2to3.cpp collCounts.cpp alignment.cpp \
	vocab.cpp MoveSwapMatrix.cpp transpair_model3.cpp transpair_model5.cpp \
	transpair_model4.cpp utility.cpp parse.cpp reports.cpp model3_viterbi.cpp \
	model3_viterbi_with_tricks.cpp Dictionary.cpp model345-peg.cpp hmm.cpp \
	HMMTables.cpp ForwardBackward.cpp

headers = Alignment.h AlignTables.h Array2.h Array4.h Array.h ATables.h \
	collCounts.h D4Tables.h D5Tables.h defs.h Dictionary.h file_spec.h \
	FlexArray.h ForwardBackward.h getSentence.h Globals.h hmm.h HMMTables.h \
	logprob.h model1.h model2.h  model3.h MoveSwapMatrix.h myassert.h \
	mymath.h mystl.h NTables.h Parameter.h Perplexity.h Pointer.h \
	transpair_model1.h ranspair_model2.h  ranspair_model3.h transpair_model4.h \
	transpair_model5.h transpair_modelhmm.h TTables.h  utility.h Vector.h vocab.h \
	WordClasses.h

GIZA___SOURCES = $(sources) $(headers)
snt2plain_out_SOURCES = snt2plain.cpp
plain2snt_out_SOURCES = plain2snt.cpp
snt2cooc_out_SOURCES = snt2cooc.cpp

bin_PROGRAMS= snt2plain.out plain2snt.out snt2cooc.out GIZA++

AM_CXXFLAGS = -O3 -funroll-loops -DNDEBUG -DWORDINDEX_WITH_4_BYTE -DBINARY_SEARCH_FOR_TTABLE -DWORDINDEX_WITH_4_BYTE

