// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_DOWNLOAD_CLIENT_H_
#define BRICK_DOWNLOAD_CLIENT_H_
#pragma once

#include <string>
#include <fstream>

#include "include/wrapper/cef_helpers.h"
#include "include/cef_urlrequest.h"
#include "include/cef_dialog_handler.h"

class DownloadClientDialogCallback : public CefFileDialogCallback {
public:
  DownloadClientDialogCallback(std::string url);
  virtual void Continue(int selected_accept_filter,
                        const std::vector<CefString>& file_paths) OVERRIDE;

  virtual void Cancel() OVERRIDE {
  }

private:
  std::string url_;

IMPLEMENT_REFCOUNTING(DownloadClientDialogCallback);
};


class DownloadClient : public CefURLRequestClient {
public:

  DownloadClient(const std::string& id, const std::string& path, const std::string& name);

  void Detach();

  void OnRequestComplete(CefRefPtr<CefURLRequest> request) OVERRIDE;
  void OnDownloadProgress(CefRefPtr<CefURLRequest> request, int64 current, int64 total) OVERRIDE;
  void OnDownloadData(CefRefPtr<CefURLRequest> request, const void* data, size_t data_length) OVERRIDE;

  void OnUploadProgress(CefRefPtr<CefURLRequest> request, int64 current, int64 total) OVERRIDE {
  }

  bool GetAuthCredentials(bool isProxy,
                          const CefString& host,
                          int port,
                          const CefString& realm,
                          const CefString& scheme,
                          CefRefPtr<CefAuthCallback> callback) OVERRIDE {
    return false;
  }

  static CefRefPtr<CefURLRequest> CreateRequest(const std::string& id,
                                                const std::string& url,
                                                const std::string& path,
                                                const std::string& name);

private:
  std::string id_;
  std::string file_path_;
  std::string tmp_file_path_;
  std::string file_name_;
  std::ofstream tmp_file_;
  int last_percent_;
  int64 last_bytes_;

IMPLEMENT_REFCOUNTING(DownloadClient);
};

#endif  // BRICK_DOWNLOAD_CLIENT_H_
