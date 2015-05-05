// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_CACHE_MANAGER_H_
#define BRICK_CACHE_MANAGER_H_
#pragma once

#include <string>

#include "include/cef_base.h"

class CacheManager : public CefBase {
 public:
  enum TYPE {
    DEFAULT = 0,
    BUDDY_ICON
  };

  CacheManager();
  ~CacheManager() {}

  void Init(const std::string& path);
  const std::string GetCacheDir(TYPE type = TYPE::DEFAULT);
  const std::string GetCachePath(const std::string& url, TYPE type = TYPE::DEFAULT);
  void CleanUpCache();

 protected:
  std::string cache_path_;

IMPLEMENT_REFCOUNTING(CacheManager);
};

#endif  // BRICK_CACHE_MANAGER_H_
