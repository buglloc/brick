#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import dbus
from dbus import glib
import argparse


class BrickClient(object):

    def __init__(self):
        glib.init_threads()
        dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
        self._bus = dbus.SessionBus()
        self._remote_app = None
        self._app = None
        self._remote_app_window = None
        self._app_window = None

    @property
    def is_running(self):
        return self._bus.name_has_owner('org.brick.Brick')

    @property
    def remote_app(self):
        if not self._remote_app:
            self._remote_app = self._bus.get_object('org.brick.Brick', '/org/brick/Brick/App')

        return self._remote_app

    @property
    def app(self):
        if not self._app:
            self._app = dbus.Interface(
                self.remote_app,
                'org.brick.Brick.AppInterface'
            )

        return self._app

    @property
    def remote_app_window(self):
        if not self._remote_app_window:
            self._remote_app_window = self._bus.get_object('org.brick.Brick', '/org/brick/Brick/AppWindow')

        return self._remote_app_window

    @property
    def app_window(self):
        if not self._app_window:
            self._app_window = dbus.Interface(
                self.remote_app_window,
                'org.brick.Brick.AppWindowInterface'
            )

    @staticmethod
    def on_indicator_badge(badge, is_important):
        print('Indicator badge changed: {badge} ({notice})'.format(
            badge=badge,
            notice='important' if is_important else 'regular'
        ))

    @staticmethod
    def on_indicator_state(state):
        print('Indicator got state: {}'.format(state))

    @staticmethod
    def on_indicator_tooltip(tooltip):
        print('Indicator got new tooltip: {}'.format(tooltip.encode('UTF-8')))

    def command_present(self):
        self.app_window.Present()

    def command_hide(self):
        self.app_window.Hide()

    def command_toggle(self):
        self.app_window.ToggleVisibility()

    def command_user_away(self):
        self.app.UserAway()
    
    def command_user_present(self):
        self.app.UserPresent()

    def command_introspect(self):
        print('--==App==--\n')
        dbus_iface = dbus.Interface(self.remote_app, 'org.freedesktop.DBus.Introspectable')
        print(dbus_iface.Introspect())
        print('\n--==Window==--\n')
        dbus_iface = dbus.Interface(self.remote_app_window, 'org.freedesktop.DBus.Introspectable')
        print(dbus_iface.Introspect())

    def command_watch(self):
        import gobject
        
        print('Starts signals watching...')
        self.app.connect_to_signal('IndicatorBadgeChanged', self.on_indicator_badge)
        self.app.connect_to_signal('IndicatorStateChanged', self.on_indicator_state)
        self.app.connect_to_signal('IndicatorTooltipChanged', self.on_indicator_tooltip)
        loop = gobject.MainLoop()
        loop.run()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='''Simple D-BUS client for Brick.\nCan send simple command (see below) and watch indicator signals.'''
    )
    parser.add_argument(
        'command',
        choices=['preset', 'hide', 'toggle', 'user_away', 'user_present', 'watch', 'introspect']
    )
    args = parser.parse_args()

    brick = BrickClient()
    if brick.is_running:
        try:
            method = getattr(brick, 'command_{}'.format(args.command))
            method()
        except AttributeError:
            print('Command not found...')
    else:
        print("Brick doesn't run")