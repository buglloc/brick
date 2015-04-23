# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

#
# Determine the platform specific features.
#

if(OS_LINUX)
  set(DESKTOP $ENV{XDG_CURRENT_DESKTOP})
  if(NOT DEFINED UNITY_DESKTOP)
    if ("${DESKTOP}" STREQUAL "Unity")
      set(UNITY_DESKTOP 1)
    else()
      set(UNITY_DESKTOP 0)
    endif()
  endif()


  INCLUDE(CMakeDependentOption)
  CMAKE_DEPENDENT_OPTION(UNITY "Compile Unity version" ON
                        "UNITY_DESKTOP" OFF)

  if (UNITY)
    message(STATUS "Turning on Unity version")
    add_definitions(-Dunity)
  endif()

endif()

#
# Source files.
#

# desktop sources.
set(BRICK_SRCS
  brick_app.cc
  brick_app.h
  cef_app.cc
  cef_app.h
  cef_handler.cc
  cef_handler.h
  command_callbacks.cc
  command_callbacks.h
  helper.h
  helper.cc
  resource_util.h
  resource_util_posix.cc
  window_util.h
  notification.h
  platform_util.h
  api_error.h
  cache_manager.h
  cache_manager.cc
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
  cef_handler_linux.cc
  brick_app_linux.cc
  window_util_linux.cc
  notification_linux.cc
  platform_util_linux.cc
  )
set(BRICK_EXTENSIONS_SRCS
  v8_handler.cc
  v8_handler.h
  message_delegate/app_message_delegate.cc
  message_delegate/app_message_delegate.h
  message_delegate/app_window_message_delegate.cc
  message_delegate/app_window_message_delegate.h
  )
set(BRICK_STATUS_ICON_SRCS
  indicator/indicator.cc
  indicator/indicator.h
  )
set(BRICK_STATUS_ICON_SRCS_LINUX
  indicator/indicator_linux.cc
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
  )
set(BRICK_HTTPCLIENT_SRCS
  httpclient/httpclient.cc
  httpclient/httpclient.h
  )

APPEND_PLATFORM_SOURCES(BRICK_SRCS)
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
  ${BRICK_STATUS_ICON_SRCS}
  ${BRICK_WINDOWS_SRCS}
  ${BRICK_HTTPCLIENT_SRCS}
  ${BRICK_JSON_SRCS}
  ${BRICK_EXTERNAL_INTERFACE}
  ${BRICK_EVENT_SYSTEM}
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
  indicators
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


FIND_LINUX_LIBRARIES("gmodule-2.0 gtk+-2.0 gthread-2.0 libcurl libnotify xscrnsaver")
if (UNITY)
  FIND_LINUX_LIBRARIES("unity appindicator-0.1")
endif()

# Executable target.
add_executable(${CEF_TARGET} ${BRICK_SRCS} ${BRICK_EMBED_FILES})
add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
add_dependencies(${CEF_TARGET} sodium)
target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper sodium ${CEF_STANDARD_LIBS})
install(
    TARGETS
        brick
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
)

# Set rpath so that libraries can be placed next to the executable.
set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CEF_TARGET_OUT_DIR}/bin")

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
  FILES "${CMAKE_SOURCE_DIR}/${CEF_RESOURCE_DIR}/install/brick/brick"
  DESTINATION "${CMAKE_INSTALL_PREFIX}"
)
install(
  FILES "${CMAKE_SOURCE_DIR}/${CEF_RESOURCE_DIR}/install/brick/brick.desktop"
  DESTINATION "${CMAKE_INSTALL_PREFIX}"
)
install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/${CEF_RESOURCE_DIR}/install/usr"
  DESTINATION "/"
)
# Create link to executable in /usr/bin
INSTALL_SYMLINK("${CMAKE_INSTALL_PREFIX}/bin/brick" "/usr/bin")

# Set suid flag for chromium purposes
install(
  FILES "${CMAKE_SOURCE_DIR}/${CEF_BINARY_DIR}/chrome-sandbox"
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID
  DESTINATION "bin"
)