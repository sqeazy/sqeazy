# adapted from https://bitbucket.org/sergiu/libav-cmake/src/51fb1cd97ce401e0fdebe8452d81accfcb8bc8e3/FindLibAV.cmake?at=default
# Module for locating Ffmpeg.
#
# Customizable variables:
#   FFMPEG_ROOT_DIR
#     Specifies Ffmpeg's root directory.
#   FFMPEG_IGNORE_PKG_CONFIG
#     Do not use pkg_config even if availabe
#
# Read-only variables:
#   FFMPEG_FOUND
#     Indicates whether the library has been found.
#
#   FFMPEG_INCLUDE_DIR
#      Specifies Ffmpeg's include directory.
#
#   FFMPEG_USE_STATIC_LIBS
#      favor static libs over shared libraries
#
#   FFMPEG_LIBRARIES
#     Specifies Ffmpeg libraries that should be passed to target_link_libararies.
#
#   FFMPEG_<COMPONENT>_LIBRARIES
#     Specifies the libraries of a specific <COMPONENT>.
#
#   FFMPEG_<COMPONENT>_FOUND
#     Indicates whether the specified <COMPONENT> was found.
#
#
# Copyright (c) 2013, 2014 Sergiu Dotenco
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

INCLUDE (FindPackageHandleStandardArgs)
INCLUDE (FindPkgConfig)
INCLUDE (CheckFunctionExists)

#Check whether to search static or dynamic libs
set( CMAKE_FIND_LIBRARY_SUFFIXES_SAV ${CMAKE_FIND_LIBRARY_SUFFIXES} )

if( ${FFMPEG_USE_STATIC_LIBS} )
  # if(UNIX)
  #   LIST (APPEND FFMPEG_LIBRARIES x264;x265)
  # endif()

  set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX} )
else()
  set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX} )
endif()

IF (CMAKE_VERSION VERSION_GREATER 2.8.7)
  SET (_FFMPEG_CHECK_COMPONENTS FALSE)
ELSE (CMAKE_VERSION VERSION_GREATER 2.8.7)
  SET (_FFMPEG_CHECK_COMPONENTS TRUE)
ENDIF (CMAKE_VERSION VERSION_GREATER 2.8.7)

#added by psteinb
SET(EXT_FFMPEG_ROOT_DIR EXT_FFMPEG_ROOT_DIR-NOTFOUND)
IF (IS_DIRECTORY $ENV{FFMPEG_ROOT})
  SET(EXT_FFMPEG_ROOT_DIR $ENV{FFMPEG_ROOT})
ENDIF ()

IF (DEFINED FFMPEG_ROOT AND IS_DIRECTORY ${FFMPEG_ROOT})
  SET(EXT_FFMPEG_ROOT_DIR ${FFMPEG_ROOT})
ENDIF ()
#end of add


# FIND INCLUDE DIR #############################################################

FIND_PATH (FFMPEG_ROOT_DIR
  NAMES include/ffmpegcodec/avcodec.h
        include/ffmpegdevice/avdevice.h
        include/ffmpegfilter/avfilter.h
        include/ffmpegutil/avutil.h
        include/libswscale/swscale.h
  PATHS ${EXT_FFMPEG_ROOT_DIR}
  DOC "FFMPEG root directory")

 if(NOT FFMPEG_FIND_QUIETLY)
      message("** [FindFFMPEG] found root path ${FFMPEG_ROOT_DIR}")
endif()

  
FIND_PATH (FFMPEG_INCLUDE_DIR
  NAMES ffmpegcodec/avcodec.h
        ffmpegdevice/avdevice.h
        ffmpegfilter/avfilter.h
        ffmpegutil/avutil.h
        libswscale/swscale.h
  HINTS ${FFMPEG_ROOT_DIR}
  PATH_SUFFIXES include inc
  DOC "FFMPEG include directory")

if(NOT FFMPEG_FIND_QUIETLY)
      message("** [FindFFMPEG] found include path ${FFMPEG_INCLUDE_DIR}")
endif()
SET (FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIR})

# PREPARE COMPONENTS ###########################################################
 
  
if (NOT FFMPEG_FIND_COMPONENTS)

  set (FFMPEG_FIND_COMPONENTS avdevice avformat avfilter avcodec swresample swscale avutil postproc)
endif (NOT FFMPEG_FIND_COMPONENTS)

if(NOT FFMPEG_FIND_QUIETLY)
      message("** [FindFFMPEG] searching for ${FFMPEG_FIND_COMPONENTS}")
endif()

# PKG_CONFIG (Linux only) ######################################################
set(MY_PKGCONFIG_FINDARGS "")
if(FFMPEG_FIND_QUIETLY)
  set(MY_PKGCONFIG_FINDARGS QUIET)
endif()

