project (dekorator)
cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

################# set KDE specific information #################
find_package(ECM 0.0.8 REQUIRED NO_MODULE)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5Core REQUIRED NO_MODULE)
find_package(Qt5DBus REQUIRED NO_MODULE)
find_package(Qt5Widgets REQUIRED NO_MODULE)
find_package(Qt5X11Extras REQUIRED NO_MODULE)

find_package(KF5 NO_MODULE REQUIRED COMPONENTS Archive KIO Config I18n IconThemes NewStuff WindowSystem )
find_package(KDecorations REQUIRED NO_MODULE)
find_package(KF5KDE4Support REQUIRED NO_MODULE)
find_package(QImageBlitz REQUIRED MODULE)

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

include_directories(${QIMAGEBLITZ_INCLUDES})

add_subdirectory (client)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

