cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "0.1.0")

project(xdg-desktop-portal-lxqt VERSION ${PROJECT_VERSION})

set(QT_MIN_VERSION "5.15.0")
set(LIBFMQT_MINIMUM_VERSION "1.0.0")
set(KF5_MIN_VERSION "5.78")

set(CMAKE_AUTOMOC on)

include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Widgets
)
find_package(Qt5X11Extras ${QT_MINIMUM_VERSION} REQUIRED)  # seems to be required by fm-qt
find_package(fm-qt ${LIBFMQT_MINIMUM_VERSION} REQUIRED)
find_package(KF5WindowSystem ${KF5_MIN_VERSION} REQUIRED)

add_subdirectory(data)
add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
