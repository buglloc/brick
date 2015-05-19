// Copyright (c) 2015 The Brick Authors.

#include "brick/notification_manager.h"

NotificationManager::NotificationManager() :
    notification_(NULL),
    last_id_(0) {

};

void
NotificationManager::OnClose() {
 notification_ = NULL;
}