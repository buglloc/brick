#ifndef BRICK_APP_H_
#define BRICK_APP_H_
#pragma once

// ToDo: move defines to separate file?

#define VERSION "0.1.11.29" // IM API version in last part!
#define APP_VERSION "Robin-Brick/" VERSION
#define CEF_PRODUCT_VERSION APP_VERSION " (BitrixDesktop/3.x compatible) Chrome/39.0.2171.0"
#define APP_NAME "Brick"
#define APP_COMMON_NAME "brick"

#define IDLE_TIMEOUT 600000L
#define IDLE_CHECK_INTERVAL 4000L

#include <include/internal/cef_types_wrappers.h>

#include "common/app_settings.h"

class BrickApp : public CefBase {

  public:
    static CefBrowserSettings GetBrowserSettings(std::string work_dir, AppSettings app_settings);
    static CefSettings GetCefSettings(std::string work_dir, AppSettings app_settings);
    static std::string GetUserConfig();
    static std::string GetSystemConfig(std::string work_dir);

    // Platform specified methods

    static const char* GetConfigHome();
    static const char* GetCacheHome();
  private:
    IMPLEMENT_REFCOUNTING(BrickApp);
};

#endif // BRICK_APP_H_
