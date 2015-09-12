// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_EVENT_H_
#define BRICK_EVENT_EVENT_H_
#pragma once

#include <typeindex>
#include <typeinfo>
#include <vector>
#include <stdexcept>

#include "brick/event/event_object.h"
#include "brick/event/event_sender.h"

/**
 * The base event class, all events inherit from this class
 */
class Event : public EventObject {
 public:
  /**
   * Default constructor
   *
   * @param typeIndex The type ID of the inherited class
   * @param sender The sender of the event
   */
  explicit Event(const EventSender* sender) :
    sender (sender),
    canceled (false) {
  }

  Event() :
     sender (nullptr),
     canceled (false) {
  }


  /**
   * Empty virtual destructor
   */
  virtual ~Event() { }


  /**
   * Gets the source object for this event
   *
   * @return The event sender
   */
  const EventSender* GetSender() const {
    return sender;
  }


  /**
   * Gets whether the event has been canceled
   *
   * @return true if the event is canceled
   */
  bool GetCanceled() {
    return canceled;
  }


  /**
   * Sets the canceled status for the event
   *
   * @param canceled Whether the even is canceled or not
   */
  void SetCanceled(bool canceled) {
    this->canceled = canceled;
  }

 private:
  const EventSender* sender;
  bool canceled;

};

#endif  // BRICK_EVENT_EVENT_H_
