// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_BRICK_APP_H_
#define BRICK_BRICK_APP_H_
#pragma once

// ToDo: move defines to separate file?

#define VERSION "0.1.25.30"  // IM API version in last part!
#define APP_VERSION "Robin-Brick/" VERSION
#define CEF_PRODUCT_VERSION APP_VERSION " (BitrixDesktop/3.x compatible) Chrome/41.0.2272.104"
#define APP_NAME "Brick"
#define APP_COMMON_NAME "brick"

#define BRICK_SHUTDOWN_TIMEOUT 2000L

#include <string>
#include "include/internal/cef_types_wrappers.h"
#include "brick/common/app_settings.h"

class BrickApp : public CefBase {

 public:
  static CefBrowserSettings GetBrowserSettings(std::string work_dir, AppSettings app_settings);
  static CefSettings GetCefSettings(std::string work_dir, AppSettings app_settings);
  static std::string GetUserConfig();
  static std::string GetSystemConfig();

  // Platform specified methods
  static std::string FindUserConfig(const char* name);
  static std::string FindSystemConfig(const char* name);
  static const char* GetConfigHome();
  static const char* GetCacheHome();

  IMPLEMENT_REFCOUNTING(BrickApp);
};

#endif  // BRICK_BRICK_APP_H_
