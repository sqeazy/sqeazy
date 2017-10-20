INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(verbs)
INCLUDE_DIRECTORIES(encoders)

IF(WIN32)
  #http://stackoverflow.com/a/25148990
  add_definitions(-DBOOST_ALL_NO_LIB)
ENDIF()

#DYNAMIC LIBRARY
SET(SQY_SRC_TARGETS ${PROJECT_NAME})
add_library(${PROJECT_NAME} SHARED sqeazy.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PROJECT_BINARY_DIR}/inc/sqeazy.h;${PROJECT_BINARY_DIR}/inc/H5PLextern.h;${PROJECT_BINARY_DIR}/inc/sqeazy_h5_filter.hpp;${PROJECT_BINARY_DIR}/inc/sqeazy_definitions.h")


if(NOT WIN32)
	target_link_libraries(${PROJECT_NAME} ${LZ4_LIBRARY} ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES} ${OpenMP++_LIBRARIES})
else()
	target_link_libraries(${PROJECT_NAME} ${LZ4_LIBRARY} ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES})
endif()
message("++ [src] libsqeazy deps: ${LZ4_LIBRARY} ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES} ${OpenMP++_LIBRARIES}")

if(HAS_BSYMBOLIC_COMPILERFLAG AND UNIX AND NOT APPLE)
  set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-Bsymbolic")
endif()

if(${FFMPEG_USE_STATIC_LIBS})
  set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " ${FFMPEG_EXTRA_LINK_FLAGS}")
endif()

INSTALL(TARGETS ${SQY_SRC_TARGETS}
  EXPORT sqeazy-targets
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT inc)



########################################### SQY ######################################################
#if(NOT BUNDLE_SQEAZY OR NOT ${BUNDLE_SQEAZY})
add_executable(sqy sqy.cpp)


set_target_properties(sqy PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
set_target_properties(sqy PROPERTIES POSITION_INDEPENDENT_CODE ON)
message("++ [src] sqy deps: ${LZ4_LIBRARY} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES} ${HDF5_LIBRARIES} ${TIFF_LIB_FOR_SQY} ${OpenMP++_LIBRARIES}")
if(NOT WIN32)
	target_link_libraries(sqy ${LZ4_LIBRARY} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES} ${HDF5_LIBRARIES} ${TIFF_LIB_FOR_SQY} ${OpenMP++_LIBRARIES})
else()
	target_link_libraries(sqy ${LZ4_LIBRARY} ${Boost_LIBRARIES} ${FFMPEG_LIBRARIES} ${HDF5_LIBRARIES} ${TIFF_LIB_FOR_SQY})
endif()

if(HAS_BSYMBOLIC_COMPILERFLAG AND UNIX AND NOT APPLE)
  set_property(TARGET sqy APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-Bsymbolic")
endif()

if(${FFMPEG_USE_STATIC_LIBS})
  set_property(TARGET sqy APPEND_STRING PROPERTY LINK_FLAGS " ${FFMPEG_EXTRA_LINK_FLAGS}")
endif()

  INSTALL(TARGETS sqy
    EXPORT sqeazy-targets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
    COMPONENT bin)
#endif()

########################################### DEPLOYMENT LIBSQY ######################################################
if(BUNDLE_SQEAZY OR ${BUNDLE_SQEAZY})

  #TODO: checkout this link https://stackoverflow.com/questions/16991225/cmake-and-static-linking
  set(STATIC_LIB_FLAGS "LIBGCC;LIBSTDC++;LIBC++")
  foreach(_flag IN LISTS STATIC_LIB_FLAGS)
    string(TOLOWER "-static-${_flag}" _lower_flag)
    if(HAS_STATIC_${_flag}_COMPILERFLAG)
      if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${_lower_flag}")
      endif()
      add_definitions(${_lower_flag})
    endif()
  endforeach()

  include(${PROJECT_SOURCE_DIR}/cmake/bundle_target.cmake)

  set(BUNDLE_DIR ${PROJECT_BINARY_DIR}/bundle CACHE STRING "directory to put sqeazy bundle into")
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUNDLE_DIR})

  bundle(${PROJECT_NAME} ${BUNDLE_DIR})
  bundle(sqy ${BUNDLE_DIR})

  add_custom_target(bundle DEPENDS bundle_${PROJECT_NAME} bundle_sqy)

  set_target_properties(bundle PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
