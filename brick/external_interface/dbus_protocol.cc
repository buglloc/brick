// Copyright (c) 2015 The Brick Authors.

#include <gio/gio.h>

#include "include/base/cef_logging.h"
#include "include/wrapper/cef_helpers.h"
#include "brick/platform_util.h"
#include "brick/event/event_bus.h"
#include "brick/event/sleep_event.h"
#include "brick/event/user_away_event.h"

#include "brick/external_interface/dbus_protocol.h"
#include "brick/external_interface/app_message_delegate.h"
#include "brick/external_interface/app_window_message_delegate.h"

namespace {

  const char kOwnName[] = "org.brick.Brick";
  const char kAppInterface[] = "org.brick.Brick.AppInterface";
  const char kAppPath[] = "/org/brick/Brick/App";
  const char kAppWindowInterface[] = "org.brick.Brick.AppWindowInterface";
  const char kAppWindowPath[] = "/org/brick/Brick/AppWindow";

  const char kSessionUnityInterface[] = "com.canonical.Unity.Session";
  const char kSessionUnityPath[] = "/com/canonical/Unity/Session";
  const char kScreenLockInterface[] = "org.freedesktop.ScreenSaver";
  const char kScreenLockPath[] = "/org/freedesktop/ScreenSaver";
  const char kScreenLockGnomeInterface[] = "org.gnome.ScreenSaver";
  const char kScreenLockGnomePath[] = "/org/gnome/ScreenSaver";
  const char kScreenLockCinnamonInterface[] = "org.cinnamon.ScreenSaver";
  const char kScreenLockCinnamonPath[] = "/org/cinnamon/ScreenSaver";


  const gchar introspection_xml[] =
     R"xml(<node>
          <interface name='org.brick.Brick.AppInterface'>
            <method name='UserAway' />
            <method name='UserPresent' />
            <method name='ShowAddAccountDialog'>
               <arg type='b' name='switch_on_save' direction='in'/>
            </method>
            <method name='ShowAccountsDialog' />
            <method name='Action'>
              <arg type='s' name='action' direction='in'/>
              <arg type='a{ss}' name='parameters' direction='in'/>
            </method>
            <method name='Quit' />
            <signal name='IndicatorTooltipChanged'>
              <arg type='s' name='text'/>
            </signal>
            <signal name='IndicatorStateChanged'>
              <arg type='s' name='state'/>
            </signal>
            <signal name='IndicatorBadgeChanged'>
              <arg type='i' name='badge'/>
              <arg type='b' name='important'/>
            </signal>
          </interface>
          <interface name='org.brick.Brick.AppWindowInterface'>
            <method name='Hide' />
            <method name='Present' />
            <method name='ToggleVisibility' />
          </interface>
        </node>
     )xml";

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

    DBusProtocol *self = reinterpret_cast<DBusProtocol *>(user_data);
    CefRefPtr<CefProcessMessage> message =
       CefProcessMessage::Create(method_name);
    CefRefPtr<CefListValue> message_args = message->GetArgumentList();
    // TODO(buglloc): callback needed?
    message_args->SetNull(0);

    if (!g_strcmp0(method_name, "ShowAddAccountDialog")) {
      bool switch_on_save;
      g_variant_get(parameters, "(b)", &switch_on_save);
      message_args->SetBool(1, switch_on_save);
    } else if (!g_strcmp0(method_name, "Action")) {
      const gchar *action;
      GVariantIter *call_params;
      GVariant *item;
      const gchar *key;
      const gchar *value;

      g_variant_get(parameters, "(sa{ss})", &action, &call_params);
      CefRefPtr<CefDictionaryValue> js_params = CefDictionaryValue::Create();
      while ((item = g_variant_iter_next_value(call_params))) {
        g_variant_get(item, "{ss}", &key, &value);
        js_params->SetString(key, value);
        g_variant_unref(item);
      }

      message_args->SetString(1, action);
      message_args->SetDictionary(2, js_params);
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

  void
  on_sleep(GDBusConnection *connection,
     const gchar *sender_name,
     const gchar *object_path,
     const gchar *interface_name,
     const gchar *signal_name,
     GVariant *parameters,
     gpointer user_data) {

    bool is_sleep;
    g_variant_get(parameters, "(b)", &is_sleep);
    SleepEvent e(is_sleep);
    EventBus::FireEvent(e);

  }

  void
  on_locked_unity(GDBusConnection *connection,
     const gchar *sender_name,
     const gchar *object_path,
     const gchar *interface_name,
     const gchar *signal_name,
     GVariant *parameters,
     gpointer user_data) {

    UserAwayEvent e(true, true);
    EventBus::FireEvent(e);
  }

  void
  on_unlocked_unity(GDBusConnection *connection,
     const gchar *sender_name,
     const gchar *object_path,
     const gchar *interface_name,
     const gchar *signal_name,
     GVariant *parameters,
     gpointer user_data) {

    UserAwayEvent e(false, true);
    EventBus::FireEvent(e);
  }

  void
  on_session_active_changed(GDBusConnection *connection,
      const gchar *sender_name,
      const gchar *object_path,
      const gchar *interface_name,
      const gchar *signal_name,
      GVariant *parameters,
      gpointer user_data) {

    bool is_locked;
    g_variant_get(parameters, "(b)", &is_locked);
    UserAwayEvent e(is_locked, true);
    EventBus::FireEvent(e);
  }


}  // namespace

