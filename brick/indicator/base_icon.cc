// Copyright (c) 2015 The Brick Authors.

#include "brick/indicator/base_icon.h"


namespace {

  const char *const icon_names_[] = {
      "offline",
      "offline",
      "online",
      "dnd",
      "away",
      "flash",
      "flash-important"
  };

  const char *const icon_files_[] = {
      "offline.svg",
      "offline.svg",
      "online.svg",
      "dnd.svg",
      "away.svg",
      "flash.svg",
      "flash-important.svg"
  };

}  // namespace


BaseIcon::BaseIcon(std::string icons_dir) :
    icons_folder_(icons_dir) {
}

std::string
BaseIcon::GetIconPath(IndicatorStatusIcon icon) {
  return icons_folder_ + icon_files_[icon];
}

std::string
BaseIcon::GetIconName(IndicatorStatusIcon icon) {
  return icon_names_[icon];
}
