# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

#
# Source files.
#

# desktop sources.
set(BRICK_SRCS
  brick_app.cpp
  brick_app.h
  cef_app.cpp
  cef_app.h
  cef_handler.cpp
  cef_handler.h
  command_callbacks.cpp
  command_callbacks.h
  helper.h
  helper.cpp
  resource_util.h
  resource_util_posix.cpp
  window_util.h
  notification.h
  platform_util.h
  )
set(BRICK_COMMON_SRCS
  common/app_settings.cpp
  common/app_settings.h
  )
  set(BRICK_ACCOUNTS_SRCS
  account_manager.cpp
  account_manager.h
  account.cpp
  account.h
  )
set(BRICK_SRCS_LINUX
  cef_handler_linux.cpp
  brick_app_linux.cpp
  resource_util_linux.cpp
  window_util_linux.cpp
  notification_linux.cpp
  platform_util_linux.cpp
  )
set(BRICK_EXTENSIONS_SRCS
  v8_handler.cpp
  v8_handler.h
  message_delegate/app_message_delegate.cpp
  message_delegate/app_message_delegate.h
  message_delegate/app_window_message_delegate.cpp
  message_delegate/app_window_message_delegate.h
  )
set(BRICK_STATUS_ICON_SRCS
  status_icon/status_icon.cpp
  status_icon/status_icon.h
  )
set(BRICK_STATUS_ICON_SRCS_LINUX
  status_icon/status_icon_linux.cpp
  )
set(BRICK_WINDOWS_SRCS
  window/main_window.cpp
  window/main_window.h
  window/about_window.cpp
  window/about_window.h
  window/accounts_window.cpp
  window/accounts_window.h
  )
set(BRICK_WINDOWS_SRCS_LINUX
  window/main_window_linux.cpp
  window/about_window_linux.cpp
  window/accounts_window_linux.cpp
  )
set(BRICK_JSON_SRCS
  ../third-party/json/json.h
  ../third-party/json/json-forwards.h
  ../third-party/json/jsoncpp.cpp
  )

set(BRICK_RESOURCE_FILES
  ../Resources/files/empty.html
  )

APPEND_PLATFORM_SOURCES(BRICK_SRCS)
APPEND_PLATFORM_SOURCES(BRICK_STATUS_ICON_SRCS)
APPEND_PLATFORM_SOURCES(BRICK_WINDOWS_SRCS)
source_group(brick FILES ${BRICK_SRCS})

set(BRICK_HTTPCLIENT_SRCS
  ../third-party/httpclient/httpclient.cpp
  ../third-party/httpclient/httpclient.h
  )

set(BRICK_SRCS
  ${BRICK_SRCS}
  ${BRICK_COMMON_SRCS}
  ${BRICK_ACCOUNTS_SRCS}
  ${BRICK_EXTENSIONS_SRCS}
  ${BRICK_STATUS_ICON_SRCS}
  ${BRICK_WINDOWS_SRCS}
  ${BRICK_HTTPCLIENT_SRCS}
  ${BRICK_JSON_SRCS}
  )

set(BRICK_RESOURCE_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/brick/files")

# Target executable names.
set(CEF_TARGET "brick")

# Logical target used to link the libcef library.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# Determine the target output directory.
SET_CEF_TARGET_OUT_DIR()

#
# Linux configuration.
#

set(BRICK_DESKTOP_EXT_EMBED_OBJ_FILES "desktop_extension.js")
foreach(FILENAME ${BRICK_DESKTOP_EXT_EMBED_OBJ_FILES})
    get_filename_component(FILENAME_ONLY ${FILENAME} NAME)
    get_filename_component(FILEPATH_ONLY ${FILENAME} PATH)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o
                       WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/brick/"
                       COMMAND ${CMAKE_OBJCOPY}
                       ARGS -I binary -O elf64-x86-64 -B i386 ${FILENAME_ONLY} ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o )
    list(APPEND BRICK_DESKTOP_EXT_EMBED_OBJ_FILES ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o)
    message(STATUS "Objcopy'ing ${FILENAME}")
endforeach(FILENAME)

find_library(LIBNOTIFY_LIBRARIES notify
    NAMES ${LIBNOTIFY_NAMES}
)

find_package(CURL REQUIRED)
if(NOT CURL_FOUND)
    message(SEND_ERROR "Failed to find curl")
    return()
else()
    include_directories(${CURL_INCLUDE_DIRS})
endif()


FIND_LINUX_LIBRARIES("gmodule-2.0 gtk+-2.0 gthread-2.0")


# Executable target.
add_executable(${CEF_TARGET} ${BRICK_SRCS} ${BRICK_DESKTOP_EXT_EMBED_OBJ_FILES})
add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

# Set rpath so that libraries can be placed next to the executable.
set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

# Copy binary and resource files to the target output directory.
COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

#COPY_FILES("${CEF_TARGET}" "${BRICK_RESOURCE_FILES}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

target_link_libraries(${CEF_TARGET} ${LIBNOTIFY_LIBRARIES})
target_link_libraries(${CEF_TARGET} ${CURL_LIBRARIES})

# Set SUID permissions on the chrome-sandbox target.
SET_LINUX_SUID_PERMISSIONS("${CEF_TARGET}" "${CEF_TARGET_OUT_DIR}/chrome-sandbox")