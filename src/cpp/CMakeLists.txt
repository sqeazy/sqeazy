# cmake compatibility issues
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)


# project name
PROJECT(sqeazy CXX)

# this project is C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)


if(NOT(${CMAKE_VERSION} VERSION_LESS "3.0.0"))
cmake_policy(SET CMP0042 NEW)
endif()

if(NOT(${CMAKE_VERSION} VERSION_LESS "3.3.0"))
cmake_policy(SET CMP0054 NEW)
endif()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
 
# Project information
SET (SQEAZY_NAME "Sqeazy - Fast and Flexible Volume Compression Library")
SET (SQEAZY_CODENAME "${PROJECT_NAME}")
SET (SQEAZY_COPYRIGHT_YEARS "2016")
SET (SQEAZY_VERSION_MAJOR 0)
SET (SQEAZY_VERSION_MINOR 2)
SET (SQEAZY_VERSION_PATCH 1)
SET (SQEAZY_VERSION "${SQEAZY_VERSION_MAJOR}.${SQEAZY_VERSION_MINOR}.${SQEAZY_VERSION_PATCH}")
SET (SQEAZY_VENDOR_ID "mpi cbg")
SET (SQEAZY_VENDOR_NAME "Max Planck Institute of Molecular Cell Biology and Genetics ")
SET (SQEAZY_VENDOR_URL "www.mpi-cbg.de")
SET (SQEAZY_ID "${SQEAZY_VENDOR_ID}.${PROJECT_NAME}")

option(READY_4_BUNDLE "compile sqeazy ready for bundling" OFF)

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

check_cxx_compiler_flag(-Wl,-Bsymbolic HAS_BSYMBOLIC_COMPILERFLAG)
check_cxx_compiler_flag(-ftree-vectorize HAS_GCC_TREE_VECTORIZE_COMPILERFLAG)
check_cxx_compiler_flag(-march=native HAS_MARCH_COMPILERFLAG)
check_cxx_compiler_flag(-ffast-math HAS_FAST_MATH_COMPILERFLAG)
check_cxx_compiler_flag("-Xclang -march=native" HAS_XCLANG_COMPILERFLAG)
check_cxx_compiler_flag(-fvectorize HAS_CLANG_VECTORIZE_COMPILERFLAG)
check_cxx_compiler_flag(-ffast-math HAS_FAST_MATH_COMPILERFLAG)

#C/C++ specific flags
#for GCC
IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")
ENDIF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
 
 #for clang
IF("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")
   SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -ggdb3 -Wno-overflow")
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

