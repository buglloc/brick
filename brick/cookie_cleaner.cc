#include "cookie_cleaner.h"

bool
CookieCleaner::Visit(
   const CefCookie& cookie,
   int count,
   int total,
   bool& deleteCookie) {

  if (target_domain_.empty()) {
    deleteCookie = true;
  } else {
    const std::string domain = CefString(&cookie.domain);
    deleteCookie = (target_domain_ == domain);
  }

  return true;
}

void
CookieCleaner::SetTargetDomain(std::string domain) {
  target_domain_ = domain;
}