// Copyright (c) 2015 The Brick Authors.

#include "brick/client_handler.h"

#include <gtk/gtk.h>

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"
#include "brick/window_util.h"
#include "brick/brick_app.h"

namespace {
  const int kPreviewWidth = 256;
  const int kPreviewHeight = 512;

  void
  OnFileDialogResponse(GtkDialog *dialog, gint response_id, ClientHandler *client_handler) {
    std::vector<CefString> files;
    bool success = false;
    gint mode = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog), "mode"));
    gint selected_accept_filter = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog), "selected_accept_filter"));
    CefFileDialogCallback *callback = reinterpret_cast<CefFileDialogCallback *>(
        g_object_get_data(G_OBJECT(dialog), "callback")
    );

    if (response_id == GTK_RESPONSE_ACCEPT) {
      if (mode == FILE_DIALOG_OPEN || mode == FILE_DIALOG_SAVE) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        files.push_back(std::string(filename));
        success = true;
      } else if (mode == FILE_DIALOG_OPEN_MULTIPLE) {
        GSList* filenames =
           gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
        if (filenames) {
          for (GSList* iter = filenames; iter != NULL;
               iter = g_slist_next(iter)) {
            std::string path(static_cast<char*>(iter->data));
            g_free(iter->data);
            files.push_back(path);
          }
          g_slist_free(filenames);
          success = true;
        }
      }
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));

    if (success)
      callback->Continue(selected_accept_filter, files);
    else
      callback->Cancel();
  }

  void
  OnUpdatePreviewPanel(GtkWidget *chooser, GtkWidget *preview) {
    gchar* filename = gtk_file_chooser_get_preview_filename(GTK_FILE_CHOOSER(chooser));
    if (!filename)
      return;

    // This will preserve the image's aspect ratio.
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_size(filename, kPreviewWidth, kPreviewHeight, NULL);
    g_free(filename);

    if (pixbuf) {
      gtk_image_set_from_pixbuf(GTK_IMAGE(preview), pixbuf);
      g_object_unref(pixbuf);
    }

    gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(chooser),
       pixbuf != NULL);
  }

  gboolean
  DoShutdown(gpointer data) {
    ClientHandler *self = reinterpret_cast<ClientHandler*>(data);
    if (self->InShutdownState()) {
      self->CloseAllBrowsers(true);
    }

    // Run only once
    return true;
  }
}  // namespace

void
ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  BrowserWindow *window = window_util::LookupBrowserWindow(
     browser->GetHost()->GetWindowHandle()
  );
  std::string window_title = APP_NAME;
  window_title.append(": ");

  if (browser->IsPopup()) {
    window_title.append(title);
  } else {
    window_title.append(account_manager_->GetCurrentAccount()->GetLabel());
  }

  window->SetTitle(window_title);
}

bool
ClientHandler::OnFileDialog(
    CefRefPtr<CefBrowser> browser,
    FileDialogMode mode_type,
    const CefString& title,
    const CefString& default_file_path,
    const std::vector<CefString>& accept_filters,
    int selected_accept_filter,
    CefRefPtr<CefFileDialogCallback> callback) {

  // Remove any modifier flags.
  FileDialogMode mode =
     static_cast<FileDialogMode>(mode_type & FILE_DIALOG_TYPE_MASK);

  GtkFileChooserAction action;
  const gchar* accept_button;
  if (mode == FILE_DIALOG_OPEN || mode == FILE_DIALOG_OPEN_MULTIPLE) {
    action = GTK_FILE_CHOOSER_ACTION_OPEN;
    accept_button = GTK_STOCK_OPEN;
  } else if (mode == FILE_DIALOG_SAVE) {
    action = GTK_FILE_CHOOSER_ACTION_SAVE;
    accept_button = GTK_STOCK_SAVE;
  } else {
    NOTREACHED();
    return false;
  }

  std::string title_str;
  if (!title.empty()) {
    title_str = title;
  } else {
    switch (mode) {
      case FILE_DIALOG_OPEN:
        title_str = "Open File";
        break;
      case FILE_DIALOG_OPEN_MULTIPLE:
        title_str = "Open Files";
        break;
      case FILE_DIALOG_SAVE:
        title_str = "Save File";
        break;
      default:
        break;
    }
  }

  GtkWidget* dialog = gtk_file_chooser_dialog_new(
     title_str.c_str(),
     NULL,
     action,
     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     accept_button, GTK_RESPONSE_ACCEPT,
     NULL);

  if (mode == FILE_DIALOG_OPEN_MULTIPLE || mode == FILE_DIALOG_OPEN) {
    GtkWidget *thumbnail = gtk_image_new();
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), thumbnail);
    g_signal_connect (dialog, "update-preview", G_CALLBACK(OnUpdatePreviewPanel), thumbnail);
  }

  if (mode == FILE_DIALOG_OPEN_MULTIPLE) {
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), true);
  } else if (mode == FILE_DIALOG_SAVE) {
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
       true);
  }

  if (!default_file_path.empty()) {
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
       default_file_path.ToString().c_str());
  }

  callback->AddRef();
  g_object_set_data(G_OBJECT(dialog), "mode", GINT_TO_POINTER(mode));
  g_object_set_data(G_OBJECT(dialog), "callback", callback.get());
  g_object_set_data(G_OBJECT(dialog), "selected_accept_filter", GINT_TO_POINTER(selected_accept_filter));
  g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnFileDialogResponse), this);
  gtk_widget_show(dialog);

  return true;
}

void
ClientHandler::Shutdown(bool force) {
  if (force) {
    CloseAllBrowsers(true);
  } else if (!InShutdownState()) {
    in_shutdown_state_ = true;
    shutdown_timer_id_ = gtk_timeout_add(BRICK_SHUTDOWN_TIMEOUT, DoShutdown, this);
    // Send event to JS APP
    SendJSEvent(GetBrowser(), "BXExitApplication");
  }
}


void
ClientHandler::PreventShutdown() {
  if (!InShutdownState())
    return;

  in_shutdown_state_ = false;
  gtk_timeout_remove(shutdown_timer_id_);
}
