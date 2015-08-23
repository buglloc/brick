// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_RESOURCE_TEMPORARY_PAGE_RESOURCE_PROVIDER_H_
#define BRICK_RESOURCE_TEMPORARY_PAGE_RESOURCE_PROVIDER_H_

#include <string>

#include "include/wrapper/cef_resource_manager.h"
#include "brick/client_handler.h"

class TemporaryPageResourceProvider : public CefResourceManager::Provider {
 public:
  TemporaryPageResourceProvider(const std::string& url_path, ClientHandler::TemporaryPageMap* pages);

  bool OnRequest(scoped_refptr<CefResourceManager::Request> request) OVERRIDE;

 private:
  std::string url_path_;
  ClientHandler::TemporaryPageMap* pages_;

  DISALLOW_COPY_AND_ASSIGN(TemporaryPageResourceProvider);
};

#endif  // BRICK_RESOURCE_TEMPORARY_PAGE_RESOURCE_PROVIDER_H_
