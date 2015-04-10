#ifndef BRICK_APP_H_
#define BRICK_APP_H_
#pragma once

// ToDo: move defines to separate file?

#define VERSION "0.1.16.29" // IM API version in last part!
#define APP_VERSION "Robin-Brick/" VERSION
#define CEF_PRODUCT_VERSION APP_VERSION " (BitrixDesktop/3.x compatible) Chrome/41.0.2272.104"
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
    static std::string GetSystemConfig();

    // Platform specified methods
    static std::string FindUserConfig(const char* name);
    static std::string FindSystemConfig(const char* name);
    static const char* GetConfigHome();
    static const char* GetCacheHome();
  private:
    IMPLEMENT_REFCOUNTING(BrickApp);
};

#endif // BRICK_APP_H_
