#ifndef BRICK_APP_MESSAGE_DELEGATE_H_
#define BRICK_APP_MESSAGE_DELEGATE_H_

#include "brick/cef_handler.h"

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

    // Custom methods
    static void SetCookie(CefRefPtr<CefCookieManager> manager, const CefString &url, CefCookie cookie);
    static CefString ParseAuthSessid(std::string body);

  IMPLEMENT_REFCOUNTING(ProcessMessageDelegate);
};

#endif // BRICK_APP_MESSAGE_DELEGATE_H_