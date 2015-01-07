#ifndef BRICK_BASE_STATUS_ICON_H_
#define BRICK_BASE_STATUS_ICON_H_
#pragma once

#if defined(__linux__)
#include "gtk/gtk.h"
#endif
#include <string>


class StatusIcon {

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

  private:
    unsigned char current_icon_;
    std::string icons_folder_;
  #if defined(__linux__)
    GtkStatusIcon* icon_handler_;
  #endif

};

#endif /* end of BRICK_BASE_STATUS_ICON_H_ */
