#include <include/base/cef_logging.h>
#include <brick/cef_handler.h>
#include <utmp.h>
#include "edit_account_window.h"

namespace {

    const char kGladeWindow[]  = ""
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       "<interface>\n"
       "  <requires lib=\"gtk+\" version=\"2.24\"/>\n"
       "  <!-- interface-naming-policy project-wide -->\n"
       "  <object class=\"GtkDialog\" id=\"edit_account_dialog\">\n"
       "    <property name=\"can_focus\">False</property>\n"
       "    <property name=\"border_width\">5</property>\n"
       "    <property name=\"title\" translatable=\"yes\">Add/Edit account</property>\n"
       "    <property name=\"resizable\">False</property>\n"
       "    <property name=\"window_position\">center</property>\n"
       "    <property name=\"type_hint\">dialog</property>\n"
       "    <property name=\"has_separator\">True</property>\n"
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
       "              <object class=\"GtkButton\" id=\"save_button\">\n"
       "                <property name=\"label\" translatable=\"yes\">Save</property>\n"
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
       "              <object class=\"GtkButton\" id=\"cancel_button\">\n"
       "                <property name=\"label\" translatable=\"yes\">Cancel</property>\n"
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
       "          </object>\n"
       "          <packing>\n"
       "            <property name=\"expand\">False</property>\n"
       "            <property name=\"fill\">False</property>\n"
       "            <property name=\"position\">0</property>\n"
       "          </packing>\n"
       "        </child>\n"
       "        <child>\n"
       "          <object class=\"GtkTable\" id=\"table1\">\n"
       "            <property name=\"visible\">True</property>\n"
       "            <property name=\"can_focus\">False</property>\n"
       "            <property name=\"n_rows\">3</property>\n"
       "            <property name=\"n_columns\">2</property>\n"
       "            <child>\n"
       "              <object class=\"GtkLabel\" id=\"domain_label\">\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">False</property>\n"
       "                <property name=\"label\" translatable=\"yes\">Domain</property>\n"
       "                <property name=\"track_visited_links\">False</property>\n"
       "              </object>\n"
       "            </child>\n"
       "            <child>\n"
       "              <object class=\"GtkLabel\" id=\"login_label\">\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">False</property>\n"
       "                <property name=\"label\" translatable=\"yes\">Login</property>\n"
       "                <property name=\"track_visited_links\">False</property>\n"
       "              </object>\n"
       "              <packing>\n"
       "                <property name=\"top_attach\">1</property>\n"
       "                <property name=\"bottom_attach\">2</property>\n"
       "              </packing>\n"
       "            </child>\n"
       "            <child>\n"
       "              <object class=\"GtkLabel\" id=\"password_label\">\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">False</property>\n"
       "                <property name=\"label\" translatable=\"yes\">Password</property>\n"
       "                <property name=\"track_visited_links\">False</property>\n"
       "              </object>\n"
       "              <packing>\n"
       "                <property name=\"top_attach\">2</property>\n"
       "                <property name=\"bottom_attach\">3</property>\n"
       "              </packing>\n"
       "            </child>\n"
       "            <child>\n"
       "              <object class=\"GtkEntry\" id=\"login_entry\">\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">True</property>\n"
       "                <property name=\"invisible_char\">●</property>\n"
       "                <property name=\"primary_icon_activatable\">False</property>\n"
       "                <property name=\"secondary_icon_activatable\">False</property>\n"
       "                <property name=\"primary_icon_sensitive\">True</property>\n"
       "                <property name=\"secondary_icon_sensitive\">True</property>\n"
       "              </object>\n"
       "              <packing>\n"
       "                <property name=\"left_attach\">1</property>\n"
       "                <property name=\"right_attach\">2</property>\n"
       "                <property name=\"top_attach\">1</property>\n"
       "                <property name=\"bottom_attach\">2</property>\n"
       "              </packing>\n"
       "            </child>\n"
       "            <child>\n"
       "              <object class=\"GtkEntry\" id=\"password_entry\">\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">True</property>\n"
       "                <property name=\"invisible_char\">●</property>\n"
       "                <property name=\"primary_icon_activatable\">False</property>\n"
       "                <property name=\"secondary_icon_activatable\">False</property>\n"
       "                <property name=\"primary_icon_sensitive\">True</property>\n"
       "                <property name=\"secondary_icon_sensitive\">True</property>\n"
       "              </object>\n"
       "              <packing>\n"
       "                <property name=\"left_attach\">1</property>\n"
       "                <property name=\"right_attach\">2</property>\n"
       "                <property name=\"top_attach\">2</property>\n"
       "                <property name=\"bottom_attach\">3</property>\n"
       "              </packing>\n"
       "            </child>\n"
       "            <child>\n"
       "              <object class=\"GtkEntry\" id=\"domain_entry\">\n"
       "                <property name=\"visible\">True</property>\n"
       "                <property name=\"can_focus\">True</property>\n"
       "                <property name=\"invisible_char\">●</property>\n"
       "                <property name=\"primary_icon_activatable\">False</property>\n"
       "                <property name=\"secondary_icon_activatable\">False</property>\n"
       "                <property name=\"primary_icon_sensitive\">True</property>\n"
       "                <property name=\"secondary_icon_sensitive\">True</property>\n"
       "              </object>\n"
       "              <packing>\n"
       "                <property name=\"left_attach\">1</property>\n"
       "                <property name=\"right_attach\">2</property>\n"
       "              </packing>\n"
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
       "      <action-widget response=\"0\">save_button</action-widget>\n"
       "      <action-widget response=\"0\">cancel_button</action-widget>\n"
       "    </action-widgets>\n"
       "  </object>\n"
       "</interface>";

