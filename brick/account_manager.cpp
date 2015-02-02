#include <fstream>
#include "include/base/cef_logging.h"

#include <third-party/json/json.h>
#include <sys/stat.h>
#include "event/account_list_event.h"
#include "event/event_bus.h"
#include "event/account_switch_event.h"
#include "account_manager.h"

AccountManager::AccountManager()
   : initialized_(false),
     current_account_(NULL),
     last_id_(0)
{
}

AccountManager::~AccountManager() {
}

bool
AccountManager::AddAccount(const CefRefPtr<Account> account) {
  accounts_[++last_id_] = account;
  account->SetId(last_id_);
  if (initialized_) {
    AccountListEvent e;
    EventBus::FireEvent(e);
  }

  return true;
}

bool
AccountManager::DeleteAccount(int id) {
  if (accounts_.count(id) == 0)
    return false;

  accounts_.erase(id);
  if (initialized_) {
    AccountListEvent e;
    EventBus::FireEvent(e);
  }

  return true;
}

bool
AccountManager::SwitchAccount(int id) {
  if (accounts_.count(id) == 0)
    return false;

  current_account_ = accounts_[id];

  AccountSwitchEvent e(current_account_);
  EventBus::FireEvent(e);
  return true;
}

CefRefPtr<Account>
AccountManager::GetById(int id) {
  if (accounts_.count(id) == 0) {
    LOG(WARNING) << "Non existent account: " << id;
    return NULL;
  }

  return accounts_[id];
}

bool
AccountManager::Commit() {
  Json::Value json(Json::objectValue);
  Json::Value json_accounts(Json::arrayValue);

  accounts_map::iterator it = accounts_.begin();
  for (; it != accounts_.end(); ++it) {
    CefRefPtr<Account> account = (*it).second;
    Json::Value json_account(Json::objectValue);
    json_account["secure"] = account->IsSecure();
    json_account["domain"] = account->GetDomain();
    json_account["login"] = account->GetLogin();
    json_account["password"] = account->GetPassword();
    json_account["default"] = account == current_account_;
    json_accounts.append(json_account);
  }
  json["accounts"] = json_accounts;

  std::ofstream ofs(config_path_);
  ofs << json;
  chmod(config_path_.c_str(), S_IRUSR|S_IWUSR);
  return true;
}

CefRefPtr<Account>
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

  if (!config.empty()) {
    bool parsingSuccessful = reader.parse(config, json);
    if (!parsingSuccessful) {
      LOG(ERROR) << "Failed to parse configuration\n"
         << reader.getFormattedErrorMessages();
      return;
    }

    const Json::Value accounts = json["accounts"];
    for(unsigned int i=0; i < accounts.size(); ++i) {
      CefRefPtr<Account> account(new Account);
      account->SetLogin(accounts[i].get("login", "").asString());
      account->SetPassword(accounts[i].get("password", "").asString());
      account->SetSecure(accounts[i].get("secure", false).asBool());
      account->SetDomain(accounts[i].get("domain", "").asString());

      AddAccount(account);
      if (accounts[i].get("default", false).asBool()) {
        SwitchAccount(account->GetId());
      }
    }
  } else {
    LOG(INFO) << "Empty accounts config: " << config_path;
  }

  if (!accounts_.size()) {
    // If we don't have any accounts - lets fake current account
    current_account_ = new Account;
    current_account_->SetSecure(true);
    current_account_->SetDomain("brick.internal");
  }

  if (current_account_ == NULL) {
    SwitchAccount(1);
  }

  initialized_ = true;
}