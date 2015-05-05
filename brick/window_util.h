// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_WINDOW_UTIL_H_
#define BRICK_WINDOW_UTIL_H_
#pragma once

#include <string>
#if defined(__linux__)
#include <glib.h>
#else
#endif

#include "include/internal/cef_linux.h"
#include "window/browser_window.h"

namespace window_util {
  CefWindowHandle GetParent(CefWindowHandle handle);

  void Resize(CefWindowHandle handle, int width, int height);
  void SetMinSize(CefWindowHandle handle, int width, int height);
  void FixSize(CefWindowHandle handle, int width, int height);
  void SetTitle(CefWindowHandle handle, std::string title);

  void CenterPosition(CefWindowHandle handle);

  void Hide(CefWindowHandle handle);
  void Show(CefWindowHandle handle);

  void ConfigureAsDialog(CefWindowHandle handle);
  void ConfigureAsTopmost(CefWindowHandle handle);
  void InitAsPopup(CefWindowHandle handle);

  void SetClassHints(CefWindowHandle handle, char *res_name, char *res_class);
  void SetLeaderWindow(CefWindowHandle handle);
  CefWindowHandle GetLeaderWindow();

  void InitHooks();
  void InitWindow(CefWindowHandle handle, bool is_leader);

  CefRect GetDefaultScreenRect();

  void FlushChanges();

#if defined(__linux__)
  GList* GetDefaultIcons();
  void SetDefaultIcons(GList* icons);
#else
#endif

  BrowserWindow* LookupBrowserWindow(CefWindowHandle native_window);
  BrowserWindow* LookupBrowserWindow(GdkEvent* event);

}  // namespace window_util
#endif  // BRICK_WINDOW_UTIL_H_
