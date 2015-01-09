
/*---------- Fake globals ---------*/
var BXIM = BXIM || {};
var TAB_CP = 1;
var TAB_B24NET = 2;
/*---------- Fake globals ---------*/

/*---------- App extension ---------*/
var app = {
  windowCallbacks: {},
  defaultNotifyDuration: 3000,
  authErrorCodes: {NONE:0, HTTP:1, CAPTCHA:2, OTP:3, UNKNOWN:4},
  setWindowCallback: function(windowId, callback) {
    app.windowCallbacks[windowId] = callback;
  },
  getWindowCallback: function(windowId, callback) {
    var result = null;
  if (!app.windowCallbacks[windowId]) {
      console.error('app.getWindowCallback: undefined windowId, windowId: ' + windowId);
    } else {
      result = app.windowCallbacks[windowId];
      delete app.windowCallbacks[windowId];
    }

    return result;
  },
  login: function(onSuccess, onFailure) {
    native function AppExLogin();

    onSuccess = onSuccess || (function() {
      window.location.reload();
    });

    onFailure = onFailure || (function(error_code, reason) {
      window.location.href =
          '/desktop_app/internals/pages/login-failed#' +
          '&code=' + error_code +
          '&reason=' + encodeURIComponent(reason)
      ;
    });

    AppExLogin(function(response, success, error_code, reason) {
        if (success) {
          onSuccess();
        } else {
          onFailure(error_code, reason);
        }
    }.bind(this));
  },
  navigate: function(url) {
    native function AppExNavigate();

    AppExNavigate(null, url);
  },
  loadPortal: function() {
    app.navigate('current_portal');
  },
  browse: function(url) {
    native function AppExBrowse();

    AppExBrowse(null, url);
  },
  changeTooltip: function(text) {
    native function AppExChangeTooltip();

    AppExChangeTooltip(null, text);
  },
  loader: function() {
    window.location.href = '/desktop_app/internals/pages/portal-loader';
  },
  setIndicator: function(type) {
    native function AppExSetIndicator();

    AppExSetIndicator(null, type);
  },
  setIndicatorBadge: function(badge, isImportant) {
    native function AppExIndicatorBadgee();

    AppExIndicatorBadgee(
        null,
        Math.floor(badge),
        !!isImportant
    );
  },
  showNotification: function(title, text, duration) {
    native function AppExShowNotification();

    duration = duration || this.defaultNotifyDuration;
    AppExShowNotification(null, title, text, duration);  
  },
  hideNotification: function() {
    native function AppExHideNotification();

    AppExHideNotification(null);  
  },
  openWindow: function(name, callback) {
    var windowId = 'window' + Math.random();
    var windowHandle = window.open(
        '/desktop_app/internals/pages/empty?#name=' + name + '&id=' + windowId,
        name,
        'width=300,height=300,resizable=no'
    );

    app.setWindowCallback(windowId, callback);
    return windowHandle;
  },
  getOption: function(name, defaultValue) {
    var value = localStorage.getItem(name);
    if (value != null) {
      return value;
    } else {
      return defaultValue;
    }
  },
  setOption: function(name, value) {
    localStorage.setItem(name, value);
    return true;
  },
  getHashValue: function(key) {
    var matches = location.hash.match(new RegExp(key+'=([^&]*)'));
    return matches ? matches[1] : null;
  },
  addAccount: function(isSwitchNeeded) {
    native function AppExAddAccount();

    if (isSwitchNeeded !== true)
      isSwitchNeeded = false;

    AppExAddAccount(null, isSwitchNeeded);
  }
};
/*---------- App extension ---------*/

