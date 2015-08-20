// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_INDICATOR_BASE_ICON_H_
#define BRICK_INDICATOR_BASE_ICON_H_
#pragma once

#include <string>

#include "include/cef_base.h"
#include "brick/brick_types.h"

class BaseIcon : public CefBase {

 public:
  explicit BaseIcon(std::string icons_dir);

  std::string GetIconPath(IndicatorStatusIcon icon);
  std::string GetIconName(IndicatorStatusIcon icon);

  virtual void SetIcon(IndicatorStatusIcon icon) = 0;
  virtual void SetTooltip(const char* text) = 0;
  virtual void Show() = 0;

 protected:
  std::string icons_folder_;

  IMPLEMENT_REFCOUNTING(BaseIcon);
};

#endif  // BRICK_INDICATOR_BASE_ICON_H_
