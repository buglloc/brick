// Copyright (c) 2015 The Brick Authors.

#include "brick/brick_app.h"

#include <unistd.h>
#include <gdk/gdkx.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

#include "third-party/json/json.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_app.h"
#include "brick/indicator/indicator.h"
#include "brick/client_handler.h"
#include "brick/client_app.h"
#include "brick/window_util.h"
#include "brick/helper.h"
#include "brick/platform_util.h"
#include "brick/external_interface/dbus_protocol.h"

#undef Status   // Definition conflicts with cef_urlrequest.h
#undef Success  // Definition conflicts with cef_message_router.h

namespace {
  const int kMainWindowWidth = 914;
  const int kMainWindowHeight = 454;
  const int32 kIdleTimeout = 600000;
  const int32 kIdleCheckInterval = 4000;
  std::string kAppIcons[] = {"brick16.png", "brick32.png", "brick48.png", "brick128.png", "brick256.png"};
  std::string szWorkingDir;  // The current working directory

  bool
  GetWorkingDir(std::string& dir) {
    char buff[1024];

    // Retrieve the executable path.
    ssize_t len = readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len == -1)
      return false;

    buff[len] = 0;
    dir = std::string(buff);
    // Remove the executable name from the path.
    dir = dir.substr(0, dir.find_last_of("/"));

    return true;
  }

  bool
  EnsureSystemDirectoryExists() {
    return (
       platform_util::MakeDirectory(std::string(BrickApp::GetConfigHome()) + "/" + APP_COMMON_NAME)
        && platform_util::MakeDirectory(std::string(BrickApp::GetCacheHome()) + "/" + APP_COMMON_NAME)
    );
  }

  bool
  EnsureSingleInstance() {
    std::string lock_file = std::string(BrickApp::GetCacheHome()) + "/" + APP_COMMON_NAME + "/run.lock";
    int fd = open(lock_file.c_str(), O_CREAT, 0600);
    if (fd == -1)
      return true;

    return flock(fd, LOCK_EX|LOCK_NB) == 0;
  }

  gboolean
  CheckUserIdle(gpointer data) {
    /* Query xscreensaver */
    static XScreenSaverInfo *mit_info = NULL;
    static int has_extension = -1;
    int event_base, error_base;

    if (has_extension == -1)
      has_extension = XScreenSaverQueryExtension(GDK_DISPLAY(), &event_base, &error_base);

    if (!has_extension) {
      LOG(WARNING) << "XScreenSaver extension not found, 'auto away' feature is  being disabled.";
      // Don't call this function any more
      return false;
    }

    CefRefPtr<ClientHandler> handler = ClientHandler::GetInstance();
    if (!handler)
      return true;

    if (mit_info == NULL)
      mit_info = XScreenSaverAllocInfo();

    XScreenSaverQueryInfo(GDK_DISPLAY(), GDK_ROOT_WINDOW(), mit_info);

    if (mit_info->idle >= kIdleCheckInterval && !handler->IsIdle()) {
      UserAwayEvent e(true);
      EventBus::FireEvent(e);
    } else if (mit_info->idle < kIdleCheckInterval && handler->IsIdle() && handler->IsIdlePending()) {
      UserAwayEvent e(false);
      EventBus::FireEvent(e);
    } else if (!handler->IsIdlePending()) {
      handler->SetIdlePending(true);
    }

    return true;
  }

  void
  GdkEventDispatcher(GdkEvent *event, gpointer data) {
    BrowserWindow *window = window_util::LookupBrowserWindow(event);
    if (window != NULL) {
      // We have event for browser window
      window->OnNativeEvent(event);
    }
    gtk_main_do_event(event);
  }

  bool
  CheckLogFileSize(CefString path) {
    std::string file_path = path;
    struct stat stat_buf;
    int rc = stat(file_path.c_str(), &stat_buf);

    if (rc == 0 && stat_buf.st_size > 1024*1024*20) {
      unlink(file_path.c_str());
      LOG(INFO) << "Runtime log is too big (" << stat_buf.st_size << ") and will be truncated";
    }

    return true;
  }
}  // namespace

// static
const char*
BrickApp::GetConfigHome() {
  return g_get_user_config_dir();
}

// static
std::string
BrickApp::FindUserConfig(const char* name) {
  std::string result = "";
  char* filename = g_build_filename(g_get_user_config_dir(), APP_COMMON_NAME, name, NULL);
  if (platform_util::IsPathExists(filename))
    result = filename;
  g_free(filename);
  return result;
}

// static
std::string
BrickApp::FindSystemConfig(const char* name) {
  std::string result = "";
  const gchar* const *dirs = g_get_system_config_dirs();
  const gchar* const *dir;
  char* filename;

  for (dir = dirs; *dir; ++dir) {
    filename = g_build_filename(*dir, APP_COMMON_NAME, name, NULL);
    if (platform_util::IsPathExists(filename)) {
      result = filename;
      g_free(filename);
      break;
    }
    g_free(filename);
  }

  return result;
}

// static
const char*
BrickApp::GetCacheHome() {
  return g_get_user_cache_dir();
}

