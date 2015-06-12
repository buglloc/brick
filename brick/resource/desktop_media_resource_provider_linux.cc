// Copyright (c) 2015 The Brick Authors.

#include "brick/resource/desktop_media_resource_provider.h"

#include "include/cef_base.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "brick/helper.h"
#include "brick/resource/resource_util.h"
#include "brick/desktop_media.h"


namespace {
  const char kMimeType[] = "image/png";
  const char* kErrorContent = "Failed to load resource";
}  // namespace

DesktopMediaResourceProvider::DesktopMediaResourceProvider(const std::string& url_path):
    url_path_(url_path) {

  DCHECK(!url_path_.empty());
}

bool
DesktopMediaResourceProvider::OnRequest(scoped_refptr<CefResourceManager::Request> request) {
  CEF_REQUIRE_IO_THREAD();

  const std::string& url = request->url();
  if (url.find(url_path_) != 0U) {
    return false;
  }

  size_t delimiter = url.find('-', url_path_.length());
  if (delimiter == std::string::npos) {
    return false;
  }

  std::string type = url.substr(url_path_.length(), delimiter - url_path_.length());
  int64 id = atoi(url.substr(delimiter + 1).c_str());

  CefPostTask(TID_UI,
              base::Bind(&DesktopMediaResourceProvider::GetPreviewOnUIThread, type, id, request));
  return true;
}

void
DesktopMediaResourceProvider::GetPreviewOnUIThread(
  const std::string& media_type,
  int64 media_id,
  scoped_refptr<CefResourceManager::Request> request) {
  CEF_REQUIRE_UI_THREAD();

  std::vector<unsigned char> preview;
  if (desktop_media::GetMediaPreview(media_type, media_id, &preview)) {
    CefPostTask(TID_IO,
                base::Bind(&DesktopMediaResourceProvider::SendPreviewOnIOThread, preview, request));
    preview.clear();
  }
  CefPostTask(TID_IO,
              base::Bind(&DesktopMediaResourceProvider::SendErrorOnIOThread, request));
}

void
DesktopMediaResourceProvider::SendPreviewOnIOThread(
  std::vector<unsigned char> preview,
  scoped_refptr<CefResourceManager::Request> request) {
  CEF_REQUIRE_IO_THREAD();

  CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForData(
    static_cast<void*>(preview.data()),
    preview.size()
  );
  ASSERT(stream.get());

  request->Continue(new CefStreamResourceHandler(
      kMimeType,
      stream
  ));
}

void
DesktopMediaResourceProvider::SendErrorOnIOThread(
    scoped_refptr<CefResourceManager::Request> request) {
  CEF_REQUIRE_IO_THREAD();

  CefResponse::HeaderMap header_map;
  CefRefPtr<CefStreamReader> stream =
      CefStreamReader::CreateForData(
          static_cast<void*>(const_cast<char*>(kErrorContent)),
          strlen(kErrorContent)
      );

  ASSERT(stream.get());
  request->Continue(new CefStreamResourceHandler(404, "Not Found", kMimeType, header_map, stream));
}