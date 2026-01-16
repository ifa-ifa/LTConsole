cmake_minimum_required(VERSION 3.21)

set(CMAKE_AUTOMOC ON)

project("LTConsole")

add_link_options("/PDBALTPATH:%_PDB%")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


set(LTCONSOLE_SOURCES
	"src/LTConsole.cpp"
	"src/ui/MainWindow.cpp"
 "src/ui/Terminal.cpp" "src/ui/Terminal.h"  "src/Bindings.cpp" "src/LuaConsoleManager.h" "src/LuaConsoleManager.cpp"   "src/ui/Atlas.cpp" "src/ui/MapView.h" "src/ui/MapView.cpp" "src/ui/Atlas.h" "src/util/AtlasImporter.h" "src/util/AtlasImporter.cpp" "src/GameData.h" "src/GameData.cpp" "src/Callbacks.h" "src/Callbacks.cpp"  "src/ui/Toolbox.h" "src/ui/Toolbox.cpp" "src/Patch.cpp" "src/Patch.h" "src/ui/EntityViewer.h" "src/ui/EntityViewer.cpp" "src/ui/InfoWidget.h" "src/ui/InfoWidget.cpp" "src/ui/Inspector.h" "src/ui/Inspector.cpp" "src/common/GameStrings.cpp")


add_library(LTConsole SHARED ${LTCONSOLE_SOURCES})
add_library(LTConsoleLoadStub SHARED "src/LoaderStub.cpp")

add_executable(LTConsole_TEST ${LTCONSOLE_SOURCES})


target_link_libraries(LTConsole PRIVATE Qt6::Widgets Qt6::Gui libreplicant)
target_link_libraries(LTConsole_TEST PRIVATE Qt6::Widgets Qt6::Gui libreplicant)

target_link_directories(LTConsole PRIVATE lib/)
target_link_directories(LTConsole_TEST PRIVATE lib/)

target_include_directories(LTConsole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(LTConsole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(LTConsoleLoadStub PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(LTConsoleLoadStub PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(LTConsole_TEST PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(LTConsole_TEST PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)