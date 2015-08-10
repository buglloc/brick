// Copyright (c) 2015 The Brick Authors.

#include "brick/client_handler.h"

#include "third-party/json/json.h"

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_stream_resource_handler.h"

#include "brick/message_delegate/app_message_delegate.h"
#include "brick/message_delegate/app_window_message_delegate.h"
#include "brick/helper.h"
#include "brick/resource/resource_util.h"
#include "brick/resource/last_resource_provider.h"
#include "brick/resource/injected_js_resource_provider.h"
#include "brick/resource/temporary_page_resource_provider.h"
#include "brick/resource/desktop_media_resource_provider.h"
#include "brick/platform_util.h"
#include "brick/window_util.h"
#include "brick/brick_app.h"
#include "brick/desktop_media.h"

namespace {

  CefRefPtr<ClientHandler> g_instance = NULL;
  const char kResourcesPath[]         = "/desktop_app/internals/";
  const char kWebResourcePath[]       = "/web/";
  const char kTemporaryPagePath[]     = "/temp-pages/";
  const char kInjectedJsPath[]        = "/injected-js/";
  const char kDesktopMediaPath[]      = "/desktop-media/";

}  // namespace

ClientHandler::ClientHandler()
    : is_idle_(false),
      main_handle_(NULL),
      indicator_handle_(NULL),
      account_manager_(NULL),
      last_temporary_page_(0),
      in_shutdown_state_(false),
#ifdef __linux__
      shutdown_timer_id_(0)
#endif
{
  DCHECK(!g_instance);
  g_instance = this;
  callbackId = 0;
}

ClientHandler::~ClientHandler() {
  g_instance = NULL;
}

// static
CefRefPtr<ClientHandler>
ClientHandler::GetInstance() {
  return g_instance;
}

bool
ClientHandler::Initialize() {
  CreateProcessMessageDelegates(&process_message_delegates_);
  RegisterSystemEventListeners();
  SetupResourceManager();
  return true;
}

void
ClientHandler::OnWindowCreated(CefRefPtr<CefBrowser> browser) {
  CefWindowHandle window = browser->GetHost()->GetWindowHandle();
  if (browser->IsPopup()) {
    window_util::InitWindow(window, false);
    if (last_popup_features_.get() && last_popup_features_->topmost) {
      window_util::ConfigureAsTopmost(window);
    } else {
      window_util::ConfigureAsDialog(window);
    }
  } else {
    window_util::InitWindow(window, true);
  }
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
    // popup_browsers_.push_back(browser);

    CefRefPtr<BrowserWindow> window(new BrowserWindow);
    window->WrapNative(browser->GetHost()->GetWindowHandle());
    if (last_popup_features_.get()) {
      if (last_popup_features_->topmost)
        window->Stick();
    }
    last_popup_features_ = NULL;

    // Give focus to the popup browser. Perform asynchronously because the
    // parent window may attempt to keep focus after launching the popup.
    CefPostTask(TID_UI,
       base::Bind(&CefBrowserHost::SetFocus, browser->GetHost().get(), true));
  }
  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool
ClientHandler::OnCloseBrowser(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  bool handled = false;
  if (browser->IsPopup()) {
    CefWindowHandle cef_window = browser->GetHost()->GetWindowHandle();
    BrowserWindow *window = window_util::LookupBrowserWindow(cef_window);
    handled = !window->IsClosable();
  } else if (app_settings_.hide_on_delete) {
    // Doesn't close main window
    main_handle_->Hide();
    handled = true;
  } else {
    // Shutdown application
    Shutdown(false);
    handled = true;
  }

  return handled;
}

void
ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::const_iterator bit = browser_list_.cbegin();
  for (; bit != browser_list_.cend(); ++bit) {
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

  frame->LoadURL(GetAccountManager()->GetCurrentAccount()->GetBaseUrl() + "internals/web/pages/offline");
}

