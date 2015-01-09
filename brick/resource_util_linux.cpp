// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "resource_util.h"
#include "cef_handler.h"
#include <unistd.h>

namespace {
    std::string resource_dir = "";
}

bool GetResourceDir(std::string& dir) {

  if (resource_dir.empty()) {
    resource_dir = ClientHandler::GetInstance()->GetAppSettings().resource_dir + "/web";
  }

  dir = resource_dir;
  return true;
}
