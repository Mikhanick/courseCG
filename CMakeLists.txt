# CMakeLists.txt для совместимости с CMake Tools в VS Code

cmake_minimum_required(VERSION 3.16)

# Определяем проект
project(RendererGUI)

# Устанавливаем стандарт C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

# Включаем автоматическую обработку MOC, UI и RCC файлов
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Определяем исходные файлы
set(SOURCES
    main.cpp
    MainWindow.cpp
    renderer/GraphicObject.cpp
    renderer/DirectionalLight.cpp
    renderer/TriangleRasterizer.cpp
    renderer/Renderer.cpp
)

set(HEADERS
    MainWindow.h
    renderer/Face.h
    renderer/GraphicObject.h
    renderer/Light.h
    renderer/DirectionalLight.h
    renderer/Camera.h
    renderer/ProjectionStrategy.h
    renderer/ScreenBuffer.h
    renderer/ZBuffer.h
    renderer/ColorBuffer.h
    renderer/ShadeBuffer.h
    renderer/TriangleRasterizer.h
    renderer/Scene.h
    renderer/Renderer.h
    renderer/rastrizercommand.h
)

# Добавляем исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Подключаем необходимые модули Qt
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Gui)

# Подключаем OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
endif()

# Указываем пути к заголовкам
target_include_directories(${PROJECT_NAME} PRIVATE renderer)

# Устанавливаем флаги компилятора
target_compile_options(${PROJECT_NAME} PRIVATE -fopenmp)
target_link_options(${PROJECT_NAME} PRIVATE -fopenmp)