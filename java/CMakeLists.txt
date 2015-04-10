INCLUDE_DIRECTORIES(.)

MESSAGE("[java] simple copy_directory to ${INSTALL_JAVA_DIR}")

#TODO: make this configurable
SET(SQY_JNAERATE_PATH "${PROJECT_SOURCE_DIR}/java/jnaerator.jar")
IF(UNIX)
  IF(APPLE)
    SET(SQY_JNAERATE_ARCH "darwin_universal")
  ELSE()
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
      SET(SQY_JNAERATE_ARCH "linux_x64")
    ELSE()
      SET(SQY_JNAERATE_ARCH "linux_x86")
    ENDIF()
  ENDIF()
  configure_file(jnaerate.sh.in jnaerate.sh @ONLY)
ELSE()
  IF(WIN32)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(SQY_JNAERATE_ARCH "win64")
  ELSE()
    SET(SQY_JNAERATE_ARCH "win32")
  ENDIF()
  configure_file(jnaerate.bat.in jnaerate.bat @ONLY)
  ENDIF()
ENDIF()

configure_file(sqeazy.config.arch.jnaerator.in sqeazy.config.${SQY_JNAERATE_ARCH}.jnaerator @ONLY)

add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/java/sqeazy.jar
  COMMAND java -jar ${SQY_JNAERATE_PATH} sqeazy.config.${SQY_JNAERATE_ARCH}.jnaerator
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/java
  )
add_custom_target(${PROJECT_NAME}_java_bindings ALL DEPENDS ${PROJECT_NAME} ${PROJECT_BINARY_DIR}/java/sqeazy.jar)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sqeazy.jar DESTINATION ${INSTALL_JAVA_DIR})
