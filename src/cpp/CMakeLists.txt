# cmake compatibility issues
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)


# project name
PROJECT(sqeazy VERSION ${SQEAZY_VERSION} LANGUAGES CXX)

# this project is C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)


if(NOT(${CMAKE_VERSION} VERSION_LESS "3.3.0"))
cmake_policy(SET CMP0054 NEW)
endif()


include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)


# Project information
SET (SQEAZY_NAME "Sqeazy - Fast and Flexible Volume Compression Library")
SET (SQEAZY_CODENAME "${PROJECT_NAME}")
SET (SQEAZY_COPYRIGHT_YEARS "2016")

#extract the version from the git tree
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )
include(git_utils)

git_describe(SQEAZY_VERSION --tags --abbrev=0)
git_describe(SQEAZY_VERSION_HEADREF --always)
string(REPLACE "." ";" SQEAZY_VERSION_ASLIST ${SQEAZY_VERSION})
list(GET SQEAZY_VERSION_ASLIST 0 SQEAZY_VERSION_MAJOR)
list(GET SQEAZY_VERSION_ASLIST 1 SQEAZY_VERSION_MINOR)
list(GET SQEAZY_VERSION_ASLIST 2 SQEAZY_VERSION_PATCH)

message(STATUS "sqeazy version: ${SQEAZY_VERSION_MAJOR}.${SQEAZY_VERSION_MINOR}.${SQEAZY_VERSION_PATCH} (${SQEAZY_VERSION_HEADREF})")

SET (SQEAZY_VENDOR_ID "mpi cbg")
SET (SQEAZY_VENDOR_NAME "Max Planck Institute of Molecular Cell Biology and Genetics ")
SET (SQEAZY_VENDOR_URL "www.mpi-cbg.de")
SET (SQEAZY_ID "${SQEAZY_VENDOR_ID}.${PROJECT_NAME}")

option(BUNDLE_SQEAZY "compile sqeazy ready for bundling" OFF)
option(WITH_MARCH_NATIVE "compile sqeazy for the currently found hardware" OFF)
option(USE_CXX11_ABI "enable _GLIBCXX_USE_CXX11_ABI in GCC 5.0+" ON)

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
ELSE(UNIX)

  IF(WIN32 AND NOT CYGWIN)
    set(INSTALL_SHARE_DIR CMake CACHE PATH "Installation directory for shared files")
  ELSE(WIN32 AND NOT CYGWIN)
    MESSAGE(FATAL_ERROR "UNKNOWN ARCHITECTURE .. unable to set share dir")
  ENDIF(WIN32 AND NOT CYGWIN)

ENDIF(UNIX)

