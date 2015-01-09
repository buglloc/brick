#include <include/base/cef_logging.h>
#include <brick/cef_handler.h>
#include "accounts_window.h"
#include "edit_account_window.h"

extern char _binary_window_accounts_glade_start;
extern char _binary_window_accounts_glade_size;

namespace {

    static bool
    on_delete_event(GtkDialog *dialog, gpointer data, AccountsWindow *self) {
       self->Hide();
       return true;
    }

    static void
    on_edit_finished(GtkWidget *widget, AccountsWindow *self) {
      self->ReloadAccounts();
    }

    static void
    on_add_button(GtkWidget *widget, AccountsWindow *self) {
      self->window_objects_.edit_account_window->Init(CefRefPtr<Account> (new Account));
      g_signal_connect(GTK_OBJECT(self->window_objects_.edit_account_window->window_objects_.window), "destroy", G_CALLBACK(on_edit_finished), self);
      self->window_objects_.edit_account_window->Show();
    }

    static void
    on_edit_button(GtkWidget *widget, AccountsWindow *self) {
      GtkTreeSelection *selection;
      GtkTreeModel *model;
      GtkTreeIter  iter;

      selection = gtk_tree_view_get_selection(self->window_objects_.accounts_view);
      model = gtk_tree_view_get_model(self->window_objects_.accounts_view);

      LOG_IF(WARNING, !gtk_tree_model_get_iter_first(model, &iter))
         << "Can't get first list iter";

      if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint ref_id;
        gtk_tree_model_get(model, &iter,
           AccountsWindow::REF_ID, &ref_id,
           -1
        );

        self->window_objects_.edit_account_window->Init(
           self->window_objects_.account_manager->GetById((int) ref_id)
        );
        self->window_objects_.edit_account_window->Show();
        g_signal_connect(GTK_OBJECT(self->window_objects_.edit_account_window->window_objects_.window), "destroy", G_CALLBACK(on_edit_finished), self);

      } else {
        LOG(WARNING)
           << "Can't find selected row";
      }
    }

    static void
    on_delete_button(GtkWidget *widget, AccountsWindow *self) {
      GtkTreeSelection *selection;
      GtkTreeModel *model;
      GtkTreeIter  iter;

      selection = gtk_tree_view_get_selection(self->window_objects_.accounts_view);
      model = gtk_tree_view_get_model(self->window_objects_.accounts_view);

      LOG_IF(WARNING, !gtk_tree_model_get_iter_first(model, &iter))
         << "Can't get first list iter";

      if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint ref_id;
        gtk_tree_model_get(model, &iter,
           AccountsWindow::REF_ID, &ref_id,
           -1
        );
        self->window_objects_.account_manager->DeleteAccount((int) ref_id);
        self->window_objects_.account_manager->Commit();

        gtk_list_store_remove(self->window_objects_.accounts_store, &iter);

      } else {
        LOG(WARNING)
           << "Can't remove row";
      }
    }

    static void
    on_close_button(GtkWidget *widget, AccountsWindow *self) {
      self->Hide();
    }

} // namespace


void
AccountsWindow::Init() {
  GtkBuilder *builder = gtk_builder_new ();
  GError* error = NULL;

  if (!gtk_builder_add_from_string(builder, &_binary_window_accounts_glade_start, (gsize)&_binary_window_accounts_glade_size, &error))
  {
    LOG(WARNING) << "Failed to build aboud window: " << error->message;
    g_error_free (error);
  }

  window_objects_.account_manager = ClientHandler::GetInstance()->GetAccountManager();
  window_objects_.edit_account_window = new EditAccountWindow();
  window_handler_ = GTK_WIDGET(gtk_builder_get_object(builder, "accounts_dialog"));
  window_objects_.window = window_handler_;
  window_objects_.accounts_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "accounts_view"));
  window_objects_.accounts_store = GTK_LIST_STORE(gtk_builder_get_object(builder, "accounts_store"));

  g_signal_connect(gtk_builder_get_object(builder, "accounts_dialog"), "delete_event", G_CALLBACK(on_delete_event), this);
  g_signal_connect(gtk_builder_get_object(builder, "add_button"), "clicked", G_CALLBACK(on_add_button), this);
  g_signal_connect(gtk_builder_get_object(builder, "edit_button"), "clicked", G_CALLBACK(on_edit_button), this);
  g_signal_connect(gtk_builder_get_object(builder, "delete_button"), "clicked", G_CALLBACK(on_delete_button), this);
  g_signal_connect(gtk_builder_get_object(builder, "close_button"), "clicked", G_CALLBACK(on_close_button), this);

  g_object_unref(builder);

  ReloadAccounts();
}

void
AccountsWindow::AddToList(int id, std::string label) {
  GtkListStore *store;
  GtkTreeIter iter;

  store = window_objects_.accounts_store;
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter,
     REF_ID, id,
     LABEL, label.c_str(),
     -1
  );
}

void
AccountsWindow::Clear() {
  gtk_list_store_clear(window_objects_.accounts_store);
}