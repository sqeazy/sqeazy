INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(verbs)
INCLUDE_DIRECTORIES(encoders)

IF(WIN32)

  #http://stackoverflow.com/a/25148990
  add_definitions(-DBOOST_ALL_NO_LIB)

  
ENDIF()

# FIND_PACKAGE(LZ4 REQUIRED QUIET)
if(LZ4_FOUND)
  
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
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/inc/sqeazy.h;${PROJECT_SOURCE_DIR}/inc/H5PLextern.h")

target_link_libraries(${PROJECT_NAME} ${LZ4_LIBRARY} ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})



if(HAS_BSYMBOLIC_COMPILERFLAG AND UNIX AND NOT APPLE)
  set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-Bsymbolic")
endif()

if(${FFMPEG_USE_STATIC_LIBS})
  set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " ${FFMPEG_EXTRA_LINK_FLAGS}")
endif()


SET(SQY_SRC_TARGETS ${PROJECT_NAME})
INSTALL(TARGETS ${SQY_SRC_TARGETS}
  EXPORT sqeazy-targets 
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" 
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT inc)



########################################### SQY ######################################################
if(NOT READY_4_BUNDLE)
  add_executable(sqy sqy.cpp)

  set_target_properties(sqy PROPERTIES POSITION_INDEPENDENT_CODE ON)
  message("++ [src] sqy deps: ${LZ4_LIBRARY} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES} ${HDF5_LIBRARIES} ${TIFF_LIBRARY}")
  target_link_libraries(sqy ${LZ4_LIBRARY} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES} ${HDF5_LIBRARIES} ${TIFF_LIBRARY})

  INSTALL(TARGETS sqy
    EXPORT sqeazy-targets 
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" 
    COMPONENT bin) 
endif()

########################################### DEPLOYMENT LIBSQY ######################################################
include(${PROJECT_SOURCE_DIR}/cmake/bundle_target.cmake)

set(BUNDLE_DIR ${PROJECT_BINARY_DIR}/bundle CACHE STRING "directory to put sqeazy bundle into")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUNDLE_DIR})

bundle(${PROJECT_NAME} ${BUNDLE_DIR})

# if(${FFMPEG_USE_STATIC_LIBS})
#   set_property(TARGET bundle_${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " ${FFMPEG_EXTRA_LINK_FLAGS}")
# endif()

add_custom_target(bundle DEPENDS bundle_${PROJECT_NAME} )

set_target_properties(bundle PROPERTIES EXCLUDE_FROM_ALL TRUE)
