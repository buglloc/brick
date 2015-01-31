#ifndef _BRICK_INDICATOR_STATE_EVENT_H_
#define _BRICK_INDICATOR_STATE_EVENT_H_

#include <string>
#include "event.h"


class IndicatorStateEvent : public Event
{
public:
  IndicatorStateEvent(EventSender &sender, std::string const &state) :
     Event(sender),
     state_(state) {
  }

  virtual ~IndicatorStateEvent() { }

  std::string const &getState() {
    return state_;
  }

private:
  std::string const &state_;

IMPLEMENT_REFCOUNTING(IndicatorStateEvent);
};

#endif /* _BRICK_INDICATOR_STATE_EVENT_H_ */
