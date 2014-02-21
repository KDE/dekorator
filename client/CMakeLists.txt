project (kwin-deKorator)

set (kwin_deKorator_SOURCES deKoratorclient.cc shadow.cpp)

add_library(kwin3_deKorator MODULE ${kwin_deKorator_SOURCES})
target_link_libraries(kwin3_deKorator
    Qt5::Core
    Qt5::Widgets
    Qt5::X11Extras
    KF5::ConfigCore
    KF5::IconThemes
    KF5::I18n
    KF5::WindowSystem
    KF5::KDE4Support
    ${QIMAGEBLITZ_LIBRARIES}
    KDE4Workspace__kdecorations
)
install (TARGETS kwin3_deKorator DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES deKorator.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin)

add_subdirectory (config)