/*---------- AppWindow extension ---------*/
var appWindow = {
  width: 0,
  height: 0,
  setSize: function(width, height) {
    native function AppWindowExSetSize();

    if (width == this.width || height == this.height)
      return;

    this.width = width;
    this.height = height;
    AppWindowExSetSize(
      methodLogger.bind(null, 'appWindow.setSize'),
      parseInt(width),
      parseInt(height)
    );
  },
  setClientSize: function(width, height) {
    native function AppWindowExSetClientSize();

    if (width == this.width || height == this.height)
      return;

    this.width = width;
    this.height = height;
    AppWindowExSetClientSize(
      methodLogger.bind(null, 'appWindow.setClientSize'),
      parseInt(width),
      parseInt(height)
    );
  },
  setMinClientSize: function(width, height) {
    native function AppWindowExSetMinClientSize();

    AppWindowExSetMinClientSize(
      null,
      parseInt(width),
      parseInt(height)
    );
  },
  setResizable: function(isResizable) {
    native function AppWindowExFixSize();

    if (isResizable)
      AppWindowExFixSize(null, 0, 0);
    else
      AppWindowExFixSize(null, this.width, this.height);
  },
  setClosable: function(isClosable) {
    native function AppWindowExSetClosable();

    AppWindowExSetClosable(null, isClosable || true);
  },
  showMain: function() {
    native function AppWindowExShowMain();
    
    AppWindowExShowMain(null);
  },
  show: function() {
    native function AppWindowExShow();

    AppWindowExShow(null);
  },
  hide: function() {
    native function AppWindowExHide();

    AppWindowExHide(null);
  },
  close: function() {
    window.close();
  },
  center: function() {
    native function AppWindowExCenter();

    AppWindowExCenter(null);
  },
  openDeveloperTools: function() {
    native function AppWindowExOpenDeveloperTools();

    AppWindowExOpenDeveloperTools(null);
  },
  loadScript: function(url) {
    var script = document.createElement('script');
    script.src = url;

    (document.head || document.documentElement).appendChild(script);
    script.onload = function () {
      script.parentNode.removeChild(script);
    };
  }
};
/*---------- AppWindow extension ---------*/

/*---------- Original API ---------*/
var BXDesktopWindow = BXDesktopWindow || {};
var BXDesktopSystem = BXDesktopSystem || {};
var BXWindows = BXWindows || [];

BXDesktopWindow.GetProperty = function(name) {
    var result = null;
    switch (name) {
        case 'isForeground':
            result = true;
            break;
        default:
          implementMe('BXDesktopWindow.GetProperty', arguments);
            break;
    }
  return result;
};

BXDesktopWindow.SetProperty = function(name,value) {    
  switch (name) {
    case 'title':
      window.document.title = value;
      break;
    case 'Size':
    case 'size':
      appWindow.setSize(value['Width'], value['Height']);
      break;
    case 'clientSize':
      appWindow.setClientSize(value['Width'], value['Height']);
      break;
    case 'minClientSize':
      appWindow.setMinClientSize(value['Width'], value['Height']);
      break;
    case 'resizable':
      appWindow.setResizable(value);
      break;
    case 'closable':
      appWindow.setClosable(value);
      break;
    default:
    implementMe('BXDesktopWindow.SetProperty', arguments);
  }
};

BXDesktopWindow.ExecuteCommand = function(cmd, params) {
  switch (cmd) {
    case 'show':
    if(!opener)
        appWindow.showMain();
      else
        appWindow.show();
      break;
    case 'show.main':
      appWindow.showMain();
      break;
    case 'hide':
      appWindow.hide();
      break;
    case 'close':
      appWindow.close();
      break;
    case 'center':
      appWindow.center();
      break;
    case 'html.load':
      window.document.write(params);
      break;
    default: 
    implementMe('BXDesktopWindow.ExecuteCommand', arguments);
  }
};

BXDesktopSystem.SetProperty = function(name, value) {
  implementMe('BXDesktopSystem.SetProperty', arguments);
};

BXDesktopSystem.ParseNotificationHtml = function(html) {
  var date = /(<span class="bx-notifier-item-date">)([\s\S]+?)(<\/span>)/.exec(html)
  var title = /(<span class="bx-notifier-item-name">)([\s\S]+?)(<\/span>)/.exec(html)
  var text = /(<span class="bx-notifier-item-text">)([\s\S]+?)(<\/span>)/.exec(html)

  return {
    'date': date[2],
    'title': title[2].replace(/<[^>]+>/g, ''),
    'text': text[2].replace(/<[^>]+>/g, '')
  }
};

BXDesktopSystem.ExecuteCommand = function(command, params) {
  switch (command) {
    case 'tooltip.change':
       app.changeTooltip(params);
       break;
    case 'notification.show.html':
      var parsed = this.ParseNotificationHtml(params);
      app.showNotification(
        parsed.title,
        parsed.text
      );
      break;
    case 'browse':
      app.browse(params);
      break;
    default:
     implementMe('BXDesktopSystem.ExecuteCommand', arguments);
  }
};

