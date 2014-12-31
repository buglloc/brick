#include "brick/window_util.h"
#include "app_window_message_delegate.h"
#include "brick/v8_handler.h"


namespace {
    const char kNameSpace[]                      = "AppWindowEx";
    const char kMessageSetSizeName[]             = "SetSize";
    const char kMessageSetClientSizeName[]       = "SetClientSize";
    const char kMessageSetMinClientSizeName[]    = "SetMinClientSize";
    const char kMessageFixName[]                 = "FixSize";
    const char kMessageHideName[]                = "Hide";
    const char kMessageShowName[]                = "Show";
    const char kMessageOpenDeveloperToolsName[]  = "OpenDeveloperTools";


} // namespace

AppWindowMessageDelegate::AppWindowMessageDelegate()
   : ProcessMessageDelegate (kNameSpace)
{
}


bool
AppWindowMessageDelegate::OnProcessMessageReceived(
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
  CefWindowHandle window = browser->GetHost()->GetWindowHandle();
  if (!browser->IsPopup())
    window = window_util::GetParent(window);

  if(message_name == kMessageSetSizeName || message_name == kMessageSetClientSizeName) {

    // Parameters:
    //  0: int32 - callback id
    //  1: int - width
    //  2: int - height
    if (
       request_args->GetSize() != 3
          || request_args->GetType(1) != VTYPE_INT
          || request_args->GetType(2) != VTYPE_INT
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {

      window_util::Resize(
         window,
         request_args->GetInt(1),
         request_args->GetInt(2)
      );
    }

  } else if(message_name == kMessageSetMinClientSizeName) {

    // Parameters:
    //  0: int32 - callback id
    //  1: int - min width
    //  2: int - min height
    if (
       request_args->GetSize() != 3
          || request_args->GetType(1) != VTYPE_INT
          || request_args->GetType(2) != VTYPE_INT
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      window_util::SetMinSize(
         window,
         request_args->GetInt(1),
         request_args->GetInt(2)
      );
    }
  } else if (message_name == kMessageFixName) {

    // Parameters:
    //  0: int32 - callback id
    //  1: int - width
    //  2: int - height
    if (
       request_args->GetSize() != 3
          || request_args->GetType(1) != VTYPE_INT
          || request_args->GetType(2) != VTYPE_INT
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      window_util::FixSize(
         window,
         request_args->GetInt(1),
         request_args->GetInt(2)
      );
    }

  } else if (message_name == kMessageHideName) {
    window_util::Hide(window);

  } else if (message_name == kMessageShowName) {
    window_util::Show(window);

  } else if (message_name == kMessageOpenDeveloperToolsName) {
    ClientHandler::GetInstance()->ShowDevTools(browser, CefPoint());


  } else {
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
AppWindowMessageDelegate::CreateProcessMessageDelegates(ClientHandler::ProcessMessageDelegateSet& delegates) {
  delegates.insert(new AppWindowMessageDelegate);
}