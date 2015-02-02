#ifndef _BRICK_ACCOUNT_LIST_EVENT_EVENT_H_
#define _BRICK_ACCOUNT_LIST_EVENT_EVENT_H_

#include "event.h"
#include "../account.h"

class AccountListEvent : public Event
{
public:
  AccountListEvent(const EventSender* sender) :
     Event(sender) {
  }

  AccountListEvent() :
     Event(nullptr) {
  }

  virtual ~AccountListEvent() { }

IMPLEMENT_REFCOUNTING(AccountListEvent);
};

#endif /* _BRICK_ACCOUNT_LIST_EVENT_EVENT_H_ */
