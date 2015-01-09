#ifndef BRICK_ACCOUNTS_WINDOW_H_
#define BRICK_ACCOUNTS_WINDOW_H_
#pragma once


#include "base_window.h"
#include "edit_account_window.h"
#include "../account_manager.h"

class AccountsWindow : public BaseWindow {

public:

  AccountsWindow() {};
  virtual ~AccountsWindow() {};
  static AccountsWindow* Instance();
  void ReloadAccounts();

  // Platform specific methods
  void Init() OVERRIDE;
  void Clear();
  void AddToList(int id, std::string label);

  enum {
    REF_ID = 0,
    LABEL,
    N_COLUMN
  };
#if defined(__linux__)
  struct WindowObjects
  {
    ClientWindowHandle           window;
    CefRefPtr<AccountManager>    account_manager;
    CefRefPtr<EditAccountWindow> edit_account_window;
    GtkTreeView                 *accounts_view;
    GtkListStore                *accounts_store;
  } window_objects_;
#else
  struct WindowObjects
  {
    ClientWindowHandle           window;
    CefRefPtr<AccountManager>    account_manager;
    CefRefPtr<EditAccountWindow> edit_account_window;
  } window_objects_;
#endif

private:
  static AccountsWindow* instance_;

IMPLEMENT_REFCOUNTING(AccountsWindow);
};

#endif /* end of BRICK_ACCOUNTS_WINDOW_H_ */
