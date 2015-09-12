// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_PLATFORM_UTIL_H_
#define BRICK_PLATFORM_UTIL_H_
#pragma once

#include <string>

namespace platform_util {

    enum DesktopEnvironment {
        DESKTOP_ENVIRONMENT_OTHER,
        DESKTOP_ENVIRONMENT_GNOME,
        // KDE3 and KDE4+ are sufficiently different that we count
        // them as two different desktop environments here.
        DESKTOP_ENVIRONMENT_KDE_OLD,
        DESKTOP_ENVIRONMENT_KDE,
        DESKTOP_ENVIRONMENT_UNITY,
        DESKTOP_ENVIRONMENT_XFCE,
    };

    void OpenExternal(const std::string &url);
    void ShowInFolder(const std::string &path);
    bool IsPathExists(const std::string &path);
    bool IsDir(const std::string &path);
    bool MakeDirectory(const std::string &path);
    bool GetEnv(const char* variable_name, std::string* result);
    bool HasEnv(const char* variable_name);
    DesktopEnvironment GetDesktopEnvironment();
    const std::string GetDefaultDownloadDir();
    const std::string GetHomeDir();
}  // namespace platform_util

#endif  // BRICK_PLATFORM_UTIL_H_
