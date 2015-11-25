// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_RESOURCE_SETTINGS_RESOURCE_PROVIDER_H_
#define BRICK_RESOURCE_SETTINGS_RESOURCE_PROVIDER_H_

#include <string>

#include "include/wrapper/cef_resource_manager.h"
#include "brick/client_handler.h"

class SettingsResourceProvider : public CefResourceManager::Provider {
 public:
  explicit SettingsResourceProvider(const std::string& url_path);

  bool OnRequest(scoped_refptr<CefResourceManager::Request> request) OVERRIDE;

 private:
  const std::string url_path_;

  DISALLOW_COPY_AND_ASSIGN(SettingsResourceProvider);
};

#endif  // BRICK_RESOURCE_SETTINGS_RESOURCE_PROVIDER_H_
