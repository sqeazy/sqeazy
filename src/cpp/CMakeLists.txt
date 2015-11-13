# cmake compatibility issues
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)


# project name
PROJECT(sqeazy CXX)
if(NOT(${CMAKE_VERSION} VERSION_LESS "3.0.0"))
cmake_policy(SET CMP0042 NEW)
endif()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
 
# version number
SET (SQEAZY_NAME "Sqeazy - Fast and Flexible Volume Compression Library")
SET (SQEAZY_CODENAME "${PROJECT_NAME}")
SET (SQEAZY_COPYRIGHT_YEARS "2014")
SET (SQEAZY_VERSION_MAJOR 0)
SET (SQEAZY_VERSION_MINOR 0)
SET (SQEAZY_VERSION_PATCH 0)
SET (SQEAZY_VERSION "${SQEAZY_VERSION_MAJOR}.${SQEAZY_VERSION_MINOR}.${SQEAZY_VERSION_PATCH}")
SET (SQEAZY_VENDOR_ID "mpi cbg")
SET (SQEAZY_VENDOR_NAME "Max Planck Institute of Molecular Cell Biology and Genetics ")
SET (SQEAZY_VENDOR_URL "www.mpi-cbg.de")
SET (SQEAZY_ID "${SQEAZY_VENDOR_ID}.${PROJECT_NAME}")

# trying to setup paths so this package can be picked up
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")

# shared path is architecture independent for now, TODO extend this to lib/bin/include
IF(UNIX)
  IF(APPLE)
    set(INSTALL_SHARE_DIR ${PROJECT_NAME}.app/Contents/Resources/ CACHE PATH "Installation directory for shared files")
  ELSE(APPLE)
    set(INSTALL_SHARE_DIR lib/CMake/${PROJECT_NAME} CACHE PATH "Installation directory for shared files")
  ENDIF(APPLE)
    #the following was tested with OSX 10.8.5 and Xcode 5.0.2
    #seems to me that under apple the rpath is not stripped automatically when doing the install
    #under linux it is
    #SET(CMAKE_SKIP_RPATH ON)
ELSE(UNIX)
  IF(WIN32 AND NOT CYGWIN)
    set(INSTALL_SHARE_DIR CMake CACHE PATH "Installation directory for shared files")
  ELSE(WIN32 AND NOT CYGWIN)
    MESSAGE(FATAL_ERROR ">> UNKNOWN ARCHITECTURE .. unable to set share dir")
  ENDIF(WIN32 AND NOT CYGWIN)
ENDIF(UNIX)
 
# Make relative paths absolute (needed later on)
foreach(p BIN LIB INCLUDE SHARE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

include_directories("${PROJECT_SOURCE_DIR}/inc")

# set project build type
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
MESSAGE(STATUS ">> Setting up ${CMAKE_BUILD_TYPE} build")

#C/C++ specific flags
#for GCC
IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")
  check_cxx_compiler_flag(-ftree-vectorize HAS_GCC_TREE_VECTORIZE_COMPILERFLAG)
  check_cxx_compiler_flag(-march=native HAS_MARCH_COMPILERFLAG)
  check_cxx_compiler_flag(-ffast-math HAS_FAST_MATH_COMPILERFLAG)
ENDIF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
 
 #for clang
IF("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")
   SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")
   check_cxx_compiler_flag(-fvectorize HAS_CLANG_VECTORIZE_COMPILERFLAG)
   check_cxx_compiler_flag(-march=native HAS_MARCH_COMPILERFLAG)
   check_cxx_compiler_flag(-ffast-math HAS_FAST_MATH_COMPILERFLAG)
ENDIF()

#TODO: think about support parallel studio as well
IF(UNIX)
SET(HOST_COMPILER_RELEASE_FLAGS "-Wall -O3")
ELSE(UNIX)
IF(WIN32)
MESSAGE(STATUS "[Win32] Compiler used C++ ${CMAKE_CXX_COMPILER_ID}")
MESSAGE(STATUS "[Win32] Compiler used C ${CMAKE_C_COMPILER_ID}")
MESSAGE(STATUS "[Win32] default HOST_COMPILER_RELEASE_FLAGS : ${HOST_COMPILER_RELEASE_FLAGS}")

#for intrinsics to work

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Oi")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Oi")

