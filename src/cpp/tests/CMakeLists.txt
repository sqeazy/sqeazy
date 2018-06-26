#include project relevant directories
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)



#ATTENTION:
#boost unit test framework has some defects if we wanna bind against shared and/or static libs
##that's why we bind against the dynamic loaded libraries primarily
SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE (Boost 1.55 COMPONENTS regex filesystem system serialization REQUIRED QUIET)
if(Boost_FOUND)
    MESSAGE("++ [tests] Boost found at ${Boost_LIBRARY_DIR} ${Boost_LIBRARIES} ")
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    # IF(MSVC)
    # add_definitions(/DBOOST_TEST_DYN_LINK)

    # ELSE(MSVC)
    # add_definitions(-DBOOST_TEST_DYN_LINK)
    # ENDIF(MSVC)
else(Boost_FOUND)
    MESSAGE(FATAL_ERROR "++ Boost not found!")
endif(Boost_FOUND)


set(SQY_LIB ${PROJECT_NAME})

if((${WITH_FFMPEG} EQUAL ON) AND (DEFINED READY_4_BUNDLE))
  set(SQY_LIB ${SQY_LIB} ${FFMPEG_LIBRARIES} )
endif()


message(STATUS "++ [tests] linking against ${SQY_LIB} ${Boost_LIBRARIES} ")

add_executable(test_compass_impl test_compass_impl.cpp)
target_link_libraries(test_compass_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_algorithms_impl test_algorithms_impl.cpp)
target_link_libraries(test_algorithms_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_string_parsers_impl test_string_parsers_impl.cpp)
target_link_libraries(test_string_parsers_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_base64_impl test_base64_impl.cpp)
target_link_libraries(test_base64_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_string_shapers_impl test_string_shapers_impl.cpp)
target_link_libraries(test_string_shapers_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})


add_executable(test_diff_scheme_impl test_diff_scheme_impl.cpp)
target_link_libraries(test_diff_scheme_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})


add_executable(test_bitswap_scheme_impl test_bitswap_scheme_impl.cpp)
target_link_libraries(test_bitswap_scheme_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_bitplane_reorder_impl test_bitplane_reorder_impl.cpp)
target_link_libraries(test_bitplane_reorder_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_bitplane_reorder_on_ramp_impl test_bitplane_reorder_on_ramp_impl.cpp)
target_link_libraries(test_bitplane_reorder_on_ramp_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(bench_bitplane_reorder_impl bench_bitplane_reorder_impl.cpp)
target_link_libraries(bench_bitplane_reorder_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_sse_utils_impl test_sse_utils_impl.cpp)
target_link_libraries(test_sse_utils_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

# add_executable(test_sse_bitshuffle_impl test_sse_bitshuffle_impl.cpp)
# target_link_libraries(test_sse_bitshuffle_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_shift_by_intrinsics test_shift_by_intrinsics.cpp)
target_link_libraries(test_shift_by_intrinsics ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_rotate_by_intrinsics test_rotate_by_intrinsics.cpp)
target_link_libraries(test_rotate_by_intrinsics ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_sqeazy_utils_impl test_sqeazy_utils_impl.cpp)
target_link_libraries(test_sqeazy_utils_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} )

add_executable(test_sqeazy_header_impl test_sqeazy_header_impl.cpp)
target_link_libraries(test_sqeazy_header_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_header_tag_impl test_header_tag_impl.cpp)
target_link_libraries(test_header_tag_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_background_scheme_impl test_background_scheme_impl.cpp)
target_link_libraries(test_background_scheme_impl  ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_raster_reorder_scheme_impl test_raster_reorder_scheme_impl.cpp)
target_link_libraries(test_raster_reorder_scheme_impl  ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_tile_shuffle_scheme_impl test_tile_shuffle_scheme_impl.cpp)
target_link_libraries(test_tile_shuffle_scheme_impl  ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_frame_shuffle_scheme_impl test_frame_shuffle_scheme_impl.cpp)
target_link_libraries(test_frame_shuffle_scheme_impl  ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_zcurve_reorder_scheme_impl test_zcurve_reorder_scheme_impl.cpp)
target_link_libraries(test_zcurve_reorder_scheme_impl  ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_hist_impl test_hist_impl.cpp)
target_link_libraries(test_hist_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

#REDO tests with pipeline C API:  add_executable(test_lz4_encoding test_lz4_encoding.cpp)
add_executable(test_hdf5_interface test_hdf5_interface.cpp)


add_executable(test_pipeline_interface test_pipeline_interface.cpp)



if(EXISTS ${LZ4_STATIC_LIB})
  #if static libsqeazy exists, it will contain lz4, no need to link against it yet another time
  if(NOT DEFINED READY_4_BUNDLE)
#REDO tests with pipeline C API:    target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(test_pipeline_interface ${PROJECT_NAME}_static ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})
  else()
    #REDO tests with pipeline C API:    target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
    if(WITH_FFMPEG)
    target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
    target_link_libraries(test_pipeline_interface ${PROJECT_NAME}_static ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
  endif()

  endif()
else()
#REDO tests with pipeline C API: target_link_libraries(test_lz4_encoding ${SQY_LIB} ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY})
  target_link_libraries(test_hdf5_interface ${SQY_LIB} ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY} ${HDF5_LIBRARIES})
  target_link_libraries(test_pipeline_interface ${SQY_LIB} ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY})
endif()



add_executable(test_dynamic_pipeline_impl test_dynamic_pipeline_impl.cpp)
target_link_libraries(test_dynamic_pipeline_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY})

