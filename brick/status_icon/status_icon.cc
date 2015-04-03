#include <include/cef_app.h>


#include "../cef_handler.h"
#include "../window/about_window.h"
#include "../window/accounts_window.h"
#include "../event/indicator_badge_event.h"

namespace {

    const char *const icon_names_[] = {
       "offline",
       "offline",
       "online",
       "dnd",
       "away",
       "flash",
       "flash-important"
    };

    const char *const icon_files_[] = {
       "offline.svg",
       "offline.svg",
       "online.svg",
       "dnd.svg",
       "away.svg",
       "flash.svg",
       "flash-important.svg"
    };
}

StatusIcon::StatusIcon(std::string icons_dir)
   : current_icon_ (Icon::DEFAULT),
     idle_icon_ (Icon::DEFAULT),
     icons_folder_ (icons_dir),
     idle_ (true),
     extended_status_ (true),
     icon_handler_ (NULL)
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

void
StatusIcon::SetBadge(int badge, bool is_important) {
  if (badge > 0) {
    if (is_important) {
        SetIcon(StatusIcon::Icon::FLASH_IMPORTANT);
    } else {
        SetIcon(StatusIcon::Icon::FLASH);
    }
#ifdef unity
    // Parts of simple Unity integration - let's set badge in launcher!
    unity_launcher_entry_set_count(launcher_handler_, badge);
    unity_launcher_entry_set_count_visible(launcher_handler_, true);
#endif
  } else {
    // restore icon for current application state
    SwitchToIdle();
#ifdef unity
    unity_launcher_entry_set_count(launcher_handler_, 0);
    unity_launcher_entry_set_count_visible(launcher_handler_, false);
#endif
  }

  IndicatorBadgeEvent e(badge, is_important);
  EventBus::FireEvent(e);
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

  SetIdleIcon(Icon::DEFAULT);
  SetIcon(Icon::DEFAULT);
  client_handler->SwitchAccount(id);

  return true;
}

void
StatusIcon::SwitchToIdle() {
  idle_ = true;
  SetIcon((Icon) idle_icon_);
}

void
StatusIcon::UseExtendedStatus(bool use) {
  extended_status_ = use;
}

void
StatusIcon::onEvent(AccountListEvent &event) {
  UpdateAccountsMenu();
};

void
StatusIcon::onEvent(AccountSwitchEvent &event) {
  UpdateAccountsMenu();
};

void
StatusIcon::RegisterEventListeners() {
  EventBus::AddHandler<AccountListEvent>(*this);
  EventBus::AddHandler<AccountSwitchEvent>(*this);
}