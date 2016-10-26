sources = alignerTool.cpp alignment.cpp bloom.cpp bookToMatrix.cpp cooccurrence.cpp cooccurrenceTool.cpp dictionary.cpp main.cpp networkFlow.cpp oldAlignTest.cpp trailPostprocessors.cpp translate.cpp wordAlignment.cpp ../utils/stringsAndStreams.cpp ../utils/argumentsParser.cpp ../utils/timer.cpp

headers = alignment.h bloom.h bookToMatrix.h cooccurrence.h dictionary.h dicTree.h help.h networkFlow.h quasiDiagonal.h similarityEvaluator.h TEIReader.h trailPostprocessors.h translate.h wordAlignment.h words.h

EXTRA_DIST=../include

AM_CXXFLAGS = -O9 -ffast-math -funroll-loops -I ../include

hunalign_SOURCES = $(sources) $(headers)

bin_PROGRAMS=hunalign
