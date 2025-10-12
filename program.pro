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
    renderer/Renderer.cpp \
    city/core/CityMap.cpp \
    city/strategies/SubdivisionRoadGenerationStrategy.cpp \
    city/strategies/SimpleRoadGenerationStrategy.cpp \
    city/objects/ResidentialRoad.cpp \
    city/strategies/DistanceFromCenterCostStrategy.cpp \
    city/strategies/SimpleBuildingSelector.cpp

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
    renderer/rastrizercommand.h \
    city/core/AbstractRoad.h \
    city/strategies/AbstractBuildingSelector.h \
    city/strategies/AbstractCostStrategy.h \
    city/strategies/AbstractRoadGenerationStrategy.h \
    city/core/CityMap.h \
    city/strategies/SubdivisionRoadGenerationStrategy.h \
    city/strategies/SimpleRoadGenerationStrategy.h \
    city/objects/ResidentialRoad.h \
    city/strategies/DistanceFromCenterCostStrategy.h \
    city/strategies/SimpleBuildingSelector.h

# Указываем путь к заголовкам
INCLUDEPATH += renderer
