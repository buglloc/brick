// Copyright (c) 2015 The Brick Authors.

#include "brick/notification_manager.h"

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
  const char kPersistenceCapability[] = "persistence";
  const char kActionsCapability[] = "actions";
  const char kAppendHint[] = "x-canonical-append";
  const char kResidentHint[] = "resident";
  const char kTransientHint[] = "transient";
  const char kDefaultActionName[] = "default";
  const char kDataJsIdName[] = "js-id";
  const char kDataTypeName[] = "type";
  const gint kTypeRegular = 0;
  const gint kTypeMessage = 1;
  // See https://developer.gnome.org/notification-spec/#signal-notification-closed
  const gint kCloseReasonExpire = 1;
  const gint kCloseReasonDismissed = 2;
  const gint kCloseReasonProgrammaticaly = 3;
  const gint kCloseReasonUndefined = 4;
  // See kde-workspace/plasma-workspace/dataengines/notifications/notificationsengine.cpp
  const char kKdeVendorName[] = "KDE";


  void
  OnCloseNotification(NotifyNotification *notify, NotificationManager *self) {
    const char* js_id = NULL;
    if (notify_notification_get_closed_reason(notify) == kCloseReasonDismissed)
      js_id = static_cast<const char *>(g_object_get_data(G_OBJECT(notify), kDataJsIdName));

    gint type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(notify), kDataTypeName));
    self->OnClose(js_id != NULL ? js_id : "", type == kTypeMessage);
  }

  void
  OnAction(NotifyNotification *notify, char *action, NotificationManager *self) {
    const char* js_id = static_cast<char *>(g_object_get_data(G_OBJECT(notify), kDataJsIdName));
    gint type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(notify), kDataTypeName));
    self->OnClick(js_id != NULL ? js_id : "", type == kTypeMessage);

    // Clear notification jsId
    g_object_set_data(G_OBJECT(notify), kDataJsIdName, NULL);

    if (self->IsPersistenceSupported()) {
      // In persistence mode notification daemon don't close notification itself
      self->Close();
    }
  }

}  // namespace

void
NotificationManager::Notify(
    const std::string &title,
    std::string body,
    std::string icon,
    int delay,
    const std::string &js_id,
    bool is_message) {

  notify_init("brick");

  last_id_++;
  bool need_download = false;
  std::string notification_icon = TryGetIcon(icon, need_download);

  if (notification_ != nullptr && !is_append_supported_) {
    // Permanently close previous notification if server doesn't supports message appending.
    // Due to KDE NotificationEngine logic.
    Close();
  }

  notification_ = notify_notification_new(
     title.c_str(),
     body.c_str(),
     need_download || notification_icon.empty() ? GetDefaultIcon().c_str() : notification_icon.c_str()
  );

  g_object_set_data_full(G_OBJECT(notification_), kDataJsIdName, g_strdup(js_id.c_str()), (GDestroyNotify) g_free);
  g_object_set_data(G_OBJECT(notification_), kDataTypeName, GINT_TO_POINTER(is_message ? kTypeMessage : kTypeRegular));

  if (is_append_supported_) {
    notify_notification_set_hint_string(notification_, kAppendHint, "1");
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

  if (is_persistence_supported_) {
    // Ugly hack for Gnome Shell, see: https://github.com/buglloc/brick/issues/23
    notify_notification_set_hint(notification_, kResidentHint, g_variant_new_boolean(true));
    notify_notification_set_hint(notification_, kTransientHint, g_variant_new_boolean(true));
  }

  g_signal_connect(notification_, "closed", G_CALLBACK(OnCloseNotification), this);

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
NotificationManager::Close() {
  if (notification_ == nullptr)
    return;

  if (!notify_notification_close(notification_, NULL))
    LOG(WARNING) << "Failed to close notification";

  g_object_unref (G_OBJECT(notification_));
  notification_ = NULL;
}

void
NotificationManager::OnClose(const std::string &js_id, bool is_message) {
  notification_ = nullptr;

  // On KDE notification has timed out and has been dismissed by the user closes with the same reason (2)
  if (!on_kde_ && !js_id.empty()) {
    NotificationEvent e(js_id, is_message, false);
    EventBus::FireEvent(e);
  }
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
  if (notification_ == nullptr)
    return;

  if (!success)
    return;

  if (id < last_id_)
    return;

  // Notification update was broken on KDE
  // TODO(buglloc): Can fix this?
  if (on_kde_)
    return;

  g_object_set(G_OBJECT(notification_), "icon-name", icon_path.c_str(), NULL);

  // We must show notification again to update icon :-(
  // TODO(buglloc): Research!
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
    } else if (strcmp(cap, kPersistenceCapability) == 0) {
      LOG(INFO) << "Notification server supports " << kPersistenceCapability;
      is_persistence_supported_ = true;
    }
  }
  g_list_foreach(capabilities, (GFunc)g_free, NULL);
  g_list_free(capabilities);

  // Detect KDE, because it have some strange behavior
  char* vendor;
  if (notify_get_server_info(NULL, &vendor, NULL, NULL)) {
    on_kde_ = !strcmp(vendor, kKdeVendorName);
    g_free(vendor);
  } else {
    LOG(WARNING) << "Failed to get notification server info";
  }
}
