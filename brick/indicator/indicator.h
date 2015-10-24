// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_INDICATOR_INDICATOR_H_
#define BRICK_INDICATOR_INDICATOR_H_
#pragma once

#include <string>
#include "include/cef_base.h"
#include "brick/brick_app.h"
#include "brick/brick_types.h"
#include "brick/indicator/base_icon.h"
#include "brick/indicator/unity_launcher.h"
#include "brick/event/account_switch_event.h"
#include "brick/event/account_list_event.h"
#include "brick/event/event_handler.h"
#include "brick/event/event_bus.h"

class BrickIndicator : public CefBase,
                   public EventHandler<AccountListEvent>,
                   public EventHandler<AccountSwitchEvent> {

 public:
  explicit BrickIndicator(std::string icons_dir);

  void SetIdleIcon(IndicatorStatusIcon icon);
  void SetIcon(IndicatorStatusIcon icon);
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
  virtual void OnEvent(const AccountListEvent &event) OVERRIDE;
  virtual void OnEvent(const AccountSwitchEvent &event) OVERRIDE;

 protected:
  void RegisterEventListeners();
  void SwitchToIdle();

 private:
  IndicatorStatusIcon current_icon_;
  IndicatorStatusIcon idle_icon_;
  std::string icons_folder_;
  bool idle_;
  bool extended_status_;
  CefRefPtr<BaseIcon> icon_;
  CefRefPtr<UnityLauncher> unity_launcher_;

IMPLEMENT_REFCOUNTING(BrickIndicator);
};

#endif  // BRICK_INDICATOR_INDICATOR_H_
