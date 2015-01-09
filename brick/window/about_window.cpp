#include "about_window.h"

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