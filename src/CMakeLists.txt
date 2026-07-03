set(SRCS
    utils.cpp
    access.cpp
    choices.cpp
    filedialoghelper.cpp
    filechooser.cpp
    desktopportal.cpp
    main.cpp
)

add_executable(xdg-desktop-portal-lxqt ${SRCS})

target_link_libraries(xdg-desktop-portal-lxqt
    PUBLIC
        Qt6::Core
        Qt6::DBus
        Qt6::Widgets
        fm-qt6
        KF6::WindowSystem
    PRIVATE
        Qt6::GuiPrivate
)

install(TARGETS xdg-desktop-portal-lxqt DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR})
