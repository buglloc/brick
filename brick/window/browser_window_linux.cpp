
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <include/base/cef_logging.h>

#include "../window_util.h"
#include "browser_window.h"


void
BrowserWindow::WrapNative(CefWindowHandle window) {
  if (window_handler_) {
    LOG(ERROR) << "Browser window already have native window. Can't wrap one more window.";
    return;
  }

  AddRef();
  window_handler_ = gdk_window_foreign_new(window);
  g_object_set_data(G_OBJECT(window_handler_), "wrapper", this);
  gdk_window_set_icon_list(window_handler_, window_util::GetDefaultIcons());
  gdk_window_set_events(window_handler_, (GdkEventMask) (
          GDK_STRUCTURE_MASK|GDK_VISIBILITY_NOTIFY_MASK
  ));
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
BrowserWindow::Present() {
  Show();
  if (!hided_) {
      // If window mapped - activate it immediately
      SetActive();
  }
}

void
BrowserWindow::Show() {
  bool handled = OnShow();
  if (!handled)
    gdk_window_show(window_handler_);
}

void
BrowserWindow::Hide() {
  bool handled = OnHide();
  if (!handled)
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

void
BrowserWindow::OnNativeEvent(BrowserWindowNativeEvent event) {
  // ToDo: implement native window event handlers
  switch (event->type) {
    case GDK_DESTROY:
      // Release reference when wrapped window destroy
      Release();
      break;
    case GDK_MAP:
      hided_ = false;
      SetActive();
      break;
    case GDK_UNMAP:
      hided_ = true;
      break;
    case GDK_VISIBILITY_NOTIFY:
      visible_ = (event->visibility.state != GDK_VISIBILITY_FULLY_OBSCURED);
      break;
    default:
//      LOG(INFO) << "window: " << window_handler_ << ", event: " << event->type;
      break;
  }

}

void
BrowserWindow::ToggleVisibility() {
  bool is_hided = (
     hided_
     || !visible_
     || (gdk_window_get_state(window_handler_) & (GDK_WINDOW_STATE_ICONIFIED|GDK_WINDOW_STATE_WITHDRAWN))
  );

  if (is_hided) {
    Present();
  } else {
    Hide();
  }
}

void
BrowserWindow::SetActive() {
  gdk_window_focus(window_handler_,
          gdk_x11_display_get_user_time(gdk_window_get_display(window_handler_))
  );
}