// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_ACCOUNT_MANAGER_H_
#define BRICK_ACCOUNT_MANAGER_H_
#pragma once

#include <map>
#include <string>

#include "include/cef_base.h"
#include "brick/event/event_sender.h"
#include "brick/account.h"

class AccountManager : public CefBase {
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
  bool initialized_;
  std::string config_path_;
  accounts_map accounts_;
  CefRefPtr<Account> current_account_;
  int last_id_;

  IMPLEMENT_REFCOUNTING(AccountManager);
};
#endif  // BRICK_ACCOUNT_MANAGER_H_
