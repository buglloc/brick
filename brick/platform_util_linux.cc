// Copyright (c) 2015 The Brick Authors.

#include "brick/platform_util.h"

#include <sys/stat.h>
#include <glib.h>
#include <gio/gio.h>

#include <string>
#include <fstream>

#include "include/wrapper/cef_helpers.h"
#include "brick/helper.h"

namespace {

  // The KDE session version environment variable used in KDE 4.
  const char kKDE4SessionEnvVar[] = "KDE_SESSION_VERSION";
  const char kFileManagerName[] = "org.freedesktop.FileManager1";
  const char kFileManagerPath[] = "/org/freedesktop/FileManager1";
  const char kFileManagerInterface[] = "org.freedesktop.FileManager1";
  const char kFileManagerMethod[] = "ShowItems";

  bool
  LaunchProcess(const std::vector<std::string>& args) {
    std::vector<char*> argv(args.size() + 1, NULL);
    for (size_t i = 0; i < args.size(); ++i) {
      argv[i] = const_cast<char*>(args[i].c_str());
    }

    gboolean result = g_spawn_async(nullptr, &argv[0], nullptr,
                                    static_cast<GSpawnFlags>(G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL),
                                    nullptr, nullptr, nullptr, nullptr);

    if (!result) {
      std::ostringstream command_line;
      for (const auto &arg : args) {
        command_line << arg << " ";
      }

      LOG(WARNING) << "Failed to launch external process: " << command_line.str();
      return false;
    }

    return true;
  }

  void
  XDGUtil(const std::string& util, const std::string& arg) {
    std::vector<std::string> argv;
    argv.push_back(util.c_str());
    argv.push_back(arg.c_str());
    LaunchProcess(argv);
  }

  void
  XDGOpen(const std::string& path) {
    XDGUtil("xdg-open", path);
  }

  void
  XDGEmail(const std::string& email) {
    XDGUtil("xdg-email", email);
  }


  bool
  ShowInFileManager(const std::string &path) {
    static bool available = true;
    GDBusProxy *proxy = NULL;
    GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE;
    gchar *uri = NULL;
    GVariant *call_result = NULL;
    GError *error = NULL;

    if (!available) {
      return false;
    }

    proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                          flags,
                                          NULL,
                                          kFileManagerName,
                                          kFileManagerPath,
                                          kFileManagerInterface,
                                          NULL,
                                          &error);

    if (proxy == NULL) {
      LOG(WARNING) << "Can't create proxy for file showing. Error: " << error->message;
      g_error_free(error);
      return false;
    }

    uri = g_filename_to_uri(path.c_str(), NULL, NULL);
    if (uri == NULL) {
      return false;
    }

    const gchar *uris[2] = {uri, NULL};
    const gchar *startup_id = "dontstealmyfocus";

    call_result = g_dbus_proxy_call_sync(proxy,
                                         kFileManagerMethod,  // method
                                         g_variant_new("(^ass)", uris, startup_id),  // parameters
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    g_object_unref(proxy);
    g_free(uri);

    if (call_result != NULL) {
      g_variant_unref(call_result);
    } else {
      available = false;
      LOG(WARNING) << "Can't call " << kFileManagerInterface << "." << kFileManagerMethod << ". Error: " << error->message;
      g_error_free(error);
      return false;
    }

    return true;
  }

  const std::string
  GetAutostartEntryPath() {
    return platform_util::GetConfigHome() + "/autostart/brick.desktop";
  }

}  // namespace

namespace platform_util {

  void
  OpenExternal(const std::string &url) {
    // TODO(buglloc): Use gtk_show_uri instead?
    if (url.find("mailto:") == 0)
      XDGEmail(url);
    else
      XDGOpen(url);
  }

  bool
  IsPathExists(const std::string &path) {
    struct stat stat_data;
    return stat(path.c_str(), &stat_data) != -1;
  }

  bool
  IsDir(const std::string &path) {
    struct stat stat_data;
    return stat(path.c_str(), &stat_data) != -1 && S_ISDIR(stat_data.st_mode);
  }

  bool
  MakeDirectory(const std::string &path) {
    if (IsPathExists(path))
      return true;

    size_t pre = 0;
    size_t pos;
    std::string dir;
    std::string target_path = path;
    if (target_path[target_path.size()-1] != '/') {
      // force trailing / so we can handle everything in loop
      target_path += '/';
    }

    while ((pos = target_path.find_first_of('/', pre)) != std::string::npos) {
      dir = target_path.substr(0, pos++);
      pre = pos;

      if (dir.size() == 0)
        continue;

      if (mkdir(dir.c_str(), 0700) && errno != EEXIST) {
        return false;
      }
    }

    return true;
  }

  bool
  GetEnv(const char* variable_name, std::string* result) {
    const char *env_value = getenv(variable_name);

    if (!env_value)
      return false;

    // Note that the variable may be defined but empty.
    if (result)
      *result = env_value;

    return true;
  }

