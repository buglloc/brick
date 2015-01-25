#include "browser_window.h"

CefRefPtr<OSRWindow>
BrowserWindow::GetOsrWindow() {
  return osr_window_;
}