# Findbenchmark.cmake
# - Try to find benchmark
#
# The following variables are optionally searched for defaults
#  GOOGLEBENCHMARK_ROOT:  Base directory where all benchmark components are found
#
# Once done this will define
#  googlebenchmark_FOUND - System has benchmark
#  googlebenchmark_INCLUDE_DIR - The benchmark include directories
#  googlebenchmark_LIBRARY - The libraries needed to use benchmark

set(GOOGLEBENCHMARK_ROOT "" CACHE PATH "Folder containing benchmark")
#message("FindGoogleBenchmark :: ROOT dir given ${GOOGLEBENCHMARK_ROOT}")

find_path(googlebenchmark_INCLUDE_DIR "benchmark/benchmark.h"
  PATHS ${GOOGLEBENCHMARK_ROOT}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH)
find_path(googlebenchmark_INCLUDE_DIR "benchmark/benchmark.h")

find_library(googlebenchmark_LIBRARY NAMES "benchmark"
  PATHS ${GOOGLEBENCHMARK_ROOT}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH)
find_library(googlebenchmark_LIBRARY NAMES "benchmark")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set googlebenchmark_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(googlebenchmark 
	FOUND_VAR googlebenchmark_FOUND
	REQUIRED_VARS googlebenchmark_LIBRARY googlebenchmark_INCLUDE_DIR)

if(googlebenchmark_FOUND)
  set(googlebenchmark_LIBRARIES ${googlebenchmark_LIBRARY})
  set(googlebenchmark_INCLUDE_DIRS ${googlebenchmark_INCLUDE_DIR})
endif()

mark_as_advanced(googlebenchmark_INCLUDE_DIR googlebenchmark_LIBRARY)
