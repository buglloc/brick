#ifndef BRICK_HTTPCLIENT_H_
#define BRICK_HTTPCLIENT_H_

#include "curl/curl.h"
#include "curl/easy.h"
#include <string>
#include <map>
#include <cstdlib>
#include <algorithm>

#define HTTP_CLIENT_USER_AGENT "HttpClient/0.1"

class HttpClient
{
  public:
    /**
     * public data definitions
     */
    typedef std::map<std::string, std::string> header_map;
    typedef std::map<std::string, std::string> cookie_map;

    /** response struct for queries */
    typedef struct
    {
      int code;
      char error[CURL_ERROR_SIZE];
      std::string body;
      header_map headers;
      cookie_map cookies;
    } response;

    /** struct used for uploading data */
    typedef struct
    {
      const char* data;
      size_t length;
    } upload_object;

    /** public methods */
    // Auth
    static void ClearAuth();
    static void SetAuth(const std::string &user, const std::string &password);
    // HTTP GET
    static response Get(const std::string &url);
    // HTTP POST
    static response PostForm(const std::string &url, const std::string &data);
    static response Post(const std::string &url, const std::string &ctype,
       const std::string &data);
    // HTTP PUT
    static response Put(const std::string &url, const std::string &ctype,
       const std::string &data);
    // HTTP DELETE
    static response Del(const std::string &url);

    static std::string Urldecode(const std::string& str, bool plusAsSpace = false);
    static std::string Urlencode(const std::string& str, bool plusAsSpace = false);

  private:
    static void InitCurl(CURL *curl, HttpClient::response *ret);
    static void ParseCookie(response *resp, const std::string &header);
    // writedata callback function
    static size_t WriteCallback(void *ptr, size_t size, size_t nmemb,
       void *userdata);

    // header callback function
    static size_t HeaderCallback(void *ptr, size_t size, size_t nmemb,
       void *userdata);
    // read callback function
    static size_t ReadCallback(void *ptr, size_t size, size_t nmemb,
       void *userdata);
    static const char* user_agent;
    static std::string user_pass;

    // trim from start
    static inline std::string &ltrim(std::string &s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
      return s;
    }

    // trim from end
    static inline std::string &rtrim(std::string &s) {
      s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
      return s;
    }

    // trim from both ends
    static inline std::string &trim(std::string &s) {
      return ltrim(rtrim(s));
    }

};

#endif  // BRICK_HTTPCLIENT_H_
