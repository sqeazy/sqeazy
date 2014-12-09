INCLUDE_DIRECTORIES(.)

FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system filesystem unit_test_framework REQUIRED)
IF(Boost_FOUND)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()

add_executable(test_apply test_apply.cpp)
target_link_libraries(test_apply boost_system boost_filesystem boost_unit_test_framework lz4)

add_executable(test_diff_schemes test_diff_schemes.cpp)
target_link_libraries(test_diff_schemes ${PROJECT_NAME} boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_diff_scheme_impl test_diff_scheme_impl.cpp)
target_link_libraries(test_diff_scheme_impl boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_bitswap_schemes test_bitswap_schemes.cpp)
target_link_libraries(test_bitswap_schemes ${PROJECT_NAME} boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_bitswap_scheme_impl test_bitswap_scheme_impl.cpp)
target_link_libraries(test_bitswap_scheme_impl boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_bitplane_reorder_impl test_bitplane_reorder_impl.cpp)
target_link_libraries(test_bitplane_reorder_impl boost_system boost_filesystem boost_unit_test_framework)
set_target_properties(test_bitplane_reorder_impl PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src -march=native")

add_executable(test_lz4_encoding test_lz4_encoding.cpp)
target_link_libraries(test_lz4_encoding ${PROJECT_NAME} boost_system boost_filesystem boost_unit_test_framework lz4)


add_executable(test_sqeazy_header_impl test_sqeazy_header_impl.cpp)
target_link_libraries(test_sqeazy_header_impl ${PROJECT_NAME} boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_background_schemes test_background_schemes.cpp)
target_link_libraries(test_background_schemes ${PROJECT_NAME} boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_background_scheme_impl test_background_scheme_impl.cpp)
target_link_libraries(test_background_scheme_impl ${PROJECT_NAME} boost_system boost_filesystem boost_unit_test_framework)


add_executable(test_hist_impl test_hist_impl.cpp)
target_link_libraries(test_hist_impl boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_bench_utils_impl test_bench_utils_impl.cpp)
target_link_libraries(test_bench_utils_impl boost_system boost_filesystem boost_unit_test_framework tiff)
set_target_properties(test_bench_utils_impl PROPERTIES COMPILE_FLAGS "-std=c++11 -I${PROJECT_SOURCE_DIR}/src")

add_executable(test_tiff_fixture test_tiff_fixture.cpp)
target_link_libraries(test_tiff_fixture boost_system boost_filesystem boost_unit_test_framework tiff)
set_target_properties(test_tiff_fixture PROPERTIES COMPILE_FLAGS "-std=c++11 -I${PROJECT_SOURCE_DIR}/bench")

add_executable(test_pipeline_select test_pipeline_select.cpp)
target_link_libraries(test_pipeline_select boost_system boost_filesystem boost_unit_test_framework tiff lz4)
set_target_properties(test_pipeline_select PROPERTIES COMPILE_FLAGS "-std=c++11 -I${PROJECT_SOURCE_DIR}/bench -I${PROJECT_SOURCE_DIR}/src ")


add_executable(test_huffman_utils_impl test_huffman_utils_impl.cpp)
target_link_libraries(test_huffman_utils_impl boost_system boost_filesystem boost_unit_test_framework tiff)
# FIND_PACKAGE(TIFF REQUIRED)
# IF(TIFF_FOUND)
# add_executable(test_view_from_disk_fixture test_view_from_disk_fixture.cpp)
# target_link_libraries(test_view_from_disk_fixture boost_system boost_filesystem boost_unit_test_framework tiff)
# ELSE(TIFF_FOUND)
# WARNING("Skipping tests based on reference images, libtiff not found\!")
# ENDIF(TIFF_FOUND)


