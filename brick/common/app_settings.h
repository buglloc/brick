// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_COMMON_APP_SETTINGS_H_
#define BRICK_COMMON_APP_SETTINGS_H_
#pragma once

#include <set>
#include <map>
#include <string>

#include "include/internal/cef_types_wrappers.h"
#include "include/cef_command_line.h"
#include "brick/account.h"

struct AppSettings {
  typedef std::map<std::string, std::string> client_scripts_map;

  std::string app_token;
  std::string profile_path;
  std::string cache_path;
  std::string log_file;
  std::string resource_dir;
  std::string download_dir;
  bool ignore_certificate_errors;
  cef_log_severity_t log_severity;
  bool start_minimized;
  bool auto_away;
  bool external_api;
  bool hide_on_delete;
  bool extended_status;
  bool implicit_file_download;
  bool auto_download;
  client_scripts_map client_scripts;

  static AppSettings InitByJson(std::string json);

  AppSettings();
  void UpdateByJson(std::string json);
  void UpdateByCommandLine(CefRefPtr<CefCommandLine> command_line);
  std::string DumpJson();
};

#endif  // BRICK_COMMON_APP_SETTINGS_H_
