#include "brick/helper.h"
#include "app_settings.h"
#include "third-party/json/json.h"

AppSettings::AppSettings()
: app_token(""),
  profile_path(""),
  cache_path(""),
  log_file(""),
  proxy_server(""),
  resource_dir(""),
  ignore_certificate_errors(false),
  log_severity(LOGSEVERITY_DEFAULT),
#ifndef NDEBUG
  start_minimized(true)
#else
  start_minimized(false)
#endif
{

}

AppSettings::~AppSettings() {

}

AppSettings
AppSettings::InitByJson(std::string json) {
  AppSettings settings;
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;

  bool parsingSuccessful = reader.parse(json, root);
  if (!parsingSuccessful) {
#if 0 // Turn off error message while we haven't any configuration file by default
      fprintf(stderr, "Failed to parse configuration: %s", reader.getFormattedErrorMessages().c_str());
#endif
      return settings;
  }

  if (root.isMember("app_token")
     && root["app_token"].isString()) {
    settings.profile_path = root["app_token"].asString();
  }

  if (root.isMember("profile_path")
      && root["profile_path"].isString()) {
    settings.profile_path = root["profile_path"].asString();
  }

  if (root.isMember("cache_path")
      && root["cache_path"].isString()) {
    settings.cache_path = root["cache_path"].asString();
  }

  if (root.isMember("log_file")
     && root["log_file"].isString()) {
    settings.log_file = root["log_file"].asString();
  }

  if (root.isMember("proxy_server")
     && root["proxy_server"].isString()) {
    settings.proxy_server = root["proxy_server"].asString();
  }

  if (root.isMember("ignore_certificate_errors")
     && root["ignore_certificate_errors"].isBool()) {
    settings.profile_path = root["ignore_certificate_errors"].asBool();
  }

  if (root.isMember("start_minimized")
     && root["start_minimized"].isBool()) {
    settings.start_minimized = root["start_minimized"].asBool();
  }

  if (root.isMember("resource_dir")
     && root["resource_dir"].isString()) {
    settings.profile_path = root["resource_dir"].asString();
  }

  return settings;
}

std::string
AppSettings::DumpJson() {
  return "Implement me!";
}