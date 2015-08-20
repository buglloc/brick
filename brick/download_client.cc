// Copyright (c) 2015 The Brick Authors.

#include "brick/download_client.h"

#include <ctime>
#include <fstream>
#include <include/cef_parser.h>

#include "include/base/cef_logging.h"
#include "brick/brick_types.h"
#include "brick/client_handler.h"
#include "brick/platform_util.h"
#include "brick/helper.h"

namespace {
  const char kTmpSuffix[] = ".brdown";

}  // namespace

DownloadClient::DownloadClient(const std::string& id, const std::string& path, const std::string& name)
    : id_ (id),
      file_path_ (path),
      file_name_ (name),
      last_percent_ (0) {

  CEF_REQUIRE_UI_THREAD();
  tmp_file_path_ = path + kTmpSuffix;
  tmp_file_.open(tmp_file_path_, std::ofstream::binary);
}

void
DownloadClient::Detach() {
  CEF_REQUIRE_UI_THREAD();

  tmp_file_.close();
}

void
DownloadClient::OnRequestComplete(CefRefPtr<CefURLRequest> request) {
  CEF_REQUIRE_UI_THREAD();

  tmp_file_.close();
  DownloadClientStatus status = DC_STATUS_FAILED;
  DownloadClientReason reason = DC_REASON_UNKNOWN;
  if (request->GetRequestError() == ERR_NONE && request->GetResponse()->GetStatus() == 200) {
    if (rename(tmp_file_path_.c_str(), file_path_.c_str()) == 0) {
      status = DC_STATUS_SUCCESS;
      reason = DC_REASON_NONE;
    } else {
      reason = DC_REASON_UNKNOWN;
      LOG(WARNING) << "Can't rename downloaded file. ID: " << id_ << ". "
                                << "Tmp path: " << tmp_file_path_ << "."
                                << "Result path: " << file_path_ ;
    }
  } else if (request->GetRequestError() == ERR_ABORTED) {
    reason = DC_REASON_ABORTED;
    unlink(tmp_file_path_.c_str());
    LOG(INFO) << "Download aborted. ID: " << id_ << ". "
                 << "Url: " << request->GetRequest()->GetURL().ToString();
  } else {
    reason = DC_REASON_HTTP;
    LOG(WARNING) << "Download failed. ID: " << id_ << ". "
                 << "Url: " << request->GetRequest()->GetURL().ToString() << "."
                 << "Reason: " << request_util::GetErrorString(request->GetRequestError());
  }

  DownloadCompleteEvent e(id_, file_path_, status, reason);
  EventBus::FireEvent(e);
}


void
DownloadClient::OnDownloadProgress(CefRefPtr<CefURLRequest> request, int64 current, int64 total) {
  if (total <= 0)
    return;
  // ToDo: what should be without total size?
  // Send only "current" w/o "percent" and "total"? How often?

  int progress = static_cast<int>(current * 100.0 / total);
  if (progress == last_percent_) {
    return;
  }

  last_percent_ = progress;
  DownloadProgressEvent e(id_, progress, current, total);
  EventBus::FireEvent(e);
}

void
DownloadClient::OnDownloadData(CefRefPtr<CefURLRequest> request,
                             const void* data,
                             size_t data_length) {

  CEF_REQUIRE_UI_THREAD();
  tmp_file_.write(static_cast<const char*>(data), data_length);
}

// static methods

CefRefPtr<CefURLRequest>
DownloadClient::CreateRequest(
    const std::string& id,
    const std::string& url,
    const std::string& path,
    const std::string& name) {

  CEF_REQUIRE_UI_THREAD();

// ToDo: What we must todo in this case?
//  if (platform_util::IsPathExists(path + kTmpSuffix))
//    return NULL;  // Probably we already started downloading

  CefRefPtr<CefRequest> request = CefRequest::Create();
  request->SetURL(url);
  request->SetMethod("GET");
  request->SetFlags(UR_FLAG_ALLOW_CACHED_CREDENTIALS|UR_FLAG_NO_RETRY_ON_5XX|UR_FLAG_STOP_ON_REDIRECT|UR_FLAG_SKIP_CACHE);

  // Create and start the new CefURLRequest.
  return CefURLRequest::Create(request, new DownloadClient(id, path, name), NULL);
}


DownloadClientDialogCallback::DownloadClientDialogCallback(std::string url)
  : url_ (url) {

}

void
DownloadClientDialogCallback::Continue(
    int selected_accept_filter,
    const std::vector<CefString>& file_paths) {

  if (file_paths.empty())
    return;

  std::string file = file_paths.cbegin()->ToString();
  std::string filename = helper::GetFileName(file);
  std::string id = std::to_string(helper::HashString(url_ + file));

  CefRefPtr<CefURLRequest> request = DownloadClient::CreateRequest(
      id,
      url_,
      file,
      filename
  );

  if (request.get()) {
    ClientHandler::GetInstance()->RegisterDownload(id , request);
    DownloadStartEvent e(id, filename, file, url_, std::time(nullptr));
    EventBus::FireEvent(e);
  }
}