// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef_app.h"

#include "cef_handler.h"
#include "v8_handler.h"
#include "include/wrapper/cef_helpers.h"
#include "helper.h"

extern char _binary_desktop_extension_js_start;
extern char _binary_desktop_extension_js_size;

ClientApp::ClientApp() {
//  CreateRenderDelegates(render_delegates_);
}

void ClientApp::OnRegisterCustomSchemes(
   CefRefPtr<CefSchemeRegistrar> registrar) {
  // Default schemes that support cookies.
  cookieable_schemes_.push_back("http");
  cookieable_schemes_.push_back("https");
}

void ClientApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

}

void ClientApp::OnWebKitInitialized() {
  // Define the extension contents.
  std::string extensionCode = GetExtensionJSSource();
  // Create an instance of my CefV8Handler object.
  CefRefPtr<CefV8Handler> handler = new V8Handler(this);

  // Register the extension.
  CefRegisterExtension("app", extensionCode, handler);
}

bool ClientApp::OnProcessMessageReceived(
   CefRefPtr<CefBrowser> browser,
   CefProcessId source_process,
   CefRefPtr<CefProcessMessage> message) {
  ASSERT(source_process == PID_BROWSER);

  bool handled = false;

  // Execute delegate callbacks.
  RenderDelegateSet::iterator it = render_delegates_.begin();
  for (; it != render_delegates_.end() && !handled; ++it) {
    handled = (*it)->OnProcessMessageReceived(this, browser, source_process, message);
  }

  if (!handled) {
    if (message->GetName() == "invokeCallback") {
      // This is called by the desktop extension handler to invoke the asynchronous
      // callback function

      CefRefPtr<CefListValue> messageArgs = message->GetArgumentList();
      int32 callbackId = messageArgs->GetInt(0);

      CefRefPtr<CefV8Context> context = callback_map_[callbackId].first;
      CefRefPtr<CefV8Value> callbackFunction = callback_map_[callbackId].second;
      CefV8ValueList arguments;
      context->Enter();

      // Sanity check to make sure the context is still attched to a browser.
      // Async callbacks could be initiated after a browser instance has been deleted,
      // which can lead to bad things. If the browser instance has been deleted, don't
      // invoke this callback.
      if (context->GetBrowser()) {
        for (size_t i = 1; i < messageArgs->GetSize(); i++) {
          arguments.push_back(helper::ListValueToV8Value(messageArgs, (int) i));
        }

        callbackFunction->ExecuteFunction(NULL, arguments);
      }

      context->Exit();

      callback_map_.erase(callbackId);

    } else if (message->GetName() == "dispatchEvent") {
      // This is called by the browser process to dispatch a Event to JavaScript
      //
      // The first argument is the event name. This is required.
      // The second argument is a custom event data. This is optional. Currently only string data supported :-(

      CefRefPtr<CefListValue> messageArgs = message->GetArgumentList();
      std::string eventName = messageArgs->GetString(0);
      std::string eventData = messageArgs->GetSize() > 1 ? messageArgs->GetString(1) : "[]";

      CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
      context->Enter();
      CefRefPtr<CefV8Value> global = context->GetGlobal();
      if (global->HasValue("BXDesktopWindow")) {
        CefRefPtr<CefV8Value> js_window = global->GetValue("BXDesktopWindow");
        CefRefPtr<CefV8Value> dispatcher = js_window->GetValue("DispatchCustomEvent");
        if (dispatcher->IsFunction()) {
          CefV8ValueList args;
          args.push_back(CefV8Value::CreateString(eventName));
          args.push_back(CefV8Value::CreateString(eventData));

          dispatcher->ExecuteFunction(global, args);
        }
      }

      context->Exit();
    }
  }

  return handled;
}


bool ClientApp::OnBeforeNavigation(CefRefPtr<CefBrowser> browser,
   CefRefPtr<CefFrame> frame,
   CefRefPtr<CefRequest> request,
   NavigationType navigation_type,
   bool is_redirect) {

  if (is_redirect)
    return true; // Disable redirects at all

  RenderDelegateSet::iterator it = render_delegates_.begin();
  for (; it != render_delegates_.end(); ++it) {
    if ((*it)->OnBeforeNavigation(this, browser, frame, request,
       navigation_type, is_redirect)) {
      return true;
    }
  }

  return false;
}

std::string ClientApp::GetExtensionJSSource() {
  //# We objcopy the desktop_extensions.js file, and link it directly into the binary.
  //# See http://www.linuxjournal.com/content/embedding-file-executable-aka-hello-world-version-5967
  //# And look at BRICK_EMBED_FILES on CMake
  const char* p = &_binary_desktop_extension_js_start;
  unsigned long size = (unsigned long)&_binary_desktop_extension_js_size;

  std::string content(p, size);
  return helper::string_replace(
     content, "#VERSION#", VERSION
  );
}

void
ClientApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) {
  command_line.get()->AppendSwitch("enable-media-stream");
}