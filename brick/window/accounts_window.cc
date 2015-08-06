// Copyright (c) 2015 The Brick Authors.

#include "brick/window/accounts_window.h"

void
AccountsWindow::ReloadAccounts() {
  Clear();
  for (const auto account: *(window_objects_.account_manager->GetAccounts())) {
    AddToList(account.first, account.second->GetLabel());
  }
}

// static
AccountsWindow*
AccountsWindow::Instance() {
  if (!instance_) {
    instance_ = new AccountsWindow();
    instance_->Init();
  }

  return instance_;
}

AccountsWindow* AccountsWindow::instance_ = 0;
