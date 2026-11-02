cmake_minimum_required(VERSION 3.21)
project(TimeTracker VERSION 0.1.0 LANGUAGES CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Sql)

add_library(ttcore STATIC
    src/core/Database.cpp
    src/core/CsvExport.cpp
    src/core/CsvExport.h
    src/core/Database.h
    src/core/FakeClock.cpp
    src/core/FakeClock.h
    src/core/IClock.h
    src/core/Models.h
    src/core/Settings.cpp
    src/core/Settings.h
    src/core/SessionRepository.cpp
    src/core/SessionRepository.h
    src/core/SystemClock.cpp
    src/core/SystemClock.h
    src/core/TaskRepository.cpp
    src/core/TaskRepository.h
    src/core/TimeUtils.cpp
    src/core/TimeUtils.h
    src/core/TrackerService.cpp
    src/core/TrackerService.h
    src/core/Version.cpp
    src/core/Version.h
)
target_include_directories(ttcore PUBLIC src)
target_link_libraries(ttcore PUBLIC Qt6::Core Qt6::Sql)

add_library(ttui STATIC
    src/ui/HistoryPage.cpp
    src/ui/HistoryPage.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/RecoveryDialog.cpp
    src/ui/RecoveryDialog.h
    src/ui/SessionMonitor.cpp
    src/ui/SessionMonitor.h
    src/ui/SettingsDialog.cpp
    src/ui/SettingsDialog.h
    src/ui/SessionEditDialog.cpp
    src/ui/StatusItem.h
    src/ui/SessionEditDialog.h
    src/ui/TaskTableModel.cpp
    src/ui/TaskTableModel.h
    src/ui/TrackerPage.cpp
    src/ui/TrackerPage.h
    src/ui/TrayController.cpp
    src/ui/TrayController.h
)
if(APPLE)
    target_sources(ttui PRIVATE src/ui/StatusItem_mac.mm)
    set_source_files_properties(src/ui/StatusItem_mac.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
    target_link_libraries(ttui PUBLIC "-framework AppKit")
else()
    target_sources(ttui PRIVATE src/ui/StatusItem_stub.cpp)
endif()
target_include_directories(ttui PUBLIC src)
target_link_libraries(ttui PUBLIC ttcore Qt6::Widgets)

add_executable(TimeTracker
    src/app/main.cpp
)
target_link_libraries(TimeTracker PRIVATE ttui)
set_target_properties(TimeTracker PROPERTIES MACOSX_BUNDLE ON)

enable_testing()
add_subdirectory(tests)
