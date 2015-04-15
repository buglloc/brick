#include <fstream>
#include "brick_app.h"
#include "helper.h"
#include "platform_util.h"


CefBrowserSettings
BrickApp::GetBrowserSettings(std::string work_dir, AppSettings app_settings) {
  CefBrowserSettings settings;
  return settings;
}

CefSettings
BrickApp::GetCefSettings(std::string work_dir, AppSettings app_settings) {
  CefSettings settings;
  CefString(&settings.product_version) = CEF_PRODUCT_VERSION;

  if (!app_settings.cache_path.empty())
    CefString(&settings.cache_path) = app_settings.cache_path;
  else
    CefString(&settings.cache_path) = std::string(GetCacheHome()) + "/" + APP_COMMON_NAME + "/cef";

  if (!app_settings.log_file.empty())
    CefString(&settings.log_file) = app_settings.log_file;
  else
    CefString(&settings.log_file) = std::string(GetCacheHome()) + "/" + APP_COMMON_NAME + "/runtime.log";

  CefString(&settings.resources_dir_path) = app_settings.resource_dir + "/cef";
  CefString(&settings.locales_dir_path) = app_settings.resource_dir + "/cef/locales";
  settings.ignore_certificate_errors = app_settings.ignore_certificate_errors;
  settings.log_severity = app_settings.log_severity;

  // HACK: we have to set white background to avoid black flashes.
  settings.background_color = CefColorSetARGB(255, 255, 255, 255);

  return settings;
}

std::string
BrickApp::GetSystemConfig() {
  std::string file = FindSystemConfig("config.json");
  if (file.empty())
    return "";

  std::ifstream ifs(file.c_str());
  std::string content(
     (std::istreambuf_iterator<char>(ifs) ),
     (std::istreambuf_iterator<char>()    )
  );

  return content;
}

std::string
BrickApp::GetUserConfig() {
  std::string file = FindUserConfig("config.json");
  if (file.empty())
    return "";

  std::ifstream ifs(file.c_str());
  std::string content(
     (std::istreambuf_iterator<char>(ifs) ),
     (std::istreambuf_iterator<char>()    )
  );

  return content;
}