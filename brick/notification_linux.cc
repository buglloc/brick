#include <unistd.h>
#include <string>
#include <libnotify/notify.h>
#include <include/base/cef_logging.h>
#include "include/wrapper/cef_closure_task.h"
#include <include/base/cef_bind.h>

#include "notification.h"
#include "httpclient/httpclient.h"
#include "cef_handler.h"
#include "platform_util.h"

namespace {
    NotifyNotification *notification = NULL;
    int last_id = 0;

    void
    UpdateIconById(int id, std::string icon_path) {
      if (!CefCurrentlyOn(TID_UI)) {
        CefPostTask(TID_UI, base::Bind(UpdateIconById, id, icon_path));
        return;
      }

      if (notification == NULL)
        return;

      if (id < last_id)
        return;

      g_object_set(G_OBJECT(notification), "icon-name", icon_path.c_str(), NULL);

      // We must show notification again to update icon :-(
      // ToDo: Research!
      if (!notify_notification_show (notification, NULL)) {
        LOG(WARNING) << "Failed to resend notification";
      }
    }

    void
    AsyncDownloadIcon(int id, std::string url, std::string cache_path) {
      if (!CefCurrentlyOn(TID_CACHE)) {
        CefPostTask(TID_CACHE, base::Bind(AsyncDownloadIcon, id, url, cache_path));
        return;
      }

      if (HttpClient::Download(url, cache_path, "image/")) {
        UpdateIconById(id, cache_path);
      } else {
        LOG(WARNING) << "Can't download notification icon '" << url << "' to file '" << cache_path << "'";
      }
    }

    void
    CloseNotificationCb(NotifyNotification *notify) {
      notification = NULL;
    }

} //namespace

void
Notification::Notify(const std::string title, std::string body, std::string icon, int delay) {
  notify_init("brick");

  last_id++;
  bool need_download = false;
  std::string notification_icon = TryGetIcon(icon, need_download);

  if (notification == NULL) {
    notification = notify_notification_new(
       title.c_str(),
       body.c_str(),
       need_download || notification_icon.empty() ? GetDefaultIcon().c_str() : notification_icon.c_str()
    );
    g_signal_connect(notification, "closed", G_CALLBACK(CloseNotificationCb), NULL);
  } else {
    notify_notification_update(
       notification,
       title.c_str(),
       body.c_str(),
       need_download || notification_icon.empty() ? GetDefaultIcon().c_str() : notification_icon.c_str()
    );
  }

  notify_notification_set_timeout(notification, delay);
  notify_notification_set_urgency(notification, NOTIFY_URGENCY_NORMAL);

  if (need_download) {
    AsyncDownloadIcon(last_id, icon, notification_icon);
  }

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
Notification::TryGetIcon(std::string icon, bool &need_download) {
  need_download = false;
  if (icon.empty()) {
    return "";
  }

  if (icon.find("http://") != 0 && icon.find("https://") != 0) {
    return icon;
  }

  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  if (!client_handler) {
    return "";
  }

  std::string result = client_handler->GetCacheManager()->GetCachePath(icon, CacheManager::BUDDY_ICON);
  need_download = !platform_util::IsPathExists(result);
  return result;
}

std::string
Notification::GetDefaultIcon() {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  if (!client_handler)
    return "";

  return client_handler->GetAppSettings().resource_dir + "/ui/buddy.png";
}