# Make relative paths absolute (needed later on)
foreach(p BIN LIB INCLUDE SHARE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()


# set project build type
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
MESSAGE(STATUS "Setting up ${CMAKE_BUILD_TYPE} build")
string(TOUPPER "${CMAKE_BUILD_TYPE}" ALLCAPITAL_BUILD_TYPE)


check_cxx_compiler_flag(-Wl,-Bsymbolic HAS_BSYMBOLIC_COMPILERFLAG)
check_cxx_compiler_flag(-ftree-vectorize HAS_GCC_TREE_VECTORIZE_COMPILERFLAG)
check_cxx_compiler_flag(-march=native HAS_MARCH_NATIVE_COMPILERFLAG)
check_cxx_compiler_flag(-march=generic HAS_MARCH_GENERIC_COMPILERFLAG)
check_cxx_compiler_flag(-ffast-math HAS_FAST_MATH_COMPILERFLAG)
check_cxx_compiler_flag("-Xclang -march=native" HAS_XCLANG_COMPILERFLAG)
check_cxx_compiler_flag(-fvectorize HAS_CLANG_VECTORIZE_COMPILERFLAG)
check_cxx_compiler_flag(-ffast-math HAS_FAST_MATH_COMPILERFLAG)
check_cxx_compiler_flag(-Wall HAS_WALL_COMPILERFLAG)
check_cxx_compiler_flag(-Wfatal-errors HAS_WFATAL_ERRORS_COMPILERFLAG)
check_cxx_compiler_flag(-Wno-unused-function HAS_WNO_UNUSED_FUNCTION_COMPILERFLAG)
check_cxx_compiler_flag(-ggdb HAS_GGDB_COMPILERFLAG)
check_cxx_compiler_flag(-ggdb3 HAS_GGDB3_COMPILERFLAG)
check_cxx_compiler_flag(-O3 HAS_O3_COMPILERFLAG)
check_cxx_compiler_flag(-Ofast HAS_OFAST_COMPILERFLAG)
check_cxx_compiler_flag(-flto HAS_LTO_COMPILERFLAG)
check_cxx_compiler_flag(-msse HAS_MSSE_COMPILERFLAG)
check_cxx_compiler_flag(-msse2 HAS_MSSE2_COMPILERFLAG)
check_cxx_compiler_flag(-msse3 HAS_MSSE3_COMPILERFLAG)
check_cxx_compiler_flag(-msse4 HAS_MSSE4_COMPILERFLAG)
check_cxx_compiler_flag(-msse4.1 HAS_MSSE4.1_COMPILERFLAG)
check_cxx_compiler_flag(-msse4.2 HAS_MSSE4.2_COMPILERFLAG)
check_cxx_compiler_flag(-mssse3 HAS_MSSSE3_COMPILERFLAG)

check_cxx_compiler_flag(/Oi HAS_OI_COMPILERFLAG)
check_cxx_compiler_flag(/w HAS_NO_WARNINGS) #if given to cl.exe, no output will be printed to the console
check_cxx_compiler_flag(/W1 HAS_VS_W1_WARNINGS) #see https://msdn.microsoft.com/en-us/library/thxezb7y.aspx for details
check_cxx_compiler_flag(-Wno-overflow HAS_WNOOVERFLOW_COMPILERFLAG)

#C/C++ specific flags
#for GCC
IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "5.0")
    set(USE_CXX11_ABI ON)
  endif()
  if(${USE_CXX11_ABI})
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
    message(">> [GCC ${CMAKE_CXX_COMPILER_VERSION}] adding -D_GLIBCXX_USE_CXX11_ABI=1")
  else()
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    message(">> [GCC ${CMAKE_CXX_COMPILER_VERSION}] adding -D_GLIBCXX_USE_CXX11_ABI=1")
  endif()

  if(ALLCAPITAL_BUILD_TYPE STREQUAL DEBUG)
    if(HAS_GGDB3_COMPILERFLAG)
      add_definitions(-ggdb3)
    endif()
  endif()

  # if(ALLCAPITAL_BUILD_TYPE STREQUAL RELWDBG OR ALLCAPITAL_BUILD_TYPE STREQUAL RELWITHDEBINFO)
  #   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO -O2 -ggdb)

  #   if(HAS_GGDB_COMPILERFLAG)
  #     add_definitions(-ggdb)
  #   endif()

  #   if(HAS_O3_COMPILERFLAG)
  #     add_definitions(-O2)
  #   endif()
  # endif()

ENDIF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)

if(HAS_WALL_COMPILERFLAG)
  add_definitions(-Wall)
endif()

if(HAS_WFATAL_ERRORS_COMPILERFLAG)
  add_definitions(-Wfatal-errors)
endif()

if(HAS_WNO_UNUSED_FUNCTION_COMPILERFLAG)
  add_definitions(-Wno-unused-function)
endif()


if(HAS_WNOOVERFLOW_COMPILERFLAG)
  add_definitions(-Wno-overflow)
endif()

if(HAS_VS_W1_WARNINGS)
  add_definitions(/W1)
endif()

if(ALLCAPITAL_BUILD_TYPE STREQUAL DEBUG)
  if(HAS_GGDB3_COMPILERFLAG)
    add_definitions(-ggdb3)
  endif()
  IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    if(HAS_OI_COMPILERFLAG)
      add_definitions(/Oi)
    endif()

  endif()
  
  if(NOT MSVC)
    add_definitions(-DSQY_TRACE=1)
    ADD_DEFINITIONS(-D_SQY_DEBUG_=1)
  else()
    add_definitions(/DSQY_TRACE=1)
    ADD_DEFINITIONS(/D_SQY_DEBUG_=1)
  endif()
endif()

if(ALLCAPITAL_BUILD_TYPE STREQUAL RELWDBG)
  if(HAS_GGDB_COMPILERFLAG)
    add_definitions(-ggdb)
  endif()

  if(HAS_O3_COMPILERFLAG)
    add_definitions(-O2)
  endif()
