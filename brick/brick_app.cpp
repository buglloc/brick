#include <fstream>
#include "brick_app.h"


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

  // HACK: we have to set blue background to avoid white flashes (сan badly impact on history and other popups).
  settings.background_color = CefColorSetARGB(255, 47, 198, 247);

  return settings;
}

std::string
BrickApp::GetConfig() {
  std::ifstream ifs(std::string(GetConfigHome()) + "/" + APP_COMMON_NAME + "/config.json");
  std::string content(
     (std::istreambuf_iterator<char>(ifs) ),
     (std::istreambuf_iterator<char>()    )
  );

  return content;
}