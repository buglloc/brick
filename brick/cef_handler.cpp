// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef_handler.h"


#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "message_delegate/app_message_delegate.h"
#include "message_delegate/app_window_message_delegate.h"
#include "helper.h"
#include "resource_util.h"
#include "platform_util.h"
#include "window_util.h"


namespace {

    CefRefPtr<ClientHandler> g_instance = NULL;
    const char kInterceptionPath[] = "/desktop_app/internals/";
    std::string kScriptLoader = "appWindow.loadScript('#URL#');";
    std::string kUnknownInternalContent = "Failed to load resource";

} // namespace

ClientHandler::ClientHandler()
    : is_idle_ (false),
      main_handle_ (NULL),
      status_icon_handle_ (NULL),
      account_manager_ (NULL)
{
  DCHECK(!g_instance);
  g_instance = this;
  callbackId = 0;
  CreateProcessMessageDelegates(process_message_delegates_);
  RegisterSystemEventListeners();
}

ClientHandler::~ClientHandler() {
  g_instance = NULL;
}

// static
CefRefPtr<ClientHandler>
ClientHandler::GetInstance() {
  return g_instance;
}

void
ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  if (!GetBrowser()) {
    base::AutoLock lock_scope(lock_);
    // We need to keep the main child window, but not popup windows
    browser_ = browser;
    browser_id_ = browser->GetIdentifier();
    main_handle_ = new BrowserWindow;
    main_handle_->WrapNative(browser->GetHost()->GetWindowHandle());
  } else if (browser->IsPopup()) {
    // Add to the list of popup browsers.
//    popup_browsers_.push_back(browser);

    CefRefPtr<BrowserWindow> window(new BrowserWindow);
    window->WrapNative(browser->GetHost()->GetWindowHandle());
    window->Popupping();

    // Give focus to the popup browser. Perform asynchronously because the
    // parent window may attempt to keep focus after launching the popup.
    CefPostTask(TID_UI,
       base::Bind(&CefBrowserHost::SetFocus, browser->GetHost().get(), true));
  }
  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool
ClientHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  if (app_settings_.hide_on_delete && !browser->IsPopup()) {
    // Doesn't close main window
    main_handle_->Hide();
    // ToDo: Remove!
    // Browser will losses mostly all event handlers...so we do some ugly hack with reloading here
    browser->Reload();
    return true;
  }
  return false;
}

void
ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  } else if (!browser->IsPopup()) {
    // If we close main window - close all popups...
    CloseAllBrowsers(true);
  }
}

void
ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  std::stringstream error_explain;
  error_explain << std::string(failedUrl) <<
     ", with error" << std::string(errorText) <<
     " (" << errorCode << ")";

  LOG(INFO) << "Failed to load URL:" << error_explain.str();

  frame->LoadURL(GetAccountManager()->GetCurrentAccount()->GetBaseUrl() + "internals/pages/offline");
}

void
ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
   CefRefPtr<CefFrame> frame,
   int httpStatusCode) {

  if (httpStatusCode != 200 || !frame->IsMain())
    return; // We need only successful loaded main frame

#if 0
  frame->ExecuteJavaScript(
     helper::string_replace(kScriptLoader, "#URL#", "http://ya.ru"),
     "",
     0
  );
#endif
  // Fix IM autoresize
  std::string code =
    "(function(window) {"
      "var event = document.createEvent('UIEvents');"
      "event.initUIEvent('resize', true, false, window, 0);"
      "window.dispatchEvent(event);"
    "})(window)";

  frame->ExecuteJavaScript(
     code,
     "",
     0
  );
}

void
ClientHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
        base::Bind(&ClientHandler::CloseAllBrowsers, this, force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
    (*it)->GetHost()->CloseBrowser(force_close);
}

void
ClientHandler::CloseAllPopups(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
       base::Bind(&ClientHandler::CloseAllBrowsers, this, force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it) {
    if (!(*it)->IsPopup())
      continue;

    (*it)->GetHost()->CloseBrowser(force_close);
  }

}

void
ClientHandler::OnBeforeContextMenu(
   CefRefPtr<CefBrowser> browser,
   CefRefPtr<CefFrame> frame,
   CefRefPtr<CefContextMenuParams> params,
   CefRefPtr<CefMenuModel> model) {
  CEF_REQUIRE_UI_THREAD();

  if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0) {
    // Add a separator if the menu already has items.
    if (model->GetCount() > 0)
      model->Clear();

#ifndef NDEBUG
    // Add DevTools items to all context menus in debug build.
    model->AddItem(1, "&Show DevTools");
    model->AddItem(2, "Close DevTools");
    model->AddSeparator();
    model->AddItem(3, "Inspect Element");
#endif
  }
}

