#ifndef BRICK_ACCOUNT_MANAGER_H_
#define BRICK_ACCOUNT_MANAGER_H_
#pragma once

#include <map>
#include <include/cef_base.h>
#include "account.h"

class AccountManager {
  public:
    typedef std::map<int, Account*> accounts_map;

    AccountManager();
    ~AccountManager();

    void Init(std::string config_path);
    accounts_map* GetAccounts();
    bool AddAccount(Account* account);
    bool DeleteAccount(int id);
    bool SwitchAccount(int id);
    bool Commit();
    Account* GetCurrentAccount();

  protected:
    std::string config_path_;
    accounts_map accounts_;
    Account* current_account_;
    int last_id_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(AccountManager);
};
#endif /* end of BRICK_ACCOUNT_MANAGER_H_ */
