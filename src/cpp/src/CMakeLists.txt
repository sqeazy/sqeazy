INCLUDE_DIRECTORIES(.)


#SET(HDF5_USE_STATIC_LIBRARIES TRUE)#only works on Unix and requires static lib to be compiled with -fPIC
FIND_PACKAGE(HDF5 1.8 QUIET REQUIRED CXX)

IF(HDF5_FOUND)
  INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
  ADD_DEFINITIONS(${HDF5_DEFINITIONS})
  LINK_DIRECTORIES(${HDF5_LIBRARY_DIRS}) 
ENDIF()


IF(WIN32 OR MSYS)
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
ENDIF()

FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS filesystem system REQUIRED)
IF(Boost_FOUND)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
ENDIF()

#DYNAMIC LIBRARY
add_library(${PROJECT_NAME} SHARED sqeazy.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-I${HDF5_INCLUDE_DIRS}")

IF(LZ4_FOUND)

	IF(EXISTS ${LZ4_STATIC_LIB})
		add_library(ext_lz4 STATIC IMPORTED )
		set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_STATIC_LIB}") 
	ELSE()
		IF(EXISTS ${LZ4_SHARED_LIB})
			add_library(ext_lz4 SHARED IMPORTED )
			set_target_properties(ext_lz4 PROPERTIES IMPORTED_LOCATION "${LZ4_SHARED_LIB}") 
		ENDIF()
	ENDIF()

ENDIF(LZ4_FOUND)

# IF(WIN32)
# STRING(REPLACE dll lib LZ4_SHARED_IMPLIB ${LZ4_SHARED_LIB})
# set_target_properties(ext_lz4 PROPERTIES IMPORTED_IMPLIB "${LZ4_SHARED_LIB}")
# #TODO: #add HDF5_cxx_libraries in a similar fashion
# ENDIF(WIN32)
message("[src/cpp/src] lz4 libs used: (shared) ${LZ4_SHARED_LIB} (static) ${LZ4_STATIC_LIB}")
message("[src/cpp/src] hdf5 libs used: ${HDF5_LIBRARIES} vs ${HDF5_CXX_LIBRARIES}")



#shared libsqy (needed for Java interface)
target_link_libraries(${PROJECT_NAME} ext_lz4 ${HDF5_LIBRARIES} ${Boost_LIBRARIES})
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/inc/sqeazy.h;")
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/inc/sqeazy.h;${PROJECT_SOURCE_DIR}/inc/H5PLextern.h")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_START_STATIC 1)
SET(SQY_SRC_TARGETS ${PROJECT_NAME})

#static libsqy (needed for Java interface)
if(EXISTS ${LZ4_STATIC_LIB})

  add_library(${PROJECT_NAME}_static STATIC sqeazy.cpp)
  set_target_properties(${PROJECT_NAME}_static PROPERTIES COMPILE_FLAGS "-I${HDF5_INCLUDE_DIRS}")
  #SET_TARGET_PROPERTIES(${PROJECT_NAME}_static PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/inc/sqeazy.h;")
  SET_TARGET_PROPERTIES(${PROJECT_NAME}_static PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/inc/sqeazy.h;${PROJECT_SOURCE_DIR}/inc/H5PLextern.h")
  add_library(ext_lz4_static STATIC IMPORTED)
  set_target_properties(ext_lz4_static PROPERTIES IMPORTED_LOCATION "${LZ4_STATIC_LIB}")

  #just here to reset ${Boost_LIBRARIES}
  #REMINDER: it would be nice to use Boost_USE_STATIC_LIBS for the shared lib as well (requires an fPIC enabled installed boost)
  FIND_PACKAGE (Boost 1.42 QUIET COMPONENTS system REQUIRED)
  
  target_link_libraries(${PROJECT_NAME}_static ext_lz4_static ${HDF5_LIBRARIES} ${Boost_LIBRARIES}) 
  set_target_properties(${PROJECT_NAME}_static PROPERTIES LINK_SEARCH_START_STATIC 1)
  IF(UNIX)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
  ENDIF(UNIX)
  SET(SQY_SRC_TARGETS ${SQY_SRC_TARGETS} ${PROJECT_NAME}_static)
  set_target_properties(${PROJECT_NAME}_static PROPERTIES LINK_SEARCH_END_STATIC 1)
else()
  message(WARNING "static lz4 library (${LZ4_STATIC_LIB}) not found, static libsqeazy cannot be built")
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_END_STATIC 1)


INSTALL(TARGETS ${SQY_SRC_TARGETS}
  EXPORT sqeazy-targets 
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" 
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT inc)


