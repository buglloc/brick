#include <gio/gio.h>
#include <include/base/cef_logging.h>
#include <include/wrapper/cef_helpers.h>

#include "../event/event_bus.h"
#include "dbus_protocol.h"
#include "app_message_delegate.h"
#include "app_window_message_delegate.h"

namespace {

    const char kOwnName[] = "org.brick.Brick";
    const char kAppInterface[] = "org.brick.Brick.AppInterface";
    const char kAppPath[] = "/org/brick/Brick/App";
    const char kAppWindowInterface[] = "org.brick.Brick.AppWindowInterface";
    const char kAppWindowPath[] = "/org/brick/Brick/AppWindow";

    const gchar introspection_xml[] =
       "<node>"
          "  <interface name='org.brick.Brick.AppInterface'>"
          "    <method name='UserAway' />"
          "    <method name='UserPresent' />"
          "    <method name='ShowAddAccountDialog'>"
          "       <arg type='b' name='switch_on_save' direction='in'/>"
          "    </method>"
          "    <method name='ShowAccountsDialog' />"
          "    <method name='Quit' />"
          "    <signal name='IndicatorTooltipChanged'>"
          "      <arg type='s' name='text'/>"
          "    </signal>"
          "    <signal name='IndicatorStateChanged'>"
          "      <arg type='s' name='state'/>"
          "    </signal>"
          "    <signal name='IndicatorBadgeChanged'>"
          "      <arg type='i' name='badge'/>"
          "      <arg type='b' name='important'/>"
          "    </signal>"
          "  </interface>"
          "  <interface name='org.brick.Brick.AppWindowInterface'>"
          "    <method name='Hide' />"
          "    <method name='Present' />"
          "    <method name='ToggleVisibility' />"
          "  </interface>"
          "</node>";

    void
    handle_method_call(GDBusConnection *conn,
       const gchar *sender,
       const gchar *object_path,
       const gchar *interface_name,
       const gchar *method_name,
       GVariant *parameters,
       GDBusMethodInvocation *invocation,
       gpointer user_data) {

      LOG(INFO) << "Bus method " << interface_name << "::" << method_name << " called";

      DBusProtocol *self = (DBusProtocol *) user_data;
      CefRefPtr<CefProcessMessage> message =
         CefProcessMessage::Create(method_name);
      CefRefPtr<CefListValue> message_args = message->GetArgumentList();
      // ToDo: callback needed?
      message_args->SetNull(0);

      if (!g_strcmp0(method_name, "ShowAddAccountDialog")) {
        bool switch_on_save;
        g_variant_get(parameters, "(b)", &switch_on_save);
        message_args->SetBool(1, switch_on_save);
      }

      self->Handle(
         std::string(interface_name),
         message
      );

      g_dbus_method_invocation_return_value(invocation, g_variant_new("()"));
    }

    GVariant *
    handle_get_property(GDBusConnection *conn,
       const gchar *sender,
       const gchar *object_path,
       const gchar *interface_name,
       const gchar *property_name,
       GError **error,
       gpointer user_data) {

      return NULL;
    }

    gboolean
    handle_set_property(GDBusConnection *conn,
       const gchar *sender,
       const gchar *object_path,
       const gchar *interface_name,
       const gchar *property_name,
       GVariant *value,
       GError **error,
       gpointer user_data) {

      return FALSE;
    }

    const GDBusInterfaceVTable interface_vtable = {
       &handle_method_call,
       &handle_get_property,
       &handle_set_property
    };

    void
    on_bus_acquired(GDBusConnection *connection, const gchar *name, DBusProtocol *self) {
      LOG(INFO) << "Acquired the bus " << name;

      GDBusNodeInfo *introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);
      g_assert(introspection_data);

      GDBusInterfaceInfo *interface_app = g_dbus_node_info_lookup_interface(introspection_data, kAppInterface);
      g_assert(interface_app);
      g_dbus_connection_register_object(
         connection,
         kAppPath,
         interface_app,
         &interface_vtable,
         self,
         NULL,
         NULL
      );

      GDBusInterfaceInfo *interface_app_window = g_dbus_node_info_lookup_interface(introspection_data, kAppWindowInterface);
      g_assert(interface_app_window);
      g_dbus_connection_register_object(
         connection,
         kAppWindowPath,
         interface_app_window,
         &interface_vtable,
         self,
         NULL,
         NULL
      );
    }

} // namespace

DBusProtocol::~DBusProtocol() {

}

DBusProtocol::DBusProtocol() {

}

