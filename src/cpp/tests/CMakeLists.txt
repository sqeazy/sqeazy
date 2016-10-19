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


message(STATUS "[tests] linking against ${SQY_LIB} ${Boost_LIBRARIES} ")

add_executable(test_compass_impl test_compass_impl.cpp)
target_link_libraries(test_compass_impl ${Boost_LIBRARIES})

add_executable(test_string_parsers_impl test_string_parsers_impl.cpp)
target_link_libraries(test_string_parsers_impl ${Boost_LIBRARIES})

add_executable(test_string_shapers_impl test_string_shapers_impl.cpp)
target_link_libraries(test_string_shapers_impl ${Boost_LIBRARIES})


add_executable(test_diff_scheme_impl test_diff_scheme_impl.cpp)
target_link_libraries(test_diff_scheme_impl ${Boost_LIBRARIES})


add_executable(test_bitswap_scheme_impl test_bitswap_scheme_impl.cpp)
target_link_libraries(test_bitswap_scheme_impl ${Boost_LIBRARIES})

add_executable(test_bitplane_reorder_impl test_bitplane_reorder_impl.cpp)
target_link_libraries(test_bitplane_reorder_impl ${Boost_LIBRARIES})

add_executable(bench_bitplane_reorder_impl bench_bitplane_reorder_impl.cpp)
target_link_libraries(bench_bitplane_reorder_impl ${Boost_LIBRARIES})

add_executable(test_sse_utils_impl test_sse_utils_impl.cpp)
target_link_libraries(test_sse_utils_impl ${Boost_LIBRARIES})

add_executable(test_shift_by_intrinsics test_shift_by_intrinsics.cpp)
target_link_libraries(test_shift_by_intrinsics ${Boost_LIBRARIES})

add_executable(test_rotate_by_intrinsics test_rotate_by_intrinsics.cpp)
target_link_libraries(test_rotate_by_intrinsics ${Boost_LIBRARIES})

add_executable(test_sqeazy_utils_impl test_sqeazy_utils_impl.cpp)
target_link_libraries(test_sqeazy_utils_impl ${Boost_LIBRARIES} )

add_executable(test_sqeazy_header_impl test_sqeazy_header_impl.cpp)
target_link_libraries(test_sqeazy_header_impl ${Boost_LIBRARIES})

add_executable(test_background_scheme_impl test_background_scheme_impl.cpp)
target_link_libraries(test_background_scheme_impl # ${SQY_LIB}
  ${Boost_LIBRARIES})


add_executable(test_hist_impl test_hist_impl.cpp)
target_link_libraries(test_hist_impl ${Boost_LIBRARIES})
add_executable(test_bench_utils_impl test_bench_utils_impl.cpp)


add_executable(test_lz4_encoding test_lz4_encoding.cpp)
add_executable(test_hdf5_interface test_hdf5_interface.cpp)


add_executable(test_pipeline_interface test_pipeline_interface.cpp)



