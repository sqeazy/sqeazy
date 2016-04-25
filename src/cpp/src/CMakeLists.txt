INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(verbs)
INCLUDE_DIRECTORIES(encoders)

#SET(HDF5_USE_STATIC_LIBRARIES TRUE)#only works on Unix and requires static lib to be compiled with -fPIC
#needed for FindHDF5 as it expects an ENVIRONMENT variable called HDF5_ROOT as hint to the HDF5 path
IF(EXISTS ${HDF5_ROOT})
  SET(ENV{HDF5_ROOT} ${HDF5_ROOT})
ENDIF(EXISTS ${HDF5_ROOT})

FIND_PACKAGE(HDF5 1.8 QUIET REQUIRED CXX)
IF(HDF5_FOUND)
  INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
  ADD_DEFINITIONS(${HDF5_DEFINITIONS})
  LINK_DIRECTORIES(${HDF5_LIBRARY_DIRS}) 
ENDIF()


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(FFMPEG REQUIRED)
IF(FFMPEG_FOUND)
  INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
  LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)

  add_library(ext_avcodec SHARED IMPORTED)
  set_target_properties(ext_avcodec PROPERTIES IMPORTED_LOCATION "${FFMPEG_AVCODEC_LIBRARIES}")
  

  add_library(ext_avutil SHARED IMPORTED)
  set_target_properties(ext_avutil PROPERTIES IMPORTED_LOCATION "${FFMPEG_AVUTIL_LIBRARIES}")

  add_library(ext_avformat SHARED IMPORTED)
  set_target_properties(ext_avformat PROPERTIES IMPORTED_LOCATION "${FFMPEG_AVFORMAT_LIBRARIES}")

  add_library(ext_swscale SHARED IMPORTED)
  set_target_properties(ext_swscale PROPERTIES IMPORTED_LOCATION "${FFMPEG_SWSCALE_LIBRARIES}")

  SET(SQY_FFMPEG_Libraries ext_avcodec ext_avutil ext_avformat ext_swscale m)
ELSE()
  MESSAGE(WARNING "[src/cpp/src] FFMPEG not found!")
ENDIF()

#needed for MPL include path
IF(WIN32)
  SET(Boost_USE_STATIC_LIBS ON)
  SET(Boost_USE_MULTITHREADED ON)
ENDIF()
FIND_PACKAGE (Boost 1.55 QUIET COMPONENTS filesystem system REQUIRED)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ELSE()
  MESSAGE(WARNING "[src/cpp/src] BOOST not found!")
ENDIF()

FIND_PACKAGE(LZ4 REQUIRED)
IF(LZ4_FOUND)
  INCLUDE_DIRECTORIES(${LZ4_INCLUDE_DIRS})
  LINK_DIRECTORIES(${LZ4_LIBRARY_DIRS}) 
  MESSAGE(STATUS "[src/cpp/src] LZ4 found !! shared: ${LZ4_SHARED_LIB} static: ${LZ4_STATIC_LIB}")

  IF(EXISTS ${LZ4_SHARED_LIB})
    add_library(ext_lz4 SHARED IMPORTED )
    set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_SHARED_LIB}")

    IF(WIN32)
      STRING(REPLACE dll lib LZ4_SHARED_IMPLIB ${LZ4_SHARED_LIB})
      set_target_properties(ext_lz4 PROPERTIES IMPORTED_IMPLIB "${LZ4_SHARED_LIB}")
      #TODO: #add HDF5_cxx_libraries in a similar fashion
    ENDIF(WIN32)

  ENDIF()


ENDIF()

########################################### GLOBAL DEFINES ######################################################
#SET(LIST_ADDITIONAL_COMPILE_FLAGS "")
if(SQY_VERBOSE)
  MESSAGE("ADDING -D_SQY_VERBOSE=1")
  ADD_DEFINITIONS(-D_SQY_VERBOSE_=1)
endif(SQY_VERBOSE)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)# OR ${CMAKE_BUILD_TYPE} MATCHES RELWITHDEBINFO
  MESSAGE("ADDING -D_SQY_DEBUG_=1")
  ADD_DEFINITIONS(-D_SQY_DEBUG_=1)
endif()

if(DEFINED LIST_ADDITIONAL_COMPILE_FLAGS )
  STRING(REPLACE ";" " " ADDITIONAL_COMPILE_FLAGS ${LIST_ADDITIONAL_COMPILE_FLAGS})
ENDIF()

