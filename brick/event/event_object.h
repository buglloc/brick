#ifndef _BRICK_EVENT_OBJECT_H_
#define _BRICK_EVENT_OBJECT_H_

#include <include/cef_base.h>

class EventObject : public CefBase {
public:
	/**
	 * Default empty constructor
	 */
  EventObject() { }


	/**
	 * Empty virtual destructor
	 */
	virtual ~EventObject() {	}


	/**
	 * Default empty copy constructor
	 * @param other The instance to copy from
	 */
  EventObject (const EventObject &other) { }

  IMPLEMENT_REFCOUNTING(EventObject);
};

#endif /* _BRICK_EVENT_OBJECT_H_ */