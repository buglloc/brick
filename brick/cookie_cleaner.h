#ifndef BRICK_COOKIE_CLEANER_H
#define BRICK_COOKIE_CLEANER_H

#include <include/cef_cookie.h>

class CookieCleaner : public CefCookieVisitor {
public:

  CookieCleaner() : target_domain_ ("") {};

  bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie) OVERRIDE;
  void SetTargetDomain(std::string domain);

protected:
  std::string target_domain_;

  IMPLEMENT_REFCOUNTING(CookieCleaner);
};

#endif //BRICK_COOKIE_CLEANER_H
