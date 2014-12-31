#ifndef BRICK_NOTIFICATION_H_
#define BRICK_NOTIFICATION_H_
#pragma once


class Notification {

public:
  static void Notify(const std::string title, std::string body, int delay);
  static void Hide();
};

#endif  // BRICK_NOTIFICATION_H_
