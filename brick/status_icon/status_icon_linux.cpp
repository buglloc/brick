#include <gtk/gtk.h>
#include <include/base/cef_logging.h>
#include "../event/indicator_state_event.h"
#include "../event/indicator_tooltip_event.h"
#include "../account_manager.h"
#include "../cef_handler.h"
#include "../brick_app.h"
#include "status_icon.h"

namespace {

    GtkWidget *menu;
    GtkWidget *accounts_menu;

    void
    status_icon_click(GtkWidget *status_icon, StatusIcon *self) {
      self->OnClick();
    }

#ifdef unity
#else
    void
    status_icon_popup(GtkWidget *status_icon, guint button, guint32 activate_time, StatusIcon *self) {
      if (self->OnPopup())
        return;

      gtk_widget_show_all(menu);
      gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, status_icon, button, activate_time);
    }
#endif
    void
    menu_about(GtkMenuItem *item, StatusIcon *self) {
      self->OnMenuAbout();
    }

    void
    menu_manage_accounts(GtkMenuItem *item, StatusIcon *self) {
      self->OnMenuManageAccount();
    }

    void
    menu_quit(GtkMenuItem *item, StatusIcon *self) {
      self->OnMenuQuit();
    }

    void
    menu_change_account(GtkMenuItem *item, StatusIcon *self) {
      int account_id = GPOINTER_TO_INT(
         g_object_get_data(G_OBJECT(item), "account_id")
      );
      self->OnMenuChangeAccount(account_id);
    }

} // namespace


void
StatusIcon::Init() {
#ifdef unity
  icon_handler_ = app_indicator_new(
          "brick",
          "indicator-messages",
          APP_INDICATOR_CATEGORY_APPLICATION_STATUS
  );
  launcher_handler_ = unity_launcher_entry_get_for_desktop_id(APP_COMMON_NAME ".desktop");
#else
  icon_handler_ = gtk_status_icon_new();
  g_signal_connect(icon_handler_, "activate", G_CALLBACK(status_icon_click), this);
  g_signal_connect_swapped(icon_handler_, "popup-menu", G_CALLBACK(status_icon_popup), this);
#endif

  // Create popup menu
  menu = gtk_menu_new();
  GtkWidget * show_item = gtk_menu_item_new_with_label ("Show/Hide");
  g_signal_connect(G_OBJECT(show_item), "activate", G_CALLBACK(status_icon_click), NULL);
  gtk_menu_append(GTK_MENU(menu), show_item);
  gtk_menu_append(GTK_MENU(menu), gtk_separator_menu_item_new());
  accounts_menu = gtk_menu_item_new_with_label("Accounts");
  gtk_menu_append(GTK_MENU(menu), accounts_menu);
  gtk_menu_append(GTK_MENU(menu), gtk_separator_menu_item_new());
  GtkWidget * about_item = gtk_menu_item_new_with_label ("About");
  g_signal_connect(G_OBJECT(about_item), "activate", G_CALLBACK(menu_about), NULL);
  gtk_menu_append(GTK_MENU(menu), about_item);
  GtkWidget * quit_item = gtk_menu_item_new_with_label ("Quit");
  g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(menu_quit), NULL);
  gtk_menu_append(GTK_MENU(menu), quit_item);

  UpdateAccountsMenu();
  RegisterEventListeners();

#ifdef unity
  app_indicator_set_status (icon_handler_, APP_INDICATOR_STATUS_ACTIVE);
  gtk_widget_show_all(menu);
  app_indicator_set_menu(icon_handler_, GTK_MENU(menu));
  app_indicator_set_secondary_activate_target(icon_handler_, show_item);
  app_indicator_set_icon_theme_path(icon_handler_, icons_folder_.c_str());
#else
  gtk_status_icon_set_visible(icon_handler_, true);
#endif
  SetIcon(DEFAULT);
}

void
StatusIcon::SetIcon(Icon icon) {
  current_icon_ = icon;
#ifdef unity
  app_indicator_set_icon(icon_handler_, GetIconName(icon).c_str());
#else
  gtk_status_icon_set_from_file(icon_handler_, GetIconPath(icon).c_str());
#endif

  IndicatorStateEvent e(GetIconName(icon));
  EventBus::FireEvent(e);
}

void
StatusIcon::SetTooltip(const char *tooltip) {
#ifdef unity
  app_indicator_set_title(icon_handler_, tooltip);
#else
  gtk_status_icon_set_tooltip_text(icon_handler_, tooltip);
  gtk_status_icon_set_title(icon_handler_, tooltip);
#endif

  IndicatorTooltipEvent e(tooltip);
  EventBus::FireEvent(e);
}

void
StatusIcon::UpdateAccountsMenu() {
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