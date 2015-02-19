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
SET(BENCH_COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src")

IF(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
SET(BENCH_COMPILE_FLAGS "${BENCH_COMPILE_FLAGS} ${HAS_CPP11_COMPILERFLAG}")
ENDIF()

if(SQY_VERBOSE)
list(APPEND BENCH_COMPILE_FLAGS " -D_SQY_VERBOSE_=1")
endif(SQY_VERBOSE)

STRING(REPLACE ";" " " BENCH_COMPILE_FLAGS ${BENCH_COMPILE_FLAGS})
message(">> adding compile flag to bench: ${BENCH_COMPILE_FLAGS}")
set_target_properties(bench PROPERTIES COMPILE_FLAGS "${BENCH_COMPILE_FLAGS}")

add_executable(sqy sqy.cpp)

target_link_libraries(sqy tiff lz4 boost_filesystem boost_system boost_program_options)
SET(SQY_COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src")

IF(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
SET(SQY_COMPILE_FLAGS "${BENCH_COMPILE_FLAGS} ${HAS_CPP11_COMPILERFLAG}")
ENDIF()

if(SQY_VERBOSE)
list(APPEND SQY_COMPILE_FLAGS " -D_SQY_VERBOSE_=1")
endif(SQY_VERBOSE)

STRING(REPLACE ";" " " SQY_COMPILE_FLAGS ${SQY_COMPILE_FLAGS})
message(">> adding compile flag to sqy: ${SQY_COMPILE_FLAGS}")
set_target_properties(sqy PROPERTIES COMPILE_FLAGS "${SQY_COMPILE_FLAGS}")



FIND_PACKAGE(ROOT QUIET)
IF(ROOT_FOUND)
add_executable(histo histo.cpp)
target_link_libraries(histo tiff ${ROOT_LIBRARIES} boost_system boost_filesystem)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --cflags
    OUTPUT_VARIABLE ROOT_INCLUDE_FLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE)

SET(HISTO_COMPILE_FLAGS "${ROOT_INCLUDE_FLAGS} -I${PROJECT_SOURCE_DIR}/src")
IF(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
SET(HISTO_COMPILE_FLAGS "${BENCH_COMPILE_FLAGS} ${HAS_CPP11_COMPILERFLAG}")
ENDIF()

if(SQY_VERBOSE)
list(APPEND HISTO_COMPILE_FLAGS " -D_SQY_VERBOSE_=1")
endif(SQY_VERBOSE)

STRING(REPLACE ";" " " HISTO_COMPILE_FLAGS ${HISTO_COMPILE_FLAGS})
set_target_properties(histo PROPERTIES COMPILE_FLAGS "${HISTO_COMPILE_FLAGS}")
Message("Building histo, added \n\t${ROOT_LIBRARIES}\n\t${ROOT_INCLUDE_FLAGS}")

ELSE(ROOT_FOUND)
message(">> Skipping optional bench/histo utility, ROOT libraries not found!")
ENDIF(ROOT_FOUND)


ELSE(TIFF_FOUND)
WARNING("Skipping benchmark utils, libtiff not found (but required for build)\!")
ENDIF(TIFF_FOUND)


