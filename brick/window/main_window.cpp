#include <stdio.h>
#include "brick/cef_handler.h"
#include "brick/notification.h"

#include "main_window.h"

MainWindow::MainWindow()
  : window_handler_(NULL),
    hided_(true),
    focused_(false)
{
}

MainWindow::~MainWindow() {
}

ClientWindowHandle
MainWindow::GetHandler() {
  return window_handler_;
}

bool
MainWindow::GetHided() {
  return hided_;
}

bool
MainWindow::SetFocus(bool focused) {
  focused_ = focused;
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

bool
MainWindow::HasFocus() {
  return focused_;
}