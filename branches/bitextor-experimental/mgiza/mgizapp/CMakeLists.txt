PROJECT (mgiza)

SET(MGIZA_VERSION_MAJOR "0")
SET(MGIZA_VERSION_MINOR "7")
SET(MGIZA_VERSION_PATCH "0")

MATH(EXPR MGIZA_INT_VERSION "(${MGIZA_VERSION_MAJOR} * 10000) + (${MGIZA_VERSION_MINOR} * 100) + (${MGIZA_VERSION_PATCH} * 1)" )

SET(MGIZA_VERSION "${MGIZA_VERSION_MAJOR}.${MGIZA_VERSION_MINOR}.${MGIZA_VERSION_PATCH}")

#CMake 2.6+ is recommended to an improved Boost module
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)

IF(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX
        "inst" CACHE PATH "MGIZA install prefix" FORCE
        )
    MESSAGE(STATUS "You have not set the install dir, default to './inst', if
    you want to set it, use cmake -DCMAKE_INSTALL_PREFIX to do so")
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#set various platform specific global options
IF(WIN32)
 SET(CMAKE_DEBUG_POSTFIX "d")
 OPTION( USE_64_BIT "Set to on if you want to compile Win64" OFF )
ENDIF(WIN32)

SET(CMAKE_C_COMPILER "gcc" )
SET(CMAKE_C_FLAGS
    "-O3 -DNDEBUG -DWORDINDEX_WITH_4_BYTE -DBINARY_SEARCH_FOR_TTABLE -MD -MP -MF -MT"
    )

SET(CMAKE_CXX_COMPILER "g++" )
SET(CMAKE_CXX_FLAGS
    "-O3 -DNDEBUG -DWORDINDEX_WITH_4_BYTE -DBINARY_SEARCH_FOR_TTABLE -MD -MP -MF -MT -lrt"
    )

INCLUDE(cmake/FindTR1.cmake)

# include specific modules
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


# Find Boost library, specify
# BOOST_ROOT=/e/programs/boost_1_35_0
# BOOST_LIBRARYDIR=$BOOST_ROOT/stage/lib

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)


FIND_PACKAGE( Boost 1.41 COMPONENTS thread system)


IF(Boost_FOUND)
    IF (NOT _boost_IN_CACHE)
     MESSAGE( "Boost found" )
     message(STATUS "Boost_INCLUDE_DIR    : ${Boost_INCLUDE_DIR}")
    ENDIF (NOT _boost_IN_CACHE)
	LINK_DIRECTORIES ( ${Boost_LIBRARY_DIRS} )
	INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIRS} )
ELSE()
    set(Boost_USE_STATIC_LIBS        OFF)
    
    FIND_PACKAGE( Boost 1.41 COMPONENTS thread system)
    IF(Boost_FOUND)
       IF (NOT _boost_IN_CACHE)
          MESSAGE( "Boost found" )
          message(STATUS "Boost_INCLUDE_DIR    : ${Boost_INCLUDE_DIR}")
       ENDIF (NOT _boost_IN_CACHE)
        LINK_DIRECTORIES ( ${Boost_LIBRARY_DIRS} )
        INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIRS} )
    ELSE()
       MESSAGE(FATAL_ERROR "Boost not found, please set the BOOST_ROOT and BOOST_LIBRARYDIR environment variables " )
    ENDIF()
ENDIF()

ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (src/mkcls)
ADD_SUBDIRECTORY (scripts)

IF (WIN32)
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS "USE_64_BIT = ${USE_64_BIT}" )
MESSAGE( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )


SET( USE_64_BIT "${USE_64_BIT}" CACHE BOOL
    "Set to ON to build Win64" FORCE )
ENDIF()




