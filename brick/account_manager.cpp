#include <third-party/json/json.h>
#include "account_manager.h"
#include "include/base/cef_logging.h"

AccountManager::AccountManager()
   : current_account_(NULL)
{
}

AccountManager::~AccountManager() {
}

void
AccountManager::AddAccount(Account* account) {
//  account_list_.push_back(account);
  if (account->IsDefault()) {
    current_account_ = account;
  }
}

Account*
AccountManager::GetCurrentAccount() {
  return current_account_;
}

// static
CefRefPtr<AccountManager>
AccountManager::CreateInstance(std::string config) {
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  CefRefPtr<AccountManager> account_manager(new AccountManager);

  bool parsingSuccessful = reader.parse(config, root);
  if (!parsingSuccessful) {
    LOG(ERROR) << "Failed to parse configuration\n"
       << reader.getFormattedErrorMessages();

    return NULL;
  }

  const Json::Value accounts = root["accounts"];
  for(unsigned int i=0; i < accounts.size(); ++i) {
    account_manager->AddAccount(new Account(
       accounts[i].get("login", "").asString(),
       accounts[i].get("password", "").asString(),
       accounts[i].get("uri", "").asString(),
       accounts[i].get("default", false).asBool()
    ));
  }

  return account_manager;
}