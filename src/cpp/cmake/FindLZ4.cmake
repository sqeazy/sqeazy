# - Find LZ4 (lz4.h, liblz4.a, liblz4.so, and liblz4.so.1)
# This module defines
#  LZ4_INCLUDE_DIRS, directory containing headers
#  LZ4_LIBRARY_DIRS, directory containing lz4 libraries
#  LZ4_STATIC_LIB, path to liblz4.a
#  LZ4_FOUND, whether lz4 has been found
set(_LZ4_ROOT $ENV{LZ4_ROOT})

if(IS_DIRECTORY ${LZ4_ROOT})

  
  IF(WIN32) #assumes MSVC
	find_library(LZ4_LIB_PATH NAMES lz4 liblz4 lz4.dll liblz4.lib lz4.lib liblz4.dll NAMES_PER_DIR HINTS ${LZ4_ROOT} PATHS ${LZ4_ROOT} PATH_SUFFIXES lib bin NO_DEFAULT_PATH)
	IF(NOT LZ4_LIB_PATH)
		FILE(GLOB REC_PATHS ${LZ4_ROOT}/bin/liblz4* ${LZ4_ROOT}/liblz4* ${LZ4_ROOT}/lib/liblz4* ${LZ4_ROOT}/lib/lz4*)
		FOREACH(FPATH ${REC_PATHS})
			IF(${FPATH} MATCHES ".*lz4.dll")
				SET(LZ4_LIB_PATH ${FPATH})
			ENDIF()
			IF(${FPATH} MATCHES ".*lz4.lib")
				SET(LZ4_LIB_PATH ${FPATH})
			ENDIF()
			IF(${FPATH} MATCHES ".*lz4.a")
				SET(LZ4_LIB_PATH ${FPATH})
			ENDIF()
		ENDFOREACH(FPATH)
		
	ENDIF()
	find_path(LZ4_INC_PATH lz4.h HINTS ${LZ4_ROOT} PATHS ${LZ4_ROOT} PATH_SUFFIXES inc include NO_DEFAULT_PATH)
	IF(NOT LZ4_INC_PATH)
		FILE(GLOB REC_PATHS ${LZ4_ROOT}/lz4*.h ${LZ4_ROOT}/include/lz4*.h ${LZ4_ROOT}/inc/lz4*.h)
		FOREACH(FPATH ${REC_PATHS})
			IF(${FPATH} MATCHES ".*lz4.h")
				SET(LZ4_INC_PATH ${FPATH})
			ENDIF()
			
		ENDFOREACH(FPATH)
	ENDIF()
  ELSE()
	find_library(LZ4_LIB_PATH NAMES lz4 liblz4 liblz4.dylib liblz4.a lz4.a NAMES_PER_DIR HINTS ${LZ4_ROOT} PATHS ${LZ4_ROOT} PATH_SUFFIXES lib bin NO_DEFAULT_PATH)
	#find_library(LZ4_LIB_PATH NAMES lz4 liblz4 HINTS ${LZ4_ROOT} ${LZ4_ROOT}/lib NO_DEFAULT_PATH)
	#find_path(LZ4_INC_PATH lz4.h HINTS ${LZ4_ROOT} "${LZ4_ROOT}/lib ${LZ4_ROOT}/include ${LZ4_ROOT}/inc" NO_DEFAULT_PATH)
	find_path(LZ4_INC_PATH lz4.h HINTS ${LZ4_ROOT} PATHS ${LZ4_ROOT} PATH_SUFFIXES inc include NO_DEFAULT_PATH)
  ENDIF()
  
  if (NOT LZ4_FIND_QUIETLY)
    message(STATUS "[FindLZ4] Found: ${LZ4_LIB_PATH} ${LZ4_INC_PATH}")
  endif ()
  
  
else()    

  #MESSAGE(AUTHOR_WARNING "[FindLZ4] LZ4_ROOT not given $ENV{LZ4_ROOT} ${LZ4_ROOT} ${LZ4_DIR}")
  find_path(LZ4_INC_PATH lz4.h)
  find_library(LZ4_LIB_PATH NAMES lz4)
	

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
if (NOT LZ4_FIND_QUIETLY)
    message(STATUS "[FindLZ4] searching ${LZ4_LIBRARY_DIRS} for libraries")
  endif ()
  
if (LZ4_FOUND)
	if(UNIX)
		if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.a)
			set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/liblz4.a)
		endif()
  
		if (APPLE)
			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.dylib)	       		       
				set(LZ4_SHARED_LIB ${LZ4_LIBRARY_DIRS}/liblz4.dylib)
			endif()
		else(APPLE)
			
			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.so)	       		       
				set(LZ4_SHARED_LIB ${LZ4_LIBRARY_DIRS}/liblz4.so)
			endif()
		endif(APPLE)
  
	ELSE(UNIX)
		IF(WIN32)
			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4_static.lib)
				set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/liblz4_static.lib)
			endif()
			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.a)
				set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/liblz4.a)
			endif()
			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.lib)
				set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/liblz4.lib)
			endif()
			if (EXISTS ${LZ4_LIBRARY_DIRS}/lz4.lib)
				set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/lz4.lib)
			endif()
			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.dll)
				set(LZ4_SHARED_LIB ${LZ4_LIBRARY_DIRS}/liblz4.dll)
			endif()
			#honoring msys2
			if (EXISTS ${LZ4_LIBRARY_DIRS}/../bin/liblz4.dll)
				set(LZ4_SHARED_LIB ${LZ4_LIBRARY_DIRS}/../bin/liblz4.dll)
			endif()
		ENDIF(WIN32)
	ENDIF(UNIX)
  if (NOT LZ4_FIND_QUIETLY)
    message(STATUS "Found LZ4 library: ${LZ4_LIBRARY_DIRS} ${LZ4_INCLUDE_DIRS} \n(static lib: ${LZ4_STATIC_LIB}), dyn lib: ${LZ4_SHARED_LIB}")
  endif ()
else ()
  if (NOT LZ4_FIND_QUIETLY)
      message(WARNING "LZ4_LIB_PATH ${LZ4_LIB_PATH}")
      message(WARNING "LZ4_INC_PATH ${LZ4_INC_PATH}")
      message(FATAL_ERROR "LZ4 not found")
  endif ()
endif ()

mark_as_advanced(LZ4_LIBRARY_DIRS LZ4_INCLUDE_DIRS LZ4_STATIC_LIB LZ4_SHARED_LIB)