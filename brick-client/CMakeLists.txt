#
# Source files.
#

set(BRICK_CLIENT_SRCS
  main.cc
  main.h
  )
source_group(brick-client FILES ${BRICK_CLIENT_SRCS})

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
add_executable(${CEF_TARGET} ${BRICK_CLIENT_SRCS})
target_link_libraries(${CEF_TARGET} ${CEF_STANDARD_LIBS} ${CMAKE_DL_LIBS})
install(
    TARGETS
        brick-client
    RUNTIME DESTINATION
        "${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/bin"
)

# Additional install rules
install(
  FILES "${CMAKE_SOURCE_DIR}/${CEF_RESOURCE_DIR}/install/brick-client/brick-client.desktop"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}"
)

# Create link to executable in /usr/bin
install(DIRECTORY DESTINATION "${CMAKE_INSTALL_PREFIX}/usr/bin")
INSTALL_SYMLINK("${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/bin/brick-client" "${CMAKE_INSTALL_PREFIX}/usr/bin")

if(NOT SKIP_BRICK_REGISTER)
    # If `make install` calls user we want to install .desktop files, icons, etc
    find_program(DESKTOP_INSTALL_PATH "desktop-file-install")
    if(NOT DESKTOP_INSTALL_PATH)
        message(FATAL_ERROR "Could not find 'desktop-file-install' command.
            Please install 'desktop-file-utils' package or add '-DSKIP_BRICK_REGISTER=1'")
    endif()

    find_program(UPDATE_DESKTOP_PATH "update-desktop-database")
    if(NOT UPDATE_DESKTOP_PATH)
        message(FATAL_ERROR "Could not find 'update-desktop-database' command.
            Please install 'desktop-file-utils' package or add '-DSKIP_BRICK_REGISTER=1'")
    endif()
    # Install .desktop file and update database
    INSTALL(CODE "execute_process(COMMAND ${DESKTOP_INSTALL_PATH}
            \"${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/brick-client.desktop\")")
    INSTALL(CODE "execute_process(COMMAND update-desktop-database -q)")
endif()