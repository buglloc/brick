#ifndef _BRICK_SWITCH_EVENT_H_
#define _BRICK_SWITCH_EVENT_H_

#include "event.h"

#include <string>
#include "../account.h"

class AccountSwitchEvent : public Event
{
public:
  AccountSwitchEvent(const EventSender* sender, const Account *account) :
     Event(sender),
     account_(account) {
  }

  AccountSwitchEvent(const Account *account) :
     Event(nullptr),
     account_(account) {
  }

  virtual ~AccountSwitchEvent() { }

  const Account *getAccount() {
    return account_;
  }

private:
  const Account *account_;

IMPLEMENT_REFCOUNTING(AccountSwitchEvent);
};

#endif /* _BRICK_SWITCH_EVENT_H_ */
