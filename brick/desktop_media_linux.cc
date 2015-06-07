// Copyright (c) 2015 The Brick Authors.

#include "brick/desktop_media.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <cairo/cairo.h>

#include <vector>

#include "include/internal/cef_linux.h"
#include "include/cef_task.h"
#include "include/base/cef_logging.h"
#include "third-party/codec/base64.h"

namespace {
  const char kWindowTypeName[] = "window";
  const char kScreenTypeName[] = "screen";
  const int shotWidth = 160;
  const int shotHeight = 160;

  cairo_status_t
  CairoWriteToString(void* user_data, const unsigned char *data, unsigned int length)
  {
    std::vector<unsigned char>* result = reinterpret_cast<std::vector<unsigned char>*>(user_data);
    size_t old_size = result->size();
    result->resize(old_size + length);
    memcpy(&(*result)[old_size], data, length);
    return CAIRO_STATUS_SUCCESS;
  }

  cairo_surface_t *
  CairoFitSurface(cairo_surface_t *surface, int width, int height) {
    cairo_surface_t *new_surface =
        cairo_surface_create_similar(surface,
                                     cairo_surface_get_content(surface),
                                     shotWidth,
                                     shotHeight);
    cairo_t *cr = cairo_create (new_surface);

    /* Scale *before* setting the source surface (1) */
    cairo_scale(cr, (double) shotWidth / width, (double) shotHeight / height);
    cairo_set_source_surface(cr, surface, 0, 0);

    /* To avoid getting the edge pixels blended with 0 alpha, which would
     * occur with the default EXTEND_NONE. Use EXTEND_PAD for 1.2 or newer (2)
      */
    cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REFLECT);

    /* Replace the destination with the source instead of overlaying */
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_destroy(cr);
    return new_surface;
  }
}  // namespace

namespace desktop_media {

  bool
  GetWindowPreview(::XDisplay* display, ::Window window, std::vector<unsigned char>* result) {
    int shot_x = 0, shot_y = 0;
    unsigned int shot_width = 0, shot_height = 0;
    Window root_ret = None;
    int x_ret = 0, y_ret = 0;
    unsigned int border_width_ret = 0, depth_ret = 0;
    XGetGeometry(display, window,
                 &root_ret,
                 &x_ret, &y_ret,
                 &shot_width, &shot_height,
                 &border_width_ret, &depth_ret);

    XImage* image = XGetImage(display, window,
                              shot_x, shot_y,
                              shot_width, shot_height,
                              AllPlanes, ZPixmap);

    if (!image) {
      LOG(WARNING) << "Can't get window image. Window id: " << window;
      return false;
    }

    if (image->depth != 24 && image->depth != 32) {
      LOG(WARNING) << "Unsupported image depth " << image->depth;
      return false;
    }

    cairo_surface_t* surface =
        cairo_image_surface_create_for_data(
            reinterpret_cast<unsigned char*>(image->data),
            image->depth == 24 ? CAIRO_FORMAT_RGB24 : CAIRO_FORMAT_ARGB32,
            image->width,
            image->height,
            image->bytes_per_line);

    CHECK(surface) << "Unable to create Cairo surface from XImage data";

    cairo_surface_t *new_surface = CairoFitSurface(surface, image->width, image->height);
    cairo_surface_write_to_png_stream(new_surface, CairoWriteToString, result);
    cairo_surface_destroy(new_surface);
    cairo_surface_destroy(surface);
    XDestroyImage(image);

    return true;
  }

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
    if (request != Success)
      return 0;

    if (exists) {
      state = *(long *) data;
      if (state == NormalState) {
        // Window has WM_STATE == NormalState. It's good window:)
        XFree(data);
        return window;
      } else if (state == IconicState) {
        // Window was minimized
        XFree(data);
        return 0;
      }
    }

    // If the window is in WithdrawnState then look at all of its children.
    ::Window root, parent;
    ::Window *children;
    unsigned int num_children;
    if (!XQueryTree(display, window, &root, &parent, &children,
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
    // ToDo: R&D, should i look at _NET_CLIENT_LIST?
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
        ::Window window = GetApplicationWindow(children[i]);
        std::string title;
        if (
            window
            && IsDesktopElement(window)
            && GetWindowTitle(window, &title)
            ) {
          std::string id = kWindowTypeName;
          id.append(":");
          id.append(std::to_string(window));
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
    std::string id = kScreenTypeName;
    id.append(":");
    id.append("0");
    media->SetString(0, id);
    media->SetString(1, "Entire screen");

    list->SetList(list->GetSize(), media);

    return true;
  }

  bool
  GetMediaPreview(std::string type, int32 id, std::vector<unsigned char>* out) {
    // ToDo: what are hell?!
    bool selfConnect = !CefCurrentlyOn(TID_UI);
    bool result = false;
    ::Display* display;
    if (selfConnect)
      display = XOpenDisplay(NULL);
    else
      display = cef_get_xdisplay();

    if (!display) {
      LOG(ERROR) << "Can't open display for creating desktop media preview";
      return result;
    }

    if (type == kWindowTypeName) {
      result = GetWindowPreview(display, id, out);
    } else if (type == kScreenTypeName) {
      result = GetWindowPreview(display, XDefaultRootWindow(display), out);
    }

    if (selfConnect)
      XCloseDisplay(display);

    return result;
  }
}  // namespace desktop_media
