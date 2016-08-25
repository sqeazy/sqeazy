INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(verbs)
INCLUDE_DIRECTORIES(encoders)

IF(WIN32)

  #http://stackoverflow.com/a/25148990
  add_definitions(-DBOOST_ALL_NO_LIB)

  
ENDIF()

# FIND_PACKAGE(LZ4 REQUIRED QUIET)
if(LZ4_FOUND)
  # INCLUDE_DIRECTORIES(${LZ4_INCLUDE_DIRS})
  # LINK_DIRECTORIES(${LZ4_LIBRARY_DIRS}) 
  
	IF(WIN32)
		IF(EXISTS ${LZ4_STATIC_LIB})
			add_library(ext_lz4 STATIC IMPORTED )
			set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_STATIC_LIB}")
		ELSE()
		IF(EXISTS ${LZ4_SHARED_LIB})
			add_library(ext_lz4 SHARED IMPORTED )
			set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_SHARED_LIB}")
			STRING(REPLACE dll lib LZ4_SHARED_IMPLIB ${LZ4_SHARED_LIB})
			set_target_properties(ext_lz4 PROPERTIES IMPORTED_IMPLIB "${LZ4_SHARED_IMPLIB}")
		#TODO: #add HDF5_cxx_libraries in a similar fashion
		ENDIF(EXISTS ${LZ4_SHARED_LIB})
		ENDIF()
	ELSE(WIN32)
		IF(EXISTS ${LZ4_STATIC_LIB})
			add_library(ext_lz4 STATIC IMPORTED )
			set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_SHARED_LIB}")
		ELSE()
			IF(EXISTS ${LZ4_SHARED_LIB})
				add_library(ext_lz4 SHARED IMPORTED )
				set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_SHARED_LIB}")
			ENDIF()
		ENDIF()
	ENDIF(WIN32)

else(LZ4_FOUND)

message(WARNING "-- [src/cpp/src] lz4 not found!")

endif(LZ4_FOUND)

########################################### GLOBAL DEFINES ######################################################
#SET(LIST_ADDITIONAL_COMPILE_FLAGS "")


# if(DEFINED LIST_ADDITIONAL_COMPILE_FLAGS )
#   STRING(REPLACE ";" " " ADDITIONAL_COMPILE_FLAGS ${LIST_ADDITIONAL_COMPILE_FLAGS})
# ENDIF()

########################################### LIBSQY ######################################################

#DYNAMIC LIBRARY
add_library(${PROJECT_NAME} SHARED sqeazy.cpp)
#set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-I${HDF5_INCLUDE_DIRS}")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/inc/sqeazy.h;${PROJECT_SOURCE_DIR}/inc/H5PLextern.h")

target_link_libraries(${PROJECT_NAME} ${LZ4_LIBRARY} ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${SQY_FFMPEG_Libraries})

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
  #set(Boost_USE_STATIC_LIBS ON)
  #FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system REQUIRED)
  
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

  add_executable(sqy sqy.cpp)
  
  target_link_libraries(sqy ${LZ4_LIBRARY} ${Boost_LIBRARIES} ${SQY_FFMPEG_Libraries} ${HDF5_LIBRARIES} ${TIFF_LIBRARY})
  
  INSTALL(TARGETS sqy
    EXPORT sqeazy-targets 
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" 
    COMPONENT bin) 


########################################### DEPLOYMENT LIBSQY ######################################################
include(${PROJECT_SOURCE_DIR}/cmake/bundle_target.cmake)

set(BUNDLE_DIR ${PROJECT_BINARY_DIR}/bundle CACHE STRING "directory to put sqeazy bundle into")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUNDLE_DIR})

bundle(${PROJECT_NAME} ${BUNDLE_DIR})

add_custom_target(bundle DEPENDS bundle_${PROJECT_NAME} )
set_target_properties(bundle PROPERTIES EXCLUDE_FROM_ALL TRUE)
