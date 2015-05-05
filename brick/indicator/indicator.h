// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_INDICATOR_INDICATOR_H_
#define BRICK_INDICATOR_INDICATOR_H_
#pragma once

#if defined(__linux__)

#ifdef unity
#include <unity.h>
#include <libappindicator/app-indicator.h>
#else
#include "gtk/gtk.h"
#endif

#endif

#include <string>
#include "include/cef_base.h"
#include "brick/event/account_switch_event.h"
#include "brick/event/account_list_event.h"
#include "brick/event/event_handler.h"
#include "brick/event/event_bus.h"

class BrickIndicator : public CefBase,
                   public EventHandler<AccountListEvent>,
                   public EventHandler<AccountSwitchEvent> {

 public:
  enum Icon {
    DEFAULT = 0,
    OFFLINE,
    ONLINE,
    DND,
    AWAY,
    FLASH,
    FLASH_IMPORTANT
  };

  explicit BrickIndicator(std::string icons_dir);

  void SetIdleIcon(Icon icon);
  void SetIcon(Icon icon);
  void SetTooltip(const char* text);
  void SetBadge(int badge, bool is_important = false);
  // Platform specific methods
  void Init();
  void UpdateAccountsMenu();
  void UseExtendedStatus(bool use = true);

  // Event handlers
  bool OnClick();
  bool OnPopup() { return false; }
  bool OnMenuQuit();
  bool OnMenuPortalOpen();
  bool OnMenuAbout();
  bool OnMenuManageAccount();
  bool OnMenuChangeAccount(int id);

  // System events handler
  virtual void onEvent(const AccountListEvent& event) OVERRIDE;
  virtual void onEvent(const AccountSwitchEvent& event) OVERRIDE;

 protected:
  void RegisterEventListeners();
  std::string GetIconPath(Icon icon);
  std::string GetIconName(Icon icon);
  void SwitchToIdle();

 private:
  Icon current_icon_;
  Icon idle_icon_;
  std::string icons_folder_;
  bool idle_;
  bool extended_status_;
#if defined(__linux__)

#ifdef unity
  AppIndicator *icon_handler_;
  UnityLauncherEntry *launcher_handler_;
#else
  GtkStatusIcon* icon_handler_;
#endif

#endif
IMPLEMENT_REFCOUNTING(BrickIndicator);
};

#endif  // BRICK_INDICATOR_INDICATOR_H_
