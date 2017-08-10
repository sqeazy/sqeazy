# adapted from https://bitbucket.org/sergiu/libav-cmake/src/51fb1cd97ce401e0fdebe8452d81accfcb8bc8e3/FindLibAV.cmake?at=default
# Module for locating Ffmpeg.
#
# Customizable variables:
#   FFMPEG_ROOT_DIR
#     Specifies Ffmpeg's root directory.
#
#   FFMPEG_IGNORE_PKG_CONFIG
#     Do not use pkg_config even if availabe
#
#   FFMPEG_USE_STATIC_LIBS
#      favor static libs over shared libraries

# Read-only variables:
#   FFMPEG_FOUND
#     Indicates whether the library has been found.
#
#   FFMPEG_INCLUDE_DIR
#      Specifies Ffmpeg's include directory.
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
#   FFMPEG_EXTRA_LINK_FLAGS
#     Contains extra linker flags that may be beneficial for external use
#

# CMAKE INCLUDES #############################################################
INCLUDE (FindPackageHandleStandardArgs)
INCLUDE (FindPkgConfig)
INCLUDE (CheckFunctionExists)

find_package(PkgConfig QUIET)

#doc: REGEX_REMOVE_ITEM("${TESTLIST}" ".wo" result_list)
#     remove any item that matches the regex .wo from TESTLIST
#     save the result in result_list
function(REGEX_REMOVE_ITEM VALUES REGEX_STR OUTPUT)

  foreach(_ITEM IN LISTS VALUES)
    if("${_ITEM}" STRLESS " ")
      continue()
    endif()

    string(REPLACE "++" ".*" REGEX_STR "${REGEX_STR}")
    string(REGEX MATCH "${REGEX_STR}" _MATCHRESULT ${_ITEM})

    if(${_MATCHRESULT} MATCHES ${REGEX_STR})
      list(REMOVE_ITEM VALUES ${_ITEM})
    endif()
    unset(_MATCHRESULT)
  endforeach()

  set(${OUTPUT} ${VALUES} PARENT_SCOPE)
endfunction()

# SETUP VARIABLES #############################################################
# set( CMAKE_FIND_LIBRARY_SUFFIXES_SAV ${CMAKE_FIND_LIBRARY_SUFFIXES} )


IF (CMAKE_VERSION VERSION_GREATER 2.8.7)
  SET (_FFMPEG_CHECK_COMPONENTS FALSE)
ELSE (CMAKE_VERSION VERSION_GREATER 2.8.7)
  SET (_FFMPEG_CHECK_COMPONENTS TRUE)
ENDIF (CMAKE_VERSION VERSION_GREATER 2.8.7)

SET(EXT_FFMPEG_ROOT_DIR EXT_FFMPEG_ROOT_DIR-NOTFOUND)
IF (IS_DIRECTORY $ENV{FFMPEG_ROOT})
  SET(EXT_FFMPEG_ROOT_DIR $ENV{FFMPEG_ROOT})
ENDIF ()

IF (IS_DIRECTORY "${FFMPEG_ROOT}")
  SET(EXT_FFMPEG_ROOT_DIR ${FFMPEG_ROOT})
ENDIF ()
#end of add


# FIND INCLUDE DIR #############################################################

FIND_PATH (FFMPEG_ROOT_DIR
  NAMES include/libavcodec/avcodec.h
  libavformat/avformat.h
  libavutil/avutil.h
  PATHS ${EXT_FFMPEG_ROOT_DIR}
  HINTS ${EXT_FFMPEG_ROOT_DIR}
  DOC "FFMPEG root directory")

if(NOT FFMPEG_FIND_QUIETLY)
  message("** [FindFFMPEG] found root path ${FFMPEG_ROOT_DIR}")
endif()


