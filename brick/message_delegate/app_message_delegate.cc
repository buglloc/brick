// Copyright (c) 2015 The Brick Authors.

#include "brick/message_delegate/app_message_delegate.h"

#include <error.h>

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "brick/brick_app.h"
#include "brick/brick_types.h"
#include "brick/window/edit_account_window.h"
#include "brick/helper.h"
#include "brick/notification_manager.h"
#include "brick/platform_util.h"
#include "brick/desktop_media.h"

namespace {
  const char kNameSpace[]                   = "AppEx";
  const char kMessageLoginName[]            = "Login";
  const char kMessageNavigateName[]         = "Navigate";
  const char kMessageBrowseName[]           = "Browse";
  const char kMessageShowLogFileName[]      = "ShowLogFile";
  const char kMessageChangeTooltipName[]    = "ChangeTooltip";
  const char kMessageSetIndicatorName[]     = "SetIndicator";
  const char kMessageSetIdleIndicatorName[] = "SetIdleIndicator";
  const char kMessageIndicatorBadgeName[]   = "IndicatorBadgee";
  const char kMessageShowNotificationName[] = "ShowNotification";
  const char kMessageAddAccountName[]       = "AddAccount";
  const char kMessageEditAccountName[]      = "EditAccount";
  const char kMessageAddTemporaryPageName[] = "AddTemporaryPage";
  const char kMessagePreventShutdownName[]  = "PreventShutdown";
  const char kMessageShutdownName[]         = "Shutdown";
  const char kMessageListDesktopMedia[]     = "ListDesktopMedia";
  const char kMessageRequestDownloadName[]  = "RequestDownload";
  const char kMessageRestartDownloadName[]  = "RestartDownload";
  const char kMessageCancelDownloadName[]   = "CancelDownload";
  const char kMessageRemoveDownloadName[]   = "RemoveDownload";
  const char kMessageOpenDownloadedName[]   = "OpenDownloaded";
  const char kMessageShowDownloadedName[]   = "ShowDownloaded";
  const char kMessageOpenDownloadFolder[]   = "OpenDownloadFolder";
  const char kMessageListDownloadHistory[]  = "ListDownloadHistory";

  const char kCurrentPortalId[]             = "current_portal";

  const char kIndicatorOnlineName[]         = "online";
  const char kIndicatorDndName[]            = "dnd";
  const char kIndicatorAwayName[]           = "away";
  const char kIndicatorOfflineName[]        = "offline";
  const char kIndicatorFlashName[]          = "flash";
  const char kIndicatorFlashImportantName[] = "flash_important";

}  // namespace

AppMessageDelegate::AppMessageDelegate()
    : ProcessMessageDelegate (kNameSpace) {
}


