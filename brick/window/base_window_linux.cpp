#include "base_window.h"

void
BaseWindow::Minimize() {
  gtk_window_iconify(GTK_WINDOW(window_handler_));
}

void
BaseWindow::Maximize() {
  gtk_window_maximize(GTK_WINDOW(window_handler_));
}

void
BaseWindow::Restore() {
  if( this->GetState() == STATE_MAXIMIZED ) {
    gtk_window_unmaximize(GTK_WINDOW(window_handler_));
  } else if( this->GetState() == STATE_FULLSCREEN ) {
    gtk_window_unfullscreen(GTK_WINDOW(window_handler_));
  } else {
    gtk_window_deiconify(GTK_WINDOW(window_handler_));
  }

}

void
BaseWindow::Show() {
  hided_ = false;
  gtk_widget_show_all(window_handler_);
}

void
BaseWindow::Hide() {
  hided_ = true;
  gtk_widget_hide(window_handler_);
}

void
BaseWindow::Close() {
  gtk_widget_destroy(window_handler_);
}

void
BaseWindow::Focus() {
  gtk_window_present(GTK_WINDOW(window_handler_));
}

void
BaseWindow::Destroy() {
  gtk_widget_destroy(window_handler_);
}

void
BaseWindow::Move(int left, int top, int width, int height) {
  GtkWindow* window = GTK_WINDOW(window_handler_);
  gtk_window_move(window, left, top);
  gtk_window_resize(window, width, height);
}

void
BaseWindow::Move(int left, int top) {
  gtk_window_move(GTK_WINDOW(window_handler_), left, top);
}

void
BaseWindow::Resize(int width, int height) {
  gtk_window_resize(GTK_WINDOW(window_handler_), width, height);
}

const char*
BaseWindow::GetTitle() {
  char* title = (char*) gtk_window_get_title(GTK_WINDOW(window_handler_));
  if(title == NULL)
    return "";
  else
    return title;
}

void
BaseWindow::SetTitle(const char* title) {
  gtk_window_set_title(GTK_WINDOW(window_handler_), title);
}

void
BaseWindow::Fullscreen() {
  gtk_window_fullscreen(GTK_WINDOW(window_handler_));
}

BaseWindow::STATE
BaseWindow::GetState() {
  gint state = gdk_window_get_state(window_handler_->window);
  if (state & GDK_WINDOW_STATE_FULLSCREEN) {
    return STATE_FULLSCREEN;
  } else if (state & GDK_WINDOW_STATE_MAXIMIZED) {
    return STATE_MAXIMIZED;
  } else if (state & GDK_WINDOW_STATE_ICONIFIED) {
    return STATE_MINIMIZED;
  } else {
    return STATE_NORMAL;
  }
}

void
BaseWindow::SetTopmost(bool ontop) {
  gtk_window_set_keep_above(GTK_WINDOW(window_handler_), ontop);
}


void
BaseWindow::SetResizable(bool resizable) {
  gtk_window_set_resizable(GTK_WINDOW(window_handler_), resizable);
}

bool
BaseWindow::GetResizable() {
  return (bool) gtk_window_get_resizable(GTK_WINDOW(window_handler_));
}

void
BaseWindow::SetShowChrome(bool showChrome) {
  gtk_window_set_decorated(GTK_WINDOW(window_handler_), showChrome);
}

bool
BaseWindow::GetShowChrome() {
  return (bool) gtk_window_get_decorated(GTK_WINDOW(window_handler_));
}

void
BaseWindow::SetOpacity(double opacity) {
  gtk_window_set_opacity(GTK_WINDOW(window_handler_), opacity);
}

double
BaseWindow::GetOpacity() {
  return gtk_window_get_opacity(GTK_WINDOW(window_handler_));
}