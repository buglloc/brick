#include "../notification.h"
#include "../cef_handler.h"
#include "main_window.h"

void
MainWindow::SetFocus(bool focused) {
  BaseWindow::SetFocus(focused);

  if (!focused)
    return;

  CefRefPtr<ClientHandler> handler = ClientHandler::GetInstance();
  if (!handler)
    return;

  CefRefPtr<CefBrowser> browser = handler->GetBrowser();
  if (!browser)
    return;

  // Give focus to the browser window.
  browser->GetHost()->SetFocus(true);

  // Hide previously  showed notification
  Notification::Hide();
}