#include <unistd.h>
#include <brick/httpclient/httpclient.h>
#include "account.h"
#include "httpclient/httpclient.h"
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
Account::Auth(bool renew_password, std::string otp) {
  AuthResult result;
  HttpClient::form_map form;
  // ToDo: use new authentication IM protocol
//  form["json"] = "y"; // New versions of IM must return result in json format
  if (renew_password) {
    // New versions of IM must generate application password
    form["renew_password"] = "y";
  }

  form["action"] = "login";
  form["login"] = login_;
  if (renew_password && otp.empty()) {
    // Ugly hack to get application password in older IM versions
    form["password"] = password_.substr(0, password_.length() - 1);
    form["otp"] = password_.substr(password_.length() - 1, 1);
  } else {
    form["password"] = password_;
    form["otp"] = otp;
  }


  form["user_os_mark"] = GetOsMark();

  HttpClient::response r = HttpClient::PostForm(
     base_url_ + "/login/", &form
  );

  if (r.code == 200) {
    // Auth successful
    if (r.body.find("success: true") != std::string::npos) {
      // Maybe server returns application password?
      std::string new_password = TryParseApplicationPassword(r.body);
      if (password_ != new_password) {
        LOG_IF(WARNING, !renew_password) << "Unexpected password update";
        password_ = new_password;
      }

      result.success = true;
      result.error_code = ERROR_CODE::NONE;
      result.cookies = r.cookies;
    } else {
      // Probably application fatal occurred
      result.success = false;
      result.error_code = ERROR_CODE::UNKNOWN;
    }

  } else if (r.code == -1 ) {
    // http query failed
    LOG(WARNING) << "Auth failed (HTTP error): " << r.error;
    result.success = false;
    result.error_code = ERROR_CODE::HTTP;
    result.http_error = r.error;
  } else if (r.code == 401 ) {
    // Auth failed
    LOG(WARNING) << "Auth failed: " << r.body;
    if (r.body.find("needOtp:") != std::string::npos) {
      // ToDo: implement OTP authorization
      result.error_code = ERROR_CODE::OTP;
    } else if (r.body.find("captchaCode:") != std::string::npos) {
      result.error_code = ERROR_CODE::CAPTCHA;
    } else {
      result.error_code = ERROR_CODE::AUTH;
    }

    result.success = false;
    result.cookies = r.cookies;
  } else {
    // Some error occurred...
    LOG(WARNING) << "Auth failed (Application error): " << r.body;
    result.error_code = ERROR_CODE::UNKNOWN;
    result.success = false;
    result.cookies = r.cookies;
  }

  return result;
}

std::string
Account::TryParseApplicationPassword(std::string body) {
  std::string password;
  size_t pos = body.find("appPassword: '");
  if (pos == std::string::npos)
    return password_;

  pos += sizeof("appPassword: '");
  password = body.substr(
     pos - 1,
     body.find("'", pos + 1) - pos + 1
  );

  return password;
}

std::string
Account::GetOsMark() {
  // ToDo: use app_token!
  char hostname[1024];
  gethostname(hostname, 1024);

  return std::string(hostname);
}