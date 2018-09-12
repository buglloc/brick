// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_INDICATOR_GTK2_STATUS_ICON_H_
#define BRICK_INDICATOR_GTK2_STATUS_ICON_H_
#pragma once

#include <gtk/gtk.h>
#include <string>
#include "brick/indicator/base_icon.h"

class Gtk2StatusIcon : public BaseIcon {

 public:
  explicit Gtk2StatusIcon(std::string icons_dir);

  virtual void SetIcon(IndicatorStatusIcon icon) override;
  virtual void SetTooltip(const char* text) override;
  virtual void Show() override;

  GtkStatusIcon* GetHandler();

 private:
  GtkStatusIcon* icon_handler_;

  IMPLEMENT_REFCOUNTING(Gtk2StatusIcon);
};

#endif  // BRICK_INDICATOR_GTK2_STATUS_ICON_H_