if(FFMPEG_FIND_REQUIRED)
  set(MY_PKGCONFIG_FINDARGS ${MY_PKGCONFIG_FINDARGS} REQUIRED)
endif()

# FIND COMPONENTS ##############################################################

if(PKG_CONFIG_FOUND AND NOT FFMPEG_IGNORE_PKG_CONFIG)
  set(comp_with_fixed_names "")
  foreach(_COMP IN LISTS FFMPEG_FIND_COMPONENTS)
    if(NOT ${_COMP} MATCHES lib*)
      set(comp_with_fixed_names "${comp_with_fixed_names} lib${_COMP}")
    else()
      set(comp_with_fixed_names "${comp_with_fixed_names} ${_COMP}")
    endif()
  endforeach()

  pkg_search_module(LOCAL_FFMPEG ${MY_PKGCONFIG_FINDARGS} ${comp_with_fixed_names})
  
  if(LOCAL_FFMPEG_FOUND)

    foreach(_LDIR IN LISTS LOCAL_FFMPEG_LIBRARY_DIRS)
      link_directories(${_LDIR})
    endforeach()

    foreach(_IDIR IN LISTS LOCAL_FFMPEG_INCLUDE_DIRS)
      include_directories(${_IDIR})
    endforeach()

    foreach(_COMP ${FFMPEG_FIND_COMPONENTS})
      STRING (TOUPPER ${_COMP} _FFMPEG_COMPONENT_UPPER)
      
      if("${LOCAL_FFMPEG_LIBRARIES}" MATCHES ".*${_COMP}.*")
	SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND TRUE)
	
	list(FIND FFMPEG_LIBRARIES ${_COMP} _COMP_INDEX)
	list(GET FFMPEG_LIBRARIES ${_COMP_INDEX} FFMPEG_${_FFMPEG_COMPONENT_UPPER}_LIBRARY)

	MARK_AS_ADVANCED (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_LIBRARY)
      else()
	SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND FALSE)
	IF (_FFMPEG_CHECK_COMPONENTS)
	  LIST (APPEND _FFMPEG_MISSING_LIBRARIES ${_FFMPEG_LIBRARY_BASE})
	ENDIF (_FFMPEG_CHECK_COMPONENTS)
      endif()
      
    endforeach()
    
    LIST (APPEND FFMPEG_LIBRARIES ${LOCAL_FFMPEG_LIBRARIES})
    LIST (APPEND _FFMPEG_ALL_LIBS ${LOCAL_FFMPEG_LIBRARIES})
  else()
    foreach(_COMP ${FFMPEG_FIND_COMPONENTS})
      STRING (TOUPPER ${_FFMPEG_COMPONENT} _FFMPEG_COMPONENT_UPPER)
      SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND FALSE)
	IF (_FFMPEG_CHECK_COMPONENTS)
	  LIST (APPEND _FFMPEG_MISSING_LIBRARIES ${_FFMPEG_LIBRARY_BASE})
	endif()
  endif()
  
else(PKG_CONFIG_FOUND AND NOT FFMPEG_IGNORE_PKG_CONFIG)

  FOREACH (_FFMPEG_COMPONENT ${FFMPEG_FIND_COMPONENTS})
    STRING (TOUPPER ${_FFMPEG_COMPONENT} _FFMPEG_COMPONENT_UPPER)
    SET (_FFMPEG_LIBRARY_BASE FFMPEG_${_FFMPEG_COMPONENT_UPPER}_LIBRARY)


    #tries to obtain FFMPEG_COMPONENT_LIBRARY


    FIND_LIBRARY (${_FFMPEG_LIBRARY_BASE}
      NAMES ${_FFMPEG_COMPONENT}
      HINTS ${FFMPEG_ROOT_DIR}
      PATH_SUFFIXES bin lib
      DOC "Ffmpeg ${_FFMPEG_COMPONENT} library")

    MARK_AS_ADVANCED (${_FFMPEG_LIBRARY_BASE})

    SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND TRUE)

    if(NOT FFMPEG_FIND_QUIETLY)
      message("** [FindFFMPEG] found ${_FFMPEG_LIBRARY_BASE}, setting FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND")

    endif()
    
    #add what was found to global variables
    IF (${_FFMPEG_LIBRARY_BASE})
      # setup the FFMPEG_<COMPONENT>_LIBRARIES variable
      SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_LIBRARIES ${${_FFMPEG_LIBRARY_BASE}})
      LIST (APPEND FFMPEG_LIBRARIES ${FFMPEG_${_FFMPEG_COMPONENT_UPPER}_LIBRARIES})
      LIST (APPEND _FFMPEG_ALL_LIBS ${${_FFMPEG_LIBRARY_BASE}})
    ELSE (${_FFMPEG_LIBRARY_BASE})
      SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND FALSE)

      IF (_FFMPEG_CHECK_COMPONENTS)
	LIST (APPEND _FFMPEG_MISSING_LIBRARIES ${_FFMPEG_LIBRARY_BASE})
      ENDIF (_FFMPEG_CHECK_COMPONENTS)
    ENDIF (${_FFMPEG_LIBRARY_BASE})

    
    
    
    
    SET (FFMPEG_${_FFMPEG_COMPONENT}_FOUND ${FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND})
    
  ENDFOREACH (_FFMPEG_COMPONENT ${FFMPEG_FIND_COMPONENTS})
