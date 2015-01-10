#ifndef BRICK_MAIN_WINDOW_H_
#define BRICK_MAIN_WINDOW_H_
#pragma once

#include "base_window.h"

#define MAIN_WINDOW_DEFAULT_WIDTH 800
#define MAIN_WINDOW_DEFAULT_HEIGHT 400

class MainWindow : public BaseWindow {

  public:

    MainWindow() {};
    ~MainWindow() {} ;
    void Init() OVERRIDE;
    void SetFocus(bool focused) OVERRIDE;

IMPLEMENT_REFCOUNTING(MainWindow);
};

#endif /* end of BRICK_MAIN_WINDOW_H_ */
