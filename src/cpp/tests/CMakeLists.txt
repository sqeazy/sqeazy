#include project relevant directories
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

#needed for FindHDF5 as it expects an ENVIRONMENT variable called HDF5_ROOT as hint to the HDF5 path
IF(EXISTS ${HDF5_ROOT})
	SET(ENV{HDF5_ROOT} ${HDF5_ROOT})
ENDIF(EXISTS ${HDF5_ROOT})

#SET(HDF5_USE_STATIC_LIBRARIES TRUE)#only works on Unix
FIND_PACKAGE(HDF5 1.8 QUIET REQUIRED CXX)
IF(HDF5_FOUND)
INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
ADD_DEFINITIONS(${HDF5_DEFINITIONS})
LINK_DIRECTORIES(${HDF5_LIBRARY_DIRS}) 
ENDIF()

IF(TIFF_ROOT)
  SET(TIFF_DIR ${TIFF_ROOT})
ENDIF()

FIND_PACKAGE(TIFF REQUIRED)
IF(TIFF_FOUND)
INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
ENDIF(TIFF_FOUND)

##LZ4, HDF5. TIFF
IF(WIN32)
  set(Boost_USE_STATIC_LIBS ON)

  add_library(ext_lz4 STATIC IMPORTED)
  set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_STATIC_LIB}")
  
  add_library(ext_hdf5 SHARED IMPORTED)
  set_target_properties(ext_hdf5 PROPERTIES IMPORTED_LOCATION "${HDF5_LIBRARIES}")

  add_library(ext_tiff STATIC IMPORTED)

  set(SQY_LIB ${PROJECT_NAME}_static)
  
ELSE(WIN32)

  add_library(ext_lz4 SHARED IMPORTED)
  set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_SHARED_LIB}")

  add_library(ext_hdf5 SHARED IMPORTED)
  set_target_properties(ext_hdf5 PROPERTIES IMPORTED_LOCATION "${HDF5_LIBRARIES}") 

  add_library(ext_tiff SHARED IMPORTED)
  add_definitions(-DBOOST_TEST_DYN_LINK)

  set(SQY_LIB ${PROJECT_NAME})
ENDIF(WIN32)



set_target_properties(ext_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_LIBRARIES}") 

FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS regex system filesystem unit_test_framework REQUIRED)
IF(Boost_FOUND)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()

add_executable(test_string_parsers_impl test_string_parsers_impl.cpp)
target_link_libraries(test_string_parsers_impl ${Boost_LIBRARIES})

add_executable(test_diff_schemes test_diff_schemes.cpp)
target_link_libraries(test_diff_schemes ${SQY_LIB} ${Boost_LIBRARIES})

add_executable(test_diff_scheme_impl test_diff_scheme_impl.cpp)
target_link_libraries(test_diff_scheme_impl ${Boost_LIBRARIES})

add_executable(test_bitswap_schemes test_bitswap_schemes.cpp)
target_link_libraries(test_bitswap_schemes ${SQY_LIB} ${Boost_LIBRARIES})

add_executable(test_bitswap_scheme_impl test_bitswap_scheme_impl.cpp)
target_link_libraries(test_bitswap_scheme_impl ${Boost_LIBRARIES})

add_executable(test_bitplane_reorder_impl test_bitplane_reorder_impl.cpp)

target_link_libraries(test_bitplane_reorder_impl ${Boost_LIBRARIES})
# set_target_properties(test_bitplane_reorder_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

add_executable(test_shift_by_intrinsics test_shift_by_intrinsics.cpp)
target_link_libraries(test_shift_by_intrinsics ${Boost_LIBRARIES})
# set_target_properties(test_shift_by_intrinsics PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

add_executable(test_rotate_by_intrinsics test_rotate_by_intrinsics.cpp)
target_link_libraries(test_rotate_by_intrinsics ${Boost_LIBRARIES})
# set_target_properties(test_rotate_by_intrinsics PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

