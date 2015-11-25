// Copyright (c) 2015 The Brick Authors.

#include "brick/client_app.h"

#include <iomanip>

#include "include/wrapper/cef_helpers.h"
#include "brick/client_handler.h"
#include "brick/v8_handler.h"
#include "brick/helper.h"
#include "brick/platform_util.h"

extern char _binary_desktop_extension_js_start;
extern char _binary_desktop_extension_js_size;

ClientApp::ClientApp()
  : device_scale_factor_(1.0) {
//  CreateRenderDelegates(render_delegates_);
}

void
ClientApp::OnRegisterCustomSchemes(
    CefRefPtr<CefSchemeRegistrar> registrar) {

  // Default schemes that support cookies.
  cookieable_schemes_.push_back("http");
  cookieable_schemes_.push_back("https");
}

void
ClientApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

}

void
ClientApp::OnWebKitInitialized() {
  // Define the extension contents.
  std::string extensionCode = GetExtensionJSSource();
  // Create an instance of my CefV8Handler object.
  CefRefPtr<CefV8Handler> handler = new V8Handler(this);

  // Register the extension.
  CefRegisterExtension("app", extensionCode, handler);
}

bool
ClientApp::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {

  ASSERT(source_process == PID_BROWSER);
  bool handled = false;

  // Execute delegate callbacks.
  for (const auto &delegate : render_delegates_) {
    handled = delegate->OnProcessMessageReceived(this, browser, source_process, message);
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
        for (unsigned int i = 1; i < messageArgs->GetSize(); i++) {
          arguments.push_back(helper::ListValueToV8Value(messageArgs, i));
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


bool
ClientApp::OnBeforeNavigation(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    NavigationType navigation_type,
    bool is_redirect) {

  if (is_redirect)
    return true;  // Disable redirects at all

  if (navigation_type == NAVIGATION_LINK_CLICKED) {
    // We have single page app, so we open all links and redirects in external browser
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("openExternal");
    message->GetArgumentList()->SetString(0, request->GetURL());
    browser->SendProcessMessage(PID_BROWSER, message);
    return true;
  }


  RenderDelegateSet::iterator it = render_delegates_.begin();
  for (; it != render_delegates_.end(); ++it) {
    if ((*it)->OnBeforeNavigation(this, browser, frame, request,
       navigation_type, is_redirect)) {
      return true;
    }
  }

  return false;
}

std::string
ClientApp::GetExtensionJSSource() {
  //# We objcopy the desktop_extensions.js file, and link it directly into the binary.
  //# See http://www.linuxjournal.com/content/embedding-file-executable-aka-hello-world-version-5967
  //# And look at BRICK_EMBED_FILES on CMake
  const char* p = &_binary_desktop_extension_js_start;
  unsigned long size = (unsigned long)&_binary_desktop_extension_js_size;

  std::string content(p, size);
  return helper::stringReplace(
     content, "#VERSION#", VERSION
  );
}

void
ClientApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) {
  command_line->AppendSwitch("enable-media-stream");
  // After update to CEF3 2357 we have some strange issues with GPU-procceses. Try to temporary disable it...
  command_line->AppendSwitch("disable-gpu");
  command_line->AppendSwitch("disable-gpu-compositing");
  // We don't need any plugins, such as PDF viewer
  command_line->AppendSwitch("disable-pdf-extension");

  if (!command_line->HasSwitch("force-device-scale-factor") && device_scale_factor_ > 1.0) {
    // Chromium hack for HiDPI "supporting", https://code.google.com/p/chromium/issues/detail?id=143619
    // TODO(buglloc): Recheck this ugly solution, when Brick reached Chromium 43+
    LOG(INFO) << "Using device scale factor: " << device_scale_factor_;
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed << device_scale_factor_;
    command_line->AppendSwitchWithValue("force-device-scale-factor", ss.str());
  }
}

void
ClientApp::SetDeviceScaleFactor(const double scale) {
  device_scale_factor_ = scale;
}
