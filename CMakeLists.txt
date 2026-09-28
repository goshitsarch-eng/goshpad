cmake_minimum_required(VERSION 3.16)

project(goshpad VERSION 4.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs6)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
remove_definitions(-DQT_NO_KEYWORDS)
include(ECMFindQmlModule)
include(FeatureSummary)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 QuickDialogs2 Test)
find_package(KF6 6.0 REQUIRED COMPONENTS Kirigami I18n CoreAddons DBusAddons Config)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

add_executable(goshpad
    src/main.cpp
    src/textops.cpp
    src/document.cpp
    src/settings.cpp
    src/controller.cpp
)

qt_add_qml_module(goshpad
    URI com.goshapps.goshpad
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
        src/qml/SaveChangesDialog.qml
        src/qml/GoToDialog.qml
        src/qml/FontDialog.qml
        src/qml/ErrorDialog.qml
        src/qml/AboutDialog.qml
)

target_compile_definitions(goshpad PRIVATE
    GOSHPAD_VERSION="${PROJECT_VERSION}"
    GOSHPAD_ICON_PATH="${CMAKE_SOURCE_DIR}/data/icons/hicolor/scalable/apps/com.goshapps.GoshPad.svg"
)

target_link_libraries(goshpad PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickDialogs2
    KF6::Kirigami
    KF6::I18n
    KF6::I18nQml
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::ConfigCore
)

install(TARGETS goshpad DESTINATION ${KDE_INSTALL_BINDIR})
install(FILES data/com.goshapps.GoshPad.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/com.goshapps.GoshPad.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES data/icons/hicolor/scalable/apps/com.goshapps.GoshPad.svg
    DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
install(FILES LICENSE COPYRIGHT
    DESTINATION ${KDE_INSTALL_DATADIR}/licenses/com.goshapps.GoshPad)

enable_testing()

add_executable(tst_textops
    tests/tst_textops.cpp
    src/textops.cpp
)
target_include_directories(tst_textops PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tst_textops PRIVATE Qt6::Test)
add_test(NAME textops COMMAND tst_textops)

add_executable(tst_document
    tests/tst_document.cpp
    src/document.cpp
    src/textops.cpp
)
target_include_directories(tst_document PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tst_document PRIVATE Qt6::Test KF6::I18n)
add_test(NAME document COMMAND tst_document)

add_executable(tst_controller
    tests/tst_controller.cpp
    src/controller.cpp
    src/document.cpp
    src/settings.cpp
    src/textops.cpp
)
target_include_directories(tst_controller PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(tst_controller PRIVATE GOSHPAD_VERSION="${PROJECT_VERSION}")
target_link_libraries(tst_controller PRIVATE Qt6::Test Qt6::Gui KF6::I18n KF6::ConfigCore)
add_test(NAME controller COMMAND tst_controller)
set_tests_properties(controller PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
