COMPILE_FLAGS="-O3 -DNDEBUG -DWORDINDEX_WITH_4_BYTE -DBINARY_SEARCH_FOR_TTABLE -MD -MP -MF -MT "

GCC="gcc $COMPILE_FLAGS"
GPP="g++ $COMPILE_FLAGS"
LDFLAGS="-static"

# mac, 'cos OSX doesn't support static linking and other such nonsense
#GCC="gcc-mp-4.5 $COMPILE_FLAGS"
#GPP="g++-mp-4.5 $COMPILE_FLAGS"
#GCC="clang $COMPILE_FLAGS"
#GPP="clang++ $COMPILE_FLAGS"

SRC_DIR=~/workspace/github/mgiza/mgizapp/src
BOOST_ROOT=~/workspace/boost/boost_1_55_0
BOOST_INCLUDE=$BOOST_ROOT/include
BOOST_LIBRARYDIR=$BOOST_ROOT/lib64

rm *.o libmgiza.a d4norm hmmnorm mgiza plain2snt snt2cooc snt2coocrmp snt2plain symal mkcls

$GPP -I$SRC_DIR -I$BOOST_INCLUDE -c -fPIC   \
 $SRC_DIR/alignment.cpp \
 $SRC_DIR/AlignTables.cpp \
 $SRC_DIR/ATables.cpp \
 $SRC_DIR/collCounts.cpp \
 $SRC_DIR/Dictionary.cpp \
 $SRC_DIR/ForwardBackward.cpp \
 $SRC_DIR/getSentence.cpp \
 $SRC_DIR/hmm.cpp \
 $SRC_DIR/HMMTables.cpp \
 $SRC_DIR/logprob.cpp \
 $SRC_DIR/model1.cpp \
 $SRC_DIR/model2.cpp \
 $SRC_DIR/model2to3.cpp \
 $SRC_DIR/model345-peg.cpp \
 $SRC_DIR/model3.cpp \
 $SRC_DIR/model3_viterbi.cpp \
 $SRC_DIR/model3_viterbi_with_tricks.cpp \
 $SRC_DIR/MoveSwapMatrix.cpp \
 $SRC_DIR/myassert.cpp \
 $SRC_DIR/NTables.cpp \
 $SRC_DIR/Parameter.cpp \
 $SRC_DIR/parse.cpp \
 $SRC_DIR/Perplexity.cpp \
 $SRC_DIR/reports.cpp \
 $SRC_DIR/SetArray.cpp \
 $SRC_DIR/transpair_model3.cpp \
 $SRC_DIR/transpair_model4.cpp \
 $SRC_DIR/transpair_model5.cpp \
 $SRC_DIR/TTables.cpp \
 $SRC_DIR/utility.cpp \
 $SRC_DIR/vocab.cpp

$GCC -c -fPIC $SRC_DIR/cmd.c

ar rvs libmgiza.a *.o

$GPP -o d4norm $SRC_DIR/d4norm.cxx      $LDFLAGS -I$BOOST_INCLUDE -I$SRC_DIR -L. -lmgiza  -L$BOOST_LIBRARYDIR -lboost_system -lboost_thread -lpthread 

$GPP -o hmmnorm $SRC_DIR/hmmnorm.cxx    $LDFLAGS -I$BOOST_INCLUDE -I$SRC_DIR ./libmgiza.a  -L$BOOST_LIBRARYDIR -lboost_system -lboost_thread -lpthread 

$GPP -o mgiza $SRC_DIR/main.cpp         $LDFLAGS -I$BOOST_INCLUDE -I$SRC_DIR ./libmgiza.a  -L$BOOST_LIBRARYDIR -lboost_system -lboost_thread -lpthread -lrt

$GPP -o plain2snt $SRC_DIR/plain2snt.cpp

$GPP -o snt2cooc  $SRC_DIR/snt2cooc.cpp 

$GPP -o snt2coocrmp $SRC_DIR/snt2cooc-reduce-mem-preprocess.cpp 

$GPP -o snt2plain $SRC_DIR/snt2plain.cpp 

$GPP -o symal $SRC_DIR/symal.cpp        $LDFLAGS -I$BOOST_INCLUDE -I$SRC_DIR ./libmgiza.a  -L$BOOST_LIBRARYDIR -lboost_system -lboost_thread -lpthread 

$GPP -I$SRC_DIR/mkcls  -o mkcls $SRC_DIR/mkcls/mkcls.cpp $SRC_DIR/mkcls/general.cpp $SRC_DIR/mkcls/KategProblemKBC.cpp $SRC_DIR/mkcls/KategProblem.cpp $SRC_DIR/mkcls/Problem.cpp $SRC_DIR/mkcls/ProblemTest.cpp $SRC_DIR/mkcls/IterOptimization.cpp $SRC_DIR/mkcls/StatVar.cpp $SRC_DIR/mkcls/TAOptimization.cpp $SRC_DIR/mkcls/SAOptimization.cpp $SRC_DIR/mkcls/GDAOptimization.cpp $SRC_DIR/mkcls/MYOptimization.cpp $SRC_DIR/mkcls/RRTOptimization.cpp $SRC_DIR/mkcls/HCOptimization.cpp $SRC_DIR/mkcls/Optimization.cpp $SRC_DIR/mkcls/KategProblemWBC.cpp $SRC_DIR/mkcls/KategProblemTest.cpp


