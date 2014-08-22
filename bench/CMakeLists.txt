INCLUDE_DIRECTORIES(.)

FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system filesystem unit_test_framework REQUIRED)
IF(Boost_FOUND)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()


FIND_PACKAGE(TIFF REQUIRED)
IF(TIFF_FOUND)
add_executable(bench bench.cpp)
# target_link_libraries(bench ${PROJECT_NAME} tiff)
target_link_libraries(bench tiff lz4 boost_filesystem boost_system)
set_target_properties(bench PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/bench/cmake)

FIND_PACKAGE(ROOT QUIET)
IF(ROOT_FOUND)
add_executable(histo histo.cpp)
target_link_libraries(histo tiff ${ROOT_LIBRARIES})

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --cflags
    OUTPUT_VARIABLE ROOT_INCLUDE_FLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set_target_properties(histo PROPERTIES COMPILE_FLAGS ${ROOT_INCLUDE_FLAGS})
Message("Building histo, added \n\t${ROOT_LIBRARIES}\n\t${ROOT_INCLUDE_FLAGS}")

ELSE(ROOT_FOUND)
message(">> Skipping optional bench/histo utility, ROOT libraries not found!")
ENDIF(ROOT_FOUND)


ELSE(TIFF_FOUND)
WARNING("Skipping benchmark utils, libtiff not found (but required for build)\!")
ENDIF(TIFF_FOUND)


