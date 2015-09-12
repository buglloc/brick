// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_ACCOUNT_SWITCH_EVENT_H_
#define BRICK_EVENT_ACCOUNT_SWITCH_EVENT_H_
#pragma once

#include <string>

#include "brick/event/event.h"
#include "brick/account.h"

class AccountSwitchEvent : public Event {
 public:
  AccountSwitchEvent(const EventSender* sender, const Account *account) :
     Event (sender),
     account_ (account) {
  }

  explicit AccountSwitchEvent(const Account *account) :
     AccountSwitchEvent(nullptr, account)
  { }

  virtual ~AccountSwitchEvent() { }

  const Account *getAccount() {
    return account_;
  }

 private:
  const Account *account_;

IMPLEMENT_REFCOUNTING(AccountSwitchEvent);
};

#endif  // BRICK_EVENT_ACCOUNT_SWITCH_EVENT_H_
