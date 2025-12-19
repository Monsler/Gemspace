cmake_minimum_required(VERSION 3.10)
project(Gemspace)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC
    src/main.cpp
    src/ui/MainWindow.cpp
    include/ui/MainWindow.hpp
    include/browser/Requester.hpp
    src/browser/Requester.cpp
    include/ui/Gembling.hpp
    src/ui/Gembling.cpp
)

find_package(ECM 1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools Multimedia DBus)

configure_file(${CMAKE_SOURCE_DIR}/resources.qrc
               ${CMAKE_BINARY_DIR}/resources.qrc
               COPYONLY)

qt_add_executable(Gemspace
    ${SRC}
    ${CMAKE_BINARY_DIR}/resources.qrc
)

qt_add_translations(Gemspace
    TS_FILES
        languages/gemspace_en.ts
        languages/gemspace_ru.ts
    SOURCES
        ${SRC}
)

target_include_directories(Gemspace PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Gemspace PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia Qt6::DBus)
