// Copyright (c) 2015 The Brick Authors.

#include "brick/external_interface/app_message_delegate.h"

#include <string>

#include "include/cef_app.h"
#include "third-party/json/json.h"
#include "brick/window/accounts_window.h"
#include "brick/window/edit_account_window.h"
#include "brick/client_handler.h"


namespace {
  const char kNameSpace[]                       = "AppInterface";
  const char kMessageShowAddAccountDialogName[] = "ShowAddAccountDialog";
  const char kMessageShowAccountsDialogName[]   = "ShowAccountsDialog";
  const char kMessageUserAwayName[]             = "UserAway";
  const char kMessageUserPresentName[]          = "UserPresent";
  const char kMessageActionName[]               = "Action";
  const char kMessageQuitName[]                 = "Quit";

}  // namespace

ExternalAppMessageDelegate::ExternalAppMessageDelegate()
    : ExternalMessageDelegate(kNameSpace)
{ }

bool
ExternalAppMessageDelegate::OnMessageReceived(
    CefRefPtr<CefProcessMessage> message) {

  std::string message_name = message->GetName();
  CefRefPtr<CefListValue> request_args = message->GetArgumentList();
//  int32 callbackId = -1;
  int32 error = NO_ERROR;

  if (message_name == kMessageShowAddAccountDialogName) {
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

  } else if (message_name == kMessageShowAccountsDialogName) {
    AccountsWindow::Instance()->Show();

  } else if (message_name == kMessageQuitName) {
    CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
    if (client_handler)
      client_handler->Shutdown(false);

  } else if (message_name == kMessageUserAwayName) {
    UserAwayEvent e(true, true);
    EventBus::FireEvent(e);

  } else if (message_name == kMessageActionName) {
    // Parameters:
    // 0: int32 - callback id
    // 1: string - action name
    // 2: dictionary - params

    if (
        request_args->GetSize() != 3
        || request_args->GetType(1) != VTYPE_STRING
        || request_args->GetType(2) != VTYPE_DICTIONARY
        ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
      if (client_handler) {
        Json::Value action_params(Json::objectValue);
        CefDictionaryValue::KeyList key_list;
        CefRefPtr<CefDictionaryValue> params = request_args->GetDictionary(2);
        if (params->GetKeys(key_list)) {
          for (const auto &key : key_list) {
            action_params[key.ToString()] = params->GetString(key).ToString();
          }
        }

        Json::Value event_params(Json::arrayValue);
        event_params[0] = request_args->GetString(1).ToString();
        event_params[1] = action_params;

        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        client_handler->SendJSEvent(client_handler->GetBrowser(), "BXProtocolUrl", writer.write(event_params));
      }
    }

  } else if (message_name == kMessageQuitName) {
    UserAwayEvent e(false, true);
    EventBus::FireEvent(e);

  } else {
    return false;
  }

  return true;
}