bool
AppMessageDelegate::OnProcessMessageReceived(
    CefRefPtr<ClientHandler> handler,
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {

  std::string message_name = message->GetName();
  CefRefPtr<CefListValue> request_args = message->GetArgumentList();
  int32 callbackId = -1;
  int32 error = NO_ERROR;
  bool invoke_callback = true;
  CefRefPtr<CefProcessMessage> response =
     CefProcessMessage::Create("invokeCallback");
  CefRefPtr<CefListValue> response_args = response->GetArgumentList();

  // V8 extension messages are handled here. These messages come from the
  // render process thread (in cef_app.cpp), and have the following format:
  //   name - the name of the native function to call
  //   argument0 - the id of this message. This id is passed back to the
  //               render process in order to execute callbacks
  //   argument1 - argumentN - the arguments for the function
  //

  // If we have any arguments, the first is the callbackId
  if (request_args->GetSize() > 0 && request_args->GetType(0) != VTYPE_NULL) {
    callbackId = request_args->GetInt(0);

    if (callbackId != -1)
      response_args->SetInt(0, callbackId);
  }

  message_name = message_name.substr(strlen(kNameSpace));

#ifndef NDEBUG
  LOG(INFO) << "App message: " << message_name << helper::DumpList(request_args);
#endif

  if (message_name == kMessageLoginName) {
    // Parameters:
    //  0: int32 - callback id

    if (request_args->GetSize() != 1) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      CefRefPtr<Account> account = ClientHandler::GetInstance()->GetAccountManager()->GetCurrentAccount();
      account->Auth(
          false,
          base::Bind(&AppMessageDelegate::OnAfterLogin, this, browser, response)
      );
      invoke_callback = false;
    }

  } else if (message_name == kMessageNavigateName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: string - url

    if (
       request_args->GetSize() != 2
          || request_args->GetType(1) != VTYPE_STRING
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      std::string url = request_args->GetString(1);
      if (url == kCurrentPortalId) {
        url = ClientHandler::GetInstance()->GetAccountManager()->GetCurrentAccount()->GetBaseUrl();
        // BXD_API_VERSION is mandatory for stable IM work
        url += "?BXD_API_VERSION=" JS_API_VERSION;
        url += "&user_lang=" + BrickApp::GetCurrentLanguage();
      }

      if (
          url.find("https://") == 0
          || url.find("http://") == 0
         ) {
        browser->GetMainFrame()->LoadURL(url);
      } else {
        LOG(WARNING) << "Trying to navigate to the forbidden url: " << url;
        error = ERR_INVALID_URL;
      }
    }

  } else if (message_name == kMessageBrowseName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: string - url

    if (
       request_args->GetSize() != 2
          || request_args->GetType(1) != VTYPE_STRING
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      std::string url = request_args->GetString(1);
      if (
         url.find("https://") == 0
            || url.find("http://") == 0
         ) {
        // Currently allow only web urls opening...
        platform_util::OpenExternal(url);
      } else {
        LOG(WARNING) << "Trying to browse forbidden url: " << url;
        error = ERR_INVALID_URL;
      }
    }

  } else if (message_name == kMessageShowLogFileName) {
    // Parameters:
    //  0: int32 - callback id

    if (request_args->GetSize() != 1) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      platform_util::ShowInFolder(handler->GetAppSettings().log_file);
    }

  } else if (message_name == kMessageChangeTooltipName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: string - text

    if (
       request_args->GetSize() != 2
          || request_args->GetType(1) != VTYPE_STRING
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      std::string text = request_args->GetString(1);
      ClientHandler::GetInstance()->GetIndicatorHandle()->SetTooltip(
         text.c_str()
      );
    }

  } else if (message_name == kMessageSetIdleIndicatorName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: string - status from IndicatorStatusIcon

    if (
       request_args->GetSize() != 2
          || request_args->GetType(1) != VTYPE_STRING
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      std::string status = request_args->GetString(1);

      if (status == kIndicatorOnlineName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIdleIcon(IndicatorStatusIcon::ONLINE);
      } else if (status == kIndicatorDndName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIdleIcon(IndicatorStatusIcon::DND);
      } else if (status == kIndicatorAwayName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIdleIcon(IndicatorStatusIcon::AWAY);
      } else if (status == kIndicatorOfflineName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIdleIcon(IndicatorStatusIcon::OFFLINE);
      }
    }

  } else if (message_name == kMessageSetIndicatorName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: string - status from IndicatorStatusIcon

    if (
       request_args->GetSize() != 2
          || request_args->GetType(1) != VTYPE_STRING
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      std::string status = request_args->GetString(1);

      if (status == kIndicatorOnlineName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIcon(IndicatorStatusIcon::ONLINE);
      } else if (status == kIndicatorDndName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIcon(IndicatorStatusIcon::DND);
      } else if (status == kIndicatorAwayName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIcon(IndicatorStatusIcon::AWAY);
      } else if (status == kIndicatorOfflineName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIcon(IndicatorStatusIcon::OFFLINE);
      } else if (status == kIndicatorFlashName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIcon(IndicatorStatusIcon::FLASH);
      } else if (status == kIndicatorFlashImportantName) {
        ClientHandler::GetInstance()->GetIndicatorHandle()->SetIcon(IndicatorStatusIcon::FLASH_IMPORTANT);
      }
    }

  } else if (message_name == kMessageIndicatorBadgeName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: int - count
    //  2: bool - important

    if (
       request_args->GetSize() != 3
          || request_args->GetType(1) != VTYPE_INT
          || request_args->GetType(2) != VTYPE_BOOL
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      ClientHandler::GetInstance()->GetIndicatorHandle()->SetBadge(
                request_args->GetInt(1),
                request_args->GetBool(2)
        );
    }

  } else if (message_name == kMessageShowNotificationName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: string - title
    //  2: string - text
    //  3: string - icon path
    //  4: int - duration
    //  5: string - jsId
    //  6: bool - "message notify" flag

    if (
       request_args->GetSize() != 7
          || request_args->GetType(1) != VTYPE_STRING
          || request_args->GetType(2) != VTYPE_STRING
          || request_args->GetType(3) != VTYPE_STRING
          || request_args->GetType(4) != VTYPE_INT
          || request_args->GetType(5) != VTYPE_STRING
          || request_args->GetType(6) != VTYPE_BOOL
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      std::string icon_url = request_args->GetString(3);

      if (
         icon_url.empty()
            || icon_url.find("https://") == 0
            || icon_url.find("http://") == 0
         ) {

        ClientHandler::GetInstance()->GetNotificationManager()->Notify(
           request_args->GetString(1),
           request_args->GetString(2),
           icon_url,
           request_args->GetInt(4),
           request_args->GetString(5),
           request_args->GetBool(6)
        );
      } else {
        LOG(WARNING) << "Trying to show forbidden icon: " << icon_url;
        error = ERR_INVALID_URL;
      }
    }

  } else if (message_name == kMessageAddAccountName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: bool - switch after add

    if (
       request_args->GetSize() != 2
          || request_args->GetType(1) != VTYPE_BOOL
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      EditAccountWindow *window(new EditAccountWindow);
      window->Init(CefRefPtr<Account> (new Account), request_args->GetBool(1));
      window->Show();
    }

  } else if (message_name == kMessageEditAccountName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: bool - switch after add

    if (
       request_args->GetSize() != 2
       || request_args->GetType(1) != VTYPE_BOOL
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      EditAccountWindow *window(new EditAccountWindow);
      window->Init(
         ClientHandler::GetInstance()->GetAccountManager()->GetCurrentAccount(),
         request_args->GetBool(1)
      );
      window->Show();
    }

  } else if (message_name == kMessageAddTemporaryPageName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: string - html content

    if (
       request_args->GetSize() != 2
       || request_args->GetType(1) != VTYPE_STRING
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      CefString url = ClientHandler::GetInstance()->AddTemporaryPage(
         request_args->GetString(1)
      );

      response_args->SetString(2, url);
    }

  } else if (message_name == kMessagePreventShutdownName) {
    // Parameters:
    // 0: int32 - callback id

    if (request_args->GetSize() != 1) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      ClientHandler::GetInstance()->PreventShutdown();
    }

  } else if (message_name == kMessageShutdownName) {
    // Parameters:
    // 0: int32 - callback id

    if (request_args->GetSize() != 1) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      ClientHandler::GetInstance()->Shutdown(true);
    }

  } else if (message_name == kMessageListDesktopMedia) {
    // Parameters:
    // 0: int32 - callback id
    // 1: bool - list_screens
    // 2: bool - list_windows

    if (request_args->GetSize() != 3
        || request_args->GetType(1) != VTYPE_BOOL
        || request_args->GetType(2) != VTYPE_BOOL
        ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      CefRefPtr<CefListValue> media_list = CefListValue::Create();
      // TODO(buglloc): switch to dictionary (first impelemt dictionary support in V8Helper)

      if (request_args->GetBool(1)) {
        desktop_media::EnumerateScreens(media_list);
      }

      if (request_args->GetBool(2)) {
        desktop_media::EnumerateWindows(media_list);
      }

      response_args->SetList(2, media_list);
    }

  } else if (message_name == kMessageRequestDownloadName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: string - url
    // 2: string - filename

    if (request_args->GetSize() != 3
        || request_args->GetType(1) != VTYPE_STRING
        || request_args->GetType(2) != VTYPE_STRING
        ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      handler->InitDownload(
          request_args->GetString(1).ToString(),
          request_args->GetString(2).ToString()
      );
    }

  } else if (message_name == kMessageCancelDownloadName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: string - id

    if (request_args->GetSize() != 2
        || request_args->GetType(1) != VTYPE_STRING
        ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      handler->CancelDownload(request_args->GetString(1));
    }

  } else if (message_name == kMessageRestartDownloadName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: string - id

    if (request_args->GetSize() != 2
        || request_args->GetType(1) != VTYPE_STRING
        ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      handler->RestartDownload(request_args->GetString(1));
    }

  } else if (message_name == kMessageRemoveDownloadName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: string - id

    if (request_args->GetSize() != 2
        || request_args->GetType(1) != VTYPE_STRING
        ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      handler->RemoveDownload(request_args->GetString(1));
    }

  } else if (message_name == kMessageOpenDownloadedName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: string - id

    if (request_args->GetSize() != 2
        || request_args->GetType(1) != VTYPE_STRING
        ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      CefRefPtr<DownloadHistoryItem> item = handler->GetDownloadHistoryItem(request_args->GetString(1));
      if (item.get() && item->Status() == DC_STATUS_SUCCESS) {
        platform_util::OpenExternal(item->GetPath());
      } else {
        error = ERR_NOT_FOUND;
      }

    }

  } else if (message_name == kMessageShowDownloadedName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: string - id

    if (request_args->GetSize() != 2
        || request_args->GetType(1) != VTYPE_STRING
        ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      CefRefPtr<DownloadHistoryItem> item = handler->GetDownloadHistoryItem(request_args->GetString(1));
      if (item.get() && item->Status() == DC_STATUS_SUCCESS) {
        platform_util::ShowInFolder(item->GetPath());
      } else {
        error = ERR_NOT_FOUND;
      }

    }

  } else if (message_name == kMessageListDownloadHistory) {
    // Parameters:
    // 0: int32 - callback id

    if (request_args->GetSize() != 1) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      response_args->SetList(2, handler->GetDownloadHistoryList());
    }

  } else if (message_name == kMessageOpenDownloadFolder) {
    // Parameters:
    // 0: int32 - callback id

    if (request_args->GetSize() != 1) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      platform_util::ShowInFolder(handler->GetAppSettings().download_dir);
    }

  } else {
    return false;
  }

