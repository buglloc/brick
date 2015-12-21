// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_ACCOUNT_H_
#define BRICK_ACCOUNT_H_
#pragma once

#include <string>

#include "include/cef_urlrequest.h"
#include "include/cef_base.h"
#include "include/base/cef_callback.h"
#include "brick/request_util.h"

class Account : public CefBase {

 public:
  enum ERROR_CODE {
  // Must be synced with desktop_extension.js!
    NONE = 0,
    HTTP,
    CAPTCHA,
    OTP,
    AUTH,
    INVALID_URL,
    UNKNOWN,
    N_AUTH_ERROR
  };

  typedef struct {
    bool success;
    std::string bitrix_sessid;
    ERROR_CODE error_code;
    std::string http_error;
    request_util::CookiesMap cookies;
  } AuthResult;

  typedef base::Callback<void(const CefRefPtr<Account>, AuthResult)> AuthCallback;

  Account();
  ~Account();

  int GetId() const;
  std::string GetLogin() const;
  std::string GetPassword() const;
  std::string GetDomain() const;
  std::string GetLabel() const;
  std::string GetOrigin() const;
  std::string GetBaseUrl() const;
  std::string GetAuthUrl() const;
  bool IsExisted() const;
  bool IsSecure() const;
  bool IsAppPasswordUsed() const;
  bool CheckBaseUrl(const std::string& url);

  void Set(bool secure, std::string domain, std::string login, std::string password, bool use_app_password);
  void SetLogin(std::string login);
  void SetPassword(std::string password);
  void SetDomain(std::string domain);
  void SetSecure(bool is_secure);
  void SetUseAppPassword(bool use);
  void SetId(int id);

  std::string GenLabel();
  std::string GenBaseUrl();

  void Auth(bool renew_password, const AuthCallback& callback, const std::string& otp = "");
  void OnAuthComplete(const AuthResult auth_result, const std::string& new_password);
  void OnAuthTimedOut(const CefURLRequest* urlrequest);
  void CancelAuthPending(bool call_callback);

 protected:
  int id_;
  std::string login_;
  std::string password_;
  std::string domain_;
  std::string base_url_;
  std::string label_;
  bool secure_;
  bool use_app_password_;
  AuthCallback callback_;
  CefRefPtr<CefURLRequest> urlrequest_;

  IMPLEMENT_REFCOUNTING(Account);
};

#endif  // BRICK_ACCOUNT_H_
