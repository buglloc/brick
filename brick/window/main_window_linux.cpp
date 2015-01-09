#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <include/base/cef_logging.h>
#include <include/base/cef_scoped_ptr.h>
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
      CefRefPtr<ClientHandler> handler = ClientHandler::GetInstance();

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

      window->SetFocus(true);
      return true;
    }

    gboolean
    focus_out_handler(GtkWidget *widget, GdkEventFocus *event, MainWindow *window) {
      if (event->in)
        return false;

      window->SetFocus(false);
      return false;
    }


    gboolean
    state_handler(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data) {
      CefRefPtr<ClientHandler> handler = ClientHandler::GetInstance();

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
//  CefRefPtr<ClientHandler> handler = ClientHandler::GetInstance();
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

  GtkWidget* vbox = gtk_vbox_new(false, 0);
  gtk_container_add(GTK_CONTAINER(window_handler_), vbox);
}