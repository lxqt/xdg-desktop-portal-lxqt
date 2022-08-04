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
$ make 
# make install
```
### Use LXQt filedialog in applications

* Firefox version 98 and higher:  Open in the address bar `about:config`, search for "portal" and set both `widget.use-xdg-desktop-portal.file-picker` and `widget.use-xdg-desktop-portal.mime-handler`  from `2` to `1`. Older versions of firefox are using the same setting as thunderbird.
* Thunderbird: Preferences > Configuration Editor `widget.use-xdg-desktop-portal` has to be set to `true`. 
* Some other GTK applications can be started with `GTK_USE_PORTAL=1 application`

A general use of `GTK_USE_PORTAL=1` in `~/.profile` or `/etc/profile` can lead to issues and
 is not recommended.
