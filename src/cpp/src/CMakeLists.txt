INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(verbs)
INCLUDE_DIRECTORIES(encoders)

#SET(HDF5_USE_STATIC_LIBRARIES TRUE)#only works on Unix and requires static lib to be compiled with -fPIC
#needed for FindHDF5 as it expects an ENVIRONMENT variable called HDF5_ROOT as hint to the HDF5 path
IF(EXISTS ${HDF5_ROOT})
  SET(ENV{HDF5_ROOT} ${HDF5_ROOT})
ENDIF(EXISTS ${HDF5_ROOT})



# FIND_PACKAGE(HDF5 1.8 QUIET REQUIRED CXX HL)
# IF(HDF5_FOUND)
#   INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
#   #only required for hdf5 1.8.16 and newer
#   #http://hdf-forum.184993.n3.nabble.com/Re-linking-HDF5-with-VS2015-C-64-bit-only-works-with-static-libraries-td4028678.html
  
#   if(WIN32)
#   add_definitions(/DH5_BUILT_AS_DYNAMIC_LIB=1)
#   else()
#   add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB=1)
#   endif()
  
#   #to discard compiler warnings in Debug mode
#   IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
#     SET(FORTIFY_INDEX "")
#     list(FIND HDF5_DEFINITIONS "-D_FORTIFY_SOURCE=2" FORTIFY_INDEX)
    
#     IF(${FORTIFY_INDEX} GREATER -1)
#       list(REMOVE_AT HDF5_DEFINITIONS ${FORTIFY_INDEX})
#       list(APPEND HDF5_DEFINITIONS -D_FORTIFY_SOURCE=0)
#     ENDIF()
#   ENDIF()
  
#   ADD_DEFINITIONS(${HDF5_DEFINITIONS})
#   IF(DEFINED HDF5_LIBRARY_DIRS)
# 	LINK_DIRECTORIES(${HDF5_LIBRARY_DIRS}) 
#   ENDIF()
#   IF(WIN32)
# 	ADD_DEFINITIONS(-DH5_BUILT_AS_DYNAMIC_LIB)
# 	MESSAGE("[src/cpp/src] -I${HDF5_INCLUDE_DIRS} -L${HDF5_LIBRARY_DIRS} : ${HDF5_LIBRARIES}")
#   ENDIF(WIN32)
# ELSE()
# MESSAGE(WARNING "[src/cpp/src] HDF5 not found!")
# ENDIF()

# include(CheckFunctionExists)
# CHECK_FUNCTION_EXISTS(pow EXTRA_LIBM_NOT_NEEDED)

# find_package(FFMPEG REQUIRED QUIET)
# IF(FFMPEG_FOUND)
# 	INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
# 	LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)

# 	# add_library(ext_avcodec SHARED IMPORTED)
# 	# add_library(ext_avutil SHARED IMPORTED)
# 	# add_library(ext_avformat SHARED IMPORTED)
# 	# add_library(ext_swscale SHARED IMPORTED)	
	
# 	# IF(WIN32) 
# 		# set_target_properties(ext_avcodec  PROPERTIES IMPORTED_IMPLIB "${FFMPEG_AVCODEC_LIBRARY}")
# 		# set_target_properties(ext_avutil   PROPERTIES IMPORTED_IMPLIB "${FFMPEG_AVUTIL_LIBRARY}")
# 		# set_target_properties(ext_avformat PROPERTIES IMPORTED_IMPLIB "${FFMPEG_AVFORMAT_LIBRARY}")
# 		# set_target_properties(ext_swscale PROPERTIES IMPORTED_IMPLIB "${FFMPEG_SWSCALE_LIBRARY}")
# 		# SET(SQY_FFMPEG_LIBRARY ext_avcodec ext_avutil ext_avformat ext_swscale)
# 	# ELSE(WIN32)
# 		# set_target_properties(ext_avcodec  PROPERTIES IMPORTED_LOCATION "${FFMPEG_AVCODEC_LIBRARY}")
# 		# set_target_properties(ext_avutil   PROPERTIES IMPORTED_LOCATION "${FFMPEG_AVUTIL_LIBRARY}")
# 		# set_target_properties(ext_avformat PROPERTIES IMPORTED_LOCATION "${FFMPEG_AVFORMAT_LIBRARY}")
# 		# set_target_properties(ext_swscale PROPERTIES IMPORTED_LOCATION "${FFMPEG_SWSCALE_LIBRARY}")
# 		# SET(SQY_FFMPEG_LIBRARY ext_avcodec ext_avutil ext_avformat ext_swscale m)
# 	# ENDIF(WIN32)

#   SET(SQY_FFMPEG_Libraries ${FFMPEG_AVCODEC_LIBRARY} ${FFMPEG_AVUTIL_LIBRARY} ${FFMPEG_AVFORMAT_LIBRARY} ${FFMPEG_SWSCALE_LIBRARY})
#   if(NOT EXTRA_LIBM_NOT_NEEDED)
# 	set(SQY_FFMPEG_Libraries ${SQY_FFMPEG_Libraries} m)
#   endif()
  
