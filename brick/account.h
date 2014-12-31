#ifndef BRICK_ACCOUNT_H_
#define BRICK_ACCOUNT_H_
#pragma once

#include <string>

class Account {

  public:
    Account(std::string login, std::string password, std::string base_url, bool is_default);
    ~Account();

    std::string GetLogin();
    std::string GetPassword();
    std::string GetBaseUrl();
    bool IsDefault();
    bool IsSecure();
    bool CheckBaseUrl(std::string url);


  protected:
    std::string login_;
    std::string password_;
    std::string base_url_;
    bool default_;
    bool secure_;
};


#endif // BRICK_ACCOUNT_H_
