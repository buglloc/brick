#include "../notification.h"
#include "../cef_handler.h"
#include "main_window.h"

bool
MainWindow::SetFocus(bool focused) {
  BaseWindow::SetFocus(focused);

  CefRefPtr<ClientHandler> handler = ClientHandler::GetInstance();
  if (!handler)
    return false;

  CefRefPtr<CefBrowser> browser = handler->GetBrowser();
  if (!browser)
    return false;

  // Give focus to the browser window.
  browser->GetHost()->SetFocus(focused);

  if (focused) {
    // Hide previously  showed notification
    Notification::Hide();
  }

  // Hack: CEF have strange behaviour for onfocus/onblur event, so we send custom event
  // ToDo: Research
  handler->SendJSEvent(browser, "BXForegroundChanged", focused? "[true]": "[false]");

  return true;
}