endif()

if(ALLCAPITAL_BUILD_TYPE STREQUAL RELEASE)
  if(HAS_OFAST_COMPILERFLAG)
    add_definitions(-Ofast)
  endif()
  add_definitions(-DNDEBUG)
endif()

 #for clang
# IF("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")
#    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")

#    SET(CMAKE_CXX_FLAGS_RELWDBG "${CMAKE_CXX_FLAGS_RELWDBG} -Wall -ggdb -O3 -Wno-overflow")
#    SET(CMAKE_C_FLAGS_RELWDBG "${CMAKE_C_FLAGS_RELWDBG} -Wall -ggdb -O3 -Wno-overflow")

#  ENDIF()

 #TODO: think about support parallel studio as well
 IF(WIN32)
   MESSAGE(STATUS "[Win32] Compiler used C++ ${CMAKE_CXX_COMPILER_ID}")
   MESSAGE(STATUS "[Win32] Compiler used C ${CMAKE_C_COMPILER_ID}")
   MESSAGE(STATUS "[Win32] default HOST_COMPILER_RELEASE_FLAGS : ${HOST_COMPILER_RELEASE_FLAGS}")
   MESSAGE(STATUS "[Win32] CMAKE_CXX_FLAGS          : ${CMAKE_CXX_FLAGS}")
   MESSAGE(STATUS "[Win32] CMAKE_CXX_FLAGS_RELEASE  : ${CMAKE_CXX_FLAGS_RELEASE}")
   MESSAGE(STATUS "[Win32] CMAKE_CXX_FLAGS_DEBUG    : ${CMAKE_CXX_FLAGS_DEBUG}")
   MESSAGE(STATUS "[Win32] CMAKE_VS_PLATFORM_TOOLSET: ${CMAKE_VS_PLATFORM_TOOLSET}")
   MESSAGE(STATUS "[Win32] CMAKE_VS_PLATFORM_TOOLSET: ${CMAKE_VS_PLATFORM_TOOLSET}")
   MESSAGE(STATUS "[Win32] CMAKE_MODULE_LINKER_FLAGS: ${CMAKE_MODULE_LINKER_FLAGS}")
 ENDIF(WIN32)


IF(HAS_CLANG_VECTORIZE_COMPILERFLAG)
  add_definitions(-fvectorize)
ENDIF()

IF(HAS_GCC_TREE_VECTORIZE_COMPILERFLAG)
  add_definitions(-ftree-vectorize)
ENDIF()



IF(HAS_MARCH_NATIVE_COMPILERFLAG)
  if(${WITH_MARCH_NATIVE})
    add_definitions(-march=native)
    MESSAGE(STATUS "allowing compiler to choose x86 architecture, using -march=native")
  else()
    #check https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html for details
    set(MY_RELEASE_FLAG "")
    if(HAS_MARCH_GENERIC_COMPILERFLAG)
      set(MY_RELEASE_FLAG "-march=generic")
    endif()
    set(SSE_FLAGS "MSSE;MSSE2;MSSE3;MSSSE3;MSSE4;MSSE4.1;MSSE4.2")
    foreach(_flag IN LISTS REQUIRED_H5_TARGETS)
      string(TOLOWER _compiler_flag "-${_flag}")
      if(HAS_${_flag}_COMPILERFLAG)
        set(MY_RELEASE_FLAG "${MY_RELEASE_FLAG} ${_compiler_flag}")
      endif()
    endforeach()
    add_definitions(${MY_RELEASE_FLAG})
    MESSAGE(STATUS "using ${MY_RELEASE_FLAG} as build flag")
  endif()
ELSE()
  MESSAGE(WARNING "Compiler does not support -march=native!")
ENDIF()

