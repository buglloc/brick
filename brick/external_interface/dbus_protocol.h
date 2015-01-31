#ifndef BRICK_DBUS_PROTOCOL_H_
#define BRICK_DBUS_PROTOCOL_H_

#include <include/cef_base.h>
#include <set>
#include "external_message_delegate.h"

class DBusProtocol : public CefBase {
public:
  typedef std::set<CefRefPtr<ExternalMessageDelegate> >
     ExternalMessageDelegateSet;

  DBusProtocol();
  ~DBusProtocol();

  char Init(bool send_show_on_exists = true);
  bool Handle(std::string interface_name, CefRefPtr<CefProcessMessage> message);

protected:
  void RegisterMessageDelegates();
  GDBusConnection  *connection_;
  guint own_id_;

  // Registered delegates.
  ExternalMessageDelegateSet external_message_delegates_;

IMPLEMENT_REFCOUNTING(DBusProtocol);
};

#endif // BRICK_DBUS_PROTOCOL_H_