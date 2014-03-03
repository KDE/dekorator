set (kwin_deKorator_config_SOURCES deKoratorconfig.cc deKoratorthemes.cpp themes.cpp kthemeselector.cpp)
qt5_wrap_ui (kwin_deKorator_config_SOURCES configdialog.ui kthemeselectorwidgets.ui)

add_library (kwin_deKorator_config MODULE ${kwin_deKorator_config_SOURCES})
target_link_libraries (kwin_deKorator_config
    Qt5::Widgets
    KF5::Archive
    KF5::I18n
    KF5::KDE4Support
    KF5::NewStuff
    KF5::Service
)

install (TARGETS kwin_deKorator_config DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES deKoratorthemes.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