#TODO: look into preference of libc++ over other std libraries
IF( CMAKE_GENERATOR STREQUAL Xcode )
    SET( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11" )
    SET( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
ENDIF()

if(SQY_VERBOSE)
  ADD_DEFINITIONS(-D_SQY_VERBOSE_=1)
endif(SQY_VERBOSE)


IF(HAS_FAST_MATH_COMPILERFLAG)
  #SET(HOST_COMPILER_RELEASE_FLAGS "${HOST_COMPILER_RELEASE_FLAGS} -ffast-math")
  add_definitions(-ffast-math)
ENDIF()



IF(SQY_EXPERIMENTAL)
  add_definitions(-D_SQY_X_=1)
  MESSAGE(STATUS "[sqy] appended experimental flags: -D_SQY_X_=1")
else(SQY_EXPERIMENTAL)
  add_definitions(-D_SQY_X_=0)
endif(SQY_EXPERIMENTAL)

#the following shouldn't be needed due to CMAKE_CXX_STANDARD variable
#check_cxx_compiler_flag(-std=c++11 HAS_CPP11_COMPILERFLAG)

#lz4 is needed for the entire project, so let's include it here


if(${BUNDLE_SQEAZY})
  #implies that all find_package calls will primarily look for static libs
  set(BUILD_SHARED_LIBS OFF)
  set(Boost_USE_STATIC_LIBS ON)
  #TODO: unclear if that might also hold for UNIX derivates
  if(WIN32) #was MSVC
    set(Boost_USE_STATIC_RUNTIME ON)
  endif()
  set(LZ4_USE_STATIC_LIBS ON)
  set(HDF5_USE_STATIC_LIBRARIES ON)
  set(FFMPEG_USE_STATIC_LIBS ON)
  set(OpenMP_LINK_STATIC ON)
endif()

################################# DEPENDENCIES #################################
#-------------------------------- OpenMP          ---------------------------------
find_package(OpenMP++ REQUIRED)

if(${OPENMP_CXX_FOUND})
  # add_definitions(${OpenMP_CXX_FLAGS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP++_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP++_FLAGS}")
  MESSAGE(STATUS "[sqy] appended openmp flags: ${OpenMP++_FLAGS}")
ENDIF()

#-------------------------------- LZ4          ---------------------------------
FIND_PACKAGE(LZ4 REQUIRED # QUIET
  )
if(LZ4_FOUND)
  INCLUDE_DIRECTORIES(${LZ4_INCLUDE_DIRS})
  LINK_DIRECTORIES(${LZ4_LIBRARY_DIRS})
  MESSAGE("++ lz4 found at ${LZ4_INCLUDE_DIRS} ${LZ4_LIBRARY_DIRS} : ${LZ4_LIBRARY}")
else()
  MESSAGE(FATAL_ERROR "++ lz4 not found!")
endif()

#-------------------------------- BOOST        ---------------------------------
IF(WIN32)
  SET(Boost_USE_STATIC_LIBS ON)
  SET(Boost_USE_MULTITHREADED ON)
  if(NOT DEFINED BUNDLE_SQEAZY OR NOT ${BUNDLE_SQEAZY})
    set(Boost_USE_STATIC_RUNTIME    OFF)
  endif()
ENDIF()


FIND_PACKAGE (Boost 1.55 COMPONENTS program_options regex  filesystem  serialization REQUIRED QUIET)
IF(Boost_FOUND)
  MESSAGE("++ Boost found at ${Boost_LIBRARY_DIR} ${Boost_LIBRARIES} : ${Boost_LIBRARIES}")
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
else()
  MESSAGE(FATAL_ERROR "++ Boost not found!")
endif()

#-------------------------------- HDF5         ---------------------------------
#needed for FindHDF5 as it expects an ENVIRONMENT variable called HDF5_ROOT as hint to the HDF5 path
IF(EXISTS ${HDF5_DIR})
  set(HDF5_ROOT ${HDF5_DIR})
ENDIF()

IF(EXISTS $ENV{HDF5_DIR})
  set(HDF5_ROOT $ENV{HDF5_DIR})
ENDIF()


IF(EXISTS $ENV{HDF5_ROOT})
  set(HDF5_ROOT $ENV{HDF5_ROOT})
endif()

set(HDF5_NEW_FINDMODULE False)

IF(# EXISTS ${HDF5_ROOT} AND
    IS_DIRECTORY "${HDF5_ROOT}")

	set($ENV{HDF5_ROOT} ${HDF5_ROOT})
	set($ENV{HDF5_DIR} ${HDF5_ROOT})
	set(HDF5_DIR ${HDF5_ROOT})
	set($ENV{HDF5_ROOT_DIR_HINT} ${HDF5_ROOT})

	if(EXISTS ${HDF5_ROOT}/share/cmake/FindHDF5.cmake)
		message(STATUS "found embedded share/cmake/FindHDF5.cmake in ${HDF5_ROOT}, adding it to CMAKE_MODULE_PATH")
		list(APPEND CMAKE_MODULE_PATH "${HDF5_ROOT}/share/cmake")
		list(APPEND CMAKE_PREFIX_PATH "${HDF5_ROOT}/share/cmake")
		set(HDF5_NEW_FINDMODULE True)
	endif()

	if(EXISTS ${HDF5_ROOT}/cmake/FindHDF5.cmake AND IS_DIRECTORY ${HDF5_ROOT}/cmake)
		message(STATUS "found embedded cmake/FindHDF5.cmake in ${HDF5_ROOT}, adding it to CMAKE_MODULE_PATH")
		list(APPEND CMAKE_MODULE_PATH "${HDF5_ROOT}/cmake")
		list(APPEND CMAKE_PREFIX_PATH "${HDF5_ROOT}/cmake")
		set(HDF5_NEW_FINDMODULE True)
	endif()

	if(IS_DIRECTORY ${HDF5_ROOT})
		message(STATUS "calling FindHDF5 with ${HDF5_ROOT} (HDF5_NEW_FINDMODULE: ${HDF5_NEW_FINDMODULE})\n-- CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}\n-- CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
	else()
		message(FATAL_ERROR "calling FindHDF5 with non-existing ${HDF5_ROOT}")
	endif()

ELSE()
	message(STATUS "HDF5_ROOT (${HDF5_ROOT}) not found")
ENDIF()


if(${HDF5_USE_STATIC_LIBRARIES})
  set(FIND_HDF5_MODE static)
else()
  set(FIND_HDF5_MODE shared)
endif()

FIND_PACKAGE(HDF5 1.8.15 QUIET REQUIRED COMPONENTS CXX HL)
if(NOT "${HDF5_NEW_FINDMODULE}" MATCHES "False")
  #unset(HDF5_FOUND)
  message(STATUS "detected FindHDF5.cmake introduced with 1.8.16")
  #FIND_PACKAGE(HDF5 ${HDF5_VERSION} NAMES hdf5 COMPONENTS CXX HL ${FIND_HDF5_MODE})
  FIND_PACKAGE(HDF5 1.8.15 QUIET REQUIRED COMPONENTS CXX HL ${FIND_HDF5_MODE} NAMES hdf5 )
else()
  message(STATUS "FindHDF5.cmake undetected below ${HDF5_ROOT}, ${HDF5_VERSION}, (HDF5_NEW_FINDMODULE: ${HDF5_NEW_FINDMODULE})")
endif()

IF(HDF5_FOUND)

  if(NOT DEFINED HDF5_INCLUDE_DIRS # OR NOT IS_DIRECTORY ${HDF5_INCLUDE_DIRS}
      )

    find_file   (HDF5_H5PUBLIC_H "H5public.h"
      HINTS ${HDF5_ROOT}
      PATHS ${HDF5_ROOT}
      PATH_SUFFIXES
      include
      )
    if(EXISTS ${HDF5_H5PUBLIC_H})
      get_filename_component(HDF5_INCLUDE_DIRS "${HDF5_H5PUBLIC_H}" DIRECTORY)
    endif()
  endif()


  INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})

  #only required for hdf5 1.8.16 and newer
  #https://lists.hdfgroup.org/pipermail/hdf-forum_lists.hdfgroup.org/2016-March/009294.html

  if(${FIND_HDF5_MODE} MATCHES "shared")
    message(STATUS "adding preprocessor definition H5_BUILT_AS_DYNAMIC_LIB=1")
    if(MSVC)
      add_definitions(/DH5_BUILT_AS_DYNAMIC_LIB=1)
    else()
      add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB=1)
    endif()
  endif()

  #to discard compiler warnings in Debug mode
  IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    SET(FORTIFY_INDEX "")
    list(FIND HDF5_DEFINITIONS "-D_FORTIFY_SOURCE=2" FORTIFY_INDEX)

    IF(${FORTIFY_INDEX} GREATER -1)
      list(REMOVE_AT HDF5_DEFINITIONS ${FORTIFY_INDEX})
      list(APPEND HDF5_DEFINITIONS -D_FORTIFY_SOURCE=0)
    ENDIF()

  ENDIF()

  ADD_DEFINITIONS(${HDF5_DEFINITIONS})
  if(DEFINED HDF5_LIBRARY_DIRS)
    LINK_DIRECTORIES(${HDF5_LIBRARY_DIRS})
  else()
    if(IS_DIRECTORY ${HDF5_ROOT}/lib)
      LINK_DIRECTORIES(${HDF5_ROOT}/lib)
    endif()
  endif()

  if(NOT HDF5_LIBRARIES)
    message(STATUS "FindHDF5 didn't fill HDF5_LIBRARIES (${HDF5_LIBRARIES}), trying to fill it based on known targets")
    set(REQUIRED_H5_TARGETS hdf5_hl_cpp;hdf5_hl;hdf5_cpp;hdf5)
    foreach(_stub IN LISTS REQUIRED_H5_TARGETS)
      if(TARGET ${_stub}-${FIND_HDF5_MODE})
        list(APPEND HDF5_LIBRARIES ${_stub}-${FIND_HDF5_MODE})
      else()
        message(WARNING "${_stub}-${FIND_HDF5_MODE} not known as possible target")
      endif()
    endforeach()
  endif()


  if(DEFINED UNIX AND DEFINED HDF5_USE_STATIC_LIBRARIES)
    #many Linux distros distribute static libs without -fPIC
    #if they are linked into libsqeazy, an error will be thrown
    set(HDF5_LIBRARIES_CLONE ${HDF5_LIBRARIES})
    foreach(_H5_LIB IN LISTS HDF5_LIBRARIES_CLONE)
      if(${_H5_LIB} MATCHES ".*lib(m|dl).a$")
        message(STATUS "removing ${_H5_LIB} from HDF5_LIBRARIES")
        list(REMOVE_ITEM HDF5_LIBRARIES ${_H5_LIB})
        get_filename_component(_H5_FSTEM ${_H5_LIB} NAME_WE)
        string(REGEX REPLACE "^lib" "" _H5_LIBID ${_H5_FSTEM})
        list(APPEND HDF5_LIBRARIES ${_H5_LIBID})

      endif()
    endforeach()

    list(REMOVE_DUPLICATES HDF5_LIBRARIES)
  endif()
  MESSAGE("++ hdf5 found at ${HDF5_INCLUDE_DIRS} ${HDF5_LIBRARY_DIRS} : ${HDF5_LIBRARIES}")
