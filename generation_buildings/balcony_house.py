from createBuilding import (
    Building,
    GeometrySegment,
    RectangleWithCutout,
    SimplePolygon,
    RoofGeometry,
)


def create_balcony_panel(panel_width, panel_height, color_theme, window_scale=1.0):
    """Создает панель с полноформатным балконом на всю ширину"""
    balcony_panel = GeometrySegment(invert=True)

    # 🏢 Основная стена здания
    outer_frame = [
        (0, 0, 0),
        (panel_width, 0, 0),
        (panel_width, panel_height, 0),
        (0, panel_height, 0),
    ]

    # 🪟 Окно в верхней части (уменьшенное на 15% при scale=0.85)
    window_offset = 0.6 * window_scale
    window_width = (panel_width - 2 * window_offset) * window_scale
    window_height = (panel_height - 1.6) * window_scale
    window_bottom = 1.4

    # Корректируем позицию окна по центру при изменении размера
    window_left = (panel_width - window_width) / 2

    window_frame = [
        (window_left, window_bottom, 0.1),
        (window_left + window_width, window_bottom, 0.1),
        (window_left + window_width, window_bottom + window_height, 0.1),
        (window_left, window_bottom + window_height, 0.1),
    ]

    # Добавляем основную стену с окном
    balcony_panel.add_child(
        RectangleWithCutout(
            outer_points=outer_frame,
            cutout_points=window_frame,
            color=color_theme["wall"],
        )
    )
    balcony_panel.add_child(
        SimplePolygon(
            points=window_frame,
            color=color_theme["window"],
        )
    )

    # 🌬️ Параметры балкона
    balcony_depth = 1.2  # Глубина балкона
    balcony_bottom = 0.3  # Высота от пола до пола балкона
    railing_height = 1.0  # Высота ограждения

    # 1️⃣ Нижняя часть (пол балкона)
    balcony_floor = [
        (0, balcony_bottom, 0),  # Задний левый нижний
        (panel_width, balcony_bottom, 0),  # Задний правый нижний
        (panel_width, balcony_bottom, balcony_depth),  # Передний правый нижний
        (0, balcony_bottom, balcony_depth),  # Передний левый нижний
    ]

    # 2️⃣ Передняя вертикальная стена (ограждение)
    front_railing = [
        (0, balcony_bottom, balcony_depth),  # Нижний левый
        (panel_width, balcony_bottom, balcony_depth),  # Нижний правый
        (panel_width, balcony_bottom + railing_height, balcony_depth),  # Верхний правый
        (0, balcony_bottom + railing_height, balcony_depth),  # Верхний левый
    ]

    # 3️⃣ Левая боковая стена
    left_railing = [
        (0, balcony_bottom, 0),  # Задний нижний
        (0, balcony_bottom, balcony_depth),  # Передний нижний
        (0, balcony_bottom + railing_height, balcony_depth),  # Передний верхний
        (0, balcony_bottom + railing_height, 0),  # Задний верхний
    ]

    # 4️⃣ Правая боковая стена
    right_railing = [
        (panel_width, balcony_bottom, 0),  # Задний нижний
        (panel_width, balcony_bottom, balcony_depth),  # Передний нижний
        (
            panel_width,
            balcony_bottom + railing_height,
            balcony_depth,
        ),  # Передний верхний
        (panel_width, balcony_bottom + railing_height, 0),  # Задний верхний
    ]

    # 5️⃣ Верхняя часть ограждения
    top_railing = [
        (0, balcony_bottom + railing_height, 0),
        (panel_width, balcony_bottom + railing_height, 0),
        (panel_width, balcony_bottom + railing_height, balcony_depth),
        (0, balcony_bottom + railing_height, balcony_depth),
    ]

    # Добавляем все элементы балкона
    balcony_panel.add_child(
        SimplePolygon(points=balcony_floor, color="#8B7D6B")  # Темно-бежевый для пола
    )    
    balcony_panel.add_child(
        SimplePolygon(points=balcony_floor, color="#8B7D6B", invert=True)  # Темно-бежевый для пола
    )
    balcony_panel.add_child(
        SimplePolygon(points=front_railing, color="#696969")  # Серый для ограждений
    )
    balcony_panel.add_child(SimplePolygon(points=left_railing, color="#696969", invert=True))
    balcony_panel.add_child(SimplePolygon(points=right_railing, color="#696969"))
    balcony_panel.add_child(SimplePolygon(points=left_railing, color="#696969", invert=True))
    balcony_panel.add_child(SimplePolygon(points=right_railing, color="#696969"))
    # balcony_panel.add_child(
    #     SimplePolygon(points=top_railing, color="#555555")  # Темнее для верха
    # )

    return balcony_panel


def create_standard_window_panel(
    panel_width, panel_height, color_theme, window_scale=1.0
):
    """Создает стандартную панель с окном (уменьшенным на 15%)"""
    window_panel = GeometrySegment()

    # 🏢 Основная стена
    outer_frame = [
        (0, 0, 0),
        (panel_width, 0, 0),
        (panel_width, panel_height, 0),
        (0, panel_height, 0),
    ]

    # 🪟 Окно (уменьшенное на 15%)
    window_offset = 0.7 * window_scale
    window_width = (panel_width - 2 * window_offset) * window_scale * 0.8
    window_height = (panel_height - 0.8) * window_scale
    window_bottom = 0.4

    # Центрируем окно
    window_left = (panel_width - window_width) / 2

    window_frame = [
        (window_left, window_bottom, 0.1),
        (window_left + window_width, window_bottom, 0.1),
        (window_left + window_width, window_bottom + window_height, 0.1),
        (window_left, window_bottom + window_height, 0.1),
    ]

    # Добавляем основную стену с окном
    window_panel.add_child(
        RectangleWithCutout(
            outer_points=outer_frame,
            cutout_points=window_frame,
            color=color_theme["wall"],
        )
    )
    window_panel.add_child(
        SimplePolygon(
            points=window_frame,
            color=color_theme["window"],
        )
    )

    return window_panel


def create_entry_panel(panel_width, panel_height, color_theme, window_scale=1.0):
    """Создает панель с входной группой для первого этажа"""
    entry_panel = GeometrySegment()

    # 🏢 Основная стена
    outer_frame = [
        (0, 0, 0),
        (panel_width, 0, 0),
        (panel_width, panel_height, 0),
        (0, panel_height, 0),
    ]

    # 🚪 Дверной проем
    door_width = 1.8
    door_height = 2.2
    door_bottom = 0.2
    door_left = (panel_width - door_width) / 2

    door_frame = [
        (door_left, door_bottom, 0.1),
        (door_left + door_width, door_bottom, 0.1),
        (door_left + door_width, door_bottom + door_height, 0.1),
        (door_left, door_bottom + door_height, 0.1),
    ]

    # Добавляем основную стену с вырезом под дверь
    entry_panel.add_child(
        RectangleWithCutout(
            outer_points=outer_frame,
            cutout_points=door_frame,
            color=color_theme["wall"],
        )
    )

    # 🚪 Сама дверь
    entry_panel.add_child(
        SimplePolygon(
            points=door_frame,
            color="#8B4513",  # Коричневый для дерева
        )
    )

    # 🌂 Навес над входом
    canopy_width = panel_width * 0.8  # 80% ширины панели
    canopy_depth = 1.5  # Глубина навеса
    canopy_height = 2.6  # Высота крепления навеса
    canopy_thickness = 0.15  # Толщина навеса
    canopy_left = (panel_width - canopy_width) / 2

    # Нижняя поверхность навеса
    canopy_bottom = [
        (canopy_left, canopy_height, 0),
        (canopy_left + canopy_width, canopy_height, 0),
        (canopy_left + canopy_width, canopy_height, canopy_depth),
        (canopy_left, canopy_height, canopy_depth),
    ]

    # Верхняя поверхность навеса
    canopy_top = [
        (canopy_left, canopy_height + canopy_thickness, 0),
        (canopy_left + canopy_width, canopy_height + canopy_thickness, 0),
        (canopy_left + canopy_width, canopy_height + canopy_thickness, canopy_depth),
        (canopy_left, canopy_height + canopy_thickness, canopy_depth),
    ]

    # Передняя кромка навеса
    canopy_front = [
        (canopy_left, canopy_height, canopy_depth),
        (canopy_left + canopy_width, canopy_height, canopy_depth),
        (canopy_left + canopy_width, canopy_height + canopy_thickness, canopy_depth),
        (canopy_left, canopy_height + canopy_thickness, canopy_depth),
    ]

    # Боковые кромки навеса
    canopy_left_side = [
        (canopy_left, canopy_height, 0),
        (canopy_left, canopy_height, canopy_depth),
        (canopy_left, canopy_height + canopy_thickness, canopy_depth),
        (canopy_left, canopy_height + canopy_thickness, 0),
    ]

    canopy_right_side = [
        (canopy_left + canopy_width, canopy_height, 0),
        (canopy_left + canopy_width, canopy_height, canopy_depth),
        (canopy_left + canopy_width, canopy_height + canopy_thickness, canopy_depth),
        (canopy_left + canopy_width, canopy_height + canopy_thickness, 0),
    ]

    # Опорные столбы навеса
    pillar_width = 0.25
    pillar_height = canopy_height - 0.3  # Отступ от земли

    left_pillar_x = canopy_left + 0.2
    right_pillar_x = canopy_left + canopy_width - pillar_width - 0.2

    left_pillar = [
        (left_pillar_x, 0.3, 0),
        (left_pillar_x + pillar_width, 0.3, 0),
        (left_pillar_x + pillar_width, 0.3 + pillar_height, 0),
        (left_pillar_x, 0.3 + pillar_height, 0),
    ]

    right_pillar = [
        (right_pillar_x, 0.3, 0),
        (right_pillar_x + pillar_width, 0.3, 0),
        (right_pillar_x + pillar_width, 0.3 + pillar_height, 0),
        (right_pillar_x, 0.3 + pillar_height, 0),
    ]

    # Добавляем элементы навеса
    entry_panel.add_child(
        SimplePolygon(points=canopy_bottom, color="#A9A9A9")  # Серый для навеса
    )
    entry_panel.add_child(
        SimplePolygon(points=canopy_top, color="#808080")  # Темнее для верха
    )
    entry_panel.add_child(SimplePolygon(points=canopy_front, color="#696969"))
    entry_panel.add_child(SimplePolygon(points=canopy_left_side, color="#777777"))
    entry_panel.add_child(SimplePolygon(points=canopy_right_side, color="#777777"))

    # Добавляем опорные столбы
    entry_panel.add_child(
        SimplePolygon(points=left_pillar, color="#5D4037")  # Темно-коричневый
    )
    entry_panel.add_child(SimplePolygon(points=right_pillar, color="#5D4037"))

    return entry_panel


def create_side_panel(panel_width, panel_height, color_theme, window_scale=1.0):
    """Создает панель для боковых стен"""
    side_panel = GeometrySegment()

    # 🏢 Основная стена
    outer_frame = [
        (0, 0, 0),
        (panel_width, 0, 0),
        (panel_width, panel_height, 0),
        (0, panel_height, 0),
    ]

    # 🪟 Окно на боковой стене (уменьшенное на 15%)
    window_offset = 0.6 * window_scale
    window_width = (panel_width - 2 * window_offset) * window_scale
    window_height = (panel_height - 1.0) * window_scale
    window_bottom = 0.6

    # Центрируем окно
    window_left = (panel_width - window_width) / 2

    window_frame = [
        (window_left, window_bottom, 0.1),
        (window_left + window_width, window_bottom, 0.1),
        (window_left + window_width, window_bottom + window_height, 0.1),
        (window_left, window_bottom + window_height, 0.1),
    ]

    # Добавляем основную стену с окном
    side_panel.add_child(
        RectangleWithCutout(
            outer_points=outer_frame,
            cutout_points=window_frame,
            color=color_theme["wall"],
        )
    )
    side_panel.add_child(
        SimplePolygon(
            points=window_frame,
            color=color_theme["window"],
        )
    )

    return side_panel


def create_beige_building(name, color_theme, panel_count=5):
    """Создает здание в заданном цветовом решении с указанной длиной"""
    # Основные параметры здания
    panel_width_front = 6.0  # Ширина панели фасада
    panel_width_side = 4.0  # Ширина боковой панели
    panel_height = 2.8  # Высота каждого этажа
    floor_count = 5  # Количество этажей

    # Коэффициент уменьшения окон на 15%
    window_scale = 0.85

    # Инициализация здания
    building = Building(
        name=name,
        description=f"{floor_count}-этажный жилой дом с {panel_count} панелями и полноформатными балконами ({name})",
        floor_count=floor_count,
        width=panel_width_front * panel_count,  # Ширина зависит от количества панелей
        depth=panel_width_side * 3,  # Глубина остается постоянной (3 боковые панели)
        ground_floor_height=panel_height,
        typical_floor_height=panel_height,
        roof_type="flat",
        roof_height=0.8,  # Небольшой бортик крыши
    )

    # 🎨 Создаем все типы панелей
    # Для фасада и задней стены
    standard_front = create_standard_window_panel(
        panel_width_front, panel_height, color_theme, window_scale
    )
    balcony_front = create_balcony_panel(
        panel_width_front, panel_height, color_theme, window_scale
    )
    entry_front = create_entry_panel(
        panel_width_front, panel_height, color_theme, window_scale
    )

    # Для боковых стен
    side_window = create_side_panel(
        panel_width_side, panel_height, color_theme, window_scale
    )

    # 🧱 Определяем расположение панелей в зависимости от их количества

    # Для 5 панелей: балконы на 2-й и 4-й панелях, вход в центре (3-я панель)
    # Для 7 панелей: балконы на 2-й, 4-й и 6-й панелях, вход в центре (4-я панель)

    if panel_count == 5:
        # Порядок панелей для 5-панельного здания: [окно, балкон, вход/окно, балкон, окно]
        balcony_positions = [1, 3]  # 0-based индексы для балконов (2-я и 4-я панели)
        entrance_position = 2  # 0-based индекс для входа (3-я панель)
    elif panel_count == 7:
        # Порядок панелей для 7-панельного здания: [окно, балкон, окно, вход/окно, окно, балкон, окно]
        balcony_positions = [1, 5]  # 0-based индексы для балконов (2-я и 6-я панели)
        entrance_position = 3  # 0-based индекс для входа (4-я панель)
    else:
        raise ValueError("Поддерживаются только 5 или 7 панелей")

    # 🏗️ Формируем панели для разных этажей

    # Для первого этажа
    front_panels_ground = []
    for i in range(panel_count):
        if i == entrance_position:
            front_panels_ground.append(entry_front)  # Входная группа в центре
        elif i in balcony_positions:
            front_panels_ground.append(balcony_front)  # Балконы на указанных позициях
        else:
            front_panels_ground.append(standard_front)  # Стандартные панели с окнами

    # Для типовых этажей (без входа)
    front_panels_typical = []
    for i in range(panel_count):
        if i in balcony_positions:
            front_panels_typical.append(balcony_front)  # Балконы на указанных позициях
        else:
            front_panels_typical.append(standard_front)  # Стандартные панели с окнами

    # Задние стены теперь такие же как передние
    back_panels_ground = front_panels_ground.copy()
    back_panels_typical = front_panels_typical.copy()

    # 🧱 Добавление стен к зданию

    # Фасадные стены (передняя часть)
    building.add_wall(
        wall_type="front",
        panels=front_panels_ground,
        floor_type="ground",
        invert=False,
    )

    building.add_wall(
        wall_type="front",
        panels=front_panels_typical,
        floor_type="typical",
        invert=False,
    )

    # Задние стены (теперь такие же как передние)
    building.add_wall(
        wall_type="back",
        panels=back_panels_ground,
        floor_type="ground",
        invert=True,
    )

    building.add_wall(
        wall_type="back",
        panels=back_panels_typical,
        floor_type="typical",
        invert=True,
    )

    # Боковые стены
    left_panels_ground = [side_window] * 3
    left_panels_typical = [side_window] * 3

    right_panels_ground = [side_window] * 3
    right_panels_typical = [side_window] * 3

    # Левая стена
    building.add_wall(
        wall_type="left",
        panels=left_panels_ground,
        floor_type="ground",
        invert=False,
    )

    building.add_wall(
        wall_type="left",
        panels=left_panels_typical,
        floor_type="typical",
        invert=False,
    )

    # Правая стена
    building.add_wall(
        wall_type="right",
        panels=right_panels_ground,
        floor_type="ground",
        invert=True,
    )

    building.add_wall(
        wall_type="right",
        panels=right_panels_typical,
        floor_type="typical",
        invert=True,
    )

    # 🏠 Финализация здания
    building.finalize()

    # 💾 Сохранение здания
    building.save_to_file(f"buildings/{name}.json")

    return building


# 🎨 Цветовые схемы для разных вариантов домов

# 🏠 1. Светлый песочный дом
light_sand_theme = {
    "wall": "#E8D8C9",  # Светлый песочный
    "window": Building.get_window_color
}

# 🏠 2. Теплый кремовый дом
warm_cream_theme = {
    "wall": "#D9C8B3",  # Теплый кремовый
    "window": Building.get_window_color
}

# 🏠 3. Бежевый "кофе с молоком"
coffee_beige_theme = {
    "wall": "#C2B29E",  # Бежевый "кофе с молоком"
    "window": Building.get_window_color
}

# 🏗️ Создание зданий с разным количеством панелей

print("🏗️ Начинаем создание зданий с 5 и 7 панелями...")

# 🏠 5-панельные здания
print("\n🏠 Создание 5-панельных зданий:")

light_sand_5p = create_beige_building("light_sand_5p", light_sand_theme, panel_count=5)
print("✅ Светлый песочный дом (5 панелей) успешно создан!")

warm_cream_5p = create_beige_building("warm_cream_5p", warm_cream_theme, panel_count=5)
print("✅ Теплый кремовый дом (5 панелей) успешно создан!")

coffee_beige_5p = create_beige_building(
    "coffee_beige_5p", coffee_beige_theme, panel_count=5
)
print("✅ Бежевый 'кофе с молоком' дом (5 панелей) успешно создан!")

# 🏠 7-панельные здания
print("\n🏠 Создание 7-панельных зданий:")

light_sand_7p = create_beige_building("light_sand_7p", light_sand_theme, panel_count=7)
print("✅ Светлый песочный дом (7 панелей) успешно создан!")

warm_cream_7p = create_beige_building("warm_cream_7p", warm_cream_theme, panel_count=7)
print("✅ Теплый кремовый дом (7 панелей) успешно создан!")

coffee_beige_7p = create_beige_building(
    "coffee_beige_7p", coffee_beige_theme, panel_count=7
)
print("✅ Бежевый 'кофе с молоком' дом (7 панелей) успешно создан!")

print("\n🎉 Все здания успешно созданы и сохранены в папку buildings/")
