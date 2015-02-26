#include <unistd.h>
#include <string>
#include <libnotify/notify.h>
#include <include/base/cef_logging.h>
#include <ksba.h>

#include "notification.h"
#include "httpclient/httpclient.h"
#include "cef_handler.h"

namespace {
    NotifyNotification *notification = NULL;
} //namespace

void
Notification::Notify(const std::string title, std::string body, std::string icon, int delay) {
  notify_init ("brick");

  std::string notification_icon;
  if (!icon.empty()) {
    notification_icon = TryGetIcon(icon);
  } else {
    notification_icon = GetDefaultIcon();
  }

  if (notification == NULL) {
    notification = notify_notification_new(
       title.c_str(),
       body.c_str(),
       notification_icon.empty() ? "notification-message-IM" : notification_icon.c_str()
    );
  } else {
    notify_notification_update(
       notification,
       title.c_str(),
       body.c_str(),
       notification_icon.empty() ? "notification-message-IM" : notification_icon.c_str()
    );
  }

  notify_notification_set_timeout(notification, delay);
  notify_notification_set_urgency(notification, NOTIFY_URGENCY_NORMAL);

  if (!notify_notification_show (notification, NULL)) {
    LOG(WARNING) << "Failed to send notification";
  }
}

void
Notification::Hide() {
  if (notification == NULL)
    return;

  if (!notify_notification_close(notification, NULL))
    LOG(WARNING) << "Failed to close notification";

  g_object_unref (G_OBJECT(notification));
  notification = NULL;
}

std::string
Notification::TryGetIcon(std::string icon) {
  std::string result;

  if (icon.find("http://") == 0 || icon.find("https://") == 0) {
    result = HttpClient::GetCached(icon, CacheManager::TYPE::BUDDY_ICON);
  } else {
    result = icon;
  }

  return result.empty()? GetDefaultIcon() : "file://" + result;
}

std::string
Notification::GetDefaultIcon() {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  if (!client_handler)
    return "";

  return "file://" + client_handler->GetAppSettings().resource_dir + "/ui/buddy.png";
}