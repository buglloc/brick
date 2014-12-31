#ifndef BRICK_ACCOUNT_MANAGER_H_
#define BRICK_ACCOUNT_MANAGER_H_
#pragma once

#include <include/cef_base.h>
#include "account.h"

class AccountManager {
  public:
    AccountManager();
    ~AccountManager();

//    typedef std::map<Account> AccountMap;
    void AddAccount(Account* account);
    Account* GetCurrentAccount();

  protected:
//    AccountMap account_list_;
    Account* current_account_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(AccountManager);
};
#endif /* end of BRICK_ACCOUNT_MANAGER_H_ */
