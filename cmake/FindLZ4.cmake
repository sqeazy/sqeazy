# - Find LZ4 (lz4.h, liblz4.a, liblz4.so, and liblz4.so.1)
# This module defines
#  LZ4_INCLUDE_DIRS, directory containing headers
#  LZ4_LIBRARY_DIRS, directory containing lz4 libraries
#  LZ4_STATIC_LIB, path to liblz4.a
#  LZ4_FOUND, whether lz4 has been found

if(NOT LZ4_ROOT)

  find_path(LZ4_INC_PATH lz4.h)
  find_library(LZ4_LIB_PATH NAMES lz4)
else()    

  find_library(LZ4_LIB_PATH NAMES lz4 PATHS ${LZ4_ROOT} NO_DEFAULT_PATH)
  find_path(LZ4_INC_PATH lz4.h PATHS ${LZ4_ROOT} NO_DEFAULT_PATH)

endif()


if (LZ4_INC_PATH AND LZ4_LIB_PATH)
  set(LZ4_FOUND TRUE)
  #set(LZ4_LIBRARY_DIRS ${LZ4_LIB_PATH})
  if(${CMAKE_VERSION} VERSION_GREATER "2.8.11")
    get_filename_component(LZ4_LIBRARY_DIRS ${LZ4_LIB_PATH} DIRECTORY)
  else()
    get_filename_component(LZ4_LIBRARY_DIRS ${LZ4_LIB_PATH} PATH)
  endif()
  set(LZ4_INCLUDE_DIRS ${LZ4_INC_PATH})
else ()
  set(LZ4_FOUND FALSE)
endif ()

if (LZ4_FOUND)
  if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.a)
    set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/liblz4.a)
  endif()
  if (NOT LZ4_FIND_QUIETLY)
    message(STATUS "Found LZ4 library: ${LZ4_LIBRARY_DIRS} ${LZ4_INCLUDE_DIRS} (static lib: ${LZ4_STATIC_LIB})")
  endif ()
else ()
  if (NOT LZ4_FIND_QUIETLY)
      message(WARNING "LZ4_LIB_PATH ${LZ4_LIB_PATH}")
      message(WARNING "LZ4_INC_PATH ${LZ4_INC_PATH}")
      message(FATAL_ERROR "LZ4 not found")
  endif ()
endif ()

mark_as_advanced(LZ4_LIBRARY_DIRS LZ4_INCLUDE_DIRS LZ4_STATIC_LIB)