// Copyright (c) 2015 The Brick Authors.

#include "brick/window_util.h"

#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <cmath>

#include "include/internal/cef_linux.h"
#include "include/base/cef_logging.h"

#include "brick/brick_app.h"

namespace {
  GList *default_icons = NULL;
  CefWindowHandle leader_window_;
  double device_scale_factor = 0;
  const double kCSSDefaultDPI = 96.0;

  int
  XErrorHandlerImpl(Display *display, XErrorEvent *event) {
    LOG(WARNING)
    << "X error received: "
    << "type " << event->type << ", "
    << "serial " << event->serial << ", "
    << "error_code " << static_cast<int>(event->error_code) << ", "
    << "request_code " << static_cast<int>(event->request_code) << ", "
    << "minor_code " << static_cast<int>(event->minor_code);
    return 0;
  }

  int
  XIOErrorHandlerImpl(Display *display) {
    return 0;
  }
}  // namespace

namespace window_util {

  CefWindowHandle
  GetParent(CefWindowHandle handle) {
    ::Window root;
    ::Window parent;
    ::Window *children;
    unsigned int nchildren;
    XQueryTree(cef_get_xdisplay(), handle, &root, &parent, &children, &nchildren);
    XFree(children);

    return parent;
  }

  void
  Resize(CefWindowHandle handle, int width, int height) {
    XResizeWindow(
       cef_get_xdisplay(),
       handle,
       (unsigned int) width,
       (unsigned int) height
    );
  }

  void
  SetMinSize(CefWindowHandle handle, int width, int height) {
    XSizeHints *size_hints = XAllocSizeHints();
    if (!size_hints) {
      LOG(ERROR) << "SetMinSize: Can't allocate memory for XAllocSizeHints";
      return;
    }

    size_hints->flags = PMinSize;
    size_hints->min_width = width;
    size_hints->min_height = height;

    XSetWMNormalHints(
       cef_get_xdisplay(),
       handle,
       size_hints
    );
    XFree(size_hints);
  }