FIND_PATH (FFMPEG_INCLUDE_DIR
  NAMES libavcodec/avcodec.h
  libavformat/avformat.h
  libavutil/avutil.h
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

# PKG_CONFIG (Unix only) ######################################################
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


  pkg_check_modules(LOCAL_FFMPEG ${MY_PKGCONFIG_FINDARGS} ${comp_with_fixed_names})
  set(FFMPEG_EXTRA_LINK_FLAGS ${LOCAL_FFMPEG_LDFLAGS} ${LOCAL_FFMPEG_LDFLAGS_OTHER})
  message("!! ${FFMPEG_EXTRA_LINK_FLAGS}")

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

        if(${FFMPEG_USE_STATIC_LIBS})
          find_library(${_COMP}_PATH
            NAMES ${_COMP}${CMAKE_STATIC_LIBRARY_SUFFIX} lib${_COMP}${CMAKE_STATIC_LIBRARY_SUFFIX} NAMES_PER_DIR
            PATHS ${FFMPEG_ROOT_DIR}
            HINTS ${FFMPEG_ROOT_DIR}
            PATH_SUFFIXES lib lib64)
        else()
          find_library(${_COMP}_PATH
            NAMES ${_COMP} lib${_COMP} NAMES_PER_DIR
            PATHS ${FFMPEG_ROOT_DIR}
            HINTS ${FFMPEG_ROOT_DIR}
            PATH_SUFFIXES lib lib64 bin)
        endif()

        set(TEMP_FLAGS "")
        if(${${_COMP}_PATH} MATCHES ".*${CMAKE_SHARED_LIBRARY_SUFFIX}")
          #message(STATUS "[FindFFMPEG] shared ${_COMP} found inside ${LOCAL_FFMPEG_LIBRARIES} (${${_COMP}_PATH})")
          add_library(${_COMP} SHARED IMPORTED)
        else()
          #message(STATUS "[FindFFMPEG] static ${_COMP} found inside ${LOCAL_FFMPEG_LIBRARIES} (${${_COMP}_PATH})")
          add_library(${_COMP} STATIC IMPORTED)
        endif()


        set_target_properties(${_COMP} PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR}
          LINKER_LANGUAGE C
          IMPORTED_LINK_INTERFACE_LANGUAGES "C"
          IMPORTED_LOCATION ${${_COMP}_PATH}
          IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
          IMPORTED_LOCATION_RELEASE ${${_COMP}_PATH}
          )

        set_property(TARGET ${_COMP} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)

        mark_as_advanced (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_LIBRARY)
        mark_as_advanced (${_COMP})


      else()
        SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND FALSE)
        IF (_FFMPEG_CHECK_COMPONENTS)
          LIST (APPEND _FFMPEG_MISSING_LIBRARIES ${_FFMPEG_LIBRARY_BASE})
        ENDIF (_FFMPEG_CHECK_COMPONENTS)
      endif()

    endforeach()

    LIST (APPEND FFMPEG_LIBRARIES ${LOCAL_FFMPEG_LIBRARIES})
    LIST (APPEND _FFMPEG_ALL_LIBS ${LOCAL_FFMPEG_LIBRARIES})
  else(LOCAL_FFMPEG_FOUND)
    foreach(_COMP ${FFMPEG_FIND_COMPONENTS})
      STRING (TOUPPER ${_FFMPEG_COMPONENT} _FFMPEG_COMPONENT_UPPER)
      SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND FALSE)
      IF (_FFMPEG_CHECK_COMPONENTS)
        LIST (APPEND _FFMPEG_MISSING_LIBRARIES ${_FFMPEG_LIBRARY_BASE})
      endif(_FFMPEG_CHECK_COMPONENTS)
    endforeach()
  endif(LOCAL_FFMPEG_FOUND)

