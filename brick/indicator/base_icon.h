// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_INDICATOR_BASE_ICON_H_
#define BRICK_INDICATOR_BASE_ICON_H_
#pragma once

#include <string>

#include "include/cef_base.h"
#include "brick/brick_app.h"

class BaseIcon : public CefBase {

 public:
  explicit BaseIcon(std::string icons_dir);

  std::string GetIconPath(BrickApp::StatusIcon icon);
  std::string GetIconName(BrickApp::StatusIcon icon);

  virtual void SetIcon(BrickApp::StatusIcon icon) = 0;
  virtual void SetTooltip(const char* text) = 0;
  virtual void Show() = 0;

 protected:
  std::string icons_folder_;

  IMPLEMENT_REFCOUNTING(BaseIcon);
};

#endif  // BRICK_INDICATOR_BASE_ICON_H_
