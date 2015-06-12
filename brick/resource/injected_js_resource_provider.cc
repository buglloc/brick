// Copyright (c) 2015 The Brick Authors.

#include "brick/resource/injected_js_resource_provider.h"

#include "include/wrapper/cef_stream_resource_handler.h"
#include "brick/helper.h"
#include "brick/resource/resource_util.h"


namespace {
  const char kMimeType[] = "application/javascript";
}  // namespace

InjectedJsResourceProvider::InjectedJsResourceProvider(
    const std::string& url_path,
    AppSettings::client_scripts_map* scripts):
    url_path_(url_path),
    scripts_(scripts) {

  DCHECK(!url_path_.empty());
  DCHECK(scripts_);
}

bool
InjectedJsResourceProvider::OnRequest(scoped_refptr<CefResourceManager::Request> request) {
  CEF_REQUIRE_IO_THREAD();

  const std::string& url = request->url();
  if (url.find(url_path_) != 0U) {
    return false;
  }

  const std::string& file_path = GetFilePath(url);
  if (file_path.empty()) {
    return false;
  }

  CefRefPtr<CefStreamReader> stream = resource_util::GetBinaryFileReader(file_path);
  ASSERT(stream.get());

  request->Continue(new CefStreamResourceHandler(
      kMimeType,
      stream
  ));

  return true;
}

std::string
InjectedJsResourceProvider::GetFilePath(const std::string& url) {
  std::string path_part = url.substr(url_path_.length());
  if (scripts_->count(path_part)) {
        // If we found our client script - load it :)
      return scripts_->at(path_part);
  }

  return "";
}
