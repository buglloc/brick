#ifndef BRICK_EXTERNAL_APP_MESSAGE_DELEGATE_H_
#define BRICK_EXTERNAL_APP_MESSAGE_DELEGATE_H_

#include "external_message_delegate.h"

class ExternalAppMessageDelegate : public ExternalMessageDelegate {
public:
  ExternalAppMessageDelegate();

  virtual bool OnMessageReceived(CefRefPtr<CefProcessMessage> message) OVERRIDE;

IMPLEMENT_REFCOUNTING(ExternalAppMessageDelegate);
};

#endif // BRICK_EXTERNAL_APP_MESSAGE_DELEGATE_H_