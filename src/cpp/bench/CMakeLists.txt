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

if(EXISTS ${BENCHMARK_ROOT})
  set(benchmark_ROOT_DIR ${BENCHMARK_ROOT})
else(EXISTS ${BENCHMARK_ROOT})
  if(EXISTS $ENV{BENCHMARK_ROOT})
	set(benchmark_ROOT_DIR $ENV{BENCHMARK_ROOT})
  endif()
endif(EXISTS ${BENCHMARK_ROOT})

find_package(GoogleBenchmark REQUIRED)
if(NOT ${benchmark_FOUND})
  message(FATAL_ERROR "GoogleBenchmark not found, ${benchmark_ROOT_DIR}")
else()
  include_directories(${benchmark_INCLUDE_DIRS})
  message(">> GoogleBenchmark found, ${benchmark_INCLUDE_DIRS} ${benchmark_LIBRARIES}")
endif()

# message(STATUS "[bench] linking against ${SQY_LIB} ${Boost_LIBRARIES} ")

add_executable(test_benchmark_fixture_impl test_benchmark_fixture_impl.cpp)
target_link_libraries(test_benchmark_fixture_impl ${Boost_LIBRARIES} ${benchmark_LIBRARIES})

# if(EXISTS ${LZ4_STATIC_LIB})
#   #if static libsqeazy exists, it will contain lz4, no need to link against it yet another time
#   if(NOT DEFINED READY_4_BUNDLE)
#     target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${Boost_LIBRARIES})
#     target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES})
#     target_link_libraries(test_pipeline_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES})
#   else()
#     target_link_libraries(test_lz4_encoding ${PROJECT_NAME}_static ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
#     target_link_libraries(test_hdf5_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
#     target_link_libraries(test_pipeline_interface ${PROJECT_NAME}_static ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
#   endif()
# else()
#   target_link_libraries(test_lz4_encoding ${SQY_LIB} ${Boost_LIBRARIES} ${LZ4_LIBRARY})
#   target_link_libraries(test_hdf5_interface ${SQY_LIB} ${Boost_LIBRARIES} ${LZ4_LIBRARY} ${HDF5_LIBRARIES})
#   target_link_libraries(test_pipeline_interface ${SQY_LIB} ${Boost_LIBRARIES} ${LZ4_LIBRARY})
# endif()


# add_executable(test_diff_schemes test_diff_schemes.cpp)
# target_link_libraries(test_diff_schemes ${SQY_LIB} ${Boost_LIBRARIES})


# IF(TIFF_FOUND)

# # add_executable(test_tiff_fixture test_tiff_fixture.cpp)
# # target_link_libraries(test_tiff_fixture ${Boost_LIBRARIES} ${TIFF_LIBRARY})
# # set_target_properties(test_tiff_fixture PROPERTIES COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/bench")


# IF(FFMPEG_FOUND)
#   if(WIN32)
# 	LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}\\lib)
# 	else()
# 	LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)
#     endif()

#   SET(SQY_FFMPEG_Libraries ${FFMPEG_LIBRARIES})

#   if(DEFINED FFMPEG_EXTRA_LINK_FLAGS AND DEFINED FFMPEG_USE_STATIC_LIBS)
# 	set(SQY_FFMPEG_Libraries ${SQY_FFMPEG_Libraries} ${FFMPEG_EXTRA_LINK_FLAGS})
#   endif()

#   add_executable(test_avcodec_sandbox test_avcodec_sandbox.cpp)
#   target_link_libraries(test_avcodec_sandbox ${Boost_LIBRARIES} ${TIFF_LIBRARY} ${SQY_FFMPEG_Libraries})

#   add_executable(deprecated_pipeline_select_impl deprecated_pipeline_select_impl.cpp)
#   target_link_libraries(deprecated_pipeline_select_impl ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${TIFF_LIBRARY} ${LZ4_LIBRARY} ${SQY_FFMPEG_Libraries})

# ELSE(FFMPEG_FOUND)
#   MESSAGE(">> [tests] libavcodec not found. skipping test_avcodec_sandbox ...")
# ENDIF(FFMPEG_FOUND)

# ELSE(FFMPEG_FOUND)
#   MESSAGE(">> [tests] libtiff not found. skipping related tests ...")
# ENDIF(FFMPEG_FOUND)

# ENDIF(TIFF_FOUND)
