// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_BRICK_APP_H_
#define BRICK_BRICK_APP_H_
#pragma once

// TODO(buglloc): move defines to separate file?

#define JS_API_VERSION "31"
#define VERSION "0.2.7" "." JS_API_VERSION  // IM API version in last part!
#define APP_VERSION "Brick/" VERSION "/beta"
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
  static const std::string GetCurrentLanguage(bool withTags = false);
  static const std::string GetAcceptLanguageList();

  // Platform specified methods
  static std::string FindUserConfig(const char* name);
  static std::string FindSystemConfig(const char* name);
  static const char* GetConfigHome();

  IMPLEMENT_REFCOUNTING(BrickApp);
};

#endif  // BRICK_BRICK_APP_H_
