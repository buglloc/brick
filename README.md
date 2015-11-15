Brick
=====

An open source [Bitrix24](https://www.bitrix24.com/) messenger client.

Supported features:
  - multiple accounts (switch tab)
  - chat
  - notifications (including Unity Launcher integration)
  - voice & video calls
  - desktop/window sharing (experimental)
  - file sharing
  - simple integration with external systems based on D-Bus


Project is currently under (rather) heavy development, so stay tuned, more features will come soon.
**Currently supported version Bitrix24 of "Instant Messenger" module 15.1.5 and above**

## Some screenshots

Chat:
![Chat](/doc/chat.png?raw=true&v=2)

Video call/Screen Sharing:
![Video](/doc/video.png?raw=true)

System notification:
![Notify](/doc/notify.png?raw=true)


## How to install and run Brick

### PPA for Ubuntu
You can install Brick from PPA [buglloc/brick](https://launchpad.net/~buglloc/+archive/ubuntu/brick):
```bash
sudo add-apt-repository ppa:buglloc/brick
    Hit [Enter]
sudo apt-get update
sudo apt-get install brick
```
if you are using KDE5 your may want to install `libappindicator`

***Ubuntu Precise is no longer supports***

### AUR for Arch Linux
You can install Brick from AUR [package](https://aur.archlinux.org/packages/brick/):
```bash
yaourt -S brick
```

### Copr for CentOS/Fedora
You can install Brick from Copr [buglloc/Brick](https://copr.fedoraproject.org/coprs/buglloc/Brick/).

For example, Fedora:
```plain
sudo dnf copr enable buglloc/Brick -y
sudo dnf install brick -y
```
if you are using KDE5 your may want to install `libappindicator`

### OBS for openSUSE
You can install Brick from openSUSE Software [home:buglloc/brick](https://build.opensuse.org/package/show/home:buglloc/brick).

For openSUSE 13.2:
```plain
sudo zypper addrepo http://download.opensuse.org/repositories/home:buglloc/openSUSE_13.2/home:buglloc.repo
sudo zypper refresh
sudo zypper install brick
```
For openSUSE 13.1:
```plain
sudo zypper addrepo http://download.opensuse.org/repositories/home:buglloc/openSUSE_13.1/home:buglloc.repo
sudo zypper refresh
sudo zypper install brick
```
if you are using KDE5 your may want to install `libappindicator`

### Build manually
Or you can build Brick from sources:
```bash
# Install some dependencies and build tools
sudo apt-get install build-essential cmake
sudo apt-get install libgtk2.0-dev libudev1 libnss3 libgconf-2-4 libnotify-dev libxss-dev

# Let's make it
mkdir ~/tmp && cd ~/tmp
git clone --recursive https://github.com/buglloc/brick.git
mkdir build && cd build
cmake ../brick
make
sudo make install

# And run it!
brick
```
if you are using KDE5 your may want to install `libappindicator`

##  Contributing

Any kind of contribution would be very very welcome. Check out the issue tracker or contact me directly.

## License

```
Copyright © 2015 Andrew Krasichkov <buglloc@yandex.ru>
This work is free. You can redistribute it and/or modify it under the
terms of the Do What The Fuck You Want To Public License, Version 2,
as published by Sam Hocevar. See the LICENSE.txt file for more details.
```