    static void
    on_save_button(GtkWidget *widget, EditAccountWindow *self) {
      LOG(INFO) << "Save button";
      const gchar* domain =
        gtk_entry_get_text(self->window_objects_.domain_entry);
      const gchar* login =
         gtk_entry_get_text(self->window_objects_.login_entry);
      const gchar* password =
         gtk_entry_get_text(GTK_ENTRY(self->window_objects_.password_entry));

      self->Save(
         std::string(domain),
         std::string(login),
         std::string(password)
      );
    }

    static void
    on_cancel_button(GtkWidget *widget, EditAccountWindow *self) {
      self->Close();
    }

} // namespace


void
EditAccountWindow::Init(CefRefPtr<Account> account) {
  GtkBuilder *builder = gtk_builder_new ();
  GError* error = NULL;
  if (!gtk_builder_add_from_string(builder, kGladeWindow, strlen(kGladeWindow), &error))
  {
    LOG(WARNING) << "Failed to build aboud window: " << error->message;
    g_error_free (error);
  }

  window_objects_.account = account;
  window_objects_.window = GTK_WIDGET(gtk_builder_get_object(builder, "edit_account_dialog"));
  window_objects_.domain_entry = GTK_ENTRY(gtk_builder_get_object(builder, "domain_entry"));
  window_objects_.login_entry = GTK_ENTRY(gtk_builder_get_object(builder, "login_entry"));
  window_objects_.password_entry = GTK_ENTRY(gtk_builder_get_object(builder, "password_entry"));


  g_signal_connect(gtk_builder_get_object(builder, "save_button"), "clicked", G_CALLBACK(on_save_button), this);
  g_signal_connect(gtk_builder_get_object(builder, "cancel_button"), "clicked", G_CALLBACK(on_cancel_button), this);


  g_object_unref(builder);

  gtk_entry_set_text(
     window_objects_.domain_entry,
     account->GetDomain().c_str()
  );

  gtk_entry_set_text(
     window_objects_.login_entry,
     account->GetLogin().c_str()
  );

  gtk_entry_set_text(
     window_objects_.password_entry,
     account->GetPassword().c_str()
  );
}

void
EditAccountWindow::Show() {
  gtk_widget_show_all(window_objects_.window);
}

void
EditAccountWindow::Close() {
  gtk_widget_destroy(GTK_WIDGET(window_objects_.window));
}

void
EditAccountWindow::Hide() {
  gtk_widget_hide(window_objects_.window);
}