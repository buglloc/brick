#ifndef BRICK_DBUS_PROTOCOL_H_
#define BRICK_DBUS_PROTOCOL_H_

#include <include/cef_base.h>
#include <set>
#include "external_message_delegate.h"
#include "../event/account_list_event.h"
#include "../event/account_switch_event.h"
#include "../event/indicator_badge_event.h"
#include "../event/indicator_state_event.h"
#include "../event/indicator_tooltip_event.h"
#include "../event/event_handler.h"


class DBusProtocol :
   public CefBase,
   public EventHandler<AccountListEvent>,
   public EventHandler<AccountSwitchEvent>,
   public EventHandler<IndicatorBadgeEvent>,
   public EventHandler<IndicatorStateEvent>,
   public EventHandler<IndicatorTooltipEvent> {
public:
  typedef std::set<CefRefPtr<ExternalMessageDelegate> >
     ExternalMessageDelegateSet;

  DBusProtocol():
     connection_(NULL),
     owned_(false) {

  }

  ~DBusProtocol() {}

  bool Init();
  bool isSingleInstance();
  void BringAnotherInstance();
  bool Handle(std::string interface_name, CefRefPtr<CefProcessMessage> message);

  // Event handlers
  virtual void onEvent(AccountListEvent &event) OVERRIDE;
  virtual void onEvent(AccountSwitchEvent &event) OVERRIDE;
  virtual void onEvent(IndicatorBadgeEvent &event) OVERRIDE;
  virtual void onEvent(IndicatorStateEvent &event) OVERRIDE;
  virtual void onEvent(IndicatorTooltipEvent &event) OVERRIDE;

protected:
  void RegisterMessageDelegates();
  void RegisterEventListeners();

  GDBusConnection  *connection_;
  bool owned_;

  // Registered delegates.
  ExternalMessageDelegateSet external_message_delegates_;

IMPLEMENT_REFCOUNTING(DBusProtocol);
};

#endif // BRICK_DBUS_PROTOCOL_H_