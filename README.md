Brick
=====

Unofficial, simple [Bitrix24](https://www.bitrix24.com/) messenger client.

Project is currently under (rather) heavy development, so stay tuned, more features will come soon.
**Currently supported version Bitrix24 of "Instant Messenger" module 15.1.5 and above**

## Some screenshots

![Chat](/doc/chat.png?raw=true&v=2)
![Video](/doc/video.png?raw=true)
![Notify](/doc/notify.png?raw=true)

## How to install and run Brick

### PPA for Ubuntu
You can install Brick from PPA [buglloc/brick](https://launchpad.net/~buglloc/+archive/ubuntu/brick):
```bash
sudo add-apt-repository ppa:buglloc/brick
    Hit [Enter]
sudo apt-get update
# If you are using Unity (included appindicator support etc.)
sudo apt-get install brick-unity
# Otherwise
sudo apt-get install brick
```

### AUR for Arch Linux
You can install Brick from AUR [https://aur.archlinux.org/packages/brick/](https://aur.archlinux.org/packages/brick/):
```bash
yaourt brick
```
Brick for Unity doesn't supported so far. May be later:)

### Build manualy
Or you can build Brick from sources:
```bash
# Install some dependencies and build tools
sudo apt-get install build-essential cmake
sudo apt-get install libcurl3-openssl-dev libgtk2.0-dev libudev libnss3 libgconf-2-4 libnotify-dev libxss-dev
# Install this only if you are using Unity
sudo apt-get install libappindicator-dev libunity-dev

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

##  Contributing

Any kind of contribution would be very very welcome. Check out the issue tracker or contact me directly.

## License

```
Copyright © 2015 Andrew Krasichkov <buglloc@yandex.ru>
This work is free. You can redistribute it and/or modify it under the
terms of the Do What The Fuck You Want To Public License, Version 2,
as published by Sam Hocevar. See the LICENSE.txt file for more details.
```
