// Copyright (c) 2015 The Brick Authors.

#include "brick/request_helper.h"

#include "include/cef_request.h"

namespace request_helper {

  std::string
  Urlencode(const std::string& str, bool plusAsSpace) {
    static char hex[] = "0123456789ABCDEF";
    std::string result;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
      char c = *it;
      if ((c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') ||
          c == '-' || c == '_' ||
          c == '.' || c == '~') {
        result += c;
      } else {
        result += '%';
        result += hex[(c >> 4) & 0xf];
        result += hex[(c & 0xf)];
      }
    }

    return result;
  }

  std::string
  Urldecode(const std::string& str, bool plusAsSpace) {
    std::string result;
    std::string::const_iterator it  = str.begin();
    std::string::const_iterator end = str.end();
    while (it != end) {
      char c = *it++;
      // spaces may be encoded as plus signs
      if (plusAsSpace && c == '+') {
        c = ' ';
      } else if (c == '%') {
        // ToDo: throw exception on error
        if (it == end)
          return NULL;  // no hex digit following percent sign
        char hi = *it++;
        if (it == end)
          return NULL;  // two hex digits must follow percent sign
        char lo = *it++;
        if (hi >= '0' && hi <= '9')
          c = hi - '0';
        else if (hi >= 'A' && hi <= 'F')
          c = hi - 'A' + 10;
        else if (hi >= 'a' && hi <= 'f')
          c = hi - 'a' + 10;
        else
          return NULL;  // not a hex digit
        c *= 16;
        if (lo >= '0' && lo <= '9')
          c += lo - '0';
        else if (lo >= 'A' && lo <= 'F')
          c += lo - 'A' + 10;
        else if (lo >= 'a' && lo <= 'f')
          c += lo - 'a' + 10;
        else
          return NULL;  // not a hex digit
      }

      result += c;
    }

    return result;
  }

  CefRefPtr<CefPostData>
  PostFormToCefPost(PostFormMap& form) {
    std::string data;
    for (auto it = form.begin(); it != form.end(); ++it) {
      data += "&" + Urlencode(it->first) + "=" + Urlencode(it->second);
    }

    CefRefPtr<CefPostDataElement> postDataElement(CefPostDataElement::Create());
    postDataElement->SetToBytes(data.length(), data.c_str());
    CefRefPtr<CefPostData> result = CefPostData::Create();
    result->AddElement(postDataElement);
    return result;
  }

  CookiesMap
  GetCookies(CefResponse::HeaderMap& headers) {
    CookiesMap result;
    for (auto it = headers.begin(); it != headers.end(); ++it) {
      if (it->first == "Set-Cookie") {
        ParseCookie(it->second, result);
      }
    }

    return result;
  }

  void
  ParseCookie(const std::string& header, CookiesMap& destination) {
    size_t separator = header.find_first_of("=");
    size_t end_value = header.find_first_of(";");
    if (separator == std::string::npos) {
      return;
    }

    std::string key = header.substr(0, separator);
    std::string value;
    if (end_value != std::string::npos) {
      value = Urldecode(header.substr(separator + 1, end_value - separator - 1));
    } else {
      value = Urldecode(header.substr(separator + 1));
    }

    destination[key] = value;
  }

  std::string
  GetErrorString(CefURLRequest::ErrorCode code) {
    // Case condition that returns |code| as a string.
    #define CASE(code) case code: return #code

    switch (code) {
      CASE(ERR_NONE);
      CASE(ERR_FAILED);
      CASE(ERR_ABORTED);
      CASE(ERR_INVALID_ARGUMENT);
      CASE(ERR_INVALID_HANDLE);
      CASE(ERR_FILE_NOT_FOUND);
      CASE(ERR_TIMED_OUT);
      CASE(ERR_FILE_TOO_BIG);
      CASE(ERR_UNEXPECTED);
      CASE(ERR_ACCESS_DENIED);
      CASE(ERR_NOT_IMPLEMENTED);
      CASE(ERR_CONNECTION_CLOSED);
      CASE(ERR_CONNECTION_RESET);
      CASE(ERR_CONNECTION_REFUSED);
      CASE(ERR_CONNECTION_ABORTED);
      CASE(ERR_CONNECTION_FAILED);
      CASE(ERR_NAME_NOT_RESOLVED);
      CASE(ERR_INTERNET_DISCONNECTED);
      CASE(ERR_SSL_PROTOCOL_ERROR);
      CASE(ERR_ADDRESS_INVALID);
      CASE(ERR_ADDRESS_UNREACHABLE);
      CASE(ERR_SSL_CLIENT_AUTH_CERT_NEEDED);
      CASE(ERR_TUNNEL_CONNECTION_FAILED);
      CASE(ERR_NO_SSL_VERSIONS_ENABLED);
      CASE(ERR_SSL_VERSION_OR_CIPHER_MISMATCH);
      CASE(ERR_SSL_RENEGOTIATION_REQUESTED);
      CASE(ERR_CERT_COMMON_NAME_INVALID);
      CASE(ERR_CERT_DATE_INVALID);
      CASE(ERR_CERT_AUTHORITY_INVALID);
      CASE(ERR_CERT_CONTAINS_ERRORS);
      CASE(ERR_CERT_NO_REVOCATION_MECHANISM);
      CASE(ERR_CERT_UNABLE_TO_CHECK_REVOCATION);
      CASE(ERR_CERT_REVOKED);
      CASE(ERR_CERT_INVALID);
      CASE(ERR_CERT_END);
      CASE(ERR_INVALID_URL);
      CASE(ERR_DISALLOWED_URL_SCHEME);
      CASE(ERR_UNKNOWN_URL_SCHEME);
      CASE(ERR_TOO_MANY_REDIRECTS);
      CASE(ERR_UNSAFE_REDIRECT);
      CASE(ERR_UNSAFE_PORT);
      CASE(ERR_INVALID_RESPONSE);
      CASE(ERR_INVALID_CHUNKED_ENCODING);
      CASE(ERR_METHOD_NOT_SUPPORTED);
      CASE(ERR_UNEXPECTED_PROXY_AUTH);
      CASE(ERR_EMPTY_RESPONSE);
      CASE(ERR_RESPONSE_HEADERS_TOO_BIG);
      CASE(ERR_CACHE_MISS);
      CASE(ERR_INSECURE_RESPONSE);
      default:
        return "UNKNOWN";
    }
  }
}  // namespace request_helper
