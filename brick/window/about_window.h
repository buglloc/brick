// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_WINDOW_ABOUT_WINDOW_H_
#define BRICK_WINDOW_ABOUT_WINDOW_H_
#pragma once

#include "brick/window/base_window.h"

class AboutWindow : public BaseWindow {

 public:
  AboutWindow() {}
  virtual ~AboutWindow() {}
  static AboutWindow* Instance();

  // Platform specific methods
  void Init() OVERRIDE;

 private:
  static AboutWindow* instance_;

IMPLEMENT_REFCOUNTING(AboutWindow);
};

#endif  // BRICK_WINDOW_ABOUT_WINDOW_H_
