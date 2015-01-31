#ifndef BRICK_ACCOUNT_MANAGER_H_
#define BRICK_ACCOUNT_MANAGER_H_
#pragma once

#include <map>
#include <include/cef_base.h>
#include "event/event_sender.h"
#include "account.h"

class AccountManager : public CefBase,
                       public EventSender {
  public:
    typedef std::map<int, CefRefPtr<Account>> accounts_map;

    AccountManager();
    ~AccountManager();

    void Init(std::string config_path);
    accounts_map* GetAccounts();
    bool AddAccount(const CefRefPtr<Account> account);
    bool DeleteAccount(int id);
    bool SwitchAccount(int id);
    bool Commit();
    CefRefPtr<Account> GetCurrentAccount();
    CefRefPtr<Account> GetById(int id);

  protected:
    std::string config_path_;
    accounts_map accounts_;
    CefRefPtr<Account> current_account_;
    int last_id_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(AccountManager);
};
#endif /* end of BRICK_ACCOUNT_MANAGER_H_ */
