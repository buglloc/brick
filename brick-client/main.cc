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

}  // namespace


bool
CallCommand(bool to_app, const std::string &command, GVariant *parameters, bool allow_autostart) {
  GDBusProxy *proxy = NULL;
  GVariant *call_result = NULL;
  GError *error = NULL;

  GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS;
  if (!allow_autostart)
    flags = static_cast<GDBusProxyFlags>(flags | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START);

  proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                        flags,
                                        NULL,
                                        kAppBusName,
                                        to_app ? kAppPath : kAppWindowPath,
                                        to_app ? kAppInterface : kAppWindowInterface,
                                        NULL,
                                        &error);

  if (proxy == NULL) {
    std::cerr <<  "Can't create proxy. Error: " << error->message << std::endl;
    g_error_free(error);
    std::exit(EXIT_FAILURE);
  }

  call_result = g_dbus_proxy_call_sync(proxy,
                                       command.c_str(),
                                       parameters != nullptr ? parameters : g_variant_new("()"),
                                       G_DBUS_CALL_FLAGS_NONE,
                                       -1,
                                       NULL,
                                       &error);

  g_object_unref(proxy);

  if (call_result == NULL) {
    std::cerr << "Can't call command: " << error->message << std::endl;
    g_error_free(error);
    std::exit(EXIT_FAILURE);
  }

  g_variant_unref(call_result);
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
    for (decltype(tokens.size()) i = 1; i < tokens.size() - 1; i += 2 ) {
      g_variant_builder_add(&detail_builder, "{ss}",
                            tokens[i].c_str(),
                            tokens[i + 1].c_str());
    }

    detail_params = g_variant_builder_end(&detail_builder);
  } else {
    detail_params = g_variant_new_array(G_VARIANT_TYPE("{ss}"), NULL, 0);
  }

  GVariantBuilder params_builder;
  g_variant_builder_init(&params_builder, G_VARIANT_TYPE_TUPLE);
  g_variant_builder_add(&params_builder, "s", tokens[0].c_str());
  g_variant_builder_add_value(&params_builder, detail_params);

  GVariant *params = g_variant_builder_end(&params_builder);
  CallCommand(true, "Action", params, true);
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
  for (const auto &command : window_commands) {
    std::cout << "\t" << command.first << std::endl;
  }
  std::cout << "\n" << std::endl;
  std::cout << "App commands:" << std::endl;
  for (const auto &command : app_commands) {
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
