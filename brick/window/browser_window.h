#ifndef BRICK_BROWSER_WINDOW_H_
#define BRICK_BROWSER_WINDOW_H_
#pragma once


#include <include/cef_base.h>
#include <include/base/cef_lock.h>
#include "base_window.h"

#include <include/internal/cef_linux.h>
#include "../osr_widget_gtk.h"

class BrowserWindow : public BaseWindow {

public:

  CefRefPtr<OSRWindow> GetOsrWindow();
  // Platform specific methods
  void Init() OVERRIDE;
  CefWindowHandle GetId();
  // Event handlers


  // Shared methods
//  virtual bool SetFocus(bool focused);
//  virtual bool HasFocus();

protected:
  CefRefPtr<OSRWindow> osr_window_;
IMPLEMENT_REFCOUNTING(BrowserWindow);
};

#endif /* end of BRICK_BROWSER_WINDOW_H_ */