MESSAGE(STATUS "[Win32] CMAKE_CXX_FLAGS         : ${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "[Win32] CMAKE_CXX_FLAGS_RELEASE : ${CMAKE_CXX_FLAGS_RELEASE}")
MESSAGE(STATUS "[Win32] CMAKE_CXX_FLAGS_DEBUG   : ${CMAKE_CXX_FLAGS_DEBUG}")

#SET(HOST_COMPILER_RELEASE_FLAGS "/Ox /MD /W3 /EHsc")
#MESSAGE(FATAL "Microsoft Visual Studio not supported! Please use mingw-w64!")
ENDIF()
MESSAGE(STATUS "[Win32] updated HOST_COMPILER_RELEASE_FLAGS : ${HOST_COMPILER_RELEASE_FLAGS}")
MESSAGE(STATUS "[Win32] updated HOST_COMPILER_DEBUG_FLAGS : ${HOST_COMPILER_DEBUG_FLAGS}")
ENDIF(WIN32)
ENDIF(UNIX)

IF(HAS_CLANG_VECTORIZE_COMPILERFLAG)
  SET(HOST_COMPILER_RELEASE_FLAGS "${HOST_COMPILER_RELEASE_FLAGS} -fvectorize")
ENDIF()

IF(HAS_GCC_TREE_VECTORIZE_COMPILERFLAG)
  SET(HOST_COMPILER_RELEASE_FLAGS "${HOST_COMPILER_RELEASE_FLAGS} -ftree-vectorize")
ENDIF()



IF(UNIX)
IF(HAS_MARCH_COMPILERFLAG)
  # SET(HOST_COMPILER_RELEASE_FLAGS "${HOST_COMPILER_RELEASE_FLAGS} -march=native")
  # SET(HOST_COMPILER_DEBUG_FLAGS "${HOST_COMPILER_DEBUG_FLAGS} -march=native")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
ENDIF()
ENDIF()

IF(HAS_FAST_MATH_COMPILERFLAG)
  SET(HOST_COMPILER_RELEASE_FLAGS "${HOST_COMPILER_RELEASE_FLAGS} -ffast-math")
ENDIF()

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${HOST_COMPILER_RELEASE_FLAGS} ")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${HOST_COMPILER_RELEASE_FLAGS} ")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${HOST_COMPILER_DEBUG_FLAGS} ")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${HOST_COMPILER_DEBUG_FLAGS} ")

IF(CMAKE_BUILD_TYPE STREQUAL Release)
  MESSAGE(STATUS ">> using ${CMAKE_BUILD_TYPE} build flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
ELSE()
  MESSAGE(STATUS ">> using ${CMAKE_BUILD_TYPE} build flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
ENDIF()

IF(SQY_EXPERIMENTAL) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SQY_X_=${SQY_EXPERIMENTAL}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -D_SQY_X_=${SQY_EXPERIMENTAL}")
MESSAGE(STATUS ">> appended experimental flags: ${CMAKE_CXX_FLAGS}")
else(SQY_EXPERIMENTAL)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SQY_X_=0")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -D_SQY_X_=0")
endif(SQY_EXPERIMENTAL)

IF(UNIX)
check_cxx_compiler_flag(-std=c++11 HAS_CPP11_COMPILERFLAG)
ELSE(UNIX)
  
  SET(HAS_CPP11_COMPILERFLAG "NOT NEEDED")
ENDIF(UNIX)

#lz4 is needed for the entire project, so let's include it here
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

FIND_PACKAGE(LZ4 REQUIRED)
IF(LZ4_FOUND)
INCLUDE_DIRECTORIES(${LZ4_INCLUDE_DIRS})
LINK_DIRECTORIES(${LZ4_LIBRARY_DIRS}) 
ENDIF()

# add subdirectories
# src
ADD_SUBDIRECTORY(src) # source code of the library

# bench
IF(HAS_CPP11_COMPILERFLAG)
ADD_SUBDIRECTORY(bench) # source code of the benchmark suite
export(TARGETS bench ${PROJECT_NAME}
  FILE "${PROJECT_BINARY_DIR}/sqeazy-targets.cmake")
ELSE()
MESSAGE(WARNING ">> unable to build benchmark suite due to missing C++11 support")
ENDIF()

# test
IF(WIN32)
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
ENDIF()
FIND_PACKAGE (Boost 1.42 COMPONENTS system filesystem unit_test_framework thread REQUIRED)
IF(Boost_FOUND)
MESSAGE(STATUS "Boost found at ${Boost_LIBRARY_DIR} ${Boost_LIBRARIES}")
ADD_SUBDIRECTORY(tests)
enable_testing()
include("CTestLists.txt")
ELSE()
MESSAGE(WARNING ">> Boost libraries not found, unable to compile tests (skipping)")
ENDIF()


# export stuff
export(PACKAGE ${PROJECT_NAME})
# Create the sqeazy-config.cmake and sqeazy-config-version files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_SHARE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(sqeazy-config.cmake.in
  "${PROJECT_BINARY_DIR}/sqeazy-config.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${SQEAZY}/${REL_INCLUDE_DIR}")
configure_file(sqeazy-config.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/sqeazy-config.cmake" @ONLY)
# ... for both
configure_file(sqeazy-config-version.cmake.in
  "${PROJECT_BINARY_DIR}/sqeazy-config-version.cmake" @ONLY)

# Install the sqeazy-config.cmake and sqeazy-config-version.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/sqeazy-config.cmake"
  "${PROJECT_BINARY_DIR}/sqeazy-config-version.cmake"
  DESTINATION "${INSTALL_SHARE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree 
install(EXPORT sqeazy-targets 
  DESTINATION "${INSTALL_SHARE_DIR}" COMPONENT dev)





