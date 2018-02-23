#Locate Boost libs. Windows users: make sure BOOST_ROOT and BOOST_PATH are set correctly on your environment.
#See the site FAQ for more details.

MACRO (GET_BOOST_INCLUDE_PATH path libs)
  #todo: allow this to fall back on a local distributed copy, so user doesn't have to d/l Boost seperately
  
  #todo: limit Boost version?
  #todo: use COMPONENTS threads to locate boost_threads without breaking the current support
  IF(Boost_FOUND)
    IF (NOT _boost_IN_CACHE)
      MESSAGE( "Boost found" )
      message(STATUS "Boost_INCLUDE_DIR    : ${Boost_INCLUDE_DIR}")
    ENDIF (NOT _boost_IN_CACHE)
    SET(${path} ${Boost_INCLUDE_DIRS} )
	SET(${libs} ${Boost_LIBRARIES} )
	link_directories ( ${Boost_LIBRARY_DIRS} )
  ELSE()
    MESSAGE(FATAL_ERROR "Boost not found, please set the BOOST_ROOT environment variable " )
  ENDIF()
ENDMACRO (GET_BOOST_INCLUDE_PATH path libs)

