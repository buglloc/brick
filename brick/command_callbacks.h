// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_COMMAND_CALLBACKS_H_
#define BRICK_COMMAND_CALLBACKS_H_
#pragma once

#include "include/cef_browser.h"

// Base CommandCallback class
class CommandCallback : public CefBase {

 public:
  // Called when the command is complete. When handled=true, the command
  // was handled and no further processing should occur.
  virtual void CommandComplete(bool handled) = 0;

IMPLEMENT_REFCOUNTING(CommandCallback);
};
#endif  // BRICK_COMMAND_CALLBACKS_H_
