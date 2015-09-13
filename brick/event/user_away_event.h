// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_USER_AWAY_EVENT_H_
#define BRICK_EVENT_USER_AWAY_EVENT_H_
#pragma once

#include "brick/event/event.h"

class UserAwayEvent : public Event {
 public:
  UserAwayEvent(const EventSender* sender, bool is_away, bool manual = false) :
     Event (sender),
     away_ (is_away),
     manual_ (manual) {
  }

  explicit UserAwayEvent(bool is_away, bool manual = false) :
      Event (nullptr),
      away_ (is_away),
      manual_ (manual) {
  }

  virtual ~UserAwayEvent() { }

  bool IsAway() const {
    return away_;
  }

  bool IsManual() const {
    return manual_;
  }

 private:
  bool away_;
  bool manual_;

IMPLEMENT_REFCOUNTING(UserAwayEvent);
};

#endif  // BRICK_EVENT_USER_AWAY_EVENT_H_
