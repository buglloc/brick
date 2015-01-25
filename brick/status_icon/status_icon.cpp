#include <include/cef_app.h>

#include "../cef_handler.h"
#include "../window/about_window.h"
#include "../window/accounts_window.h"

namespace {

    const char *const icon_names_[] = {
       "offline",
       "offline",
       "online",
       "flash",
       "flash-important"
    };

    const char *const icon_files_[] = {
       "offline.svg",
       "offline.svg",
       "online.svg",
       "flash.svg",
       "flash-important.svg"
    };
}

StatusIcon::StatusIcon(std::string icons_dir)
   : icons_folder_ (icons_dir),
     icon_handler_(NULL)
{
  Init();
}

StatusIcon::~StatusIcon() {
}

std::string
StatusIcon::GetIconPath(Icon icon) {
  return icons_folder_ + icon_files_[icon];
}

std::string
StatusIcon::GetIconName(Icon icon) {
  return icon_names_[icon];
}

unsigned char
StatusIcon::GetIcon() {
  return current_icon_;
}

void
StatusIcon::SetBadge(int badge, bool is_important) {
    if (
        (is_important || GetIcon() != StatusIcon::Icon::FLASH_IMPORTANT)
        && badge > 0
       ) {
        // Regular flash (e.g. notification) can't replace important flash status (e.g. messages)
        if (is_important) {
            SetIcon(StatusIcon::Icon::FLASH_IMPORTANT);
        } else {
            SetIcon(StatusIcon::Icon::FLASH);
        }
#ifdef unity
        // Parts of simple Unity integration - let's set badge in launcher!
        unity_launcher_entry_set_count(launcher_handler_, badge);
#endif
    } else {
        // if you don't know what to do just set "online" status
        SetIcon(StatusIcon::Icon::ONLINE);
#ifdef unity
        unity_launcher_entry_set_count(launcher_handler_, 0);
#endif
    }
}

bool
StatusIcon::OnClick() {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  if (!client_handler)
    return false;

  CefRefPtr<BrowserWindow> window = client_handler->GetMainWindowHandle();
  if (!window)
    return false;

  window->ToggleVisibility();

  return true;
}

bool
StatusIcon::OnMenuQuit() {
  CefQuitMessageLoop();
  return true;
}

bool
StatusIcon::OnMenuPortalOpen() {
  return true;
}

bool
StatusIcon::OnMenuAbout() {
  AboutWindow::Instance()->Show();
  return true;
}

bool
StatusIcon::OnMenuManageAccount() {
  AccountsWindow::Instance()->Show();
  return true;
}

bool
StatusIcon::OnMenuChangeAccount(int id) {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  CefRefPtr<AccountManager> account_manager = client_handler->GetAccountManager();
  if (account_manager->GetCurrentAccount()->GetId() == id)
    return true; // Selected current account

  SetIcon(Icon::OFFLINE);
  client_handler->SwitchAccount(id);

  return true;
}