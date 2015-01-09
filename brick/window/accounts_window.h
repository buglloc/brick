#ifndef BRICK_ACCOUNTS_WINDOW_H_
#define BRICK_ACCOUNTS_WINDOW_H_
#pragma once

#include <include/base/cef_lock.h>
#include <include/internal/cef_linux.h>

#if defined(__linux__)
// The Linux client uses GTK instead of the underlying platform type (X11).
#include <gtk/gtk.h>
#include <brick/account_manager.h>
#include "edit_account_window.h"

#define ClientWindowHandle GtkWidget*
#else
#define ClientWindowHandle CefWindowHandle
#endif

class AccountsWindow : public CefBase {

public:

  AccountsWindow() {};
  virtual ~AccountsWindow() {};
  static AccountsWindow* Instance();
  void ReloadAccounts();

  // Platform specific methods
  void Init();
  void Show();
  void Hide();
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
