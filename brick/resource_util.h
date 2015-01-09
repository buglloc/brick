// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_RESOURCE_UTIL_H_
#define CEF_TESTS_CEFCLIENT_RESOURCE_UTIL_H_
#pragma once

#include <string>
#include "include/cef_stream.h"

// Retrieve a resource as a string.
bool LoadBinaryResource(const std::string resource_dir, const char* resource_name, std::string& resource_data);

// Retrieve a resource as a steam reader.
CefRefPtr<CefStreamReader> GetBinaryResourceReader(const std::string resource_dir, const char* resource_name);

#endif  // CEF_TESTS_CEFCLIENT_RESOURCE_UTIL_H_
