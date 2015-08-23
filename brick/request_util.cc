// Copyright (c) 2015 The Brick Authors.

#include "brick/request_util.h"

#include "include/cef_request.h"
#include "include/cef_parser.h"

namespace request_util {

  CefRefPtr<CefPostData>
  PostFormToCefPost(const PostFormMap& form) {
    std::string data;
    for (const auto &element : form) {
      data += "&" + CefURIEncode(element.first, false).ToString() + "=" + CefURIEncode(element.second, false).ToString();
    }

    CefRefPtr<CefPostDataElement> postDataElement(CefPostDataElement::Create());
    postDataElement->SetToBytes(data.length(), data.c_str());
    CefRefPtr<CefPostData> result = CefPostData::Create();
    result->AddElement(postDataElement);
    return result;
  }

  CookiesMap
  GetCookies(const CefResponse::HeaderMap& headers) {
    CookiesMap result;
    for (const auto &it : headers) {
      if (it.first == "Set-Cookie") {
        ParseCookie(it.second, result);
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
    CefString value;
    if (end_value != std::string::npos) {
      value = CefURIDecode(
          header.substr(separator + 1, end_value - separator - 1),
          false,
          static_cast<cef_uri_unescape_rule_t>(
            UU_SPACES | UU_URL_SPECIAL_CHARS)
          );
    } else {
      value = CefURIDecode(
          header.substr(separator + 1),
          false,
          static_cast<cef_uri_unescape_rule_t>(
              UU_SPACES | UU_URL_SPECIAL_CHARS)
      );
    }

    destination[key] = value;
  }

  std::string
  GetErrorString(CefURLRequest::ErrorCode code) {
    // Case condition that returns |code| as a string.
    #define CASE(code, explain) case code: \
     return std::string(explain) + " (" + #code  + "/" + std::to_string(abs(code)) + ").";

    switch (code) {
      CASE(ERR_NONE,
           "ERR_NONE");
      CASE(ERR_FAILED,
           "A generic failure occurred");
      CASE(ERR_ABORTED,
           "An operation was aborted");
      CASE(ERR_INVALID_ARGUMENT,
           "An argument to the function is incorrect");
      CASE(ERR_INVALID_HANDLE,
           "The handle or file descriptor is invalid");
      CASE(ERR_FILE_NOT_FOUND,
           "The file or directory cannot be found");
      CASE(ERR_TIMED_OUT,
           "An operation timed out");
      CASE(ERR_FILE_TOO_BIG,
           "The file is too large");
      CASE(ERR_UNEXPECTED,
           "An unexpected error");
      CASE(ERR_ACCESS_DENIED,
           "Permission to access a resource, other than the network, was denied");
      CASE(ERR_NOT_IMPLEMENTED,
           "The operation failed because of unimplemented functionality");
      CASE(ERR_CONNECTION_CLOSED,
           "A connection was closed");
      CASE(ERR_CONNECTION_RESET,
           "A connection was reset");
      CASE(ERR_CONNECTION_REFUSED,
           "A connection attempt was refused");
      CASE(ERR_CONNECTION_ABORTED,
           "A connection timed out");
      CASE(ERR_CONNECTION_FAILED,
           "A connection attempt failed");
      CASE(ERR_NAME_NOT_RESOLVED,
           "The host name could not be resolved");
      CASE(ERR_INTERNET_DISCONNECTED,
           "The Internet connection has been lost");
      CASE(ERR_SSL_PROTOCOL_ERROR,
           "An SSL protocol error occurred");
      CASE(ERR_ADDRESS_INVALID,
           "The IP address or port number is invalid");
      CASE(ERR_ADDRESS_UNREACHABLE,
           "The IP address is unreachable");
      CASE(ERR_SSL_CLIENT_AUTH_CERT_NEEDED,
           "The server requested a client certificate for SSL client authentication");
      CASE(ERR_TUNNEL_CONNECTION_FAILED,
           "A tunnel connection through the proxy could not be established");
      CASE(ERR_NO_SSL_VERSIONS_ENABLED,
           "No SSL protocol versions are enabled");
      CASE(ERR_SSL_VERSION_OR_CIPHER_MISMATCH,
           "The client and server don't support a common SSL protocol version or cipher suite");
      CASE(ERR_SSL_RENEGOTIATION_REQUESTED,
           "The server requested a renegotiation (rehandshake)");
      CASE(ERR_CERT_COMMON_NAME_INVALID,
           "The certificate common name did not match the host name");
      CASE(ERR_CERT_DATE_INVALID,
           "The certificate appears to either not yet be valid or to have expired");
      CASE(ERR_CERT_AUTHORITY_INVALID,
           "The certificate is signed by an untrusted authority");
      CASE(ERR_CERT_CONTAINS_ERRORS,
           "The certificate contains errors");
      CASE(ERR_CERT_NO_REVOCATION_MECHANISM,
           "The certificate has no mechanism for determining if it is revoked");
      CASE(ERR_CERT_UNABLE_TO_CHECK_REVOCATION,
           "Revocation information for the security certificate for this site is not available");
      CASE(ERR_CERT_REVOKED,
           "The certificate has been revoked");
      CASE(ERR_CERT_INVALID,
           "The certificate is invalid");
      CASE(ERR_CERT_END,
           "The value immediately past the last certificate error code");
      CASE(ERR_INVALID_URL,
           "The URL is invalid");
      CASE(ERR_DISALLOWED_URL_SCHEME,
           "The scheme of the URL is disallowed");
      CASE(ERR_UNKNOWN_URL_SCHEME,
           "The scheme of the URL is unknown");
      CASE(ERR_TOO_MANY_REDIRECTS,
          "Attempting to load an URL resulted in too many redirects");
      CASE(ERR_UNSAFE_REDIRECT,
           "Attempting to load an URL resulted in an unsafe redirect");
      CASE(ERR_UNSAFE_PORT,
           "Attempting to load an URL with an unsafe port number");
      CASE(ERR_INVALID_RESPONSE,
           "The server's response was invalid");
      CASE(ERR_INVALID_CHUNKED_ENCODING,
           "Error in chunked transfer encoding");
      CASE(ERR_METHOD_NOT_SUPPORTED,
           "The server did not support the request method");
      CASE(ERR_UNEXPECTED_PROXY_AUTH,
           "Proxy authentication required for request without proxy");
      CASE(ERR_EMPTY_RESPONSE,
           "The server closed the connection without sending any data");
      CASE(ERR_RESPONSE_HEADERS_TOO_BIG,
           "The headers section of the response is too large");
      CASE(ERR_CACHE_MISS,
           "The cache does not have the requested entry");
      CASE(ERR_INSECURE_RESPONSE,
           "The server's response was insecure");
      default:
        return "UNKNOWN";
    }
  }

  std::string
  ParseDownloadFilename(const std::string& url) {
    std::string result;

    auto start_pos = url.find("fileName=");
    if (start_pos == std::string::npos) {
      return "undefined";
    }
    start_pos += sizeof("fileName=") - 1;

    auto end_pos = url.find_first_of("&#", start_pos);
    if (end_pos == std::string::npos) {
      result = url.substr(start_pos);
    } else {
      result = url.substr(start_pos, end_pos - start_pos);
    }

    return CefURIDecode(
        result,
        true,
        static_cast<cef_uri_unescape_rule_t>(
            UU_SPACES | UU_URL_SPECIAL_CHARS | UU_REPLACE_PLUS_WITH_SPACE)
    );
  }

}  // namespace request_util
