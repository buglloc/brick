// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_DOWNLOAD_PROGRESS_EVENT_H_
#define BRICK_EVENT_DOWNLOAD_PROGRESS_EVENT_H_
#pragma once

#include <string>

#include "brick/event/event.h"

class DownloadProgressEvent : public Event {
 public:
  DownloadProgressEvent(const EventSender* sender,
                        const std::string id,
                        const int percent,
                        const int64 speed,
                        const int64 current,
                        const int64 total) :
     Event (sender),
     id_ (id),
     percent_ (percent),
     speed_ (speed),
     current_ (current),
     total_ (total) {

  }

  DownloadProgressEvent(const std::string id,
                        const int percent,
                        const int64 speed,
                        const int64 current,
                        const int64 total) :
     Event (nullptr),
     id_ (id),
     percent_ (percent),
     speed_ (speed),
     current_ (current),
     total_ (total) {
  }

  virtual ~DownloadProgressEvent() { }

  const int getPercent() const {
    return percent_;
  }

  const int64 getSpeed() const {
    return speed_;
  }

  const int64 getCurrent() const {
    return current_;
  }

  const int64 getTotal() const {
    return total_;
  }

  const std::string getId() const {
    return id_;
  }

 private:
  const std::string id_;
  const int percent_;
  const int64 speed_;
  const int64 current_;
  const int64 total_;

IMPLEMENT_REFCOUNTING(DownloadProgressEvent);
};

#endif  // BRICK_EVENT_DOWNLOAD_PROGRESS_EVENT_H_
