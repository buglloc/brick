#include <gtk/gtk.h>
#include <include/base/cef_logging.h>
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
    menu_quit(GtkMenuItem *item, StatusIcon *self) {
      self->OnMenuQuit();
    }
} // namespace


void
StatusIcon::Init() {
  icon_handler_ = gtk_status_icon_new();
  g_signal_connect(icon_handler_, "activate", G_CALLBACK(status_icon_click), this);
  g_signal_connect_swapped(icon_handler_, "popup-menu", G_CALLBACK(status_icon_popup), this);

  // Create menu
  menu = gtk_menu_new();
  GtkWidget * quit_item = gtk_menu_item_new_with_label ("Quit");
  g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(menu_quit), NULL);
  gtk_menu_append(GTK_MENU(menu), quit_item);
  gtk_widget_show (quit_item);


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
