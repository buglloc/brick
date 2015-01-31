#ifndef _BRICK_STATUS_BADGE_EVENT_H_
#define _BRICK_STATUS_BADGE_EVENT_H_

#include "event.h"

class SleepEvent : public Event
{
public:
  SleepEvent(EventSender &sender, bool is_sleep) :
     Event(sender),
     sleep_(is_sleep) {
  }

  virtual ~SleepEvent() { }

  bool isSleep() {
    return sleep_;
  }

private:
  bool sleep_;
IMPLEMENT_REFCOUNTING(SleepEvent);
};

#endif /* _BRICK_STATUS_BADGE_EVENT_H_ */
