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


# if(DEFINED FFMPEG_EXTRA_LINK_FLAGS)
#   set(SQY_LIB ${SQY_LIB} ${FFMPEG_EXTRA_LINK_FLAGS})
# endif()
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

# message(STATUS "[bench] linking against ${SQY_LIB} ${Boost_LIBRARIES} ")

add_executable(test_benchmark_fixture_impl test_benchmark_fixture_impl.cpp)
target_link_libraries(test_benchmark_fixture_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY}
)

add_executable(benchmark_bitswap_scheme_impl benchmark_bitswap_scheme_impl.cpp)
target_link_libraries(benchmark_bitswap_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY})

add_executable(benchmark_frame_shuffle_scheme_impl benchmark_frame_shuffle_scheme_impl.cpp)
target_link_libraries(benchmark_frame_shuffle_scheme_impl ${Boost_LIBRARIES} ${googlebenchmark_LIBRARY} ${googlebenchmark_AUX_LIBRARY})

# TODO list
#frame_shuffle_scheme_impl.hpp
#diff_scheme_impl.hpp
#flatten_to_neighborhood_scheme_impl.hpp
#h264_scheme_impl.hpp
#hevc_scheme_impl.hpp
#quantiser_scheme_impl.hpp
#raster_reorder_scheme_impl.hpp
#remove_background_scheme_impl.hpp
#remove_estimated_background_scheme_impl.hpp
#tile_shuffle_scheme_impl.hpp
#zcurve_reorder_scheme_impl.hpp





# if(EXISTS ${LZ4_STATIC_LIB})
#   #if static libsqeazy exists, it will contain lz4, no need to link against it yet another time
#   if(NOT DEFINED READY_4_BUNDLE)
#     target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${Boost_LIBRARIES})
#     target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES})
#     target_link_libraries(test_pipeline_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES})
#   else()
#     target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
#     target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
#     target_link_libraries(test_pipeline_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
#   endif()
# else()
#   target_link_libraries(test_lz4_encoding ${SQY_LIB} ${Boost_LIBRARIES} ${LZ4_LIBRARY})
#   target_link_libraries(test_hdf5_interface ${SQY_LIB} ${Boost_LIBRARIES} ${LZ4_LIBRARY} ${HDF5_LIBRARIES})
#   target_link_libraries(test_pipeline_interface ${SQY_LIB} ${Boost_LIBRARIES} ${LZ4_LIBRARY})
# endif()


# add_executable(test_diff_schemes test_diff_schemes.cpp)
# target_link_libraries(test_diff_schemes ${SQY_LIB} ${Boost_LIBRARIES})


# IF(TIFF_FOUND)

# # add_executable(test_tiff_fixture test_tiff_fixture.cpp)
# # target_link_libraries(test_tiff_fixture ${Boost_LIBRARIES} ${TIFF_LIBRARY})
# # set_target_properties(test_tiff_fixture PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/bench")


# IF(FFMPEG_FOUND)
#   if(WIN32)
#   LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}\\lib)
#   else()
#   LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)
#     endif()

#   SET(SQY_FFMPEG_Libraries ${FFMPEG_LIBRARIES})

#   if(DEFINED FFMPEG_EXTRA_LINK_FLAGS AND DEFINED FFMPEG_USE_STATIC_LIBS)
#   set(SQY_FFMPEG_Libraries ${SQY_FFMPEG_Libraries} ${FFMPEG_EXTRA_LINK_FLAGS})
#   endif()

#   add_executable(test_avcodec_sandbox test_avcodec_sandbox.cpp)
#   target_link_libraries(test_avcodec_sandbox ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})

#   add_executable(deprecated_pipeline_select_impl deprecated_pipeline_select_impl.cpp)
#   target_link_libraries(deprecated_pipeline_select_impl ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${TIFF_LIBRARY} ${LZ4_LIBRARY} ${SQY_FFMPEG_Libraries})

# ELSE(FFMPEG_FOUND)
#   MESSAGE(">> [tests] libavcodec not found. skipping test_avcodec_sandbox ...")
# ENDIF(FFMPEG_FOUND)

# ELSE(FFMPEG_FOUND)
#   MESSAGE(">> [tests] libtiff not found. skipping related tests ...")
# ENDIF(FFMPEG_FOUND)

# ENDIF(TIFF_FOUND)
