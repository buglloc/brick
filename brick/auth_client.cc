// Copyright (c) 2015 The Brick Authors.

#include "brick/auth_client.h"

#include "third-party/json/json.h"
#include "include/cef_parser.h"
#include "brick/account.h"

namespace {

  const char kValidResponseType[] = "application/json";

  std::string
  GetOsMark() {
    // TODO(buglloc): use app_token!
    char hostname[1024];
    gethostname(hostname, 1024);

    return std::string(hostname);
  }

  Json::Value
  TryToParseOldResponse(const std::string& body) {
    Json::Value result;
    if (body.find('{') != 0)
      return result;

    if (body.find("success:") == std::string::npos)
      return result;

    result["success"] = (body.find("success: true") != std::string::npos);
    result["needOtp"] = (body.find("needOtp:") != std::string::npos);

    if (body.find("captchaCode:") != std::string::npos)
      result["captchaCode"] = true;

    std::string::size_type pos = body.find("appPassword: '");
    if (pos != std::string::npos) {
      pos += sizeof("appPassword: '");
      result["appPassword"] = body.substr(
          pos - 1,
          body.find("'", pos + 1) - pos + 1
      );
    }

    pos = body.find("bitrixSessionId: '");
    if (pos != std::string::npos) {
      pos += sizeof("bitrixSessionId: '");
      result["bitrixSessionId"] = body.substr(
          pos - 1,
          body.find("'", pos + 1) - pos + 1
      );
    }


    return result;
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
  result.bitrix_sessid = "";

  bool finished = false;
  CefRefPtr<CefResponse> response = request->GetResponse();

  if (
      request->GetRequest()->GetURL() != response->GetURL()  // HSTS?
      || (request->GetRequestStatus() == UR_CANCELED
        && (response->GetStatus() / 100) == 3)
      ) {
    // Deal with redirects
    LOG(WARNING) << "Auth failed (redirect occurred): " << response->GetURL().ToString();
    result.success = false;
    result.error_code = Account::ERROR_CODE::INVALID_URL;
    result.http_error = "Redirect occurred: ";
    CefURLParts redirect_parts;
    if (CefParseURL(response->GetURL(), redirect_parts)) {
      result.http_error +=  CefString(&redirect_parts.origin);
    }
    finished = true;
  }

  if (!finished && request->GetRequestStatus() == UR_CANCELED) {
    // CEF was pragmatically cancel our request. It may be certificate error or something other...
    LOG(WARNING) << "Auth request was canceled, probably SSL error";
    result.success = false;
    result.error_code = Account::ERROR_CODE::HTTP;
    result.http_error = request_util::GetErrorString(ERR_CONNECTION_FAILED);
    finished = true;
  }

  if (!finished
      && (request->GetRequestStatus() == UR_FAILED
          || request->GetRequestError() != ERR_NONE)) {

    std::string error = request_util::GetErrorString(request->GetRequestError());

    LOG(WARNING) << "Auth request was failed: " << error;
    result.success = false;
    result.error_code = Account::ERROR_CODE::HTTP;
    result.http_error = error;
    finished = true;
  }

  if (!finished && request->GetRequestStatus() != UR_SUCCESS) {
    LOG(WARNING) << "Unexpected request status: " << request->GetRequestStatus();
    result.success = false;
    result.error_code = Account::ERROR_CODE::HTTP;
    result.http_error = request_util::GetErrorString(ERR_UNEXPECTED);
    finished = true;
  }

  Json::Value json_response;
  if (!finished && !response->GetMimeType().empty()) {
    if (response->GetMimeType() == kValidResponseType) {
      Json::Reader reader;

      LOG_IF(ERROR, !reader.parse(body_, json_response)) << "Failed to parse auth response\n"
                   << reader.getFormattedErrorMessages();
    } else {
      json_response = TryToParseOldResponse(body_);
    }
  }

  if (!finished && (!json_response.isMember("success") || !json_response["success"].isBool())) {
    // Server return incorrect JSON response...it's...it's so strange :(
    LOG(WARNING) << "Auth failed (can't find status in response): " << body_;
    result.success = false;
    result.error_code = Account::ERROR_CODE::HTTP;
    result.http_error = request_util::GetErrorString(ERR_INVALID_RESPONSE)
                        + "\nPlease check the server schema and host";
    finished = true;
  }

  if (!finished) {
    CefResponse::HeaderMap headerMap;
    response->GetHeaderMap(headerMap);

    switch (response->GetStatus()) {
      case 200:
        if (json_response.isMember("appPassword")) {
          // Server may return new password for our auth request
          app_password = json_response["appPassword"].asString();
          LOG(INFO) << "Received App Password";
        }

        if (json_response.isMember("bitrixSessionId")) {
          result.bitrix_sessid = json_response["bitrixSessionId"].asString();
        }

        LOG(INFO) << "Successful auth";
        result.success = json_response["success"].asBool();
        result.error_code = Account::ERROR_CODE::NONE;
        result.cookies = request_util::GetCookies(headerMap);
      break;
      case 401:
        // Auth failed
        LOG(WARNING) << "Auth failed: " << body_;
        if (json_response.isMember("needOtp") && json_response["needOtp"].asBool()) {
          result.error_code = Account::ERROR_CODE::OTP;
        } else if (json_response.isMember("captchaCode")) {
          result.error_code = Account::ERROR_CODE::CAPTCHA;
        } else {
          result.error_code = Account::ERROR_CODE::AUTH;
        }
      break;
      case 403:
        LOG(WARNING) << "Auth failed (403)";
        result.success = false;
        result.error_code = Account::ERROR_CODE::HTTP;
        result.http_error = request_util::GetErrorString(ERR_INVALID_RESPONSE)
                            + "\nPlease check the server schema and host";
      default:
        // Some error occurred...
        LOG(WARNING) << "Auth failed (Application error): " << body_;
        result.success = false;
        result.error_code = Account::ERROR_CODE::HTTP;
        result.http_error = request_util::GetErrorString(ERR_INVALID_RESPONSE)
                            + "\nPlease check the server schema and host";

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
  request_util::PostFormMap form;
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
    request_util::PostFormMap form) {

  CEF_REQUIRE_UI_THREAD();
  CefRefPtr<CefRequest> request = CefRequest::Create();
  request->SetURL(url);
  request->SetMethod("POST");
  request->SetPostData(request_util::PostFormToCefPost(form));
  request->SetFlags(UR_FLAG_SKIP_CACHE|UR_FLAG_NO_RETRY_ON_5XX|UR_FLAG_STOP_ON_REDIRECT);

  // Create and start the new CefURLRequest.
  return CefURLRequest::Create(request, new AuthClient(callback, url), NULL);
}
