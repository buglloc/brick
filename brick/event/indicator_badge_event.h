#ifndef _BRICK_INDICATOR_BADGE_EVENT_H_
#define _BRICK_INDICATOR_BADGE_EVENT_H_

#include "event.h"
#include <string>

class IndicatorBadgeEvent : public Event
{
public:
  IndicatorBadgeEvent(const EventSender* sender, int badge, bool important) :
     Event(sender),
     badge_(badge),
     important_(important) {
  }

  IndicatorBadgeEvent(int badge, bool important) :
     Event(nullptr),
     badge_(badge),
     important_(important) {
  }

  virtual ~IndicatorBadgeEvent() { }

  int getBadge() {
    return badge_;
  }

  bool isImportant() {
    return important_;
  }

private:
  int badge_;
  bool important_;

IMPLEMENT_REFCOUNTING(IndicatorBadgeEvent);
};

#endif /* _BRICK_INDICATOR_BADGE_EVENT_H_ */
