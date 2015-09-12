// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_INDICATOR_BADGE_EVENT_H_
#define BRICK_EVENT_INDICATOR_BADGE_EVENT_H_
#pragma once

#include <string>

#include "brick/event/event.h"

class IndicatorBadgeEvent : public Event {
 public:
  IndicatorBadgeEvent(const EventSender* sender, int badge, bool important) :
     Event (sender),
     badge_ (badge),
     important_ (important) {
  }

  explicit IndicatorBadgeEvent(int badge, bool important) :
     IndicatorBadgeEvent(nullptr, badge, important)
  { }

  virtual ~IndicatorBadgeEvent() { }

  int GetBadge() const {
    return badge_;
  }

  bool IsImportant() const {
    return important_;
  }

 private:
  int badge_;
  bool important_;

IMPLEMENT_REFCOUNTING(IndicatorBadgeEvent);
};

#endif  // BRICK_EVENT_INDICATOR_BADGE_EVENT_H_
