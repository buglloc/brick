// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EXTERNAL_INTERFACE_APP_MESSAGE_DELEGATE_H_
#define BRICK_EXTERNAL_INTERFACE_APP_MESSAGE_DELEGATE_H_

#include "brick/external_interface/external_message_delegate.h"

class ExternalAppMessageDelegate : public ExternalMessageDelegate {
 public:
  ExternalAppMessageDelegate();

  virtual bool OnMessageReceived(CefRefPtr<CefProcessMessage> message) OVERRIDE;

IMPLEMENT_REFCOUNTING(ExternalAppMessageDelegate);
};

#endif  // BRICK_EXTERNAL_INTERFACE_APP_MESSAGE_DELEGATE_H_
