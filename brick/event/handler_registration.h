// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_HANDLER_REGISTRATION_H_
#define BRICK_EVENT_HANDLER_REGISTRATION_H_
#pragma once

#include "brick/event/event_object.h"

class HandlerRegistration : public EventObject {
 public:
  virtual ~HandlerRegistration() { }

  virtual void RemoveHandler() = 0;
};

#endif  // BRICK_EVENT_HANDLER_REGISTRATION_H_
