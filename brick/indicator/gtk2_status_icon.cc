// Copyright (c) 2015 The Brick Authors.

#include "brick/indicator/gtk2_status_icon.h"

#include <string>

Gtk2StatusIcon::Gtk2StatusIcon(std::string icons_dir):
  BaseIcon(icons_dir) {

  icon_handler_ = gtk_status_icon_new();
}

void
Gtk2StatusIcon::SetIcon(IndicatorStatusIcon icon) {
  gtk_status_icon_set_from_file(icon_handler_, GetIconPath(icon).c_str());
}

void
Gtk2StatusIcon::SetTooltip(const char* text) {
  gtk_status_icon_set_tooltip_text(icon_handler_, text);
  gtk_status_icon_set_title(icon_handler_, text);
}

void
Gtk2StatusIcon::Show() {
  gtk_status_icon_set_visible(icon_handler_, true);
}

GtkStatusIcon*
Gtk2StatusIcon::GetHandler() {
  return icon_handler_;
}
