#ifndef _BRICK_INDICATOR_TOOLTIP_EVENT_H_
#define _BRICK_INDICATOR_TOOLTIP_EVENT_H_


#include <string>
#include "event.h"


class IndicatorTooltipEvent : public Event
{
public:
  IndicatorTooltipEvent(const EventSender* sender, const std::string tooltip) :
     Event(sender),
     tooltip_(tooltip) {
  }

  IndicatorTooltipEvent(const std::string tooltip) :
     Event(nullptr),
     tooltip_(tooltip) {
  }

  virtual ~IndicatorTooltipEvent() { }

  const std::string getTooltip() {
    return tooltip_;
  }

private:
  const std::string tooltip_;

IMPLEMENT_REFCOUNTING(IndicatorTooltipEvent);
};

#endif /* _BRICK_INDICATOR_TOOLTIP_EVENT_H_ */
