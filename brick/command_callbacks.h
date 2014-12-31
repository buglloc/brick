#ifndef CEF_SSSS
#define CEF_SSSS
#pragma once

#include "include/cef_browser.h"

// Base CommandCallback class
class CommandCallback : public CefBase {

public:
  // Called when the command is complete. When handled=true, the command
  // was handled and no further processing should occur.
  virtual void CommandComplete(bool handled) = 0;

private:
IMPLEMENT_REFCOUNTING(CommandCallback);
};
#endif