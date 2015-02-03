#ifndef _BRICK_USER_AWAY_EVENT_H_
#define _BRICK_USER_AWAY_EVENT_H_

#include "event.h"

class UserAwayEvent : public Event
{
public:
  UserAwayEvent(const EventSender* sender, bool is_away, bool manual=false) :
     Event(sender),
     away_(is_away),
     manual_(manual) {
  }

  UserAwayEvent(bool is_away, bool manual=false) :
     Event(nullptr),
     away_(is_away),
     manual_(manual) {
  }

  virtual ~UserAwayEvent() { }

  bool isAway() {
    return away_;
  }

  bool isManual() {
    return manual_;
  }

private:
  bool away_;
  bool manual_;

IMPLEMENT_REFCOUNTING(UserAwayEvent);
};

#endif /* _BRICK_USER_AWAY_EVENT_H_ */
