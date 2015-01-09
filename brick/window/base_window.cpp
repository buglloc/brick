#include "base_window.h"

ClientWindowHandle
BaseWindow::GetHandler() {
  return window_handler_;
}

bool
BaseWindow::GetHided() {
  return hided_;
}

void
BaseWindow::SetFocus(bool focused) {
  focused_ = focused;
}

bool
BaseWindow::HasFocus() {
  return focused_;
}