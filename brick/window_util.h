#ifndef BRICK_WINDOW_UTIL_H_
#define BRICK_WINDOW_UTIL_H_
#pragma once

#include <include/internal/cef_linux.h>
#if defined(__linux__)
#include <glib.h>
#else
#endif

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

  void SetTypeDialog(CefWindowHandle handle);
  void InitAsPopup(CefWindowHandle handle);

  void SetLeaderWindow(CefWindowHandle handle);
  CefWindowHandle GetLeaderWindow();

  void InitHooks();
  void InitWindow(CefWindowHandle handle);

#if defined(__linux__)
  GList* GetDefaultIcons();
  void SetDefaultIcons(GList* icons);
#else
#endif

  BrowserWindow* LookupBrowserWindow(CefWindowHandle native_window);
  BrowserWindow* LookupBrowserWindow(GdkEvent* event);

}
#endif  // BRICK_WINDOW_UTIL_H_
