// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_DESKTOP_MEDIA_H_
#define BRICK_DESKTOP_MEDIA_H_
#pragma once

#include <string>
#if defined(__linux__)
#include <glib.h>
#else
#endif

#include "include/internal/cef_linux.h"
#include "include/cef_values.h"

namespace desktop_media {
  bool EnumerateWindows(CefListValue* list);
  bool EnumerateScreens(CefListValue* list);
  bool GetMediaPreview(std::string type, int32 id, std::vector<unsigned char>* out);

}  // namespace desktop_media
#endif  // BRICK_DESKTOP_MEDIA_H_
