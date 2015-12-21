# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.


#
# Source files.
#

# desktop sources.
set(BRICK_SRCS
  brick_app.cc
  brick_app.h
  client_app.cc
  client_app.h
  client_handler.cc
  client_handler.h
  command_callbacks.h
  helper.h
  helper.cc
  window_util.h
  notification_manager.h
  notification_manager.cc
  platform_util.h
  api_error.h
  cache_manager.h
  cache_manager.cc
  auth_client.h
  auth_client.cc
  request_util.h
  request_util.cc
  avatar_client.h
  avatar_client.cc
  desktop_media.h
  download_client.h
  download_client.cc
  download_history_item.h
  brick_types.h
  )
set(BRICK_COMMON_SRCS
  common/app_settings.cc
  common/app_settings.h
  )
set(BRICK_ACCOUNTS_SRCS
  account_manager.cc
  account_manager.h
  account.cc
  account.h
  )
set(BRICK_SRCS_LINUX
  client_handler_linux.cc
  brick_app_linux.cc
  window_util_linux.cc
  notification_manager_linux.cc
  platform_util_linux.cc
  desktop_media_linux.cc
  )
set(BRICK_EXTENSIONS_SRCS
  v8_handler.cc
  v8_handler.h
  message_delegate/app_message_delegate.cc
  message_delegate/app_message_delegate.h
  message_delegate/app_window_message_delegate.cc
  message_delegate/app_window_message_delegate.h
  )
set(BRICK_RESOURCE_SRCS
  resource/resource_util.h
  resource/resource_util_posix.cc
  resource/injected_js_resource_provider.h
  resource/injected_js_resource_provider.cc
  resource/last_resource_provider.h
  resource/last_resource_provider.cc
  resource/temporary_page_resource_provider.h
  resource/temporary_page_resource_provider.cc
  resource/desktop_media_resource_provider.h
  resource/settings_resource_provider.h
  resource/settings_resource_provider.cc
  )
set(BRICK_RESOURCE_SRCS_LINUX
  resource/desktop_media_resource_provider_linux.cc
  )
set(BRICK_STATUS_ICON_SRCS
  indicator/indicator.cc
  indicator/indicator.h
  indicator/base_icon.h
  indicator/base_icon.cc
  )
set(BRICK_STATUS_ICON_SRCS_LINUX
  indicator/indicator_linux.cc
  indicator/gtk2_status_icon.h
  indicator/gtk2_status_icon.cc
  indicator/app_indicator_icon.h
  indicator/app_indicator_icon.cc
  indicator/unity_launcher.h
  indicator/unity_launcher.cc
  )
set(BRICK_WINDOWS_SRCS
  window/base_window.h
  window/base_window.cc
  window/about_window.cc
  window/about_window.h
  window/accounts_window.cc
  window/accounts_window.h
  window/edit_account_window.cc
  window/edit_account_window.h
  window/browser_window.h
  window/browser_window.cc
  window/window_features.h
  window/window_features.cc
  )
set(BRICK_WINDOWS_SRCS_LINUX
  window/base_window_linux.cc
  window/about_window_linux.cc
  window/accounts_window_linux.cc
  window/edit_account_window_linux.cc
  window/browser_window_linux.cc
  )
set(BRICK_JSON_SRCS
  ../third-party/json/json.h
  ../third-party/json/json-forwards.h
  ../third-party/json/jsoncpp.cc
  )
set(BRICK_CODECS_SRCS
  )
set(BRICK_EXTERNAL_INTERFACE
  external_interface/dbus_protocol.h
  external_interface/dbus_protocol.cc
  external_interface/external_message_delegate.h
  external_interface/external_message_delegate.cc
  external_interface/app_message_delegate.h
  external_interface/app_message_delegate.cc
  external_interface/app_window_message_delegate.h
  external_interface/app_window_message_delegate.cc
  )
