project (dekorator)
cmake_minimum_required (VERSION 2.4)

find_package (KDE4 REQUIRED)
find_package (QImageBlitz REQUIRED)

#add_definitions (-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)
add_definitions (-DQT3_SUPPORT)
add_definitions (-DQT3_SUPPORT_WARNINGS)

include (KDE4Defaults)
include (MacroLibrary)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )
include_directories (${KDE4_INCLUDES} ${QIMAGEBLITZ_INCLUDES})

add_subdirectory (client)