bool
ClientHandler::OnContextMenuCommand(
   CefRefPtr<CefBrowser> browser,
   CefRefPtr<CefFrame> frame,
   CefRefPtr<CefContextMenuParams> params,
   int command_id,
   EventFlags event_flags) {
  CEF_REQUIRE_UI_THREAD();

#ifndef NDEBUG
  switch (command_id) {
    case 1:
      ShowDevTools(browser, CefPoint());
      return true;
    case 2:
      CloseDevTools(browser);
      return true;
    case 3:
      ShowDevTools(browser, CefPoint(params->GetXCoord(), params->GetYCoord()));
      return true;
    default:
      return false;
  }

#else
  return false;
#endif
}

void
ClientHandler::ShowDevTools(CefRefPtr<CefBrowser> browser,
   const CefPoint& inspect_element_at) {
  CefWindowInfo windowInfo;
  CefBrowserSettings settings;

#if defined(OS_WIN)
  windowInfo.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
#endif

  browser->GetHost()->ShowDevTools(windowInfo, this, settings,
     inspect_element_at);
}

void
ClientHandler::CloseDevTools(CefRefPtr<CefBrowser> browser) {
  browser->GetHost()->CloseDevTools();
}


bool
ClientHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
   CefRefPtr<CefFrame> frame,
   const CefString& target_url,
   const CefString& target_frame_name,
   const CefPopupFeatures& popupFeatures,
   CefWindowInfo& windowInfo,
   CefRefPtr<CefClient>& client,
   CefBrowserSettings& settings,
   bool* no_javascript_access) {
  CEF_REQUIRE_IO_THREAD();

  std::string url = target_url;
  if (!IsAllowedUrl(url)) {
    platform_util::OpenExternal(url);
    return true;
  }

  // Temporary&Ugly hack for strange IM settings window size behavior. Most likely caused by asynchronous X11.
  // ToDo: Research&Fix!

  windowInfo.width = 567;
  windowInfo.height = 335;

  return false;
}

bool
ClientHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
   CefRefPtr<CefFrame> frame,
   CefRefPtr<CefRequest> request,
   bool is_redirect) {

  std::string url = request->GetURL();
  if (!IsAllowedUrl(url)) {
    platform_util::OpenExternal(url);
    return true;
  }

  return false;
}

CefRefPtr<CefResourceHandler>
ClientHandler::GetResourceHandler(
   CefRefPtr<CefBrowser> browser,
   CefRefPtr<CefFrame> frame,
   CefRefPtr<CefRequest> request) {
  CEF_REQUIRE_IO_THREAD();

  std::string url = request->GetURL();
  if (url.find(kInterceptionPath) == std::string::npos)
    return NULL;

  // Handle URLs in interception path
  std::string file_name, mime_type;
  if (helper::ParseUrl(url, &file_name, &mime_type)) {
    // Remove interception path
    file_name = file_name.substr(strlen(kInterceptionPath) - 1);
    // Load the resource from file.
    CefRefPtr<CefStreamReader> stream =
       GetBinaryResourceReader(app_settings_.resource_dir, file_name.c_str());
    if (stream.get())
      return new CefStreamResourceHandler(mime_type, stream);
  }

  // Never let the internal links to the external world
  CefRefPtr<CefStreamReader> stream =
     CefStreamReader::CreateForData(
        static_cast<void*>(const_cast<char*>(kUnknownInternalContent.c_str())),
        kUnknownInternalContent.size()
     );
  ASSERT(stream.get());
  return new CefStreamResourceHandler("text/plain", stream);
}

bool
ClientHandler::OnProcessMessageReceived(
   CefRefPtr<CefBrowser> browser,
   CefProcessId source_process,
   CefRefPtr<CefProcessMessage> message) {
  bool handled = false;

  if (message->GetName() == "executeCommandCallback") {
    // Check for callbacks first
    int32 commandId = message->GetArgumentList()->GetInt(0);
    bool result = message->GetArgumentList()->GetBool(1);

    CefRefPtr<CommandCallback> callback = command_callback_map_[commandId];
    callback->CommandComplete(result);
    command_callback_map_.erase(commandId);

    handled = true;

  } else if (message->GetName() == "openExternal") {
    std::string url = message->GetArgumentList()->GetString(0);
    platform_util::OpenExternal(url);
    handled = true;
  }

  std::string message_name = message->GetName();
  // Execute delegate callbacks.
  ProcessMessageDelegateSet::iterator it = process_message_delegates_.begin();
  for (; it != process_message_delegates_.end() && !handled; ++it) {
    if ((*it)->IsAcceptedNamespace(message_name)) {
      handled = (*it)->OnProcessMessageReceived(
         this,
         browser,
         source_process,
         message
      );
    }
  }

  if (!handled)
    LOG(WARNING) << "Unknown proccess message: " << message_name;

  return handled;
}

