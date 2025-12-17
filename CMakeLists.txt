cmake_minimum_required(VERSION 3.16)
project(ModuleSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 跨平台配置
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS deployment version")
    set(CMAKE_PREFIX_PATH "/opt/homebrew" ${CMAKE_PREFIX_PATH})
    message(STATUS "Building for macOS")
elseif(WIN32)
    message(STATUS "Building for Windows")
    # Windows Qt路径可以通过CMAKE_PREFIX_PATH设置
endif()

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

if(Qt6_FOUND)
    message(STATUS "Qt6 found: ${Qt6_VERSION}")
    message(STATUS "Qt6 location: ${Qt6_DIR}")
else()
    message(FATAL_ERROR "Qt6 not found! Please set CMAKE_PREFIX_PATH to Qt installation directory")
endif()

# Enable Qt6 automoc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/PerformanceMonitor.cpp
    src/modules/ModuleBase.cpp
    src/modules/ModuleManager.cpp
    src/modules/ExampleModule.cpp
    src/modules/CustomModuleTemplate.cpp
)

# Header files
set(HEADERS
    include/MainWindow.h
    include/PerformanceMonitor.h
    include/modules/ModuleBase.h
    include/modules/ModuleManager.h
    include/modules/ExampleModule.h
    include/modules/CustomModuleTemplate.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.modulesystem.app"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

# macOS特定设置
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    )
endif()

# Windows特定设置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Create test executable
add_executable(test_modules src/test_modules.cpp
    src/modules/ModuleBase.cpp
    src/modules/ModuleManager.cpp
    src/modules/ExampleModule.cpp
    src/modules/CustomModuleTemplate.cpp
    include/modules/ModuleBase.h
    include/modules/ModuleManager.h
    include/modules/ExampleModule.h
    include/modules/CustomModuleTemplate.h
)

target_link_libraries(test_modules Qt6::Core Qt6::Widgets)
set_target_properties(test_modules PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)