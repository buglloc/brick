// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_INDICATOR_UNITY_LAUNCHER_H_
#define BRICK_INDICATOR_UNITY_LAUNCHER_H_
#pragma once

#include <unity.h>

#include <string>
#include <map>

#include "include/cef_base.h"
#include "brick/event/event_bus.h"
#include "brick/event/download_progress_event.h"
#include "brick/event/download_complete_event.h"

class UnityLauncher : public CefBase,
                      public EventHandler<DownloadProgressEvent>,
                      public EventHandler<DownloadCompleteEvent>{

 public:
  void Init();
  void SetBadge(int badge);
  // System events handler
  virtual void OnEvent(const DownloadProgressEvent &event) OVERRIDE;
  virtual void OnEvent(const DownloadCompleteEvent &event) OVERRIDE;

 protected:
  void RegisterEventListeners();
  void Update();

 private:
  int badges_;
  std::map<std::string, int> downloads_;
  UnityLauncherEntry *handler_;

IMPLEMENT_REFCOUNTING(UnityLauncher);
};

#endif  // BRICK_INDICATOR_UNITY_LAUNCHER_H_
