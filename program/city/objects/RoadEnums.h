#ifndef CITYENUMS_H
#define CITYENUMS_H

namespace City {

/**
 * @brief Стороны размещения зданий относительно дороги
 * 
 * LEFT (-1): Здания только слева от направления дороги (от start к end)
 * BOTH (0): Здания с обеих сторон
 * RIGHT (1): Здания только справа от направления дороги
 * NONE (2): Нет зданий с обеих сторон
 */
enum class BuildingSide {
    LEFT = -1,        // Only left side
    BOTH = 0,         // Both sides
    RIGHT = 1,        // Only right side
    NONE = 2          // No buildings
};

/**
 * @brief Преобразование enum в человекочитаемую строку
 */
inline QString buildingSideToString(BuildingSide side) {
    switch(side) {
        case BuildingSide::LEFT:  return "LEFT (-1)";
        case BuildingSide::BOTH:  return "BOTH (0)";
        case BuildingSide::RIGHT: return "RIGHT (1)";
        case BuildingSide::NONE:  return "NONE (2)";
        default: return "UNKNOWN";
    }
}

} // namespace City

#endif // CITYENUMS_H