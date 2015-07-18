// Copyright (c) 2015 The Brick Authors.

#include "brick/indicator/app_indicator_icon.h"

#include <dlfcn.h>
#include <string>

#include "brick/platform_util.h"


namespace {

  typedef enum {
    APP_INDICATOR_CATEGORY_APPLICATION_STATUS,
    APP_INDICATOR_CATEGORY_COMMUNICATIONS,
    APP_INDICATOR_CATEGORY_SYSTEM_SERVICES,
    APP_INDICATOR_CATEGORY_HARDWARE,
    APP_INDICATOR_CATEGORY_OTHER
  } AppIndicatorCategory;

  typedef enum {
    APP_INDICATOR_STATUS_PASSIVE,
    APP_INDICATOR_STATUS_ACTIVE,
    APP_INDICATOR_STATUS_ATTENTION
  } AppIndicatorStatus;

  typedef AppIndicator* (*app_indicator_new_func)(const gchar* id,
                                                  const gchar* icon_name,
                                                  AppIndicatorCategory category);

  typedef void (*app_indicator_set_status_func)(AppIndicator* self,
                                                AppIndicatorStatus status);

  typedef void (*app_indicator_set_menu_func)(AppIndicator* self, GtkMenu* menu);

  typedef void (*app_indicator_set_icon_func)(AppIndicator* self,
                                                   const gchar* icon_name);

  typedef void (*app_indicator_set_icon_theme_path_func)(
      AppIndicator* self,
      const gchar* icon_theme_path);

  typedef void (*app_indicator_set_title_func)(AppIndicator* self,
                                              const gchar* title);

  typedef void (*app_indicator_set_secondary_activate_target_func)(AppIndicator* self,
                                                                   GtkWidget* menu);

  bool g_attempted_load = false;
  bool g_opened = false;

  // Retrieved functions from libappindicator.
  app_indicator_new_func app_indicator_new = NULL;
  app_indicator_set_status_func app_indicator_set_status = NULL;
  app_indicator_set_menu_func app_indicator_set_menu = NULL;
  app_indicator_set_icon_func app_indicator_set_icon = NULL;
  app_indicator_set_icon_theme_path_func app_indicator_set_icon_theme_path = NULL;
  app_indicator_set_title_func app_indicator_set_title = NULL;
  app_indicator_set_secondary_activate_target_func app_indicator_set_secondary_activate_target = NULL;

  void EnsureMethodsLoaded() {
    if (g_attempted_load)
      return;

    g_attempted_load = true;

    // Only use libappindicator where it is needed to support dbus based status
    // icons. In particular, libappindicator does not support a click action.
    platform_util::DesktopEnvironment environment = platform_util::GetDesktopEnvironment();
    if (environment != platform_util::DESKTOP_ENVIRONMENT_KDE &&
        environment != platform_util::DESKTOP_ENVIRONMENT_UNITY) {
      return;
    }

    void* indicator_lib = dlopen("libappindicator.so", RTLD_LAZY);
    if (!indicator_lib) {
      indicator_lib = dlopen("libappindicator3.so.1", RTLD_LAZY);
    }
    if (!indicator_lib) {
      indicator_lib = dlopen("libappindicator.so.1", RTLD_LAZY);
    }
    if (!indicator_lib) {
      indicator_lib = dlopen("libappindicator.so.0", RTLD_LAZY);
    }

    if (!indicator_lib) {
      return;
    }

    g_opened = true;

    app_indicator_new = reinterpret_cast<app_indicator_new_func>(
        dlsym(indicator_lib, "app_indicator_new"));

    app_indicator_set_status = reinterpret_cast<app_indicator_set_status_func>(
        dlsym(indicator_lib, "app_indicator_set_status"));

    app_indicator_set_menu = reinterpret_cast<app_indicator_set_menu_func>(
        dlsym(indicator_lib, "app_indicator_set_menu"));

    app_indicator_set_icon =
        reinterpret_cast<app_indicator_set_icon_func>(
            dlsym(indicator_lib, "app_indicator_set_icon"));

    app_indicator_set_icon_theme_path =
        reinterpret_cast<app_indicator_set_icon_theme_path_func>(
            dlsym(indicator_lib, "app_indicator_set_icon_theme_path"));

    app_indicator_set_title =
        reinterpret_cast<app_indicator_set_title_func>(
            dlsym(indicator_lib, "app_indicator_set_title"));

    app_indicator_set_secondary_activate_target =
        reinterpret_cast<app_indicator_set_secondary_activate_target_func>(
            dlsym(indicator_lib, "app_indicator_set_secondary_activate_target"));

  }

}  // namespace

AppIndicatorIcon::AppIndicatorIcon(std::string icons_dir):
    BaseIcon(icons_dir) {

  icon_handler_ = app_indicator_new(
      "brick",
      "indicator-messages",
      APP_INDICATOR_CATEGORY_APPLICATION_STATUS
  );
  app_indicator_set_icon_theme_path(icon_handler_, icons_folder_.c_str());
}

void
AppIndicatorIcon::SetIcon(BrickApp::StatusIcon icon) {
  app_indicator_set_icon(icon_handler_, GetIconName(icon).c_str());
}

void
AppIndicatorIcon::SetTooltip(const char* text) {
  app_indicator_set_title(icon_handler_, text);
}

void
AppIndicatorIcon::Show() {
  app_indicator_set_status(icon_handler_, APP_INDICATOR_STATUS_ACTIVE);
  gtk_widget_show_all(menu_);
}

void
AppIndicatorIcon::SetMenu(GtkWidget *menu, GtkWidget *activate_item) {
  menu_ = menu;
  app_indicator_set_menu(icon_handler_, GTK_MENU(menu_));
  app_indicator_set_secondary_activate_target(icon_handler_, activate_item);
}

// static
bool AppIndicatorIcon::CouldOpen() {
  EnsureMethodsLoaded();
  return g_opened;
}
