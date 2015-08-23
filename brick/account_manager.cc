// Copyright (c) 2015 The Brick Authors.

#include "brick/account_manager.h"

#include <sys/stat.h>
#include <fstream>

#include "third-party/json/json.h"
#include "include/base/cef_logging.h"
#include "brick/event/account_list_event.h"
#include "brick/event/event_bus.h"
#include "brick/event/account_switch_event.h"

AccountManager::AccountManager()
    : initialized_(false),
     current_account_(NULL),
     last_id_(0) {
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

  accounts_map::const_iterator it = accounts_.cbegin();
  for (; it != accounts_.cend(); ++it) {
    CefRefPtr<Account> account = (*it).second;
    Json::Value json_account(Json::objectValue);
    json_account["secure"] = account->IsSecure();
    json_account["domain"] = account->GetDomain();
    json_account["login"] = account->GetLogin();
    json_account["password"] = account->GetPassword();
    json_account["default"] = account == current_account_;
    json_account["use_app_password"] = account->IsAppPasswordUsed();
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
    for (const auto &account : accounts) {
      CefRefPtr<Account> newAccount(new Account);
      newAccount->SetLogin(account.get("login", "").asString());
      newAccount->SetPassword(account.get("password", "").asString());
      newAccount->SetSecure(account.get("secure", false).asBool());
      newAccount->SetDomain(account.get("domain", "").asString());
      newAccount->SetUseAppPassword(account.get("use_app_password", true).asBool());

      AddAccount(newAccount);
      if (account.get("default", false).asBool()) {
        SwitchAccount(newAccount->GetId());
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
