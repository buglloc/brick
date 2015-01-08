// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include <list>
#include <map>
#include <set>
#include "status_icon/status_icon.h"

#include "include/cef_client.h"

#include "window/main_window.h"
#include "command_callbacks.h"
#include "account_manager.h"

class ClientHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefDialogHandler,
                      public CefContextMenuHandler,
                      public CefRequestHandler {
public:
  // Interface for process message delegates. Do not perform work in the
  // RenderDelegate constructor.
  class ProcessMessageDelegate : public virtual CefBase {
  public:
    ProcessMessageDelegate(const char* message_namespace)
       : message_namespace_(message_namespace) {};

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

  void SetAccountManager(CefRefPtr<AccountManager> account_manager);
  CefRefPtr<AccountManager> GetAccountManager() const;

  void SetMainWindowHandle(CefRefPtr<MainWindow> handle);
  CefRefPtr<MainWindow> GetMainWindowHandle() const;

  void SetStatusIconHandle(CefRefPtr<StatusIcon> handle);
  CefRefPtr<StatusIcon> GetStatusIconHandle() const;

  // Provide access to the single global instance of this object.
  static CefRefPtr<ClientHandler> GetInstance();

  // CefClient methods:
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
  virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
     FileDialogMode mode,
     const CefString& title,
     const CefString& default_file_name,
     const std::vector<CefString>& accept_types,
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

  // CefDisplayHandler methods:
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
     const CefString &title) OVERRIDE;

  // CefLifeSpanHandler methods:
  // CefLifeSpanHandler methods
  virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
     CefRefPtr<CefFrame> frame,
     const CefString &target_url,
     const CefString &target_frame_name,
     const CefPopupFeatures &popupFeatures,
     CefWindowInfo &windowInfo,
     CefRefPtr<CefClient> &client,
     CefBrowserSettings &settings,
     bool *no_javascript_access) OVERRIDE;

  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefLoadHandler methods:
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

  bool IsClosing() const {
    return is_closing_;
  }

  void ShowDevTools(CefRefPtr<CefBrowser> browser,
     const CefPoint &inspect_element_at);

  void CloseDevTools(CefRefPtr<CefBrowser> browser);

  virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
     CefProcessId source_process,
     CefRefPtr<CefProcessMessage> message)
     OVERRIDE;

  bool IsAllowedUrl(std::string url);

protected:
  // Create all of ProcessMessageDelegate objects.
  static void CreateProcessMessageDelegates(
     ProcessMessageDelegateSet &delegates);

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

  bool is_closing_;

  // The main frame window handle.
  CefRefPtr<MainWindow> main_handle_;

  CefRefPtr<StatusIcon> status_icon_handle_;

  CefRefPtr<AccountManager> account_manager_;

  // Include the default reference counting implementation.
IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
