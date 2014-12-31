#ifndef BRICK_V8_HANDLER_H_
#define BRICK_V8_HANDLER_H_

#include <include/cef_v8.h>
#include "include/cef_client.h"

#include "cef_app.h"
#include "cef_handler.h"

// Messages error codes.
#if !defined(OS_WIN) // NO_ERROR is defined on windows
static const int NO_ERROR                   = 0;
#endif
static const int ERR_UNKNOWN                = 1;
static const int ERR_INVALID_PARAMS         = 2;
static const int ERR_NOT_FOUND              = 3;
static const int ERR_CANT_READ              = 4;
static const int ERR_UNSUPPORTED_ENCODING   = 5;
static const int ERR_CANT_WRITE             = 6;
static const int ERR_OUT_OF_SPACE           = 7;
static const int ERR_NOT_FILE               = 8;
static const int ERR_NOT_DIRECTORY          = 9;
static const int ERR_FILE_EXISTS            = 10;
static const int ERR_BROWSER_NOT_INSTALLED  = 11;
static const int ERR_PID_NOT_FOUND          = -9999; // negative int to avoid confusion with real PIDs

class V8Handler : public CefV8Handler {
  public:
    explicit V8Handler(CefRefPtr<ClientApp> cef_app)
       : cef_app_(cef_app)
       , messageId(0) {
    }

    virtual bool Execute(const CefString& name,
       CefRefPtr<CefV8Value> object,
       const CefV8ValueList& arguments,
       CefRefPtr<CefV8Value>& retval,
       CefString& exception) OVERRIDE;

    private:
      CefRefPtr<ClientApp> cef_app_;
      int32 messageId;
      // Provide the reference counting implementation for this class.
    IMPLEMENT_REFCOUNTING(V8Handler);
    };
#endif  // BRICK_V8_HANDLER_H_