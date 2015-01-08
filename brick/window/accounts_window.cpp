#include "accounts_window.h"

void
AccountsWindow::ReloadAccounts() {
  Clear();
  AccountManager::accounts_map *accounts = window_objects_.account_manager->GetAccounts();
  AccountManager::accounts_map::iterator it = accounts->begin();
  for (; it != accounts->end(); ++it) {
    AddToList((*it).first, (*it).second->GetLabel());
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