void
ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    int httpStatusCode) {

  if (httpStatusCode != 200 || !frame->IsMain())
    return;  // We need only successful loaded main frame

  std::string injected_js = R"js((function(window) {
       window.addEventListener('BXExitApplication', function (e) {
          if (window.BX === void 0 || window.BX.desktop === void 0)
            window.BrickApp.shutdown();

       }, false);

       var event = document.createEvent('UIEvents');
       event.initUIEvent('resize', true, false, window, 0);
       window.dispatchEvent(event);
     })(window);
  ;)js";

  // ToDo: Use CefV8Value::ExecuteFunction? Maybe something like SendJSEvent...
  if (!app_settings_.client_scripts.empty()) {
    Json::Value client_scripts(Json::arrayValue);
    std::string url;
    for (const auto script: app_settings_.client_scripts) {
      url = kResourcesPath;
      // Without trailing slash
      url.append(kInjectedJsPath, 1, sizeof(kInjectedJsPath) - 1);
      url.append(script.first);
      client_scripts.append(url);
    }

    Json::FastWriter json_writer;
    injected_js += "if (typeof BX != 'undefined' && BrickApp.loadScripts !== void 0) BrickApp.loadScripts(";
    injected_js += json_writer.write(client_scripts);
    injected_js += ");";
  }

  frame->ExecuteJavaScript(
     injected_js,
     "ijected_js",
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

  for (const auto browser: browser_list_)
    browser->GetHost()->CloseBrowser(force_close);
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

  BrowserList::const_iterator it = browser_list_.cbegin();
  for (; it != browser_list_.cend(); ++it) {
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
ClientHandler::OnBeforePopup(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& target_url,
    const CefString& target_frame_name,
    CefLifeSpanHandler::WindowOpenDisposition target_disposition,
    bool user_gesture,
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

  windowInfo.width = static_cast<unsigned int>(popupFeatures.width);
  windowInfo.height = static_cast<unsigned int>(popupFeatures.height);

  // ToDo: Fix it!
  // For now we disable scaling popup width and height due to IM settings dialog issues
  // But we still must calculate right position for it

  // Calculate window placement
  if (!windowInfo.x && !windowInfo.y) {
    unsigned int width = static_cast<unsigned int>(windowInfo.width * window_util::GetDeviceScaleFactor());
    unsigned int height = static_cast<unsigned int>(windowInfo.height * window_util::GetDeviceScaleFactor());

    CefRect screen_rect = window_util::GetDefaultScreenRect();
    windowInfo.x = screen_rect.x + (screen_rect.width - width) / 2;
    windowInfo.y = screen_rect.y + (screen_rect.height - height) / 2;
  }

  // ToDo: R&D, too ugly hack to catch popup features in OnWindowCreated
  last_popup_features_ = new WindowFeatures(popupFeatures);
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

cef_return_value_t
ClientHandler::OnBeforeResourceLoad(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    CefRefPtr<CefRequestCallback> callback) {
  CEF_REQUIRE_IO_THREAD();

  std::string url = request->GetURL();
  if (url.find(kResourcesPath) == std::string::npos)
    return RV_CONTINUE;

  return resource_manager_->OnBeforeResourceLoad(browser, frame, request,
                                                 callback);
}

CefRefPtr<CefResourceHandler>
ClientHandler::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request) {
  CEF_REQUIRE_IO_THREAD();

  std::string url = request->GetURL();
  if (url.find(kResourcesPath) == std::string::npos)
    return NULL;

  return resource_manager_->GetResourceHandler(browser, frame, request);
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
  for (const auto delegate: process_message_delegates_) {
    if (delegate->IsAcceptedNamespace(message_name)) {
      handled = delegate->OnProcessMessageReceived(
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

void
ClientHandler::SetNotificationManager(CefRefPtr<NotificationManager> manager) {
  notification_manager_ = manager;
}

CefRefPtr<NotificationManager>
ClientHandler::GetNotificationManager() const {
  return notification_manager_;
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
ClientHandler::SetIndicatorHandle(CefRefPtr<BrickIndicator> handle) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
                base::Bind(&ClientHandler::SetIndicatorHandle, this, handle));
    return;
  }

  indicator_handle_ = handle;
}

CefRefPtr<BrickIndicator>
ClientHandler::GetIndicatorHandle() const {
  CEF_REQUIRE_UI_THREAD();
  return indicator_handle_;
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
  // Clear cookies
  CefCookieManager::GetGlobalManager(NULL)->DeleteCookies(
      account_manager_->GetCurrentAccount()->GetOrigin(),
      CefString(),
      NULL
  );

  last_temporary_page_ = 0;
  temporary_page_map_.clear();
  account_manager_->SwitchAccount(id);
  browser_->GetMainFrame()->LoadURL(
     account_manager_->GetCurrentAccount()->GetBaseUrl() + "internals/web/pages/portal-loader#login=yes"
  );
  account_manager_->Commit();
}

bool
ClientHandler::SendJSEvent(CefRefPtr<CefBrowser> browser, const CefString& name, const CefString& data) {
  CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("dispatchEvent");
  message->GetArgumentList()->SetString(0, name);
  if (!data.empty())
    message->GetArgumentList()->SetString(1, data);

  browser->SendProcessMessage(PID_RENDERER, message);

  return true;
}

void
ClientHandler::onEvent(const UserAwayEvent& event) {
  is_idle_ = event.isAway();
  if (event.isManual())
    idle_pending_ = false;

  CefRefPtr<CefBrowser> browser = GetBrowser();
  if (!browser)
    return;

  SendJSEvent(browser, "BXUserAway", is_idle_? "[true]": "[false]");
}

void
ClientHandler::onEvent(const SleepEvent& event) {
  CefRefPtr<CefBrowser> browser = GetBrowser();
  if (!browser)
    return;

  if (event.isSleep()) {
    SendJSEvent(browser, "BXSleepAction");
  } {
    SendJSEvent(browser, "BXWakeAction");
  }

}

bool
ClientHandler::InShutdownState() {
  return in_shutdown_state_;
}

void
ClientHandler::RegisterSystemEventListeners() {
  EventBus::AddHandler<UserAwayEvent>(*this);
  EventBus::AddHandler<SleepEvent>(*this);
}

std::string
ClientHandler::AddTemporaryPage(const std::string& content) {
  std::string url = kResourcesPath;
  // Without trailing slash
  url.append(kTemporaryPagePath, 1, sizeof(kTemporaryPagePath) - 1);
  std::string pageId = std::to_string(++last_temporary_page_) + ".html";
  url.append(pageId);
  temporary_page_map_[pageId] = content;

  return url;
}

void
ClientHandler::SetupResourceManager() {
  resource_manager_ = new CefResourceManager();
  // Add the URL filter.
  resource_manager_->SetUrlFilter(base::Bind(resource_util::UrlToResourcePath));

  // Desktop media preview (screen or window).
  resource_manager_->AddProvider(new DesktopMediaResourceProvider(kDesktopMediaPath), 97, "");

  // Temporary (or runtime) pages. Mostly used by topmost windows.
  resource_manager_->AddProvider(new TemporaryPageResourceProvider(kTemporaryPagePath, &temporary_page_map_), 98, "");

  // Read client scripts from config
  resource_manager_->AddProvider(new InjectedJsResourceProvider(kInjectedJsPath, &app_settings_.client_scripts), 99, "");

  // Read resources from a directory on disk.
  resource_manager_->AddDirectoryProvider(kWebResourcePath, app_settings_.resource_dir + "/web/", 100, "");

  // Never let the internal links walk on the external world
  resource_manager_->AddProvider(new LastResourceProvider, 1000, "");
}

// static
void
ClientHandler::CreateProcessMessageDelegates(ProcessMessageDelegateSet *delegates) {
  AppMessageDelegate::CreateProcessMessageDelegates(delegates);
  AppWindowMessageDelegate::CreateProcessMessageDelegates(delegates);
}
