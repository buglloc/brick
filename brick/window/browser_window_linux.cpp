#include <gtk/gtkeventbox.h>
#include <gtk/gtk.h>
#include "../window_util.h"
#include "browser_window.h"

void
BrowserWindow::WrapNative(CefWindowHandle window) {
  window_handler_ = gdk_window_foreign_new(window);
  g_object_set_data(G_OBJECT(window_handler_), "wrapper", this);
  gdk_window_set_icon_list(window_handler_, window_util::GetDefaultIcons());
}

void
BrowserWindow::Resize(int width, int height) {
  gdk_window_resize(window_handler_, width, height);
}

void
BrowserWindow::SetMinSize(int width, int height) {
  GdkGeometry hints;
  hints.min_width = width;
  hints.min_height = height;
  gdk_window_set_geometry_hints(window_handler_, &hints, GDK_HINT_MIN_SIZE);
}

void
BrowserWindow::FrozeSize(int width, int height) {
  GdkGeometry hints = {0};
  if (width && height) {
    hints.min_width = width;
    hints.min_height = height;
    hints.max_width = width;
    hints.max_height = height;
  }

  gdk_window_set_geometry_hints(window_handler_, &hints, (GdkWindowHints) (GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));
}

void
BrowserWindow::SetTitle(std::string title) {
  gdk_window_set_title(window_handler_, title.c_str());
}

void
BrowserWindow::Show() {
  hided_ = false;
  gdk_window_show(window_handler_);
}

void
BrowserWindow::Hide() {
  hided_ = true;
  gdk_window_hide(window_handler_);
}

void
BrowserWindow::Close() {
  gdk_window_destroy(window_handler_);
}

void
BrowserWindow::Popupping() {
  gdk_window_set_type_hint(window_handler_, GDK_WINDOW_TYPE_HINT_DIALOG);
}
