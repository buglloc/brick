// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_WINDOW_WINDOW_FEATURES_H_
#define BRICK_WINDOW_WINDOW_FEATURES_H_
#pragma once

#include "include/internal/cef_types_wrappers.h"
#include "include/cef_base.h"

class WindowFeatures : public CefBase {

 public:
  explicit WindowFeatures(const CefPopupFeatures& popupFeatures);

  int x;
  int y;
  int width;
  int height;
  bool dialog;
  bool topmost;

IMPLEMENT_REFCOUNTING(WindowFeatures);
};

#endif  // BRICK_WINDOW_WINDOW_FEATURES_H_

