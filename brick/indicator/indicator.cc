// Copyright (c) 2015 The Brick Authors.

#include "brick/indicator/indicator.h"

#include "include/cef_app.h"

#include "brick/client_handler.h"
#include "brick/window/about_window.h"
#include "brick/window/accounts_window.h"
#include "brick/event/indicator_badge_event.h"

BrickIndicator::BrickIndicator(std::string icons_dir)
    : current_icon_(IndicatorStatusIcon::DEFAULT),
     idle_icon_(IndicatorStatusIcon::DEFAULT),
     icons_folder_(icons_dir),
     idle_(true),
     extended_status_(true),
     icon_(NULL) {

#ifdef unity
  unity_launcher_ = new UnityLauncher();
#endif
  Init();
}

void
BrickIndicator::SetBadge(int badge, bool is_important) {
  if (badge > 0) {
    if (is_important) {
        SetIcon(IndicatorStatusIcon::FLASH_IMPORTANT);
    } else {
        SetIcon(IndicatorStatusIcon::FLASH);
    }
  } else {
    // restore icon for current application state
    SwitchToIdle();
  }

#ifdef unity
  unity_launcher_->SetBadge(badge);
#endif

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

  SetIdleIcon(IndicatorStatusIcon::DEFAULT);
  SetIcon(IndicatorStatusIcon::DEFAULT);
  client_handler->SwitchAccount(id);

  return true;
}

void
BrickIndicator::SwitchToIdle() {
  idle_ = true;
  SetIcon((IndicatorStatusIcon) idle_icon_);
}

void
BrickIndicator::UseExtendedStatus(bool use) {
  extended_status_ = use;
}

void
BrickIndicator::OnEvent(const AccountListEvent &event) {
  UpdateAccountsMenu();
}

void
BrickIndicator::OnEvent(const AccountSwitchEvent &event) {
  UpdateAccountsMenu();
}

void
BrickIndicator::RegisterEventListeners() {
  EventBus::AddHandler<AccountListEvent>(*this);
  EventBus::AddHandler<AccountSwitchEvent>(*this);
}
