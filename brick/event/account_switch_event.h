#ifndef _BRICK_SWITCH_EVENT_H_
#define _BRICK_SWITCH_EVENT_H_

#include "event.h"

#include <string>
#include "../account.h"

class AccountSwitchEvent : public Event
{
public:
  AccountSwitchEvent(EventSender &sender) :
     Event(sender) {
  }

  virtual ~AccountSwitchEvent() { }

private:

IMPLEMENT_REFCOUNTING(AccountSwitchEvent);
};

#endif /* _BRICK_SWITCH_EVENT_H_ */
