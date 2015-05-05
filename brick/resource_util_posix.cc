// Copyright (c) 2015 The Brick Authors.

#include "brick/resource_util.h"

namespace {

  bool FileExists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (f) {
      fclose(f);
      return true;
    }
    return false;
  }

  bool ReadFileToString(const char* path, std::string& data) {
    // Implementation adapted from base/file_util.cc
    FILE* file = fopen(path, "rb");
    if (!file)
      return false;

    char buf[1 << 16];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), file)) > 0)
      data.append(buf, len);
    fclose(file);

    return true;
  }

}  // namespace

bool
LoadBinaryResource(const std::string resource_dir, const char* resource_name, std::string& resource_data) {
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
GetBinaryFileReader(std::string path) {
  if (!FileExists(path.c_str()))
    return NULL;

  return CefStreamReader::CreateForFile(path);
}