void
TerminationSignalHandler(int signatl) {
  CefRefPtr<ClientHandler> handler = ClientHandler::GetInstance();
  if (handler) {
    handler->Shutdown(false);
  } else {
    CefQuitMessageLoop();
  }
}

int main(int argc, char* argv[]) {
  // ToDo: Good refactoring candidate!

  CefMainArgs main_args(argc, argv);
  CefRefPtr<ClientApp> app(new ClientApp);

  // Execute the secondary process, if any.
  int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
  if (exit_code >= 0)
    return exit_code;

  if (!EnsureSystemDirectoryExists()) {
    printf("Can't create system directories");
    return 1;
  }

  GetWorkingDir(szWorkingDir);
  AppSettings app_settings = AppSettings::InitByJson(
     BrickApp::GetSystemConfig()
  );
  app_settings.UpdateByJson(
     BrickApp::GetUserConfig()
  );

  app_settings.resource_dir = helper::BaseDir(szWorkingDir) + "/resources/";
  CefSettings cef_settings = BrickApp::GetCefSettings(szWorkingDir, app_settings);

  LOG_IF(WARNING,
     !CheckLogFileSize(&cef_settings.log_file)) << "Can't check runtie log file size";

  // If we have D-BUS - check single running with it (and call App::Present)
  // Otherwise - fallback to EnsureSingleInstance with flock
  CefRefPtr<DBusProtocol> dbus = NULL;
  if (app_settings.external_api) {
    dbus = new DBusProtocol;
    if (dbus->Init() && !dbus->isSingleInstance()) {
      // We already own D-BUS session in another instance
      dbus->BringAnotherInstance();
      printf("Another instance is already running.");
      return 0;
    }
  } else if (!EnsureSingleInstance()) {
    printf("Another instance is already running.");
    return 0;
  }

  CefRefPtr<AccountManager> account_manager(new AccountManager);
  account_manager->Init(
     std::string(BrickApp::GetConfigHome()) + "/" + APP_COMMON_NAME + "/accounts.json"
  );

  CefRefPtr<CacheManager> cache_manager(new CacheManager);
  cache_manager->Init(
     std::string(BrickApp::GetCacheHome()) + "/" + APP_COMMON_NAME + "/app/"
  );
  // ToDo: need to be safer?
  cache_manager->CleanUpCache();

  gtk_init(0, NULL);
  // ToDo: Maybe better to set scale in main_args here?
  app->SetDeviceScaleFactor(window_util::GetDeviceScaleFactor());
  // Initialize CEF.
  CefInitialize(main_args, cef_settings, app.get(), NULL);

  gdk_event_handler_set(GdkEventDispatcher, NULL, NULL);
  window_util::InitHooks();

  if (app_settings.auto_away) {
    gtk_timeout_add(kIdleTimeout, CheckUserIdle, NULL);
  }

  // Set default windows icon. Important to do this before any GTK window created!
  GList *icons = NULL;
  std::string icon_path = app_settings.resource_dir + "/app_icons/";
  int icons_count = sizeof(kAppIcons) / sizeof(kAppIcons[0]);
  for (int i = 0; i < icons_count; ++i) {
    GdkPixbuf *icon = gdk_pixbuf_new_from_file((icon_path + kAppIcons[i]).c_str(), NULL);
    if (!icon)
      continue;
    icons = g_list_append(icons, icon);
  }
  window_util::SetDefaultIcons(icons);

  // Create the handler.
  CefRefPtr<ClientHandler> client_handler(new ClientHandler);
  client_handler->SetAppSettings(app_settings);
  client_handler->SetAccountManager(account_manager);
  client_handler->SetNotificationManager(new NotificationManager);
  client_handler->SetCacheManager(cache_manager);

  // Initialize status icon
  CefRefPtr<BrickIndicator> brick_indicator(new BrickIndicator(app_settings.resource_dir + "/indicators/"));
  brick_indicator->UseExtendedStatus(app_settings.extended_status);
  client_handler->SetIndicatorHandle(brick_indicator);

  std::string startup_url = account_manager->GetCurrentAccount()->GetBaseUrl();
  if (account_manager->GetCurrentAccount()->IsExisted()) {
    // Login to our account
    startup_url += "internals/pages/portal-loader#login=yes";
  } else {
    // Otherwise let's show error page
    startup_url += "internals/pages/home";
  }

  // Setup main window size & positions
  CefRect screen_rect = window_util::GetDefaultScreenRect();
  CefWindowInfo window_info;
  window_info.width = static_cast<unsigned int>(kMainWindowWidth * window_util::GetDeviceScaleFactor());
  window_info.height = static_cast<unsigned int>(kMainWindowHeight * window_util::GetDeviceScaleFactor());
  window_info.x = screen_rect.x + (screen_rect.width - window_info.width) / 2;
  window_info.y = screen_rect.y + (screen_rect.height - window_info.height) / 2;

  // Create browser
  CefBrowserHost::CreateBrowserSync(
     window_info, client_handler.get(),
     startup_url, BrickApp::GetBrowserSettings(szWorkingDir, app_settings), NULL);

  // Install a signal handler so we clean up after ourselves.
  signal(SIGINT, TerminationSignalHandler);
  signal(SIGTERM, TerminationSignalHandler);

  CefRunMessageLoop();

  CefShutdown();

  return 0;
}
