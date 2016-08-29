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
  
  #message("++ [COPY_IN] received target dir ${copydest} and ${flist}")
  if(NOT TARGET bundle_copy_${_tgt})
    add_custom_target(bundle_copy_${_tgt})
  endif()

  #TODO: append .so* files for UNIX
  if(UNIX)
    foreach(_ITEM IN LISTS flist)

      file(GLOB GLOBRESULT "${_ITEM}.*")
      list(APPEND flist ${GLOBRESULT})
    endforeach()
  endif()

  foreach(_ITEM IN LISTS flist)
    
    get_filename_component(FNAME ${_ITEM} NAME)

    if(NOT ${FNAME} MATCHES ".*${CMAKE_STATIC_LIBRARY_SUFFIX}")
      get_filename_component(RDIR ${_ITEM} REALPATH)
      get_filename_component(RDIRFNAME ${RDIR} NAME)

      #message("++ [COPY_IN] ${_ITEM} :\n ${RDIR} -> ${destdir}/${FNAME}")
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

  if(NOT TARGET ${tgt})
    message("++ [BUNDLE] There is no target named '${tgt}'")
    return()
  else()
    message("++ [BUNDLE] bundling '${tgt}' to ${destdir}")
  endif()

  ## introspect target
  get_property(TGT_LIBS TARGET ${tgt} PROPERTY LINK_LIBRARIES)
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
	# message("<< [BUNDLE ${_LIB}] is not a target that exists")
	set(LIB_2_ADD ${_LIB} )
      endif()
      
    endif()

    if(NOT ${LIB_2_ADD} MATCHES ".*\.${CMAKE_STATIC_LIBRARY_SUFFIX}")
      list(APPEND TGT_LIB_FILES ${LIB_2_ADD} )
    endif()
    unset(LIB_2_ADD)
  endforeach()

  add_custom_target(bundle_directory_${tgt}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${destdir})
  
  # cmake build system setup copy_in
  copy_in_fortarget(${destdir} ${tgt} ${TGT_LIB_FILES})
  if(TGT_HDR)
    copy_in_fortarget(${destdir} ${tgt} ${TGT_HDR})
  endif()

  #TODO: try to build shared library from static external dependencies
  #these variables might be related:
  #  - CMAKE_POSITION_INDEPENDENT_CODE
  #  - BUILD_SHARED_LIBS
  #
  add_library(bundle_${tgt} SHARED ${TGT_SRC})
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
      list(APPEND DEPS_FNAME_LIST ${destdir}\\${_FNAME})
    else()
      if(NOT ${_FNAME} MATCHES ".*(lib|l)(m|dl|pthread)")
	list(APPEND DEPS_FNAME_LIST ${_FNAME})
      endif()
    endif()
  endforeach()
  
  message("++ [BUNDLE] link bundle to : ${DEPS_FNAME_LIST}")
  target_link_libraries(bundle_${tgt} ${DEPS_FNAME_LIST} pthread m dl)


endfunction(BUNDLE)
