// Copyright (c) 2015 The Brick Authors.

#include <ftw.h>
#include <unistd.h>

#include "include/cef_url.h"
#include "brick/cache_manager.h"
#include "brick/brick_app.h"
#include "brick/helper.h"

namespace {
  time_t old_file_time;

  const std::string cache_dir_[] = {
     "other",
     "buddy"
  };

  int
  RemoveOldFile(const char *path, const struct stat *sb, int type) {
    if (type == FTW_F) {
      struct stat stat_data;
      if (stat(path, &stat_data) != -1) {
        if (stat_data.st_atim.tv_sec < old_file_time)
          unlink(path);
      }
    }

    // tell ftw to continue
    return 0;
  }
}  // namespace

CacheManager::CacheManager()
    : cache_path_("") {

}

void
CacheManager::Init(const std::string& path) {
  cache_path_ = path;
}

const std::string
CacheManager::GetCacheDir(TYPE type) {
  if (cache_path_.empty()) {
    cache_path_ = std::string(BrickApp::GetCacheHome()) + "/" + APP_COMMON_NAME + "/app/";
  }

  return cache_path_ + cache_dir_[type];
}

const std::string
CacheManager::GetCachePath(const std::string& url, TYPE type) {
  std::string result = GetCacheDir(type);
  std::string hash = std::to_string(helper::HashString(url));
  CefURLParts parts;
  CefParseURL(url, parts);

  if (parts.host.length) {
    std::string host = CefString(&parts.host);
    result += "/" + host;
  }

  result  += "/" + hash.substr(0, 1) + "/" + hash;

  if (parts.path.length) {
    std::string file_path = CefString(&parts.path);
    std::string ext = helper::GetFileExtension(file_path);
    if (!ext.empty()) {
      result += "." + ext;
    }
  }

  return result;
}

void CacheManager::CleanUpCache() {
  struct timeval now;
  gettimeofday(&now, NULL);
  old_file_time = now.tv_sec - 86400 * 31;
  ftw(cache_path_.c_str(), RemoveOldFile, FTW_F);
}