MESSAGE(STATUS "[Win32] CMAKE_CXX_FLAGS          : ${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "[Win32] CMAKE_CXX_FLAGS_RELEASE  : ${CMAKE_CXX_FLAGS_RELEASE}")
MESSAGE(STATUS "[Win32] CMAKE_CXX_FLAGS_DEBUG    : ${CMAKE_CXX_FLAGS_DEBUG}")
MESSAGE(STATUS "[Win32] CMAKE_VS_PLATFORM_TOOLSET: ${CMAKE_VS_PLATFORM_TOOLSET}")
#SET(HOST_COMPILER_RELEASE_FLAGS "/Ox /MD /W3 /EHsc")
#MESSAGE(FATAL "Microsoft Visual Studio not supported! Please use mingw-w64!")
ENDIF()

MESSAGE(STATUS "[Win32] CMAKE_VS_PLATFORM_TOOLSET: ${CMAKE_VS_PLATFORM_TOOLSET}")
MESSAGE(STATUS "[Win32] CMAKE_MODULE_LINKER_FLAGS: ${CMAKE_MODULE_LINKER_FLAGS}")
ENDIF(WIN32)
ENDIF(UNIX)

IF(HAS_CLANG_VECTORIZE_COMPILERFLAG)
  SET(HOST_COMPILER_RELEASE_FLAGS "${HOST_COMPILER_RELEASE_FLAGS} -fvectorize")
ENDIF()

IF(HAS_GCC_TREE_VECTORIZE_COMPILERFLAG)
  SET(HOST_COMPILER_RELEASE_FLAGS "${HOST_COMPILER_RELEASE_FLAGS} -ftree-vectorize")
ENDIF()



IF(UNIX OR APPLE)
  IF(HAS_MARCH_COMPILERFLAG)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
  ELSE()
    MESSAGE(WARNING "Compiler does not support -march=native!")
  ENDIF()

  # if(HAS_BSYMBOLIC_COMPILERFLAG)
  #   SET(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -Wl,-Bsymbolic")
  #   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-Bsymbolic")
  # endif()
ENDIF()

#TODO: look into preference of libc++ over other std libraries
IF( CMAKE_GENERATOR STREQUAL Xcode )
    SET( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11" )
    SET( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
ENDIF()					     

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_definitions(-DSQY_TRACE=1)
  ADD_DEFINITIONS(-D_SQY_DEBUG_=1)
endif()

if(SQY_VERBOSE)
  ADD_DEFINITIONS(-D_SQY_VERBOSE_=1)
endif(SQY_VERBOSE)


IF(HAS_FAST_MATH_COMPILERFLAG)
  SET(HOST_COMPILER_RELEASE_FLAGS "${HOST_COMPILER_RELEASE_FLAGS} -ffast-math")
ENDIF()

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${HOST_COMPILER_RELEASE_FLAGS} ")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${HOST_COMPILER_RELEASE_FLAGS} ")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${HOST_COMPILER_DEBUG_FLAGS} ")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${HOST_COMPILER_DEBUG_FLAGS} ")

IF(CMAKE_BUILD_TYPE STREQUAL Release)
  MESSAGE(STATUS "using ${CMAKE_BUILD_TYPE} build flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
ELSE()
  MESSAGE(STATUS "using ${CMAKE_BUILD_TYPE} build flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
ENDIF()

IF(SQY_EXPERIMENTAL) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SQY_X_=1")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -D_SQY_X_=1")
MESSAGE(STATUS "appended experimental flags: ${CMAKE_CXX_FLAGS}")
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

if(${READY_4_BUNDLE})
  #implies that all find_package calls will primarily look for static libs
  set(BUILD_SHARED_LIBS OFF)
  set(Boost_USE_STATIC_LIBS ON)
  set(LZ4_USE_STATIC_LIBS ON)
  set(HDF5_USE_STATIC_LIBRARIES ON)
  set(FFMPEG_USE_STATIC_LIBS ON)
  
  # if(NOT APPLE)
  #   set(FFMPEG_IGNORE_PKG_CONFIG ON)
  # endif()
endif()

################################# DEPENDENCIES #################################

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
  set(Boost_USE_STATIC_RUNTIME    OFF)
ENDIF()


FIND_PACKAGE (Boost 1.55 COMPONENTS program_options regex system filesystem unit_test_framework REQUIRED QUIET)
IF(Boost_FOUND)
  MESSAGE("++ Boost found at ${Boost_LIBRARY_DIR} ${Boost_LIBRARIES} : ${Boost_LIBRARIES}")
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
else()
  MESSAGE(FATAL_ERROR "++ Boost not found!")
endif()

#-------------------------------- HDF5         ---------------------------------
#needed for FindHDF5 as it expects an ENVIRONMENT variable called HDF5_ROOT as hint to the HDF5 path
IF(EXISTS ${HDF5_ROOT})
  SET(ENV{HDF5_ROOT} ${HDF5_ROOT})
ENDIF(EXISTS ${HDF5_ROOT})

FIND_PACKAGE(HDF5 1.8 QUIET REQUIRED CXX HL)
IF(HDF5_FOUND)
  INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
  #only required for hdf5 1.8.16 and newer
  #http://hdf-forum.184993.n3.nabble.com/Re-linking-HDF5-with-VS2015-C-64-bit-only-works-with-static-libraries-td4028678.html
  if(WIN32)
    add_definitions(/DH5_BUILT_AS_DYNAMIC_LIB=1)
  else()
    add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB=1)
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
  IF(DEFINED HDF5_LIBRARY_DIRS)
	LINK_DIRECTORIES(${HDF5_LIBRARY_DIRS}) 
  ENDIF()

  MESSAGE("++ hdf5 found at ${HDF5_INCLUDE_DIRS} ${HDF5_LIBRARY_DIRS} : ${HDF5_LIBRARIES}")
ELSE()
MESSAGE(FATAL_ERROR "++ HDF5 not found!")
ENDIF()

#-------------------------------- FFMPEG       ---------------------------------

find_package(FFMPEG REQUIRED COMPONENTS  avformat avcodec swscale avutil swresample)
IF(FFMPEG_FOUND)
  INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
  LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)

  MESSAGE("++ FFMPEG found at ${FFMPEG_INCLUDE_DIRS} ${FFMPEG_ROOT_DIR}/lib ${FFMPEG_LIBRARIES}")
