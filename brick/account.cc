// Copyright (c) 2015 The Brick Authors.

#include "brick/account.h"

#include <unistd.h>

#include "include/cef_parser.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "brick/auth_client.h"
#include "brick/request_util.h"

namespace {
  const char kFakeId = -1;
  const int kAuthTimeout = 6;
  const char kDefaultAppUrl[] = "/desktop_app/";
}

Account::Account() {
  id_ = kFakeId;
  login_ = "";
  password_ = "";
  domain_ = "";
  secure_ = true;
  label_ = "";
  base_url_ = "";
  use_app_password_ = true;
}

Account::~Account() {
  CancelAuthPending(false);
}

bool
Account::IsExisted() {
  return id_ != kFakeId;
}

bool
Account::IsSecure() {
  return secure_;
}

bool
Account::IsAppPasswordUsed() {
  return use_app_password_;
}

int
Account::GetId() {
  return id_;
}

void
Account::SetId(int id) {
  id_ = id;
}

std::string
Account::GetLogin() {
  return login_;
}

std::string
Account::GetDomain() {
  return domain_;
}

std::string
Account::GetPassword() {
  /**
  * ToDo: Нужно передалать работу с паролем на корню:
  *  1. Хранить в памяти пароль как можно меньше времени
  *  2. Занулять область памяти где хранился пароль после его использования
  *  3. Сделать поддержку GNOME Keyring и KWallet. И только при их отсутствии самому хранить пароль
  *  4. При самостоятельном хранении паролей их нужно чемнить зашифровать (libsodium?).
  *  5. Ключ для шифрования/дешифрования должен каждый раз читаться из файла и следовать правилу из пунктов 1 и 2
  **/
  return password_;
}

std::string
Account::GetOrigin() {
  return (secure_ ? "https://" : "http://" ) + domain_;
}

std::string
Account::GetBaseUrl() {
  return base_url_;
}

std::string
Account::GetAuthUrl() {
  return base_url_ + "login/";
}

std::string
Account::GetLabel() {
  return label_;
}

bool
Account::CheckBaseUrl(const std::string& url) {
  return (url.find(base_url_) == 0);
}

std::string
Account::GenLabel() {
  return (
     domain_ + "/" + login_
  );
}

void
Account::SetLogin(std::string login) {
  login_ = login;
  label_ = GenLabel();
}

void
Account::SetPassword(std::string password) {
  password_ = password;
}

void
Account::SetDomain(std::string domain) {
  domain_ = domain;
  label_ = GenLabel();
  base_url_ = GenBaseUrl();
}

void
Account::SetSecure(bool is_secure) {
  secure_ = is_secure;
  base_url_ = GenBaseUrl();
}

void
Account::SetUseAppPassword(bool use) {
  use_app_password_ = use;
}

void
Account::Set(
    bool secure,
    std::string domain,
    std::string login,
    std::string password,
    bool use_app_password) {

  secure_ = secure;
  domain_ = domain;
  login_ = login;
  password_ = password;
  label_ = GenLabel();
  base_url_ = GenBaseUrl();
  use_app_password_ = use_app_password;
}

std::string
Account::GenBaseUrl() {
  // ToDo: Need option here?
  return (GetOrigin() + kDefaultAppUrl);
}

void
Account::Auth(bool renew_password, const AuthCallback& callback, const std::string& otp) {
  CEF_REQUIRE_UI_THREAD();

  CancelAuthPending(false);

  callback_ = callback;
  urlrequest_ = AuthClient::CreateRequest(
      base::Bind(&Account::OnAuthComplete, this),
      this,
      otp,
      renew_password);

  // Cancel auth request after timeout
  CefPostDelayedTask(
      TID_UI,
      base::Bind(&Account::OnAuthTimedOut, this, urlrequest_.get()),
      kAuthTimeout * 1000);
}

void
Account::OnAuthComplete(const AuthResult auth_result, const std::string& new_password) {
  CEF_REQUIRE_UI_THREAD();

  if (!new_password.empty() && password_ != new_password) {
    password_ = new_password;
  }

  callback_.Run(this, auth_result);
  callback_.Reset();
  urlrequest_ = NULL;
}

void
Account::CancelAuthPending(bool call_callback) {
  CEF_REQUIRE_UI_THREAD();

  if (!urlrequest_.get())
    return;

  // Don't execute the callback when we explicitly cancel the request.
  static_cast<AuthClient*>(urlrequest_->GetClient().get())->Detach();

  urlrequest_->Cancel();
  urlrequest_ = NULL;

  LOG(WARNING) << "Auth failed (canceled)";

  if (!callback_.is_null()) {
    if (call_callback) {
      AuthResult auth_result;
      auth_result.success = false;
      auth_result.error_code = ERROR_CODE::HTTP;
      auth_result.http_error = "ERR_CANCELED";

      callback_.Run(this, auth_result);
    }

    callback_.Reset();
  }
}

void
Account::OnAuthTimedOut(const CefURLRequest *urlrequest) {
  CEF_REQUIRE_UI_THREAD();

  if (!urlrequest_.get() || urlrequest_.get() != urlrequest)
    return;

  // Don't execute the callback when we explicitly cancel the request.
  static_cast<AuthClient*>(urlrequest_->GetClient().get())->Detach();

  urlrequest_->Cancel();
  urlrequest_ = NULL;

  LOG(WARNING) << "Auth failed (timeout of " << kAuthTimeout << "s reached)";

  if (!callback_.is_null()) {
    AuthResult auth_result;
    auth_result.success = false;
    auth_result.error_code = ERROR_CODE::HTTP;
    auth_result.http_error = request_util::GetErrorString(ERR_TIMED_OUT);

    callback_.Run(this, auth_result);
    callback_.Reset();
  }
}
