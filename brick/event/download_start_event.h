// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_EVENT_DOWNLOAD_START_EVENT_H_
#define BRICK_EVENT_DOWNLOAD_START_EVENT_H_
#pragma once

#include <string>
#include <ctime>
#include "brick/event/event.h"

class DownloadStartEvent : public Event {
 public:
  DownloadStartEvent(const EventSender* sender,
                     const std::string id,
                     const std::string filename,
                     const std::string filepath,
                     const std::string url,
                     const std::time_t date) :

     Event (sender),
     id_ (id),
     filename_ (filename),
     filepath_ (filepath),
     url_ (url),
     date_ (date) {

  }

  DownloadStartEvent(const std::string id,
                     const std::string filename,
                     const std::string filepath,
                     const std::string url,
                     const std::time_t date) :
     Event (nullptr),
     id_ (id),
     filename_ (filename),
     filepath_ (filepath),
     url_ (url),
     date_ (date) {

  }

  virtual ~DownloadStartEvent() { }

  const std::string getFilename() const {
    return filename_;
  }

  const std::string getFilepath() const {
    return filepath_;
  }

  const std::string getUrl() const {
    return url_;
  }

  const std::string getId() const {
    return id_;
  }

  const std::time_t getDate() const {
    return date_;
  }

 private:
  const std::string id_;
  const std::string filename_;
  const std::string filepath_;
  const std::string url_;
  const std::time_t date_;

IMPLEMENT_REFCOUNTING(DownloadStartEvent);
};

#endif  // BRICK_EVENT_DOWNLOAD_START_EVENT_H_
