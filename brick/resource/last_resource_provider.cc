// Copyright (c) 2015 The Brick Authors.

#include "brick/resource/last_resource_provider.h"

#include "include/wrapper/cef_stream_resource_handler.h"
#include "brick/helper.h"


namespace {
  const char* kBodyContent = "Failed to load resource";
}  // namespace

bool
LastResourceProvider::OnRequest(scoped_refptr<CefResourceManager::Request> request) {
  CEF_REQUIRE_IO_THREAD();
  LOG(WARNING) << "Intercepted resource \"" << request->url() << "\" was not found.";

  CefResponse::HeaderMap header_map;
  CefRefPtr<CefStreamReader> stream =
     CefStreamReader::CreateForData(
        static_cast<void*>(const_cast<char*>(kBodyContent)),
        strlen(kBodyContent)
     );

  ASSERT(stream.get());
  request->Continue(new CefStreamResourceHandler(404, "Not Found", "text/plain", header_map, stream));
  return true;
}