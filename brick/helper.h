// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_HELPER_H_
#define BRICK_HELPER_H_
#pragma once

#include <string>

#include "include/cef_v8.h"
#include <assert.h>  // NOLINT(build/include_order)

#ifndef NDEBUG
#define ASSERT(condition) if (!(condition)) { assert(false); }
#else
#define ASSERT(condition) ((void)0)
#endif

#define REQUIRE_UI_THREAD()   ASSERT(CefCurrentlyOn(TID_UI));
#define REQUIRE_IO_THREAD()   ASSERT(CefCurrentlyOn(TID_IO));
#define REQUIRE_FILE_THREAD() ASSERT(CefCurrentlyOn(TID_FILE));

namespace helper {

  void SetList(CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target);
  void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target);
  void SetListValue(CefRefPtr<CefListValue> list, int index, CefRefPtr<CefV8Value> value);
  void SetListValue(CefRefPtr<CefV8Value> list, int index, CefRefPtr<CefListValue> value);
  CefRefPtr<CefV8Value> ListValueToV8Value(CefRefPtr<CefListValue> value, int index);
  std::string DumpListValue(CefRefPtr<CefListValue> list, int index);
  std::string DumpList(CefRefPtr<CefListValue> list);
  std::string stringReplace(std::string src, std::string const &target, std::string const &repl);
  bool ParseUrl(const std::string& url, std::string* file_name, std::string* mime_type);
  std::string BaseDir(const std::string& path);
  std::string GetFileExtension(const std::string& path);
  std::string GetFileName(const std::string& path);
  uint32 HashString(const std::string& str);
}  // namespace helper

#endif  // BRICK_HELPER_H_
