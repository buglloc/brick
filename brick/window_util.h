#ifndef BRICK_WINDOW_UTIL_H_
#define BRICK_WINDOW_UTIL_H_
#pragma once

#include <include/internal/cef_linux.h>

namespace window_util {
  CefWindowHandle GetParent(CefWindowHandle handle);

  void Resize(CefWindowHandle handle, int width, int height);
  void SetMinSize(CefWindowHandle handle, int width, int height);
  void FixSize(CefWindowHandle handle, int width, int height);
  void SetTitle(CefWindowHandle handle, std::string title);

  void Hide(CefWindowHandle handle);
  void Show(CefWindowHandle handle);

  void SetTypeDialog(CefWindowHandle handle);
  void InitAsPopup(CefWindowHandle handle);

  void SetLeaderWindow(CefWindowHandle handle);
  CefWindowHandle GetLeaderWindow();

  void InitHooks();
  void InitWindow(CefWindowHandle handle);
}
#endif  // BRICK_WINDOW_UTIL_H_
