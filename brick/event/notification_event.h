// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_NOTIFICATION_EVENT_H_
#define BRICK_EVENT_NOTIFICATION_EVENT_H_
#pragma once

#include <string>

#include "brick/event/event.h"

class NotificationEvent : public Event {
 public:
  NotificationEvent(const EventSender* sender, const std::string& id, const bool is_message, const bool show_action) :
     Event (sender),
     id_ (id),
     is_show_action_ (show_action),
     is_message_ (is_message) {
  }

  NotificationEvent(const std::string& id, const bool is_message, const bool show_action) :
     NotificationEvent(nullptr, id, show_action, is_message)
  { }

  virtual ~NotificationEvent() { }

  const std::string& GetId() const {
    return id_;
  }

  int IsShowAction() const {
    return is_show_action_;
  }

  int IsMessage() const {
   return is_message_;
  }

 private:
  const std::string id_;
  const bool is_show_action_;
  const bool is_message_;

IMPLEMENT_REFCOUNTING(NotificationEvent);
};

#endif  // BRICK_EVENT_NOTIFICATION_EVENT_H_
