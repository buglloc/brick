#
# Source files.
#

set(BRICK_SRCS
  main.cc
  main.h
  )
APPEND_PLATFORM_SOURCES(BRICK_SRCS)
source_group(brick-client FILES ${BRICK_SRCS})

set(BRICK_SRCS
  ${BRICK_SRCS}
  )

# Target executable names.
set(CEF_TARGET "brick-client")

# Determine the target output directory.
set(OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

SET(BRICK_INSTALL_DIR "opt/brick")

#
# Linux configuration.
#
FIND_LINUX_LIBRARIES("gio-2.0")


# Executable target.
add_executable(${CEF_TARGET} ${BRICK_SRCS})
target_link_libraries(${CEF_TARGET} ${CEF_STANDARD_LIBS} ${CMAKE_DL_LIBS})
install(
    TARGETS
        brick-client
    RUNTIME DESTINATION
        "${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/bin"
)

# Set rpath so that libraries can be placed next to the executable.
set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}/bin")

# Copy binary and resource files to the target output directory.
COPY_AND_INSTALL_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "bin")

# Additional install rules
install(
  FILES "${CMAKE_SOURCE_DIR}/${CEF_RESOURCE_DIR}/install/brick-client/brick-client.desktop"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}"
)
# Create link to executable in /usr/bin
install(DIRECTORY DESTINATION "${CMAKE_INSTALL_PREFIX}/usr/bin")
INSTALL_SYMLINK("${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/bin/brick-client" "${CMAKE_INSTALL_PREFIX}/usr/bin")
