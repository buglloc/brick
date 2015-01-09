#include <third-party/httpclient/httpclient.h>
#include "account.h"
#include "include/base/cef_logging.h"

namespace {
    const char fake_id = -1;

}

Account::Account() {
  id_ = fake_id;
  login_ = "";
  password_ = "";
  domain_ = "";
  secure_ = true;
  label_ = "";
  base_url_ = "";
}

Account::~Account() {
}

bool
Account::IsExisted() {
  return id_ != fake_id;
}

bool
Account::IsSecure() {
  return secure_;
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
  return password_;
}

std::string
Account::GetBaseUrl() {
  return base_url_;
}

std::string
Account::GetLabel() {
  return label_;
}

bool
Account::CheckBaseUrl(std::string url) {
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
Account::Set(
   bool secure,
   std::string domain,
   std::string login,
   std::string password) {

  secure_ = secure;
  domain_ = domain;
  login_ = login;
  password_ = password;
  label_ = GenLabel();
  base_url_ = GenBaseUrl();
}

std::string
Account::GenBaseUrl() {
  return (
     (secure_ ? "https://" : "http://" )
      + domain_
      + "/desktop_app/" // ToDo: Need option here?
  );
}

Account::AuthResult
Account::Auth() {
  AuthResult result;
  HttpClient::response r = HttpClient::PostForm(
     base_url_ + "/login/",
     "action=login&login=" + login_ + "&password=" + password_
  );

  if (r.code == -1 ) {
    // http query failed
    LOG(WARNING) << "Auth failed (HTTP error): " << r.error;
    result.success = false;
    result.error_code = ERROR_CODE::NONE;
    result.http_error = r.error;
  } else if (r.code == 200) {
    // Auth successful
    result.success = true;
    result.error_code = ERROR_CODE::HTTP;
    result.cookies = r.cookies;
    result.http_error = "";
  } else {
    // Auth failed
    LOG(WARNING) << "Auth failed (Application error): " << r.body;
    if (r.body.find("needOtp:")) {
      // ToDo: implement OTP authorization
      result.error_code = ERROR_CODE::OTP;
    } else if (r.body.find("captchaCode:")) {
      result.error_code = ERROR_CODE::CAPTCHA;
    } else {
      result.error_code = ERROR_CODE::UNKNOWN;
    }

    result.success = false;
    result.cookies = r.cookies;
    result.http_error = "";
  }

  return result;
}