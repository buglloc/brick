#ifndef BRICK_BASE_STATUS_ICON_H_
#define BRICK_BASE_STATUS_ICON_H_
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
#include <include/cef_base.h>
#include "../event/account_switch_event.h"
#include "../event/account_list_event.h"
#include "../event/event_handler.h"
#include "../event/event_bus.h"

class StatusIcon : public CefBase,
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

  StatusIcon(std::string icons_dir);
  ~StatusIcon();
  unsigned char GetIcon();
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
  bool OnPopup() {return false;};
  bool OnMenuQuit();
  bool OnMenuPortalOpen();
  bool OnMenuAbout();
  bool OnMenuManageAccount();
  bool OnMenuChangeAccount(int id);

  // System events handler
  virtual void onEvent(AccountListEvent &event) OVERRIDE;
  virtual void onEvent(AccountSwitchEvent &event) OVERRIDE;

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
IMPLEMENT_REFCOUNTING(StatusIcon);
};

#endif /* end of BRICK_BASE_STATUS_ICON_H_ */
