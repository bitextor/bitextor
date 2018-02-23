
# Set output directory

FIND_PACKAGE(Threads)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_DEFINITIONS("-DNDEBUG")
ADD_DEFINITIONS("-DWORDINDEX_WITH_4_BYTE")
ADD_DEFINITIONS("-DBINARY_SEARCH_FOR_TTABLE")
ADD_DEFINITIONS("-DDEBUG")
IF (WIN32)

ELSE()
ADD_DEFINITIONS("-Wno-deprecated")
ADD_DEFINITIONS("-Wno-write-strings")
ENDIF()

SET( LIBMGIZA_SRC
    alignment.cpp alignment.h
    AlignTables.cpp AlignTables.h
    Array2.h Array4.h
    Array.h ATables.cpp
    ATables.h cmd.c
    cmd.h collCounts.cpp
    collCounts.h common.h
    D4Tables.h
    D5Tables.h defs.h
    Dictionary.cpp Dictionary.h
    file_spec.h FlexArray.h
    ForwardBackward.cpp ForwardBackward.h
    getSentence.cpp getSentence.h
    Globals.h hmm.cpp
    hmm.h 
    HMMTables.cpp HMMTables.h
    logprob.cpp logprob.h
    model1.cpp
    model1.h model2.cpp
    model2.h model2to3.cpp
    model345-peg.cpp model3.cpp
    model3.h model3_viterbi.cpp
    model3_viterbi_with_tricks.cpp MoveSwapMatrix.cpp
    MoveSwapMatrix.h myassert.cpp
    myassert.h mymath.h
    mystl.h NTables.cpp
    NTables.h Parameter.cpp
    Parameter.h parse.cpp
    Perplexity.cpp Perplexity.h
    Pointer.h
    reports.cpp SetArray.cpp
    SetArray.h
    syncObj.h transpair_model1.h
    transpair_model2.h transpair_model3.cpp
    transpair_model3.h transpair_model4.cpp
    transpair_model4.h transpair_model5.cpp
    transpair_model5.h transpair_modelhmm.h
    ttableDiff.hpp TTables.cpp
    TTables.h types.h
    utility.cpp utility.h
    Vector.h vocab.cpp
    vocab.h WordClasses.h
)

ADD_LIBRARY(mgiza_lib STATIC ${LIBMGIZA_SRC})
SET_TARGET_PROPERTIES(mgiza_lib PROPERTIES OUTPUT_NAME "mgiza")

INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
IF (WIN32)
	INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/w32 )
	LINK_DIRECTORIES ( ${PROJECT_SOURCE_DIR}/w32 )
	IF( USE_64_BIT )
		SET(CMAKE_THREAD_LIBS_INIT pthread64)
	ELSE( USE_64_BIT )
		SET(CMAKE_THREAD_LIBS_INIT pthread)
	ENDIF(  )
	
	
ENDIF()	
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/src/ )
LINK_DIRECTORIES ( ${LIBRARY_OUTPUT_PATH} )

SET( MGIZA_SRC main.cpp )

ADD_EXECUTABLE( mgiza ${MGIZA_SRC} )

TARGET_LINK_LIBRARIES (
    mgiza
    mgiza_lib
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )


ADD_EXECUTABLE(snt2cooc snt2cooc.cpp)
ADD_EXECUTABLE(snt2coocrmp snt2cooc-reduce-mem-preprocess.cpp)
ADD_EXECUTABLE(snt2plain snt2plain.cpp)
ADD_EXECUTABLE(plain2snt plain2snt.cpp)
ADD_EXECUTABLE(symal symal.cpp cmd.c)
ADD_EXECUTABLE(hmmnorm hmmnorm.cxx)
ADD_EXECUTABLE(d4norm d4norm.cxx)
TARGET_LINK_LIBRARIES (
    hmmnorm
    mgiza_lib
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )
TARGET_LINK_LIBRARIES (
    d4norm
    mgiza_lib
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

INSTALL(TARGETS mgiza_lib mgiza snt2cooc snt2plain plain2snt symal hmmnorm d4norm snt2coocrmp
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )



