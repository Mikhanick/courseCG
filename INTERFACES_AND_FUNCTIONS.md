# Интерфейсы и функции для создания кварталов

## Основные интерфейсы

### AbstractRoadGenerationStrategy (city/strategies/AbstractRoadGenerationStrategy.h)
Абстрактный класс для стратегии генерации дорог

```cpp
class AbstractRoadGenerationStrategy {
public:
    virtual ~AbstractRoadGenerationStrategy() = default;
    
    /**
     * @param cityArea Общая площадь города (м²)
     * @return Список уникальных указателей на дороги
     */
    virtual std::vector<std::unique_ptr<AbstractRoad>> generate(
        float cityArea) = 0;
};
```

### AbstractRoad (city/core/AbstractRoad.h)
Абстрактный класс для дороги

```cpp
class AbstractRoad {
public:
    // Геометрия
    virtual QVector3D getStart() const = 0;
    virtual QVector3D getEnd() const = 0;
    virtual float getWidth() const = 0;
    virtual float getLength() const = 0;

    virtual float getTypeWeight() const = 0;

    // Размещение зданий
    virtual void divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const = 0;
    virtual QVector3D calculateGlobalPosition(const QRectF& plot, const QVector3D& buildingSize) const = 0;
    virtual QVector3D calculateNormal() const = 0;

    // Экспорт
    virtual GraphicObject getRoadMesh() const = 0;
    virtual std::vector<GraphicObject> getBuildingMeshes() const = 0;
    
    // Добавление зданий
    virtual void addBuildingMesh(GraphicObject&& building) = 0;
    
    // Направление размещения зданий
    virtual void setBuildingSide(int side) = 0;
    virtual int getBuildingSide() const = 0;
    
    // Prototype pattern
    virtual std::unique_ptr<AbstractRoad> clone() const = 0;
};
```

## Классы реализации

### SubdivisionRoadGenerationStrategy (city/strategies/SubdivisionRoadGenerationStrategy.h)

#### Внутренний класс Block
```cpp
class Block {
public:
    QRectF rect;  // Границы квартала
    int depth;    // Глубина рекурсии
    std::vector<std::unique_ptr<AbstractRoad>> roads;  // Дороги внутри квартала
    
    // Конструктор
    Block(const QRectF& r, int d, std::vector<std::unique_ptr<AbstractRoad>> rds = {});
    
    // Метод для генерации внутренних дорог в квартале
    void generateInternalRoads(const std::vector<std::unique_ptr<AbstractRoad>>& externalRoads);
};
```

#### Основные методы
```cpp
// Конструктор
SubdivisionRoadGenerationStrategy(
    float minBlockSize = 100.0f,  // минимальный размер квартала (м)
    int maxDepth = 10              // максимальная глубина рекурсии
);

// Основной метод генерации дорог
std::vector<std::unique_ptr<AbstractRoad>> generate(
    float cityArea) override;

// Метод для деления квартала
void subdivide(std::vector<Block>& blocks, const Block& block);

// Преобразование кварталов в дороги
std::vector<std::unique_ptr<AbstractRoad>> blocksToRoads(const std::vector<Block>& blocks) const;

// Вспомогательные функции для обнаружения пересечений
static bool doLinesIntersect(const QVector3D& line1Start, const QVector3D& line1End,
                             const QVector3D& line2Start, const QVector3D& line2End);
static QVector3D findLineIntersection(const QVector3D& line1Start, const QVector3D& line1End,
                                      const QVector3D& line2Start, const QVector3D& line2End);
```

#### Метод generateInternalRoads
- Собирает точки присоединения из внешних дорог
- Генерирует случайные точки на границе квартала
- Создает сквозные (розовые) дороги с вероятностью 30%
- Добавляет разрывы в сквозные дороги
- Генерирует зеленые дороги (параллельные границе)
- Генерирует параллельные (синие) дороги
- Генерирует локальные (черные) дороги
- Удаляет изолированные дороги (менее 20 м)

### BlockSeparatorRoad (city/objects/BlockSeparatorRoad.h)
Специальный тип дороги для разделения блоков/кварталов

### ResidentialRoad (city/objects/ResidentialRoad.h)
Жилая дорога внутри кварталов

## Основной алгоритм генерации кварталов

1. **Начальный квартал**: Город начинается как один большой квартал (Block)
2. **Рекурсивное деление**: Кварталы делятся до достижения максимальной глубины или минимального размера
3. **Типы деления**:
   - Для больших блоков (> 500) - только прямоугольные подразделения
   - Для средних блоков (400-700) - альтернативные подразделения с внутренними дорогами
4. **Генерация внутренней структуры**: В каждом квартале генерируются внутренние дороги
5. **Типы внутренних дорог**:
   - Сквозные (розовые): основные артерии
   - Зеленые: параллельные границе дороги
   - Синие: параллельные сквозным дорогам
   - Черные: локальные ответвления
6. **Создание внешних дорог**: Границы кварталов становятся разделительными дорогами

## Используемые типы данных

- `QRectF`: прямоугольная область квартала
- `QVector3D`: 3D точки и вектора для позиционирования
- `std::vector<std::unique_ptr<AbstractRoad>>`: коллекция дорог
- `std::pair<QRectF, int>`: участки для строительства зданий

## Ключевые параметры

- `minBlockSize`: минимальный размер квартала
- `maxDepth`: максимальная глубина рекурсивного деления
- `minSide`: минимальная сторона для определения стратегии деления