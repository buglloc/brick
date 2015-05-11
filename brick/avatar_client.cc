// Copyright (c) 2015 The Brick Authors.

#include "brick/avatar_client.h"

#include <fstream>

#include "include/base/cef_logging.h"
#include "brick/platform_util.h"
#include "brick/helper.h"

namespace {
  const char kTmpSuffix[] = ".tmp";
}  // namespace

AvatarClient::AvatarClient(const Callback& callback, const std::string& path)
    : callback_(callback),
      file_path_(path) {

  CEF_REQUIRE_UI_THREAD();
  DCHECK(!callback_.is_null());
  tmp_file_path_ = path + kTmpSuffix;
  tmp_file_.open(tmp_file_path_, std::ofstream::binary);
}

void
AvatarClient::Detach() {
  CEF_REQUIRE_UI_THREAD();
  if (!callback_.is_null())
    callback_.Reset();

  tmp_file_.close();
}

void
AvatarClient::OnRequestComplete(CefRefPtr<CefURLRequest> request) {
  CEF_REQUIRE_UI_THREAD();
  if (callback_.is_null())
    return;

  tmp_file_.close();
  if (request->GetRequestError() == ERR_NONE && request->GetResponse()->GetStatus() == 200) {
    rename(tmp_file_path_.c_str(), file_path_.c_str());
    callback_.Run(true);
  } else {
    LOG(WARNING) << "Can't download notification icon '"
                 << request->GetRequest()->GetURL().ToString()
                 << "' to file '" << file_path_ << "'";

    callback_.Run(false);
  }

  callback_.Reset();
}

void
AvatarClient::OnDownloadData(CefRefPtr<CefURLRequest> request,
                           const void* data,
                           size_t data_length) {

  CEF_REQUIRE_UI_THREAD();
  tmp_file_.write(static_cast<const char*>(data), data_length);
}

// static methods

CefRefPtr<CefURLRequest>
AvatarClient::CreateRequest(
    const Callback& callback,
    const std::string& url,
    const std::string& path) {

  CEF_REQUIRE_UI_THREAD();

  if (platform_util::IsPathExists(path + kTmpSuffix))
    return NULL;  // Probably we already started downloading for current avatar

  if (!platform_util::MakeDirectory(helper::BaseDir(path)))
    return NULL;

  CefRefPtr<CefRequest> request = CefRequest::Create();
  request->SetURL(url);
  request->SetMethod("GET");
  request->SetFlags(UR_FLAG_NO_RETRY_ON_5XX);

  // Create and start the new CefURLRequest.
  return CefURLRequest::Create(request, new AvatarClient(callback, path));
}
