// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_V8_HANDLER_H_
#define BRICK_V8_HANDLER_H_
#pragma once

#include <string>

#include "include/cef_v8.h"
#include "include/cef_client.h"
#include "brick/client_app.h"
#include "brick/client_handler.h"

class V8Handler : public CefV8Handler {
 public:
  explicit V8Handler(CefRefPtr<ClientApp> cef_app)
     : cef_app_(cef_app)
     , messageId(0) {
  }

  virtual bool Execute(const CefString& name,
     CefRefPtr<CefV8Value> object,
     const CefV8ValueList& arguments,
     CefRefPtr<CefV8Value>& retval,
     CefString& exception) OVERRIDE;

 private:
    CefRefPtr<ClientApp> cef_app_;
    int32 messageId;

IMPLEMENT_REFCOUNTING(V8Handler);
};
#endif  // BRICK_V8_HANDLER_H_
