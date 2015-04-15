#include <include/cef_url.h>
#include "helper.h"

namespace helper {
    // Transfer a V8 value to a List index.
    void SetListValue(CefRefPtr<CefListValue> list, int index,
       CefRefPtr<CefV8Value> value) {
      if (value->IsArray()) {
        CefRefPtr<CefListValue> new_list = CefListValue::Create();
        SetList(value, new_list);
        list->SetList(index, new_list);
      } else if (value->IsString()) {
        list->SetString(index, value->GetStringValue());
      } else if (value->IsBool()) {
        list->SetBool(index, value->GetBoolValue());
      } else if (value->IsInt()) {
        list->SetInt(index, value->GetIntValue());
      } else if (value->IsDouble()) {
        list->SetDouble(index, value->GetDoubleValue());
      }
    }

// Transfer a V8 array to a List.
    void SetList(CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target) {
      ASSERT(source->IsArray());

      int arg_length = source->GetArrayLength();
      if (arg_length == 0)
        return;

      // Start with null types in all spaces.
      target->SetSize(arg_length);

      for (int i = 0; i < arg_length; ++i)
        SetListValue(target, i, source->GetValue(i));
    }

    CefRefPtr<CefV8Value> ListValueToV8Value(CefRefPtr<CefListValue> value, int index) {
      CefRefPtr<CefV8Value> new_value;

      CefValueType type = value->GetType(index);

      switch (type) {
        case VTYPE_LIST: {
          CefRefPtr<CefListValue> list = value->GetList(index);
          new_value = CefV8Value::CreateArray(list->GetSize());
          SetList(list, new_value);
        } break;
        case VTYPE_BOOL:
          new_value = CefV8Value::CreateBool(value->GetBool(index));
          break;
        case VTYPE_DOUBLE:
          new_value = CefV8Value::CreateDouble(value->GetDouble(index));
          break;
        case VTYPE_INT:
          new_value = CefV8Value::CreateInt(value->GetInt(index));
          break;
        case VTYPE_STRING:
          new_value = CefV8Value::CreateString(value->GetString(index));
          break;
        default:
          new_value = CefV8Value::CreateNull();
          break;
      }

      return new_value;
    }

    // Transfer a List value to a V8 array index.
    void SetListValue(CefRefPtr<CefV8Value> list, int index,
       CefRefPtr<CefListValue> value) {
      CefRefPtr<CefV8Value> new_value;

      CefValueType type = value->GetType(index);
      switch (type) {
        case VTYPE_LIST: {
          CefRefPtr<CefListValue> list = value->GetList(index);
          new_value = CefV8Value::CreateArray(list->GetSize());
          SetList(list, new_value);
        } break;
        case VTYPE_BOOL:
          new_value = CefV8Value::CreateBool(value->GetBool(index));
          break;
        case VTYPE_DOUBLE:
          new_value = CefV8Value::CreateDouble(value->GetDouble(index));
          break;
        case VTYPE_INT:
          new_value = CefV8Value::CreateInt(value->GetInt(index));
          break;
        case VTYPE_STRING:
          new_value = CefV8Value::CreateString(value->GetString(index));
          break;
        default:
          break;
      }

      if (new_value.get()) {
        list->SetValue(index, new_value);
      } else {
        list->SetValue(index, CefV8Value::CreateNull());
      }
    }

    // Transfer a List to a V8 array.
    void SetList(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target) {
      ASSERT(target->IsArray());

      int arg_length = source->GetSize();
      if (arg_length == 0)
        return;

      for (int i = 0; i < arg_length; ++i)
        SetListValue(target, i, source);
    }

    std::string string_replace(std::string src, std::string const& target, std::string const& repl) {
      // handle error situations/trivial cases

      if (!target.length()) {
        // searching for a match to the empty string will result in
        //  an infinite loop
        //  it might make sense to throw an exception for this case
        return src;
      }

      if (!src.length()) {
        return src;  // nothing to match against
      }

      size_t idx = 0;

      for (;;) {
        idx = src.find( target, idx);
        if (idx == std::string::npos)  break;

        src.replace( idx, target.length(), repl);
        idx += repl.length();
      }

      return src;
    }

    // Retrieve the file name and mime type based on the specified url.
    bool
    ParseUrl(const std::string& url, std::string* file_name, std::string* mime_type) {
      // Retrieve the path component.
      CefURLParts parts;
      CefParseURL(url, parts);
      std::string file = CefString(&parts.path);
      if (file.size() < 2)
        return false;

      // Verify that the file name is valid.
      if (file.find("..") != std::string::npos)
        return false;

      for(size_t i = 0; i < file.size(); ++i) {
        const char c = file[i];
        if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-' && c != '.' && c != '/') {
          return false;
        }
      }

      // Determine the mime type based on the file extension, if any.
      std::string ext = GetFileExtension(file);
      if (!ext.empty()) {
        if (ext == "html")
          *mime_type = "text/html";
        else if (ext == "png")
          *mime_type = "image/png";
        else if (ext == "jpg")
          *mime_type = "image/jpeg";
        else if (ext == "gif")
          *mime_type = "image/gif";
        else if (ext == "js")
          *mime_type = "text/javascript";
        else if (ext == "css")
          *mime_type = "text/css";
        else
          return false;
      } else {
        // Default to an html extension if none is specified.
        *mime_type = "text/html";
        file += ".html";
      }

      *file_name = file;
      return true;
    }

    unsigned int
    HashString(const std::string& str) {
      // Currently based on FNV-1 with 32 bit prime
      const unsigned int fnv_prime = 0x811C9DC5;
      unsigned int hash = 0;
      for(std::size_t i = 0; i < str.length(); i++) {
        hash *= fnv_prime;
        hash ^= str[i];
      }

      return hash;
    }

    std::string
    BaseDir(const std::string& source) {
      if (source.size() <= 1) {
        //Make sure it's possible to check the last character.
        return source;
      }

      std::string result;
      if (*(source.rbegin() + 1) == '/') {
        //Remove trailing slash if it exists.
        result = source.substr(0, source.length() - 1);
      } else {
        result = source;
      }

      size_t pos = result.find_last_of('/');
      if (pos == std::string::npos)
        return source;

      return result.substr(0, pos);
    }

    std::string
    GetFileExtension(const std::string& source) {
      size_t pos = source.rfind(".");
      if (pos != std::string::npos) {
        return source.substr(pos + 1);
      }

      return "";
    }

}