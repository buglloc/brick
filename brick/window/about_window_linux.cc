// Copyright (c) 2015 The Brick Authors.

#include "brick/window/about_window.h"

#include <gtk/gtk.h>
#include <error.h>

#include "include/base/cef_logging.h"
#include "brick/brick_app.h"

extern char _binary_window_about_glade_start;
extern char _binary_window_about_glade_size;

namespace {
  const char kComment[]  = "Simple Bitrix messenger client\nVersion " APP_VERSION;

  bool
  on_delete_event(GtkDialog *dialog, gpointer data, AboutWindow *self) {
     self->Hide();
     return true;
  }

  void
  on_response(GtkDialog *dialog, gint response_id, AboutWindow *self) {
     self->Hide();
  }

}  // namespace

void
AboutWindow::Init() {
  GtkBuilder *builder = gtk_builder_new ();
  GError* error = NULL;

  if (!gtk_builder_add_from_string(builder, &_binary_window_about_glade_start, (gsize)&_binary_window_about_glade_size, &error)) {
    LOG(WARNING) << "Failed to build aboud window: " << error->message;
    g_error_free (error);
  }

  window_handler_ = GTK_WIDGET(gtk_builder_get_object(builder, "about_dialog"));

  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(window_handler_), kComment);
  g_signal_connect(G_OBJECT(window_handler_), "response", G_CALLBACK(on_response), this);
  g_signal_connect(G_OBJECT(window_handler_), "delete_event", G_CALLBACK(on_delete_event), this);
  g_object_unref(builder);
}
