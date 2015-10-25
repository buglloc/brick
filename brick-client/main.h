// Copyright (c) 2015 The Brick Authors.

#ifndef BRICK_CLIENT_MAIN_H_
#define BRICK_CLIENT_MAIN_H_
#pragma once

#include <gio/gio.h>

#include <string>
#include <exception>


void PrintUsage();
bool CallCommand(bool to_app, const std::string &command, GVariant *parameters = nullptr, bool allow_autostart = false);
void HandleBxProtocol(const std::string &requestLine);
void HandleCommand(const std::string &command);

#endif  // BRICK_CLIENT_MAIN_H_
