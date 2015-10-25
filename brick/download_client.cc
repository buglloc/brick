// Copyright (c) 2015 The Brick Authors.

#include "brick/download_client.h"

#include <ctime>
#include <fstream>

#include "include/cef_parser.h"
#include "include/base/cef_logging.h"
#include "brick/brick_types.h"
#include "brick/client_handler.h"
#include "brick/platform_util.h"
#include "brick/helper.h"

namespace {
  const char kTmpSuffix[] = ".brdown";
  const char kPageType[] = "text/html";
  const char kJsonType[] = "application/json";
  const char kDispositionHeaderName[] = "Content-Disposition";

  const int kHighPercentBound = 10 * 1024 * 1024;
  const int kMiddleBytesBound = 5 * 1024 * 1024;
  const int kHighBytesBound = 100 * 1024 * 1024;
  const int kLowPercentLimit = 5;
  const int kHighPercentLimit = 1;
  const int kLowBytesLimit = 512 * 1024;
  const int kMiddleBytesLimit = 1024 * 1024;
  const int kHighBytesLimit = 10 * 1024 * 1024;

  bool
  IsDownloadResponse(CefRefPtr<CefResponse> response) {
    // TODO(buglloc): checks Content-Disposition for all responses?
    const std::string mime_type = response->GetMimeType().ToString();

    if (mime_type.find(kPageType) == 0 || mime_type.find(kJsonType) == 0)
      return response->GetHeader(kDispositionHeaderName).ToString().find("attachment") == 0;

    return true;
  }

}  // namespace

DownloadClient::DownloadClient(const std::string& id, const std::string& path, const std::string& name)
    : id_ (id),
      file_path_ (path),
      file_name_ (name),
      last_percent_ (-1),
      last_bytes_ (-1) {

  CEF_REQUIRE_UI_THREAD();

  start_time_ = std::chrono::system_clock::now();
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
  DownloadClientStatus status;
  DownloadClientReason reason;

  if (request->GetRequestError() == ERR_NONE && request->GetResponse()->GetStatus() == 200) {
    if (!IsDownloadResponse(request->GetResponse())) {
      status = DC_STATUS_FAILED;
      reason = DC_REASON_NOT_ATTACHMENT;
      LOG(WARNING) << "Strange response type, not a file? ID: " << id_ << ". "
                   << "Url: " << request->GetRequest()->GetURL().ToString() << ". "
                   << "Type: " << request->GetResponse()->GetMimeType().ToString();

    } else if (rename(tmp_file_path_.c_str(), file_path_.c_str()) != 0) {
      status = DC_STATUS_FAILED;
      reason = DC_REASON_UNKNOWN;
      LOG(WARNING) << "Can't rename downloaded file. ID: " << id_ << ". "
                   << "Tmp path: " << tmp_file_path_ << "."
                   << "Result path: " << file_path_;

    } else {
      status = DC_STATUS_SUCCESS;
      reason = DC_REASON_NONE;
    }
  } else if (request->GetRequestError() == ERR_ABORTED) {
    status = DC_STATUS_FAILED;
    reason = DC_REASON_ABORTED;
    LOG(INFO) << "Download aborted. ID: " << id_ << ". "
              << "Url: " << request->GetRequest()->GetURL().ToString();

  } else {
    status = DC_STATUS_FAILED;
    reason = DC_REASON_HTTP;
    LOG(WARNING) << "Download failed. ID: " << id_ << ". "
                 << "Url: " << request->GetRequest()->GetURL().ToString() << "."
                 << "Reason: " << request_util::GetErrorString(request->GetRequestError());
  }

  if (status != DC_STATUS_SUCCESS) {
    unlink(tmp_file_path_.c_str());
  }

  DownloadCompleteEvent e(id_, file_path_, status, reason);
  EventBus::FireEvent(e);
}


void
DownloadClient::OnDownloadProgress(CefRefPtr<CefURLRequest> request, int64 current, int64 total) {
  bool percent_mode = (total > 0);

  int progress = 0;
  if (percent_mode) {
    progress = static_cast<int>(current * 100.0 / total);
    if (last_percent_ != -1) {
      const int delta_progress = progress - last_percent_;
      int limit;
      if (current < kHighPercentBound) {
        limit = kLowPercentLimit;
      } else {
        limit = kHighPercentLimit;
      }

      if (delta_progress < limit)
        return;
    }

    last_percent_ = progress;
  } else {
    if (last_bytes_ != -1) {
      const int64 delta_bytes = current - last_bytes_;
      int limit;
      if (current < kMiddleBytesBound) {
        limit = kLowBytesLimit;
      } else if (current < kHighBytesBound) {
        limit = kMiddleBytesLimit;
      } else {
        limit = kHighBytesLimit;
      }

      if (delta_bytes < limit)
        return;
    }

    last_bytes_ = current;
  }

  const auto diff_time = std::chrono::system_clock::now() - start_time_;
  const auto time_elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(diff_time).count();

  int64 speed = 0;
  if (time_elapsed > 0) {
    speed = static_cast<int64>(current * 1000 / time_elapsed);
  }

  DownloadProgressEvent e(id_, progress, speed, current, total);
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

  if (!platform_util::MakeDirectory(helper::BaseDir(path)))
    return NULL;

// TODO(buglloc): What we must todo in this case?
//  if (platform_util::IsPathExists(path + kTmpSuffix))
//    return NULL;  // Probably we already started downloading

  CefRefPtr<CefRequest> request = CefRequest::Create();
  request->SetURL(url);
  request->SetMethod("GET");
  request->SetFlags(UR_FLAG_ALLOW_CACHED_CREDENTIALS|UR_FLAG_NO_RETRY_ON_5XX|UR_FLAG_SKIP_CACHE);

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
  std::string id = std::to_string(helper::HashString(url_));

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
