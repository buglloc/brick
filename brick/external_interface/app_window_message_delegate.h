// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EXTERNAL_INTERFACE_APP_WINDOW_MESSAGE_DELEGATE_H_
#define BRICK_EXTERNAL_INTERFACE_APP_WINDOW_MESSAGE_DELEGATE_H_

#include "brick/external_interface/external_message_delegate.h"

class ExternalAppWindowMessageDelegate : public ExternalMessageDelegate {
 public:
  ExternalAppWindowMessageDelegate();

  virtual bool OnMessageReceived(CefRefPtr<CefProcessMessage> message) OVERRIDE;

IMPLEMENT_REFCOUNTING(ExternalAppWindowMessageDelegate);
};

#endif  // BRICK_EXTERNAL_INTERFACE_APP_WINDOW_MESSAGE_DELEGATE_H_
