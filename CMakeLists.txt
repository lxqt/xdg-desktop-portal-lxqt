cmake_minimum_required(VERSION 3.18.0)

set(PROJECT_VERSION "1.0.0")

project(xdg-desktop-portal-lxqt VERSION ${PROJECT_VERSION})

set(QT_MIN_VERSION "6.6.0")
set(LIBFMQT_MINIMUM_VERSION "2.0.0")
set(KF6_MIN_VERSION "6.0.0")

set(CMAKE_AUTOMOC on)

include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Widgets
)
find_package(Qt5X11Extras ${QT_MINIMUM_VERSION} REQUIRED)  # seems to be required by fm-qt
find_package(fm-qt6 ${LIBFMQT_MINIMUM_VERSION} REQUIRED)
find_package(KF6WindowSystem ${KF6_MIN_VERSION} REQUIRED)

add_subdirectory(data)
add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
