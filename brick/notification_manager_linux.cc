// Copyright (c) 2015 The Brick Authors.

#include "notification_manager.h"

#include <unistd.h>
#include <libnotify/notify.h>

#include <string>

#include "include/base/cef_logging.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_bind.h"
#include "brick/client_handler.h"
#include "brick/platform_util.h"
#include "brick/avatar_client.h"

namespace {
  const char kAppendCapability[] = "x-canonical-append";
  const char kActionsCapability[] = "actions";
  const char kDefaultActionName[] = "default";

  void
  OnCloseNotification(NotifyNotification *notify, NotificationManager *self) {
    self->OnClose();
  }

  void
  OnAction(NotifyNotification *notify, char *action, NotificationManager *self) {
    self->OnClick();
  }

}  // namespace

void
NotificationManager::Notify(const std::string title, std::string body, std::string icon, int delay) {
  notify_init("brick");

  last_id_++;
  bool need_download = false;
  std::string notification_icon = TryGetIcon(icon, need_download);

  if (notification_ == NULL || is_append_supported_) {
    notification_ = notify_notification_new(
       title.c_str(),
       body.c_str(),
       need_download || notification_icon.empty() ? GetDefaultIcon().c_str() : notification_icon.c_str()
    );

    if (is_append_supported_) {
      notify_notification_set_hint_string(notification_, kAppendCapability, "1");
    }

    if (is_actions_supported_) {
      notify_notification_add_action(
        notification_,
        kDefaultActionName,
        "Show",
        (NotifyActionCallback) OnAction,
        this,
        NULL
      );
    }

    g_signal_connect(notification_, "closed", G_CALLBACK(OnCloseNotification), this);
  } else {
    /* if the notification server supports x-canonical-append, it is
   better to not use notify_notification_update to avoid
   overwriting the current notification message */
    notify_notification_update(
       notification_,
       title.c_str(),
       body.c_str(),
       need_download || notification_icon.empty() ? GetDefaultIcon().c_str() : notification_icon.c_str()
    );
  }

  notify_notification_set_timeout(notification_, delay);
  notify_notification_set_urgency(notification_, NOTIFY_URGENCY_NORMAL);

  if (need_download) {
    AsyncDownloadIcon(last_id_, icon, notification_icon);
  }

  if (!notify_notification_show (notification_, NULL)) {
    LOG(WARNING) << "Failed to send notification";
  }
}

void
NotificationManager::Hide() {
  if (notification_ == NULL)
    return;

  if (!notify_notification_close(notification_, NULL))
    LOG(WARNING) << "Failed to close notification";

  g_object_unref (G_OBJECT(notification_));
  notification_ = NULL;
}

std::string
NotificationManager::TryGetIcon(std::string icon, bool &need_download) {
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
NotificationManager::GetDefaultIcon() {
  CefRefPtr<ClientHandler> client_handler = ClientHandler::GetInstance();
  if (!client_handler)
    return "";

  return client_handler->GetAppSettings().resource_dir + "/ui/buddy.png";
}

void
NotificationManager::UpdateIcon(int id, std::string icon_path, bool success) {
  if (notification_ == NULL)
    return;

  if (!success)
    return;

  if (id < last_id_)
    return;

  g_object_set(G_OBJECT(notification_), "icon-name", icon_path.c_str(), NULL);

  // We must show notification again to update icon :-(
  // ToDo: Research!
  if (!notify_notification_show (notification_, NULL)) {
    LOG(WARNING) << "Failed to resend notification";
  }
}

void
NotificationManager::AsyncDownloadIcon(int id, const std::string& url, const std::string& path) {
  if (!CefCurrentlyOn(TID_UI)) {
    CefPostTask(TID_UI, base::Bind(&NotificationManager::AsyncDownloadIcon, this, id, url, path));
    return;
  }

  AvatarClient::CreateRequest(
      base::Bind(&NotificationManager::UpdateIcon, this, id, path),
      url,
      path
  );
}

void
NotificationManager::InitializeCapabilities() {
  // Fetch capabilities
  GList *capabilities = notify_get_server_caps ();
  for (auto c = capabilities; c != NULL; c = g_list_next(c)) {
    char * cap = static_cast<char*>(c->data);
    if (strcmp(cap, kAppendCapability) == 0) {
      LOG(INFO) << "Notification server supports " << kAppendCapability;
      is_append_supported_ = true;
    } else if (strcmp(cap, kActionsCapability) == 0) {
      LOG(INFO) << "Notification server supports " << kActionsCapability;
      is_actions_supported_ = true;
    }
  }
  g_list_foreach(capabilities, (GFunc)g_free, NULL);
  g_list_free(capabilities);
}
