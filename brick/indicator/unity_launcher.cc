// Copyright (c) 2015 The Brick Authors.

#include "brick/indicator/unity_launcher.h"
#include "brick/brick_app.h"
#include "include/base/cef_logging.h"

void
UnityLauncher::Init() {
  handler_ = unity_launcher_entry_get_for_desktop_id(APP_COMMON_NAME ".desktop");
  RegisterEventListeners();
}

void
UnityLauncher::onEvent(const DownloadProgressEvent& event) {
  if (event.getTotal() <= 0)
    return;

  if (event.getPercent() >= 100 && downloads_.count(event.getId())) {
    downloads_.erase(event.getId());
  }

  downloads_[event.getId()] = event.getPercent();
  Update();
}

void
UnityLauncher::onEvent(const DownloadCompleteEvent& event) {
  if (downloads_.count(event.getId())) {
    downloads_.erase(event.getId());
  }
  Update();
}

void
UnityLauncher::Update() {
  int badges = badges_ + static_cast<int>(downloads_.size());
  if (badges > 0) {
    unity_launcher_entry_set_count(handler_, badges);
    unity_launcher_entry_set_count_visible(handler_, true);
    if (!downloads_.empty()) {
      unity_launcher_entry_set_progress_visible(handler_, true);
      double progress = 0;
      for (const auto &download: downloads_) {
        progress += download.second;
      }

      unity_launcher_entry_set_progress(handler_, progress / downloads_.size() / 100);
    }
  } else {
    unity_launcher_entry_set_count(handler_, 0);
    unity_launcher_entry_set_count_visible(handler_, false);
    unity_launcher_entry_set_progress(handler_, 0);
    unity_launcher_entry_set_progress_visible(handler_, false);
  }
}

void
UnityLauncher::SetBadge(int badge) {
  badges_ = badge;
  Update();
}

void
UnityLauncher::RegisterEventListeners() {
  EventBus::AddHandler<DownloadProgressEvent>(*this);
  EventBus::AddHandler<DownloadCompleteEvent>(*this);
}
