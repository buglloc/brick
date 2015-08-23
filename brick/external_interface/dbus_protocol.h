// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EXTERNAL_INTERFACE_DBUS_PROTOCOL_H_
#define BRICK_EXTERNAL_INTERFACE_DBUS_PROTOCOL_H_

#include <string>
#include <set>

#include "include/cef_base.h"
#include "brick/external_interface/external_message_delegate.h"
#include "brick/event/account_list_event.h"
#include "brick/event/account_switch_event.h"
#include "brick/event/indicator_badge_event.h"
#include "brick/event/indicator_state_event.h"
#include "brick/event/indicator_tooltip_event.h"
#include "brick/event/event_handler.h"


class DBusProtocol : public CefBase,
                     public EventHandler<AccountListEvent>,
                     public EventHandler<AccountSwitchEvent>,
                     public EventHandler<IndicatorBadgeEvent>,
                     public EventHandler<IndicatorStateEvent>,
                     public EventHandler<IndicatorTooltipEvent> {
 public:
  typedef std::set<CefRefPtr<ExternalMessageDelegate> >
     ExternalMessageDelegateSet;

  DBusProtocol():
     session_bus_(NULL),
     owned_(false) {
  }

  bool Init();
  bool isSingleInstance();
  void BringAnotherInstance();
  bool Handle(std::string interface_name, CefRefPtr<CefProcessMessage> message);

  // Event handlers
  virtual void onEvent(const AccountListEvent& event) OVERRIDE;
  virtual void onEvent(const AccountSwitchEvent& event) OVERRIDE;
  virtual void onEvent(const IndicatorBadgeEvent& event) OVERRIDE;
  virtual void onEvent(const IndicatorStateEvent& event) OVERRIDE;
  virtual void onEvent(const IndicatorTooltipEvent& event) OVERRIDE;

 protected:
  void RegisterSystemListeners();
  void RegisterMessageDelegates();
  void RegisterEventListeners();

  GDBusConnection  *session_bus_;
  bool owned_;

  // Registered delegates.
  ExternalMessageDelegateSet external_message_delegates_;

IMPLEMENT_REFCOUNTING(DBusProtocol);
};

#endif  // BRICK_EXTERNAL_INTERFACE_DBUS_PROTOCOL_H_
