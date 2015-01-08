#ifndef BRICK_ACCOUNT_H_
#define BRICK_ACCOUNT_H_
#pragma once

#include <string>
#include <include/cef_base.h>

class Account : public CefBase {

  public:
    Account(std::string login, std::string password, std::string base_url, bool is_default, int id = 0);
    ~Account();

    int GetId();
    std::string GetLogin();
    std::string GetPassword();
    std::string GetBaseUrl();
    std::string GetLabel();
    void SetDefault(bool is_default);
    bool IsDefault();
    bool IsSecure();
    bool CheckBaseUrl(std::string url);

    void SetId(int id);


  protected:
    int id_;
    std::string login_;
    std::string password_;
    std::string base_url_;
    std::string label_;
    bool default_;
    bool secure_;
  private:
    IMPLEMENT_REFCOUNTING(Account);
};

#endif // BRICK_ACCOUNT_H_
