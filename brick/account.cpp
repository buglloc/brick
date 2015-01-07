#include "account.h"

Account::Account(std::string login, std::string password, std::string base_url, bool is_default, int id) {
  id_ = id;
  login_ = login;
  password_ = password;
  base_url_ = base_url;
  label_ = login;
  default_ = is_default;
  secure_ = base_url.find("https://") == 0;
}

Account::~Account() {
}

bool
Account::IsDefault() {
  return default_;
}

bool
Account::IsSecure() {
  return secure_;
}

int
Account::GetId() {
  return id_;
}

void
Account::SetId(int id) {
  id_ = id;
}

std::string
Account::GetLogin() {
  return login_;
}

std::string
Account::GetPassword() {
  return password_;
}

std::string
Account::GetBaseUrl() {
  return base_url_;
}

std::string
Account::GetLabel() {
  return label_;
}

bool
Account::CheckBaseUrl(std::string url) {
  return (url.find(base_url_) == 0);
}