add_executable(test_sqeazy_utils_impl test_sqeazy_utils_impl.cpp)
target_link_libraries(test_sqeazy_utils_impl ${Boost_LIBRARIES} )

add_executable(test_sqeazy_header_impl test_sqeazy_header_impl.cpp)
target_link_libraries(test_sqeazy_header_impl ${SQY_LIB} ${Boost_LIBRARIES})

add_executable(test_background_schemes test_background_schemes.cpp)
target_link_libraries(test_background_schemes ${SQY_LIB} ${Boost_LIBRARIES})

add_executable(test_background_scheme_impl test_background_scheme_impl.cpp)
target_link_libraries(test_background_scheme_impl ${SQY_LIB} ${Boost_LIBRARIES})


add_executable(test_hist_impl test_hist_impl.cpp)
target_link_libraries(test_hist_impl ${Boost_LIBRARIES})
add_executable(test_bench_utils_impl test_bench_utils_impl.cpp)


add_executable(test_lz4_encoding test_lz4_encoding.cpp)
add_executable(test_hdf5_interface test_hdf5_interface.cpp)
set_target_properties(test_hdf5_interface PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src ")
add_executable(test_pipeline_interface test_pipeline_interface.cpp)
set_target_properties(test_pipeline_interface PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src ")


if(EXISTS ${LZ4_STATIC_LIB})
  #if static libsqeazy exists, it will contain lz4, no need to link against it yet another time
  target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${Boost_LIBRARIES})
  target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES})
  target_link_libraries(test_pipeline_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES})
else()
  target_link_libraries(test_lz4_encoding ${PROJECT_NAME} ${Boost_LIBRARIES} ext_lz4)
  target_link_libraries(test_hdf5_interface ${PROJECT_NAME} ${Boost_LIBRARIES} ext_lz4 ext_hdf5)
  target_link_libraries(test_pipeline_interface ${PROJECT_NAME} ${Boost_LIBRARIES} ext_lz4)
endif()

