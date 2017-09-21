# doc:
function(COPY_IN copydest files)

  set(flist ${ARGV})
  list(REMOVE_ITEM flist ${copydest})

  #message("++ [COPY_IN] received target dir ${copydest} and ${flist}")


  foreach(_ITEM IN LISTS flist)

    get_filename_component(FNAME ${_ITEM} NAME)
    get_filename_component(RDIR ${_ITEM} REALPATH)
    get_filename_component(RDIRFNAME ${RDIR} NAME)

    #message("++ [COPY_IN] ${_ITEM} :\n ${RDIR} -> ${destdir}/${FNAME}")
    file(COPY ${RDIR} DESTINATION ${destdir})
    file(RENAME ${destdir}/${RDIRFNAME} ${destdir}/${FNAME})

    unset(FNAME CACHE)
    unset(RDIR CACHE)
    unset(RDIRFNAME CACHE)
  endforeach()

endfunction(COPY_IN)

# doc:
function(COPY_IN_FORTARGET copydest _tgt files)


  set(flist ${ARGV})
  list(REMOVE_ITEM flist ${copydest} ${_tgt})

  if(NOT TARGET bundle_copy_${_tgt})
    add_custom_target(bundle_copy_${_tgt})
  endif()


  if(UNIX)
    foreach(_ITEM IN LISTS flist)

      file(GLOB GLOBRESULT "${_ITEM}.*")
      list(APPEND flist ${GLOBRESULT})
    endforeach()
  endif()

  foreach(_ITEM IN LISTS flist)

    get_filename_component(FNAME ${_ITEM} NAME)
    get_filename_component(FNAME_EXT ${_ITEM} EXT)

    if(NOT ${FNAME_EXT} STREQUAL ${CMAKE_STATIC_LIBRARY_SUFFIX})
      get_filename_component(RDIR ${_ITEM} REALPATH)
      get_filename_component(RDIRFNAME ${RDIR} NAME)

      #message("++ [BUNDLE::COPY_IN_FORTARGET] ${_ITEM} :\n ${RDIR} -> ${destdir}/${FNAME}")
      add_custom_command(TARGET bundle_copy_${_tgt} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy ${RDIR} ${destdir}/${FNAME})

      unset(RDIR CACHE)
      unset(RDIRFNAME CACHE)
    endif()
    unset(FNAME CACHE)

  endforeach()

endfunction(COPY_IN_FORTARGET)

