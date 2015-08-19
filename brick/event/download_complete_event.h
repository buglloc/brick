// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_DOWNLOAD_COMPLETE_EVENT_H_
#define BRICK_EVENT_DOWNLOAD_COMPLETE_EVENT_H_
#pragma once

#include <string>

#include "brick/event/event.h"
#include "brick/brick_types.h"

class DownloadCompleteEvent : public Event {
 public:
  DownloadCompleteEvent(const EventSender* sender,
                        const std::string id,
                        const std::string filepath,
                        const DownloadClientStatus status,
                        const DownloadClientReason reason) :
     Event (sender),
     id_ (id),
     filepath_ (filepath),
     status_ (status),
     reason_ (reason) {
  }

  DownloadCompleteEvent(const std::string id,
                        const std::string filepath,
                        const DownloadClientStatus status,
                        const DownloadClientReason reason) :
     Event (nullptr),
     id_ (id),
     filepath_ (filepath),
     status_ (status),
     reason_ (reason) {
  }

  virtual ~DownloadCompleteEvent() { }

  const std::string getId() const {
    return id_;
  }

  const std::string getFilepath() const {
    return filepath_;
  }

  const DownloadClientStatus getStatus() const {
    return status_;
  }

  const DownloadClientReason getReason() const {
    return reason_;
  }

 private:
  const std::string id_;
  const std::string filepath_;
  const DownloadClientStatus status_;
  const DownloadClientReason reason_;

IMPLEMENT_REFCOUNTING(DownloadCompleteEvent);
};

#endif  // BRICK_EVENT_DOWNLOAD_COMPLETE_EVENT_H_