if(EXISTS ${LZ4_STATIC_LIB})
  #if static libsqeazy exists, it will contain lz4, no need to link against it yet another time
  if(NOT DEFINED READY_4_BUNDLE)
    target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${Boost_LIBRARIES})
    target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES})
    target_link_libraries(test_pipeline_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES})
  else()
    target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
    target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
    target_link_libraries(test_pipeline_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
  endif()
else()
  target_link_libraries(test_lz4_encoding ${SQY_LIB} ${Boost_LIBRARIES} ${LZ4_LIBRARY})
  target_link_libraries(test_hdf5_interface ${SQY_LIB} ${Boost_LIBRARIES} ${LZ4_LIBRARY} ${HDF5_LIBRARIES})
  target_link_libraries(test_pipeline_interface ${SQY_LIB} ${Boost_LIBRARIES} ${LZ4_LIBRARY})
endif()



add_executable(test_dynamic_pipeline_impl test_dynamic_pipeline_impl.cpp)
target_link_libraries(test_dynamic_pipeline_impl ${Boost_LIBRARIES} ${LZ4_LIBRARY})


add_executable(test_dynamic_stage_impl test_dynamic_stage_impl.cpp)
target_link_libraries(test_dynamic_stage_impl ${Boost_LIBRARIES})

add_executable(test_dynamic_stage_factory_impl test_dynamic_stage_factory_impl.cpp)
target_link_libraries(test_dynamic_stage_factory_impl ${Boost_LIBRARIES})

add_executable(test_quantiser_impl test_quantiser_impl.cpp)
target_link_libraries(test_quantiser_impl ${Boost_LIBRARIES})


add_executable(test_yuv_utils test_yuv_utils.cpp)
target_link_libraries(test_yuv_utils ${Boost_LIBRARIES})


add_executable(test_background_schemes test_background_schemes.cpp)
target_link_libraries(test_background_schemes ${SQY_LIB} ${Boost_LIBRARIES})

add_executable(test_diff_schemes test_diff_schemes.cpp)
target_link_libraries(test_diff_schemes ${SQY_LIB} ${Boost_LIBRARIES})

add_executable(test_bitswap_schemes test_bitswap_schemes.cpp)
target_link_libraries(test_bitswap_schemes ${SQY_LIB} ${Boost_LIBRARIES})


IF(TIFF_FOUND)

add_executable(test_tiff_fixture test_tiff_fixture.cpp)
target_link_libraries(test_tiff_fixture ${Boost_LIBRARIES} ${TIFF_LIBRARY})
set_target_properties(test_tiff_fixture PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/bench")




add_executable(test_huffman_utils_impl test_huffman_utils_impl.cpp)
target_link_libraries(test_huffman_utils_impl ${Boost_LIBRARIES} ${TIFF_LIBRARY})

add_executable(test_volume_fixtures test_volume_fixtures.cpp)
target_link_libraries(test_volume_fixtures ${Boost_LIBRARIES} ${TIFF_LIBRARY})


IF(FFMPEG_FOUND)
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

  
  add_executable(test_avcodec_sandbox test_avcodec_sandbox.cpp)
  target_link_libraries(test_avcodec_sandbox ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})

  add_executable(deprecated_pipeline_select_impl deprecated_pipeline_select_impl.cpp)
  target_link_libraries(deprecated_pipeline_select_impl ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${TIFF_LIBRARY} ${LZ4_LIBRARY} ${SQY_FFMPEG_Libraries})


  add_executable(deprecated_pipeline_impl deprecated_pipeline_impl.cpp)
  target_link_libraries(deprecated_pipeline_impl ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${LZ4_LIBRARY} ${SQY_FFMPEG_Libraries})

  add_executable(test_hevc_scheme_impl test_hevc_scheme_impl.cpp)
  target_link_libraries(test_hevc_scheme_impl ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})

  add_executable(test_h264_scheme_impl test_h264_scheme_impl.cpp)
  target_link_libraries(test_h264_scheme_impl ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})

  add_executable(test_video_utils_impl test_video_utils_impl.cpp)
  target_link_libraries(test_video_utils_impl ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})



  add_executable(test_hevc_scheme_utils test_hevc_scheme_utils.cpp)
  target_link_libraries(test_hevc_scheme_utils ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})


  add_executable(test_sqeazy_pipelines_impl test_sqeazy_pipelines_impl.cpp)
  target_link_libraries(test_sqeazy_pipelines_impl ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries} ${LZ4_LIBRARY})

  add_executable(test_hdf5_impl test_hdf5_impl.cpp)
  target_link_libraries(test_hdf5_impl ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${LZ4_LIBRARY} ${SQY_FFMPEG_Libraries})

ELSE(FFMPEG_FOUND)
  MESSAGE(">> [tests] libavcodec not found. skipping test_avcodec_sandbox ...")
ENDIF(FFMPEG_FOUND)


target_link_libraries(test_bench_utils_impl ${Boost_LIBRARIES} ${TIFF_LIBRARY})

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
