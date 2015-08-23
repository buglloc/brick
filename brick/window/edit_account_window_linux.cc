// Copyright (c) 2015 The Brick Authors.

#include <brick/auth_client.h>
#include "include/base/cef_logging.h"
#include "brick/client_handler.h"
#include "brick/window/edit_account_window.h"

extern char _binary_window_edit_account_glade_start;
extern char _binary_window_edit_account_glade_size;

namespace {

  // Forward declaration
  void OnSave(GtkWidget *widget, EditAccountWindow *self);
  void OnOtpDialogResponse(GtkDialog *dialog, gint response_id, EditAccountWindow *self);
  void ShowOtpDialog(EditAccountWindow *self);

  const char kOtpPromptId[] = "otp_prompt_text";
  const char kBitrix24DomainMark[] = ".bitrix24.";

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
  OnErrorDialogResponse(GtkDialog *dialog, gint response_id, EditAccountWindow *self) {
    self->OnSaveEnded();
    gtk_widget_destroy(GTK_WIDGET(dialog));
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
  ShowError(EditAccountWindow *self, const gchar *text) {
    GtkWidget *dialog = gtk_message_dialog_new_with_markup(
        GTK_WINDOW(self->window_objects_.window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_OTHER,
        GTK_BUTTONS_OK_CANCEL,
        NULL
    );

    gtk_window_set_title(GTK_WINDOW(dialog), "Authorization failed");
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), text);
    gtk_widget_show_all(GTK_WIDGET(dialog));
    g_signal_connect(dialog, "response", G_CALLBACK(OnErrorDialogResponse), self);
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
    bool renew = static_cast<bool>(
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->window_objects_.use_app_password)));

    GtkWidget* otp_widget = static_cast<GtkWidget*>(
       g_object_get_data(G_OBJECT(widget), kOtpPromptId));

    const gchar* otp = "";
    if (otp_widget) {
      otp = gtk_entry_get_text(GTK_ENTRY(otp_widget));

      if (!renew) {
        renew = true;
        // If we have OTP - turn on App Password
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(self->window_objects_.use_app_password),
            true
        );
      }
    }

    if (!secure && strstr(domain, kBitrix24DomainMark)) {
      // If we add Bitrix24 account always set secure protocol
      secure = true;
      gtk_combo_box_set_active(
          self->window_objects_.protocol_chooser,
          0
      );
    }

    if (!strlen(domain)) {
      ShowError(self, "Empty domain");
    } else if (!strlen(login)) {
      ShowError(self, "Empty login");
    } else if (!strlen(password)) {
      ShowError(self, "Empty password");
    } else {
      self->OnSaveStarted();
      self->window_objects_.auth_account = new Account;
      self->window_objects_.auth_account->Set(
          secure,
          domain,
          login,
          password,
          renew
      );

      self->window_objects_.auth_account->Auth(renew, base::Bind(&EditAccountWindow::OnAuthComplete, self), otp);
    }
  }

  void
  OnCancel(GtkWidget *widget, EditAccountWindow *self) {
    self->Close();
    self->CancelAuthPending();
  }

  bool
  OnDestroy(GtkWidget *widget, EditAccountWindow *self) {
    self->CancelAuthPending();
    self->Release();
    // Allow the close.
    return FALSE;
  }

}  // namespace


void
EditAccountWindow::Init(CefRefPtr<Account> account, bool switch_on_save) {
  GtkBuilder *builder = gtk_builder_new ();
  GError* error = NULL;

  if (!gtk_builder_add_from_string(builder, &_binary_window_edit_account_glade_start, (gsize)&_binary_window_edit_account_glade_size, &error)) {
    LOG(WARNING) << "Failed to build account edditting window: " << error->message;
    g_error_free (error);
  }

  AddRef();
  window_objects_.account = account;
  window_objects_.switch_on_save = switch_on_save;
  window_handler_ = GTK_WIDGET(gtk_builder_get_object(builder, "edit_account_dialog"));
  window_objects_.window = window_handler_;
  window_objects_.protocol_chooser = GTK_COMBO_BOX(gtk_builder_get_object(builder, "protocol_selector"));
  window_objects_.domain_entry = GTK_ENTRY(gtk_builder_get_object(builder, "domain_entry"));
  window_objects_.login_entry = GTK_ENTRY(gtk_builder_get_object(builder, "login_entry"));
  window_objects_.password_entry = GTK_ENTRY(gtk_builder_get_object(builder, "password_entry"));
  window_objects_.use_app_password = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "use_app_password"));
  window_objects_.save_button = GTK_BUTTON(gtk_builder_get_object(builder, "save_button"));
  window_objects_.cancel_button = GTK_BUTTON(gtk_builder_get_object(builder, "cancel_button"));

  g_signal_connect(GTK_OBJECT(window_objects_.window), "destroy", G_CALLBACK(OnDestroy), this);
  g_signal_connect(window_objects_.save_button, "clicked", G_CALLBACK(OnSave), this);
  g_signal_connect(window_objects_.cancel_button, "clicked", G_CALLBACK(OnCancel), this);


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

  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(window_objects_.use_app_password),
     account->IsAppPasswordUsed()
  );
}

void
EditAccountWindow::OnAuthComplete(const CefRefPtr<Account> account, const Account::AuthResult auth_result) {
  if (auth_result.success) {
    OnSaveEnded();
    Save(
        account->IsSecure(),
        account->GetDomain(),
        account->GetLogin(),
        account->GetPassword(),  // Server may update user password while login,
        account->IsAppPasswordUsed()
    );
  } else {
    std::string error_message;

    switch (auth_result.error_code) {
      case Account::ERROR_CODE::HTTP:
        error_message = auth_result.http_error;
        break;
      case Account::ERROR_CODE::CAPTCHA:
        error_message = "You’ve exceeded the maximum number of login attempts allowed.\n"
            "Please, authorize in your <b>browser</b> first";
        break;
      case Account::ERROR_CODE::OTP:
        ShowOtpDialog(this);
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

    ShowError(this, error_message.c_str());
  }
}

void
EditAccountWindow::CancelAuthPending() {
  if (!window_objects_.auth_account.get())
    return;

  window_objects_.auth_account->CancelAuthPending(false);
  window_objects_.auth_account = NULL;
}

void EditAccountWindow::OnSaveStarted() {
  gtk_widget_set_sensitive(GTK_WIDGET(window_objects_.save_button), false);
}

void EditAccountWindow::OnSaveEnded() {
  gtk_widget_set_sensitive(GTK_WIDGET(window_objects_.save_button), true);
}
