#ifndef BRICK_BASE_STATUS_ICON_H_
#define BRICK_BASE_STATUS_ICON_H_
#pragma once

#if defined(__linux__)

#ifdef unity
#include <libappindicator/app-indicator.h>
#else
#include "gtk/gtk.h"
#endif

#endif

#include <string>
#include <include/cef_base.h>

class StatusIcon : public CefBase {

public:
  enum Icon {
    DEFAULT, OFFLINE, ONLINE, FLASH, FLASH_IMPORTANT
  };

  StatusIcon(std::string icons_dir);
  ~StatusIcon();
  unsigned char GetIcon();
  void SetIcon(Icon icon);
  void SetTooltip(const char* text);

  // Platform specific methods
  void Init();

  // Event handlers
  bool OnClick();
  bool OnPopup() {return false;};
  bool OnMenuQuit();
  bool OnMenuPortalOpen();
  bool OnMenuAbout();
  bool OnMenuManageAccount();
  bool OnMenuChangeAccount(int id);

protected:
  std::string GetIconPath(Icon icon);
  std::string GetIconName(Icon icon);

private:
  unsigned char current_icon_;
  std::string icons_folder_;
#if defined(__linux__)

#ifdef unity
  AppIndicator *icon_handler_;
#else
  GtkStatusIcon* icon_handler_;
#endif

#endif
IMPLEMENT_REFCOUNTING(StatusIcon);
};

#endif /* end of BRICK_BASE_STATUS_ICON_H_ */
