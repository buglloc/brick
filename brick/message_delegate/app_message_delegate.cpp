#include <include/base/cef_bind.h>
#include "brick/helper.h"
#include "brick/notification.h"
#include "brick/platform_util.h"
#include "app_message_delegate.h"
#include "brick/v8_handler.h"
#include "include/wrapper/cef_closure_task.h"


namespace {
    const char kNameSpace[]                   = "AppEx";
    const char kMessageLoginName[]            = "Login";
    const char kMessageNavigateName[]         = "Navigate";
    const char kMessageBrowseName[]           = "Browse";
    const char kMessageChangeTooltipName[]    = "ChangeTooltip";
    const char kMessageSetIndicatorName[]     = "SetIndicator";
    const char kMessageIndicatorBadgeName[]   = "IndicatorBadgee";
    const char kMessageShowNotificationName[] = "ShowNotification";

    const char kCurrentPortalId[] = "current_portal";

    const char kIndicatorOnlineName[]         = "online";
    const char kIndicatorOfflineName[]        = "offline";
    const char kIndicatorFlashName[]          = "flash";
    const char kIndicatorFlashImportantName[] = "flash_important";

} // namespace

AppMessageDelegate::AppMessageDelegate()
   : ProcessMessageDelegate (kNameSpace)
{
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
  
  if (message_name == kMessageLoginName) {
    // Parameters:
    //  0: int32 - callback id

    if (request_args->GetSize() != 1) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      CefRefPtr<Account> account = ClientHandler::GetInstance()->GetAccountManager()->GetCurrentAccount();
      HttpClient::response r = HttpClient::PostForm(
         account->GetBaseUrl() + "/login/",
         "action=login&login=" + account->GetLogin() + "&password=" + account->GetPassword()
      );

      if (r.code != 200) {
        LOG(WARNING) << "Auth failed: " << r.error << ", Body: " << r.body;
        error = ERR_ACCESS_DENIED;
        response_args->SetBool(2, false);
        response_args->SetString(3, CefString(r.error));
      }
      else {
        SetCookies(CefCookieManager::GetGlobalManager(), account->GetBaseUrl(), r.cookies, account->IsSecure());
        response_args->SetBool(2, true);
        response_args->SetString(3, "");
      }
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
      if (url == kCurrentPortalId)
        url = ClientHandler::GetInstance()->GetAccountManager()->GetCurrentAccount()->GetBaseUrl();

      browser->GetMainFrame()->LoadURL(url);
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
      if (url.find("http://") == 0
         || url.find("https://") == 0
         ) {
        // Currently allow only web urls opening...
        platform_util::OpenExternal(url);
      }
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
      ClientHandler::GetInstance()->GetStatusIconHandle()->SetTooltip(
         text.c_str()
      );
    }

  } else if (message_name == kMessageSetIndicatorName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: string - status from StatusIcon::Icon

    if (
       request_args->GetSize() != 2
          || request_args->GetType(1) != VTYPE_STRING
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      std::string status = request_args->GetString(1);

      if (status == kIndicatorOnlineName) {
        ClientHandler::GetInstance()->GetStatusIconHandle()->SetIcon(StatusIcon::Icon::ONLINE);
      } else if (status == kIndicatorOfflineName) {
        ClientHandler::GetInstance()->GetStatusIconHandle()->SetIcon(StatusIcon::Icon::OFFLINE);
      } else if (status == kIndicatorFlashName) {
        ClientHandler::GetInstance()->GetStatusIconHandle()->SetIcon(StatusIcon::Icon::FLASH);
      } else if (status == kIndicatorFlashImportantName) {
        ClientHandler::GetInstance()->GetStatusIconHandle()->SetIcon(StatusIcon::Icon::FLASH_IMPORTANT);
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
      CefRefPtr<StatusIcon> status_icon = ClientHandler::GetInstance()->GetStatusIconHandle();
      bool is_important = request_args->GetBool(2);
      if (
          (is_important || status_icon->GetIcon() != StatusIcon::Icon::FLASH_IMPORTANT)
          && request_args->GetInt(1) > 0
         ) {
        // Regular flash (e.g. notification) can't replace important flash status (e.g. messages)
        if (is_important) {
          status_icon->SetIcon(StatusIcon::Icon::FLASH_IMPORTANT);
        } else {
          status_icon->SetIcon(StatusIcon::Icon::FLASH);
        }
      } else {
        // if you don't know what to do just set "online" status
        status_icon->SetIcon(StatusIcon::Icon::ONLINE);
      }

    }

  } else if (message_name == kMessageShowNotificationName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: string - title
    //  2: string - text
    //  3: int - duration

    if (
       request_args->GetSize() != 4
          || request_args->GetType(1) != VTYPE_STRING
          || request_args->GetType(2) != VTYPE_STRING
          || request_args->GetType(3) != VTYPE_INT
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (
       error == NO_ERROR
          && !ClientHandler::GetInstance()->GetMainWindowHandle()->HasFocus()
       ) {
      Notification::Notify(
         request_args->GetString(1),
         request_args->GetString(2),
         request_args->GetInt(3)
      );
    };
  }

  else {
    return false;
  }

  if (callbackId != -1) {
    response_args->SetInt(1, error);

    // Send response
    browser->SendProcessMessage(PID_RENDERER, response);
  }

  return true;
}


void
AppMessageDelegate::CreateProcessMessageDelegates(ClientHandler::ProcessMessageDelegateSet& delegates) {
  delegates.insert(new AppMessageDelegate);
}


void
AppMessageDelegate::SetCookies(CefRefPtr<CefCookieManager> manager,
                              const CefString &url,
                              HttpClient::cookie_map cookies,
                              bool is_secure) {
  if (!CefCurrentlyOn(TID_IO)) {
    // Execute on the IO thread.
    CefPostTask(TID_IO, base::Bind(&AppMessageDelegate::SetCookies, manager, url, cookies, is_secure));
    return;
  }

  for (HttpClient::cookie_map::iterator value=cookies.begin(); value != cookies.end(); ++value) {
    CefCookie cookie;
    CefString(&cookie.name) = value->first;
    CefString(&cookie.value) = value->second;
    cookie.secure = is_secure;
    cookie.httponly = true;
    manager->SetCookie(url, cookie);
  }
}


CefString
AppMessageDelegate::ParseAuthSessid(std::string body) {
  std::string sessId;
  size_t pos = body.find("sessionId: '") + sizeof("sessionId: '");
  sessId = body.substr(
     pos - 1,
     body.find("'", pos + 1) - pos + 1
  );

  CefString result;
  result.FromASCII(sessId.c_str());
  return result;
}