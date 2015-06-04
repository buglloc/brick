// Copyright (c) 2015 The Brick Authors.

#include "brick/desktop_media.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "include/internal/cef_linux.h"
#include "include/base/cef_logging.h"


namespace desktop_media {

  ::Window
  GetApplicationWindow(::Window window) {
    ::XDisplay *display = cef_get_xdisplay();
    Atom wm_state = XInternAtom(display, "WM_STATE", False);
    Atom type;
    int format;
    unsigned long exists, bytes_left;
    unsigned char *data;
    long state;

    long request = XGetWindowProperty(display, window, wm_state, 0L, 2L, False,
                                    wm_state, &type, &format, &exists,
                                    &bytes_left, &data);
    if (request != Success || !exists)
      return 0;

    state = *(long *) data;
    if (state == NormalState) {
      // Window has WM_STATE == NormalState. It's good window:)
      return window;
    } else if (state == IconicState) {
      // Window was minimized
      return 0;
    }

    // If the window is in WithdrawnState then look at all of its children.
    ::Window root, parent;
    ::Window *children;
    unsigned int num_children;
    if (!XQueryTree(cef_get_xdisplay(), window, &root, &parent, &children,
                    &num_children)) {
      LOG(ERROR) << "Failed to query for child windows although window"
                 << "does not have a valid WM_STATE.";
      return 0;
    }

    ::Window app_window = 0;
    for (unsigned int i = 0; i < num_children; ++i) {
      app_window = GetApplicationWindow(children[i]);
      if (app_window)
        break;
    }
    if (children)
      XFree(children);

    return app_window;
  }

  bool
  IsDesktopElement(::Window window) {
    ::XDisplay *display = cef_get_xdisplay();
    Atom wm_state = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom normal_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    Atom type;
    int format;
    unsigned long exists, bytes_left;
    unsigned char *data;
    bool result = true;

    long request = XGetWindowProperty(display, window, wm_state, 0L, sizeof(Atom), False,
                                      XA_ATOM, &type, &format, &exists,
                                      &bytes_left, &data);

    if (request != Success || !exists)
      return result;

    result = (*(Atom *) data) == normal_window_type;
    XFree(data);

    return result;
  }

  bool
  GetWindowTitle(::Window window, std::string* title) {
    int status;

    bool result = false;
    XTextProperty window_name;
    window_name.value = NULL;
    if (!window)
      return result;

    status = XGetWMName(cef_get_xdisplay(), window, &window_name);
    if (status && window_name.value && window_name.nitems) {
      int cnt;
      char **list = NULL;
      status = Xutf8TextPropertyToTextList(cef_get_xdisplay(), &window_name, &list,
                                           &cnt);
      if (status >= Success && cnt && *list) {
        if (cnt > 1) {
          LOG(INFO) << "Window has " << cnt
                       << " text properties, only using the first one.";
        }
        *title = *list;
        result = true;
      }
      if (list)
        XFreeStringList(list);
    }
    if (window_name.value)
      XFree(window_name.value);
    return result;
  }

  bool
  EnumerateWindows(CefListValue* list) {
    int last_index = list->GetSize();
    int num_screens = XScreenCount(cef_get_xdisplay());
    for (int screen = 0; screen < num_screens; ++screen) {
      ::Window root_window = XRootWindow(cef_get_xdisplay(), screen);
      ::Window parent;
      ::Window *children;
      unsigned int num_children;
      int status = XQueryTree(cef_get_xdisplay(), root_window, &root_window, &parent,
                              &children, &num_children);
      if (status == 0) {
        LOG(ERROR) << "Failed to query for child windows for screen "
                      << screen;
        continue;
      }

      for (unsigned int i = 0; i < num_children; ++i) {
        // Iterate in reverse order to return windows from front to back.
        ::Window window = GetApplicationWindow(children[num_children - 1 - i]);
        std::string title;
        if (
            window
            && IsDesktopElement(window)
            && GetWindowTitle(window, &title)
            ) {
          std::string id = "window:" + std::to_string(window);
          CefRefPtr<CefListValue> media = CefListValue::Create();
          media->SetString(0, id);
          media->SetString(1, title);
          list->SetList(last_index++, media);
        }
      }

      if (children)
        XFree(children);
    }
    return true;
  }

  bool
  EnumerateScreens(CefListValue* list) {
    // ToDo: implement when WebRTC starts support screen enumeration
    CefRefPtr<CefListValue> media = CefListValue::Create();
    media->SetString(0, "screen:0");
    media->SetString(1, "Entire screen");
    list->SetList(list->GetSize(), media);

    return true;
  }
}  // namespace desktop_media
