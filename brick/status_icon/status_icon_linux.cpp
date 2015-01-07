#include <gtk/gtk.h>
#include <include/base/cef_logging.h>
#include <brick/account_manager.h>
#include <brick/cef_handler.h>
#include "status_icon.h"

namespace {

  GtkWidget *menu;

  void
  status_icon_click(GtkWidget *status_icon, StatusIcon *self) {
    self->OnClick();
  }

  void
  status_icon_popup(GtkWidget *status_icon, guint button, guint32 activate_time, StatusIcon *self) {
    if (self->OnPopup())
      return;

    gtk_widget_show_all(menu);
    gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, status_icon, button, activate_time);
  }

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
  icon_handler_ = gtk_status_icon_new();
  g_signal_connect(icon_handler_, "activate", G_CALLBACK(status_icon_click), this);
  g_signal_connect_swapped(icon_handler_, "popup-menu", G_CALLBACK(status_icon_popup), this);

  // Create accounts menu
  GtkWidget *accounts_menu = gtk_menu_new();
  GtkWidget *manage_accounts_item = gtk_menu_item_new_with_label("Manage Accounts");
  g_signal_connect(G_OBJECT(manage_accounts_item), "activate", G_CALLBACK(menu_manage_accounts), NULL);
  gtk_menu_append(GTK_MENU(accounts_menu), manage_accounts_item);
  gtk_menu_append(GTK_MENU(accounts_menu), gtk_separator_menu_item_new());
  AccountManager::accounts_map *accounts = ClientHandler::GetInstance()->GetAccountManager()->GetAccounts();
  AccountManager::accounts_map::iterator it = accounts->begin();
  for (; it != accounts->end(); ++it) {
    GtkWidget *account_item = gtk_menu_item_new_with_label(
       (*it).second->GetLabel().c_str()
    );
    gtk_menu_append(GTK_MENU(accounts_menu), account_item);
    g_object_set_data(G_OBJECT(account_item), "account_id", GINT_TO_POINTER((*it).first));
    g_signal_connect(G_OBJECT(account_item), "activate", G_CALLBACK(menu_change_account), this);
  }

  // Create popup menu
  menu = gtk_menu_new();
  GtkWidget * accounts_item = gtk_menu_item_new_with_label("Accounts");
  gtk_menu_append(GTK_MENU(menu), accounts_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(accounts_item), accounts_menu);
  gtk_menu_append(GTK_MENU(menu), gtk_separator_menu_item_new());
  GtkWidget * about_item = gtk_menu_item_new_with_label ("About");
  g_signal_connect(G_OBJECT(about_item), "activate", G_CALLBACK(menu_about), NULL);
  gtk_menu_append(GTK_MENU(menu), about_item);
  GtkWidget * quit_item = gtk_menu_item_new_with_label ("Quit");
  g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(menu_quit), NULL);
  gtk_menu_append(GTK_MENU(menu), quit_item);

  gtk_status_icon_set_visible(icon_handler_, true);
  SetIcon(DEFAULT);
}

void
StatusIcon::SetIcon(Icon icon) {
  current_icon_ = icon;
  gtk_status_icon_set_from_file(icon_handler_, GetIconPath(icon).c_str());
}

void
StatusIcon::SetTooltip(const char *tooltip) {
  gtk_status_icon_set_tooltip_text(icon_handler_, tooltip);
  gtk_status_icon_set_title(icon_handler_, tooltip);
}
