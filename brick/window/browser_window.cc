#include "browser_window.h"

BrowserWindowHandle
BrowserWindow::GetHandler() {
  return window_handler_;
}

bool
BrowserWindow::GetHided() {
  return hided_;
}

bool
BrowserWindow::OnHide() {
  hided_ = true;
  return false;
}

bool
BrowserWindow::OnShow() {
  hided_ = false;
  return false;
}

void
BrowserWindow::SetClosable(bool closable) {
  closable_ = closable;
}

bool
BrowserWindow::IsClosable() {
  return closable_;
}