  void
  ConfigureAsDialog(CefWindowHandle handle) {
    ::XDisplay *display = cef_get_xdisplay();

    Atom type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom value = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    XChangeProperty(display, handle, type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&value), 1);
  }

  void
  ConfigureAsTopmost(CefWindowHandle handle) {
    ConfigureAsDialog(handle);

    ::XDisplay *display = cef_get_xdisplay();

    Atom state[2];
    state[0] = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", False);
    state[1] = XInternAtom(display, "_NET_WM_STATE_SKIP_PAGER", False);
    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    XChangeProperty (display, handle, wm_state, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&state), 2);
  }

  void
  SetGroupByLeader(CefWindowHandle handle) {
    ::XDisplay *display = cef_get_xdisplay();
    XWMHints base_hints;
    XWMHints * h = XGetWMHints(display, handle);
    if (!h) {
      h = &base_hints;
      h->flags = 0;
    }
    h->flags |= WindowGroupHint;
    h->window_group = leader_window_;
    XSetWMHints(display, handle, h);
    if(h != &base_hints) {
      XFree(h);
    }
  }

  void
  SetClientLeader(CefWindowHandle handle) {
    ::XDisplay *display = cef_get_xdisplay();

    Atom type = XInternAtom(display, "WM_CLIENT_LEADER", False);
    XChangeProperty(display, handle, type, XA_WINDOW, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&leader_window_), 1);
    SetGroupByLeader(handle);
  }

  void
  FixSize(CefWindowHandle handle, int width, int height) {
    XSizeHints *size_hints = XAllocSizeHints();
    ::XDisplay *display = cef_get_xdisplay();

    if (!size_hints) {
      LOG(ERROR) << "FixSize: Can't allocate memory for XAllocSizeHints";
      return;
    }

    if (width && height) {
      size_hints->flags = PSize | PMinSize | PMaxSize;
      size_hints->width = width;
      size_hints->height = height;
      size_hints->min_width = width;
      size_hints->min_height = height;
      size_hints->max_width = width;
      size_hints->max_height = height;
    } else {
      size_hints->flags = PMinSize | PMaxSize;
      size_hints->max_width = width;
      size_hints->max_height = height;
    }

    XSetWMNormalHints(
       display,
       handle,
       size_hints
    );
    XFree(size_hints);
  }

  void
  InitAsPopup(CefWindowHandle handle) {
    ConfigureAsDialog(handle);
  }

  void
  Hide(CefWindowHandle handle) {
    ::Display *display = cef_get_xdisplay();
    DCHECK(display);

    XUnmapWindow(display, handle);
  }

  void
  Show(CefWindowHandle handle) {
    ::Display *display = cef_get_xdisplay();
    DCHECK(display);

    XMapWindow(display, handle);
  }

  void
  CenterPosition(CefWindowHandle handle) {
    ::Display *display = cef_get_xdisplay();
    DCHECK(display);

    XSizeHints *size_hints = XAllocSizeHints();
    if (!size_hints) {
      LOG(ERROR) << "CenterPosition: Can't allocate memory for XAllocSizeHints";
      return;
    }

    size_hints->flags = PWinGravity;
    size_hints->win_gravity = CenterGravity;

    XSetWMNormalHints(
       display,
       handle,
       size_hints
    );
    XFree(size_hints);
  }

  void
  SetTitle(CefWindowHandle handle, std::string title) {
    std::string titleStr(title);

    // Retrieve the X11 display shared with Chromium.
    ::Display *display = cef_get_xdisplay();
    DCHECK(display);

    DCHECK(handle != kNullWindowHandle);

    // Retrieve the atoms required by the below XChangeProperty call.
    const char *kAtoms[] = {
       "_NET_WM_NAME",
       "UTF8_STRING"
    };
    Atom atoms[2];
    int result = XInternAtoms(display, const_cast<char **>(kAtoms), 2, false,
       atoms);
    if (!result)
      NOTREACHED();

    // Set the window title.
    XChangeProperty(display,
       handle,
       atoms[0],
       atoms[1],
       8,
       PropModeReplace,
       reinterpret_cast<const unsigned char *>(titleStr.c_str()),
       titleStr.size());

    // TODO(erg): This is technically wrong. So XStoreName and friends expect
    // this in Host Portable Character Encoding instead of UTF-8, which I believe
    // is Compound Text. This shouldn't matter 90% of the time since this is the
    // fallback to the UTF8 property above.
    XStoreName(display, handle, titleStr.c_str());
  }

  void
  SetClassHints(CefWindowHandle handle, char *res_name, char *res_class) {
    XClassHint *class_hints = XAllocClassHint();
    ::XDisplay *display = cef_get_xdisplay();

    if (!class_hints) {
      LOG(ERROR) << "SetClassHints: Can't allocate memory for XAllocClassHint";
      return;
    }

    class_hints->res_name = res_name;
    class_hints->res_class = res_class;

    XSetClassHint(display, handle, class_hints);
    XFree(class_hints);
  }

  void
  SetLeaderWindow(CefWindowHandle handle) {
    leader_window_ = handle;
  }

  CefWindowHandle
  GetLeaderWindow() {
    return leader_window_;
  }

  void
  InitHooks() {
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);
  }

  void
  InitWindow(CefWindowHandle handle, bool is_leader) {
    if (is_leader)
      SetLeaderWindow(handle);
    else
      SetClientLeader(handle);

    SetClassHints(handle, const_cast<char *>(APP_COMMON_NAME), const_cast<char *>(APP_NAME));
  }

  GList*
  GetDefaultIcons() {
    return default_icons;
  }

  void SetDefaultIcons(GList* icons) {
    if (default_icons) {
      g_list_foreach(default_icons, (GFunc) g_object_unref, NULL);
      g_list_free(default_icons);
    }

    default_icons = icons;
    gtk_window_set_default_icon_list(icons);
  }

  BrowserWindow*
  LookupBrowserWindow(CefWindowHandle native_window) {
    GdkWindow * gdk_window = GDK_WINDOW(gdk_xid_table_lookup(native_window));
    return reinterpret_cast<BrowserWindow*>(
        g_object_get_data(G_OBJECT(gdk_window), "wrapper")
    );
  }

  BrowserWindow*
  LookupBrowserWindow(GdkEvent* event) {
    if (event->type == GDK_NOTHING) {
      // GDK_NOTHING event doesn't have any windows
      return NULL;
    }

    GObject *object = G_OBJECT(event->any.window);
    if (!G_IS_OBJECT(object)) {
      LOG(ERROR) << "Try lookup browser window of bad native window"
         << ", event type: " << event->type
         << ", event window: " << event->any.window;
      return NULL;
    }

    return reinterpret_cast<BrowserWindow*>(
        g_object_get_data(object, "wrapper")
    );
  }

  void
  FlushChanges() {
    ::XDisplay *display = cef_get_xdisplay();

    XFlush(display);
  }

  CefRect
  GetDefaultScreenRect() {
    GdkRectangle monitor_rect;
    gdk_screen_get_monitor_geometry(gdk_screen_get_default(), 0, &monitor_rect);

    return CefRect(
       monitor_rect.x,
       monitor_rect.y,
       monitor_rect.width,
       monitor_rect.height
    );
  }

  double
  GetDPI() {
    GtkSettings* gtk_settings = gtk_settings_get_default();
    DCHECK(gtk_settings);
    gint gtk_dpi = -1;
    g_object_get(gtk_settings, "gtk-xft-dpi", &gtk_dpi, NULL);

    // GTK multiplies the DPI by 1024 before storing it.
    return (gtk_dpi > 0) ? gtk_dpi / 1024.0 : kCSSDefaultDPI;
  }

  double
  GetDeviceScaleFactor() {
    if (!device_scale_factor)
      device_scale_factor = floor((GetDPI() / kCSSDefaultDPI) * 100) / 100;

    return device_scale_factor;
  }
}  // namespace window_util
