cmake_minimum_required(VERSION 2.8.11)

project(StackManagerQt)

set(CMAKE_AUTOMOC ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Core)
find_package(Qt5Network)
find_package(QuaZip)
find_package(ZLIB)

include_directories(${Qt5Widgets_INCLUDE_DIRS}
					${Qt5Gui_INCLUDE_DIRS}
					${Qt5Core_INCLUDE_DIRS}
					${Qt5Network_INCLUDE_DIRS}
					${QUAZIP_INCLUDE_DIRS}
					${ZLIB_INCLUDE_DIRS}
					${CMAKE_CURRENT_BINARY_DIR}
					src)

file(GLOB SRCS "src/*.cpp")
file(GLOB QT_UI_FILES "ui/*.ui")
file(GLOB QT_RES_FILES "src/*.qrc")
qt5_wrap_ui(QT_UI_HEADERS "${QT_UI_FILES}")
qt5_add_resources(QT_RES "${QT_RES_FILES}")
set(SM_SRCS ${QT_RES} ${QT_UI_HEADERS} ${SRCS})

if (APPLE)
	set(MACOSX_BUNDLE_BUNDLE_NAME "Stack Manager")
	add_executable(stackmanager MACOSX_BUNDLE ${SM_SRCS})
else ()
    add_executable(stackmanager ${SM_SRCS})
endif()

target_link_libraries(stackmanager Qt5::Core Qt5::Gui Qt5::Network Qt5::Widgets ${QUAZIP_LIBRARIES} ${ZLIB_LIBRARIES})
