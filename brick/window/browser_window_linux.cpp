#include <gtk/gtk.h>
#include <brick/osr_widget_gtk.h>
#include "../window_util.h"
#include "browser_window.h"
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include "include/base/cef_scoped_ptr.h"
#include "include/base/cef_logging.h"
#include "../cef_handler.h"

namespace {

    class MainBrowserProvider : public OSRBrowserProvider {
      virtual CefRefPtr<CefBrowser> GetBrowser() {
        if (ClientHandler::GetInstance())
          return ClientHandler::GetInstance()->GetBrowser();

        return NULL;
      }
    } g_main_browser_provider;

    void
    OnDestroy(GtkWidget* widget, BrowserWindow* self) {
      self->Release();
    }

    gboolean
    OnDelete(GtkWidget* widget, GdkEvent* event,
       GtkWindow* window) {

      if (ClientHandler::GetInstance()) {
        CefRefPtr<CefBrowser> browser = ClientHandler::GetInstance()->GetBrowser();
        if (browser.get()) {
          // Notify the browser window that we would like to close it. This
          // will result in a call to ClientHandler::DoClose() if the
          // JavaScript 'onbeforeunload' event handler allows it.
          browser->GetHost()->CloseBrowser(false);

          // Cancel the close.
          return true;
        }
      }

      // Allow the close.
      return false;
    }

    gboolean
    OnFocusIn(GtkWidget *widget,
       GdkEventFocus *event,
       gpointer user_data) {
      if (ClientHandler::GetInstance() && event->in) {
        CefRefPtr<CefBrowser> browser = ClientHandler::GetInstance()->GetBrowser();
        if (browser) {
          if (browser->GetHost()->IsWindowRenderingDisabled()) {
            // Give focus to the off-screen browser.
            browser->GetHost()->SendFocusEvent(true);
          } else {
            // Give focus to the browser window.
            browser->GetHost()->SetFocus(true);
            return true;
          }
        }
      }

      return false;
    }

    gboolean
    OnState(GtkWidget *widget,
       GdkEventWindowState *event,
       gpointer user_data) {
      if (!(event->changed_mask & GDK_WINDOW_STATE_ICONIFIED))
        return true;

      if (!ClientHandler::GetInstance())
        return true;

      CefRefPtr<CefBrowser> browser = ClientHandler::GetInstance()->GetBrowser();
      if (!browser)
        return true;

      const bool iconified = (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED);
      if (browser->GetHost()->IsWindowRenderingDisabled()) {
        // Notify the off-screen browser that it was shown or hidden.
        browser->GetHost()->WasHidden(iconified);
      } else {
        // Forward the state change event to the browser window.
        ::Display* xdisplay = cef_get_xdisplay();
        ::Window xwindow = browser->GetHost()->GetWindowHandle();

        // Retrieve the atoms required by the below XChangeProperty call.
        const char* kAtoms[] = {
           "_NET_WM_STATE",
           "ATOM",
           "_NET_WM_STATE_HIDDEN"
        };
        Atom atoms[3];
        int result = XInternAtoms(xdisplay, const_cast<char**>(kAtoms), 3, false,
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
             reinterpret_cast<const unsigned char*>(data.get()),
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
      }

      return true;
    }

    gboolean
    OnConfigure(GtkWindow *window,
       GdkEvent *event,
       gpointer data) {
      // Called when size, position or stack order changes.
      if (ClientHandler::GetInstance()) {
        CefRefPtr<CefBrowser> browser = ClientHandler::GetInstance()->GetBrowser();
        if (browser) {
          // Notify the browser of move/resize events so that:
          // - Popup windows are displayed in the correct location and dismissed
          //   when the window moves.
          // - Drag&drop areas are updated accordingly.
          browser->GetHost()->NotifyMoveOrResizeStarted();
        }
      }

      return false;  // Don't stop this message.
    }

    void
    OnVboxSizeAllocated(GtkWidget *widget,
       GtkAllocation *allocation,
       void *data) {
      if (ClientHandler::GetInstance()) {
        CefRefPtr<CefBrowser> browser = ClientHandler::GetInstance()->GetBrowser();
        if (browser && !browser->GetHost()->IsWindowRenderingDisabled()) {
          // Size the browser window to match the GTK widget.
          ::Display* xdisplay = cef_get_xdisplay();
          ::Window xwindow = browser->GetHost()->GetWindowHandle();
          XWindowChanges changes = {0};
          changes.width = allocation->width;
          changes.height = allocation->height;
          XConfigureWindow(xdisplay, xwindow, CWHeight | CWWidth, &changes);
        }
      }
    }
}

void
BrowserWindow::Init() {
  // Increment ref counting, it will release on gtk window destroy
  AddRef();
  window_handler_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window_handler_), 800, 600);

  g_signal_connect(window_handler_, "focus-in-event", G_CALLBACK(OnFocusIn), this);
  g_signal_connect(window_handler_, "window-state-event", G_CALLBACK(OnState), this);
  g_signal_connect(window_handler_, "configure-event", G_CALLBACK(OnConfigure), this);
  g_signal_connect(window_handler_, "destroy", G_CALLBACK(OnDestroy), this);
  g_signal_connect(window_handler_, "delete_event", G_CALLBACK(OnDelete), this);

  GtkWidget* vbox = gtk_vbox_new(false, 0);
  g_signal_connect(vbox, "size-allocate", G_CALLBACK(OnVboxSizeAllocated), NULL);
  gtk_container_add(GTK_CONTAINER(window_handler_), vbox);

  // Create the GTKGL surface.
  osr_window_ = OSRWindow::Create(&g_main_browser_provider, false, false, vbox);
}

CefWindowHandle
BrowserWindow::GetId() {
  return GDK_WINDOW_XID(gtk_widget_get_window(osr_window_->GetWindowHandle()));
}