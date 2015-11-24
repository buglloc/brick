// Copyright (c) 2015 The Brick Authors.

'use strict';

/*---------- Fake globals ---------*/
const TAB_CP = 1;
const TAB_B24NET = 2;
/*---------- Fake globals ---------*/

/*------ Ugly desktop globals ---------*/
// Window position :'(
const STP_LEFT = 0;
const STP_RIGHT = 1;
const STP_CENTER = 2;
const STP_TOP = 3;
const STP_BOTTOM = 4;
const STP_VCENTER = 5;
const STP_FRONT = 6;
/*------ Ugly desktop globals ---------*/

/*---------- App extension ---------*/
var BrickApp = {
  windowCallbacks: {},
  defaultNotifyDuration: 3000,
  authErrorCodes: {NONE:0, HTTP:1, CAPTCHA:2, OTP:3, AUTH:4, INVALID_URL:5, UNKNOWN:6},
  downloadStatus: {SUCCESS:0, FAILED:1, PROGRESS:2},
  downloadFailedReason: {NONE:0, UNKNOWN:1, ABORTED:2, HTTP:3, NOT_ATTACHMENT: 4},
  setWindowCallback: function(windowId, callback) {
    BrickApp.windowCallbacks[windowId] = callback;
  },
  getWindowCallback: function(windowId, callback) {
    let result = null;
    if (!BrickApp.windowCallbacks[windowId]) {
      console.error('BrickApp.getWindowCallback: undefined windowId, windowId: ' + windowId);
    } else {
      result = BrickApp.windowCallbacks[windowId];
      delete BrickApp.windowCallbacks[windowId];
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
          '/desktop_app/internals/web/pages/login-failed#' +
          '&code=' + error_code +
          '&reason=' + encodeURIComponent(reason)
      ;
    });

    AppExLogin(function(response, success, error_code, reason, bitrixSessid) {
      // TODO(buglloc): change additional arguments (error_code, reason, bitrixSessid) to dictionary
      if (success) {
        onSuccess(bitrixSessid);
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
    BrickApp.navigate('current_portal');
  },
  browse: function(url) {
    native function AppExBrowse();

    AppExBrowse(null, url);
  },
  showLogFile: function() {
    native function AppExShowLogFile();

    AppExShowLogFile();
  },
  requestDownload: function(url, filename) {
    native function AppExRequestDownload();

    AppExRequestDownload(null, url, filename || '');
  },
  restartDownload: function(id) {
    native function AppExRestartDownload();

    AppExRestartDownload(null, id);
  },
  cancelDownload: function(id) {
    native function AppExCancelDownload();

    AppExCancelDownload(null, id);
  },
  removeDownload: function(id) {
    native function AppExRemoveDownload();

    AppExRemoveDownload(null, id);
  },
  openDownloaded: function(id) {
    native function AppExOpenDownloaded();

    AppExOpenDownloaded(null, id);
  },
  showDownloaded: function(id) {
    native function AppExShowDownloaded();

    AppExShowDownloaded(null, id);
  },
  openDownloadFolder: function() {
    native function AppExOpenDownloadFolder();

    AppExOpenDownloadFolder();
  },
  listDownloads: function(callback) {
    native function AppExListDownloadHistory();

    AppExListDownloadHistory(function(error, list) {
      if (error) {
        console.error('Can\'t list download history. Error: ' + error);
        return;
      }

      var result = [];
      list.forEach(function(item) {
        result.push({
          'id': item[0],
          'name': item[1],
          'path': item[2],
          'url': item[3],
          'date': item[4],
          'status': item[5],
          'reason': item[6],
          'progress': item[7],
          'current': item[8],
          'size': item[9]
        });
      });

      callback(result);
    });
  },
  changeTooltip: function(text) {
    native function AppExChangeTooltip();

    AppExChangeTooltip(null, text);
  },
  loader: function(login) {
    window.location.href = '/desktop_app/internals/web/pages/portal-loader' + (login? '#login=yes': '');
  },
  setIdleIndicator: function(type) {
    native function AppExSetIdleIndicator();

    AppExSetIdleIndicator(null, type);
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
  showNotification: function(title, text, icon, jsId, isMessage, duration) {
    native function AppExShowNotification();

    duration = duration || this.defaultNotifyDuration;
    icon = icon || '';
    jsId = jsId || '';
    isMessage = isMessage || false;

    AppExShowNotification(
        null,
        title,
        text,
        icon,
        duration,
        jsId,
        isMessage
    );
  },
  hideNotification: function() {
    native function AppExHideNotification();

    AppExHideNotification(null);  
  },
  addTemporaryPage: function(content, callback) {
    native function AppExAddTemporaryPage();

    AppExAddTemporaryPage(callback, content);
  },
  openTopmostWindow: function(content, callback) {

    BrickApp.addTemporaryPage(content, function(response, url) {
      if (!url) {
        console.error('Can\'t add internal page for topmost window');
        return;
      }

      var newWindow = window.open(
          url,
          'topmost',
          'width=1,height=1,resizable=no,topmost'
      );

      if (callback) {
        callback(newWindow);
      }
    });
  },
  openWindow: function(name, callback) {
    var windowId = 'window' + Math.random();
    var windowHandle = window.open(
        '/desktop_app/internals/web/pages/empty?#name=' + name + '&id=' + windowId,
        name,
        // default popup sizes from Bitrix im.js
        'width=567,height=335,resizable=no'
    );

    BrickApp.setWindowCallback(windowId, callback);
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
  listDesktopMedia: function(callback, types) {
    native function AppExListDesktopMedia();

    types = types || ['screen', 'window'];

    var list_screens = types.indexOf('screen') != -1;
    var list_windows = types.indexOf('window') != -1;
    if (!list_screens && !list_windows) {
      callback([]);
      return;
    }

    AppExListDesktopMedia(function(error, list) {
      if (error)
        return;

      var result = [];
      list.forEach(function(media) {
        result.push({
          'id': media[0],
          'title': media[1],
          'preview': '/desktop_app/internals/desktop-media/' + media[0].replace(':', '-') + '.png?' + Math.random()
        });
      });

      callback(result);
    }, list_screens, list_windows);
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
  },
  editAccount: function(isSwitchNeeded) {
    native function AppExEditAccount();

    if (isSwitchNeeded !== true)
      isSwitchNeeded = false;

    AppExEditAccount(null, isSwitchNeeded);
  },
  loadScript: function(url) {
    var script = document.createElement('script');
    script.src = url;

    (document.head || document.documentElement).appendChild(script);
    script.onload = function () {
      script.parentNode.removeChild(script);
    };
  },
  loadScripts: function(urls) {
    urls.forEach(this.loadScript.bind(this));
  },
  preventShutdown: function() {
    native function AppExPreventShutdown();

    AppExPreventShutdown(null);
  },
  shutdown: function() {
    native function AppExShutdown();

    AppExShutdown(null);
  },
  switchAutostart(enable) {
    native function AppExSwitchAutostart();

    if (enable === void 0)
      enable = true;

    AppExSwitchAutostart(null, enable);
  }
};
/*---------- App extension ---------*/

/*---------- AppWindow extension ---------*/
var BrickWindow = {
  width: 0,
  height: 0,
  positions: {NorthWest: 0, North: 1, NorthEast: 2, West: 3, Center: 4, East: 5, SouthWest: 6, South: 7, SouthEast: 8},
  setSize: function(width, height) {
    native function AppWindowExSetSize();

    if (width == this.width || height == this.height)
      return;

    this.width = width;
    this.height = height;
    AppWindowExSetSize(
      null,
      parseInt(width),
      parseInt(height)
    );
  },
  setClientSize: function(width, height) {
    native function AppWindowExSetClientSize();

    if (width == this.width && height == this.height)
      return;

    this.width = width;
    this.height = height;
    AppWindowExSetClientSize(
      null,
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
  moveResize: function(position, width, height) {
    native function AppWindowExMoveResize();

    if (width == this.width && height == this.height)
      return;

    this.width = width;
    this.height = height;
    AppWindowExMoveResize(
        null,
        parseInt(position),
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

    AppWindowExSetClosable(null, !!isClosable);
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
      BrickHelper.implementMe('BXDesktopWindow.GetProperty', arguments);
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
      BrickWindow.setSize(value['Width'], value['Height']);
      break;
    case 'clientSize':
      BrickWindow.setClientSize(value['Width'], value['Height']);
      break;
    case 'minClientSize':
      BrickWindow.setMinClientSize(value['Width'], value['Height']);
      break;
    case 'position':
      var position = BrickHelper.translateOldPosition(value['X'], value['Y']);
      if (position === null) {
        console.error('Unknown position: ' + value['X'] + 'x' + value['Y']);
        break;
      }

      BrickWindow.moveResize(position, value['Width'], value['Height']);
      break;
    case 'resizable':
      BrickWindow.setResizable(value);
      break;
    case 'closable':
      BrickWindow.setClosable(value);
      break;
    default:
      BrickHelper.implementMe('BXDesktopWindow.SetProperty', arguments);
  }
};

BXDesktopWindow.ExecuteCommand = function(cmd, params) {
  switch (cmd) {
    case 'show':
    if(!opener)
        BrickWindow.showMain();
      else
        BrickWindow.show();
      break;
    case 'show.main':
      BrickWindow.showMain();
      break;
    case 'hide':
      BrickWindow.hide();
      break;
    case 'close':
      BrickWindow.close();
      break;
    case 'center':
      BrickWindow.center();
      break;
    case 'html.load':
      window.document.write(params);
      break;
    default:
      BrickHelper.implementMe('BXDesktopWindow.ExecuteCommand', arguments);
  }
};

BXDesktopSystem.DownloadStatus = BrickApp.downloadStatus;
BXDesktopSystem.DownloadFailedReason = BrickApp.downloadFailedReason;

BXDesktopSystem.ListScreenMedia = function(callback, types) {
  BrickApp.listDesktopMedia(callback, types);
};

BXDesktopSystem.SetProperty = function(name, value) {
  switch (name) {
    case 'autostart':
      BrickApp.switchAutostart(value);
      break;
    default:
      BrickHelper.implementMe('BXDesktopSystem.SetProperty', arguments);
  }
};

BXDesktopSystem.ParseNotificationHtml = function(html) {
  if (!this.hasOwnProperty('parser')) {
      this.parser = new DOMParser();
  }

  var document = this.parser.parseFromString(html, "text/html");

  var jsId = null;
  var isMessage = false;
  var notificationItem = document.body.querySelector('div.bx-notifier-item');
  if (notificationItem && html.indexOf('bxImClickNewMessage') > 0) {
    // Notification for user message
    isMessage = true;
    jsId = notificationItem.dataset.userid;
  } else if (notificationItem && html.indexOf('bxImClickNotify') > 0) {
    // "True" Notification
    isMessage = false;
    jsId = notificationItem.dataset.notifyid;
  }
  var icon = document.body.querySelector('img.bx-notifier-item-avatar-img');
  var date = document.body.querySelector('span.bx-notifier-item-date');
  var title = document.body.querySelector('span.bx-notifier-item-name');
  var text = document.body.querySelector('span.bx-notifier-item-text');


  var iconUri = null;
  if (icon !== null) {
    iconUri = BrickHelper.qualifyUrl(icon.getAttribute('src'));
  }

  return {
    'jsId': jsId !== void 0 ? jsId : null,
    'isMessage': isMessage,
    'icon': /blank\.gif$/.test(iconUri) ? null : iconUri, // Don't show blank.gif (1x1 px) as notification icon
    'date': date !== null ? date.innerHTML : '',
    'title': title !== null ? title.innerHTML.replace(/<[^>]+>/g, '') : '',
    'text': (text !== null ? text.innerHTML: html)
      .replace(/<br[^>]*>/gi, '\n').replace(/<p[^>]*>/gi, '\n').replace(/<[^>]+>/g, '').replace(/&nbsp;/g, ' ')
  }
};

BXDesktopSystem.ExecuteCommand = function(command, params) {
  switch (command) {
    case 'tooltip.change':
       // Tooltips intentionally disabled.
       // Useful information is not much, but a lot of problems for various desktop environments (e.g. Plasma).

       //BrickApp.changeTooltip(params);
       break;
    case 'notification.show.html':
      var parsed = this.ParseNotificationHtml(params);
      BrickApp.showNotification(
        parsed.title,
        parsed.text,
        parsed.icon,
        parsed.jsId,
        parsed.isMessage
      );
      break;
    case 'browse':
      BrickApp.browse(params);
      break;
    case 'topmost.show.html':
      // Fast&Ugly hack for non UTF-8 portals. Strange but they may set not UTF-8 encoding for the UTF-8 content
      var html = params.replace(/text\/html;\s?charset=[a-zA-Z0-9-]+/, 'text/html;charset=UTF-8');
      BrickApp.openTopmostWindow(html, function (newWindow) {
        BXWindows.push(newWindow);
        // Ugly hack for the official app logic (doesn't like asynchronous API) :'(
        if (BXIM !== void 0 && BXIM.desktop !== void 0) {
          BXIM.desktop.topmostWindow = newWindow;
        }
        if (BX !== void 0 && BX.desktop !== void 0) {
          BX.desktop.topmostWindow = newWindow;
        }
      });
      break;
    default:
      BrickHelper.implementMe('BXDesktopSystem.ExecuteCommand', arguments);
  }
};

BXDesktopSystem.QuerySettings = function(name, def) {
  return BrickApp.getOption(name, def);
};

BXDesktopSystem.StoreSettings = function(name, value) {
  return BrickApp.setOption(name, value);
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
  // Deal with ClientHandler::SendJSEvent
  if (Object.prototype.toString.call(params) == '[object String]')
    params = JSON.parse(params);

  var event = new CustomEvent(name, {"detail": params});
  try {
    window.dispatchEvent(event);
  } catch(e) {
    console.error('BXDesktopWindow.DispatchCustomEvent failed:' + name + ', handler exception: ' + e.message);
  }
};

BXDesktopSystem.GetProperty = function GetProperty(property) {
  var version = '#VERSION#'.split('~')[0];
  switch (property) {
    case 'versionParts':
      return version.split('.');
    case 'version':
      return version;
    case 'autostart':
      // TODO(buglloc): Implement...somehow...somewhere...
      return false;
    default:
      BrickHelper.implementMe('BXDesktopSystem.GetProperty', arguments);
  }
};

BXDesktopSystem.SetIconStatus = function(status) {
    BrickApp.setIdleIndicator(status);
};

BXDesktopSystem.FlashIcon = function(flag) {
  // We haven't any "flashing effects" so far
};

BXDesktopSystem.SetTabBadge = function(index, counter) {
  // We haven't any tabs
};

BXDesktopSystem.SetIconBadge = function(badge, important) {
  BrickApp.setIndicatorBadge(badge, important);
};

BXDesktopSystem.IsB24net = function IsB24net() {
  return false;
};

BXDesktopSystem.Login = function Login(params) {
  var success = null;
  if (params !== void(0) && params.success !== void(0)) {
    success = params.success;
  }

  BrickApp.login(success);
};

BXDesktopSystem.LoginForm = function(f) {
  BrickHelper.implementMe('BXDesktopSystem.LoginForm', arguments);
};

BXDesktopSystem.Logout = function(src) {
  BrickHelper.implementMe('BXDesktopSystem.Logout', arguments);
  // I don't known what mean "Logout" in this case, so just shutdown application.
  BXDesktopSystem.Shutdown();
};

BXDesktopSystem.GetMainWindow = function() {
  if (opener)
    return opener.BXDesktopWindow;

  if (window != top)
    return top.BXDesktopWindow;

  return BXDesktopWindow;
};

BXDesktopSystem.OpenLogsFolder = function() {
  BrickApp.showLogFile();
};

BXDesktopWindow.AddTrayMenuItem = function(obj) {
  BrickHelper.implementMe('BXDesktopWindow.AddTrayMenuItem', arguments);
};

BXDesktopWindow.EndTrayMenuItem = function() {
  BrickHelper.implementMe('BXDesktopWindow.EndTrayMenuItem', arguments);
};

BXDesktopWindow.ExecuteTrayMenuItem = function(id) {
  BrickHelper.implementMe('BXDesktopWindow.ExecuteTrayMenuItem', arguments);
};

BXDesktopSystem.BindSound = function(msg,url) {
  BrickHelper.implementMe('BXDesktopSystem.BindSound', arguments);
};

BXDesktopSystem.PlaySound = function(sound) {
  BrickHelper.implementMe('BXDesktopSystem.PlaySound', arguments);
};

BXDesktopSystem.GetWindow = function(name, callback) {
  BXWindows.push(BrickApp.openWindow(name, callback));
};

BXDesktopWindow.OpenDeveloperTools = function() {
  BrickWindow.openDeveloperTools();
};

BXDesktopSystem.CheckDebugBuild = function CheckDebugBuild() {
  return true;
};

BXDesktopSystem.PreventShutdown = function() {
  BrickApp.preventShutdown();
};

BXDesktopSystem.Shutdown = function() {
  BrickApp.shutdown();
};

BXDesktopSystem.ReportStorageNotification = function ReportStorageNotification(command, params) {
  return null;
};

BXDesktopSystem.ShowTab = function(index) {
  implementMe('BXDesktopSystem.ShowTab', arguments);
};

BXDesktopSystem.HideTab = function(index) {
  BrickHelper.implementMe('BXDesktopSystem.HideTab', arguments);
};

BXDesktopSystem.SetActiveTabNumber = function(index) {
  BrickHelper.implementMe('BXDesktopSystem.SetActiveTabNumber', arguments);
};

BXDesktopSystem.SetActiveTabI = function(index) {
  BrickHelper.implementMe('BXDesktopSystem.SetActiveTabI', arguments);
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
  BrickApp.navigate(url);
};

BXDesktopSystem.GetWorkArea = function() {
  BrickHelper.implementMe('BXDesktopSystem.GetWorkArea', arguments);
};

BXDesktopSystem.StartDownload = function(url, filename) {
  BrickApp.requestDownload(url, filename);
};

BXDesktopSystem.RestartDownload = function(id) {
  BrickApp.restartDownload(id);
};

BXDesktopSystem.CancelDownload = function(id) {
  BrickApp.cancelDownload(id);
};

BXDesktopSystem.RemoveDownload = function(id) {
  BrickApp.removeDownload(id);
};

BXDesktopSystem.OpenFileDownload = function(id) {
  BrickApp.openDownloaded(id);
};

BXDesktopSystem.OpenFolderDownload = function(id) {
  BrickApp.showDownloaded(id);
};

BXDesktopSystem.OpenDownloadFolder = function() {
  BrickApp.openDownloadFolder();
};

BXDesktopSystem.ListDownload = function(callback) {
  BrickApp.listDownloads(callback);
};

/*---------- Original API ---------*/

/*---------- Helpers ---------*/
var BrickHelper = {
  buildFunctionCall: function(name, args) {
    var preparedArgs = [].map.call(
      args,
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
  },
  responseLogger: function() {
    console.log('callback response: ' + this.buildFunctionCall(null, arguments));
  },
  methodLogger: function(method) {
    var args = [].slice.call(arguments);
    var response = this.buildFunctionCall(
        args.shift(),
        args
    );

    console.log('Method response: ' + response);
  },
  doNothing: function() {

  },
  encodeUtf8: function(text) {
    return unescape(encodeURIComponent(text));
  },
  decodeUtf8: function() {
    return decodeURIComponent(escape(text));
  },
  implementMe: function(name, args) {
    console.error('Implement me: ' + this.buildFunctionCall(name, args));
  },
  qualifyUrl: function(url) {
    if (/^https?:\/\//.test(url))
      return url;

    let a = document.createElement('a');
    a.href = url;
    return a.href;
  },
  translateOldPosition: function(x, y) {
    if (x == STP_LEFT && y == STP_TOP) {
      return BrickWindow.positions.NorthWest;
    } else if (x == STP_CENTER && y == STP_TOP) {
      return BrickWindow.positions.North;
    } else if (x == STP_RIGHT && y == STP_TOP) {
      return BrickWindow.positions.NorthEast;
    } else if (x == STP_LEFT && y == STP_VCENTER) {
      return BrickWindow.positions.West;
    } else if (x == STP_CENTER && y == STP_VCENTER) {
      return BrickWindow.positions.Center;
    } else if (x == STP_RIGHT && y == STP_VCENTER) {
      return BrickWindow.positions.East;
    } else if (x == STP_LEFT && y == STP_BOTTOM) {
      return BrickWindow.positions.SouthWest;
    } else if (x == STP_CENTER && y == STP_BOTTOM) {
      return BrickWindow.positions.South;
    } else if (x == STP_RIGHT && y == STP_BOTTOM) {
      return BrickWindow.positions.SouthEast;
    }

    return null;
  }
};
/*---------- Helpers ---------*/

/*---------- Disk API ---------*/

// Disable disk at all
var BXFileStorage = undefined;

/*---------- Disk API ---------*/