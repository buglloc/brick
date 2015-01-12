#include <X11/Xlib.h>
#include <include/wrapper/cef_helpers.h>
#include <include/cef_app.h>
#include <unistd.h>
#include <gdk/gdkx.h>
#include <sys/file.h>

#include "brick_app.h"
#include "third-party/json/json.h"
#include "status_icon/status_icon.h"
#include "cef_handler.h"
#include "cef_app.h"
#include "window_util.h"
#include "helper.h"
#include "platform_util.h"


#undef Status   // Definition conflicts with cef_urlrequest.h
#undef Success  // Definition conflicts with cef_message_router.h

namespace {
    std::string APPICONS[] = {"brick16.png", "brick32.png", "brick48.png", "brick128.png", "brick256.png"};
    std::string szWorkingDir;  // The current working directory

    bool GetWorkingDir(std::string& dir) {
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

    bool EnsureSystemDirectoryExists() {
      return (
         platform_util::MakeDirectory(std::string(BrickApp::GetConfigHome()) + "/" + APP_COMMON_NAME)
          && platform_util::MakeDirectory(std::string(BrickApp::GetCacheHome()) + "/" + APP_COMMON_NAME)
      );
    }

    bool EnsureSingleInstance() {
      // ToDo: Replaced by IPC request, when IPC will be implemented
      std::string lock_file = std::string(BrickApp::GetCacheHome()) + "/" + APP_COMMON_NAME + "/run.lock";
      int fd = open(lock_file.c_str(), O_CREAT, 0600);
      if (fd == -1)
        return true;

      return flock(fd, LOCK_EX|LOCK_NB) == 0;
    }
}

// static
const char*
BrickApp::GetConfigHome() {
  return g_get_user_config_dir();
}

// static
const char*
BrickApp::GetCacheHome() {
  return g_get_user_cache_dir();
}

void
TerminationSignalHandler(int signatl) {
  CefQuitMessageLoop();
}

int main(int argc, char* argv[]) {
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

  if (!EnsureSingleInstance()) {
    // ToDo: change main window stack order, when IPC will be implemented
    printf("Another instance is already running.\nExiting.");
    return 0;
  }

  GetWorkingDir(szWorkingDir);
  std::string plain_config = BrickApp::GetConfig();
  AppSettings app_settings = AppSettings::InitByJson(plain_config);
  app_settings.resource_dir = helper::BaseDir(szWorkingDir) + "/resources/";

  CefRefPtr<AccountManager> account_manager(new AccountManager);
  // ToDo: Fix this bullshit!
  account_manager->Init(
     std::string(BrickApp::GetConfigHome()) + "/" + APP_COMMON_NAME + "/accounts.json"
  );

  // Initialize CEF.
  CefInitialize(main_args, BrickApp::GetCefSettings(szWorkingDir, app_settings), app.get(), NULL);

  // Create the handler.
  CefRefPtr<ClientHandler> client_handler(new ClientHandler);

  // Set default windows icon. Important to do this before any GTK window created!
  GList *list = NULL;
  std::string icon_path = app_settings.resource_dir + "/app_icons/";
  int icons_count = sizeof(APPICONS) / sizeof(APPICONS[0]);
  for (int i = 0; i < icons_count; ++i) {
    GdkPixbuf *icon = gdk_pixbuf_new_from_file((icon_path + APPICONS[i]).c_str(), NULL);
    if (!icon)
      continue;
    list = g_list_append(list, icon);
  }
  gtk_window_set_default_icon_list(list);
  g_list_foreach(list, (GFunc) g_object_unref, NULL);
  g_list_free(list);

  // Initialize main window
  CefRefPtr<MainWindow> main_window(new MainWindow);
  main_window->Init();
  main_window->SetTitle(APP_NAME);
  main_window->Show();
  client_handler->SetAppSettings(app_settings);
  client_handler->SetMainWindowHandle(main_window);
  client_handler->SetAccountManager(account_manager);

  // Initialize status icon
  CefRefPtr<StatusIcon> status_icon(new StatusIcon(app_settings.resource_dir + "/indicators/"));
  client_handler->SetStatusIconHandle(status_icon);

  CefWindowInfo window_info;
  // The GTK window must be visible before we can retrieve the XID.
  ::Window xwindow = GDK_WINDOW_XID(gtk_widget_get_window(main_window->GetHandler()));
  window_info.SetAsChild(xwindow, CefRect(0, 0, 0, 0));
  window_util::SetLeaderWindow(xwindow);
  window_util::InitWindow(xwindow);
  window_util::InitHooks();

  std::string startup_url = account_manager->GetCurrentAccount()->GetBaseUrl();
  if (account_manager->GetCurrentAccount()->IsExisted()) {
    // Login to our account
    startup_url += "internals/pages/portal-loader#login=yes";
  } else {
    // Otherwise let's show error page
    startup_url += "internals/pages/home";
  }

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
