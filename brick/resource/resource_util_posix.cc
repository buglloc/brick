// Copyright (c) 2015 The Brick Authors.

#include "brick/resource/resource_util.h"

#include "include/base/cef_logging.h"
#include "brick/helper.h"

namespace {

  const char kResourcesPath[] = "/desktop_app/internals/";

  bool FileExists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (f) {
      fclose(f);
      return true;
    }
    return false;
  }

  bool ReadFileToString(const char* path, std::string* data) {
    // Implementation adapted from base/file_util.cc
    FILE* file = fopen(path, "rb");
    if (!file)
      return false;

    char buf[1 << 16];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), file)) > 0)
      data->append(buf, len);
    fclose(file);

    return true;
  }

  std::string
  GetUrlWithoutQueryOrFragment(const std::string& url) {
    auto pos = url.find_first_of("?#");
    if (pos != std::string::npos)
      return url.substr(0, pos);

    return url;
  }

}  // namespace


namespace resource_util {
  bool
  LoadBinaryResource(const std::string& resource_dir, const char* resource_name, std::string* resource_data) {
    std::string path =
       resource_dir
        + "/web/"
        + resource_name;

    return ReadFileToString(path.c_str(), resource_data);
  }

  CefRefPtr<CefStreamReader>
  GetBinaryResourceReader(const std::string resource_dir, const char* resource_name) {
    std::string path =
       resource_dir
        + "/web/"
        + resource_name;

    if (!FileExists(path.c_str()))
      return NULL;

    return CefStreamReader::CreateForFile(path);
  }

  CefRefPtr<CefStreamReader>
  GetBinaryFileReader(const std::string& path) {
    if (!FileExists(path.c_str()))
      return NULL;

    return CefStreamReader::CreateForFile(path);
  }

  std::string
  UrlToResourcePath(const std::string& url) {
    if (url.find(kResourcesPath) == std::string::npos) {
      return url;
    }

    std::string file_name, mime_type;
    if (!helper::ParseUrl(url, &file_name, &mime_type)) {
      return url;
    }

    if (file_name.find(kResourcesPath) != 0U) {
      return url;
    }

    return file_name.substr(sizeof(kResourcesPath) - 2);
  }

  std::string
  GetMimeType(const std::string& url) {
    const std::string& path = GetUrlWithoutQueryOrFragment(url);
    return helper::GetMimeType(helper::GetFileExtension(path));
  }
}  // namespace resource_util
