# Distributed under the LGPL3.0 license.

#.rst:
# FindOpen++
# ----------
#
# Finds OpenMP support using the cmake bundled FindOpenMP
#
# for more details, see the help section of FindOpenMP.cmake
# in your cmake distribution; this wrapper adds a single most important
# flag; this find module assumes that all OpenMP specific headers/libraries
# come with the current C++ compiler
#
# Variables
# ^^^^^^^^^
#
# ``OpenMP_LINK_STATIC``
# if set to ON upon invocation, this wrapper tries to set
# CMAKE_<LANG>_LIBRARIES to yield static libraries only
#
# ``OpenMP++_FOUND``
# defined and true/off, if openmp was found
#
# ``OpenMP++_FLAGS``
# contains the compile flags to be used
#
# ``OpenMP++_VERSION``
# contains the version of OpenMP if present
#
# ``OpenMP++_LIBRARIES``
# contains a list of libraries to be handed to target_link_libraries(...)
# as the right-hand side argument
#
# Tested
# ^^^^^^
# this module was already tested with cmake 3.1, 3.3, 3.5, 3.9 on Ubuntu Linux 16.04 and macOS 10.12.5
#
include(CheckCXXCompilerFlag)

get_filename_component(CXX_COMPILER_APP_PATH "${CMAKE_CXX_COMPILER}" PATH)
get_filename_component(CXX_COMPILER_ROOT_PATH "${CXX_COMPILER_APP_PATH}" PATH)

string(REPLACE "bin" "lib" CXX_COMPILER_LIBS_PATH "${CXX_COMPILER_APP_PATH}")

if(NOT EXISTS ${CXX_COMPILER_LIBS_PATH})
  string(REPLACE "lib" "lib64" CXX_COMPILER_LIBS_PATH "${CXX_COMPILER_APP_PATH}")
endif()

if(NOT EXISTS ${CXX_COMPILER_LIBS_PATH})
  string(REPLACE "lib64" "lib32" CXX_COMPILER_LIBS_PATH "${CXX_COMPILER_APP_PATH}")
endif()

string(REPLACE "bin" "include" CXX_COMPILER_INCLUDE_PATH "${CXX_COMPILER_APP_PATH}")

if(APPLE)

  if(NOT OpenMP++_FIND_QUIETLY)
    message(STATUS "[FindOpenMP++] macOS: ${CMAKE_CXX_COMPILER} => ${CXX_COMPILER_APP_PATH}")
  endif()

  if(${CXX_COMPILER_APP_PATH} MATCHES "/usr/local.*/bin")

    if(NOT OpenMP++_FIND_QUIETLY)
      message(STATUS "[FindOpenMP++] adding ${CXX_COMPILER_LIBS_PATH} to link_directories")
    endif()

    if(EXISTS ${CXX_COMPILER_LIBS_PATH})
      link_directories(${CXX_COMPILER_LIBS_PATH})
    endif()

    if(EXISTS ${CXX_COMPILER_INCLUDE_PATH})
      include_directories(${CXX_COMPILER_INCLUDE_PATH})
    endif()

  endif()
endif()

if(DEFINED OpenMP++_FIND_REQUIRED)
  find_package(OpenMP REQUIRED)
else()
  if(DEFINED OpenMP++_FIND_QUIETLY)
    find_package(OpenMP QUIET)
  else()
    find_package(OpenMP)
  endif()
endif()


if(OpenMP_FOUND)


  set(OpenMP++_FOUND TRUE)
  if(DEFINED OpenMP_CXX_FLAGS)
    set(OpenMP++_FLAGS ${OpenMP_CXX_FLAGS})
  endif()

endif()

if(OpenMP_CXX_FOUND)
  set(OpenMP_FOUND TRUE)
  set(OpenMP++_FOUND TRUE)

  if(NOT OpenMP++_FIND_QUIETLY)
    message(STATUS "[FindOpenMP++] found ${OpenMP_CXX_LIBRARIES} and ${OpenMP_CXX_FLAGS}")
  endif()

  if(${OpenMP_LINK_STATIC})

    if(NOT OpenMP++_FIND_QUIETLY)
      message(STATUS "[FindOpenMP++] found ${OpenMP_CXX_LIBRARIES}, trying to replace ${CMAKE_SHARED_LIBRARY_SUFFIX} libraries for ${CMAKE_STATIC_LIBRARY_SUFFIX} libraries")
    endif()

    if( ${OpenMP_CXX_LIBRARIES} MATCHES ${CMAKE_SHARED_LIBRARY_SUFFIX})
      string(REPLACE "${CMAKE_SHARED_LIBRARY_SUFFIX}" "${CMAKE_STATIC_LIBRARY_SUFFIX}" TEMP_OpenMP_CXX_LIBRARIES "${OpenMP_CXX_LIBRARIES}")
      if( EXISTS ${TEMP_OpenMP_CXX_LIBRARIES} )
        if(NOT OpenMP++_FIND_QUIETLY)
          message(STATUS "[FindOpenMP++] using ${TEMP_OpenMP_CXX_LIBRARIES} in favor of ${OpenMP_CXX_LIBRARIES}")
        endif()
        set(OpenMP_CXX_LIBRARIES "${TEMP_OpenMP_CXX_LIBRARIES}")
      endif()
    endif()
  endif()

endif()

