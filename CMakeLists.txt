cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

project(bitextor)

set(PREPROCESS_PATH "${PROJECT_SOURCE_DIR}/preprocess")
set(SKIP_PREPROCESS_BUILD ON) # avoid compiling preprocess a million times
set(CMAKE_BUILD_TYPE "Release")
include(GNUInstallDirs)

# override output paths of children to put everything in the same folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if(NOT SKIP_PREPROCESS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/preprocess preprocess)
    install(TARGETS b64filter cache docenc foldfilter mmhsum
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(NOT SKIP_WARC2TEXT)
    add_subdirectory(${PROJECT_SOURCE_DIR}/warc2text warc2text)
endif()

if(NOT SKIP_BLEUALIGN)
    add_subdirectory(${PROJECT_SOURCE_DIR}/bleualign-cpp bleualign-cpp)
endif()

if (NOT SKIP_HUNALIGN)
    add_subdirectory(${PROJECT_SOURCE_DIR}/hunalign/src)
    install(PROGRAMS ${PROJECT_BINARY_DIR}/bin/hunalign
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if (NOT SKIP_DOCALIGN)
    set(BUILD_TESTING OFF)
    add_subdirectory(${PROJECT_SOURCE_DIR}/document-aligner document-aligner)
endif()

if (NOT SKIP_BIROAMER)
    add_subdirectory(${PROJECT_SOURCE_DIR}/biroamer fast_align)
endif()

if (NOT SKIP_MGIZA)
    add_subdirectory(${PROJECT_SOURCE_DIR}/mgiza/mgizapp mgizapp)
endif()

if (NOT SKIP_KENLM)
    set(KENLM_MAX_ORDER 7)
    add_subdirectory(${PROJECT_SOURCE_DIR}/kenlm kenlm)
endif()
