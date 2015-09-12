// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_INDICATOR_TOOLTIP_EVENT_H_
#define BRICK_EVENT_INDICATOR_TOOLTIP_EVENT_H_
#pragma once

#include <string>

#include "brick/event/event.h"

class IndicatorTooltipEvent : public Event {
 public:
  IndicatorTooltipEvent(const EventSender* sender, const std::string tooltip) :
     Event (sender),
     tooltip_ (tooltip) {
  }

  explicit IndicatorTooltipEvent(const std::string tooltip) :
     Event (nullptr),
     tooltip_ (tooltip) {
  }

  virtual ~IndicatorTooltipEvent() { }

  const std::string GetTooltip() const {
    return tooltip_;
  }

 private:
  const std::string tooltip_;

IMPLEMENT_REFCOUNTING(IndicatorTooltipEvent);
};

#endif  // BRICK_EVENT_INDICATOR_TOOLTIP_EVENT_H_
