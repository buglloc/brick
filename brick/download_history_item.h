// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_DOWNLOAD_HISTORY_ITEM_H_
#define BRICK_DOWNLOAD_HISTORY_ITEM_H_
#pragma once

#include <string>

#include "include/wrapper/cef_helpers.h"
#include "include/cef_base.h"
#include "brick/brick_types.h"
#include "brick/helper.h"


class DownloadHistoryItem : public CefBase {
public:

  DownloadHistoryItem(const std::string url, const std::string path, const std::string name)
      : url_ (url),
        path_ (path),
        name_ (name) {

    status_ = DC_STATUS_PROGRESS;
    reason_ = DC_REASON_NONE;
    percent_ = 0;
    current_ = 0;
    total_ = 0;
  }

  const std::string GetPath() const {
    return path_;
  }

  const std::string GetName() const {
    return name_;
  }

  const std::string GetUrl() const {
    return url_;
  }

  const DownloadClientStatus Status() {
    return status_;
  }

  void SetStatus(DownloadClientStatus status) {
    status_ = status;
  }

  const DownloadClientReason Reason() {
    return reason_;
  }

  void SetReason(DownloadClientReason reason) {
    reason_ = reason;
  }

  const int Percent() {
    return percent_;
  }

  void SetPercent(int percent) {
    percent_ = percent;
  }

  const int64 CurrentBytes() {
    return current_;
  }

  void SetCurrentBytes(int64 current) {
    current_ = current;
  }

  const int64 TotalBytes() {
    return total_;
  }

  void SetTotalBytes(int64 total) {
    total_ = total;
  }

  bool InProgress() {
    return status_ == DC_STATUS_PROGRESS;
  }

  bool IsSuccess() {
    return status_ == DC_STATUS_SUCCESS;
  }

  bool IsFailed() {
    return status_ == DC_STATUS_FAILED;
  }

  void UpdateProgress(int percent, int64 current, int64 total) {
    percent_ = percent;
    current_ = current;
    total_ = total;
  }

private:
  const std::string url_;
  const std::string path_;
  const std::string name_;
  DownloadClientStatus status_;
  DownloadClientReason reason_;
  int percent_;
  int64 current_;
  int64 total_;

IMPLEMENT_REFCOUNTING(DownloadHistoryItem);
};

#endif  // BRICK_DOWNLOAD_HISTORY_ITEM_H_