add_executable(test_hdf5_impl test_hdf5_impl.cpp)
set_target_properties(test_hdf5_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src ")
target_link_libraries(test_hdf5_impl ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ext_lz4)


add_executable(test_dynamic_pipeline_impl test_dynamic_pipeline_impl.cpp)
target_link_libraries(test_dynamic_pipeline_impl ${Boost_LIBRARIES} ext_lz4)


add_executable(test_dynamic_stage_impl test_dynamic_stage_impl.cpp)
target_link_libraries(test_dynamic_stage_impl ${Boost_LIBRARIES})

add_executable(test_dynamic_stage_factory_impl test_dynamic_stage_factory_impl.cpp)
target_link_libraries(test_dynamic_stage_factory_impl ${Boost_LIBRARIES})

add_executable(test_quantiser_impl test_quantiser_impl.cpp)
target_link_libraries(test_quantiser_impl ${Boost_LIBRARIES})
set_target_properties(test_quantiser_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src")  

add_executable(test_yuv_utils test_yuv_utils.cpp)
target_link_libraries(test_yuv_utils ${Boost_LIBRARIES})
set_target_properties(test_yuv_utils PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src")  


IF(TIFF_FOUND)

add_executable(test_tiff_fixture test_tiff_fixture.cpp)
target_link_libraries(test_tiff_fixture ${Boost_LIBRARIES} ext_tiff)
set_target_properties(test_tiff_fixture PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/bench")




add_executable(test_huffman_utils_impl test_huffman_utils_impl.cpp)
target_link_libraries(test_huffman_utils_impl ${Boost_LIBRARIES} ext_tiff)

add_executable(test_volume_fixtures test_volume_fixtures.cpp)
target_link_libraries(test_volume_fixtures ${Boost_LIBRARIES} ext_tiff)




set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(FFMPEG REQUIRED)

IF(FFMPEG_FOUND)
  INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
  LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)
    
  add_library(ext_avcodec SHARED IMPORTED)
  set_target_properties(ext_avcodec PROPERTIES IMPORTED_LOCATION "${FFMPEG_AVCODEC_LIBRARIES}")

  add_library(ext_avutil SHARED IMPORTED)
  set_target_properties(ext_avutil PROPERTIES IMPORTED_LOCATION "${FFMPEG_AVUTIL_LIBRARIES}")

  add_library(ext_avformat SHARED IMPORTED)
  set_target_properties(ext_avformat PROPERTIES IMPORTED_LOCATION "${FFMPEG_AVFORMAT_LIBRARIES}")

  add_library(ext_swscale SHARED IMPORTED)
  set_target_properties(ext_swscale PROPERTIES IMPORTED_LOCATION "${FFMPEG_SWSCALE_LIBRARIES}")

  add_executable(test_avcodec_sandbox test_avcodec_sandbox.cpp)
  target_link_libraries(test_avcodec_sandbox ${Boost_LIBRARIES} ext_tiff ext_avcodec ext_avutil ext_avformat ext_swscale m)

  add_executable(deprecated_pipeline_select_impl deprecated_pipeline_select_impl.cpp)
  target_link_libraries(deprecated_pipeline_select_impl ${Boost_LIBRARIES} ext_tiff ext_lz4 ext_avcodec ext_avutil ext_avformat ext_swscale m)
#  set_target_properties(deprecated_pipeline_select_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src ")

  add_executable(deprecated_pipeline_impl deprecated_pipeline_impl.cpp)
  target_link_libraries(deprecated_pipeline_impl ${Boost_LIBRARIES} ext_lz4 ext_avcodec ext_avutil ext_avformat ext_swscale m)

  add_executable(test_hevc_scheme_impl test_hevc_scheme_impl.cpp)
  target_link_libraries(test_hevc_scheme_impl ${Boost_LIBRARIES} ext_tiff ext_avcodec ext_avutil ext_avformat ext_swscale m)

  add_executable(test_h264_scheme_impl test_h264_scheme_impl.cpp)
  target_link_libraries(test_h264_scheme_impl ${Boost_LIBRARIES} ext_tiff ext_avcodec ext_avutil ext_avformat ext_swscale m)
#  set_target_properties(test_hevc_scheme_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src")  

  add_executable(test_hevc_scheme_utils test_hevc_scheme_utils.cpp)
  target_link_libraries(test_hevc_scheme_utils ${Boost_LIBRARIES} ext_tiff ext_avcodec ext_avutil ext_avformat ext_swscale m)
#  set_target_properties(test_hevc_scheme_utils PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src")  

  add_executable(test_sqeazy_pipelines_impl test_sqeazy_pipelines_impl.cpp)
  target_link_libraries(test_sqeazy_pipelines_impl ${Boost_LIBRARIES} ext_tiff ext_avcodec ext_avutil ext_avformat ext_swscale m ext_lz4)

  
ELSE(FFMPEG_FOUND)
  MESSAGE(">> [tests] libavcodec not found. skipping test_avcodec_sandbox ...")
ENDIF(FFMPEG_FOUND)

FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system filesystem unit_test_framework thread date_time REQUIRED)
target_link_libraries(test_bench_utils_impl ${Boost_LIBRARIES} ext_tiff)
#set_target_properties(test_bench_utils_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src")  




ENDIF(TIFF_FOUND)

FILE(COPY ${PROJECT_SOURCE_DIR}/tests/mini_example_w92_h128.yuv DESTINATION ${PROJECT_BINARY_DIR}/tests/)
FILE(COPY ${PROJECT_SOURCE_DIR}/tests/mini_example.y4m DESTINATION ${PROJECT_BINARY_DIR}/tests/)
FILE(COPY ${PROJECT_SOURCE_DIR}/tests/hdf5_create_sample.h5 DESTINATION ${PROJECT_BINARY_DIR}/tests/)
FILE(RENAME ${PROJECT_BINARY_DIR}/tests/hdf5_create_sample.h5 ${PROJECT_BINARY_DIR}/tests/sample.h5)