void
ClientHandler::SetAccountManager(CefRefPtr<AccountManager> account_manager) {
  account_manager_ = account_manager;
}

CefRefPtr<AccountManager>
ClientHandler::GetAccountManager() const {
  return account_manager_;
}

CefRefPtr<BrowserWindow>
ClientHandler::GetMainWindowHandle() const {
  CEF_REQUIRE_UI_THREAD();
  return main_handle_;
}

void
ClientHandler::SetCacheManager(CefRefPtr<CacheManager> cache_manager) {
  cache_manager_ = cache_manager;
}

CefRefPtr<CacheManager>
ClientHandler::GetCacheManager() const {
  return cache_manager_;
}

void
ClientHandler::SetAppSettings(AppSettings settings) {
 app_settings_ = settings;
}

AppSettings
ClientHandler::GetAppSettings() const {
  return app_settings_;
}

void
ClientHandler::SetStatusIconHandle(CefRefPtr<StatusIcon> handle) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
       base::Bind(&ClientHandler::SetStatusIconHandle, this, handle));
    return;
  }

  status_icon_handle_ = handle;
}

CefRefPtr<StatusIcon>
ClientHandler::GetStatusIconHandle() const {
  CEF_REQUIRE_UI_THREAD();
  return status_icon_handle_;
}

CefRefPtr<CefBrowser> ClientHandler::GetBrowser() const {
  base::AutoLock lock_scope(lock_);
  return browser_;
}

int
ClientHandler::GetBrowserId() const {
  base::AutoLock lock_scope(lock_);
  return browser_id_;
}

bool
ClientHandler::IsAllowedUrl(std::string url) {
  if (
    // Ugly temporary fix for file downloading...
    // ToDo: fix it!
    url.find("/desktop_app/download.file.php") != std::string::npos
    || url.find("/desktop_app/show.file.php") != std::string::npos
  )
    return false;

  return (
     account_manager_->GetCurrentAccount()->CheckBaseUrl(url)
     || url.find("chrome-devtools://") == 0
  );
}

void
ClientHandler::SwitchAccount(int id) {
  CloseAllPopups(true);
  // ToDo: delete host/domain cookies here!!!
  account_manager_->SwitchAccount(id);
  browser_->GetMainFrame()->LoadURL(
     account_manager_->GetCurrentAccount()->GetBaseUrl() + "internals/pages/portal-loader#login=yes"
  );
  account_manager_->Commit();
}

bool
ClientHandler::SendJSEvent(CefRefPtr<CefBrowser> browser, const CefString& name, const CefString& data)
{
  CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("dispatchEvent");
  message->GetArgumentList()->SetString(0, name);
  if (!data.empty())
    message->GetArgumentList()->SetString(1, data);

  browser->SendProcessMessage(PID_RENDERER, message);

  return true;
}

void
ClientHandler::onEvent(UserAwayEvent &event) {
  is_idle_ = event.isAway();
  if (event.isManual())
    idle_pending_ = false;

  CefRefPtr<CefBrowser> browser = GetBrowser();
  if (!browser)
    return;

  SendJSEvent(browser, "BXUserAway", is_idle_? "[true]": "[false]");
}

void
ClientHandler::onEvent(SleepEvent &event) {
  CefRefPtr<CefBrowser> browser = GetBrowser();
  if (!browser)
    return;

  if (event.isSleep()) {
    SendJSEvent(browser, "BXSleepAction");
  } {
    SendJSEvent(browser, "BXWakeAction");
  }

}

void
ClientHandler::RegisterSystemEventListeners() {
  EventBus::AddHandler<UserAwayEvent>(*this);
  EventBus::AddHandler<SleepEvent>(*this);
}

// static
void
ClientHandler::CreateProcessMessageDelegates(ProcessMessageDelegateSet& delegates) {
  AppMessageDelegate::CreateProcessMessageDelegates(delegates);
  AppWindowMessageDelegate::CreateProcessMessageDelegates(delegates);
}