add_executable(test_lz4_scheme_impl test_lz4_scheme_impl.cpp)
target_link_libraries(test_lz4_scheme_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY})

add_executable(test_lz4_utils_impl test_lz4_utils_impl.cpp)
target_link_libraries(test_lz4_utils_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY})

add_executable(test_lz4_sandbox test_lz4_sandbox.cpp)
target_link_libraries(test_lz4_sandbox ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY})

add_executable(test_dynamic_stage_impl test_dynamic_stage_impl.cpp)
target_link_libraries(test_dynamic_stage_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_dynamic_stage_factory_impl test_dynamic_stage_factory_impl.cpp)
target_link_libraries(test_dynamic_stage_factory_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_quantiser_impl test_quantiser_impl.cpp)
target_link_libraries(test_quantiser_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(run_stack_overflow run_stack_overflow.cpp)
target_link_libraries(run_stack_overflow ${OpenMP++_LIBRARIES})

add_executable(test_histogram_fill test_histogram_fill.cpp)
target_link_libraries(test_histogram_fill ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test_yuv_utils test_yuv_utils.cpp)
target_link_libraries(test_yuv_utils ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})


#REDO tests with pipeline C API: add_executable(test_background_schemes test_background_schemes.cpp)
#REDO tests with pipeline C API: target_link_libraries(test_background_schemes ${SQY_LIB} ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

#REDO tests with pipeline C API: add_executable(test_diff_schemes test_diff_schemes.cpp)
#REDO tests with pipeline C API: target_link_libraries(test_diff_schemes ${SQY_LIB} ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})

#REDO tests with pipeline C API: add_executable(test_bitswap_schemes test_bitswap_schemes.cpp)
#REDO tests with pipeline C API: target_link_libraries(test_bitswap_schemes ${SQY_LIB} ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES})


IF(TIFF_FOUND)

add_executable(test_tiff_fixture test_tiff_fixture.cpp)
target_link_libraries(test_tiff_fixture ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY})
set_target_properties(test_tiff_fixture PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/bench")




add_executable(test_huffman_utils_impl test_huffman_utils_impl.cpp)
target_link_libraries(test_huffman_utils_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY})

add_executable(test_volume_fixtures test_volume_fixtures.cpp)
target_link_libraries(test_volume_fixtures ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY})

add_executable(test_sqeazy_pipelines_impl test_sqeazy_pipelines_impl.cpp)
add_executable(test_hdf5_impl test_hdf5_impl.cpp)

