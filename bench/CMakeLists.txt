INCLUDE_DIRECTORIES(.)

# FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system filesystem unit_test_framework REQUIRED)
# IF(Boost_FOUND)
# INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
# LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
# ENDIF()


FIND_PACKAGE(TIFF REQUIRED)
IF(TIFF_FOUND)
add_executable(bench bench.cpp)
target_link_libraries(bench ${PROJECT_NAME} tiff)
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")
ELSE(TIFF_FOUND)
WARNING("Skipping tests based on reference images, libtiff not found\!")
ENDIF(TIFF_FOUND)


