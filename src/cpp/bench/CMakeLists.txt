INCLUDE_DIRECTORIES(.)
IF(WIN32)
  SET(Boost_USE_STATIC_LIBS ON)
  add_library(ext_lz4 STATIC IMPORTED)
  set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_STATIC_LIB}") 
ELSE(WIN32)
  add_library(ext_lz4 SHARED IMPORTED)
  set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_SHARED_LIB}") 
ENDIF(WIN32)





FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system filesystem program_options REQUIRED)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()

IF(TIFF_ROOT)
  SET(TIFF_DIR ${TIFF_ROOT})
ENDIF()

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

    IF(EXISTS ${TIFF_STATIC_LIB})
      MESSAGE("[BENCH] using static ${TIFF_STATIC_LIB} for bench")
    ELSE()
      MESSAGE("[BENCH] using dynamic ${TIFF_LIBRARIES} for bench")
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
  ELSE()
    SET(ADDITIONAL_COMPILE_FLAGS "/I${PROJECT_SOURCE_DIR}/src")
  ENDIF()

  IF(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    IF(HAS_CPP11_COMPILERFLAG)
      SET(ADDITIONAL_COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS} -std=c++11")
    ENDIF()
  ENDIF()

  if(SQY_VERBOSE)
    list(APPEND ADDITIONAL_COMPILE_FLAGS " -D_SQY_VERBOSE_=1")
  endif(SQY_VERBOSE)

  STRING(REPLACE ";" " " ADDITIONAL_COMPILE_FLAGS ${ADDITIONAL_COMPILE_FLAGS})
  MESSAGE(STATUS ">> adding compile flag to bench: ${ADDITIONAL_COMPILE_FLAGS}")
  set_target_properties(bench PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS}")

  add_executable(sqy sqy.cpp)
  IF(EXISTS ${TIFF_STATIC_LIB})
    target_link_libraries(sqy ext_static_tiff ext_lz4 ${Boost_LIBRARIES})
  ELSE()
    target_link_libraries(sqy ext_dyn_tiff ext_lz4 ${Boost_LIBRARIES})
  ENDIF()
  MESSAGE(STATUS ">> adding compile flag to sqy: ${ADDITIONAL_COMPILE_FLAGS}")
  set_target_properties(sqy PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS}")

  INSTALL(TARGETS sqy
    EXPORT sqeazy-targets 
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" 
    COMPONENT bin)

  FIND_PACKAGE(ROOT QUIET)
  IF(ROOT_FOUND)
    add_executable(histo histo.cpp)
    IF(EXISTS ${TIFF_STATIC_LIB})
      target_link_libraries(histo ext_static_tiff ${ROOT_LIBRARIES} ${Boost_LIBRARIES})
    ELSE()
      target_link_libraries(histo ext_dyn_tiff ${ROOT_LIBRARIES} ${Boost_LIBRARIES})
    ENDIF()

    execute_process(
      COMMAND ${ROOT_CONFIG_EXECUTABLE} --cflags
      OUTPUT_VARIABLE ROOT_INCLUDE_FLAGS
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    
    SET(HISTO_COMPILE_FLAGS "${ROOT_INCLUDE_FLAGS} ${ADDITIONAL_COMPILE_FLAGS}")
    MESSAGE(STATUS ">> adding compile flag to histo: ${HISTO_COMPILE_FLAGS}")
    set_target_properties(histo PROPERTIES COMPILE_FLAGS "${HISTO_COMPILE_FLAGS}")

  ELSE(ROOT_FOUND)
    MESSAGE(STATUS ">> Skipping optional bench/histo utility, ROOT libraries not found!")
  ENDIF(ROOT_FOUND)


ELSE(TIFF_FOUND)
  WARNING("Skipping benchmark utils, libtiff not found (but required for build)\!")
ENDIF(TIFF_FOUND)


