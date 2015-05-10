// Copyright (c) 2015 The Brick Authors.

#include "brick/account.h"

#include "third-party/json/json.h"
#include "brick/auth_client.h"
#include "account.h"

namespace {

  const char kValidReponseType[] = "application/json";

  std::string
  GetOsMark() {
    // ToDo: use app_token!
    char hostname[1024];
    gethostname(hostname, 1024);

    return std::string(hostname);
  }

}  // namespace

AuthClient::AuthClient(const Callback& callback, const std::string& url)
    : callback_(callback),
      url_(url),
      body_("") {

  CEF_REQUIRE_UI_THREAD();
  DCHECK(!callback_.is_null());
}

void
AuthClient::Detach() {
  CEF_REQUIRE_UI_THREAD();
  if (!callback_.is_null())
    callback_.Reset();
}

void
AuthClient::OnRequestComplete(CefRefPtr<CefURLRequest> request) {
  CEF_REQUIRE_UI_THREAD();
  if (callback_.is_null())
    return;

  std::string app_password = "";
  Account::AuthResult result;
  result.success = false;
  result.error_code = Account::ERROR_CODE::NONE;

  bool finished = false;
  CefRefPtr<CefResponse> response = request->GetResponse();

  if (!finished && request->GetRequestStatus() == UR_CANCELED) {
    // CEF was pragmatically cancel our request. It may be certificate error or redirect (if set UR_FLAG_STOP_ON_REDIRECT) or something other...
    LOG(WARNING) << "Auth request was canceled, probably SSL or redirect error occurred";
    result.success = false;
    result.error_code = Account::ERROR_CODE::HTTP;
    result.http_error = request_helper::GetErrorString(ERR_CONNECTION_FAILED);
    finished = true;
  };

  if (!finished
      && (request->GetRequestStatus() == UR_FAILED
          || request->GetRequestError() != ERR_NONE)) {

    std::string error = request_helper::GetErrorString(request->GetRequestError());

    LOG(WARNING) << "Auth request was failed: " << error;
    result.success = false;
    result.error_code = Account::ERROR_CODE::HTTP;
    result.http_error = error;
    finished = true;
  };

  if (!finished && request->GetRequestStatus() != UR_SUCCESS) {
    LOG(WARNING) << "Unexpected request status: " << request->GetRequestStatus();
    result.success = false;
    result.error_code = Account::ERROR_CODE::HTTP;
    result.http_error = request_helper::GetErrorString(ERR_UNEXPECTED);
    finished = true;
  }

  if (!finished && response->GetMimeType() != kValidReponseType) {
    // Strange content-type. We expect JSON response...
    LOG(WARNING) << "Auth failed due to unexpected response type: "
                 << response->GetMimeType().ToString();
    result.error_code = Account::ERROR_CODE::HTTP;
    result.http_error = request_helper::GetErrorString(ERR_INVALID_RESPONSE)
                        + "\nPlease check the server schema and host";
    result.success = false;
    finished = true;
  }

  if (!finished) {
    CefResponse::HeaderMap headerMap;
    response->GetHeaderMap(headerMap);
    Json::Value json;
    Json::Reader reader;

    if (!reader.parse(body_, json)) {
      LOG(ERROR) << "Failed to parse auth response\n"
                 << reader.getFormattedErrorMessages();
      return;
    }

    switch (response->GetStatus()) {
      case 200:
        if (json.isMember("success") && json["success"].isBool()) {

          if (json.isMember("app_password")) {
            // Server may return new password for our auth request
            app_password = json["app_password"].asString();
            LOG(INFO) << "Received App Password";
          }

          LOG(INFO) << "Successful auth";
          result.success = json["success"].asBool();
          result.error_code = Account::ERROR_CODE::NONE;
          result.cookies = request_helper::GetCookies(headerMap);
        } else {
          // Server return incorrect JSON response...it's...it's so strange :(
          LOG(WARNING) << "Auth failed (can't find status in response): " << body_;
          result.success = false;
          result.error_code = Account::ERROR_CODE::UNKNOWN;
        }
      break;
      case 401:
        // Auth failed
        LOG(WARNING) << "Auth failed: " << body_;
        if (json.isMember("needOtp")) {
          result.error_code = Account::ERROR_CODE::OTP;
        } else if (json.isMember("needOtp")) {
          result.error_code = Account::ERROR_CODE::CAPTCHA;
        } else {
          result.error_code = Account::ERROR_CODE::AUTH;
        }
      break;
      case 403:
        // Probably our request was redirected.
        // ToDo: Implement getting redirected url from CEF (UR_FLAG_STOP_ON_REDIRECT)
        LOG(WARNING) << "Auth failed (403)";
        result.success = false;
        result.error_code = Account::ERROR_CODE::HTTP;
        result.http_error = request_helper::GetErrorString(ERR_INVALID_RESPONSE)
                            + "\nPlease check the server schema and host";
      default:
        // Some error occurred...
        LOG(WARNING) << "Auth failed (Application error): " << body_;
        result.error_code = Account::ERROR_CODE::UNKNOWN;
        result.success = false;

    }
  }

  callback_.Run(result, app_password);
  callback_.Reset();
}

void
AuthClient::OnDownloadData(CefRefPtr<CefURLRequest> request,
                    const void* data,
                    size_t data_length) {

  CEF_REQUIRE_UI_THREAD();
  body_ += std::string(static_cast<const char*>(data), data_length);
}

// static methods

CefRefPtr<CefURLRequest> AuthClient::CreateRequest(
    const Callback&  callback,
    const CefRefPtr<Account> account,
    const std::string& otp,
    bool renew) {

  CEF_REQUIRE_UI_THREAD();
  return CreateRequest(
      callback,
      account->GetAuthUrl(),
      account->GetLogin(),
      account->GetPassword(),
      otp,
      renew
  );
}

CefRefPtr<CefURLRequest>
AuthClient::CreateRequest(
    const Callback& callback,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& otp,
    bool renew) {

  CEF_REQUIRE_UI_THREAD();
  request_helper::PostFormMap form;
  // New versions of IM must return result in json format
  form["json"] = "y";
  if (!otp.empty()) {
    form["otp"] = otp;
  } else if (renew) {
    form["renew_password"] = "y";
  }

  form["action"] = "login";
  form["login"] = login;
  form["password"] = password;
  form["user_os_mark"] = GetOsMark();

  return CreateRequest(callback, url, form);
}

CefRefPtr<CefURLRequest>
AuthClient::CreateRequest(
    const Callback& callback,
    const std::string& url,
    request_helper::PostFormMap form) {

  CEF_REQUIRE_UI_THREAD();
  CefRefPtr<CefRequest> request = CefRequest::Create();
  request->SetURL(url);
  request->SetMethod("POST");
  request->SetPostData(request_helper::PostFormToCefPost(form));
  request->SetFlags(UR_FLAG_SKIP_CACHE|UR_FLAG_SKIP_CACHE|UR_FLAG_NO_RETRY_ON_5XX);

  // Create and start the new CefURLRequest.
  return CefURLRequest::Create(request, new AuthClient(callback, url));
}