ELSE()
  MESSAGE(FATAL_ERROR "++ HDF5 not found!")
ENDIF()

#-------------------------------- FFMPEG       ---------------------------------

find_package(FFMPEG REQUIRED COMPONENTS avformat avcodec avutil swscale)
IF(FFMPEG_FOUND)
  INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
  LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)

  MESSAGE("++ FFMPEG found at ${FFMPEG_INCLUDE_DIRS} ${FFMPEG_ROOT_DIR}/lib ${FFMPEG_LIBRARIES}")
  if(DEFINED FFMPEG_USE_STATIC_LIBS OR DEFINED FFMPEG_EXTRA_LINK_FLAGS)
    MESSAGE("++                 ${FFMPEG_EXTRA_LINK_FLAGS}")
  endif()
ELSE()
  MESSAGE(WARNING "++ FFMPEG not found!")
ENDIF()


#-------------------------------- TIFF       ---------------------------------

if(TIFF_ROOT OR DEFINED TIFF_ROOT)
    if(EXISTS ${TIFF_ROOT})
        list(APPEND CMAKE_PREFIX_PATH ${TIFF_ROOT})
        set(TIFF_INCLUDE_DIR ${TIFF_ROOT}/include)
    else()
        message(FATAL_ERROR "provided TIFF_ROOT ${TIFF_ROOT} does not exist")
    endif()
