// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_MESSAGE_DELEGATE_APP_MESSAGE_DELEGATE_H_
#define BRICK_MESSAGE_DELEGATE_APP_MESSAGE_DELEGATE_H_
#pragma once

#include <string>

#include "brick/client_handler.h"

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
  static void CreateProcessMessageDelegates(ClientHandler::ProcessMessageDelegateSet *delegates);
  void OnAfterLogin(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefProcessMessage> response,
                           CefRefPtr<Account> account,
                           Account::AuthResult auth_result);

  void SetCookies(CefRefPtr<CefCookieManager> manager,
                         const CefString &url,
                         request_helper::CookiesMap cookies,
                         bool is_secure);

IMPLEMENT_REFCOUNTING(AppMessageDelegate);
};

#endif  // BRICK_MESSAGE_DELEGATE_APP_MESSAGE_DELEGATE_H_
