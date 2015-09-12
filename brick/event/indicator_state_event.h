// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_INDICATOR_STATE_EVENT_H_
#define BRICK_EVENT_INDICATOR_STATE_EVENT_H_
#pragma once

#include <string>

#include "brick/event/event.h"

class IndicatorStateEvent : public Event {
 public:
  IndicatorStateEvent(const EventSender* sender, const std::string state) :
     Event (sender),
     state_ (state) {
  }

  explicit IndicatorStateEvent(const std::string state) :
     IndicatorStateEvent(nullptr, state)
  { }

  virtual ~IndicatorStateEvent() { }

  const std::string GetState() const {
    return state_;
  }

 private:
  const std::string state_;

IMPLEMENT_REFCOUNTING(IndicatorStateEvent);
};

#endif  // BRICK_EVENT_INDICATOR_STATE_EVENT_H_
