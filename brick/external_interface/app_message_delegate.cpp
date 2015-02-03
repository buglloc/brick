#include <include/cef_app.h>
#include "../window/accounts_window.h"
#include "../window/edit_account_window.h"
#include "../cef_handler.h"
#include "app_message_delegate.h"


namespace {
    const char kNameSpace[]                       = "AppInterface";
    const char kMessageShowAddAccountDialogName[] = "ShowAddAccountDialog";
    const char kMessageShowAccountsDialogName[]   = "ShowAccountsDialog";
    const char kMessageUserAwayName[]   = "UserAway";
    const char kMessageUserPresentName[]   = "UserPresent";
    const char kMessageQuitName[]   = "Quit";


} // namespace

ExternalAppMessageDelegate::ExternalAppMessageDelegate()
   : ExternalMessageDelegate (kNameSpace)
{
}

bool
ExternalAppMessageDelegate::OnMessageReceived(CefRefPtr<CefProcessMessage> message) {

  std::string message_name = message->GetName();
  CefRefPtr<CefListValue> request_args = message->GetArgumentList();
//  int32 callbackId = -1;
  int32 error = NO_ERROR;

  if(message_name == kMessageShowAddAccountDialogName) {
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
    };

  } else if(message_name == kMessageShowAccountsDialogName) {
    AccountsWindow *window(new AccountsWindow);
    window->Init();
    window->Show();

  } else if (message_name == kMessageQuitName) {
    CefQuitMessageLoop();

  } else if (message_name == kMessageUserAwayName) {
    UserAwayEvent e(true, true);
    EventBus::FireEvent(e);

  } else if (message_name == kMessageQuitName) {
    UserAwayEvent e(false, true);
    EventBus::FireEvent(e);

  } else {
    return false;
  }

  return true;
}