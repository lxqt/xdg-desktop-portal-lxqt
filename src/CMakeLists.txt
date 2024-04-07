set(SRCS
    utils.cpp
    filedialoghelper.cpp
    filechooser.cpp
    desktopportal.cpp
    main.cpp
)

include_directories(
    ${Qt6Gui_PRIVATE_INCLUDE_DIRS}
)

add_executable(xdg-desktop-portal-lxqt ${SRCS})

set_property(TARGET xdg-desktop-portal-lxqt PROPERTY CXX_STANDARD 14)
set_property(TARGET xdg-desktop-portal-lxqt PROPERTY CXX_STANDARD_REQUIRED on)

target_link_libraries(xdg-desktop-portal-lxqt
    Qt6::Core
    Qt6::DBus
    Qt6::Widgets
    fm-qt6
    KF6::WindowSystem
)

install(TARGETS xdg-desktop-portal-lxqt DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR})