set(BRICK_EVENT_SYSTEM
  event/event.h
  event/event_bus.cc
  event/event_handler.h
  event/event_object.h
  event/event_sender.h
  event/handler_registration.h
  event/account_list_event.h
  event/account_switch_event.h
  event/indicator_badge_event.h
  event/indicator_state_event.h
  event/indicator_tooltip_event.h
  event/sleep_event.h
  event/user_away_event.h
  event/download_start_event.h
  event/download_progress_event.h
  event/download_complete_event.h
  event/notification_event.h
  )

APPEND_PLATFORM_SOURCES(BRICK_SRCS)
APPEND_PLATFORM_SOURCES(BRICK_RESOURCE_SRCS)
APPEND_PLATFORM_SOURCES(BRICK_STATUS_ICON_SRCS)
APPEND_PLATFORM_SOURCES(BRICK_WINDOWS_SRCS)
APPEND_PLATFORM_SOURCES(BRICK_EXTERNAL_INTERFACE)
APPEND_PLATFORM_SOURCES(BRICK_EVENT_SYSTEM)

source_group(brick FILES ${BRICK_SRCS})

set(BRICK_SRCS
  ${BRICK_SRCS}
  ${BRICK_COMMON_SRCS}
  ${BRICK_ACCOUNTS_SRCS}
  ${BRICK_EXTENSIONS_SRCS}
  ${BRICK_RESOURCE_SRCS}
  ${BRICK_STATUS_ICON_SRCS}
  ${BRICK_WINDOWS_SRCS}
  ${BRICK_JSON_SRCS}
  ${BRICK_EXTERNAL_INTERFACE}
  ${BRICK_EVENT_SYSTEM}
  ${BRICK_CODECS_SRCS}
  )
set(BRICK_EMBED_FILES
  desktop_extension.js
  window/about.glade
  window/accounts.glade
  window/edit_account.glade
  )

set(
  BRICK_RESOURCES
  app_icons
  indicator
  web
  ui
  )

# Target executable names.
set(CEF_TARGET "brick")

# Logical target used to link the libcef library.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# Determine the target output directory.
SET(CEF_TARGET_OUT_DIR "default" CACHE STRING "Output directory" PARENT_SCOPE)
if(NOT DEFINED CEF_TARGET_OUT_DIR)
  SET_CEF_TARGET_OUT_DIR()
endif()
message(STATUS "Setting output directory to: ${CEF_TARGET_OUT_DIR}")

SET(BRICK_INSTALL_DIR "opt/brick")

#
# Linux configuration.
#

if(PROJECT_ARCH STREQUAL "x86_64")
  # 64-bit architecture.
  set(BRICK_EMBED_FORMAT "elf64-x86-64")
elseif(PROJECT_ARCH STREQUAL "x86")
  # 32-bit architecture.
  set(BRICK_EMBED_FORMAT "elf32-i386")
endif()

set(BRICK_EMBED_FILES_COMPILED )
foreach(FILENAME ${BRICK_EMBED_FILES})
    get_filename_component(FILENAME_ONLY ${FILENAME} NAME)
    get_filename_component(FILEPATH_ONLY ${FILENAME} PATH)
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o
                       WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/brick/"
                       COMMAND ${CMAKE_OBJCOPY}
                       ARGS -I binary -O ${BRICK_EMBED_FORMAT} -B i386 ${FILENAME} ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o )
    list(APPEND BRICK_EMBED_FILES ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o)
    message(STATUS "Objcopy'ing ${FILENAME}")
endforeach(FILENAME)


FIND_LINUX_LIBRARIES("gmodule-2.0 gtk+-2.0 gthread-2.0 libnotify xscrnsaver cairo")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  if(PROJECT_ARCH STREQUAL "x86_64")
    set(CEF_LIBRARIES_ARCHIVE_URL     "https://github.com/buglloc/cef-builds/raw/0d15971961fda87682bd5e067cdcf5450e72b6b8/libs/cef-x86_64.tar.gz")
    set(CEF_LIBRARIES_ARCHIVE_HASH    "5edf8e04ffb6acbcb88c26b3d4fcae9020d18967c9c4b9d3afa32530354e883d")
  elseif(PROJECT_ARCH STREQUAL "x86")
    set(CEF_LIBRARIES_ARCHIVE_URL     "https://github.com/buglloc/cef-builds/raw/0d15971961fda87682bd5e067cdcf5450e72b6b8/libs/cef-i686.tar.gz")
    set(CEF_LIBRARIES_ARCHIVE_HASH    "8ed11e59556f77de7249804a6eb32fdf88f1236a7b22bc6acae0894564d4c2c5")
  endif()

  DOWNLOAD_EXTRACT(
    ${CEF_LIBRARIES_ARCHIVE_URL}
    "${CMAKE_SOURCE_DIR}/Release"
    ${CEF_LIBRARIES_ARCHIVE_HASH}
    "3.2454.1344.g2782fb8.${PROJECT_ARCH}"
  )
