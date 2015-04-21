#ifndef BRICK_APP_MESSAGE_DELEGATE_H_
#define BRICK_APP_MESSAGE_DELEGATE_H_

#include "brick/cef_handler.h"
#include "../httpclient/httpclient.h"

class AppMessageDelegate : public ClientHandler::ProcessMessageDelegate {
public:
  AppMessageDelegate();

  // From ClientHandler::ProcessMessageDelegate.
  virtual bool OnProcessMessageReceived(
     CefRefPtr<ClientHandler> handler,
     CefRefPtr<CefBrowser> browser,
     CefProcessId source_process,
     CefRefPtr<CefProcessMessage> message) OVERRIDE;

  // Create message delegates that run in the browser process
  static void CreateProcessMessageDelegates(ClientHandler::ProcessMessageDelegateSet& delegates);
  static void Login(CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> response, CefRefPtr<Account> account);
  static void SetCookies(CefRefPtr<CefCookieManager> manager, const CefString &url,
                         HttpClient::cookie_map cookies, bool is_secure);

protected:
  static CefString ParseAuthSessid(std::string body);

IMPLEMENT_REFCOUNTING(AppMessageDelegate);
};

#endif // BRICK_APP_MESSAGE_DELEGATE_H_