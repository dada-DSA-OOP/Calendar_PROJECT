cmake_minimum_required(VERSION 3.16)

project(CalendarApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport)

set(PROJECT_SOURCES
        resources.qrc

        # Nguồn chính (trong 'src/')
        src/main.cpp
        src/mainwindow.cpp src/mainwindow.h
        src/calendarview.cpp src/calendarview.h
        src/eventitem.cpp src/eventitem.h
        src/dayheader.h src/dayheader.cpp
        src/timeruler.h src/timeruler.cpp
        src/eventdialog.h src/eventdialog.cpp
        src/sidepanel.h src/sidepanel.cpp
        src/funnytipwidget.h src/funnytipwidget.cpp
        src/settingsdialog.h src/settingsdialog.cpp
        src/daydetaildialog.h src/daydetaildialog.cpp
        src/daycellwidget.h src/daycellwidget.cpp
        src/monthviewwidget.h src/monthviewwidget.cpp
        src/timetableslotwidget.h src/timetableslotwidget.cpp
        src/timetableviewwidget.h src/timetableviewwidget.cpp
        src/sessionviewwidget.h src/sessionviewwidget.cpp

        # File UI (trong 'ui/')
        # CMAKE_AUTOUIC sẽ tự động xử lý các file .ui này
        ui/mainwindow.ui

        # icon
        appicon.rc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CalendarApp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}



    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET CalendarApp APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(CalendarApp SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(CalendarApp
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(CalendarApp PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::PrintSupport)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CalendarApp)
endif()
set_target_properties(CalendarApp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS CalendarApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CalendarApp)
endif()

