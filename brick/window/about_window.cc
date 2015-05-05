// Copyright (c) 2015 The Brick Authors.

#include "brick/window/about_window.h"

// static
AboutWindow*
AboutWindow::Instance() {
  if (!instance_) {
    instance_ = new AboutWindow();
    instance_->Init();
  }

  return instance_;
}

AboutWindow* AboutWindow::instance_ = 0;
