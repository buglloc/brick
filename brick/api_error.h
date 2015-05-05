// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_API_ERROR_H_
#define BRICK_API_ERROR_H_
#pragma once

// Messages error codes.
#if !defined(OS_WIN)  // NO_ERROR is defined on windows
static const int NO_ERROR                   = 0;
#endif
static const int ERR_UNKNOWN                = 1;
static const int ERR_INVALID_PARAMS         = 2;
static const int ERR_NOT_FOUND              = 3;
static const int ERR_CANT_READ              = 4;
static const int ERR_UNSUPPORTED_ENCODING   = 5;
static const int ERR_CANT_WRITE             = 6;
static const int ERR_OUT_OF_SPACE           = 7;
static const int ERR_NOT_FILE               = 8;
static const int ERR_NOT_DIRECTORY          = 9;
static const int ERR_FILE_EXISTS            = 10;
static const int ERR_BROWSER_NOT_INSTALLED  = 11;
static const int ERR_PID_NOT_FOUND          = -9999;  // negative int to avoid confusion with real PIDs

#endif  // BRICK_API_ERROR_H_
