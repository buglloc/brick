#ifndef BRICK_ACCOUNT_H_
#define BRICK_ACCOUNT_H_
#pragma once

#include <string>
#include <include/cef_base.h>
#include "httpclient/httpclient.h"

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

  typedef struct
  {
    bool success;
    ERROR_CODE error_code;
    std::string http_error;
    HttpClient::cookie_map cookies;
  } AuthResult;

  Account();
  ~Account();

  int GetId();
  std::string GetLogin();
  std::string GetPassword();
  std::string GetDomain();
  std::string GetBaseUrl();
  std::string GetLabel();
  bool IsExisted();
  bool IsSecure();
  bool IsAppPasswordUsed();
  bool CheckBaseUrl(std::string url);

  void Set(bool secure, std::string domain, std::string login, std::string password);
  void SetLogin(std::string login);
  void SetPassword(std::string password);
  void SetDomain(std::string domain);
  void SetSecure(bool is_secure);
  void SetUseAppPassword(bool use);
  void SetId(int id);

  std::string GenLabel();
  std::string GenBaseUrl();

  AuthResult Auth(bool renew_password = false, std::string otp = "");

protected:
  int id_;
  std::string login_;
  std::string password_;
  std::string domain_;
  std::string base_url_;
  std::string label_;
  bool secure_;
  bool use_app_password_;

  std::string TryParseApplicationPassword(std::string body);
  std::string GetOsMark();

  IMPLEMENT_REFCOUNTING(Account);
};

#endif // BRICK_ACCOUNT_H_