ELSE()
  MESSAGE(WARNING "++ FFMPEG not found!")
ENDIF()


#-------------------------------- TIFF       ---------------------------------

if(TIFF_ROOT)
	if(EXISTS ${TIFF_ROOT})
		list(APPEND CMAKE_PREFIX_PATH ${TIFF_ROOT})
	else()
		message(FATAL_ERROR "provided TIFF_ROOT ${TIFF_ROOT} does not exist")
	endif()
endif()

if(DEFINED TIFF_ROOT)
	if(EXISTS ${TIFF_ROOT})
	  list(APPEND CMAKE_PREFIX_PATH ${TIFF_ROOT})
	else()
	  message(FATAL_ERROR "provided TIFF_ROOT ${TIFF_ROOT} does not exist")
	endif()
endif()

FIND_PACKAGE(TIFF REQUIRED QUIET)
IF(TIFF_FOUND)
  include_directories(${TIFF_INCLUDE_DIR})
  get_filename_component(TIFF_LIB_PATH ${TIFF_LIBRARY} ABSOLUTE)
  get_filename_component(TIFF_LIB_ABSPATH ${TIFF_LIB_PATH} DIRECTORY)
  link_directories(${TIFF_LIB_ABSPATH})
  
  SET(TIFF_DYN_LIB ${TIFF_LIBRARY})
  SET(TIFF_STATIC_LIB "")
  
  IF(WIN32)
    message("++ [tests] using ${TIFF_INCLUDE_DIR} ${TIFF_LIBRARY}")
	
  ELSE(WIN32)

    IF(APPLE)
      IF("${TIFF_LIBRARY}" MATCHES ".*.so$")
	STRING(REPLACE .so .dylib TIFF_DYN_LIB ${TIFF_LIBRARY})
	#Homebrew ships a static library that fails to contain important symbols
	#STRING(REPLACE .dylib .a TIFF_STATIC_LIB ${TIFF_DYN_LIB})
      ENDIF()
    ELSE()
      STRING(REPLACE .so .a TIFF_STATIC_LIB ${TIFF_DYN_LIB})
    ENDIF()

    #FIXME: static includes of libtiff showed missing symbols (mostly deflate, there must be a depenency to libz or something)
    # IF(EXISTS ${TIFF_STATIC_LIB})
    #   add_library(ext_tiff STATIC IMPORTED)
    #   set_target_properties(ext_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_STATIC_LIB}")
    # ELSE()
      IF(EXISTS ${TIFF_DYN_LIB})
	add_library(ext_tiff SHARED IMPORTED)
	set_target_properties(ext_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_DYN_LIB}")
      ENDIF()
    # ENDIF()
    
  ENDIF(WIN32)
  
ENDIF(TIFF_FOUND)




################################## add source ##################################

IF(Boost_FOUND AND HDF5_FOUND AND LZ4_FOUND AND FFMPEG_FOUND AND TIFF_FOUND)

  include_directories("${PROJECT_SOURCE_DIR}/inc")
  
  #library and app
  ADD_SUBDIRECTORY(src) # source code of the library

  #tests
  ADD_SUBDIRECTORY(tests)
  enable_testing()
  include("CTestLists.txt")
  
ELSE()
  MESSAGE(FATAL_ERROR "++ required dependencies not found, sqeazy cannot be build")
ENDIF()


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





