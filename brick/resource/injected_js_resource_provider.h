// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_RESOURCE_INJECTED_JS_RESOURCE_PROVIDER_H_
#define BRICK_RESOURCE_INJECTED_JS_RESOURCE_PROVIDER_H_

#include <string>

#include "include/wrapper/cef_resource_manager.h"
#include "brick/common/app_settings.h"

class InjectedJsResourceProvider : public CefResourceManager::Provider {
 public:
  InjectedJsResourceProvider(const std::string& url_path, AppSettings::client_scripts_map* scripts);

  bool OnRequest(scoped_refptr<CefResourceManager::Request> request) OVERRIDE;

 private:
  std::string GetFilePath(const std::string& url);

  std::string url_path_;
  AppSettings::client_scripts_map* scripts_;

  DISALLOW_COPY_AND_ASSIGN(InjectedJsResourceProvider);
};

#endif  // BRICK_RESOURCE_INJECTED_JS_RESOURCE_PROVIDER_H_
