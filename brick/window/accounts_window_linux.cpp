#include <include/base/cef_logging.h>
#include <brick/cef_handler.h>
#include "accounts_window.h"
#include "edit_account_window.h"

namespace {

    const char kGladeWindow[]  = ""
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       "<interface>\n"
       "  <requires lib=\"gtk+\" version=\"2.24\"/>\n"
       "  <!-- interface-naming-policy project-wide -->\n"
       "  <object class=\"GtkListStore\" id=\"accounts_store\">\n"
       "    <columns>\n"
       "      <!-- column-name Reference -->\n"
       "      <column type=\"gint\"/>\n"
       "      <!-- column-name Account -->\n"
       "      <column type=\"gchararray\"/>\n"
       "    </columns>\n"
       "  </object>\n"
       "  <object class=\"GtkDialog\" id=\"accounts_dialog\">\n"
       "    <property name=\"can_focus\">False</property>\n"
       "    <property name=\"border_width\">5</property>\n"
       "    <property name=\"title\" translatable=\"yes\">Manage Accounts</property>\n"
       "    <property name=\"window_position\">center</property>\n"
       "    <property name=\"default_width\">382</property>\n"
       "    <property name=\"default_height\">250</property>\n"
       "    <property name=\"type_hint\">dialog</property>\n"
       "    <child internal-child=\"vbox\">\n"
       "      <object class=\"GtkVBox\" id=\"dialog-vbox1\">\n"
       "        <property name=\"visible\">True</property>\n"
       "        <property name=\"can_focus\">False</property>\n"
       "        <property name=\"spacing\">2</property>\n"
       "        <child internal-child=\"action_area\">\n"
       "          <object class=\"GtkHButtonBox\" id=\"dialog-action_area1\">\n"
       "            <property name=\"visible\">True</property>\n"
       "            <property name=\"can_focus\">False</property>\n"
       "            <property name=\"layout_style\">end</property>\n"
       "            <child>\n"
       "              <object class=\"GtkButton\" id=\"add_button\">\n"
       "                <property name=\"label\" translatable=\"yes\">Add</property>\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">True</property>\n"
       "                <property name=\"receives_default\">True</property>\n"
       "              </object>\n"
       "              <packing>\n"
       "                <property name=\"expand\">False</property>\n"
       "                <property name=\"fill\">False</property>\n"
       "                <property name=\"position\">0</property>\n"
       "              </packing>\n"
       "            </child>\n"
       "            <child>\n"
       "              <object class=\"GtkButton\" id=\"edit_button\">\n"
       "                <property name=\"label\" translatable=\"yes\">Edit</property>\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">True</property>\n"
       "                <property name=\"receives_default\">True</property>\n"
       "              </object>\n"
       "              <packing>\n"
       "                <property name=\"expand\">False</property>\n"
       "                <property name=\"fill\">False</property>\n"
       "                <property name=\"position\">1</property>\n"
       "              </packing>\n"
       "            </child>\n"
       "            <child>\n"
       "              <object class=\"GtkButton\" id=\"delete_button\">\n"
       "                <property name=\"label\" translatable=\"yes\">Delete</property>\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">True</property>\n"
       "                <property name=\"receives_default\">True</property>\n"
       "              </object>\n"
       "              <packing>\n"
       "                <property name=\"expand\">False</property>\n"
       "                <property name=\"fill\">False</property>\n"
       "                <property name=\"position\">2</property>\n"
       "              </packing>\n"
       "            </child>\n"
       "            <child>\n"
       "              <object class=\"GtkButton\" id=\"close_button\">\n"
       "                <property name=\"label\" translatable=\"yes\">Close</property>\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">True</property>\n"
       "                <property name=\"receives_default\">True</property>\n"
       "              </object>\n"
       "              <packing>\n"
       "                <property name=\"expand\">False</property>\n"
       "                <property name=\"fill\">False</property>\n"
       "                <property name=\"position\">3</property>\n"
       "              </packing>\n"
       "            </child>\n"
       "          </object>\n"
       "          <packing>\n"
       "            <property name=\"expand\">False</property>\n"
       "            <property name=\"fill\">False</property>\n"
       "            <property name=\"position\">0</property>\n"
       "          </packing>\n"
       "        </child>\n"
       "        <child>\n"
       "          <object class=\"GtkTreeView\" id=\"accounts_view\">\n"
       "            <property name=\"visible\">True</property>\n"
       "            <property name=\"can_focus\">True</property>\n"
       "            <property name=\"model\">accounts_store</property>\n"
       "            <property name=\"expander_column\">treeviewcolumn2</property>\n"
       "            <property name=\"search_column\">1</property>\n"
       "            <property name=\"level_indentation\">2</property>\n"
       "            <property name=\"enable_grid_lines\">both</property>\n"
       "            <property name=\"tooltip_column\">1</property>\n"
       "            <child>\n"
       "              <object class=\"GtkTreeViewColumn\" id=\"treeviewcolumn2\">\n"
       "                <property name=\"title\" translatable=\"yes\">Account</property>\n"
       "                <child>\n"
       "                  <object class=\"GtkCellRendererText\" id=\"cellrenderertext2\"/>\n"
       "                  <attributes>\n"
       "                    <attribute name=\"text\">1</attribute>\n"
       "                  </attributes>\n"
       "                </child>\n"
       "              </object>\n"
       "            </child>\n"
       "          </object>\n"
       "          <packing>\n"
       "            <property name=\"expand\">True</property>\n"
       "            <property name=\"fill\">True</property>\n"
       "            <property name=\"position\">1</property>\n"
       "          </packing>\n"
       "        </child>\n"
       "      </object>\n"
       "    </child>\n"
       "    <action-widgets>\n"
       "      <action-widget response=\"0\">add_button</action-widget>\n"
       "      <action-widget response=\"1\">edit_button</action-widget>\n"
       "      <action-widget response=\"2\">delete_button</action-widget>\n"
       "      <action-widget response=\"3\">close_button</action-widget>\n"
       "    </action-widgets>\n"
       "  </object>\n"
       "  <object class=\"GtkAction\" id=\"action1\"/>\n"
       "</interface>";


