#include "account_manager.h"

AccountManager::AccountManager()
   : current_account_(NULL)
{
}

AccountManager::~AccountManager() {
}

void
AccountManager::AddAccount(Account* account) {
//  account_list_[account_list_.size()] = account;
  if (account->IsDefault()) {
    current_account_ = account;
  }
}

Account*
AccountManager::GetCurrentAccount() {
  return current_account_;
}