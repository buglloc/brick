// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_EVENT_HANDLER_H_
#define BRICK_EVENT_EVENT_HANDLER_H_
#pragma once

#include <typeinfo>
#include <type_traits>

#include "brick/event/event_object.h"

// Forward declare the Event class
class Event;

/**
 * For a class to be an event listener, it needs to inherit from EventHandler
 * with the specific event type as the template parameter. A class can inherit from
 * multiple EventHandler base classes each using a different template parameter.
 */
template <class T>
class EventHandler {
 public:

  EventHandler() {
    // An error here indicates you're trying to implement EventHandler with a type that is not derived from Event
    static_assert(std::is_base_of<Event, T>::value, "EventHandler<T>: T must be a class derived from Event");
  }

  virtual ~EventHandler() { }


  /**
   * @param The event instance
   */
  virtual void onEvent(const T &) = 0;


  /**
   * This method is called by the EventBus and dispatches to the correct method by
   * dynamic casting the event parameter to the template type for this handler.
   *
   * @param e The event to dispatch
   */
  void dispatch(const Event &e) {
    onEvent(dynamic_cast<const T &>(e));
  }
};

#endif  // BRICK_EVENT_EVENT_HANDLER_H_