  bool
  HasEnv(const char* variable_name) {
    return GetEnv(variable_name, NULL);
  }

  DesktopEnvironment
  GetDesktopEnvironment() {
    // XDG_CURRENT_DESKTOP is the newest standard circa 2012.
    std::string xdg_current_desktop;
    if (GetEnv("XDG_CURRENT_DESKTOP", &xdg_current_desktop)) {
      // Not all desktop environments set this env var as of this writing.
      if (xdg_current_desktop == "Unity") {
        // gnome-fallback sessions set XDG_CURRENT_DESKTOP to Unity
        // DESKTOP_SESSION can be gnome-fallback or gnome-fallback-compiz
        std::string desktop_session;
        if (GetEnv("DESKTOP_SESSION", &desktop_session) &&
            desktop_session.find("gnome-fallback") != std::string::npos) {
          return DESKTOP_ENVIRONMENT_GNOME;
        }
        return DESKTOP_ENVIRONMENT_UNITY;
      } else if (xdg_current_desktop == "GNOME") {
        return DESKTOP_ENVIRONMENT_GNOME;
      } else if (xdg_current_desktop == "KDE") {
        return DESKTOP_ENVIRONMENT_KDE;
      } else if (xdg_current_desktop == "X-Cinnamon") {
        return DESKTOP_ENVIRONMENT_CINNAMON;
      }
    }

    // DESKTOP_SESSION was what everyone used in 2010.
    std::string desktop_session;
    if (GetEnv("DESKTOP_SESSION", &desktop_session)) {
      if (desktop_session == "gnome" || desktop_session =="mate") {
        return DESKTOP_ENVIRONMENT_GNOME;
      } else if (desktop_session == "kde4" || desktop_session == "kde-plasma") {
        return DESKTOP_ENVIRONMENT_KDE;
      } else if (desktop_session == "kde") {
        // This may mean KDE4 on newer systems, so we have to check.
        if (HasEnv(kKDE4SessionEnvVar))
          return DESKTOP_ENVIRONMENT_KDE;
        return DESKTOP_ENVIRONMENT_KDE_OLD;
      } else if (desktop_session.find("xfce") != std::string::npos ||
                 desktop_session == "xubuntu") {
        return DESKTOP_ENVIRONMENT_XFCE;
      }
    }

    // Fall back on some older environment variables.
    // Useful particularly in the DESKTOP_SESSION=default case.
    if (HasEnv("GNOME_DESKTOP_SESSION_ID")) {
      return DESKTOP_ENVIRONMENT_GNOME;
    } else if (HasEnv("KDE_FULL_SESSION")) {
      if (HasEnv(kKDE4SessionEnvVar))
        return DESKTOP_ENVIRONMENT_KDE;
      return DESKTOP_ENVIRONMENT_KDE_OLD;
    }

    return DESKTOP_ENVIRONMENT_OTHER;
  }

  const std::string
  GetDefaultDownloadDir() {
    const gchar * dir = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
    if (dir == NULL) {
      // Fallback to $HOME
      dir = g_get_home_dir();
    }

    if (dir != NULL) {
      return dir;
    }

    return "";
  }

  const std::string
  GetHomeDir() {
    return g_get_home_dir();
  }

  const std::string
  GetCacheDir() {
    return g_get_user_cache_dir();
  }

  const std::string
  GetConfigHome() {
    return g_get_user_config_dir();
  }

  void
  ShowInFolder(const std::string &path) {

    if (IsDir(path)) {
      // If we already have a folder - just open it
      XDGOpen(path);
    } else if (!ShowInFileManager(path)) {
      // Fallback to open folder w/o highlighting item
      XDGOpen(helper::BaseDir(path));
    }
  }

  bool
  IsAutostartEnabled() {
    return IsPathExists(GetAutostartEntryPath());
  }

  void
  EnableAutostart() {
    /* TODO(buglloc):
     * Maybe we just copy some .desktop file from resources?
     * What if we want to override some in package?
     */

    const std::string path = GetAutostartEntryPath();
    // Ensure ~/.config/autostart directory exists
    MakeDirectory(helper::BaseDir(path));
    std::ofstream ofs(path);
    ofs << R"entry([Desktop Entry]
Encoding=UTF-8
Name=Brick
GenericName=Internet Messenger
Comment=Bitrix24 messenger
Type=Application
Categories=Network;InstantMessaging;
Exec=brick --minimized --autostarted
Icon=brick
Terminal=false
)entry";
    ofs.close();
    chmod(path.c_str(), S_IRUSR|S_IWUSR);
  }

  void
  DisableAutostart() {
    const std::string path = GetAutostartEntryPath();
    if (!IsPathExists(path))
      return;

    unlink(path.c_str());
  }
}  // namespace platform_util
