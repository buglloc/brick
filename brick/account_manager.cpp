#include "account_manager.h"

AccountManager::AccountManager()
   : current_account_(NULL)
{
}

AccountManager::~AccountManager() {
}

void
AccountManager::AddAccount(Account* account) {
//  account_list_.push_back(account);
  if (account->IsDefault()) {
    current_account_ = account;
  }
}

Account*
AccountManager::GetCurrentAccount() {
  return current_account_;
}