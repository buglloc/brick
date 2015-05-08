// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_AVATAR_CLIENT_H_
#define BRICK_AVATAR_CLIENT_H_
#pragma once

#include <string>
#include <fstream>

#include "include/wrapper/cef_helpers.h"
#include "include/base/cef_callback.h"
#include "include/cef_urlrequest.h"

class AvatarClient : public CefURLRequestClient {
 public:
  // Callback to be executed on request completion.
  typedef base::Callback<void(bool)> Callback;

  AvatarClient(const Callback& callback, const std::string& path);

  void Detach();

  void OnRequestComplete(CefRefPtr<CefURLRequest> request) OVERRIDE;

  void OnUploadProgress(CefRefPtr<CefURLRequest> request, int64 current, int64 total) OVERRIDE {
  }

  void OnDownloadProgress(CefRefPtr<CefURLRequest> request, int64 current, int64 total) OVERRIDE {
  }

  void OnDownloadData(CefRefPtr<CefURLRequest> request, const void* data, size_t data_length) OVERRIDE;

  bool GetAuthCredentials(bool isProxy,
                          const CefString& host,
                          int port,
                          const CefString& realm,
                          const CefString& scheme,
                          CefRefPtr<CefAuthCallback> callback) OVERRIDE {
    return false;
  }

  static CefRefPtr<CefURLRequest> CreateRequest(const Callback&  callback,
                                                const std::string& url,
                                                const std::string& path);

 private:
  Callback callback_;
  std::string file_path_;
  std::string tmp_file_path_;
  std::ofstream tmp_file_;

IMPLEMENT_REFCOUNTING(AvatarClient);
DISALLOW_COPY_AND_ASSIGN(AvatarClient);
};

#endif  // BRICK_AVATAR_CLIENT_H_