BXDesktopSystem.QuerySettings = function(name, def) {
  return app.getOption(name, def);
};

BXDesktopSystem.StoreSettings = function(name, value) {
  return app.setOption(name, value);
};

BXDesktopSystem.LogInfo = function LogInfo(value) {
  console.log('Info: ' + value);
};

BXDesktopSystem.Log = function(file, text) {
  console.log(file + ': ' + text);
};

BXDesktopSystem.videoLog = function(value) {
  console.log('Video: ' + value);
};

BXDesktopWindow.DispatchCustomEvent = function(name, params) {
    var event = new Event(name);
    event.detail = params;
    try {
      window.dispatchEvent(event);
    } catch(e) {
      console.error('BXDesktopWindow.DispatchCustomEvent failed:' + name + ', handler exception: ' + e.message);
    }
};

BXDesktopSystem.GetProperty = function GetProperty(property) {
  switch (property) {
    case 'versionParts':
      return '#VERSION#'.split('.');
    case 'version':
      return '#VERSION#';
    default:
    implementMe('BXDesktopSystem.GetProperty', arguments);
  }
};

BXDesktopSystem.SetIconStatus = function(status) {
    app.setIndicator(status);
};

BXDesktopSystem.FlashIcon = function(flag) {
  // We haven't any "flashing effects" so far
};

BXDesktopSystem.SetTabBadge = function(index, counter) {
  // We haven't any tabs
};

BXDesktopSystem.SetIconBadge = function(badge, important) {
  app.setIndicatorBadge(badge, important);
};

BXDesktopSystem.IsB24net = function IsB24net() {
  return false;
};

BXDesktopSystem.Login = function Login(f) {
    app.login();
};

BXDesktopSystem.LoginForm = function(f) {
  implementMe('BXDesktopSystem.LoginForm', arguments);
};

BXDesktopSystem.Logout = function(src) {
  implementMe('BXDesktopSystem.Logout', arguments);
};

BXDesktopSystem.GetMainWindow = function() {
  if (opener)
    return opener.BXDesktopWindow;

  if (window != top)
    return top.BXDesktopWindow;

  return BXDesktopWindow;
};

BXDesktopSystem.OpenLogsFolder = function() {
  implementMe('BXDesktopSystem.OpenLogsFolder', arguments);
};

BXDesktopWindow.AddTrayMenuItem = function(obj) {
  implementMe('BXDesktopWindow.AddTrayMenuItem', arguments);
};

BXDesktopWindow.EndTrayMenuItem = function() {
  implementMe('BXDesktopWindow.EndTrayMenuItem', arguments);
};

BXDesktopWindow.ExecuteTrayMenuItem = function(id) {
  implementMe('BXDesktopWindow.ExecuteTrayMenuItem', arguments);
};

BXDesktopSystem.BindSound = function(msg,url) {
  implementMe('BXDesktopSystem.BindSound', arguments);
};

BXDesktopSystem.PlaySound = function(sound) {
  implementMe('BXDesktopSystem.PlaySound', arguments);
};

BXDesktopSystem.GetWindow = function(name, callback) {
  BXWindows.push(app.openWindow(name, callback));
};

BXDesktopWindow.OpenDeveloperTools = function() {
  appWindow.openDeveloperTools();
};

BXDesktopSystem.CheckDebugBuild = function CheckDebugBuild() {
  return true;
};

BXDesktopSystem.PreventShutdown = function() {
  implementMe('BXDesktopSystem.PreventShutdown', arguments);
};

BXDesktopSystem.ReportStorageNotification = function ReportStorageNotification(command, params) {
  return null;
};

BXDesktopSystem.Shutdown = function() {
  implementMe('BXDesktopSystem.Shutdown', arguments);
};

BXDesktopSystem.ShowTab = function(index) {
  implementMe('BXDesktopSystem.ShowTab', arguments);
};

BXDesktopSystem.HideTab = function(index) {
  implementMe('BXDesktopSystem.HideTab', arguments);
};

BXDesktopSystem.SetActiveTabNumber = function(index) {
  implementMe('BXDesktopSystem.SetActiveTabNumber', arguments);
};

