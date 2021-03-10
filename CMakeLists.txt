cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

project(bitextor)

set(PREPROCESS_PATH "${PROJECT_SOURCE_DIR}/preprocess")
set(SKIP_PREPROCESS_BUILD ON) # avoid compiling preprocess a million times
set(CMAKE_BUILD_TYPE "Release")

if(NOT SKIP_PREPROCESS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/preprocess ${PROJECT_SOURCE_DIR}/preprocess)
endif()

if(NOT SKIP_WARC2TEXT)
    add_subdirectory(${PROJECT_SOURCE_DIR}/warc2text ${PROJECT_SOURCE_DIR}/warc2text)
endif()

if(NOT SKIP_BLEUALIGN)
    add_subdirectory(${PROJECT_SOURCE_DIR}/bleualign-cpp ${PROJECT_SOURCE_DIR}/bleualign-cpp)
endif()

if (NOT SKIP_HUNALIGN)
    add_custom_target(hunalign ALL
        COMMAND $(MAKE)
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/hunalign/src/hunalign
    )
endif()

if (NOT SKIP_DOCALIGN)
    set(BUILD_TESTING OFF)
    add_subdirectory(${PROJECT_SOURCE_DIR}/document-aligner ${PROJECT_SOURCE_DIR}/document-aligner)
endif()

if (NOT SKIP_BIROAMER)
    add_subdirectory(${PROJECT_SOURCE_DIR}/biroamer/fast_align ${PROJECT_SOURCE_DIR}/biroamer/fast_align/build)
endif()

if (NOT SKIP_MGIZA)
    add_subdirectory(${PROJECT_SOURCE_DIR}/mgiza/mgizapp ${PROJECT_SOURCE_DIR}/mgiza/mgizapp)
endif()

if (NOT SKIP_CLUSTERCAT)
    add_custom_target(clustercat ALL
        COMMAND $(MAKE)
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/clustercat
    )
endif()
