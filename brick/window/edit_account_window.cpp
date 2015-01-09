#include <include/base/cef_logging.h>
#include <brick/cef_handler.h>
#include "edit_account_window.h"

void
EditAccountWindow::Save(
   bool secure,
   std::string domain,
   std::string login,
   std::string password) {

  CefRefPtr<Account> account = window_objects_.account;
  bool commit_needed = false;

  if (account->IsExisted()) {

    if (account->IsSecure() != secure) {
      account->SetSecure(secure);
      commit_needed = true;
    }

    if (account->GetDomain() != domain) {
      account->SetDomain(domain);
      commit_needed = true;
    }

    if (account->GetLogin() != login) {
      account->SetLogin(login);
      commit_needed = true;
    }

    if (account->GetPassword() != password) {
      account->SetPassword(password);
      commit_needed = true;
    }
  } else {
    commit_needed = true;
    account->SetSecure(secure);
    account->SetLogin(login);
    account->SetDomain(domain);
    account->SetPassword(password);
    ClientHandler::GetInstance()->GetAccountManager()->AddAccount(account);
  }

  if (commit_needed) {
    ClientHandler::GetInstance()->GetAccountManager()->Commit();
  }

  if (window_objects_.switch_on_save) {
    ClientHandler::GetInstance()->SwitchAccount(account->GetId());
  }

  Close();
}