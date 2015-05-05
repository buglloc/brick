// Copyright (c) 2015 The Brick Authors.

#include "brick/window/window_features.h"

WindowFeatures::WindowFeatures(const CefPopupFeatures& popupFeatures) {
  this->width = popupFeatures.width;
  this->height = popupFeatures.height;
  this->x = popupFeatures.x;
  this->y = popupFeatures.y;
  this->dialog = popupFeatures.dialog > 0;
  this->topmost = false;

  if (popupFeatures.additionalFeatures) {
    CefString feature;
    int size = cef_string_list_size(popupFeatures.additionalFeatures);
    for (int i = 0; i < size; i++) {
      cef_string_list_value(popupFeatures.additionalFeatures, i, feature.GetWritableStruct());
      if (feature == "topmost") {
        this->topmost = true;
      }
    }
  }
}