else(PKG_CONFIG_FOUND AND NOT FFMPEG_IGNORE_PKG_CONFIG)

  FOREACH (_FFMPEG_COMPONENT ${FFMPEG_FIND_COMPONENTS})
    STRING (TOUPPER ${_FFMPEG_COMPONENT} _FFMPEG_COMPONENT_UPPER)
    SET (_FFMPEG_LIBRARY_BASE FFMPEG_${_FFMPEG_COMPONENT_UPPER}_LIBRARY)

    #tries to obtain FFMPEG_COMPONENT_LIBRARY

    FIND_LIBRARY (${_FFMPEG_LIBRARY_BASE}
      NAMES ${_FFMPEG_COMPONENT} lib${_FFMPEG_COMPONENT} ${_FFMPEG_COMPONENT}${CMAKE_STATIC_LIBRARY_SUFFIX} lib${_FFMPEG_COMPONENT}${CMAKE_STATIC_LIBRARY_SUFFIX} NAMES_PER_DIR
      HINTS ${FFMPEG_ROOT_DIR}
      PATHS ${FFMPEG_ROOT_DIR}
      PATH_SUFFIXES bin lib lib64
      DOC "Ffmpeg ${_FFMPEG_COMPONENT} library")

    if(NOT FFMPEG_FIND_QUIETLY)
      message("** [FindFFMPEG] found ${_FFMPEG_COMPONENT} ${_FFMPEG_LIBRARY_BASE} ${${_FFMPEG_LIBRARY_BASE}}, setting FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND")

    endif()

    #add what was found to global variables
    IF (${_FFMPEG_LIBRARY_BASE})

      SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND TRUE)

      if(${${_FFMPEG_LIBRARY_BASE}} MATCHES ".*${CMAKE_STATIC_LIBRARY_SUFFIX}")
        add_library(${_FFMPEG_COMPONENT} STATIC IMPORTED)
        message("** [FindFFMPEG] exporting target ${_FFMPEG_COMPONENT} as static library")
        message("** [FindFFMPEG] static library requested and unable to use pkg-config, FFMPEG_EXTRA_LINK_FLAGS will be filled with a wild guess")
        if(APPLE)
          # if(PKG_CONFIG_FOUND)
          #   pkg_check_modules(PKG_FFMPEG QUIET ${_FFMPEG_COMPONENT})
          #   set(FFMPEG_EXTRA_LINK_FLAGS ${FFMPEG_EXTRA_LINK_FLAGS} ${PKG_FFMPEG_STATIC_LDFLAGS} ${PKG_FFMPEG_STATIC_LDFLAGS_OTHER})

          # else(PKG_CONFIG_FOUND)
          set(FFMPEG_EXTRA_LINK_FLAGS "-L/usr/local/lib -framework CoreFoundation -framework VideoToolbox -framework CoreMedia -framework QuartzCore -framework CoreVideo -framework CoreFoundation -framework VideoDecodeAcceleration -framework QuartzCore -liconv -Wl,-framework,CoreFoundation -Wl,-framework,Security -lx265 -lx264 -lm -lbz2 -lz -pthread -framework CoreServices -framework CoreFoundation -framework VideoToolbox -framework CoreMedia -framework QuartzCore -framework CoreVideo -framework CoreFoundation -framework VideoDecodeAcceleration -framework QuartzCore -liconv -Wl,-framework,CoreFoundation -Wl,-framework,Security -lx265 -lx264 -lm -lbz2 -lz -pthread -framework CoreServices -lm")
          # endif(PKG_CONFIG_FOUND)
        else()
          if(NOT WIN32)
            # if(PKG_CONFIG_FOUND)
            #   pkg_check_modules(PKG_FFMPEG QUIET ${_FFMPEG_COMPONENT})
            #   set(FFMPEG_EXTRA_LINK_FLAGS ${FFMPEG_EXTRA_LINK_FLAGS} ${PKG_FFMPEG_STATIC_LDFLAGS} ${PKG_FFMPEG_STATIC_LDFLAGS_OTHER})
            # else()
              set(FFMPEG_EXTRA_LINK_FLAGS "-lXv -lX11 -lXext -lva -lva-x11 -lva -lxcb -lxcb-shm -lxcb -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lxcb-shape -lxcb -lX11 -lasound -lSDL -lpthread -lx265 -lx264 -lm -llzma -lbz2 -lz -pthread -lXv -lX11 -lXext -lva -lva-x11 -lva -lxcb -lxcb-shm -lxcb -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lxcb-shape -lxcb -lX11 -lasound -lSDL -lpthread -lx265 -lx264 -lm -llzma -lbz2 -lz -pthread -lm")
            # endif()
          else()
            set(FFMPEG_EXTRA_LINK_FLAGS "ws2_32.lib Secur32.lib x265.lib libx264.lib psapi.lib advapi32.lib shell32.lib")
            string(REPLACE " " ";" FFMPEG_EXTRA_LINK_FLAGS_LIST "${FFMPEG_EXTRA_LINK_FLAGS}")
            foreach(_XLIB IN LISTS FFMPEG_EXTRA_LINK_FLAGS_LIST)
              find_library(${_XLIB}_PATH ${_XLIB})
              if(${_XLIB}_PATH AND EXISTS ${${_XLIB}_PATH})
                get_filename_component(${_XLIB}_DIR "${${_XLIB}_PATH}" DIRECTORY)
                link_directories(${${_XLIB}_DIR})
              endif()
            endforeach()
          endif()
        endif()
      else()
        add_library(${_FFMPEG_COMPONENT} SHARED IMPORTED)
        message("** [FindFFMPEG] exporting target ${_FFMPEG_COMPONENT} as shared library")
      endif()

      set_target_properties(${_FFMPEG_COMPONENT} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR}
        LINKER_LANGUAGE C
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION ${_FFMPEG_LIBRARY_BASE}
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
        IMPORTED_LOCATION_RELEASE ${_FFMPEG_LIBRARY_BASE}
        )
      set_property(TARGET ${_FFMPEG_COMPONENT} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
      mark_as_advanced(${_FFMPEG_COMPONENT})
      MARK_AS_ADVANCED (${_FFMPEG_LIBRARY_BASE})

      SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_LIBRARIES ${${_FFMPEG_LIBRARY_BASE}})
      LIST (APPEND FFMPEG_LIBRARIES ${FFMPEG_${_FFMPEG_COMPONENT_UPPER}_LIBRARIES})
      LIST (APPEND _FFMPEG_ALL_LIBS ${${_FFMPEG_LIBRARY_BASE}})
    ELSE (${_FFMPEG_LIBRARY_BASE})
      SET (FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND FALSE)
      message("** [FindFFMPEG] target ${_FFMPEG_COMPONENT} not found")
      IF (_FFMPEG_CHECK_COMPONENTS)
        LIST (APPEND _FFMPEG_MISSING_LIBRARIES ${_FFMPEG_LIBRARY_BASE})
      ENDIF (_FFMPEG_CHECK_COMPONENTS)
    ENDIF (${_FFMPEG_LIBRARY_BASE})


    SET (FFMPEG_${_FFMPEG_COMPONENT}_FOUND ${FFMPEG_${_FFMPEG_COMPONENT_UPPER}_FOUND})


  ENDFOREACH (_FFMPEG_COMPONENT ${FFMPEG_FIND_COMPONENTS})

