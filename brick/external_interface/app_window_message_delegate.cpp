#include "../cef_handler.h"
#include "app_window_message_delegate.h"


namespace {
    const char kNameSpace[]                      = "AppWindowInterface";
    const char kMessageToggleVisibilityName[]    = "ToggleVisibility";
    const char kMessageHideName[]                = "Hide";
    const char kMessagePresentName[]             = "Present";


} // namespace

ExternalAppWindowMessageDelegate::ExternalAppWindowMessageDelegate()
   : ExternalMessageDelegate (kNameSpace)
{
}

bool
ExternalAppWindowMessageDelegate::OnMessageReceived(CefRefPtr<CefProcessMessage> message) {

  std::string message_name = message->GetName();
//  CefRefPtr<CefListValue> request_args = message->GetArgumentList();
//  int32 callbackId = -1;
//  int32 error = NO_ERROR;

  BrowserWindow *window = ClientHandler::GetInstance()->GetMainWindowHandle();

  if(message_name == kMessageToggleVisibilityName) {
    window->ToggleVisibility();

  } else if(message_name == kMessageHideName) {
    window->Hide();

  } else if (message_name == kMessagePresentName) {
    window->Present();

  } else if (message_name == kMessageHideName) {
    window->Hide();

  } else {
    return false;
  }

  return true;
}