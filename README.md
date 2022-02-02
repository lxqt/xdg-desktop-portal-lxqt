# xdg-desktop-portal-lxqt

A backend implementation for [xdg-desktop-portal](http://github.com/flatpak/xdg-desktop-portal)
that is using Qt/KF5/libfm-qt.

## Building xdg-desktop-portal-lxqt

### Dependencies:
- Build + Runtime
  - Qt 5
  - KDE Frameworks - KWindowSystem
  - libfm-qt
- Runtime only
  - Qt 5
  - KDE Frameworks - KWindowSystem
  - xdg-desktop-portal
  - libfm-qt

### Build instructions:
```
$ mkdir build && cd build
$ cmake .. [your_options]
$ make -j5
$ make install
```
