INCLUDE_DIRECTORIES(.)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD        # Adds a post-build event to MyTest
  COMMAND ${CMAKE_COMMAND} -E copy_directory  # which executes "cmake - E copy_if_different..."
  "${PROJECT_BINARY_DIR}/java"      # <--this is in-file
  ${INSTALL_JAVA_DIR})                 # <--this is out-file path



# Install the sqeazy-config.cmake and sqeazy-config-version.cmake
install(DIRECTORY java DESTINATION ${INSTALL_JAVA_DIR})