BXDesktopSystem.SetActiveTabI = function(index) {
  implementMe('BXDesktopSystem.SetActiveTabI', arguments);
};

BXDesktopSystem.ActiveTab = function ActiveTab() {
  return TAB_CP;
};

BXDesktopSystem.IsActiveTab = function IsActiveTab() {
  return true;
};

BXDesktopSystem.SetActiveTab = function SetActiveTab() {
  return false;
};

BXDesktopSystem.Navigate = function(index, url) {
  app.navigate(url);
};

BXDesktopSystem.GetWorkArea = function() {
  implementMe('BXDesktopSystem.GetWorkArea', arguments);
};
/*---------- Original API ---------*/

/*---------- Helpers ---------*/
function buildFunctionCall(name, arguments) {
  var preparedArgs = [].map.call(
    arguments,
    function prepareArguments(item) {
      switch (typeof(item)) {
        case 'object':
          return JSON.stringify(item);
        case 'null':
        case 'undefined':  
          return 'null';
        default:
          return '' + item
      }
    }
  );

  preparedArgs = preparedArgs.join(', ');

  if (name)
    return name + '(' + preparedArgs + ')';
  else
    return preparedArgs;
}

function responseLogger() {
  console.log('callback response: ' + buildFunctionCall(null, arguments));
}

function methodLogger(method) {
  var args = [].slice.call(arguments);
  var response = buildFunctionCall(
    args.shift(),
    args
  );

  console.log('Method response: ' + response);
}

function doNothing() {
}

function encode_utf8(text) {
  return decodeURIComponent(encodeURIComponent(text));
}

function decode_utf8(text) {
  return decodeURIComponent(encodeUriComponent(text));
}

function implementMe(name, args) {
  console.error('Implement me: ' + buildFunctionCall(name, args));
}

/*---------- Helpers ---------*/

/*---------- Disk API ---------*/
var BXFileStorage = undefined; // Disable disk at all

//var BXFileStorage = BXFileStorage || {};
//
//BXFileStorage.StopBDisk = function StopBDisk() {
//    return null;
//};
//
//BXFileStorage.StartBDisk = function StartBDisk(mode) {
//    return null;
//};
//
//BXFileStorage.GetStatus = function GetStatus() {
//    return null;
//};
//
//BXFileStorage.OpenFolder = function OpenFolder() {
//    return null;
//};
//
//BXFileStorage.OpenFileFolder = function OpenFileFolder(path) {
//    return null;
//};
//
//BXFileStorage.SetTargetFolder = function SetTargetFolder(path) {
//    return null;
//};
//
//BXFileStorage.SetDefaultTargetFolder = function SetDefaultTargetFolder(callback) {
//    return null;
//};
//
//BXFileStorage.GetTargetFolder = function GetTargetFolder() {
//    return null;
//};
//
//BXFileStorage.SelectTargetFolder = function SelectTargetFolder(callback) {
//    return null;
//};
//
//BXFileStorage.Attach = function Attach(disk_token) {
//    return null;
//};
//
//BXFileStorage.Detach = function Detach() {
//    return null;
//};
//
//BXFileStorage.FileExist = function FileExist(path, funcCallback) {
//    return null;
//};
//
//BXFileStorage.FolderExist = function FolderExist(path, funcCallback) {
//    return null;
//};
//
//BXFileStorage.FileOpen = function FileOpen(path, funcCallback) {
//    return null;
//};
//
//BXFileStorage.ObjectOpen = function ObjectOpen(path, funcCallback) {
//    return null;
//};
//
//BXFileStorage.EditFile = function EditFile(downloadLink, uploadLink,filename) {
//    return null;
//};
//
//BXFileStorage.ViewFile = function ViewFile(downloadLink, filename) {
//    return null;
//};
//
//BXFileStorage.Refresh = function Refresh(funcCallback) {
//    return null;
//};
//
//BXFileStorage.GetStorageSize = function GetStorageSize(funcCallback) {
//    return null;
//};
//
//BXFileStorage.GetSyncFilesInfo = function GetSyncFilesInfo() {
//    return null;
//};
//
//BXFileStorage.GetLogLastOperations = function GetLogLastOperations() {
//    return null;
//};
//
//BXFileStorage.GetObjectDataById = function GetObjectDataById(id) {
//    return null;
//};

/*---------- Disk API ---------*/