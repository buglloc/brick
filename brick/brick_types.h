// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_BRICK_TYPES_H_
#define BRICK_BRICK_TYPES_H_
#pragma once


typedef enum {
  DC_STATUS_SUCCESS = 0,
  DC_STATUS_FAILED,
  DC_STATUS_PROGRESS,
} DownloadClientStatus;

typedef enum {
  DC_REASON_NONE = 0,
  DC_REASON_UNKNOWN,
  DC_REASON_ABORTED,
  DC_REASON_HTTP,
  DC_REASON_NOT_ATTACHMENT
} DownloadClientReason;

typedef enum {
  DEFAULT = 0,
  OFFLINE,
  ONLINE,
  DND,
  AWAY,
  FLASH,
  FLASH_IMPORTANT
} IndicatorStatusIcon;

#endif  // BRICK_BRICK_TYPES_H_