#ifndef NDEBUG
  LOG_IF(INFO, error != NO_ERROR) << "Error while processing app message: " << error;
#endif

  if (invoke_callback && callbackId != -1) {
    response_args->SetInt(1, error);

    // Send response
    browser->SendProcessMessage(PID_RENDERER, response);
  }

  return true;
}


void
AppMessageDelegate::CreateProcessMessageDelegates(ClientHandler::ProcessMessageDelegateSet *delegates) {
  delegates->insert(new AppMessageDelegate);
}


void
AppMessageDelegate::OnAfterLogin(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefProcessMessage> response,
    CefRefPtr<Account> account,
    Account::AuthResult auth_result) {

  CefRefPtr<CefListValue> response_args = response->GetArgumentList();

  if (auth_result.success) {
    SetCookies(CefCookieManager::GetGlobalManager(NULL), account->GetOrigin(), auth_result.cookies, account->IsSecure());
    // TODO(buglloc): Replace huge argument lists to dictionary!
    response_args->SetBool(2, true);
    response_args->SetNull(3);
    response_args->SetNull(4);
    if (auth_result.bitrix_sessid.empty())
      response_args->SetNull(5);
    else
      response_args->SetString(5, auth_result.bitrix_sessid);
  } else {
    response_args->SetBool(2, false);
    response_args->SetInt(3, auth_result.error_code);
    response_args->SetString(4, auth_result.http_error);
    response_args->SetNull(5);
  }

  response_args->SetInt(1, NO_ERROR);

  // Send response
  browser->SendProcessMessage(PID_RENDERER, response);
}

void
AppMessageDelegate::SetCookies(
    CefRefPtr<CefCookieManager> manager,
    const CefString &url,
    request_util::CookiesMap cookies,
    bool is_secure) {

  if (!CefCurrentlyOn(TID_IO)) {
    // Execute on the IO thread.
    CefPostTask(TID_IO, base::Bind(&AppMessageDelegate::SetCookies, this, manager, url, cookies, is_secure));
    return;
  }

  for (const auto &item : cookies) {
    CefCookie cookie;
    CefString(&cookie.name) = item.first;
    CefString(&cookie.value) = item.second;
    cookie.secure = is_secure;
    cookie.httponly = true;
    manager->SetCookie(url, cookie, NULL);
  }
}
