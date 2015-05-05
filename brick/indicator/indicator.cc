// Copyright (c) 2015 The Brick Authors.

#include "brick/indicator/indicator.h"

#include "include/cef_app.h"

#include "brick/client_handler.h"
#include "brick/window/about_window.h"
#include "brick/window/accounts_window.h"
#include "brick/event/indicator_badge_event.h"

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

}  // namespace

BrickIndicator::BrickIndicator(std::string icons_dir)
    : current_icon_(Icon::DEFAULT),
     idle_icon_(Icon::DEFAULT),
     icons_folder_(icons_dir),
     idle_(true),
     extended_status_(true),
     icon_handler_(NULL) {

  Init();
}

std::string
BrickIndicator::GetIconPath(Icon icon) {
  return icons_folder_ + icon_files_[icon];
}

std::string
BrickIndicator::GetIconName(Icon icon) {
  return icon_names_[icon];
}

void
BrickIndicator::SetBadge(int badge, bool is_important) {
  if (badge > 0) {
    if (is_important) {
        SetIcon(Icon::FLASH_IMPORTANT);
    } else {
        SetIcon(Icon::FLASH);
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
BrickIndicator::OnClick() {
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
BrickIndicator::OnMenuQuit() {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  if (!client_handler)
    return false;

  client_handler->Shutdown(false);
  return true;
}

bool
BrickIndicator::OnMenuPortalOpen() {
  return true;
}

bool
BrickIndicator::OnMenuAbout() {
  AboutWindow::Instance()->Show();
  return true;
}

bool
BrickIndicator::OnMenuManageAccount() {
  AccountsWindow::Instance()->Show();
  return true;
}

bool
BrickIndicator::OnMenuChangeAccount(int id) {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  CefRefPtr<AccountManager> account_manager = client_handler->GetAccountManager();
  if (account_manager->GetCurrentAccount()->GetId() == id)
    return true;  // Selected current account

  SetIdleIcon(Icon::DEFAULT);
  SetIcon(Icon::DEFAULT);
  client_handler->SwitchAccount(id);

  return true;
}

void
BrickIndicator::SwitchToIdle() {
  idle_ = true;
  SetIcon((Icon) idle_icon_);
}

void
BrickIndicator::UseExtendedStatus(bool use) {
  extended_status_ = use;
}

void
BrickIndicator::onEvent(const AccountListEvent& event) {
  UpdateAccountsMenu();
}

void
BrickIndicator::onEvent(const AccountSwitchEvent& event) {
  UpdateAccountsMenu();
}

void
BrickIndicator::RegisterEventListeners() {
  EventBus::AddHandler<AccountListEvent>(*this);
  EventBus::AddHandler<AccountSwitchEvent>(*this);
}
