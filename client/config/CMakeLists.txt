set (kwin_deKorator_config_SOURCES deKoratorconfig.cc deKoratorthemes.cpp themes.cpp kthemeselector.cpp)
kde4_add_ui_files (kwin_deKorator_config_SOURCES configdialog.ui kthemeselectorwidgets.ui)

kde4_add_plugin (kwin_deKorator_config ${kwin_deKorator_config_SOURCES})
target_link_libraries (kwin_deKorator_config ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KNEWSTUFF3_LIBS})

install (TARGETS kwin_deKorator_config DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES deKoratorthemes.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
