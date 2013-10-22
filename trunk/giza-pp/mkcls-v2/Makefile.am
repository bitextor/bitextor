sources = GDAOptimization.cpp HCOptimization.cpp Problem.cpp \
            IterOptimization.cpp ProblemTest.cpp RRTOptimization.cpp \
            MYOptimization.cpp SAOptimization.cpp TAOptimization.cpp \
            Optimization.cpp KategProblemTest.cpp KategProblemKBC.cpp \
            KategProblemWBC.cpp KategProblem.cpp StatVar.cpp general.cpp \
            mkcls.cpp


headers = Array.h FixedArray.h FlexArray.h GDAOptimization.h general.h HCOptimization.h IterOptimization.h KategProblem.h KategProblemKBC.h KategProblemTest.h KategProblemWBC.h MSBOptimization.h myassert.h my.h myleda.h MYOptimization.h mystl.h Optimization.h PopOptimization.h Problem.h ProblemTest.h RRTOptimization.h SAOptimization.h StatVar.h TAOptimization.h

mkcls_SOURCES = $(sources) $(headers)

bin_PROGRAMS= mkcls

AM_CXXFLAGS = -Wall -W -DNDEBUG -O3 -funroll-loops
