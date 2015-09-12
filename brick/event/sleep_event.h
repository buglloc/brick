// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_SLEEP_EVENT_H_
#define BRICK_EVENT_SLEEP_EVENT_H_
#pragma once

#include "brick/event/event.h"

class SleepEvent : public Event {
 public:
  SleepEvent(const EventSender* sender, bool is_sleep) :
     Event (sender),
     sleep_ (is_sleep) {
  }

  explicit SleepEvent(bool is_sleep) :
     SleepEvent(nullptr, is_sleep)
  { }

  virtual ~SleepEvent() { }

  bool IsSleep() const {
    return sleep_;
  }

 private:
  bool sleep_;

IMPLEMENT_REFCOUNTING(SleepEvent);
};

#endif  // BRICK_EVENT_SLEEP_EVENT_H_
