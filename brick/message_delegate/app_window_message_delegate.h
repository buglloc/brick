// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_MESSAGE_DELEGATE_APP_WINDOW_MESSAGE_DELEGATE_H_
#define BRICK_MESSAGE_DELEGATE_APP_WINDOW_MESSAGE_DELEGATE_H_
#pragma once

#include "brick/client_handler.h"

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
  static void CreateProcessMessageDelegates(ClientHandler::ProcessMessageDelegateSet *delegates);

IMPLEMENT_REFCOUNTING(AppWindowMessageDelegate);
};

#endif  // BRICK_MESSAGE_DELEGATE_APP_WINDOW_MESSAGE_DELEGATE_H_
