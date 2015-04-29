#ifndef BRICK_EDIT_ACCOUNT_WINDOW_H_
#define BRICK_EDIT_ACCOUNT_WINDOW_H_
#pragma once

#include "base_window.h"
#include "../account.h"

class EditAccountWindow : public BaseWindow {

public:
  EditAccountWindow() {};
  virtual ~EditAccountWindow() {};
  void Save(bool secure, std::string domain, std::string login, std::string password, bool use_app_password = true);


  // Platform specific methods
  void Init() OVERRIDE;
  void Init(CefRefPtr<Account> account, bool switch_on_save = false);
#if defined(__linux__)
  struct WindowObjects
  {
    ClientWindowHandle        window;
    CefRefPtr<Account>        account;
    bool                      switch_on_save;
    GtkComboBox              *protocol_chooser;
    GtkEntry                 *domain_entry;
    GtkEntry                 *login_entry;
    GtkEntry                 *password_entry;
    GtkCheckButton           *use_app_password;
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
