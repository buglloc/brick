#include <include/base/cef_logging.h>
#include <brick/cef_handler.h>
#include "edit_account_window.h"

extern char _binary_window_edit_account_glade_start;
extern char _binary_window_edit_account_glade_size;

namespace {
    // Forward declaration
    void OnSave(GtkWidget *widget, EditAccountWindow *self);
    void OnOtpDialogResponse(GtkDialog *dialog, gint response_id, EditAccountWindow *self);
    void ShowOtpDialog(EditAccountWindow *self);

    const char kOtpPromptId[] = "otp_prompt_text";

    void
    OnOtpDialogResponse(GtkDialog *dialog, gint response_id, EditAccountWindow *self) {
      switch (response_id) {
        case GTK_RESPONSE_OK:
          OnSave(GTK_WIDGET(dialog), self);
        case GTK_RESPONSE_CANCEL:
        case GTK_RESPONSE_DELETE_EVENT:
          gtk_widget_destroy(GTK_WIDGET(dialog));
          break;
        default:
          NOTREACHED();
      }
    }

    void
    ShowOtpDialog(EditAccountWindow *self) {
      GtkWidget *dialog = gtk_message_dialog_new_with_markup(
         GTK_WINDOW(self->window_objects_.window),
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_OTHER,
         GTK_BUTTONS_OK_CANCEL,
         NULL
      );

      gtk_window_set_title(GTK_WINDOW(dialog), "Two-step authentication");
      gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog),
         "Two-step authentication has been activated.\nPlease enter your one-time password:");

      GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
      GtkWidget* text_box = gtk_entry_new();
      gtk_box_pack_start(GTK_BOX(content_area), text_box, true, true, 0);
      g_object_set_data(G_OBJECT(dialog), kOtpPromptId, text_box);
      gtk_entry_set_activates_default(GTK_ENTRY(text_box), true);

      g_signal_connect(dialog, "response", G_CALLBACK(OnOtpDialogResponse), self);
      gtk_widget_show_all(GTK_WIDGET(dialog));
    }

    void
    OnSave(GtkWidget *widget, EditAccountWindow *self) {
      bool secure =
         gtk_combo_box_get_active(self->window_objects_.protocol_chooser) == 0;
      const gchar* domain =
        gtk_entry_get_text(self->window_objects_.domain_entry);
      const gchar* login =
         gtk_entry_get_text(self->window_objects_.login_entry);
      const gchar* password =
         gtk_entry_get_text(GTK_ENTRY(self->window_objects_.password_entry));

      GtkWidget* otp_widget = static_cast<GtkWidget*>(
         g_object_get_data(G_OBJECT(widget), kOtpPromptId));

      const gchar* otp = "";
      if (otp_widget) {
        otp = gtk_entry_get_text(GTK_ENTRY(otp_widget));
      }

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

        Account::AuthResult auth_result = check_account->Auth(true, otp);
        if (!auth_result.success) {
          show_error = true;

          switch (auth_result.error_code) {
            case Account::ERROR_CODE::HTTP:
              error_message = "HTTP error: " + auth_result.http_error;
              break;
            case Account::ERROR_CODE::CAPTCHA:
              error_message = "You’ve exceeded the maximum number of login attempts allowed.\n"
                 "Please, authorize in your <b>browser</b> first";
              break;
            case Account::ERROR_CODE::OTP:
              ShowOtpDialog(self);
              return;
              break;
            case Account::ERROR_CODE::AUTH:
              error_message = "Authorization error, wrong login or password";
              break;
            case Account::ERROR_CODE::INVALID_URL:
              error_message = auth_result.http_error + "\n"
                 "Please, provide correct host name and scheme";
              break;
            default:
              error_message = "I’m sorry, unknown error :(";
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

    void
    OnCancel(GtkWidget *widget, EditAccountWindow *self) {
      self->Close();
    }

} // namespace


void
EditAccountWindow::Init(CefRefPtr<Account> account, bool switch_on_save) {
  GtkBuilder *builder = gtk_builder_new ();
  GError* error = NULL;

  if (!gtk_builder_add_from_string(builder, &_binary_window_edit_account_glade_start, (gsize)&_binary_window_edit_account_glade_size, &error)) {
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

  g_signal_connect(gtk_builder_get_object(builder, "save_button"), "clicked", G_CALLBACK(OnSave), this);
  g_signal_connect(gtk_builder_get_object(builder, "cancel_button"), "clicked", G_CALLBACK(OnCancel), this);


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
