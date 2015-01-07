#include <fstream>
#include "include/base/cef_logging.h"

#include <third-party/json/json.h>
#include "account_manager.h"

AccountManager::AccountManager()
   : current_account_(NULL),
     last_id_(0)
{
}

AccountManager::~AccountManager() {
}

bool
AccountManager::AddAccount(Account* account) {
  accounts_[++last_id_] = account;
  account->SetId(last_id_);

  if (account->IsDefault()) {
    current_account_ = account;
  }
  return true;
}

bool
AccountManager::DeleteAccount(int id) {
  if (accounts_.count(id) == 0)
    return false;

  delete accounts_[id];
  accounts_.erase(id);
  return true;
}

bool
AccountManager::SwitchAccount(int id) {
  if (accounts_.count(id) == 0)
    return false;
  current_account_->SetDefault(false);
  current_account_ = accounts_[id];
  current_account_->SetDefault(true);
  Commit();
  return true;
}


bool
AccountManager::Commit() {
  std::ofstream ofs(config_path_);
  Json::Value json(Json::objectValue);
  Json::Value json_accounts(Json::arrayValue);

  accounts_map::iterator it = accounts_.begin();
  for (; it != accounts_.end(); ++it) {
    Account *account = (*it).second;
    Json::Value json_account(Json::objectValue);
    json_account["login"] = account->GetLogin();
    json_account["password"] = account->GetPassword();
    json_account["uri"] = account->GetBaseUrl();
    json_account["default"] = account->IsDefault();
    json_accounts.append(json_account);
  }
  json["accounts"] = json_accounts;
  ofs << json;
  return true;
}

Account*
AccountManager::GetCurrentAccount() {
  return current_account_;
}

AccountManager::accounts_map*
AccountManager::GetAccounts() {
  return &accounts_;
}

void
AccountManager::Init(std::string config_path) {
  Json::Value json;   // will contains the json value after parsing.
  Json::Reader reader;
  config_path_ = config_path;
  std::ifstream ifs(config_path);
  std::string config(
     (std::istreambuf_iterator<char>(ifs) ),
     (std::istreambuf_iterator<char>()    )
  );

  bool parsingSuccessful = reader.parse(config, json);
  if (!parsingSuccessful) {
    LOG(ERROR) << "Failed to parse configuration\n"
       << reader.getFormattedErrorMessages();
    return;
  }

  const Json::Value accounts = json["accounts"];
  for(unsigned int i=0; i < accounts.size(); ++i) {
    AddAccount(new Account(
       accounts[i].get("login", "").asString(),
       accounts[i].get("password", "").asString(),
       accounts[i].get("uri", "").asString(),
       accounts[i].get("default", false).asBool()
    ));
  }
}