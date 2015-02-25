#include "httpclient.h"

#include <cstring>
#include <iostream>
#include <brick/platform_util.h>
#include "../helper.h"
#include <include/base/cef_bind.h>
#include <brick/brick_app.h>
#include <include/cef_url.h>
#include <brick/cef_handler.h>
#include "include/wrapper/cef_closure_task.h"

namespace {
    const char kCookieHeaderName[] = "Set-Cookie";
} // namespace

/** initialize user agent string */
const char* HttpClient::user_agent = HTTP_CLIENT_USER_AGENT;
/** initialize authentication variable */
std::string HttpClient::user_pass =  std::string();

/** Authentication Methods implementation */

void
HttpClient::ClearAuth(){
  HttpClient::user_pass.clear();
}

void
HttpClient::SetAuth(const std::string &user, const std::string &password) {
  HttpClient::user_pass.clear();
  HttpClient::user_pass += user+":"+password;
}

/**
 * @brief HTTP GET method
 *
 * @param url to query
 *
 * @return response struct
 */
HttpClient::response
HttpClient::Get(const std::string &url)
{
  /** create return struct */
  HttpClient::response ret = {};

  // use libcurl
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();
  if (curl)
  {
    InitCurl(curl, &ret);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /** perform the actual query */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      ret.body = "Failed to query.";
      ret.code = -1;
      return ret;
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    ret.code = static_cast<int>(http_code);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

  return ret;
}

/**
* @brief Post form
*
* @param url to query
* @param data HTTP POST body
*
* @return response struct
*/
HttpClient::response
HttpClient::PostForm(const std::string &url, form_map *form)
{
  std::string data;
  form_map::iterator it = form->begin();
  for (; it != form->end(); ++it) {
    data += "&" + Urlencode(it->first) + "=" + Urlencode(it->second);
  }
  return Post(url, "application/x-www-form-urlencoded", data);
}

/**
 * @brief HTTP POST method
 *
 * @param url to query
 * @param ctype content type as string
 * @param data HTTP POST body
 *
 * @return response struct
 */
HttpClient::response
HttpClient::Post(const std::string &url,
   const std::string &ctype,
   const std::string &data)
{
  /** create return struct */
  HttpClient::response ret = {};
  /** build content-type header string */
  std::string ctype_header = "Content-Type: " + ctype;

  // use libcurl
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();
  if (curl)
  {
    InitCurl(curl, &ret);
    /** set query URL */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /** Now specify we want to POST data */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    /** set post fields */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
    /** set content-type header */
    curl_slist* header = NULL;
    header = curl_slist_append(header, ctype_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    /** perform the actual query */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      ret.body = "Failed to query.";
      ret.code = -1;
      return ret;
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    ret.code = static_cast<int>(http_code);

    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

  return ret;
}

/**
 * @brief HTTP PUT method
 *
 * @param url to query
 * @param ctype content type as string
 * @param data HTTP PUT body
 *
 * @return response struct
 */
HttpClient::response
HttpClient::Put(const std::string &url,
   const std::string &ctype,
   const std::string &data)
{
  /** create return struct */
  HttpClient::response ret = {};
  /** build content-type header string */
  std::string ctype_header = "Content-Type: " + ctype;

  /** initialize upload object */
  HttpClient::upload_object up_obj;
  up_obj.data = data.c_str();
  up_obj.length = data.size();

  // use libcurl
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();
  if (curl)
  {
    InitCurl(curl, &ret);
    /** set query URL */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /** Now specify we want to PUT data */
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    /** set read callback function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, HttpClient::ReadCallback);
    /** set data object to pass to callback function */
    curl_easy_setopt(curl, CURLOPT_READDATA, &up_obj);
    /** set data size */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE,
                     static_cast<long>(up_obj.length));

    /** set content-type header */
    curl_slist* header = NULL;
    header = curl_slist_append(header, ctype_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    /** perform the actual query */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      ret.body = "Failed to query.";
      ret.code = -1;
      return ret;
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    ret.code = static_cast<int>(http_code);

    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

  return ret;
}

/**
 * @brief HTTP DELETE method
 *
 * @param url to query
 *
 * @return response struct
 */
HttpClient::response
HttpClient::Del(const std::string &url)
{
  /** create return struct */
  HttpClient::response ret = {};

  /** we want HTTP DELETE */
  const char* http_delete = "DELETE";

  // use libcurl
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();
  if (curl)
  {
    InitCurl(curl, &ret);
    /** set query URL */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /** set HTTP DELETE METHOD */
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, http_delete);
    /** perform the actual query */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      ret.body = "Failed to query.";
      ret.code = -1;
      return ret;
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    ret.code = static_cast<int>(http_code);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

  return ret;
}

/**
 * @brief write callback function for libcurl
 *
 * @param data returned data of size (size*nmemb)
 * @param size size parameter
 * @param nmemb memblock parameter
 * @param userdata pointer to user data to save/work with return data
 *
 * @return (size * nmemb)
 */
size_t
HttpClient::WriteCallback(void *data,
   size_t size,
   size_t nmemb,
   void *userdata)
{
  HttpClient::response* r;
  r = reinterpret_cast<HttpClient::response*>(userdata);
  r->body.append(reinterpret_cast<char*>(data), size*nmemb);

  return (size * nmemb);
}

/**
* @brief write callback function for libcurl
*
* @param data returned data of size (size*nmemb)
* @param size size parameter
* @param nmemb memblock parameter
* @param userdata pointer to user data to save/work with return data
*
* @return (size * nmemb)
*/
size_t
HttpClient::WriteFileCallback(void *data,
   size_t size,
   size_t nmemb,
   std::ofstream *output)
{
  output->write(reinterpret_cast<char*>(data), size*nmemb);
  return size*nmemb;
}

/**
 * @brief header callback for libcurl
 *
 * @param data returned (header line)
 * @param size of data
 * @param nmemb memblock
 * @param userdata pointer to user data object to save headr data
 * @return size * nmemb;
 */
size_t
HttpClient::HeaderCallback(void *data,
   size_t size,
   size_t nmemb,
   void *userdata)
{
  HttpClient::response* r;
  r = reinterpret_cast<HttpClient::response*>(userdata);
  std::string header(reinterpret_cast<char*>(data), size*nmemb);
  size_t separator = header.find_first_of(":");
  if (separator == std::string::npos) {
    //roll with non seperated headers...
    trim(header);
    if ( 0 == header.length() ){
	return (size * nmemb); //blank line;
    }
    r->headers[header] = "present";
  } else {
    std::string key = header.substr(0, separator);
    trim(key);
    std::string value = header.substr(separator + 1);
    trim(value);
    r->headers[key] = value;
    if (key == kCookieHeaderName) {
      ParseCookie(r, value);
    }

  }

  return (size * nmemb);
}

/**
 * @brief read callback function for libcurl
 *
 * @param pointer of max size (size*nmemb) to write data to
 * @param size size parameter
 * @param nmemb memblock parameter
 * @param userdata pointer to user data to read data from
 *
 * @return (size * nmemb)
 */
size_t
HttpClient::ReadCallback(void *data,
   size_t size,
   size_t nmemb,
   void *userdata)
{
  /** get upload struct */
  HttpClient::upload_object* u;
  u = reinterpret_cast<HttpClient::upload_object*>(userdata);
  /** set correct sizes */
  size_t curl_size = size * nmemb;
  size_t copy_size = (u->length < curl_size) ? u->length : curl_size;
  /** copy data to buffer */
  memcpy(data, u->data, copy_size);
  /** decrement length and increment data pointer */
  u->length -= copy_size;
  u->data += copy_size;
  /** return copied size */
  return copy_size;
}

void
HttpClient::ParseCookie(response *resp, const std::string &header) {
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

  resp->cookies[key] = value;
}

std::string
HttpClient::Urldecode(const std::string& str, bool plusAsSpace) {
  std::string result;
  std::string::const_iterator it  = str.begin();
  std::string::const_iterator end = str.end();
  while (it != end)
  {
    char c = *it++;
    // spaces may be encoded as plus signs
    if (plusAsSpace && c == '+') {
      c = ' ';
    }
    else if (c == '%')
    {
      // ToDo: throw exception on error
      if (it == end)
        return NULL; // no hex digit following percent sign
      char hi = *it++;
      if (it == end)
        return NULL; // two hex digits must follow percent sign
      char lo = *it++;
      if (hi >= '0' && hi <= '9')
        c = hi - '0';
      else if (hi >= 'A' && hi <= 'F')
        c = hi - 'A' + 10;
      else if (hi >= 'a' && hi <= 'f')
        c = hi - 'a' + 10;
      else
        return NULL; // not a hex digit
      c *= 16;
      if (lo >= '0' && lo <= '9')
        c += lo - '0';
      else if (lo >= 'A' && lo <= 'F')
        c += lo - 'A' + 10;
      else if (lo >= 'a' && lo <= 'f')
        c += lo - 'a' + 10;
      else
        return NULL; // not a hex digit
    }

    result += c;
  }

  return result;
}

std::string
HttpClient::Urlencode(const std::string& str, bool plusAsSpace) {
  static char hex[] = "0123456789abcdef";
  std::string result;
  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
  {
    char c = *it;
    if ((c >= 'a' && c <= 'z') ||
       (c >= 'A' && c <= 'Z') ||
       (c >= '0' && c <= '9') ||
       c == '-' || c == '_' ||
       c == '.' || c == '~')
    {
      result += c;
    }
    else if (c <= 0x20 || c >= 0x7F)
    {
      result += '%';
      result += hex[(c >> 4)] & 15;
      result += hex[(c & 15)] & 15;
    }
    else
      result += c;
  }

  return result;
}

void
HttpClient::InitCurl(CURL *curl, HttpClient::response *ret) {
  /** set basic authentication if present*/
  if(HttpClient::user_pass.length()>0){
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, HttpClient::user_pass.c_str());
  }

  curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_CLIENT_CONNECTION_TIMEOUT);
  /** SSL configuration **/
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
  /** set user agent */
  curl_easy_setopt(curl, CURLOPT_USERAGENT, HttpClient::user_agent);
  /** set callback function */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpClient::WriteCallback);
  /** set data object to pass to callback function */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, ret);
  /** set the header callback function */
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HttpClient::HeaderCallback);
  /** callback object for headers */
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, ret);
  /**  set error buffer for error messages **/
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, ret->error);
}

