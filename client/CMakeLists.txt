project (kwin-deKorator)

set (kwin_deKorator_SOURCES deKoratorclient.cc shadow.cpp)

kde4_add_plugin (kwin3_deKorator ${kwin_deKorator_SOURCES})
target_link_libraries (kwin3_deKorator ${KDE4_KDEUI_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${QIMAGEBLITZ_LIBRARIES} kdecorations)
install (TARGETS kwin3_deKorator DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES deKorator.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin)

add_subdirectory (config)

