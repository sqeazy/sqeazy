INCLUDE_DIRECTORIES(.)

#SET(HDF5_USE_STATIC_LIBRARIES TRUE)#only works on Unix
FIND_PACKAGE(HDF5 QUIET REQUIRED CXX )
IF(HDF5_FOUND)
INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
ADD_DEFINITIONS(${HDF5_DEFINITIONS})
LINK_DIRECTORIES(${HDF5_LIBRARY_DIRS}) 
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

FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system filesystem unit_test_framework REQUIRED)
IF(Boost_FOUND)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()


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
set_target_properties(test_bitplane_reorder_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

add_executable(test_shift_by_intrinsics test_shift_by_intrinsics.cpp)
target_link_libraries(test_shift_by_intrinsics ${Boost_LIBRARIES})
set_target_properties(test_shift_by_intrinsics PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

add_executable(test_rotate_by_intrinsics test_rotate_by_intrinsics.cpp)
target_link_libraries(test_rotate_by_intrinsics ${Boost_LIBRARIES})
set_target_properties(test_rotate_by_intrinsics PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

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



if(EXISTS ${LZ4_STATIC_LIB})
#if static libsqeazy exists, it will contain lz4, no need to link against it yet another time
target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${Boost_LIBRARIES})
target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES})
else()
  target_link_libraries(test_lz4_encoding ${PROJECT_NAME} ${Boost_LIBRARIES} ext_lz4)
  target_link_libraries(test_hdf5_interface ${PROJECT_NAME} ${Boost_LIBRARIES} ext_lz4 ext_hdf5)
endif()

add_executable(test_hdf5_impl test_hdf5_impl.cpp)
set_target_properties(test_hdf5_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src ")
target_link_libraries(test_hdf5_impl ${HDF5_LIBRARIES} ${Boost_LIBRARIES})


add_executable(test_apply test_apply.cpp)
target_link_libraries(test_apply ${Boost_LIBRARIES} ext_lz4)


IF(TIFF_FOUND)

add_executable(test_tiff_fixture test_tiff_fixture.cpp)
target_link_libraries(test_tiff_fixture ${Boost_LIBRARIES} ext_tiff)
set_target_properties(test_tiff_fixture PROPERTIES COMPILE_FLAGS "-std=c++11 -I${PROJECT_SOURCE_DIR}/bench")

add_executable(test_pipeline_select test_pipeline_select.cpp)
target_link_libraries(test_pipeline_select ${Boost_LIBRARIES} ext_tiff ext_lz4)
set_target_properties(test_pipeline_select PROPERTIES COMPILE_FLAGS "-std=c++11 -I${PROJECT_SOURCE_DIR}/bench -I${PROJECT_SOURCE_DIR}/src ")


add_executable(test_huffman_utils_impl test_huffman_utils_impl.cpp)
target_link_libraries(test_huffman_utils_impl ${Boost_LIBRARIES} ext_tiff)



FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system filesystem unit_test_framework thread date_time REQUIRED)
target_link_libraries(test_bench_utils_impl ${Boost_LIBRARIES} ext_tiff)
set_target_properties(test_bench_utils_impl PROPERTIES COMPILE_FLAGS "-std=c++11 -I${PROJECT_SOURCE_DIR}/src")


ENDIF(TIFF_FOUND)




