#ifndef BRICK_PLATFORM_UTILS_H_
#define BRICK_PLATFORM_UTILS_H_
#pragma once

#include <string>

namespace platform_util {

    void OpenExternal(std::string url);
    bool IsPathExists(std::string path);
    bool MakeDirectory(std::string path);
}

#endif  // BRICK_PLATFORM_UTILS_H_
