// Copyright (c) 2015 The Brick Authors.

#include "brick/resource/settings_resource_provider.h"

#include <string>

#include "include/wrapper/cef_stream_resource_handler.h"
#include "third-party/json/json.h"
#include "brick/platform_util.h"


namespace {
  const char kMimeType[] = "application/json";
  const char kSettingAutostartName[] = "autostart";
}  // namespace

SettingsResourceProvider::SettingsResourceProvider(const std::string& url_path)
    : url_path_(url_path) {

  DCHECK(!url_path_.empty());
}

bool
SettingsResourceProvider::OnRequest(scoped_refptr<CefResourceManager::Request> request) {
  CEF_REQUIRE_IO_THREAD();

  const std::string& url = request->url();
  if (url.find(url_path_) != 0U) {
    return false;
  }

  std::string name;
  auto ext_dot = url.rfind('.');
  if (ext_dot != std::string::npos && ext_dot > url_path_.length()) {
    name = url.substr(url_path_.length(), ext_dot - url_path_.length());
  } else {
    name = url.substr(url_path_.length());
  }

  Json::Value result;
  if (name == kSettingAutostartName) {
    result = Json::Value(platform_util::IsAutostartEnabled());

  } else {
    return false;
  }

  Json::FastWriter json_writer;
  json_writer.omitEndingLineFeed();
  const std::string& content = json_writer.write(result);
  CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForData(
      static_cast<void*>(const_cast<char*>(content.c_str())),
      content.size()
  );
  ASSERT(stream.get());

  request->Continue(new CefStreamResourceHandler(
      kMimeType,
      stream
  ));

  return true;
}
