#ifndef BRICK_EDIT_ACCOUNT_WINDOW_H_
#define BRICK_EDIT_ACCOUNT_WINDOW_H_
#pragma once

#include <include/base/cef_lock.h>
#include <include/internal/cef_linux.h>

#if defined(__linux__)
// The Linux client uses GTK instead of the underlying platform type (X11).
#include <gtk/gtk.h>
#include <brick/account_manager.h>

#define ClientWindowHandle GtkWidget*
#else
#define ClientWindowHandle CefWindowHandle
#endif

class EditAccountWindow : public CefBase {

public:

  EditAccountWindow() {};
  ~EditAccountWindow() {};
  void Save(std::string domain, std::string login, std::string password);


  // Platform specific methods
  void Init(CefRefPtr<Account> account, bool switch_on_save = false);
  void Show();
  void Hide();
  void Close();

  enum {
    REF_ID = 0,
    LABEL,
    N_COLUMN
  };
#if defined(__linux__)
  struct WindowObjects
  {
    ClientWindowHandle        window;
    CefRefPtr<Account>        account;
    bool                      switch_on_save;
    GtkEntry                 *domain_entry;
    GtkEntry                 *login_entry;
    GtkEntry                 *password_entry;
  } window_objects_;
#else
  struct WindowObjects
  {
    ClientWindowHandle        window;
    CefRefPtr<Account>        account;
    bool                      switch_on_save;
  } window_objects_;
#endif


IMPLEMENT_REFCOUNTING(EditAccountWindow);
};

#endif /* end of BRICK_EDIT_ACCOUNT_WINDOW_H_ */