# doc:
function(BUNDLE tgt destdir)

  find_package(PkgConfig QUIET)

  if(NOT TARGET ${tgt})
    message("++ [BUNDLE] There is no target named '${tgt}'")
    return()
  else()
    message("++ [BUNDLE] bundling '${tgt}' to ${destdir}")
  endif()

  ## introspect target
  get_property(TGT_LIBS TARGET ${tgt} PROPERTY LINK_LIBRARIES)
  get_property(TGT_LDFLAGS TARGET ${tgt} PROPERTY LINK_FLAGS)
  message("++ [BUNDLE] target yields LINK_FLAGS = ${TGT_LDFLAGS}")
  get_property(TGT_SRC TARGET ${tgt} PROPERTY SOURCES)
  get_property(TGT_HDR TARGET ${tgt} PROPERTY PUBLIC_HEADER)
  #get_property(TGT_ONAME TARGET ${tgt} PROPERTY OUTPUT_NAME)

  ## copy-in link_libraries
  message("++ [BUNDLE] target ${tgt}: ${TGT_LIBS} ${TGT_SRC} ${TGT_HDR}")
  foreach(_LIB IN LISTS TGT_LIBS)

    if(TARGET ${_LIB} )
      if(EXISTS ${_LIB})
        # message("<< [BUNDLE ${_LIB}] is a target that exists")
        set(LIB_2_ADD ${_LIB} )
      else()
        # message("<< [BUNDLE ${_LIB}] is a target that does not exist")
        get_property(_LIB_IMP_LOC TARGET ${_LIB} PROPERTY IMPORTED_LOCATION)
        get_property(_LIB_IMP_LIB TARGET ${_LIB} PROPERTY IMPORTED_IMPLIB)
        set(LIB_2_ADD ${_LIB_IMP_LOC};${_LIB_IMP_LIB} )
      endif()
    else()

      if(EXISTS ${_LIB})
        set(LIB_2_ADD ${_LIB} )
      endif()

    endif()

    list(APPEND TGT_LIB_FILES ${LIB_2_ADD} )
    unset(LIB_2_ADD)

  endforeach()

  add_custom_target(bundle_directory_${tgt}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${destdir})

  # cmake build system setup copy_in
  message("++ [BUNDLE] copy-in for ${tgt}: ${TGT_LIB_FILES}")
  copy_in_fortarget(${destdir} ${tgt} "${TGT_LIB_FILES}")
  if(TGT_HDR)
    copy_in_fortarget(${destdir} ${tgt} "${TGT_HDR}")
  endif()

  #TODO: try to build shared library from static external dependencies
  #these variables might be related:
  #  - CMAKE_POSITION_INDEPENDENT_CODE
  #  - BUILD_SHARED_LIBS
  #
  get_target_property(type_of_tgt ${tgt} TYPE)
  if(${type_of_tgt} MATCHES "EXECUTABLE")
    add_executable(bundle_${tgt} ${TGT_SRC})
  else()
    add_library(bundle_${tgt} SHARED ${TGT_SRC})
  endif()


  add_dependencies(bundle_copy_${tgt} bundle_directory_${tgt})
  add_dependencies(bundle_${tgt} bundle_copy_${tgt})

  set_target_properties(bundle_${tgt} PROPERTIES OUTPUT_NAME ${tgt})
  ##gcc: -Wl,-rpath,./ ./libboost_unit_test_framework.so
  set_target_properties(bundle_${tgt} PROPERTIES RPATH ./)
  set_target_properties(bundle_${tgt} PROPERTIES INSTALL_RPATH ./)
  set_target_properties(bundle_${tgt} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
  set_target_properties(bundle_${tgt} PROPERTIES EXCLUDE_FROM_ALL TRUE)


  set_target_properties(bundle_${tgt}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${destdir}"
    LIBRARY_OUTPUT_DIRECTORY "${destdir}"
    RUNTIME_OUTPUT_DIRECTORY "${destdir}"
    )

  message("++ [BUNDLE] link directories: ${destdir}")
  link_directories(${destdir})


  foreach(_PATH IN LISTS TGT_LIBS)
    get_filename_component(_FNAME ${_PATH} NAME)
    if(WIN32)
      if(EXISTS ${destdir}/${_FNAME})
        list(APPEND DEPS_FNAME_LIST ${destdir}/${_FNAME})
      else()
        list(APPEND DEPS_FNAME_LIST ${_PATH})
      endif()
    else()
      list(APPEND DEPS_FNAME_LIST ${_PATH})
    endif()
  endforeach()

  if ( MSVC )
    message(STATUS "[BUNDLE] setting output_directory to ${destdir}")
    set_target_properties( bundle_${tgt} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${destdir})
    set_target_properties( bundle_${tgt} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${destdir} )
    set_target_properties( bundle_${tgt} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${destdir} )
    set_target_properties( bundle_${tgt} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${destdir})
    set_target_properties( bundle_${tgt} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${destdir} )
    set_target_properties( bundle_${tgt} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${destdir} )
    set_target_properties( bundle_${tgt} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${destdir})
    set_target_properties( bundle_${tgt} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${destdir} )
    set_target_properties( bundle_${tgt} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${destdir} )
    target_compile_options(bundle_${tgt} PUBLIC "/MT")
    # etc for the other available configuration types (MinSizeRel, RelWithDebInfo)
  endif ( MSVC )

  ## treat ffmpeg dependencies in a special way
  if(UNIX)
    if( ${FFMPEG_USE_STATIC_LIBS})


      if(NOT SQY_MUST_INCLUDE_LIBS)
        set(SQY_MUST_INCLUDE_LIBS x264;x265)
      endif()

      message(STATUS "[BUNDLE] trying to resolve dependencies for ${SQY_MUST_INCLUDE_LIBS}")

      foreach(_DEP IN LISTS SQY_MUST_INCLUDE_LIBS)
        find_library(${_DEP}_STATIC_LIBRARY_PATH NAMES lib${_DEP}${CMAKE_STATIC_LIBRARY_SUFFIX})
        find_library(${_DEP}_DYNAMIC_LIBRARY_PATH NAMES ${_DEP} lib${_DEP} lib${_DEP}${CMAKE_SHARED_LIBRARY_SUFFIX})

        set(_DEP_INCLUDED FALSE)

        if(EXISTS ${${_DEP}_STATIC_LIBRARY_PATH} )

          get_filename_component(${_DEP}_RDIR ${${_DEP}_STATIC_LIBRARY_PATH} ABSOLUTE)
          get_filename_component(${_DEP}_RDIRFNAME ${${_DEP}_RDIR} DIRECTORY)

          link_directories(${${_DEP}_RDIRFNAME})
          if(NOT TARGET ${_DEP})
            add_library(${_DEP} STATIC IMPORTED)
            set_target_properties(${_DEP} PROPERTIES IMPORTED_LOCATION ${${_DEP}_RDIR})
            set_target_properties(${_DEP} PROPERTIES LINKER_LANGUAGE C)
            mark_as_advanced(${_DEP})
          endif()
          LIST (APPEND DEPS_FNAME_LIST ${_DEP})
          set(_DEP_INCLUDED TRUE)
          message("++ [BUNDLE] static ${_DEP} added ${${_DEP}_STATIC_LIBRARY_PATH}")
        else()

          if(EXISTS ${${_DEP}_DYNAMIC_LIBRARY_PATH})
            get_filename_component(${_DEP}_RDIR ${${_DEP}_DYNAMIC_LIBRARY_PATH} ABSOLUTE)
            get_filename_component(${_DEP}_RDIRFNAME ${${_DEP}_RDIR} DIRECTORY)

            if(NOT ${_DEP_INCLUDED})
              link_directories(${${_DEP}_RDIRFNAME})
              if(NOT TARGET ${_DEP})
                add_library(${_DEP} SHARED IMPORTED)
                set_target_properties(${_DEP} PROPERTIES IMPORTED_LOCATION ${${_DEP}_RDIR})
                set_target_properties(${_DEP} PROPERTIES LINKER_LANGUAGE C)
                mark_as_advanced(${_DEP})
              endif()
              LIST (APPEND DEPS_FNAME_LIST ${_DEP})
              set(_DEP_INCLUDED TRUE)
              message("++ [BUNDLE] shared ${_DEP} added ${${_DEP}_DYNAMIC_LIBRARY_PATH}")

            endif()
          else()

            #neither the static nor the dynamic library was found by find_library
            #if on UNIX, pkg-config may help
            if(UNIX OR APPLE)
              if(PkgConfig_FOUND)
                pkg_check_modules(PKG_${_DEP} REQUIRED IMPORTED_TARGET ${_DEP})

                if(PKG_${_DEP}_FOUND AND IS_DIRECTORY ${PKG_${_DEP}_STATIC_LIBRARY_DIRS})
                  link_directories(${PKG_${_DEP}_STATIC_LIBRARY_DIRS})
                  # if(NOT TARGET ${_DEP})
                  #   add_library(${_DEP} STATIC IMPORTED)
                  # endif()
                  LIST (APPEND DEPS_FNAME_LIST "PkgConfig::PKG_${_DEP}")
                  set(_DEP_INCLUDED TRUE)
                  set(${_DEP}_RDIR ${PKG_${_DEP}_STATIC_LIBRARY_DIRS}/lib${_DEP}${CMAKE_STATIC_LIBRARY_SUFFIX})
                  message("++ [BUNDLE] static ${_DEP} added through pkg-config (${${_DEP}_RDIR})")

                else()

                  if(PKG_${_DEP}_FOUND)
                    link_directories(${PKG_${_DEP}_LIBRARY_DIRS})
                    set(_DEP_INCLUDED TRUE)
                    set(${_DEP}_RDIR ${PKG_${_DEP}_LIBRARY_DIRS}/lib${_DEP}${CMAKE_SHARED_LIBRARY_SUFFIX})
                    message("++ [BUNDLE] shared ${_DEP} added through pkg-config (${${_DEP}_RDIR})")
                    LIST (APPEND DEPS_FNAME_LIST "PkgConfig::PKG_${_DEP}")

                    # if(NOT TARGET ${_DEP})
                    #   #add_library(PkgConfig::PKG_${_DEP} SHARED IMPORTED)
                    #   add_library(${_DEP} SHARED IMPORTED)
                    # endif()

                  endif(PKG_${_DEP}_FOUND)

                endif()
              endif(PkgConfig_FOUND)
            endif(UNIX OR APPLE)


          endif()
        endif()

        if(NOT ${_DEP_INCLUDED})
          message(WARNING "unable to locate lib${_DEP}, considered a dependency of requested FFMPEG targets")
        else()
          string(REPLACE " " ";" TGT_LDFLAGS "${TGT_LDFLAGS}")
          REGEX_REMOVE_ITEM("${TGT_LDFLAGS}" ".*${_DEP}$" TGT_LDFLAGS)
          string(REPLACE ";" " " TGT_LDFLAGS "${TGT_LDFLAGS}")
        endif()

      endforeach()

      # include(CheckFunctionExists)
      # CHECK_FUNCTION_EXISTS(pow EXTRA_LIBM_NOT_NEEDED)
      # if(NOT EXTRA_LIBM_NOT_NEEDED)
      #   LIST (APPEND FFMPEG_LIBRARIES m)
      # endif()
    endif(${FFMPEG_USE_STATIC_LIBS})

    ## many/most Linux OS deploy their static libraries being compiled with -fPIE
    ## it doesn't make too much sense to try and include them for the distribution of libsqeazy
    ## the following tries to replace all static libraries with dynamic ones
    if(NOT APPLE)

      if(NOT SQY_BLACKLIST_STATIC_LIBS)
        set(SQY_BLACKLIST_STATIC_LIBS "z;bz2;pthread;m;dl")
      else()
        string(REPLACE " " ";" SQY_BLACKLIST_STATIC_LIBS "${SQY_BLACKLIST_STATIC_LIBS}")
      endif()

      message("++ [BUNDLE] replacing blacklisted static libs ${SQY_BLACKLIST_STATIC_LIBS} in ${DEPS_FNAME_LIST}")
      foreach(_BLACKLIB IN LISTS SQY_BLACKLIST_STATIC_LIBS)
        foreach(_DEP IN LISTS DEPS_FNAME_LIST)
          if(${_DEP} MATCHES "-l${_BLACKLIB}$" OR ${_DEP} MATCHES ".*lib${_BLACKLIB}${CMAKE_STATIC_LIBRARY_SUFFIX}")
            message("++ [BUNDLE] found ${_BLACKLIB} in ${_DEP}")
            list(REMOVE_ITEM DEPS_FNAME_LIST ${_DEP})
            list(FIND DEPS_FNAME_LIST ${CMAKE_SHARED_LIBRARY_PREFIX}${_BLACKLIB}${CMAKE_SHARED_LIBRARY_SUFFIX} _BLACKLIB_INDEX)
            if(${_BLACKLIB_INDEX} LESS 0)
              list(APPEND DEPS_FNAME_LIST ${CMAKE_SHARED_LIBRARY_PREFIX}${_BLACKLIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
            endif()
          else()
            continue()
          endif()
        endforeach()
      endforeach()

    endif(NOT APPLE)


  endif()

  set_property(TARGET bundle_${tgt} PROPERTY LINK_FLAGS ${TGT_LDFLAGS})
  message("++ [BUNDLE] link bundle to : ${DEPS_FNAME_LIST}")
  target_link_libraries(bundle_${tgt} ${DEPS_FNAME_LIST})

endfunction(BUNDLE)
