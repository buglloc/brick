// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_PLATFORM_UTIL_H_
#define BRICK_PLATFORM_UTIL_H_
#pragma once

#include <string>

namespace platform_util {

    void OpenExternal(std::string url);
    bool IsPathExists(std::string path);
    bool MakeDirectory(std::string path);
}  // namespace platform_util

#endif  // BRICK_PLATFORM_UTIL_H_
