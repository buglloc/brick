// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_EVENT_OBJECT_H_
#define BRICK_EVENT_EVENT_OBJECT_H_
#pragma once

#include "include/cef_base.h"

class EventObject : public CefBase {
 public:
	/**
	 * Default empty constructor
	 */
  EventObject() { }


	/**
	 * Empty virtual destructor
	 */
  virtual ~EventObject() { }


	/**
	 * Default empty copy constructor
	 * @param other The instance to copy from
	 */
  EventObject (const EventObject &other) { }

  IMPLEMENT_REFCOUNTING(EventObject);
};

#endif  // BRICK_EVENT_EVENT_OBJECT_H_
