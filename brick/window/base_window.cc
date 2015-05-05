// Copyright (c) 2015 The Brick Authors.

#include "brick/window/base_window.h"

ClientWindowHandle
BaseWindow::GetHandler() {
  return window_handler_;
}

bool
BaseWindow::GetHided() {
  return hided_;
}

bool
BaseWindow::SetFocus(bool focused) {
  focused_ = focused;
  return false;
}

bool
BaseWindow::HasFocus() {
  return focused_;
}
