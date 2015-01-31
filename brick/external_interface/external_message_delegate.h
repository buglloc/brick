#ifndef BRICK_EXTERNAL_MESSAGE_DELEGATE_H_
#define BRICK_EXTERNAL_MESSAGE_DELEGATE_H_

#include <include/cef_base.h>
#include <include/cef_process_message.h>
#include "../api_error.h"
class ExternalMessageDelegate : public virtual CefBase {
public:
  ExternalMessageDelegate(std::string message_namespace);

  // Called when a message is received. Return true if the message was
  // handled and should not be passed on to other handlers.
  // ExternalMessageDelegates should check for unique message names to avoid
  // interfering with each other.
  virtual bool OnMessageReceived(CefRefPtr<CefProcessMessage> message);

  virtual bool IsAcceptedNamespace(std::string namespace_name);

protected:
  std::string message_namespace_;

  IMPLEMENT_REFCOUNTING(ExternalMessageDelegate);
};

#endif // BRICK_EXTERNAL_MESSAGE_DELEGATE_H_