endif()

# Executable target.
add_executable(${CEF_TARGET} ${BRICK_SRCS} ${BRICK_EMBED_FILES})
add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS} ${CMAKE_DL_LIBS})
install(
    TARGETS
        brick
    RUNTIME DESTINATION
        "${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/bin"
)

set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CEF_TARGET_OUT_DIR}/bin")
if(NOT DISABLE_RPATH)
# Set rpath so that libraries can be placed next to the executable.
set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
endif()

# Copy binary and resource files to the target output directory.
COPY_AND_INSTALL_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "bin")
COPY_AND_INSTALL_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "resources/cef")
COPY_AND_INSTALL_FILES("${CEF_TARGET}" "${BRICK_RESOURCES}" "${CEF_RESOURCE_DIR}" "resources")
# Hack for ICU
COPY_AND_INSTALL_FILES("${CEF_TARGET}" "icudtl.dat" "${CEF_RESOURCE_DIR}" "bin")

# Set SUID permissions on the chrome-sandbox target.
SET_LINUX_SUID_PERMISSIONS("${CEF_TARGET}" "${CEF_TARGET_OUT_DIR}/bin/chrome-sandbox")

# Additional install rules
install(
  PROGRAMS "${CMAKE_SOURCE_DIR}/${CEF_RESOURCE_DIR}/install/brick/brick"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}"
)
install(
  FILES "${CMAKE_SOURCE_DIR}/${CEF_RESOURCE_DIR}/install/brick/brick.desktop"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}"
)
install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/${CEF_RESOURCE_DIR}/install/usr"
  DESTINATION "${CMAKE_INSTALL_PREFIX}"
)
install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/${CEF_RESOURCE_DIR}/install/etc"
  DESTINATION "${CMAKE_INSTALL_PREFIX}"
)

# Create link to executable in /usr/bin
install(DIRECTORY DESTINATION "${CMAKE_INSTALL_PREFIX}/usr/bin")
INSTALL_SYMLINK("${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/brick" "${CMAKE_INSTALL_PREFIX}/usr/bin")

# Set suid flag for chromium purposes
install(
  PROGRAMS "${CMAKE_SOURCE_DIR}/${CEF_BINARY_DIR}/chrome-sandbox"
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/bin"
)

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

  find_program(XDG_ICON_PATH "xdg-icon-resource")
  if(NOT XDG_ICON_PATH)
    message(FATAL_ERROR "Could not find 'desktop-file-install' command.
            Please install 'xdg-utils' package or add '-DSKIP_BRICK_REGISTER=1'")
  endif()

  # Install .desktop file and update database
  INSTALL(CODE "execute_process(COMMAND ${DESKTOP_INSTALL_PATH}
          \"${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/brick.desktop\")")
  INSTALL(CODE "execute_process(COMMAND ${UPDATE_DESKTOP_PATH} -q)")

  # Install icons
  set(INSTALL_ICONS_SIZES 16 32 48 128 256 512)
  foreach(ICON_SIZE ${INSTALL_ICONS_SIZES})
    INSTALL(CODE "execute_process(COMMAND
      ${XDG_ICON_PATH} install --novendor
        --size ${ICON_SIZE} \"${CMAKE_INSTALL_PREFIX}/${BRICK_INSTALL_DIR}/resources/app_icons/brick${ICON_SIZE}.png\" \"brick\")")
  endforeach()
  INSTALL(CODE "execute_process(COMMAND ${XDG_ICON_PATH} forceupdate --theme hicolor)")
endif()