endif(PKG_CONFIG_FOUND AND NOT FFMPEG_IGNORE_PKG_CONFIG)

## POST_INCLUDES

if( ${FFMPEG_USE_STATIC_LIBS} )
  if(UNIX)
    
    LIST (APPEND FFMPEG_LIBRARIES x264;pthread;bz2;z;lzma;va;swresample;dl)
    find_library(X265_LIBRARY libx265${CMAKE_STATIC_LIBRARY_SUFFIX} x265)
    if(EXISTS ${X265_LIBRARY})
      get_filename_component(X265_RDIR ${X265_LIBRARY} REALPATH)
      get_filename_component(X265_RDIRFNAME ${X265_RDIR} NAME)
      link_directories(${X265_RDIRFNAME})
      add_library(x265 STATIC IMPORTED)
      set_target_properties(x265 PROPERTIES IMPORTED_LOCATION ${X265_LIBRARY})
      mark_as_advanced(x265)
      LIST (APPEND FFMPEG_LIBRARIES x265;numa)
    endif()
    include(CheckFunctionExists)
    CHECK_FUNCTION_EXISTS(pow EXTRA_LIBM_NOT_NEEDED)
    if(NOT EXTRA_LIBM_NOT_NEEDED)
      LIST (APPEND FFMPEG_LIBRARIES m)
    endif()
  endif()
endif()

IF (DEFINED _FFMPEG_MISSING_COMPONENTS AND _FFMPEG_CHECK_COMPONENTS)
  IF (NOT FFMPEG_FIND_QUIETLY)
    MESSAGE (STATUS "One or more Ffmpeg components were not found:")
    # Display missing components indented, each on a separate line
    FOREACH (_FFMPEG_MISSING_COMPONENT ${_FFMPEG_MISSING_COMPONENTS})
      MESSAGE (STATUS "  " ${_FFMPEG_MISSING_COMPONENT})
    ENDFOREACH (_FFMPEG_MISSING_COMPONENT ${_FFMPEG_MISSING_COMPONENTS})
  ENDIF (NOT FFMPEG_FIND_QUIETLY)
ENDIF (DEFINED _FFMPEG_MISSING_COMPONENTS AND _FFMPEG_CHECK_COMPONENTS)


# CHECK VERSION FROM FFMPEG APP ################################################

FIND_PROGRAM (FFMPEG_EXECUTABLE NAMES ffmpeg
  HINTS ${FFMPEG_ROOT_DIR}
  PATH_SUFFIXES bin
  DOC "ffmpeg executable")