IF((${WITH_FFMPEG} EQUAL "ON") AND (DEFINED ${FFMPEG_FOUND}))
  if(WIN32)
    LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}\\lib)
    else()
    LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)
    endif()

  SET(SQY_FFMPEG_Libraries ${FFMPEG_LIBRARIES})

  #if(NOT EXTRA_LIBM_NOT_NEEDED)
    #set(SQY_FFMPEG_Libraries ${SQY_FFMPEG_Libraries} m)
  #endif()

  if(DEFINED FFMPEG_EXTRA_LINK_FLAGS AND DEFINED FFMPEG_USE_STATIC_LIBS)
    set(SQY_FFMPEG_Libraries ${SQY_FFMPEG_Libraries} ${FFMPEG_EXTRA_LINK_FLAGS})
  endif()

  target_link_libraries(test_sqeazy_pipelines_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries} ${LZ4_LIBRARY})

  add_executable(test_avcodec_sandbox test_avcodec_sandbox.cpp)
  target_link_libraries(test_avcodec_sandbox ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})

  add_executable(test_hevc_scheme_impl test_hevc_scheme_impl.cpp)
  target_link_libraries(test_hevc_scheme_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})

  add_executable(test_h264_scheme_impl test_h264_scheme_impl.cpp)
  target_link_libraries(test_h264_scheme_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})

  add_executable(test_video_utils_impl test_video_utils_impl.cpp)
  target_link_libraries(test_video_utils_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})



  add_executable(test_hevc_scheme_utils test_hevc_scheme_utils.cpp)
  target_link_libraries(test_hevc_scheme_utils ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})





  target_link_libraries(test_hdf5_impl ${HDF5_LIBRARIES} ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY} ${SQY_FFMPEG_Libraries})

ELSE()
  MESSAGE(">> [tests] ffmpeg switched off or libavcodec not found not found. skipping test_avcodec_sandbox ...")
  target_link_libraries(test_sqeazy_pipelines_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${LZ4_LIBRARY})
  target_link_libraries(test_hdf5_impl ${HDF5_LIBRARIES} ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY})
ENDIF()



#target_link_libraries(test_bench_utils_impl ${OpenMP++_LIBRARIES} ${Boost_LIBRARIES} ${TIFF_LIBRARY})

ENDIF(TIFF_FOUND)

#http://stackoverflow.com/questions/14474659/cmake-how-to-have-a-target-for-copying-files

if(NOT WIN32)
#required by the yuv_utils
FILE(COPY ${PROJECT_SOURCE_DIR}/tests/mini_example_w92_h128.yuv DESTINATION ${PROJECT_BINARY_DIR}/tests/)
FILE(COPY ${PROJECT_SOURCE_DIR}/tests/mini_example.y4m DESTINATION ${PROJECT_BINARY_DIR}/tests/)

#required by the hdf5 tests
FILE(COPY ${PROJECT_SOURCE_DIR}/tests/hdf5_create_sample.h5 DESTINATION ${PROJECT_BINARY_DIR}/tests/)
FILE(RENAME ${PROJECT_BINARY_DIR}/tests/hdf5_create_sample.h5 ${PROJECT_BINARY_DIR}/tests/sample.h5)
else()
#required by the yuv_utils
FILE(COPY ${PROJECT_SOURCE_DIR}/tests/mini_example_w92_h128.yuv DESTINATION ${PROJECT_BINARY_DIR}/tests/${CMAKE_BUILD_TYPE})
FILE(COPY ${PROJECT_SOURCE_DIR}/tests/mini_example.y4m DESTINATION ${PROJECT_BINARY_DIR}/tests/${CMAKE_BUILD_TYPE})

#required by the hdf5 tests
FILE(COPY ${PROJECT_SOURCE_DIR}/tests/hdf5_create_sample.h5 DESTINATION ${PROJECT_BINARY_DIR}/tests/${CMAKE_BUILD_TYPE})
FILE(RENAME ${PROJECT_BINARY_DIR}/tests/${CMAKE_BUILD_TYPE}/hdf5_create_sample.h5 ${PROJECT_BINARY_DIR}/tests/${CMAKE_BUILD_TYPE}/sample.h5)
endif()
