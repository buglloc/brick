#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <include/base/cef_logging.h>
#include "include/base/cef_scoped_ptr.h"
#include "brick/cef_handler.h"

namespace {

    void
    destroy_handler(GtkWidget *widget, gpointer data, MainWindow *self) {
      // Quitting CEF is handled in ClientHandler::OnBeforeClose().
    }

    gboolean
    delete_event_handler(GtkWidget *widget, GdkEvent *event, GtkWindow *window, MainWindow *self) {
      gtk_widget_hide(widget);
      return true;
    }

    void
    size_allocated_handler(GtkWidget *widget, GtkAllocation *allocation, void *data) {
      ClientHandler *handler = ClientHandler::GetInstance();

      if (!handler)
        return;

      CefRefPtr<CefBrowser> browser = handler->GetBrowser();
      if (!browser || browser->GetHost()->IsWindowRenderingDisabled())
        return;

      // Size the browser window to match the GTK widget.
      ::Display *xdisplay = cef_get_xdisplay();
      ::Window xwindow = browser->GetHost()->GetWindowHandle();
      XWindowChanges changes = {0};
      changes.width = allocation->width;
      changes.height = allocation->height;
      XConfigureWindow(xdisplay, xwindow, CWHeight | CWWidth, &changes);
    }

    gboolean
    focus_in_handler(GtkWidget *widget, GdkEventFocus *event, MainWindow *window) {
      if (!event->in)
        return false;

      return window->SetFocus(true);
    }

    gboolean
    focus_out_handler(GtkWidget *widget, GdkEventFocus *event, MainWindow *window) {
      if (event->in)
        return false;

      return window->SetFocus(false);
    }


    gboolean
    state_handler(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data) {
      ClientHandler *handler = ClientHandler::GetInstance();

      if (!handler)
        return true;

      if (!(event->changed_mask & GDK_WINDOW_STATE_ICONIFIED))
        return true;

      CefRefPtr<CefBrowser> browser = handler->GetBrowser();
      if (!browser)
        return true;

      const bool iconified = (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED);
      // Forward the state change event to the browser window.
      ::Display *xdisplay = cef_get_xdisplay();
      ::Window xwindow = browser->GetHost()->GetWindowHandle();

      // Retrieve the atoms required by the below XChangeProperty call.
      const char *kAtoms[] = {
         "_NET_WM_STATE",
         "ATOM",
         "_NET_WM_STATE_HIDDEN"
      };
      Atom atoms[3];
      int result = XInternAtoms(xdisplay, const_cast<char **>(kAtoms), 3, false,
         atoms);
      if (!result)
        NOTREACHED();

      if (iconified) {
        // Set the hidden property state value.
        scoped_ptr<Atom[]> data(new Atom[1]);
        data[0] = atoms[2];

        XChangeProperty(xdisplay,
           xwindow,
           atoms[0],  // name
           atoms[1],  // type
           32,  // size in bits of items in 'value'
           PropModeReplace,
           reinterpret_cast<const unsigned char *>(data.get()),
           1);  // num items
      } else {
        // Set an empty array of property state values.
        XChangeProperty(xdisplay,
           xwindow,
           atoms[0],  // name
           atoms[1],  // type
           32,  // size in bits of items in 'value'
           PropModeReplace,
           NULL,
           0);  // num items
      }

      return true;
    }

// Only in new version of CEF
//gboolean
//window_configure_callback(GtkWindow* window,
//   GdkEvent* event,
//   gpointer data) {
//  // Called when size, position or stack order changes.
//  ClientHandler* handler = ClientHandler::GetInstance();
//  if (handler) {
//    CefRefPtr<CefBrowser> browser = handler->GetBrowser();
//    if (browser) {
//      // Notify the browser of move/resize events so that:
//      // - Popup windows are displayed in the correct location and dismissed
//      //   when the window moves.
//      // - Drag&drop areas are updated accordingly.
//      browser->GetHost()->NotifyMoveOrResizeStarted();
//    }
//  }
//
//  return FALSE;  // Don't stop this message.
//}

} // namespace

