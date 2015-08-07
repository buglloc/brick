// Copyright (c) 2015 The Brick Authors.

#include "brick-client/main.h"

#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>



namespace {
  static const char *kAppBusName = "org.brick.Brick";
  static const char *kAppInterface = "org.brick.Brick.AppInterface";
  static const char *kAppPath = "/org/brick/Brick/App";
  static const char *kAppWindowInterface = "org.brick.Brick.AppWindowInterface";
  static const char *kAppWindowPath = "/org/brick/Brick/AppWindow";

  static const char *kDBusName = "org.freedesktop.DBus";
  static const char *kDBusPath = "/org/freedesktop/DBus";
  static const char *kDBusInterface = "org.freedesktop.DBus";
  static const char *kDBusMethodHasOwner = "NameHasOwner";
  static const char *kDBusMethodStartApp = "StartServiceByName";


  static const char *kUsage =
      "Usage: brick-client command\n";

  static std::map<std::string, std::string> window_commands = {
      {"present", "Present"},
      {"hide", "Hide"},
      {"toggle", "ToggleVisibility"}
  };

  static std::map<std::string, std::string> app_commands = {
      {"user_away", "UserAway"},
      {"user_present", "UserPresent"},
      {"quit", "Quit"}
  };

  bool g_dbus_inited = false;
  GDBusConnection *g_connection = nullptr;


  std::vector<std::string>
  Split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while (std::getline(ss, item, delim)) {
      if (item.empty())
        continue;

      tokens.push_back(item);
    }

    return tokens;
  }


  bool
  InitDbus() {
    if (g_dbus_inited) {
      return true;
    }

    if (glib_check_version(2, 36, 0)) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
      // In older version of glib we must call g_type_init manually (see https://developer.gnome.org/gobject/unstable/gobject-Type-Information.html#g-type-init)
      g_type_init();
#pragma GCC diagnostic pop
    }

    GError *error = nullptr;
    g_connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (g_connection == NULL) {
      std::cerr << "Failed to connect D-Bus session: " << error->message;
      g_error_free(error);
      return false;
    }

    g_dbus_inited = true;
    return true;
  }

  bool
  EnsureBrickStarted() {
    static bool first_call = true;
    bool brick_started = false;
    GError *error = nullptr;
    /* Checks if Brick is already running */
    GVariant *result = g_dbus_connection_call_sync(g_connection,
                                                   kDBusName,               /* bus name */
                                                   kDBusPath,               /* object path */
                                                   kDBusInterface,          /* interface name */
                                                   kDBusMethodHasOwner,          /* method name */
                                                   g_variant_new ("(s)", kAppBusName),
                                                   G_VARIANT_TYPE ("(b)"),
                                                   G_DBUS_CALL_FLAGS_NONE,
                                                   -1,
                                                   NULL,
                                                   &error);
    if (result == NULL) {
      std::cerr << "Failed to complete NameHasOwner" << error->message;
      g_error_free(error);
      return false;
    }

    g_dbus_inited = true;
    g_variant_get(result, "(b)", &brick_started);
    g_variant_unref(result);
    if (!brick_started && first_call) {
      first_call = false;
      result = g_dbus_connection_call_sync(g_connection,
                                           kDBusName,
                                           kDBusPath,
                                           kDBusInterface,
                                           kDBusMethodStartApp,
                                           g_variant_new ("(su)", kAppBusName, 0),
                                           NULL,
                                           G_DBUS_CALL_FLAGS_NONE,
                                           -1,
                                           NULL,
                                           &error);
      if (result == NULL) {
        std::cerr << "Failed to complete StartServiceByName: " << error->message << std::endl;
        g_error_free(error);
        return false;
      }
      return EnsureBrickStarted();
    }

    return brick_started;
  }

}  // namespace


bool
CallCommand(bool to_app, const std::string &command, GVariant *parameters) {
  if (!InitDbus() || !EnsureBrickStarted()) {
    std::cerr << "Can't start Brick." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  GError *error = NULL;
  GVariant *result = g_dbus_connection_call_sync(g_connection,
                                                 kAppBusName,
                                                 to_app ? kAppPath : kAppWindowPath,
                                                 to_app ? kAppInterface : kAppWindowInterface,
                                                 command.c_str(),
                                                 parameters != nullptr ? parameters : g_variant_new("()"),
                                                 G_VARIANT_TYPE("()"),
                                                 G_DBUS_CALL_FLAGS_NONE,
                                                 -1, NULL, &error);

  if (!result) {
    std::cerr << "Can't call command: " << error->message << std::endl;
    g_error_free(error);
    std::exit(EXIT_FAILURE);
  }

  return true;
}


void
HandleBxProtocol(const std::string &requestLine) {
  /*
    Bitrix send: /callTo/phone/+789213456576/params/
    Brick wait action in format "sa{ss}":
      ("callTo", {"phone": "+789213456576"})
   */

  std::vector<std::string> tokens = Split(requestLine, '/');

  if (tokens.empty()) {
    std::cerr << "Malformed Bitrix-protocol request." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  GVariant *detail_params = nullptr;
  if (tokens.size() > 2) {
    GVariantBuilder detail_builder;
    g_variant_builder_init(&detail_builder, G_VARIANT_TYPE_ARRAY);
    for (decltype(tokens.size()) i = 1; i < tokens.size() - 1; i+=2) {
      g_variant_builder_add(&detail_builder, "{ss}",
                            tokens[i].c_str(),
                            tokens[i + 1].c_str());
    };

    detail_params = g_variant_builder_end(&detail_builder);
  } else {
    detail_params = g_variant_new_array(G_VARIANT_TYPE("a{ss}"), NULL, 0);
  }

  GVariantBuilder params_builder;
  g_variant_builder_init(&params_builder, G_VARIANT_TYPE_TUPLE);
  g_variant_builder_add(&params_builder, "s", tokens[0].c_str());
  g_variant_builder_add_value(&params_builder, detail_params);

  GVariant *params = g_variant_builder_end(&params_builder);
  CallCommand(true, "Action", params);
}


void
HandleCommand(const std::string &command) {
  if (window_commands.count(command)) {
    CallCommand(false, window_commands[command]);
  } else if (app_commands.count(command)) {
    CallCommand(true, app_commands[command]);
  } else {
    std::cerr << "Command \"" << command << "\" not found." << std::endl;
    std::exit(EXIT_FAILURE);
  }
}


void PrintUsage() {
  std::cout << kUsage << std::endl;
  std::cout << "Window commands:" << std::endl;
  for (const auto &command: window_commands) {
    std::cout << "\t" << command.first << std::endl;
  }
  std::cout << "\n" << std::endl;
  std::cout << "App commands:" << std::endl;
  for (const auto &command: app_commands) {
    std::cout << "\t" << command.first << std::endl;
  }
}


int
main(int argc, char** argv) {

  if (argc != 2) {
    std::cerr << "Too few arguments, command expected." << std::endl;
    PrintUsage();
    std::exit(EXIT_FAILURE);
  }

  std::string command(argv[1]);
  if (command.find("bx:") == 0) {
    HandleBxProtocol(command.substr(sizeof("bx:") - 1));
  } else if (command == "-h" || command == "--help") {
    PrintUsage();
  } else {
    HandleCommand(command);
  }

  std::exit(EXIT_SUCCESS);
}