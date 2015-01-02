#ifndef BRICK_APP_H_
#define BRICK_APP_H_
#pragma once

#define VERSION "0.1.1.29" // IM API version in last part!
#define APP_VERSION "Robin-Brick/" VERSION
#define CEF_PRODUCT_VERSION APP_VERSION " (BitrixDesktop/3.x compatible) Chrome/39.0.2171.0"
#define APP_NAME "Brick"
#define APP_COMMON_NAME "brick"

#include <include/internal/cef_types_wrappers.h>

#include "common/app_settings.h"

class BrickApp {

  public:
    static CefBrowserSettings GetBrowserSettings(AppSettings app_settings);
    static CefSettings GetCefSettings(AppSettings app_settings);
    static std::string GetConfig();

    // Platform specified methods
    static const char* GetConfigHome();
    static const char* GetCacheHome();
};

#endif // BRICK_APP_H_
