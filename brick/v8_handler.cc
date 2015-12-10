// Copyright (c) 2015 The Brick Authors.

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "brick/client_handler.h"
#include "brick/helper.h"

#include "brick/v8_handler.h"

namespace {
  const char kSpellCheckWordName[] = "AppExSpellCheckWord";
}  // namespace


bool
V8Handler::Execute(
    const CefString &name, CefRefPtr<CefV8Value> object,
    const CefV8ValueList &arguments,
    CefRefPtr<CefV8Value> &retval,
    CefString &exception) {

  // Pass all messages to the browser process
  CefRefPtr<CefBrowser> browser =
     CefV8Context::GetCurrentContext()->GetBrowser();
  if (!browser) {
    // If we don't have a browser, we can't handle the command.
    return false;
  }

  CefRefPtr<CefProcessMessage> message =
     CefProcessMessage::Create(name);
  CefRefPtr<CefListValue> message_args = message->GetArgumentList();

  // The first argument must be a callback function or null
  if (
    arguments.size() > 0
    && !arguments[0]->IsFunction()
    && !arguments[0]->IsNull()
     ) {
    std::string function_name = name;
    LOG(ERROR) << "Function called without callback param: "
               << function_name;

    return false;
  }

  // TODO(buglloc): separate to another handler?
  if (name == kSpellCheckWordName) {
    if (arguments.size() != 2 || !arguments[0]->IsFunction()) {
      LOG(ERROR) << "SpellChecking called without callback param";
      return false;
    }

    std::vector<CefString> suggestions;
    bool isCorrect = browser->SpellCheckWord(arguments[1]->GetStringValue(), suggestions);

    CefRefPtr<CefV8Value> v8_suggestions = CefV8Value::CreateArray(suggestions.size());
    for (uint i = 0; i < suggestions.size(); ++i) {
      v8_suggestions->SetValue(i, CefV8Value::CreateString(suggestions[i]));
    }

    CefRefPtr<CefV8Value> callbackFunction = arguments[0];
    CefV8ValueList args;
    args.push_back(CefV8Value::CreateBool(isCorrect));
    args.push_back(v8_suggestions);
    callbackFunction->ExecuteFunction(NULL, args);
    return true;
  }


  if (arguments.size() > 0 && arguments[0]->IsFunction()) {
    // The first argument is the message id
    cef_app_->AddCallback(messageId, CefV8Context::GetCurrentContext(), arguments[0]);
    helper::SetListValue(message_args, 0, CefV8Value::CreateInt(messageId));
  } else {
    helper::SetListValue(message_args, 0, CefV8Value::CreateNull());
  }

  // Pass the rest of the arguments
  for (unsigned int i = 1; i < arguments.size(); i++)
    helper::SetListValue(message_args, i, arguments[i]);
  browser->SendProcessMessage(PID_BROWSER, message);

  messageId++;
  return true;
}
