// Copyright (c) 2015 The Brick Authors.

#include "brick/indicator/indicator.h"

#include <gtk/gtk.h>

#include "include/base/cef_logging.h"
#include "brick/event/indicator_state_event.h"
#include "brick/event/indicator_tooltip_event.h"
#include "brick/account_manager.h"
#include "brick/client_handler.h"
#include "brick/brick_app.h"
#include "brick/indicator/app_indicator_icon.h"
#include "brick/indicator/gtk2_status_icon.h"

namespace {

  GtkWidget *menu;
  GtkWidget *accounts_menu;

  void
  status_icon_click(GtkWidget *status_icon, BrickIndicator *self) {
    self->OnClick();
  }

  void
  status_icon_popup(GtkWidget *status_icon, guint button, guint32 activate_time, BrickIndicator *self) {
    if (self->OnPopup())
      return;

    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, status_icon, button, activate_time);
  }

  void
  menu_about(GtkMenuItem *item, BrickIndicator *self) {
    self->OnMenuAbout();
  }

  void
  menu_manage_accounts(GtkMenuItem *item, BrickIndicator *self) {
    self->OnMenuManageAccount();
  }

  void
  menu_quit(GtkMenuItem *item, BrickIndicator *self) {
    self->OnMenuQuit();
  }

  void
  menu_change_account(GtkMenuItem *item, BrickIndicator *self) {
    int account_id = GPOINTER_TO_INT(
       g_object_get_data(G_OBJECT(item), "account_id")
    );
    self->OnMenuChangeAccount(account_id);
  }

}  // namespace


void
BrickIndicator::Init() {
#ifdef unity
  unity_launcher_->Init();
#endif
  // Create popup menu
  menu = gtk_menu_new();
  GtkWidget * show_item = gtk_menu_item_new_with_label("Show/Hide");
  g_signal_connect(G_OBJECT(show_item), "activate", G_CALLBACK(status_icon_click), NULL);
  gtk_menu_append(GTK_MENU(menu), show_item);
  gtk_menu_append(GTK_MENU(menu), gtk_separator_menu_item_new());
  accounts_menu = gtk_menu_item_new_with_label("Accounts");
  gtk_menu_append(GTK_MENU(menu), accounts_menu);
  gtk_menu_append(GTK_MENU(menu), gtk_separator_menu_item_new());
  GtkWidget * about_item = gtk_menu_item_new_with_label("About");
  g_signal_connect(G_OBJECT(about_item), "activate", G_CALLBACK(menu_about), NULL);
  gtk_menu_append(GTK_MENU(menu), about_item);
  GtkWidget * quit_item = gtk_menu_item_new_with_label("Quit");
  g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(menu_quit), NULL);
  gtk_menu_append(GTK_MENU(menu), quit_item);

  if (AppIndicatorIcon::CouldOpen()) {
    CefRefPtr<AppIndicatorIcon> icon(new AppIndicatorIcon(icons_folder_));
    icon->SetMenu(menu, show_item);
    icon_ = icon;
  } else {
    CefRefPtr<Gtk2StatusIcon> icon(new Gtk2StatusIcon(icons_folder_));
    g_signal_connect(icon->GetHandler(), "activate", G_CALLBACK(status_icon_click), this);
    g_signal_connect_swapped(icon->GetHandler(), "popup-menu", G_CALLBACK(status_icon_popup), this);
    icon_ = icon;
  }

  UpdateAccountsMenu();
  RegisterEventListeners();

  SwitchToIdle();
  icon_->Show();
}

void
BrickIndicator::SetIdleIcon(IndicatorStatusIcon icon) {
  if (extended_status_) {
    idle_icon_ = icon;
  } else {
    if (icon > IndicatorStatusIcon::OFFLINE &&  icon < IndicatorStatusIcon::FLASH)
      idle_icon_ = IndicatorStatusIcon::ONLINE;
    else
      idle_icon_ = icon;
  }

  if (idle_)
    SetIcon(idle_icon_);
}

void
BrickIndicator::SetIcon(IndicatorStatusIcon icon) {
  idle_ = icon < IndicatorStatusIcon::FLASH;

  current_icon_ = icon;
  icon_->SetIcon(icon);

  IndicatorStateEvent e(icon_->GetIconName(icon));
  EventBus::FireEvent(e);
}

void
BrickIndicator::SetTooltip(const char *tooltip) {
  icon_->SetTooltip(tooltip);

  IndicatorTooltipEvent e(tooltip);
  EventBus::FireEvent(e);
}

void
BrickIndicator::UpdateAccountsMenu() {
  GtkWidget *submenu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(accounts_menu), submenu);
  GtkWidget *manage_accounts_item = gtk_menu_item_new_with_label("Manage Accounts");
  g_signal_connect(G_OBJECT(manage_accounts_item), "activate", G_CALLBACK(menu_manage_accounts), NULL);
  gtk_menu_append(GTK_MENU(submenu), manage_accounts_item);
  gtk_menu_append(GTK_MENU(submenu), gtk_separator_menu_item_new());
  AccountManager::accounts_map *accounts = ClientHandler::GetInstance()->GetAccountManager()->GetAccounts();
  CefRefPtr<Account> current_account = ClientHandler::GetInstance()->GetAccountManager()->GetCurrentAccount();
  AccountManager::accounts_map::iterator it = accounts->begin();
  for (; it != accounts->end(); ++it) {
    GtkWidget *account_item;
    if (current_account == (*it).second) {
      account_item = gtk_menu_item_new_with_label(
         (" --> " + (*it).second->GetLabel()).c_str()
      );
    } else {
      account_item = gtk_menu_item_new_with_label(
         (*it).second->GetLabel().c_str()
      );
    }

    gtk_menu_append(GTK_MENU(submenu), account_item);
    g_object_set_data(G_OBJECT(account_item), "account_id", GINT_TO_POINTER((*it).first));
    g_signal_connect(G_OBJECT(account_item), "activate", G_CALLBACK(menu_change_account), this);
  }
  gtk_widget_show_all(submenu);
}