endif()

# if(DEFINED TIFF_ROOT)
    # if(EXISTS ${TIFF_ROOT})
      # list(APPEND CMAKE_PREFIX_PATH ${TIFF_ROOT})
    # else()
      # message(FATAL_ERROR "provided TIFF_ROOT ${TIFF_ROOT} does not exist")
    # endif()
# endif()


FIND_PACKAGE(TIFF REQUIRED)
IF(TIFF_FOUND)
  include_directories(${TIFF_INCLUDE_DIR})
  get_filename_component(TIFF_LIB_PATH ${TIFF_LIBRARY} ABSOLUTE)
  get_filename_component(TIFF_LIB_ABSPATH ${TIFF_LIB_PATH} DIRECTORY)
  link_directories(${TIFF_LIB_ABSPATH})

  SET(TIFF_LIB_FOR_SQY ${TIFF_LIBRARIES})
  SET(TIFF_STATIC_LIB "")

  if(NOT EXISTS ${TIFF_LIB_FOR_SQY})
    message(FATAL_ERROR "libtiff found by cmake does not exist! Aborting.")
  else()
    message("++ [tiff] using ${TIFF_INCLUDE_DIR} ${TIFF_LIB_FOR_SQY}")
  endif()

  IF(NOT WIN32)

    IF(APPLE)
      IF("${TIFF_LIBRARY}" MATCHES ".*.so$")
    STRING(REPLACE .so .dylib TIFF_LIB_FOR_SQY ${TIFF_LIBRARY})
    #Homebrew ships a static library that fails to contain important symbols
    #STRING(REPLACE .dylib .a TIFF_STATIC_LIB ${TIFF_LIB_FOR_SQY})
      ENDIF()
    ELSE()
      STRING(REPLACE .so .a TIFF_STATIC_LIB ${TIFF_LIB_FOR_SQY})
    ENDIF()

    #FIXME: static includes of libtiff showed missing symbols (mostly deflate, there must be a depenency to libz or something)
    # IF(EXISTS ${TIFF_STATIC_LIB})
    #   add_library(ext_tiff STATIC IMPORTED)
    #   set_target_properties(ext_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_STATIC_LIB}")
    # ELSE()
      IF(EXISTS ${TIFF_LIB_FOR_SQY})
    add_library(ext_tiff SHARED IMPORTED)
    set_target_properties(ext_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_LIB_FOR_SQY}")
      ENDIF()
    # ENDIF()

  ENDIF(NOT WIN32)

