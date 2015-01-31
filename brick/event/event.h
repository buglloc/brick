#ifndef _BRICK_EVENT_H_
#define _BRICK_EVENT_H_

#include "event_object.h"
#include "event_sender.h"

#include <typeindex>
#include <typeinfo>
#include <vector>
#include <stdexcept>

/**
 * The base event class, all events inherit from this class
 */
class Event : public EventObject
{
public:
	/**
	 * Default constructor
	 *
	 * @param typeIndex The type ID of the inherited class
	 * @param sender The sender of the event
	 */
	Event(EventSender &sender) :
		sender(sender),
		canceled(false) {
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
  EventSender &getSender() {
		return sender;
	}


	/**
	 * Gets whether the event has been canceled
	 *
	 * @return true if the event is canceled
	 */
	bool getCanceled() {
		return canceled;
	}


	/**
	 * Sets the canceled status for the event
	 *
	 * @param canceled Whether the even is canceled or not
	 */
	void setCanceled(bool canceled) {
		this->canceled = canceled;
	}

private:
  EventSender &sender;
	bool canceled;

};

#endif /* _BRICK_EVENT_H_ */
