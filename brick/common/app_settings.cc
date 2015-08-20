// Copyright (c) 2015 The Brick Authors.

#include "brick/common/app_settings.h"

#include <string>

#include "third-party/json/json.h"
#include "include/base/cef_logging.h"
#include "brick/platform_util.h"
#include "brick/helper.h"


namespace {
  const char kAppTokenName[]                = "app-token";
  const char kProfilePathName[]             = "profile-path";
  const char kCachePathName[]               = "cache-path";
  const char kLogFileName[]                 = "log-file";
  const char kResourceDirName[]             = "resource-dir";
  const char kDownloadDirName[]             = "download-dir";
  const char kIgnoreCertificateErrors[]     = "ignore-certificate-errors";
  const char kStartMinimizedName[]          = "minimized";
  const char kAutoAwayName[]                = "auto-away";
  const char kExternalApiName[]             = "external-api";
  const char kHideOnDeleteName[]            = "hide-on-delete";
  const char kExtendedStatusName[]          = "extended-status";
  const char kImplicitFileDownloadName[]    = "implicit-file-download";
  const char kClientScriptsName[]           = "client-scripts";

}  // namespace

AppSettings::AppSettings()
: app_token (""),
  profile_path (""),
  cache_path (""),
  log_file (""),
  resource_dir (""),
  ignore_certificate_errors (false),
  log_severity (LOGSEVERITY_DEFAULT),
  start_minimized(false),
  auto_away (true),
  external_api (true),
  hide_on_delete (true),
  extended_status (true),
  implicit_file_download (false) {

  download_dir = platform_util::GetDefaultDownloadDir();
  LOG(WARNING) << "DOWNLOAD: " << download_dir;
}

AppSettings
AppSettings::InitByJson(std::string json) {
  AppSettings settings;
  settings.UpdateByJson(json);

  return settings;
}

void
AppSettings::UpdateByJson(std::string json) {
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;

  bool parsingSuccessful = reader.parse(json, root);
  if (!parsingSuccessful) {
#if 0
// Turn off error message while we haven't any configuration file by default
    fprintf(
      stderr,
      "Failed to parse configuration: %s",
      reader.getFormattedErrorMessages().c_str()
    );
#endif
    return;
  }

  if (root.isMember(kAppTokenName)
     && root[kAppTokenName].isString()) {
    profile_path = root[kAppTokenName].asString();
  }

  if (root.isMember(kProfilePathName)
     && root[kProfilePathName].isString()) {
    profile_path = root[kProfilePathName].asString();
  }

  if (root.isMember(kCachePathName)
     && root[kCachePathName].isString()) {
    cache_path = root[kCachePathName].asString();
  }

  if (root.isMember(kLogFileName)
     && root[kLogFileName].isString()) {
    log_file = root[kLogFileName].asString();
  }

  if (root.isMember(kIgnoreCertificateErrors)
     && root[kIgnoreCertificateErrors].isBool()) {
    ignore_certificate_errors = root[kIgnoreCertificateErrors].asBool();
  }

  if (root.isMember(kStartMinimizedName)
     && root[kStartMinimizedName].isBool()) {
    start_minimized = root[kStartMinimizedName].asBool();
  }

  if (root.isMember(kResourceDirName)
     && root[kResourceDirName].isString()) {
    resource_dir = root[kResourceDirName].asString();
  }

  if (root.isMember(kDownloadDirName)
      && root[kDownloadDirName].isString()) {
    download_dir = root[kDownloadDirName].asString();
  }

  if (root.isMember(kAutoAwayName)
     && root[kAutoAwayName].isBool()) {
    auto_away = root[kAutoAwayName].asBool();
  }

  if (root.isMember(kExternalApiName)
     && root[kExternalApiName].isBool()) {
    external_api = root[kExternalApiName].asBool();
  }

  if (root.isMember(kHideOnDeleteName)
     && root[kHideOnDeleteName].isBool()) {
    hide_on_delete = root[kHideOnDeleteName].asBool();
  }

  if (root.isMember(kExtendedStatusName)
     && root[kExtendedStatusName].isBool()) {
    extended_status = root[kExtendedStatusName].asBool();
  }

  if (root.isMember(kImplicitFileDownloadName)
      && root[kImplicitFileDownloadName].isBool()) {
    implicit_file_download = root[kImplicitFileDownloadName].asBool();
  }

  if (root.isMember(kClientScriptsName)
     && root[kClientScriptsName].isArray()) {
    for (auto script: root[kClientScriptsName]) {

      if (!script.isString()) {
        LOG(WARNING) << "Strange client script: " << script << "; Skipping";
        continue;
      }

      std::string script_path = script.asString();
      if (script_path.find('/') != 0) {
        LOG(WARNING) << "Can't load client script, supported only absolute path: " << script_path << "; Skipping";
        continue;
      }

      std::string id;
      id.append(std::to_string(helper::HashString(script_path)));
      id.append(".js");
      client_scripts[id] = script_path;
    }
  }
}

void
AppSettings::UpdateByCommandLine(CefRefPtr<CefCommandLine> command_line) {

  if (command_line->HasSwitch(kHideOnDeleteName)) {
    hide_on_delete = true;
  }

  if (command_line->HasSwitch(kStartMinimizedName)) {
    start_minimized = true;
  }
}

std::string
AppSettings::DumpJson() {
  return "Implement me!";
}
