INCLUDE_DIRECTORIES(.)

IF(WIN32)
SET(Boost_USE_STATIC_LIBS ON)
add_library(ext_lz4 STATIC IMPORTED)
set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_STATIC_LIB}") 
ELSE(WIN32)
add_library(ext_lz4 SHARED IMPORTED)
set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_SHARED_LIB}") 
ENDIF(WIN32)

FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system filesystem unit_test_framework REQUIRED)
IF(Boost_FOUND)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()

#FIXME: not portable
IF(DEFINED ${BOOST_ROOT} AND ${BOOST_ROOT} MATCHES ".*/.*")
  SET(BOOST_LIB_DIR ${BOOST_ROOT}/lib)
ELSE()
  SET(BOOST_LIB_DIR ${Boost_LIBRARY_DIRS})
ENDIF()


add_library( ext_boost_system  SHARED IMPORTED )
set_target_properties( ext_boost_system PROPERTIES IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_system.so )

add_library( ext_boost_filesystem  SHARED IMPORTED )
set_target_properties( ext_boost_filesystem PROPERTIES IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_filesystem.so )

add_library( ext_boost_unit_test_framework  SHARED IMPORTED )
set_target_properties( ext_boost_unit_test_framework PROPERTIES IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_unit_test_framework.so )


#this is the way that is recommended by FindBOOST.cmake
#FIXME: see what works better on windows
add_executable(test_diff_schemes test_diff_schemes.cpp)
target_link_libraries(test_diff_schemes ${PROJECT_NAME} ${Boost_LIBRARIES})

#this is the way that seems more flexible to me
#FIXME: see what works better on windows
add_executable(test_diff_scheme_impl test_diff_scheme_impl.cpp)
target_link_libraries(test_diff_scheme_impl ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)

add_executable(test_bitswap_schemes test_bitswap_schemes.cpp)
target_link_libraries(test_bitswap_schemes ${PROJECT_NAME} ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)

add_executable(test_bitswap_scheme_impl test_bitswap_scheme_impl.cpp)
target_link_libraries(test_bitswap_scheme_impl ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)

add_executable(test_bitplane_reorder_impl test_bitplane_reorder_impl.cpp)

target_link_libraries(test_bitplane_reorder_impl ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)
set_target_properties(test_bitplane_reorder_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

add_executable(test_shift_by_intrinsics test_shift_by_intrinsics.cpp)
target_link_libraries(test_shift_by_intrinsics ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)
set_target_properties(test_shift_by_intrinsics PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

add_executable(test_rotate_by_intrinsics test_rotate_by_intrinsics.cpp)
target_link_libraries(test_rotate_by_intrinsics ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)
set_target_properties(test_rotate_by_intrinsics PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

add_executable(test_sqeazy_header_impl test_sqeazy_header_impl.cpp)
target_link_libraries(test_sqeazy_header_impl ${PROJECT_NAME} ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)

add_executable(test_background_schemes test_background_schemes.cpp)
target_link_libraries(test_background_schemes ${PROJECT_NAME} ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)

add_executable(test_background_scheme_impl test_background_scheme_impl.cpp)
target_link_libraries(test_background_scheme_impl ${PROJECT_NAME} ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)


add_executable(test_hist_impl test_hist_impl.cpp)
target_link_libraries(test_hist_impl ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)

add_executable(test_bench_utils_impl test_bench_utils_impl.cpp)

target_link_libraries(test_bench_utils_impl ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework tiff)
set_target_properties(test_bench_utils_impl PROPERTIES COMPILE_FLAGS "-std=c++11 -I${PROJECT_SOURCE_DIR}/src")

add_executable(test_tiff_fixture test_tiff_fixture.cpp)
target_link_libraries(test_tiff_fixture ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework tiff)
set_target_properties(test_tiff_fixture PROPERTIES COMPILE_FLAGS "-std=c++11 -I${PROJECT_SOURCE_DIR}/bench")

add_executable(test_lz4_encoding test_lz4_encoding.cpp)
if(EXISTS ${LZ4_STATIC_LIB})
target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework)
else()
target_link_libraries(test_lz4_encoding ${PROJECT_NAME} ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework lz4)
endif()

add_executable(test_apply test_apply.cpp)
target_link_libraries(test_apply ${Boost_LIBRARIES} ext_lz4)

add_executable(test_pipeline_select test_pipeline_select.cpp)
target_link_libraries(test_pipeline_select ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework tiff lz4)
set_target_properties(test_pipeline_select PROPERTIES COMPILE_FLAGS "-std=c++11 -I${PROJECT_SOURCE_DIR}/bench -I${PROJECT_SOURCE_DIR}/src ")


add_executable(test_huffman_utils_impl test_huffman_utils_impl.cpp)
target_link_libraries(test_huffman_utils_impl ext_boost_system ext_boost_filesystem ext_boost_unit_test_framework tiff)



