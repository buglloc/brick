// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_NOTIFICATION_MANAGER_H_
#define BRICK_NOTIFICATION_MANAGER_H_
#pragma once

#include <libnotify/notification.h>

#include <string>

#include "include/cef_base.h"

class NotificationManager : public CefBase {
 public:
  NotificationManager();

  void Notify(const std::string title, std::string body, std::string icon, int delay);
  void Hide();
  void OnClose();

 protected:
  void InitializeCapabilities();
  std::string TryGetIcon(std::string icon, bool &need_download);
  void UpdateIcon(int id, std::string icon_path, bool success);
  std::string GetDefaultIcon();
  void AsyncDownloadIcon(int id, const std::string& url, const std::string& path);

 private:
  NotifyNotification *notification_ = NULL;
  int last_id_ = 0;
  bool is_append_supported_;

 IMPLEMENT_REFCOUNTING(NotificationManager);
};

#endif  // BRICK_NOTIFICATION_MANAGER_H_
