#ifndef _BRICK_EVENT_HANDLER_REGISTRATION_H_
#define _BRICK_EVENT_HANDLER_REGISTRATION_H_

#include "event_object.h"

/**
 * Interface that that allows event handlers to be removed from the EventBus
 */
class HandlerRegistration : public EventObject {
public:
	virtual ~HandlerRegistration() { }

	virtual void removeHandler() = 0;
};

#endif /* _BRICK_EVENT_HANDLER_REGISTRATION_H_ */
