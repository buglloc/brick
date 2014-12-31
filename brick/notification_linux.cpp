#include <unistd.h>
#include <string>
#include <libnotify/notify.h>

#include "notification.h"

namespace {
    NotifyNotification *notification = NULL;
} //namespace

//static gboolean
//close_handler(NotifyNotification *n) {
////  g_object_unref (G_OBJECT(n));
//  notification = NULL;
//  return FALSE;
//}

void
Notification::Notify(const std::string title, std::string body, int delay) {
 notify_init ("brick");
  if (notification == NULL) {
    notification = notify_notification_new(
       title.c_str(),
       body.c_str(),
       "notification-message-IM"
    );
  } else {
    notify_notification_update(
       notification,
       title.c_str(),
       body.c_str(),
       "notification-message-IM"
    );
  }

  notify_notification_set_timeout(notification, delay);
  notify_notification_set_urgency(notification, NOTIFY_URGENCY_NORMAL);

  if (!notify_notification_show (notification, NULL)) {
    fprintf (stderr, "failed to send notification\n");
  }
}

void
Notification::Hide() {
  if (notification == NULL)
    return;

  notify_notification_close(notification, NULL);
  g_object_unref (G_OBJECT(notification));
  notification = NULL;
}