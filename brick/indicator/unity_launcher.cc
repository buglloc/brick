// Copyright (c) 2015 The Brick Authors.

#include "brick/indicator/unity_launcher.h"

#include <gtk/gtk.h>
#include <dlfcn.h>

#include "brick/platform_util.h"
#include "brick/brick_app.h"


namespace {

  typedef LauncherEntry* (*unity_launcher_entry_get_for_desktop_id_func) (const gchar* desktop_id);

  typedef void (*unity_launcher_entry_set_count_func) (LauncherEntry* self,
                                                      gint64 value);

  typedef void (*unity_launcher_entry_set_count_visible_func) (LauncherEntry* self,
                                                               gboolean value);

  typedef void (*unity_launcher_entry_set_progress_func) (LauncherEntry* self,
                                                         gdouble value);

  typedef void (*unity_launcher_entry_set_progress_visible_func) (LauncherEntry* self,
                                                                  gboolean value);

  bool g_attempted_load = false;
  bool g_opened = false;

  // Retrieved functions from libunity.
  unity_launcher_entry_get_for_desktop_id_func
    launcher_entry_get_for_desktop_id = NULL;
  unity_launcher_entry_set_count_func
    launcher_entry_set_count = NULL;
  unity_launcher_entry_set_count_visible_func
    launcher_entry_set_count_visible = NULL;
  unity_launcher_entry_set_progress_func
    launcher_entry_set_progress = NULL;
  unity_launcher_entry_set_progress_visible_func
    launcher_entry_set_progress_visible = NULL;

  void EnsureMethodsLoaded() {
    if (g_attempted_load)
      return;

    g_attempted_load = true;

    // UnityLauncher works only on Unity ;)
    if (platform_util::GetDesktopEnvironment() != platform_util::DESKTOP_ENVIRONMENT_UNITY) {
      return;
    }

    void* unity_lib = dlopen("libunity.so.9", RTLD_LAZY);

    if (!unity_lib) {
      unity_lib = dlopen("libunity.so.6", RTLD_LAZY);
    }

    if (!unity_lib) {
      unity_lib = dlopen("libunity.so.4", RTLD_LAZY);
    }

    if (!unity_lib) {
      return;
    }

    g_opened = true;

    launcher_entry_get_for_desktop_id = reinterpret_cast<unity_launcher_entry_get_for_desktop_id_func>(
      dlsym(unity_lib, "unity_launcher_entry_get_for_desktop_id"));

    launcher_entry_set_count = reinterpret_cast<unity_launcher_entry_set_count_func>(
      dlsym(unity_lib, "unity_launcher_entry_set_count"));

    launcher_entry_set_count_visible = reinterpret_cast<unity_launcher_entry_set_count_visible_func>(
      dlsym(unity_lib, "unity_launcher_entry_set_count_visible"));

    launcher_entry_set_progress = reinterpret_cast<unity_launcher_entry_set_progress_func>(
      dlsym(unity_lib, "unity_launcher_entry_set_progress"));

    launcher_entry_set_progress_visible = reinterpret_cast<unity_launcher_entry_set_progress_visible_func>(
      dlsym(unity_lib, "unity_launcher_entry_set_progress_visible"));
  }

}  // namespace

void
UnityLauncher::Init() {
  handler_ = launcher_entry_get_for_desktop_id(APP_COMMON_NAME ".desktop");
  RegisterEventListeners();
}

void
UnityLauncher::OnEvent(const DownloadProgressEvent& event) {
  if (event.GetTotal() <= 0)
    return;

  if (event.GetPercent() >= 100 && downloads_.count(event.GetId())) {
    downloads_.erase(event.GetId());
  }

  downloads_[event.GetId()] = event.GetPercent();
  Update();
}

void
UnityLauncher::OnEvent(const DownloadCompleteEvent& event) {
  if (downloads_.count(event.GetId())) {
    downloads_.erase(event.GetId());
  }
  Update();
}

void
UnityLauncher::Update() {
  int badges = badges_ + static_cast<int>(downloads_.size());
  if (badges > 0) {
    launcher_entry_set_count(handler_, badges);
    launcher_entry_set_count_visible(handler_, true);
    if (!downloads_.empty()) {
      launcher_entry_set_progress_visible(handler_, true);
      double progress = 0;
      for (const auto &download : downloads_) {
        progress += download.second;
      }

      launcher_entry_set_progress(handler_, progress / downloads_.size() / 100);
    }
  } else {
    launcher_entry_set_count(handler_, 0);
    launcher_entry_set_count_visible(handler_, false);
    launcher_entry_set_progress(handler_, 0);
    launcher_entry_set_progress_visible(handler_, false);
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

// static
bool
UnityLauncher::CouldOpen() {
  EnsureMethodsLoaded();
  return g_opened;
}
