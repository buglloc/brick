// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef_handler.h"

#include "include/wrapper/cef_helpers.h"
#include "window_util.h"

void
ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
    CEF_REQUIRE_UI_THREAD();

    if (!browser->IsPopup())
      return;

    window_util::SetTitle(browser->GetHost()->GetWindowHandle(), title);
}

bool
ClientHandler::OnFileDialog(CefRefPtr<CefBrowser> browser,
   FileDialogMode mode,
   const CefString& title,
   const CefString& default_file_name,
   const std::vector<CefString>& accept_types,
   CefRefPtr<CefFileDialogCallback> callback) {
  std::vector<CefString> files;

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

  std::string base_name;
  if (!default_file_name.empty()) {
    base_name =
       basename(const_cast<char*>(default_file_name.ToString().c_str()));
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
    }
  }

  GtkWidget* window = gtk_widget_get_ancestor(
     GTK_WIDGET(GetMainWindowHandle()->GetHandler()), GTK_TYPE_WINDOW);
  GtkWidget* dialog = gtk_file_chooser_dialog_new(
     title_str.c_str(),
     GTK_WINDOW(window),
     action,
     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     accept_button, GTK_RESPONSE_ACCEPT,
     NULL);

  if (mode == FILE_DIALOG_OPEN_MULTIPLE) {
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
  } else if (mode == FILE_DIALOG_SAVE) {
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
       TRUE);
  }

  if (mode == FILE_DIALOG_SAVE && !base_name.empty()) {
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
       base_name.c_str());
  }

  bool success = false;

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
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

  gtk_widget_destroy(dialog);

  if (success)
    callback->Continue(files);
  else
    callback->Cancel();

  return true;
}