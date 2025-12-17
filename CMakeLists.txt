cmake_minimum_required(VERSION 3.16)
project(ModuleSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
set(CMAKE_PREFIX_PATH "/opt/homebrew" ${CMAKE_PREFIX_PATH})
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

if(Qt6_FOUND)
    message(STATUS "Qt6 found: ${Qt6_VERSION}")
else()
    message(FATAL_ERROR "Qt6 not found!")
endif()

# Enable Qt6 automoc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/modules/ModuleBase.cpp
    src/modules/ModuleManager.cpp
    src/modules/ExampleModule.cpp
    src/modules/CustomModuleTemplate.cpp
    src/modules/CalculatorModule.cpp
)

# Header files
set(HEADERS
    include/MainWindow.h
    include/modules/ModuleBase.h
    include/modules/ModuleManager.h
    include/modules/ExampleModule.h
    include/modules/CustomModuleTemplate.h
    include/modules/CalculatorModule.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Create test executable
add_executable(test_modules src/test_modules.cpp
    src/modules/ModuleBase.cpp
    src/modules/ModuleManager.cpp
    src/modules/ExampleModule.cpp
    src/modules/CustomModuleTemplate.cpp
    src/modules/CalculatorModule.cpp
    include/modules/ModuleBase.h
    include/modules/ModuleManager.h
    include/modules/ExampleModule.h
    include/modules/CustomModuleTemplate.h
    include/modules/CalculatorModule.h
)

target_link_libraries(test_modules Qt6::Core Qt6::Widgets)
set_target_properties(test_modules PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)