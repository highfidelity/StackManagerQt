cmake_minimum_required(VERSION 2.8.11)

set(TARGET_NAME "Stack Manager")
project(${TARGET_NAME})

set(CMAKE_AUTOMOC ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Svg)
find_package(Qt5Core)
find_package(Qt5Network)
find_package(QuaZip)
find_package(ZLIB)

include_directories(${Qt5Widgets_INCLUDE_DIRS}
		    ${Qt5Gui_INCLUDE_DIRS}
		    ${Qt5Svg_INCLUDE_DIRS}
		    ${Qt5Core_INCLUDE_DIRS}
		    ${Qt5Network_INCLUDE_DIRS}
		    ${QUAZIP_INCLUDE_DIRS}
		    ${ZLIB_INCLUDE_DIRS}
		    ${CMAKE_CURRENT_BINARY_DIR}
		    src
		    src/ui)

file(GLOB SRCS "src/*.cpp" "src/ui/*.cpp")
file(GLOB HEADERS "src/*.h" "src/ui/*.h")
file(GLOB QT_RES_FILES "src/*.qrc")
qt5_add_resources(QT_RES "${QT_RES_FILES}")
set(SM_SRCS ${QT_RES} ${SRCS} ${HEADERS})

if (APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "Stack Manager")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER io.highfidelity.StackManager)
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(SM_SRCS ${SM_SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.icns")
    add_executable(${TARGET_NAME} MACOSX_BUNDLE ${SM_SRCS})
else ()
    add_executable(${TARGET_NAME} ${SM_SRCS})
endif()

target_link_libraries(${TARGET_NAME} Qt5::Core Qt5::Gui Qt5::Svg Qt5::Network Qt5::Widgets ${QUAZIP_LIBRARIES} ${ZLIB_LIBRARIES})
