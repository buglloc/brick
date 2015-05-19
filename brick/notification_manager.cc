// Copyright (c) 2015 The Brick Authors.

#include "brick/notification_manager.h"

NotificationManager::NotificationManager() :
    notification_(NULL),
    last_id_(0),
    is_append_supported_(false) {

  InitializeCapabilities();
};

void
NotificationManager::OnClose() {
 notification_ = NULL;
}