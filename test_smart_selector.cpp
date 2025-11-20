#include "city/strategies/SmartBuildingSelector.h"
#include <QRectF>
#include <QDebug>

int main() {
    // Initialize the application to make Qt work properly
    QCoreApplication app(0, nullptr);
    
    // Create a SmartBuildingSelector with the models directory
    City::SmartBuildingSelector selector("/drive_d/Documents/CG_curs/program/buildings");
    
    // Test the select method with a specific area
    QRectF testArea(0, 0, 30.0f, 25.0f); // 30x25 area
    qDebug() << "Selecting building for area:" << testArea;
    
    auto result = selector.select(testArea);
    qDebug() << "Building selection completed successfully";
    
    return 0;
}