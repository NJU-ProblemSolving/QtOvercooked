cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME QtOvercooked)

project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        image.qrc

        config.h
        interfaces.h enums.h
        tile.h tile.cpp
        player.h player.cpp
        recipe.h recipe.cpp
        mixture.h foodcontainer.h
        entitymanager.h entitymanager.cpp
        ordermanager.h gamemanager.h
        guimanager.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

qt_disable_unicode_defines(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

add_subdirectory(box2d)
add_subdirectory(tiny-process-library)

target_link_libraries(${PROJECT_NAME} PUBLIC box2d)
target_link_libraries(${PROJECT_NAME} PUBLIC tiny-process-library)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CAMKE_INSTALL_BINDIR})

add_executable(runner entitymanager.cpp player.cpp recipe.cpp tile.cpp runner.cpp)
target_link_libraries(runner PUBLIC box2d)
target_link_libraries(runner PUBLIC tiny-process-library)

install(TARGETS runner LIBRARY DESTINATION ${CAMKE_INSTALL_BINDIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
