#include "../notification.h"
#include "../cef_handler.h"
#include "main_window.h"

bool
MainWindow::SetFocus(bool focused) {
  BaseWindow::SetFocus(focused);

  if (!focused)
    return false;

  CefRefPtr<ClientHandler> handler = ClientHandler::GetInstance();
  if (!handler)
    return false;

  CefRefPtr<CefBrowser> browser = handler->GetBrowser();
  if (!browser)
    return false;

  // Give focus to the browser window.
  browser->GetHost()->SetFocus(true);

  // Hide previously  showed notification
  Notification::Hide();
  return true;
}