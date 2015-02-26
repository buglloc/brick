#ifndef BRICK_NOTIFICATION_H_
#define BRICK_NOTIFICATION_H_
#pragma once
#include <string>

class Notification {

public:
  static void Notify(const std::string title, std::string body, std::string icon, int delay);
  static void Hide();

private:
  static std::string TryGetIcon(std::string icon);
  static std::string GetDefaultIcon();
};

#endif  // BRICK_NOTIFICATION_H_
