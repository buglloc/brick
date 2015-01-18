#include "browser_window.h"

BrowserWindowHandle
BrowserWindow::GetHandler() {
  return window_handler_;
}

bool
BrowserWindow::GetHided() {
  return hided_;
}