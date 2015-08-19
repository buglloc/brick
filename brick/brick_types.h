#ifndef BRICK_TYPES_H_
#define BRICK_TYPES_H_
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
  DC_REASON_HTTP
} DownloadClientReason;

#endif  // BRICK_TYPES_H_