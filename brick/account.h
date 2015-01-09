#ifndef BRICK_ACCOUNT_H_
#define BRICK_ACCOUNT_H_
#pragma once

#include <string>
#include <include/cef_base.h>

class Account : public CefBase {

  public:
    Account();
    ~Account();

    int GetId();
    std::string GetLogin();
    std::string GetPassword();
    std::string GetDomain();
    std::string GetBaseUrl();
    std::string GetLabel();
    bool IsExisted();
    bool IsSecure();
    bool CheckBaseUrl(std::string url);

    void Set(bool secure, std::string domain, std::string login, std::string password);
    void SetLogin(std::string login);
    void SetPassword(std::string password);
    void SetDomain(std::string domain);
    void SetSecure(bool is_secure);
    void SetId(int id);

    std::string GenLabel();
    std::string GenBaseUrl();

  protected:
    int id_;
    std::string login_;
    std::string password_;
    std::string domain_;
    std::string base_url_;
    std::string label_;
    bool secure_;
  private:
    IMPLEMENT_REFCOUNTING(Account);
};

#endif // BRICK_ACCOUNT_H_
