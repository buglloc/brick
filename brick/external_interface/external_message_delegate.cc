// Copyright (c) 2015 The Brick Authors.

#include "brick/external_interface/external_message_delegate.h"

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
