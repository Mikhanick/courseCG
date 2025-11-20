cmake_minimum_required(VERSION 3.16)
project(TestSmartSelector)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/city
    ${CMAKE_CURRENT_SOURCE_DIR}/city/objects
    ${CMAKE_CURRENT_SOURCE_DIR}/city/loaders
    ${CMAKE_CURRENT_SOURCE_DIR}/city/strategies
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
)

# Define source files
set(SOURCES
    test_smart_selector.cpp
    city/loaders/ModelLoader.cpp
    city/strategies/SmartBuildingSelector.cpp
    renderer/GraphicObject.cpp
)

# Create executable
add_executable(test_smart_selector ${SOURCES})

# Link Qt libraries
target_link_libraries(test_smart_selector Qt5::Core Qt5::Widgets)