// Copyright (c) 2015 The Brick Authors.

#include "brick/resource/temporary_page_resource_provider.h"

#include "include/wrapper/cef_stream_resource_handler.h"
#include "brick/helper.h"
#include "brick/resource/resource_util.h"

namespace {
  const char kMimeType[] = "text/html";
}  // namespace

TemporaryPageResourceProvider::TemporaryPageResourceProvider(
    const std::string& url_path,
    ClientHandler::TemporaryPageMap* pages):
    url_path_(url_path),
    pages_(pages) {

  DCHECK(!url_path_.empty());
  DCHECK(pages_);
}

bool
TemporaryPageResourceProvider::OnRequest(scoped_refptr<CefResourceManager::Request> request) {
  CEF_REQUIRE_IO_THREAD();

  const std::string& url = request->url();
  if (url.find(url_path_) != 0U) {
    return false;
  }

  std::string path_part = url.substr(url_path_.length());
  if (!pages_->count(path_part)) {
    return false;
  }

  const std::string& content = pages_->at(path_part);
  CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForData(
    static_cast<void*>(const_cast<char*>(content.c_str())),
    content.size()
  );

  pages_->erase(path_part);
  ASSERT(stream.get());

  request->Continue(new CefStreamResourceHandler(
      kMimeType,
      stream
  ));

  return true;
}