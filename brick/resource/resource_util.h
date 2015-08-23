// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_RESOURCE_RESOURCE_UTIL_H_
#define BRICK_RESOURCE_RESOURCE_UTIL_H_
#pragma once

#include <string>

#include "include/cef_stream.h"

namespace resource_util {
  // Retrieve a resource as a string.
  bool LoadBinaryResource(const std::string& resource_dir, const char* resource_name, std::string* resource_data);

// Retrieve a resource as a steam reader.
  CefRefPtr<CefStreamReader> GetBinaryResourceReader(const std::string resource_dir, const char* resource_name);
  CefRefPtr<CefStreamReader> GetBinaryFileReader(const std::string& path);
  std::string UrlToResourcePath(const std::string& url);
};  // namespace resource_util

#endif  // BRICK_RESOURCE_RESOURCE_UTIL_H_
