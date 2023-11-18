cmake_minimum_required(VERSION 3.21)

project(unRAWer VERSION 1.0.0 LANGUAGES CXX)

include(GenerateExportHeader)
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# https://doc.qt.io/qt-6/cmake-get-started.html
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)
qt_standard_project_setup()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(Boost REQUIRED COMPONENTS log)

find_package(libraw CONFIG REQUIRED)
find_package(OpenImageIO CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)

qt_add_executable(unrawer-qt MANUAL_FINALIZATION
    include/unrawer/file_processor.hpp
    include/unrawer/imageio.hpp
    include/unrawer/log.hpp
    include/unrawer/process.hpp
    include/unrawer/processors.hpp
    include/unrawer/settings.hpp
    include/unrawer/threadpool.hpp
    include/unrawer/timer.hpp
    include/unrawer/ui.hpp
    include/unrawer/unrawer.hpp

    src/file_processor.cpp
    src/imageio.cpp
    src/log.cpp
    src/main.cpp
    src/process.cpp
    src/processors.cpp
    src/settings.cpp
    src/ui.cpp
    src/unrawer.cpp
)

target_include_directories(unrawer-qt PRIVATE
    include
    ${PROJECT_BINARY_DIR}/include
)

target_link_libraries(unrawer-qt PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Concurrent
    OpenImageIO::OpenImageIO
    OpenImageIO::OpenImageIO_Util
    libraw::raw_r
    Boost::boost
    Boost::log
    toml11::toml11
)

set_target_properties(unrawer-qt PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_resources(unrawer-qt "images"
    PREFIX "/MainWindow"
    BASE "release"
    FILES release/unrw.ico
)

target_compile_definitions(unrawer-qt PRIVATE 
    WIN32_LEAN_AND_MEAN
    BOOST_USE_WINAPI_VERSION=BOOST_WINAPI_VERSION_WIN7
)
install(TARGETS unrawer-qt
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET unrawer-qt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    NO_TRANSLATIONS
    #NO_COMPILER_RUNTIME
)
install(SCRIPT ${deploy_script})

install(FILES
    release/unrw_config.toml

    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_target(unrawer-qt)