char
DBusProtocol::Init(bool send_show_on_exists) {
  // Simulate g_bus_own_name in sync way...

  GError *error = NULL;
  connection_ = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  if (!connection_) {
    LOG(ERROR) << "Can't register D-BUS: " << error->message;
    g_error_free(error);
    return -1;
  }

  GVariant *result = g_dbus_connection_call_sync (connection_,
     "org.freedesktop.DBus",
     "/org/freedesktop/DBus",
     "org.freedesktop.DBus",
     "RequestName",
     g_variant_new ("(su)",
        kOwnName,
        G_BUS_NAME_OWNER_FLAGS_NONE),
     G_VARIANT_TYPE ("(u)"),
     G_DBUS_CALL_FLAGS_NONE,
     -1, NULL, &error);

  if (!result) {
    LOG(ERROR) << "Can't register D-BUS: " << error->message;
    g_error_free(error);
    return -1;
  }

  guint32 request_name_reply;
  g_variant_get(result, "(u)", &request_name_reply);
  g_variant_unref(result);

  if (request_name_reply != 1 && request_name_reply != 4) {
    // If name already exists: not DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER and DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER
    if (send_show_on_exists) {
      // Send call Present method
      result = g_dbus_connection_call_sync (connection_,
         "org.brick.Brick",
         "/org/brick/Brick/AppWindow",
         "org.brick.Brick.AppWindowInterface",
         "Present",
         g_variant_new ("()"),
         G_VARIANT_TYPE ("()"),
         G_DBUS_CALL_FLAGS_NONE,
         -1, NULL, &error);

      if (!result) {
        LOG(ERROR) << "Can't present window via D-BUS: " << error->message;
        g_error_free(error);
      }
    }

    return 1;
  }

  on_bus_acquired(connection_, kOwnName, this);
  RegisterMessageDelegates();
  RegisterEventListeners();
  return 0;
}

bool
DBusProtocol::Handle(std::string interface_name, CefRefPtr<CefProcessMessage> message) {
  bool handled = false;

  // Execute delegate callbacks.
  ExternalMessageDelegateSet::iterator it = external_message_delegates_.begin();
  for (; it != external_message_delegates_.end() && !handled; ++it) {
    if ((*it)->IsAcceptedNamespace(interface_name)) {
      handled = (*it)->OnMessageReceived(
         message
      );
    }
  }

  return handled;
}

void
DBusProtocol::RegisterMessageDelegates() {
  external_message_delegates_.insert(new ExternalAppMessageDelegate);
  external_message_delegates_.insert(new ExternalAppWindowMessageDelegate);
}

void
DBusProtocol::RegisterEventListeners() {
  EventBus::AddHandler<AccountListEvent>(*this);
  EventBus::AddHandler<AccountSwitchEvent>(*this);
  EventBus::AddHandler<IndicatorBadgeEvent>(*this);
  EventBus::AddHandler<IndicatorStateEvent>(*this);
  EventBus::AddHandler<IndicatorTooltipEvent>(*this);
}

void
DBusProtocol::onEvent(AccountListEvent &event) {
  LOG(WARNING) << "D-BUS implement me: AccountListEvent";
};

void
DBusProtocol::onEvent(AccountSwitchEvent &event) {
  LOG(WARNING) << "D-BUS implement me: AccountSwitchEvent";
};

void
DBusProtocol::onEvent(IndicatorBadgeEvent &event) {
  GError *error = NULL;
  gboolean result;

  result = g_dbus_connection_emit_signal (
     connection_,
     NULL,
     kAppPath,
     kAppInterface,
     "IndicatorBadgeChanged",
     g_variant_new ("(ib)", event.getBadge(), event.isImportant()),
     &error);

  if (!result) {
    LOG(ERROR) << "Failed to emit IndicatorBadgeChanged signal: " << error->message;
    g_error_free (error);
  }
};

void
DBusProtocol::onEvent(IndicatorStateEvent &event) {
  GError *error = NULL;
  gboolean result;

  result = g_dbus_connection_emit_signal (
     connection_,
     NULL,
     kAppPath,
     kAppInterface,
     "IndicatorStateChanged",
     g_variant_new ("(s)", event.getState().c_str()),
     &error);

  if (!result) {
    LOG(ERROR) << "Failed to emit IndicatorStateChanged signal: " << error->message;
    g_error_free (error);
  }
};

void
DBusProtocol::onEvent(IndicatorTooltipEvent &event) {
  GError *error = NULL;
  gboolean result;

  result = g_dbus_connection_emit_signal (
     connection_,
     NULL,
     kAppPath,
     kAppInterface,
     "IndicatorTooltipChanged",
     g_variant_new ("(s)", event.getTooltip().c_str()),
     &error);

  if (!result) {
    LOG(ERROR) << "Failed to emit IndicatorTooltipChanged signal: " << error->message;
    g_error_free (error);
  }
};