# ELSE()
#   MESSAGE(WARNING "[src/cpp/src] FFMPEG not found!")
# ENDIF()

#needed for MPL include path
IF(WIN32)

#http://stackoverflow.com/a/25148990
add_definitions(-DBOOST_ALL_NO_LIB)

# set(Boost_USE_STATIC_LIBS        ON)
# set(Boost_USE_MULTITHREADED      ON)
# set(Boost_USE_STATIC_RUNTIME    OFF)

    
ENDIF()

# FIND_PACKAGE (Boost 1.55 QUIET COMPONENTS filesystem system REQUIRED)
# IF(Boost_FOUND)
#   INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
# ELSE()
#   message(WARNING "-- [src/cpp/src] BOOST not found!")
# ENDIF()

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
if(SQY_VERBOSE)
  ADD_DEFINITIONS(-D_SQY_VERBOSE_=1)
endif(SQY_VERBOSE)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)# OR ${CMAKE_BUILD_TYPE} MATCHES RELWITHDEBINFO
  ADD_DEFINITIONS(-D_SQY_DEBUG_=1)
endif()

if(DEFINED LIST_ADDITIONAL_COMPILE_FLAGS )
  STRING(REPLACE ";" " " ADDITIONAL_COMPILE_FLAGS ${LIST_ADDITIONAL_COMPILE_FLAGS})
ENDIF()

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
# FIND_PACKAGE (Boost 1.55 QUIET COMPONENTS regex system filesystem program_options REQUIRED)
# #TODO: is this needed?
# IF(Boost_FOUND)
#   LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}) 
# ENDIF()

# set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
# find_package(FFMPEG REQUIRED QUIET)
# IF(FFMPEG_FOUND)
#   INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
#   LINK_DIRECTORIES(${FFMPEG_ROOT_DIR}/lib)
# ENDIF()

# if(DEFINED TIFF_ROOT)
# 	if(EXISTS ${TIFF_ROOT})
# 	list(APPEND CMAKE_PREFIX_PATH ${TIFF_ROOT})
# 	else()
# 	message(FATAL_ERROR "provided TIFF_ROOT ${TIFF_ROOT} does not exist")
# 	endif()
# endif()

# FIND_PACKAGE(TIFF REQUIRED QUIET)
# IF(TIFF_FOUND)
#   include_directories(${TIFF_INCLUDE_DIR})
#   get_filename_component(TIFF_LIB_PATH ${TIFF_LIBRARY} ABSOLUTE)
#   link_directories(${TIFF_LIB_PATH})
    
# 	SET(TIFF_DYN_LIB ${TIFF_LIBRARY})
# 	SET(TIFF_STATIC_LIB "")
  
#   IF(WIN32)
      
#     message("++ using ${TIFF_INCLUDE_DIR} ${TIFF_LIBRARY}")
	
#   ELSE(WIN32)


#     IF(APPLE)
#       IF("${TIFF_LIBRARY}" MATCHES ".*.so$")
# 	STRING(REPLACE .so .dylib TIFF_DYN_LIB ${TIFF_LIBRARY})
# 	#Homebrew ships a static library that fails to contain important symbols
# 	#STRING(REPLACE .dylib .a TIFF_STATIC_LIB ${TIFF_DYN_LIB})
#       ENDIF()
#     ELSE()
#       STRING(REPLACE .so .a TIFF_STATIC_LIB ${TIFF_DYN_LIB})
#     ENDIF()

#     IF(EXISTS ${TIFF_DYN_LIB})
#       add_library(ext_tiff SHARED IMPORTED)
#       set_target_properties(ext_tiff PROPERTIES IMPORTED_LOCATION "${TIFF_DYN_LIB}")
#     ENDIF()

#     message("++ using ${TIFF_INCLUDE_DIR} ${TIFF_DYN_LIB}")
#   ENDIF(WIN32)

  add_executable(sqy sqy.cpp)
  
  target_link_libraries(sqy ${LZ4_LIBRARY} ${Boost_LIBRARIES} ${SQY_FFMPEG_Libraries} ${HDF5_LIBRARIES} ${TIFF_LIBRARY})
  
  INSTALL(TARGETS sqy
    EXPORT sqeazy-targets 
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" 
    COMPONENT bin) 

# ELSE()
#   WARNING("[src/cpp/src] Skipping sqy app, libtiff not found (but required for build)\!")
# ENDIF()




########################################### DEPLOYMENT LIBSQY ######################################################
include(${PROJECT_SOURCE_DIR}/cmake/bundle_target.cmake)

set(BUNDLE_DIR ${PROJECT_BINARY_DIR}/bundle CACHE STRING "directory to put sqeazy bundle into")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUNDLE_DIR})

bundle(${PROJECT_NAME} ${BUNDLE_DIR})

add_custom_target(bundle DEPENDS bundle_${PROJECT_NAME} )
set_target_properties(bundle PROPERTIES EXCLUDE_FROM_ALL TRUE)
