set(SRCS
    utils.cpp
    filechooser.cpp
    desktopportal.cpp
    main.cpp
)

add_executable(xdg-desktop-portal-lxqt ${SRCS})

set_property(TARGET xdg-desktop-portal-lxqt PROPERTY CXX_STANDARD 14)
set_property(TARGET xdg-desktop-portal-lxqt PROPERTY CXX_STANDARD_REQUIRED on)

target_link_libraries(xdg-desktop-portal-lxqt
    Qt::Core
    Qt::DBus
    Qt::Widgets
    KF5::WindowSystem
)

install(TARGETS xdg-desktop-portal-lxqt DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR})
