#ifndef BRICK_APP_WINDOW_MESSAGE_DELEGATE_H_
#define BRICK_APP_WINDOW_MESSAGE_DELEGATE_H_

#include "brick/cef_handler.h"

class AppWindowMessageDelegate : public ClientHandler::ProcessMessageDelegate {
public:
  AppWindowMessageDelegate();

  // From ClientHandler::ProcessMessageDelegate.
  virtual bool OnProcessMessageReceived(
     CefRefPtr<ClientHandler> handler,
     CefRefPtr<CefBrowser> browser,
     CefProcessId source_process,
     CefRefPtr<CefProcessMessage> message) OVERRIDE;

  // Create message delegates that run in the browser process
  static void CreateProcessMessageDelegates(ClientHandler::ProcessMessageDelegateSet& delegates);

IMPLEMENT_REFCOUNTING(ProcessMessageDelegate);
};

#endif // BRICK_APP_MESSAGE_DELEGATE_H_