bool
DBusProtocol::Init() {
  if (glib_check_version(2, 36, 0)) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  // In older version of glib we must call g_type_init manually (see https://developer.gnome.org/gobject/unstable/gobject-Type-Information.html#g-type-init)
    g_type_init();
#pragma GCC diagnostic pop
  }

  // Simulate g_bus_own_name in sync way...
  GError *error = NULL;
  session_bus_ = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  if (!session_bus_) {
    LOG(ERROR) << "Can't register D-BUS: " << error->message;
    g_error_free(error);
    return false;
  }

  GVariant *result = g_dbus_connection_call_sync(session_bus_,
     "org.freedesktop.DBus",
     "/org/freedesktop/DBus",
     "org.freedesktop.DBus",
     "RequestName",
     g_variant_new("(su)",
        kOwnName,
        G_BUS_NAME_OWNER_FLAGS_NONE),
     G_VARIANT_TYPE("(u)"),
     G_DBUS_CALL_FLAGS_NONE,
     -1, NULL, &error);

  if (!result) {
    LOG(ERROR) << "Can't register D-BUS: " << error->message;
    g_error_free(error);
    return false;
  }

  guint32 request_name_reply;
  g_variant_get(result, "(u)", &request_name_reply);
  g_variant_unref(result);

  // DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER or DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER
  owned_ = (request_name_reply == 1 || request_name_reply == 4);

  if (owned_) {
    on_bus_acquired(session_bus_, kOwnName, this);
    RegisterSystemListeners();
    RegisterMessageDelegates();
    RegisterEventListeners();
  }

  return true;
}

bool
DBusProtocol::isSingleInstance() {
  return owned_;
}

void
DBusProtocol::BringAnotherInstance() {
  GError *error = NULL;
  GVariant *result = g_dbus_connection_call_sync(session_bus_,
     kOwnName,
     kAppWindowPath,
     kAppWindowInterface,
     "Present",
     g_variant_new("()"),
     G_VARIANT_TYPE("()"),
     G_DBUS_CALL_FLAGS_NONE,
     -1, NULL, &error);

  if (!result) {
    LOG(ERROR) << "Can't bring another instance via D-BUS: " << error->message;
    g_error_free(error);
  }
}

bool
DBusProtocol::Handle(std::string interface_name, CefRefPtr<CefProcessMessage> message) {
  bool handled = false;

  // Execute delegate callbacks.
  for (const auto &delegate : external_message_delegates_) {
    if (delegate->IsAcceptedNamespace(interface_name)) {
      handled = delegate->OnMessageReceived(
          message
      );
    }
  }

  return handled;
}