ENDIF(TIFF_FOUND)




################################## add source ##################################

IF(Boost_FOUND AND HDF5_FOUND AND LZ4_FOUND AND FFMPEG_FOUND AND TIFF_FOUND)

  #setup headers
  if(NOT IS_DIRECTORY ${PROJECT_BINARY_DIR}/inc)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/inc)
  endif()
  configure_file(${PROJECT_SOURCE_DIR}/inc/sqeazy_definitions.h.in ${PROJECT_BINARY_DIR}/inc/sqeazy_definitions.h)
  file(COPY ${PROJECT_SOURCE_DIR}/inc/sqeazy.h ${PROJECT_SOURCE_DIR}/inc/sqeazy_h5_filter.hpp ${PROJECT_SOURCE_DIR}/inc/H5PLextern.h DESTINATION ${PROJECT_BINARY_DIR}/inc/)

  include_directories("${PROJECT_BINARY_DIR}/inc")

  #library and app
  ADD_SUBDIRECTORY(src) # source code of the library

  #tests
  ADD_SUBDIRECTORY(tests)
  enable_testing()
  include("CTestLists.txt")

  find_package(GoogleBenchmark REQUIRED)
    if(${googlebenchmark_FOUND})
        add_subdirectory(bench)
    else()
        message("google/benchmark not found, skipping benchmark suite")
    endif()

ELSE()
  MESSAGE(FATAL_ERROR "++ required dependencies not found, sqeazy cannot be build")
ENDIF()

get_directory_property( global_defs COMPILE_DEFINITIONS )
get_directory_property( src_defs DIRECTORY ${CMAKE_SOURCE_DIR}/src COMPILE_DEFINITIONS )
get_directory_property( tests_defs DIRECTORY ${CMAKE_SOURCE_DIR}/tests COMPILE_DEFINITIONS )
MESSAGE(STATUS "using ${CMAKE_BUILD_TYPE} build flags: CMAKE_CXX_FLAGS_${ALLCAPITAL_BUILD_TYPE} = ${CMAKE_CXX_FLAGS_${ALLCAPITAL_BUILD_TYPE}}\n\tglobal\t${global_defs} \n\tsrc\t${src_defs} \n\ttests\t${tests_defs}")

################################ EXPORT/INSTALL ################################

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
