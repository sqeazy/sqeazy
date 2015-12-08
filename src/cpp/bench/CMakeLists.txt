INCLUDE_DIRECTORIES(.)
IF(WIN32)
  SET(Boost_USE_STATIC_LIBS ON)
  add_library(ext_lz4 STATIC IMPORTED)
  set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_STATIC_LIB}") 
ELSE(WIN32)
  add_library(ext_lz4 SHARED IMPORTED)
  set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_SHARED_LIB}") 
ENDIF(WIN32)


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(FFMPEG REQUIRED)
IF(FFMPEG_FOUND)
  INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
  LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)
ENDIF()


FIND_PACKAGE (Boost 1.55 QUIET COMPONENTS regex system filesystem program_options REQUIRED)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()

# IF(TIFF_ROOT)
#   SET(TIFF_DIR ${TIFF_ROOT})
# ENDIF()

FIND_PACKAGE(TIFF REQUIRED)

IF(TIFF_FOUND)
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
  IF(WIN32)
    add_library(ext_static_tiff STATIC IMPORTED)
    IF("${TIFF_LIBRARIES}" MATCHES ".*.dll$")
      STRING(REPLACE .dll .lib TIFF_STATIC_LIB ${TIFF_LIBRARIES})
    ELSE()
      SET(TIFF_STATIC_LIB ${TIFF_LIBRARIES})
    ENDIF()
    set_target_properties(ext_static_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_STATIC_LIB}") 

    add_library(ext_dyn_tiff SHARED IMPORTED)
    IF("${TIFF_LIBRARIES}" MATCHES ".*.dll$")
      STRING(REPLACE .dll _i.lib TIFF_IMP_LIB ${TIFF_LIBRARIES})
      set_target_properties(ext_dyn_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_LIBRARIES}") 
      set_target_properties(ext_dyn_tiff PROPERTIES IMPORTED_IMPLIB "${TIFF_IMP_LIB}") 
    ENDIF()

  ELSE(WIN32)
    add_library(ext_dyn_tiff SHARED IMPORTED)
    set_target_properties(ext_dyn_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_LIBRARIES}") 

  ENDIF(WIN32)


  #TODO:
  add_executable(bench bench.cpp)
  IF(EXISTS ${TIFF_STATIC_LIB})
    target_link_libraries(bench ext_static_tiff ext_lz4 ${Boost_LIBRARIES})
  ELSE()
    target_link_libraries(bench ext_dyn_tiff ext_lz4 ${Boost_LIBRARIES})
  ENDIF()

  IF(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    SET(ADDITIONAL_COMPILE_FLAGS "-I${PROJECT_SOURCE_DIR}/src")
    list(APPEND ADDITIONAL_COMPILE_FLAGS " -I${PROJECT_SOURCE_DIR}/tests")
  ELSE()
    SET(ADDITIONAL_COMPILE_FLAGS "/I${PROJECT_SOURCE_DIR}/src")
    list(APPEND ADDITIONAL_COMPILE_FLAGS " /I${PROJECT_SOURCE_DIR}/tests")
  ENDIF()

  # IF(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  #   IF(HAS_CPP11_COMPILERFLAG)
  #     SET(ADDITIONAL_COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS} -std=c++11")
  #   ENDIF()
  # ENDIF()

  if(SQY_VERBOSE)
    list(APPEND ADDITIONAL_COMPILE_FLAGS " -D_SQY_VERBOSE_=1")
  endif(SQY_VERBOSE)

  STRING(REPLACE ";" " " ADDITIONAL_COMPILE_FLAGS ${ADDITIONAL_COMPILE_FLAGS})
  set_target_properties(bench PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS}")

  # add_executable(sqy sqy.cpp)
  # IF(EXISTS ${TIFF_STATIC_LIB})
  #   target_link_libraries(sqy ext_static_tiff ext_lz4 ${Boost_LIBRARIES})
  # ELSE()
  #   target_link_libraries(sqy ext_dyn_tiff ext_lz4 ${Boost_LIBRARIES})
  # ENDIF()
  # set_target_properties(sqy PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS}")

  # INSTALL(TARGETS sqy
  #   EXPORT sqeazy-targets 
  #   RUNTIME DESTINATION "${INSTALL_BIN_DIR}" 
  #   COMPONENT bin)

  


ELSE(TIFF_FOUND)
  WARNING("Skipping benchmark utils, libtiff not found (but required for build)\!")
ENDIF(TIFF_FOUND)

FILE(COPY ${PROJECT_SOURCE_DIR}/bench/hevc_roundtrip.py DESTINATION ${PROJECT_BINARY_DIR}/bench/)
