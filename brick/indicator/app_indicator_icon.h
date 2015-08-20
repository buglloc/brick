// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_INDICATOR_APP_INDICATOR_ICON_H_
#define BRICK_INDICATOR_APP_INDICATOR_ICON_H_
#pragma once

#include <gtk/gtk.h>
#include <string>
#include "brick/indicator/base_icon.h"

typedef struct _AppIndicator AppIndicator;

class AppIndicatorIcon : public BaseIcon {

 public:
  explicit AppIndicatorIcon(std::string icons_dir);

  virtual void SetIcon(IndicatorStatusIcon icon);
  virtual void SetTooltip(const char* text);
  virtual void Show();

  void SetMenu(GtkWidget *menu, GtkWidget *activate_item);

  // Indicates whether libappindicator so could be opened.
  static bool CouldOpen();

 private:
  AppIndicator *icon_handler_;
  GtkWidget *menu_;

  IMPLEMENT_REFCOUNTING(AppIndicatorIcon);
};

#endif  // BRICK_INDICATOR_APP_INDICATOR_ICON_H_
