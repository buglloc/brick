// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_RESOURCE_DESKTOP_MEDIA_RESOURCE_PROVIDER_H_
#define BRICK_RESOURCE_DESKTOP_MEDIA_RESOURCE_PROVIDER_H_

#include <string>
#include <vector>

#include "include/wrapper/cef_resource_manager.h"
#include "brick/common/app_settings.h"

class DesktopMediaResourceProvider : public CefResourceManager::Provider {
 public:
  explicit DesktopMediaResourceProvider(const std::string& url_path);

  bool OnRequest(scoped_refptr<CefResourceManager::Request> request) OVERRIDE;

 private:
  static void GetPreviewOnUIThread(
      const std::string& media_type,
      int64 media_id,
      scoped_refptr<CefResourceManager::Request> request);
  static void SendPreviewOnIOThread(
      std::vector<unsigned char> preview,
      scoped_refptr<CefResourceManager::Request> request);
  static void SendErrorOnIOThread(
      scoped_refptr<CefResourceManager::Request> request);

  std::string url_path_;

  DISALLOW_COPY_AND_ASSIGN(DesktopMediaResourceProvider);
};

#endif  // BRICK_RESOURCE_DESKTOP_MEDIA_RESOURCE_PROVIDER_H_
