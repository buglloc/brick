#ifndef BRICK_EXTERNAL_APP_WINDOW_MESSAGE_DELEGATE_H_
#define BRICK_EXTERNAL_APP_WINDOW_MESSAGE_DELEGATE_H_

#include "external_message_delegate.h"

class ExternalAppWindowMessageDelegate : public ExternalMessageDelegate {
public:
  ExternalAppWindowMessageDelegate();

  virtual bool OnMessageReceived(CefRefPtr<CefProcessMessage> message) OVERRIDE;

IMPLEMENT_REFCOUNTING(ExternalAppWindowMessageDelegate);
};

#endif // BRICK_EXTERNAL_APP_WINDOW_MESSAGE_DELEGATE_H_