########################################### LIBSQY ######################################################

#DYNAMIC LIBRARY
add_library(${PROJECT_NAME} SHARED sqeazy.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-I${HDF5_INCLUDE_DIRS}")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)


target_link_libraries(${PROJECT_NAME} ext_lz4 ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${SQY_FFMPEG_Libraries})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/inc/sqeazy.h;${PROJECT_SOURCE_DIR}/inc/H5PLextern.h")
SET(SQY_SRC_TARGETS ${PROJECT_NAME})


#STATIC LIBRARY
if(EXISTS ${LZ4_STATIC_LIB})
  add_library(${PROJECT_NAME}_static STATIC sqeazy.cpp)
  set_target_properties(${PROJECT_NAME}_static PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
  set_target_properties(${PROJECT_NAME}_static PROPERTIES COMPILE_FLAGS "-I${HDF5_INCLUDE_DIRS}")
  SET_TARGET_PROPERTIES(${PROJECT_NAME}_static PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/inc/sqeazy.h;${PROJECT_SOURCE_DIR}/inc/H5PLextern.h")
  add_library(ext_lz4_static STATIC IMPORTED)
  set_target_properties(ext_lz4_static PROPERTIES IMPORTED_LOCATION "${LZ4_STATIC_LIB}")
  MESSAGE(STATUS "defining ext_lz4_static as ${LIB_STATIC_LIB}")

  #just here to reset ${Boost_LIBRARIES}
  #REMINDER: it would be nice to use Boost_USE_STATIC_LIBS for the shared lib as well (requires an fPIC enabled installed boost)
  set(Boost_USE_STATIC_LIBS ON)
  FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system REQUIRED)
  
  target_link_libraries(${PROJECT_NAME}_static ext_lz4_static ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${SQY_FFMPEG_Libraries}) 
  IF(UNIX)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
  ENDIF(UNIX)
  SET(SQY_SRC_TARGETS ${SQY_SRC_TARGETS} ${PROJECT_NAME}_static)
else()
  message(WARNING "static lz4 library not found, static libsqeazy cannot be built")
endif()



INSTALL(TARGETS ${SQY_SRC_TARGETS}
  EXPORT sqeazy-targets 
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" 
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT inc)


########################################### SQY ######################################################
FIND_PACKAGE (Boost 1.55 QUIET COMPONENTS regex system filesystem program_options REQUIRED)
#TODO: is this needed?
IF(Boost_FOUND)
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(FFMPEG REQUIRED)
IF(FFMPEG_FOUND)
  INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
  LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)
ENDIF()

FIND_PACKAGE(TIFF REQUIRED)
IF(TIFF_FOUND)
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
  SET(TIFF_DYN_LIB ${TIFF_LIBRARIES})
  SET(TIFF_STATIC_LIB "")
  
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


    IF(APPLE)
      IF("${TIFF_LIBRARIES}" MATCHES ".*.so$")
	STRING(REPLACE .so .dylib TIFF_DYN_LIB ${TIFF_LIBRARIES})
	#Homebrew ships a static library that fails to contain important symbols
	#STRING(REPLACE .dylib .a TIFF_STATIC_LIB ${TIFF_DYN_LIB})
      ENDIF()
    ELSE()
      STRING(REPLACE .so .a TIFF_STATIC_LIB ${TIFF_DYN_LIB})
    ENDIF()

    IF(EXISTS ${TIFF_DYN_LIB})
      add_library(ext_dyn_tiff SHARED IMPORTED)
      set_target_properties(ext_dyn_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_DYN_LIB}")
    ENDIF()

    #FIXME: static includes of libtiff showed missing symbols (mostly deflate, there must be a depenency to libz or something which maybe not included in libtiff.a)
    IF(EXISTS ${TIFF_STATIC_LIB})
      add_library(ext_static_tiff STATIC IMPORTED)
      set_target_properties(ext_static_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_STATIC_LIB}")
    ENDIF()
    
  ENDIF(WIN32)


  add_executable(sqy sqy.cpp)
  target_link_libraries(sqy ext_dyn_tiff ext_lz4 ${Boost_LIBRARIES} ${SQY_FFMPEG_Libraries} ${HDF5_LIBRARIES})


  INSTALL(TARGETS sqy
    EXPORT sqeazy-targets 
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" 
    COMPONENT bin) 

ELSE()
  WARNING("[src/cpp/src] Skipping sqy app, libtiff not found (but required for build)\!")
ENDIF()

