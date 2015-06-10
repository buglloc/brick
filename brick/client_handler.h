// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_CLIENT_HANDLER_H_
#define BRICK_CLIENT_HANDLER_H_
#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include "brick/common/app_settings.h"
#include "brick/window/window_features.h"
#include "brick/window/browser_window.h"
#include "brick/event/user_away_event.h"
#include "brick/event/sleep_event.h"
#include "brick/indicator/indicator.h"

#include "include/cef_client.h"

#include "brick/cache_manager.h"
#include "brick/command_callbacks.h"
#include "brick/account_manager.h"
#include "brick/notification_manager.h"
#include "brick/api_error.h"


class ClientHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefDialogHandler,
                      public CefContextMenuHandler,
                      public CefRequestHandler,
                      public EventHandler<UserAwayEvent>,
                      public EventHandler<SleepEvent> {
 public:
  // Interface for process message delegates. Do not perform work in the
  // RenderDelegate constructor.
  class ProcessMessageDelegate : public virtual CefBase {
   public:
    explicit ProcessMessageDelegate(const char* message_namespace)
       : message_namespace_(message_namespace) {}

    // Called when a process message is received. Return true if the message was
    // handled and should not be passed on to other handlers.
    // ProcessMessageDelegates should check for unique message names to avoid
    // interfering with each other.
    virtual bool OnProcessMessageReceived(
       CefRefPtr<ClientHandler> handler,
       CefRefPtr<CefBrowser> browser,
       CefProcessId source_process,
       CefRefPtr<CefProcessMessage> message) {
      return false;
    }

    virtual bool IsAcceptedNamespace(std::string message_name) {
      return (
         message_namespace_
         && message_name.find(message_namespace_) == 0);
    }

   protected:
    const char* message_namespace_;

  };

  typedef std::set<CefRefPtr<ProcessMessageDelegate> >
     ProcessMessageDelegateSet;

  ClientHandler();
  ~ClientHandler();

  void SetCacheManager(CefRefPtr<CacheManager> cache_manager);
  CefRefPtr<CacheManager> GetCacheManager() const;

  void SetAccountManager(CefRefPtr<AccountManager> account_manager);
  CefRefPtr<AccountManager> GetAccountManager() const;

  void SetNotificationManager(CefRefPtr<NotificationManager> manager);
  CefRefPtr<NotificationManager> GetNotificationManager() const;

  void SetAppSettings(AppSettings settings);
  AppSettings GetAppSettings() const;

  CefRefPtr<BrowserWindow> GetMainWindowHandle() const;

  void SetIndicatorHandle(CefRefPtr<BrickIndicator> handle);
  CefRefPtr<BrickIndicator> GetIndicatorHandle() const;

  // Provide access to the single global instance of this object.
  static CefRefPtr<ClientHandler> GetInstance();

  // CefClient methods
  virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefDialogHandler> GetDialogHandler() OVERRIDE {
    return this;
  }

  // CefDialogHandler methods
  bool OnFileDialog(CefRefPtr<CefBrowser> browser,
     FileDialogMode mode,
     const CefString& title,
     const CefString& default_file_path,
     const std::vector<CefString>& accept_filters,
     int selected_accept_filter,
     CefRefPtr<CefFileDialogCallback> callback) OVERRIDE;

  // CefContextMenuHandler methods
  virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
     CefRefPtr<CefFrame> frame,
     CefRefPtr<CefContextMenuParams> params,
     CefRefPtr<CefMenuModel> model) OVERRIDE;

  virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
     CefRefPtr<CefFrame> frame,
     CefRefPtr<CefContextMenuParams> params,
     int command_id,
     EventFlags event_flags) OVERRIDE;

  // CefDisplayHandler methods
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
     const CefString &title) OVERRIDE;

  // CefLifeSpanHandler methods
  bool OnBeforePopup(
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
      bool* no_javascript_access) OVERRIDE;

  virtual void OnWindowCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

  virtual bool OnCloseBrowser(CefRefPtr<CefBrowser> browser) OVERRIDE;

  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefLoadHandler methods
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
     CefRefPtr<CefFrame> frame,
     ErrorCode errorCode,
     const CefString &errorText,
     const CefString &failedUrl) OVERRIDE;
  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
     CefRefPtr<CefFrame> frame,
     int httpStatusCode) OVERRIDE;

  // CefRequestHandler methods
  virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
     CefRefPtr<CefFrame> frame,
     CefRefPtr<CefRequest> request,
     bool is_redirect) OVERRIDE;
  virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
     CefRefPtr<CefBrowser> browser,
     CefRefPtr<CefFrame> frame,
     CefRefPtr<CefRequest> request) OVERRIDE;

  CefRefPtr<CefBrowser> GetBrowser() const;

  int GetBrowserId() const;

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);
  // Request that all existing popup windows close.
  void CloseAllPopups(bool force_close);

  void SetIdle(bool is_idle) {
    is_idle_ = is_idle;
  }

  bool IsIdle() const {
    return is_idle_;
  }

  // ToDo: move this logic to system events handler
  void SetIdlePending(bool pending) {
    idle_pending_ = pending;
  }

  bool IsIdlePending() const {
    return idle_pending_;
  }

  void ShowDevTools(CefRefPtr<CefBrowser> browser,
     const CefPoint &inspect_element_at);

  void CloseDevTools(CefRefPtr<CefBrowser> browser);

  virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
     CefProcessId source_process,
     CefRefPtr<CefProcessMessage> message)
     OVERRIDE;

  bool IsAllowedUrl(std::string url);

  void SwitchAccount(int id);

  bool SendJSEvent(CefRefPtr<CefBrowser> browser, const CefString& name, const CefString& data = "");

  // System event handlers
  void RegisterSystemEventListeners();
  virtual void onEvent(const UserAwayEvent& event) OVERRIDE;
  virtual void onEvent(const SleepEvent& event) OVERRIDE;

  std::string AddTemporaryPage(const std::string& content);

  bool InShutdownState();
  void Shutdown(bool force);
  void PreventShutdown();

 protected:
  // Create all of ProcessMessageDelegate objects.
  static void CreateProcessMessageDelegates(
     ProcessMessageDelegateSet *delegates);

 private:
  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
  BrowserList browser_list_;

  // Lock used to protect members accessed on multiple threads. Make it mutable
  // so that it can be used from const methods.
  mutable base::Lock lock_;

  // START LOCK PROTECTED MEMBERS
  // The following members are accessed on multiple threads and must be
  // protected by |lock_|.

  // The child browser window.
  CefRefPtr<CefBrowser> browser_;

  // The child browser id.
  int browser_id_;

  // Registered delegates.
  ProcessMessageDelegateSet process_message_delegates_;

  typedef std::map<int32, CefRefPtr<CommandCallback> > CommandCallbackMap;
  int32 callbackId;
  CommandCallbackMap command_callback_map_;

  bool is_idle_;
  bool idle_pending_;

  // The main frame window handle.
  CefRefPtr<BrowserWindow> main_handle_;

  CefRefPtr<BrickIndicator> indicator_handle_;

  CefRefPtr<AccountManager> account_manager_;

  CefRefPtr<NotificationManager> notification_manager_;

  CefRefPtr<CacheManager> cache_manager_;

  AppSettings app_settings_;

  typedef std::map<std::string, std::string> TemporaryPageMap;
  TemporaryPageMap temporary_page_map_;
  int32 last_temporary_page_;

  CefRefPtr<WindowFeatures> last_popup_features_;
  bool in_shutdown_state_;
#ifdef __linux__
  guint shutdown_timer_id_;
#endif

  // Include the default reference counting implementation.
IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif  // BRICK_CLIENT_HANDLER_H_
