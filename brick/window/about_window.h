#ifndef BRICK_ABOUT_WINDOW_H_
#define BRICK_ABOUT_WINDOW_H_
#pragma once

#include "base_window.h"

class AboutWindow : public BaseWindow {

public:

  AboutWindow() {};
  virtual ~AboutWindow() {};
  static AboutWindow* Instance();

  // Platform specific methods
  void Init() OVERRIDE;

private:
  static AboutWindow* instance_;

IMPLEMENT_REFCOUNTING(AboutWindow);
};

#endif /* end of BRICK_ABOUT_WINDOW_H_ */
