#include project relevant directories
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)


#ATTENTION:
#boost unit test framework has some defects if we wanna bind against shared and/or static libs
##that's why we bind against the dynamic loaded libraries primarily
SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE (Boost 1.55 COMPONENTS regex unit_test_framework filesystem system REQUIRED QUIET)
if(Boost_FOUND)
    MESSAGE("++ [tests] Boost found at ${Boost_LIBRARY_DIR} ${Boost_LIBRARIES} : ${Boost_LIBRARIES}")
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    IF(MSVC)
    add_definitions(/DBOOST_TEST_DYN_LINK)

    ELSE(MSVC)
    add_definitions(-DBOOST_TEST_DYN_LINK)
    ENDIF(MSVC)
else(Boost_FOUND)
    MESSAGE(FATAL_ERROR "++ Boost not found!")
endif(Boost_FOUND)

set(SQY_LIB ${PROJECT_NAME})

if(DEFINED READY_4_BUNDLE)
  set(SQY_LIB ${SQY_LIB} ${FFMPEG_LIBRARIES} )
endif()

if(EXISTS $ENV{GOOGLEBENCHMARK_ROOT})
  set(GOOGLEBENCHMARK_ROOT "$ENV{GOOGLEBENCHMARK_ROOT}")
endif()

find_package(GoogleBenchmark REQUIRED)
if(NOT ${googlebenchmark_FOUND})

  if(IS_DIRECTORY ${GOOGLEBENCHMARK_ROOT})
    message(FATAL_ERROR "GoogleBenchmark not found in ${GOOGLEBENCHMARK_ROOT}")
  else(IS_DIRECTORY ${GOOGLEBENCHMARK_ROOT})
    message(FATAL_ERROR "GoogleBenchmark not found, maybe you want to provide any custom installation path by -DGOOGLEBENCHMARK_ROOT=/path/to/library")
  endif(IS_DIRECTORY ${GOOGLEBENCHMARK_ROOT})

else()
  include_directories(${googlebenchmark_INCLUDE_DIR})
  message(">> GoogleBenchmark found, ${googlebenchmark_INCLUDE_DIR} ${googlebenchmark_LIBRARY}")
  set(googlebenchmark_LIBRARY_DIR "")
  if(EXISTS ${googlebenchmark_LIBRARY})
    get_filename_component(googlebenchmark_LIBRARY_DIR "${googlebenchmark_LIBRARY}" DIRECTORY)
  endif()

  if(IS_DIRECTORY "${googlebenchmark_LIBRARY_DIR}")
    link_directories(${googlebenchmark_LIBRARY_DIR})
  endif()
endif()

add_executable(test_benchmark_fixture_impl test_benchmark_fixture_impl.cpp)
target_link_libraries(test_benchmark_fixture_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY}
)

add_executable(benchmark_bitswap_scheme_impl benchmark_bitswap_scheme_impl.cpp)
target_link_libraries(benchmark_bitswap_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY})

add_executable(benchmark_frame_shuffle_scheme_impl benchmark_frame_shuffle_scheme_impl.cpp)
target_link_libraries(benchmark_frame_shuffle_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY})

add_executable(benchmark_diff_scheme_impl benchmark_diff_scheme_impl.cpp)
target_link_libraries(benchmark_diff_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY})

add_executable(benchmark_flatten_to_neighborhood_scheme_impl benchmark_flatten_to_neighborhood_scheme_impl.cpp)
target_link_libraries(benchmark_flatten_to_neighborhood_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY})

add_executable(benchmark_remove_background_scheme_impl benchmark_remove_background_scheme_impl.cpp)
target_link_libraries(benchmark_remove_background_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY})

add_executable(benchmark_remove_estimated_background_scheme_impl benchmark_remove_estimated_background_scheme_impl.cpp)
target_link_libraries(benchmark_remove_estimated_background_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY})

add_executable(benchmark_quantiser_scheme_impl benchmark_quantiser_scheme_impl.cpp)
target_link_libraries(benchmark_quantiser_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY})


IF(FFMPEG_FOUND)
  if(WIN32)
    LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}\\lib)
    else()
    LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)
    endif()

  SET(SQY_FFMPEG_Libraries ${FFMPEG_LIBRARIES})

  if(DEFINED FFMPEG_EXTRA_LINK_FLAGS AND DEFINED FFMPEG_USE_STATIC_LIBS)
    set(SQY_FFMPEG_Libraries ${SQY_FFMPEG_Libraries} ${FFMPEG_EXTRA_LINK_FLAGS})
  endif()

  add_executable(benchmark_h264_scheme_impl benchmark_h264_scheme_impl.cpp)
  target_link_libraries(benchmark_h264_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY} ${SQY_FFMPEG_Libraries})

  add_executable(benchmark_hevc_scheme_impl benchmark_hevc_scheme_impl.cpp)
  target_link_libraries(benchmark_hevc_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY} ${SQY_FFMPEG_Libraries})

endif()