endif(PKG_CONFIG_FOUND AND NOT FFMPEG_IGNORE_PKG_CONFIG)

#revert CMAKE_FIND_LIBRARY_SUFFIXES
# set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_SAV} )

## clean LDFLAGS from components and dependencies, on UNIX only yet
string(REPLACE " " ";" FFMPEG_EXTRA_LINK_FLAG_LIST  "${FFMPEG_EXTRA_LINK_FLAGS}")

foreach(_FOUND_LIB IN LISTS FFMPEG_LIBRARIES)
  get_filename_component(_LIB_STEM ${_FOUND_LIB} NAME_WE)
  if(${_LIB_STEM} MATCHES "lib.*")
    string(REPLACE "lib" "" _LIB_STEM "${_LIB_STEM}")
  endif()
  message("** [FindFFMPEG] removing ..${_LIB_STEM}$ from ${FFMPEG_EXTRA_LINK_FLAG_LIST}")

  #ATTENTION: this line can do quote some harm if '-l' is replaced by '..' in the second argument to REGEX_REMOVE_ITEM
  REGEX_REMOVE_ITEM("${FFMPEG_EXTRA_LINK_FLAG_LIST}" "-l${_LIB_STEM}$" FFMPEG_EXTRA_LINK_FLAG_LIST)
endforeach()

if(NOT APPLE)
  if(FFMPEG_EXTRA_LINK_FLAG_LIST)
    list(REMOVE_DUPLICATES FFMPEG_EXTRA_LINK_FLAG_LIST)
  endif()
endif()
string(REPLACE ";" " " FFMPEG_EXTRA_LINK_FLAGS  "${FFMPEG_EXTRA_LINK_FLAG_LIST}")

message(STATUS "[FindFFMPEG] defining FFMPEG_EXTRA_LINK_FLAGS to ${FFMPEG_EXTRA_LINK_FLAGS}")

mark_as_advanced (FFMPEG_EXTRA_LINK_FLAGS)


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

message("!! EXTRA FLAGS: ${FFMPEG_EXTRA_LINK_FLAGS}")

FIND_PACKAGE_HANDLE_STANDARD_ARGS (Ffmpeg REQUIRED_VARS FFMPEG_ROOT_DIR
  FFMPEG_INCLUDE_DIR ${_FFMPEG_MISSING_LIBRARIES} VERSION_VAR FFMPEG_VERSION
  ${_FFMPEG_FPHSA_ADDITIONAL_ARGS})
