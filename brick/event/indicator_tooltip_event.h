#ifndef _BRICK_INDICATOR_TOOLTIP_EVENT_H_
#define _BRICK_INDICATOR_TOOLTIP_EVENT_H_


#include <string>
#include "event.h"


class IndicatorTooltipEvent : public Event
{
public:
  IndicatorTooltipEvent(EventSender &sender, std::string const &tooltip) :
     Event(sender),
     tooltip_(tooltip) {
  }

  virtual ~IndicatorTooltipEvent() { }

  std::string const &getTooltip() {
    return tooltip_;
  }

private:
  std::string const &tooltip_;

IMPLEMENT_REFCOUNTING(IndicatorTooltipEvent);
};

#endif /* _BRICK_INDICATOR_TOOLTIP_EVENT_H_ */