void
MainWindow::Init() {
  gtk_init(0, NULL);
  window_handler_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size(GTK_WINDOW(window_handler_), MAIN_WINDOW_DEFAULT_WIDTH, MAIN_WINDOW_DEFAULT_HEIGHT);
  g_signal_connect(G_OBJECT(window_handler_), "focus-in-event", G_CALLBACK(focus_in_handler), this);
  g_signal_connect_after(G_OBJECT(window_handler_), "focus-out-event", G_CALLBACK(focus_out_handler), this);
  g_signal_connect(G_OBJECT(window_handler_), "window-state-event", G_CALLBACK(state_handler), NULL);
  g_signal_connect(G_OBJECT(window_handler_), "size-allocate", G_CALLBACK(size_allocated_handler), NULL);
  g_signal_connect(G_OBJECT(window_handler_), "destroy", G_CALLBACK(destroy_handler), this);
  g_signal_connect(G_OBJECT(window_handler_), "delete_event", G_CALLBACK(delete_event_handler), this);
  // Only in new version of CEF
//  g_signal_connect(G_OBJECT(window_handler_), "configure-event", G_CALLBACK(window_configure_callback), NULL);

  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window_handler_), vbox);
}


void
MainWindow::Minimize() {
  gtk_window_iconify(GTK_WINDOW(window_handler_));
}

void
MainWindow::Maximize() {
  gtk_window_maximize(GTK_WINDOW(window_handler_));
}

void
MainWindow::Restore() {
  if( this->GetState() == WINDOW_STATE_MAXIMIZED ) {
    gtk_window_unmaximize(GTK_WINDOW(window_handler_));
  } else if( this->GetState() == WINDOW_STATE_FULLSCREEN ) {
    gtk_window_unfullscreen(GTK_WINDOW(window_handler_));
  } else {
    gtk_window_deiconify(GTK_WINDOW(window_handler_));
  }

}

void
MainWindow::Show() {
  hided_ = false;
  gtk_widget_show_all(window_handler_);
}

void
MainWindow::Hide() {
  hided_ = true;
  gtk_widget_hide(window_handler_);
}

void
MainWindow::Focus() {
  gtk_window_present(GTK_WINDOW(window_handler_));
}

void
MainWindow::Destroy() {
  gtk_widget_destroy(window_handler_);
}

void
MainWindow::Move(int left, int top, int width, int height) {
  GtkWindow* window = GTK_WINDOW(window_handler_);
  gtk_window_move(window, left, top);
  gtk_window_resize(window, width, height);
}

void
MainWindow::Move(int left, int top) {
  gtk_window_move(GTK_WINDOW(window_handler_), left, top);
}

void
MainWindow::Resize(int width, int height) {
  gtk_window_resize(GTK_WINDOW(window_handler_), width, height);
}

const char*
MainWindow::GetTitle() {
  char* title = (char*) gtk_window_get_title(GTK_WINDOW(window_handler_));
  if(title == NULL)
    return "";
  else
    return title;
}

void
MainWindow::SetTitle(const char* title) {
  gtk_window_set_title(GTK_WINDOW(window_handler_), title);
}

void
MainWindow::Fullscreen() {
  gtk_window_fullscreen(GTK_WINDOW(window_handler_));
}

WINDOW_STATE
MainWindow::GetState() {
  gint state = gdk_window_get_state(window_handler_->window);
  if (state & GDK_WINDOW_STATE_FULLSCREEN) {
    return WINDOW_STATE_FULLSCREEN;
  } else if (state & GDK_WINDOW_STATE_MAXIMIZED) {
    return WINDOW_STATE_MAXIMIZED;
  } else if (state & GDK_WINDOW_STATE_ICONIFIED) {
    return WINDOW_STATE_MINIMIZED;
  } else {
    return WINDOW_STATE_NORMAL;
  }
  return WINDOW_STATE_NORMAL;
}

void
MainWindow::SetTopmost(bool ontop) {
  gtk_window_set_keep_above(GTK_WINDOW(window_handler_), ontop);
}


void
MainWindow::SetResizable(bool resizable) {
  gtk_window_set_resizable(GTK_WINDOW(window_handler_), resizable);
}

bool
MainWindow::GetResizable() {
  return (bool) gtk_window_get_resizable(GTK_WINDOW(window_handler_));
}

void
MainWindow::SetShowChrome(bool showChrome) {
  gtk_window_set_decorated(GTK_WINDOW(window_handler_), showChrome);
}

bool
MainWindow::GetShowChrome() {
  return (bool) gtk_window_get_decorated(GTK_WINDOW(window_handler_));
}

void
MainWindow::SetOpacity(double opacity) {
  gtk_window_set_opacity(GTK_WINDOW(window_handler_), opacity);
}

double
MainWindow::GetOpacity() {
  return gtk_window_get_opacity(GTK_WINDOW(window_handler_));
}