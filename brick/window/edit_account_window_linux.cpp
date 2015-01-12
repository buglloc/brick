#include <include/base/cef_logging.h>
#include <brick/cef_handler.h>
#include <et/com_err.h>
#include "edit_account_window.h"

extern char _binary_window_edit_account_glade_start;
extern char _binary_window_edit_account_glade_size;

namespace {

    static void
    on_save_button(GtkWidget *widget, EditAccountWindow *self) {
      bool secure =
         gtk_combo_box_get_active(self->window_objects_.protocol_chooser) == 0;
      const gchar* domain =
        gtk_entry_get_text(self->window_objects_.domain_entry);
      const gchar* login =
         gtk_entry_get_text(self->window_objects_.login_entry);
      const gchar* password =
         gtk_entry_get_text(GTK_ENTRY(self->window_objects_.password_entry));

      CefRefPtr<Account> check_account(new Account);
      bool show_error = false;
      std::string error_message;

      if (!strlen(domain)) {
        show_error = true;
        error_message = "Empty domain";
      } else if (!strlen(login)) {
        show_error = true;
        error_message = "Empty login";
      } else if (!strlen(password)) {
        show_error = true;
        error_message = "Empty password";
      }

      if (!show_error) {
        check_account->Set(
           secure,
           domain,
           login,
           password
        );

        Account::AuthResult auth_result = check_account->Auth();
        if (!auth_result.success) {
          show_error = true;

          switch (auth_result.error_code) {
            case Account::ERROR_CODE::HTTP:
              error_message = "HTTP error: " + auth_result.http_error;
              break;
            case Account::ERROR_CODE::CAPTCHA:
              error_message = "You have exceeded the maximum number of login attempts.\n"
                 "Please log in <b>browser</b> first";
              break;
            case Account::ERROR_CODE::OTP:
              error_message = "Account used two-step authentication.\n"
                 "Please use <b>Application Password</b> for authorization";
              break;
            case Account::ERROR_CODE::AUTH:
              error_message = "Authentication failed.";
              break;
            default:
              error_message = "An unknown error occurred :(";
              break;
          }
        }
      }

      if (show_error) {
        GtkWidget *dialog = gtk_message_dialog_new_with_markup(
           GTK_WINDOW(self->window_objects_.window),
           GTK_DIALOG_DESTROY_WITH_PARENT,
           GTK_MESSAGE_ERROR,
           GTK_BUTTONS_CLOSE,
           NULL
        );

        gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog),
           error_message.c_str());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
      }

      self->Save(
         secure,
         std::string(domain),
         std::string(login),
         check_account->GetPassword() // Server may update user password while login
      );
    }

    static void
    on_cancel_button(GtkWidget *widget, EditAccountWindow *self) {
      self->Close();
    }

} // namespace


void
EditAccountWindow::Init(CefRefPtr<Account> account, bool switch_on_save) {
  GtkBuilder *builder = gtk_builder_new ();
  GError* error = NULL;

  if (!gtk_builder_add_from_string(builder, &_binary_window_edit_account_glade_start, (gsize)&_binary_window_edit_account_glade_size, &error))
  {
    LOG(WARNING) << "Failed to build account edditting window: " << error->message;
    g_error_free (error);
  }

  window_objects_.account = account;
  window_objects_.switch_on_save = switch_on_save;
  window_handler_ = GTK_WIDGET(gtk_builder_get_object(builder, "edit_account_dialog"));
  window_objects_.window = window_handler_;
  window_objects_.protocol_chooser = GTK_COMBO_BOX(gtk_builder_get_object(builder, "protocol_selector"));
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

  gtk_combo_box_set_active(
     window_objects_.protocol_chooser,
     account->IsSecure()? 0: 1
  );
}
