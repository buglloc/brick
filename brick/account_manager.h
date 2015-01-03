#ifndef BRICK_ACCOUNT_MANAGER_H_
#define BRICK_ACCOUNT_MANAGER_H_
#pragma once

#include <include/cef_base.h>
#include <bits/stl_bvector.h>
#include "account.h"

class AccountManager {
  public:
    AccountManager();
    ~AccountManager();

//    typedef std::vector<Account *> account_vector;
    void AddAccount(Account* account);
    Account* GetCurrentAccount();

  protected:
//    account_vector account_list_;
    Account* current_account_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(AccountManager);
};
#endif /* end of BRICK_ACCOUNT_MANAGER_H_ */
