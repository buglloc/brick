#include <error.h>
#include <include/base/cef_logging.h>
#include "about_window.h"
#include "gtk/gtk.h"
#include "../brick_app.h"

namespace {
    const char kGladeWindow[]  = ""
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       "<interface>\n"
       "  <!-- interface-requires gtk+ 3.0 -->\n"
       "  <!-- interface-naming-policy toplevel-contextual -->\n"
       "  <object class=\"GtkAboutDialog\" id=\"about_dialog\">\n"
       "    <property name=\"can_focus\">False</property>\n"
       "    <property name=\"type_hint\">normal</property>\n"
       "    <property name=\"program_name\">Brick</property>\n"
       "    <property name=\"comments\" translatable=\"yes\">Simple Bitrix messenger client\n"
       "    Version " APP_VERSION ""
       "    </property>\n"
       "    <property name=\"website\">https://github.com/buglloc/brick</property>\n"
       "    <property name=\"website_label\" translatable=\"yes\">https://github.com/buglloc/brick</property>\n"
       "    <property name=\"authors\">Buglloc &lt;buglloc@yandex.ru&gt;\n"
       "    </property>\n"
       "    <property name=\"logo_icon_name\">brick</property>\n"
       "    <signal name=\"close\" handler=\"close_event_handler\" swapped=\"no\"/>\n"
       "    <child internal-child=\"vbox\">\n"
       "      <object class=\"GtkVBox\" id=\"dialog-vbox1\">\n"
       "        <property name=\"can_focus\">False</property>\n"
       "        <child internal-child=\"action_area\">\n"
       "          <object class=\"GtkHButtonBox\" id=\"dialog-action_area1\">\n"
       "            <property name=\"can_focus\">False</property>\n"
       "          </object>\n"
       "          <packing>\n"
       "            <property name=\"expand\">True</property>\n"
       "            <property name=\"fill\">True</property>\n"
       "            <property name=\"position\">0</property>\n"
       "          </packing>\n"
       "        </child>\n"
       "      </object>\n"
       "    </child>\n"
       "  </object>\n"
       "</interface>";


    bool
    on_delete_event(GtkDialog *dialog, gpointer data, AboutWindow *self) {
       self->Hide();
       return true;
    }

    void
    on_response(GtkDialog *dialog, gint response_id, AboutWindow *self) {
       self->Hide();
    }

} // namespace


void
AboutWindow::Init() {
  GtkBuilder *builder = gtk_builder_new ();
  GError* error = NULL;
  if (!gtk_builder_add_from_string(builder, kGladeWindow, strlen(kGladeWindow), &error))
  {
    LOG(WARNING) << "Failed to build aboud window: " << error->message;
    g_error_free (error);
  }

  window_handler_ = GTK_WIDGET(gtk_builder_get_object(builder, "about_dialog"));
  LOG_IF(WARNING, !window_handler_)
      << "Failed to handle aboud window";

  g_signal_connect(G_OBJECT(window_handler_), "response", G_CALLBACK(on_response), this);
  g_signal_connect(G_OBJECT(window_handler_), "delete_event", G_CALLBACK(on_delete_event), this);
  g_object_unref(builder);
}

void
AboutWindow::Show() {
  gtk_widget_show_all(window_handler_);
}

void
AboutWindow::Hide() {
  gtk_widget_hide(window_handler_);
}