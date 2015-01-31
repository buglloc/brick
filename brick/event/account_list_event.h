#ifndef _BRICK_ACCOUNT_LIST_EVENT_EVENT_H_
#define _BRICK_ACCOUNT_LIST_EVENT_EVENT_H_

#include "event.h"

class AccountListEvent : public Event
{
public:
  AccountListEvent(EventSender &sender) : Event(sender) {
  }

  virtual ~AccountListEvent() { }

IMPLEMENT_REFCOUNTING(AccountListEvent);
};

#endif /* _BRICK_ACCOUNT_LIST_EVENT_EVENT_H_ */