if(NOT OpenMP_FOUND)

  check_cxx_compiler_flag(-fopenmp HAS_FOPENMP_COMPILERFLAG)

  if(${HAS_FOPENMP_COMPILERFLAG})
    #I am guessing here, this should be a try_compile
    set(OpenMP++_FLAGS "-fopenmp")
    set(OpenMP++_FOUND TRUE)
  else()
    message(WARNING "[FindOpenMP++] compiler doesn't honor -fopenmp")
  endif()

endif()

if(NOT OpenMP++_FOUND)
  if(OpenMP++_FIND_REQUIRED )
    message(FATAL_ERROR "[FindOpenMP++] OpenMP not found")
  else()
    message(WARNING "[FindOpenMP++] OpenMP not found")
  endif()
endif()

if(OpenMP++_FOUND AND NOT OpenMP_CXX_LIBRARIES)


  ##TODO: check if the compiler supports this flag

  if(OpenMP_LINK_STATIC)

    find_library(OMP_LIB_PATH
      NAMES libgomp${CMAKE_STATIC_LIBRARY_SUFFIX} libomp${CMAKE_STATIC_LIBRARY_SUFFIX} gomp${CMAKE_STATIC_LIBRARY_SUFFIX} omp${CMAKE_STATIC_LIBRARY_SUFFIX}
      PATHS ${CXX_COMPILER_LIBS_PATH} ${CXX_COMPILER_ROOT_PATH}
      HINTS ${CXX_COMPILER_LIBS_PATH} ${CXX_COMPILER_ROOT_PATH}
      PATH_SUFFIXES lib lib32 lib64
      )

  else()
    find_library(OMP_LIB_PATH
      NAMES libgomp${CMAKE_SHARED_LIBRARY_SUFFIX} libomp${CMAKE_SHARED_LIBRARY_SUFFIX} gomp${CMAKE_SHARED_LIBRARY_SUFFIX} omp${CMAKE_SHARED_LIBRARY_SUFFIX}
      PATHS ${CXX_COMPILER_LIBS_PATH} ${CXX_COMPILER_ROOT_PATH}
      HINTS ${CXX_COMPILER_LIBS_PATH} ${CXX_COMPILER_ROOT_PATH}
      PATH_SUFFIXES lib lib32 lib64
      )
  endif()

  if(DEFINED OpenMP_LINK_STATIC AND ${OpenMP_LINK_STATIC} MATCHES ON)


    if(EXISTS ${OMP_LIB_PATH})

      list(APPEND OpenMP_CXX_LIBRARIES ${OMP_LIB_PATH})

    else()
      message(WARNING "[FindOpenMP++] libgomp${CMAKE_STATIC_LIBRARY_SUFFIX}/libomp${CMAKE_STATIC_LIBRARY_SUFFIX} not found, using OpenMP_CXX_LIBRARIES=${OpenMP_CXX_LIBRARIES}")
    endif()

    if(UNIX)
      list(APPEND OpenMP_CXX_LIBRARIES dl pthread)
    endif()

  else(DEFINED OpenMP_LINK_STATIC AND ${OpenMP_LINK_STATIC} MATCHES ON)

    link_directories(${CXX_COMPILER_LIBS_PATH})

    if(${LIBGOMP_SHARED_RVALUE} MATCHES "0" AND EXISTS ${LIBGOMP_SHARED_LOCATION})
      list(APPEND OpenMP_CXX_LIBRARIES gomp)
    else()
      list(APPEND OpenMP_CXX_LIBRARIES omp)
    endif()

    if(UNIX)
      list(APPEND OpenMP_CXX_LIBRARIES pthread)
    endif()

  endif(DEFINED OpenMP_LINK_STATIC AND ${OpenMP_LINK_STATIC} MATCHES ON)



endif()

if(OpenMP++_FOUND)
  # message(STATUS "[FindOpenMP++] ${OpenMP++_FLAGS} <=> ${OpenMP_CXX_FLAGS}")
  # message(STATUS "[FindOpenMP++] ${OpenMP++_LIBRARIES} <=> ${OpenMP_CXX_LIBRARIES}")
  # message(STATUS "[FindOpenMP++] ${OpenMP++_VERSION} <=> ${OpenMP_CXX_VERSION}")

  if(OpenMP_CXX_FLAGS AND NOT OpenMP++_FLAGS)
    set(OpenMP++_FLAGS ${OpenMP_CXX_FLAGS})
  endif()

  if(OpenMP_CXX_LIBRARIES AND NOT OpenMP++_LIBRARIES)
    set(OpenMP++_LIBRARIES ${OpenMP_CXX_LIBRARIES})
  endif()

  if(OpenMP_CXX_VERSION AND NOT OpenMP++_VERSION)
    set(OpenMP++_VERSION ${OpenMP_CXX_VERSION})
  endif()

  add_definitions(${OpenMP++_FLAGS})

  if(NOT OpenMP++_FIND_QUIETLY)
    message(STATUS "[FindOpenMP++] exporting: OpenMP++_FLAGS=${OpenMP++_FLAGS} OpenMP++_LIBRARIES=${OpenMP++_LIBRARIES} OpenMP++_VERSION=${OpenMP++_VERSION}")
  endif()

endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenMP++
  REQUIRED_VARS OpenMP++_FLAGS OpenMP++_LIBRARIES
  VERSION_VAR OpenMP++_VERSION)