    static bool
    on_delete_event(GtkDialog *dialog, gpointer data, AccountsWindow *self) {
       self->Hide();
       return true;
    }

    static void
    on_add_button(GtkWidget *widget, AccountsWindow *self) {
      EditAccountWindow *window(new EditAccountWindow);
      window->Init(CefRefPtr<Account> (new Account));
      window->Show();
    }

    static void
    on_edit_button(GtkWidget *widget, AccountsWindow *self) {
      LOG(INFO) << "Not implemented: Edit button";
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
  if (!gtk_builder_add_from_string(builder, kGladeWindow, strlen(kGladeWindow), &error))
  {
    LOG(WARNING) << "Failed to build aboud window: " << error->message;
    g_error_free (error);
  }

  window_objects_.account_manager = ClientHandler::GetInstance()->GetAccountManager();
  window_objects_.window = GTK_WIDGET(gtk_builder_get_object(builder, "accounts_dialog"));
  LOG_IF(WARNING, !window_objects_.window)
      << "Failed to handle window";

  window_objects_.accounts_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "accounts_view"));
  LOG_IF(WARNING, !window_objects_.accounts_view)
     << "Failed to handle AccountsView";

  window_objects_.accounts_store = GTK_LIST_STORE(gtk_builder_get_object(builder, "accounts_store"));
  LOG_IF(WARNING, !window_objects_.accounts_store)
     << "Failed to handle AccountsStore";

  g_signal_connect(gtk_builder_get_object(builder, "accounts_dialog"), "delete_event", G_CALLBACK(on_delete_event), this);
  g_signal_connect(gtk_builder_get_object(builder, "add_button"), "clicked", G_CALLBACK(on_add_button), this);
  g_signal_connect(gtk_builder_get_object(builder, "edit_button"), "clicked", G_CALLBACK(on_edit_button), this);
  g_signal_connect(gtk_builder_get_object(builder, "delete_button"), "clicked", G_CALLBACK(on_delete_button), this);
  g_signal_connect(gtk_builder_get_object(builder, "close_button"), "clicked", G_CALLBACK(on_close_button), this);

  g_object_unref(builder);

  ReloadAccounts();
}

void
AccountsWindow::Show() {
  gtk_widget_show_all(window_objects_.window);
}

void
AccountsWindow::Hide() {
  gtk_widget_hide(window_objects_.window);
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