bool
HttpClient::Download(const std::string& url, const std::string& path) {
  /** create return struct */
  HttpClient::response ret = {};

  // use libcurl
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();
  if (curl)
  {
    platform_util::MakeDirectory(helper::BaseDir(path));
    std::ofstream outfile(path, std::ofstream::binary);

    InitCurl(curl, &ret);

    /** set callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpClient::WriteFileCallback);
    /** set data object to pass to callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outfile);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /** perform the actual query */
    res = curl_easy_perform(curl);
    outfile.close();
    if (res != CURLE_OK)
    {
      ret.body = "Failed to query.";
      ret.code = -1;
      return false;
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    ret.code = static_cast<int>(http_code);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

  return true;
}

void
HttpClient::DownloadAsync(const std::string &url, const std::string &path) {
  if (!CefCurrentlyOn(TID_IO)) {
    CefPostTask(TID_IO, base::Bind(&HttpClient::DownloadAsync, url, path));
    return;
  }

  if (!Download(url, path))
    LOG(WARNING) << "Can't download url '" << url << "' to file '" << path << "'";
}

std::string
HttpClient::GetCached(const std::string& url, CacheManager::TYPE type, bool sync) {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  if (!client_handler)
    return "";

  std::string path = client_handler->GetCacheManager()->GetCachePath(url, type);

  if (platform_util::IsPathExists(path)) {
    return path;
  }

  if (sync) {
    return Download(url, path)? path: "";
  }

  DownloadAsync(url, path);
  return "";
}