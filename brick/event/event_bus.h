// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_EVENT_BUS_H_
#define BRICK_EVENT_EVENT_BUS_H_
#pragma once

#include <list>
#include <typeinfo>
#include <unordered_map>

#include "brick/event/event_object.h"
#include "brick/event/event_handler.h"
#include "brick/event/event.h"
#include "brick/event/handler_registration.h"

/**
 * An Event system that allows decoupling of code through synchronous events
 */
class EventBus : public EventObject {
 public:

  EventBus() { }
  virtual ~EventBus() { }


  /**
   * Creates a new instance of the EventBus if hasn't already been created
   *
   * @return The singleton instance
   */
  static EventBus* const GetInstance() {
    if (instance == nullptr) {
      instance = new EventBus();
    }

    return instance;
  }


  /**
   * Registers a new event handler to the EventBus with a source specifier
   *
   * The template parameter is the specific type of event that is being added. Since a class can
   * potentially inherit multiple event handlers, the template specifier will remove any ambiguity
   * as to which handler pointer is being referenced.
   *
   * @param handler The event handler class
   * @param sender The source sender object
   * @return An EventRegistration pointer which can be used to unregister the event handler
   */
  template <class T>
  static HandlerRegistration* const AddHandler(EventHandler<T> &handler, EventSender &sender) {
    EventBus* instance = GetInstance();

    // Fetch the list of event pairs unique to this event type
    const char * a = typeid(T).name();
    Registrations* registrations = instance->handlers[typeid(T)];

    // Create a new collection instance for this type if it hasn't been created yet
    if (registrations == nullptr) {
      registrations = new Registrations();
      instance->handlers[typeid(T)] = registrations;
    }

    // Create a new EventPair instance for this registration.
    // This will group the handler, sender, and registration object into the same class
    EventRegistration* registration = new EventRegistration(static_cast<void*>(&handler), registrations, &sender);

    // Add the registration object to the collection
    registrations->push_back(registration);

    return registration;
  }


  /**
   * Registers a new event handler to the EventBus with no source specified
   *
   * @param handler The event handler class
   * @return An EventRegistration pointer which can be used to unregister the event handler
   */
  template <class T>
  static HandlerRegistration* const AddHandler(EventHandler<T> &handler) {

    EventBus* instance = GetInstance();

    // Fetch the list of event pairs unique to this event type
    Registrations* registrations = instance->handlers[typeid(T)];

    // Create a new collection instance for this type if it hasn't been created yet
    if (registrations == nullptr) {
      registrations = new Registrations();
      instance->handlers[typeid(T)] = registrations;
    }

    // Create a new EventPair instance for this registration.
    // This will group the handler, sender, and registration object into the same class
    EventRegistration* registration = new EventRegistration(static_cast<void*>(&handler), registrations, nullptr);

    // Add the registration object to the collection
    registrations->push_back(registration);

    return registration;
  }


  /**
   * Fires an event
   *
   * @param e The event to fire
   */
  static void FireEvent(const Event &e) {
    EventBus* instance = GetInstance();

    Registrations* registrations = instance->handlers[typeid(e)];

    // If the registrations list is null, then no handlers have been registered for this event
    if (registrations == nullptr) {
      return;
    }

    // Iterate through all the registered handlers and dispatch to each one if the sender
    // matches the source or if the sender is not specified
    for (auto &reg : *registrations) {
      if (
         (reg->getSender() == nullptr)
         || (e.getSender() == nullptr)
         || (reg->getSender() == e.getSender())
         ) {

        // This is where some magic happens. The void * handler is statically cast to an event handler
        // of generic type Event and dispatched. The dispatch function will then do a dynamic
        // cast to the correct event type so the matching onEvent method can be called
        static_cast<EventHandler<Event>*>(reg->getHandler())->dispatch(e);
      }
    }
  }


 private:
  // Singleton class instance
  static EventBus* instance;


  /**
   * Registration class private to EventBus for registered event handlers
   */
  class EventRegistration : public HandlerRegistration {
   public:
    typedef std::list<EventRegistration*> Registrations;


    /**
     * Represents a registration object for a registered event handler
     *
     * This object is stored in a collection with other handlers for the event type.
     *
     * @param handler The event handler
     * @param registrations The handler collection for this event type
     * @param sender The registered sender object
     */
    EventRegistration(void * const handler, Registrations * const registrations, EventSender * const sender ) :
      handler(handler),
      registrations(registrations),
      sender(sender),
      registered(true)
    { }


    /**
     * Empty virtual destructor
     */
    virtual ~EventRegistration() { }


    /**
     * Gets the event handler for this registration
     *
     * @return The event handler
     */
    void * const getHandler() {
      return handler;
    }


    /**
     * Gets the sender object for this registration
     *
     * @return The registered sender object
     */
    EventSender* const getSender() {
      return sender;
    }


    /**
     * Removes an event handler from the registration collection
     *
     * The event handler will no longer receive events for this event type
     */
    virtual void removeHandler() {
      if (registered) {
        registrations->remove(this);
        registered = false;
      }
    }

   private:
    void * const handler;
    Registrations* const registrations;
    EventSender* const sender;

    bool registered;
  };

  typedef std::list<EventRegistration*> Registrations;
  typedef std::unordered_map<std::type_index, std::list<EventRegistration*>*> TypeMap;

  TypeMap handlers;

};

#endif  // BRICK_EVENT_EVENT_BUS_H_
