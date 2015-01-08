#include <brick/cef_handler.h>
#include "edit_account_window.h"

void
EditAccountWindow::Save(
   std::string domain,
   std::string login,
   std::string password) {

  CefRefPtr<Account> account = window_objects_.account;
  if (account->IsExisted()) {
      if (account->GetLogin() != login)
        account->SetLogin(login);

      if (account->GetDomain() != domain)
        account->SetDomain(domain);

      if (account->GetPassword() != password)
        account->SetPassword(domain);
  } else {
    account->SetLogin(login);
    account->SetDomain(domain);
    account->SetPassword(domain);
    ClientHandler::GetInstance()->GetAccountManager()->AddAccount(account);
    ClientHandler::GetInstance()->GetAccountManager()->Commit();
  }

  Close();
}