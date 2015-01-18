#ifndef BRICK_BROWSER_WINDOW_H_
#define BRICK_BROWSER_WINDOW_H_
#pragma once


#if defined(__linux__)
// The Linux client uses GTK instead of the underlying platform type (X11).
#include <gdk/gdkx.h>
#define BrowserWindowHandle GdkWindow*
#else

#endif

#include <include/cef_base.h>
#include <include/base/cef_lock.h>
#include <include/internal/cef_linux.h>


class BrowserWindow : public CefBase {

public:
  BrowserWindow() : window_handler_(NULL),
                 hided_(false),
                 focused_(true) {};

  BrowserWindowHandle GetHandler();

  // Platform specific methods
  virtual void WrapNative(CefWindowHandle window);
  virtual void Show();
  virtual void Hide();
  virtual bool GetHided();
  virtual void Close();
  virtual void SetMinSize(int width, int height);
  virtual void Resize(int width, int height);
  virtual void SetTitle(std::string title);
  virtual void Popupping();
  virtual void FrozeSize(int width, int height);
  // Event handlers


  // Shared methods
//  virtual bool SetFocus(bool focused);
//  virtual bool HasFocus();

protected:
  BrowserWindowHandle window_handler_;
  bool hided_;
  bool focused_;

IMPLEMENT_REFCOUNTING(BrowserWindow);
};

#endif /* end of BRICK_BROWSER_WINDOW_H_ */
