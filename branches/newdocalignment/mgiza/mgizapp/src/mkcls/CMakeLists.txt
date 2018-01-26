
# Set output directory

FIND_PACKAGE(Threads)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_DEFINITIONS("-DNDEBUG")
IF (WIN32)

ELSE()
ADD_DEFINITIONS("-Wno-deprecated")
ADD_DEFINITIONS("-Wno-write-strings")
ENDIF()

SET( MKCLS_SRC
    Array.h
    FixedArray.h 
    FlexArray.h 
    GDAOptimization.cpp 
    GDAOptimization.h 
    general.cpp 
    general.h 
    HCOptimization.cpp 
    HCOptimization.h 
    IterOptimization.cpp 
    IterOptimization.h 
    KategProblem.cpp 
    KategProblem.h 
    KategProblemKBC.cpp 
    KategProblemKBC.h 
    KategProblemTest.cpp 
    KategProblemTest.h 
    KategProblemWBC.cpp 
    KategProblemWBC.h 
    mkcls.cpp 
    my.h 
    myassert.h 
    myleda.h 
    MYOptimization.cpp 
    MYOptimization.h 
    mystl.h 
    Optimization.cpp 
    Optimization.h 
    Problem.cpp 
    Problem.h 
    ProblemTest.cpp 
    ProblemTest.h 
    RRTOptimization.cpp 
    RRTOptimization.h 
    SAOptimization.cpp 
    SAOptimization.h 
    StatVar.cpp 
    StatVar.h 
    TAOptimization.cpp 
    TAOptimization.h

    )

ADD_EXECUTABLE(mkcls ${MKCLS_SRC})
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/src/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/src/mkcls )
LINK_DIRECTORIES ( ${LIBRARY_OUTPUT_PATH} )


INSTALL(TARGETS mkcls 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )



