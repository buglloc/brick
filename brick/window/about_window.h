#ifndef BRICK_ABOUT_WINDOW_H_
#define BRICK_ABOUT_WINDOW_H_
#pragma once

#if defined(__linux__)
// The Linux client uses GTK instead of the underlying platform type (X11).
#include <gtk/gtk.h>
#define ClientWindowHandle GtkWidget*
#else
#define ClientWindowHandle CefWindowHandle
#endif

#include <include/base/cef_lock.h>
#include <include/internal/cef_linux.h>

class AboutWindow {

public:

  AboutWindow();
  virtual ~AboutWindow() {};
  static AboutWindow* Instance();

  // Platform specific methods
  void Init();
  void Show();
  void Hide();

private:
  ClientWindowHandle window_handler_;
  static AboutWindow* instance_;

};

#endif /* end of BRICK_ABOUT_WINDOW_H_ */
