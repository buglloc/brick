// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_WINDOW_EDIT_ACCOUNT_WINDOW_H_
#define BRICK_WINDOW_EDIT_ACCOUNT_WINDOW_H_
#pragma once

#include <string>

#include "brick/window/base_window.h"
#include "brick/account.h"

class EditAccountWindow : public BaseWindow {

 public:
  EditAccountWindow() {}
  virtual ~EditAccountWindow() {}

  void Save(bool secure,
            const std::string& domain,
            const std::string& login,
            const std::string& password,
            bool use_app_password);

  void OnAuthComplete(const CefRefPtr<Account> account, const Account::AuthResult auth_result);

  // Platform specific methods
  void Init() OVERRIDE;
  void Init(CefRefPtr<Account> account, bool switch_on_save = false);
#if defined(__linux__)
  struct WindowObjects {
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
  struct WindowObjects {
    ClientWindowHandle        window;
    CefRefPtr<Account>        account;
    bool                      switch_on_save;
  } window_objects_;
#endif


IMPLEMENT_REFCOUNTING(EditAccountWindow);
};

#endif  // BRICK_WINDOW_EDIT_ACCOUNT_WINDOW_H_
