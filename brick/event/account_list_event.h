// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_ACCOUNT_LIST_EVENT_H_
#define BRICK_EVENT_ACCOUNT_LIST_EVENT_H_
#pragma once

#include "brick/event/event.h"
#include "brick/account.h"

class AccountListEvent : public Event {
 public:
  explicit AccountListEvent(const EventSender* sender) :
     Event(sender) {
  }

  AccountListEvent() :
     Event(nullptr) {
  }

  virtual ~AccountListEvent() { }

IMPLEMENT_REFCOUNTING(AccountListEvent);
};

#endif  // BRICK_EVENT_ACCOUNT_LIST_EVENT_H_