void
DBusProtocol::RegisterSystemListeners() {
  GError *error = NULL;
  GDBusConnection *system_bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
  if (!system_bus) {
    LOG(ERROR) << "Can't get system D-BUS: " << error->message;
    g_error_free(error);
    return;
  }

  g_dbus_connection_signal_subscribe(
     system_bus,
     NULL,
     "org.freedesktop.login1.Manager",
     "PrepareForSleep",
     "/org/freedesktop/login1",
     NULL,
     G_DBUS_SIGNAL_FLAGS_NONE,
     on_sleep,
     NULL,
     NULL
  );

  auto environment = platform_util::GetDesktopEnvironment();
  if (environment == platform_util::DESKTOP_ENVIRONMENT_UNITY) {
    g_dbus_connection_signal_subscribe(
        session_bus_,
        NULL,
        kSessionUnityInterface,
        "Locked",
        kSessionUnityPath,
        NULL,
        G_DBUS_SIGNAL_FLAGS_NONE,
        on_locked_unity,
        NULL,
        NULL
    );

    g_dbus_connection_signal_subscribe(
        session_bus_,
        NULL,
        kSessionUnityInterface,
        "Unlocked",
        kSessionUnityPath,
        NULL,
        G_DBUS_SIGNAL_FLAGS_NONE,
        on_unlocked_unity,
        NULL,
        NULL
    );
  } else {
    std::string interface;
    std::string path;
    switch (environment) {
      case platform_util::DESKTOP_ENVIRONMENT_GNOME:
        interface = kScreenLockGnomeInterface;
        path = kScreenLockGnomePath;
        break;

      case platform_util::DESKTOP_ENVIRONMENT_KDE:
        interface = kScreenLockInterface;
        path = kScreenLockPath;
        break;

      case platform_util::DESKTOP_ENVIRONMENT_CINNAMON:
        interface = kScreenLockCinnamonInterface;
        path = kScreenLockCinnamonPath;
        break;

      default:
        interface = kScreenLockInterface;
        path = kScreenLockPath;
    }

    g_dbus_connection_signal_subscribe(
        session_bus_,
        NULL,
        interface.c_str(),
        "ActiveChanged",
        path.c_str(),
        NULL,
        G_DBUS_SIGNAL_FLAGS_NONE,
        on_session_active_changed,
        NULL,
        NULL
    );
  }

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
DBusProtocol::OnEvent(const AccountListEvent &event) {
  LOG(WARNING) << "D-BUS implement me: AccountListEvent";
}

void
DBusProtocol::OnEvent(const AccountSwitchEvent &event) {
  LOG(WARNING) << "D-BUS implement me: AccountSwitchEvent";
}

void
DBusProtocol::OnEvent(const IndicatorBadgeEvent &event) {
  GError *error = NULL;
  gboolean result;

  result = g_dbus_connection_emit_signal(
     session_bus_,
     NULL,
     kAppPath,
     kAppInterface,
     "IndicatorBadgeChanged",
     g_variant_new("(ib)", event.GetBadge(), event.IsImportant()),
     &error);

  if (!result) {
    LOG(ERROR) << "Failed to emit IndicatorBadgeChanged signal: " << error->message;
    g_error_free(error);
  }
}

void
DBusProtocol::OnEvent(const IndicatorStateEvent &event) {
  GError *error = NULL;
  gboolean result;

  result = g_dbus_connection_emit_signal(
     session_bus_,
     NULL,
     kAppPath,
     kAppInterface,
     "IndicatorStateChanged",
     g_variant_new("(s)", event.GetState().c_str()),
     &error);

  if (!result) {
    LOG(ERROR) << "Failed to emit IndicatorStateChanged signal: " << error->message;
    g_error_free(error);
  }
}

void
DBusProtocol::OnEvent(const IndicatorTooltipEvent &event) {
  GError *error = NULL;
  gboolean result;

  result = g_dbus_connection_emit_signal(
     session_bus_,
     NULL,
     kAppPath,
     kAppInterface,
     "IndicatorTooltipChanged",
     g_variant_new("(s)", event.GetTooltip().c_str()),
     &error);

  if (!result) {
    LOG(ERROR) << "Failed to emit IndicatorTooltipChanged signal: " << error->message;
    g_error_free(error);
  }
}