IF (FFMPEG_EXECUTABLE)
  EXECUTE_PROCESS (COMMAND ${FFMPEG_EXECUTABLE} -version
    OUTPUT_VARIABLE _FFMPEG_APP_OUTPUT ERROR_QUIET)

  STRING (REGEX REPLACE
    ".*ffmpeg([ \t]+version)?[ \t]+v?([0-9]+(\\.[0-9]+(\\.[0-9]+)?)?).*" "\\2"
    FFMPEG_VERSION "${_FFMPEG_APP_OUTPUT}")
  STRING (REGEX REPLACE "([0-9]+)\\.([0-9]+)(\\.([0-9]+))?" "\\1"
    FFMPEG_VERSION_MAJOR "${FFMPEG_VERSION}")
  STRING (REGEX REPLACE "([0-9]+)\\.([0-9]+)(\\.([0-9]+))?" "\\2"
    FFMPEG_VERSION_MINOR "${FFMPEG_VERSION}")

  IF ("${FFMPEG_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
    STRING (REGEX REPLACE "([0-9]+)\\.([0-9]+)(\\.([0-9]+))?" "\\3"
      FFMPEG_VERSION_PATCH "${FFMPEG_VERSION}")
    SET (FFMPEG_VERSION_COMPONENTS 3)
  ELSEIF ("${FFMPEG_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
    SET (FFMPEG_VERSION_COMPONENTS 2)
  ELSEIF ("${FFMPEG_VERSION}" MATCHES "^([0-9]+)$")
    # mostly developer/alpha/beta versions
    SET (FFMPEG_VERSION_COMPONENTS 2)
    SET (FFMPEG_VERSION_MINOR 0)
    SET (FFMPEG_VERSION "${FFMPEG_VERSION}.0")
  ENDIF ("${FFMPEG_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
ENDIF (FFMPEG_EXECUTABLE)

# WIN32: LIB PREREQUISITES #####################################################
IF (WIN32)
  FIND_PROGRAM (LIB_EXECUTABLE NAMES lib
    HINTS "$ENV{VS120COMNTOOLS}/../../VC/bin"
          "$ENV{VS110COMNTOOLS}/../../VC/bin"
          "$ENV{VS100COMNTOOLS}/../../VC/bin"
          "$ENV{VS90COMNTOOLS}/../../VC/bin"
          "$ENV{VS71COMNTOOLS}/../../VC/bin"
          "$ENV{VS80COMNTOOLS}/../../VC/bin"
    DOC "Library manager")

  MARK_AS_ADVANCED (LIB_EXECUTABLE)
ENDIF (WIN32)

MACRO (GET_LIB_REQUISITES LIB REQUISITES)
  IF (LIB_EXECUTABLE)
    GET_FILENAME_COMPONENT (_LIB_PATH ${LIB_EXECUTABLE} PATH)

    IF (MSVC)
      # Do not redirect the output
      UNSET (ENV{VS_UNICODE_OUTPUT})
    ENDIF (MSVC)

    EXECUTE_PROCESS (COMMAND ${LIB_EXECUTABLE} /nologo /list ${LIB}
      WORKING_DIRECTORY ${_LIB_PATH}/../../Common7/IDE
      OUTPUT_VARIABLE _LIB_OUTPUT ERROR_QUIET)

    STRING (REPLACE "\n" ";" "${REQUISITES}" "${_LIB_OUTPUT}")
    LIST (REMOVE_DUPLICATES ${REQUISITES})
  ENDIF (LIB_EXECUTABLE)
ENDMACRO (GET_LIB_REQUISITES)

IF (_FFMPEG_ALL_LIBS)
  # collect lib requisites using the lib tool
  FOREACH (_FFMPEG_COMPONENT ${_FFMPEG_ALL_LIBS})
    GET_LIB_REQUISITES (${_FFMPEG_COMPONENT} _FFMPEG_REQUISITES)
  ENDFOREACH (_FFMPEG_COMPONENT)
ENDIF (_FFMPEG_ALL_LIBS)

IF (NOT FFMPEG_BINARY_DIR)
  SET (_FFMPEG_UPDATE_BINARY_DIR TRUE)
ELSE (NOT FFMPEG_BINARY_DIR)
  SET (_FFMPEG_UPDATE_BINARY_DIR FALSE)
ENDIF (NOT FFMPEG_BINARY_DIR)

SET (_FFMPEG_BINARY_DIR_HINTS bin)

IF (_FFMPEG_REQUISITES)
  FIND_FILE (FFMPEG_BINARY_DIR NAMES ${_FFMPEG_REQUISITES}
	  HINTS ${FFMPEG_ROOT_DIR}
    PATH_SUFFIXES ${_FFMPEG_BINARY_DIR_HINTS} NO_DEFAULT_PATH)
ENDIF (_FFMPEG_REQUISITES)

IF (FFMPEG_BINARY_DIR AND _FFMPEG_UPDATE_BINARY_DIR)
  SET (_FFMPEG_BINARY_DIR ${FFMPEG_BINARY_DIR})
  UNSET (FFMPEG_BINARY_DIR CACHE)

  IF (_FFMPEG_BINARY_DIR)
	GET_FILENAME_COMPONENT (FFMPEG_BINARY_DIR ${_FFMPEG_BINARY_DIR} PATH)
  ENDIF (_FFMPEG_BINARY_DIR)
ENDIF (FFMPEG_BINARY_DIR AND _FFMPEG_UPDATE_BINARY_DIR)

SET (FFMPEG_BINARY_DIR ${FFMPEG_BINARY_DIR} CACHE PATH "Ffmpeg binary directory")

MARK_AS_ADVANCED (FFMPEG_INCLUDE_DIR FFMPEG_BINARY_DIR)

IF (NOT _FFMPEG_CHECK_COMPONENTS)
 SET (_FFMPEG_FPHSA_ADDITIONAL_ARGS HANDLE_COMPONENTS)
ENDIF (NOT _FFMPEG_CHECK_COMPONENTS)

#revert CMAKE_FIND_LIBRARY_SUFFIXES
set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_SAV} )


FIND_PACKAGE_HANDLE_STANDARD_ARGS (Ffmpeg REQUIRED_VARS FFMPEG_ROOT_DIR
  FFMPEG_INCLUDE_DIR ${_FFMPEG_MISSING_LIBRARIES} VERSION_VAR FFMPEG_VERSION
  ${_FFMPEG_FPHSA_ADDITIONAL_ARGS})
