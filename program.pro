QT += core gui widgets
CONFIG += c++20
TARGET = RendererGUI
TEMPLATE = app

QMAKE_CXXFLAGS += -fopenmp -std=c++20
QMAKE_LFLAGS += -fopenmp

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    renderer/GraphicObject.cpp \
    renderer/DirectionalLight.cpp \
    renderer/TriangleRasterizer.cpp \
    renderer/Renderer.cpp

HEADERS += \
    MainWindow.h \
    renderer/Face.h \
    renderer/GraphicObject.h \
    renderer/Light.h \
    renderer/DirectionalLight.h \
    renderer/Camera.h \
    renderer/ProjectionStrategy.h \
    renderer/ScreenBuffer.h \
    renderer/ZBuffer.h \
    renderer/ColorBuffer.h \
    renderer/ShadeBuffer.h \
    renderer/TriangleRasterizer.h \
    renderer/Scene.h \
    renderer/Renderer.h \
    renderer/rastrizercommand.h

# Указываем путь к заголовкам
INCLUDEPATH += renderer
