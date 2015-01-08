#ifndef BRICK_MAIN_WINDOW_H_
#define BRICK_MAIN_WINDOW_H_
#pragma once

#if defined(__linux__)
// The Linux client uses GTK instead of the underlying platform type (X11).
#include <gtk/gtk.h>
#define ClientWindowHandle GtkWidget*
#else
#define ClientWindowHandle CefWindowHandle
#endif

#include <include/base/cef_lock.h>
#include <include/internal/cef_linux.h>

#define MAIN_WINDOW_DEFAULT_WIDTH 454
#define MAIN_WINDOW_DEFAULT_HEIGHT 454

enum WINDOW_STATE {
  WINDOW_STATE_NORMAL,
  WINDOW_STATE_MINIMIZED,
  WINDOW_STATE_MAXIMIZED,
  WINDOW_STATE_FULLSCREEN
};

class MainWindow : public CefBase {

  public:

    MainWindow();
    ~MainWindow();
    ClientWindowHandle GetHandler();

    // Platform specific methods
    void Init();
    void Minimize();
    void Maximize();
    void Restore();
    void Show();
    void Hide();
    bool GetHided();
    void Focus();
    void Destroy();
    void Move(int left, int top, int width, int height);
    void Move(int left, int top);
    void Resize(int width, int height);
    const char* GetTitle();
    void SetTitle(const char* title);
    void Fullscreen();
    WINDOW_STATE GetState();
    void SetTopmost(bool ontop);
    void SetResizable(bool resizable);
    bool GetResizable();
    void SetShowChrome(bool showChrome);
    bool GetShowChrome();
    void SetOpacity(double opacity);
    double GetOpacity();
    // Event handlers


    // Shared methods
    bool SetFocus(bool focused);
    bool HasFocus();

  private:
    ClientWindowHandle window_handler_;
    bool hided_;
    bool focused_;

IMPLEMENT_REFCOUNTING(MainWindow);
};

#endif /* end of BRICK_MAIN_WINDOW_H_ */
