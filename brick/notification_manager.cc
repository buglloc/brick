// Copyright (c) 2015 The Brick Authors.

#include "brick/notification_manager.h"

#include "brick/client_handler.h"

NotificationManager::NotificationManager() :
    notification_(nullptr),
    last_id_(0),
    is_append_supported_(false),
    is_actions_supported_(false) {

  InitializeCapabilities();
}

void
NotificationManager::OnClose() {
  notification_ = nullptr;
}

void
NotificationManager::OnClick() {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  if (!client_handler)
    return;

  CefRefPtr<BrowserWindow> window = client_handler->GetMainWindowHandle();
  if (!window)
    return;

  window->Present();
}
