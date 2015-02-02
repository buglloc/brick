#ifndef _BRICK_INDICATOR_STATE_EVENT_H_
#define _BRICK_INDICATOR_STATE_EVENT_H_

#include <string>
#include "event.h"


class IndicatorStateEvent : public Event
{
public:
  IndicatorStateEvent(const EventSender* sender, const std::string state) :
     Event(sender),
     state_(state) {
  }

  IndicatorStateEvent(const std::string state) :
     Event(nullptr),
     state_(state) {
  }

  virtual ~IndicatorStateEvent() { }

  const std::string getState() {
    return state_;
  }

private:
  const std::string state_;

IMPLEMENT_REFCOUNTING(IndicatorStateEvent);
};

#endif /* _BRICK_INDICATOR_STATE_EVENT_H_ */
