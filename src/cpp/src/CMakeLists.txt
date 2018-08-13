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
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PROJECT_BINARY_DIR}/inc/sqeazy.h;${PROJECT_BINARY_DIR}/inc/H5PLextern.h;${PROJECT_BINARY_DIR}/inc/sqeazy_h5_filter.hpp;${PROJECT_BINARY_DIR}/inc/sqeazy_definitions.h")

set(LIBS2LINK ${LZ4_LIBRARY} ${HDF5_LIBRARIES} ${Boost_LIBRARIES})
if(WITH_FFMPEG)
  set(LIBS2LINK ${LIBS2LINK} ${FFMPEG_LIBRARIES})
endif()

if(NOT WIN32)
  set(LIBS2LINK ${LIBS2LINK} ${OpenMP++_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} ${LIBS2LINK})
message("++ [src] libsqeazy deps: ${LIBS2LINK}")

if(HAS_BSYMBOLIC_COMPILERFLAG AND UNIX AND NOT APPLE)
  set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-Bsymbolic")
endif()

if(${WITH_FFMPEG} EQUAL (DEFINED ${FFMPEG_USE_STATIC_LIBS}))
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


set_target_properties(sqy PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES)
set_target_properties(sqy PROPERTIES POSITION_INDEPENDENT_CODE ON)
set(LIBS2LINKAPP ${LIBS2LINK} ${TIFF_LIB_FOR_SQY})

target_link_libraries(sqy ${LIBS2LINKAPP})

if(HAS_BSYMBOLIC_COMPILERFLAG AND UNIX AND NOT APPLE)
  set_property(TARGET sqy APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-Bsymbolic")
endif()

if(${WITH_FFMPEG} EQUAL (DEFINED ${FFMPEG_USE_STATIC_LIBS}))
  set_property(TARGET sqy APPEND_STRING PROPERTY LINK_FLAGS " ${FFMPEG_EXTRA_LINK_FLAGS}")
endif()

  INSTALL(TARGETS sqy
    EXPORT sqeazy-targets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
    COMPONENT bin)
#endif()

########################################### DEPLOYMENT LIBSQY ######################################################
if(BUNDLE_SQEAZY OR ${BUNDLE_SQEAZY})

  include(${PROJECT_SOURCE_DIR}/cmake/bundle_target.cmake)
  #/usr/local/bin/c++  -DNDEBUG -D_GLIBCXX_USE_CXX11_ABI=1 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE -D_SQY_X_=0 -Dbundle_sqeazy_EXPORTS -I/build/sqeazy/build/cpp/inc -I/build/sqeazy/src/cpp/src/. -I/build/sqeazy/src/cpp/src/verbs -I/build/sqeazy/src/cpp/src/encoders  -DNDEBUG -Ofast -fPIC   -Wall -Wfatal-errors -Wno-unused-function -Wno-overflow -Ofast -ftree-vectorize -march=x86-64 -mtune=generic -msse -msse2 -msse3 -mssse3 -msse4 -msse4.1 -msse4.2 -ffast-math -fopenmp -fopenmp -static-libgcc -static-libstdc++ -std=gnu++14 -o CMakeFiles/bundle_sqeazy.dir/sqeazy.cpp.o -c /build/sqeazy/src/cpp/src/sqeazy.cpp
  #/usr/local/bin/c++ -fPIC -DNDEBUG -Ofast -Wl,-Bsymbolic -shared -Wl,-soname,libsqeazy.so -o ../bundle/libsqeazy.so CMakeFiles/bundle_sqeazy.dir/sqeazy.cpp.o -Wl,-rpath,./ -Wl,-Bstatic -llz4 -lhdf5_cpp -lhdf5 -Wl,-Bdynamic -ldl -lm -Wl,-Bstatic -lboost_program_options -lboost_regex -lboost_filesystem -lboost_system -lboost_serialization -lgomp -Wl,-Bdynamic -lz -lpthread
  set(BUNDLE_DIR ${PROJECT_BINARY_DIR}/bundle CACHE STRING "directory to put sqeazy bundle into")
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUNDLE_DIR})

  bundle(${PROJECT_NAME} ${BUNDLE_DIR})
  bundle(sqy ${BUNDLE_DIR})

    #TODO: checkout this link https://stackoverflow.com/questions/16991225/cmake-and-static-linking
  set(STATIC_LIB_FLAGS "LIBGCC;LIBSTDC++;LIBC++")
  foreach(_flag IN LISTS STATIC_LIB_FLAGS)
    string(TOLOWER "-static-${_flag}" _lower_flag)
    if(HAS_STATIC_${_flag}_COMPILERFLAG)
      if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
        set_property(TARGET bundle_${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "${_lower_flag}" )
        set_property(TARGET bundle_${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS "${_lower_flag}" )

        set_property(TARGET bundle_sqy APPEND_STRING PROPERTY LINK_FLAGS "${_lower_flag}" )
        set_property(TARGET bundle_sqy APPEND_STRING PROPERTY COMPILE_FLAGS "${_lower_flag}" )
      endif()
    endif()
  endforeach()

  add_custom_target(bundle DEPENDS bundle_${PROJECT_NAME} bundle_sqy)
  set_target_properties(bundle PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
