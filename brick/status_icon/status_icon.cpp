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

bool
StatusIcon::OnClick() {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  if (!client_handler)
    return false;

  CefRefPtr<MainWindow> window = client_handler->GetMainWindowHandle();
  if (!window)
    return false;

  if (window->GetHided()) {
    window->Present();
  } else {
    window->Hide();
  }

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