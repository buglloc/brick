// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_AUTH_CLIENT_H_
#define BRICK_AUTH_CLIENT_H_
#pragma once

#include <string>

#include "include/wrapper/cef_helpers.h"
#include "include/base/cef_callback.h"
#include "include/cef_urlrequest.h"
#include "brick/account.h"

class AuthClient : public CefURLRequestClient {
 public:
  // Callback to be executed on request completion.
  typedef base::Callback<void(const Account::AuthResult, const std::string&)> Callback;

  AuthClient(const Callback& callback, const std::string& url);

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
                                                const CefRefPtr<Account> account,
                                                const std::string& otp,
                                                bool renew);

  static CefRefPtr<CefURLRequest> CreateRequest(const Callback&  callback,
                                                const std::string& url,
                                                const std::string& login,
                                                const std::string& password,
                                                const std::string& otp,
                                                bool renew);

 private:
  static CefRefPtr<CefURLRequest> CreateRequest(const Callback&  callback,
                                                const std::string& url,
                                                request_util::PostFormMap form);

  Callback callback_;
  std::string url_;
  std::string body_;

  IMPLEMENT_REFCOUNTING(AuthClient);
  DISALLOW_COPY_AND_ASSIGN(AuthClient);
};


#endif  // BRICK_AUTH_CLIENT_H_
