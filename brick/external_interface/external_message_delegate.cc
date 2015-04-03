#include "external_message_delegate.h"

ExternalMessageDelegate::ExternalMessageDelegate(std::string message_namespace)
  : message_namespace_(message_namespace) {

};

bool
ExternalMessageDelegate::OnMessageReceived(
   CefRefPtr<CefProcessMessage> message) {

  return false;
}

bool
ExternalMessageDelegate::IsAcceptedNamespace(std::string namespace_name) {
  if (message_namespace_.empty())
    return false;

  if (message_namespace_.size() > namespace_name.size())
    return false;

  return std::equal(
     namespace_name.begin() + namespace_name.size() - message_namespace_.size(),
     namespace_name.end(), message_namespace_.begin()
  );
}