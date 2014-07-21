INCLUDE_DIRECTORIES(.)

FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system filesystem unit_test_framework REQUIRED)
IF(Boost_FOUND)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()

add_executable(test_apply test_apply.cpp)
target_link_libraries(test_apply boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_diff_schemes test_diff_schemes.cpp)
target_link_libraries(test_diff_schemes ${PROJECT_NAME} boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_diff_scheme_impl test_diff_scheme_impl.cpp)
target_link_libraries(test_diff_scheme_impl boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_bitswap_schemes test_bitswap_schemes.cpp)
target_link_libraries(test_bitswap_schemes ${PROJECT_NAME} boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_bitswap_scheme_impl test_bitswap_scheme_impl.cpp)
target_link_libraries(test_bitswap_scheme_impl boost_system boost_filesystem boost_unit_test_framework)

add_executable(test_lz4_encoding test_lz4_encoding.cpp)
target_link_libraries(test_lz4_encoding ${PROJECT_NAME} boost_system boost_filesystem boost_unit_test_framework)

# FIND_PACKAGE(TIFF REQUIRED)
# IF(TIFF_FOUND)
# add_executable(test_view_from_disk_fixture test_view_from_disk_fixture.cpp)
# target_link_libraries(test_view_from_disk_fixture boost_system boost_filesystem boost_unit_test_framework tiff)
# ELSE(TIFF_FOUND)
# WARNING("Skipping tests based on reference images, libtiff not found\!")
# ENDIF(TIFF_FOUND)


