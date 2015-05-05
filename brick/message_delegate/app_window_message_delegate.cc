// Copyright (c) 2015 The Brick Authors.

#include "brick/message_delegate/app_window_message_delegate.h"

#include <string>

#include "brick/window_util.h"
#include "brick/helper.h"


namespace {
  const char kNameSpace[]                      = "AppWindowEx";
  const char kMessageSetSizeName[]             = "SetSize";
  const char kMessageSetClientSizeName[]       = "SetClientSize";
  const char kMessageSetMinClientSizeName[]    = "SetMinClientSize";
  const char kMessageMoveResizeName[]          = "MoveResize";
  const char kMessageFixName[]                 = "FixSize";
  const char kMessageHideName[]                = "Hide";
  const char kMessageShowName[]                = "Show";
  const char kMessageShowMainName[]            = "ShowMain";
  const char kMessageCenterName[]              = "Center";
  const char kMessageSetClosableName[]         = "SetClosable";
  const char kMessageOpenDeveloperToolsName[]  = "OpenDeveloperTools";

}  // namespace

AppWindowMessageDelegate::AppWindowMessageDelegate()
    : ProcessMessageDelegate(kNameSpace) {
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
  CefWindowHandle cef_window = browser->GetHost()->GetWindowHandle();
  BrowserWindow *window = window_util::LookupBrowserWindow(cef_window);

#ifndef NDEBUG
  LOG(INFO) << "AppWindow message for window " << cef_window << ":" << message_name << helper::DumpList(request_args);
#endif

  if (message_name == kMessageSetSizeName || message_name == kMessageSetClientSizeName) {

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
      window->Resize(
         request_args->GetInt(1),
         request_args->GetInt(2)
      );
    }

  } else if (message_name == kMessageSetMinClientSizeName) {

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
      window->SetMinSize(
         request_args->GetInt(1),
         request_args->GetInt(2)
      );
    }

  } else if (message_name == kMessageMoveResizeName) {

    // Parameters:
    //  0: int32 - callback id
    //  1: int - position
    //  2: int - width
    //  3: int - height
    if (
       request_args->GetSize() != 4
       || request_args->GetType(1) != VTYPE_INT
       || request_args->GetType(2) != VTYPE_INT
       || request_args->GetType(3) != VTYPE_INT
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      int requestedPosition = request_args->GetInt(1);
      if (
         requestedPosition >= BrowserWindow::Position::NORTH_WEST
         && requestedPosition <= BrowserWindow::Position::SOUTH_EAST
         ) {

        int width = request_args->GetInt(2);
        int height = request_args->GetInt(3);

        window->MoveResize(
           (BrowserWindow::Position) requestedPosition,
           width,
           height
        );
        window->FrozeSize(width, height);

      } else {
        error = ERR_INVALID_PARAMS;
      }

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
      window->FrozeSize(
         request_args->GetInt(1),
         request_args->GetInt(2)
      );
    }

  } else if (message_name == kMessageHideName) {
    window->Hide();

  } else if (message_name == kMessageShowName) {
    window->Show();

  } else if (message_name == kMessageShowMainName) {
    ClientHandler::GetInstance()->GetMainWindowHandle()->Present();

  } else if (message_name == kMessageCenterName) {
    // ToDo: implement!

  } else if (message_name == kMessageSetClosableName) {
    // Parameters:
    //  0: int32 - callback id
    //  1: bool - closable

    if (
       request_args->GetSize() != 2
       || request_args->GetType(1) != VTYPE_BOOL
       ) {
      error = ERR_INVALID_PARAMS;
    }

    if (error == NO_ERROR) {
      window->SetClosable(request_args->GetBool(1));
    }


  } else if (message_name == kMessageOpenDeveloperToolsName) {
    ClientHandler::GetInstance()->ShowDevTools(browser, CefPoint());


  } else {
    return false;
  }

#ifndef NDEBUG
  LOG_IF(INFO, error != NO_ERROR) << "Error while processing app window message: " << error;
#endif

  if (callbackId != -1) {
    response_args->SetInt(1, error);

    // Send response
    browser->SendProcessMessage(PID_RENDERER, response);
  }

  return true;
}


void
AppWindowMessageDelegate::CreateProcessMessageDelegates(ClientHandler::ProcessMessageDelegateSet* delegates) {
  delegates->insert(new AppWindowMessageDelegate);
}
