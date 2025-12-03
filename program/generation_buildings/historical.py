from createBuilding import (
    Building,
    GeometrySegment,
    RectangleWithCutout,
    SimplePolygon,
)


def create_historical_building(
    name="historical_mansion",
    description="Историческое здание с входом и прямоугольными карнизами",
    floor_count=3,
    width=20.0,
    depth=15.0,
    ground_floor_height=4.0,
    typical_floor_height=3.2,
    roof_height=3.5,
    wall_color="#C19A6B",  # Единый цвет для всех стен
    window_color="#87CEEB",  # Голубоватое стекло
    roof_color="#8B4513",  # Коричневая черепица
    chimney_color="#A52A2A",  # Цвет дымовой трубы
    door_color="#8B4513",  # Цвет двери
):
    """
    Создаёт историческое здание с:
    - Окнами через систему вложенных вырезов
    - Одинаковой высотой боковых панелей на первом этаже
    - Входом на первом этаже спереди
    - Прямоугольными карнизами
    - Увеличенной дымовой трубой
    """
    building = Building(
        name=name,
        description=description,
        floor_count=floor_count,
        width=width,
        depth=depth,
        ground_floor_height=ground_floor_height,
        typical_floor_height=typical_floor_height,
        roof_type="custom",
        roof_height=roof_height,
    )

    # Рассчитываем ширину панелей
    panel_width = width / 6
    panel_height_ground = ground_floor_height
    panel_height_typical = typical_floor_height

    # =============================
    # СОЗДАНИЕ ПАНЕЛИ С ОКНОМ ЧЕРЕЗ ВЛОЖЕННЫЕ ВЫРЕЗЫ
    # =============================
    def create_window_panel(panel_width, panel_height, is_ground_floor=False):
        """Создаёт панель с окном через строго вложенные вырезы"""
        panel = GeometrySegment()

        # 1. Определяем размеры окна
        if is_ground_floor:
            window_inset = 0.6  # Большой отступ для первого этажа
            frame_thickness = 0.15
            glass_depth = 0.1
        else:
            window_inset = 0.4
            frame_thickness = 0.08
            glass_depth = 0.12

        # 2. Внешний контур стены (уровень 1)
        outer_wall = [
            (0, 0, 0),
            (panel_width, 0, 0),
            (panel_width, panel_height, 0),
            (0, panel_height, 0),
        ]

        # 3. Внешний контур рамки (соединяется со стеной)
        outer_frame = [
            (window_inset, window_inset, 0),
            (panel_width - window_inset, window_inset, 0),
            (panel_width - window_inset, panel_height - window_inset, 0),
            (window_inset, panel_height - window_inset, 0),
        ]

        # 4. Внутренний контур рамки (соединяется со стеклом)
        inner_frame = [
            (
                window_inset + frame_thickness,
                window_inset + frame_thickness,
                glass_depth,
            ),
            (
                panel_width - window_inset - frame_thickness,
                window_inset + frame_thickness,
                glass_depth,
            ),
            (
                panel_width - window_inset - frame_thickness,
                panel_height - window_inset - frame_thickness,
                glass_depth,
            ),
            (
                window_inset + frame_thickness,
                panel_height - window_inset - frame_thickness,
                glass_depth,
            ),
        ]

        # 5. Строим трехуровневую геометрию
        wall = RectangleWithCutout(
            outer_points=outer_wall, cutout_points=outer_frame, color=wall_color
        )
        panel.add_child(wall)

        frame = RectangleWithCutout(
            outer_points=outer_frame,  # ТОЧНО совпадает с вырезом стены!
            cutout_points=inner_frame,  # ТОЧНО совпадает с контуром стекла!
            color="#654321",  # Тёмно-коричневый для исторического стиля
        )
        panel.add_child(frame)

        glass = SimplePolygon(
            points=inner_frame,  # ТОЧНО совпадает с вырезом рамки!
            color=window_color,
        )
        panel.add_child(glass)

        return panel

    # =============================
    # СОЗДАНИЕ ПАНЕЛИ С ДВЕРЬЮ НА ПЕРВОМ ЭТАЖЕ
    # =============================
    def create_door_panel(panel_width, panel_height):
        """Создаёт панель с входной дверью на первом этаже"""
        panel = GeometrySegment()

        # 1. Внешний контур стены
        outer_wall = [
            (0, 0, 0),
            (panel_width, 0, 0),
            (panel_width, panel_height, 0),
            (0, panel_height, 0),
        ]

        # 2. Внешний контур дверной рамы
        door_inset = 0.5
        frame_thickness = 0.12
        door_depth = 0.05

        outer_frame = [
            (door_inset, 0.1, 0),  # Дверь начинается выше пола
            (panel_width - door_inset, 0.1, 0),
            (panel_width - door_inset, panel_height - 0.8, 0),  # Ниже обычного окна
            (door_inset, panel_height - 0.8, 0),
        ]

        # 3. Внутренний контур двери
        inner_frame = [
            (door_inset + frame_thickness, 0.1 + frame_thickness, door_depth),
            (
                panel_width - door_inset - frame_thickness,
                0.1 + frame_thickness,
                door_depth,
            ),
            (
                panel_width - door_inset - frame_thickness,
                panel_height - 0.8 - frame_thickness,
                door_depth,
            ),
            (
                door_inset + frame_thickness,
                panel_height - 0.8 - frame_thickness,
                door_depth,
            ),
        ]

        # 4. Строим трехуровневую геометрию для двери
        wall = RectangleWithCutout(
            outer_points=outer_wall, cutout_points=outer_frame, color=wall_color
        )
        panel.add_child(wall)

        frame = RectangleWithCutout(
            outer_points=outer_frame,
            cutout_points=inner_frame,
            color="#5D4037",  # Темно-коричневый для дверной рамы
        )
        panel.add_child(frame)

        door = SimplePolygon(points=inner_frame, color=door_color)
        panel.add_child(door)

        # 5. Добавляем декоративный козырек над дверью
        canopy_height = 0.6
        canopy_depth = 0.3
        canopy_y = panel_height - 0.8

        canopy_points = [
            (door_inset - 0.2, canopy_y, 0),
            (panel_width - door_inset + 0.2, canopy_y, 0),
            (panel_width - door_inset + 0.2, canopy_y + canopy_height, canopy_depth),
            (door_inset - 0.2, canopy_y + canopy_height, canopy_depth),
        ]

        panel.add_child(SimplePolygon(points=canopy_points, color="#654321"))

        return panel

    # =============================
    # СОЗДАНИЕ ПАНЕЛИ С ПРЯМОУГОЛЬНЫМ КАРНИЗОМ
    # =============================
    def create_rectangular_cornice_panel(panel_width, panel_height):
        """Создаёт панель с прямоугольным карнизом и окном"""
        panel = GeometrySegment()

        # 1. Основная стена
        wall_contour = [
            (0, 0, 0),
            (panel_width, 0, 0),
            (panel_width, panel_height, 0),
            (0, panel_height, 0),
        ]
        panel.add_child(
            SimplePolygon(
                points=wall_contour,
                color=wall_color,
            )
        )

        # 2. ПРЯМОУГОЛЬНЫЙ КАРНИЗ (без наклона)
        cornice_height = 0.6
        cornice_depth = 0.5
        cornice_y = panel_height - cornice_height

        # Нижняя горизонтальная часть карниза
        cornice_bottom = [
            (0, cornice_y, 0),
            (panel_width, cornice_y, 0),
            (panel_width, cornice_y + cornice_height, 0),
            (0, cornice_y + cornice_height, 0),
        ]

        # Передняя вертикальная часть карниза
        cornice_front = [
            (0, cornice_y, 0),
            (panel_width, cornice_y, 0),
            (panel_width, cornice_y, cornice_depth),
            (0, cornice_y, cornice_depth),
        ]

        # Верхняя горизонтальная часть карниза
        cornice_top = [
            (0, cornice_y + cornice_height, 0),
            (panel_width, cornice_y + cornice_height, 0),
            (panel_width, cornice_y + cornice_height, cornice_depth),
            (0, cornice_y + cornice_height, cornice_depth),
        ]

        # Добавляем части карниза
        panel.add_child(
            SimplePolygon(
                points=cornice_bottom,
                color="#654321",
            )
        )

        panel.add_child(
            SimplePolygon(
                points=cornice_front,
                color="#654321",
            )
        )

        panel.add_child(
            SimplePolygon(
                points=cornice_top,
                color="#654321",
            )
        )

        # 3. Добавляем окно через правильную систему вырезов
        window_panel = create_window_panel(panel_width, panel_height)
        # Берем только стекло и рамку (стена уже добавлена)
        panel.add_child(window_panel.children[1])  # Рамка
        panel.add_child(window_panel.children[2])  # Стекло

        return panel

    # =============================
    # ФОРМИРОВАНИЕ СТЕН ЗДАНИЯ
    # =============================

    # Панели для первого этажа
    ground_panel = create_window_panel(
        panel_width, panel_height_ground, is_ground_floor=True
    )
    door_panel = create_door_panel(
        panel_width, panel_height_ground
    )  # Центральная панель с дверью

    # Панели для типовых этажей
    typical_panel = create_window_panel(
        panel_width, panel_height_typical, is_ground_floor=False
    )

    # Панели для верхнего этажа с прямоугольным карнизом
    cornice_panel = create_rectangular_cornice_panel(panel_width, panel_height_typical)

    # Фасадные стены (front) - с дверью в центре
    front_panels_ground = (
        [ground_panel] * 2 + [door_panel] + [ground_panel] * 3
    )  # 6 панелей всего
    building.add_wall(
        wall_type="front",
        panels=front_panels_ground,
        floor_type="ground",
        invert=False,
    )

    building.add_wall(
        wall_type="front",
        panels=[typical_panel] * 6,
        floor_type="typical",
        invert=False,
    )

    if floor_count > 2:
        building.add_wall(
            wall_type="front",
            panels=[cornice_panel] * 6,
            floor_type="typical",
            invert=False,
        )

    # ЗАДНИЕ СТЕНЫ
    building.add_wall(
        wall_type="back",
        panels=[ground_panel] * 6,
        floor_type="ground",
        invert=True,
    )

    building.add_wall(
        wall_type="back",
        panels=[typical_panel] * 6,
        floor_type="typical",
        invert=True,
    )

    # БОКОВЫЕ СТЕНЫ - ИСПРАВЛЕНО: ОДИНАКОВАЯ ВЫСОТА С ПЕРЕДНЕЙ
    side_panel_width = depth / 3

    # Создаем боковые панели с правильной высотой для первого этажа
    side_panel_ground = create_window_panel(
        side_panel_width, panel_height_ground, is_ground_floor=True
    )
    side_panel_typical = create_window_panel(
        side_panel_width, panel_height_typical, is_ground_floor=False
    )

    # Левая стена
    building.add_wall(
        wall_type="left",
        panels=[side_panel_ground] * 3,
        floor_type="ground",
        invert=False,
    )

    building.add_wall(
        wall_type="left",
        panels=[side_panel_typical] * 3,
        floor_type="typical",
        invert=False,
    )

    # Правая стена
    building.add_wall(
        wall_type="right",
        panels=[side_panel_ground] * 3,
        floor_type="ground",
        invert=True,
    )

    building.add_wall(
        wall_type="right",
        panels=[side_panel_typical] * 3,
        floor_type="typical",
        invert=True,
    )

    # =============================
    # СОЗДАНИЕ КРЫШИ С УВЕЛИЧЕННОЙ ТРУБОЙ (ИСПРАВЛЕНО)
    # =============================

    # Основные вершины двускатной крыши
    roof_vertices = [
        (0, 0, 0),  # 0: Левый передний низ
        (width, 0, 0),  # 1: Правый передний низ
        (width, 0, depth),  # 2: Правый задний низ
        (0, 0, depth),  # 3: Левый задний низ
        (0, roof_height, depth / 2),  # 4: Левый конёк
        (width, roof_height, depth / 2),  # 5: Правый конёк
    ]

    # Грани крыши
    roof_faces = [
        [0, 1, 5, 4],  # Передний скат
        [3, 2, 5, 4],  # Задний скат
        [0, 3, 4],  # Левый фронтон
        [1, 2, 5],  # Правый фронтон
    ]

    # Цвета крыши
    roof_colors = [
        roof_color,
        roof_color,  # Скаты
        "#A0522D",
        "#A0522D",  # Фронтоны
    ]

    # --- УВЕЛИЧЕННАЯ ДЫМОВАЯ ТРУБА (ИСПРАВЛЕНА) ---
    chimney_x = width * 0.65  # Смещена ближе к центру
    chimney_z_start = depth * 0.25
    chimney_z_end = depth * 0.45
    chimney_height = 6.0  # Увеличена с 2.5 до 4.0
    chimney_width = 1.8

    # Сохраняем текущее количество вершин для правильных индексов
    chimney_base_index = len(roof_vertices)

    # Добавляем вершины трубы (только внешние)
    roof_vertices.extend(
        [
            # Нижние вершины
            (chimney_x, 0, chimney_z_start),  # 6: Нижний левый передний
            (
                chimney_x + chimney_width,
                0,
                chimney_z_start,
            ),  # 7: Нижний правый передний
            (chimney_x + chimney_width, 0, chimney_z_end),  # 8: Нижний правый задний
            (chimney_x, 0, chimney_z_end),  # 9: Нижний левый задний
            # Верхние вершины
            (chimney_x, chimney_height, chimney_z_start),  # 10: Верхний левый передний
            (
                chimney_x + chimney_width,
                chimney_height,
                chimney_z_start,
            ),  # 11: Верхний правый передний
            (
                chimney_x + chimney_width,
                chimney_height,
                chimney_z_end,
            ),  # 12: Верхний правый задний
            (chimney_x, chimney_height, chimney_z_end),  # 13: Верхний левый задний
        ]
    )

    # Грани трубы (только внешние)
    chimney_faces = [
        # Передняя стенка (нижняя левая -> нижняя правая -> верхняя правая -> верхняя левая)
        [
            chimney_base_index,
            chimney_base_index + 1,
            chimney_base_index + 5,
            chimney_base_index + 4,
        ],
        # Правая стенка
        [
            chimney_base_index + 1,
            chimney_base_index + 2,
            chimney_base_index + 6,
            chimney_base_index + 5,
        ],
        # Задняя стенка
        [
            chimney_base_index + 2,
            chimney_base_index + 3,
            chimney_base_index + 7,
            chimney_base_index + 6,
        ],
        # Левая стенка
        [
            chimney_base_index,
            chimney_base_index + 3,
            chimney_base_index + 4,
            chimney_base_index + 7,
        ],
        # Верхняя крышка
        [
            chimney_base_index + 4,
            chimney_base_index + 5,
            chimney_base_index + 6,
            chimney_base_index + 7,
        ],
    ]

    # Цвета для трубы
    chimney_colors = [chimney_color] * 4 + ["#654321"]  # 4 стены + крышка
    # Добавляем грани и цвета трубы
    roof_faces.extend(chimney_faces)
    roof_colors.extend(chimney_colors)

    # Добавляем кастомную крышу с трубой
    building.add_custom_roof(
        vertices=roof_vertices,
        faces=roof_faces,
        colors=roof_colors,
    )

    # =============================
    # ФИНАЛИЗАЦИЯ
    # =============================
    building.finalize()
    filename = f"buildings/{name}.json"
    building.save_to_file(filename)

    print(f"✅ Здание '{name}' успешно создано с исправлениями!")
    print("✨ Все боковые панели первого этажа имеют одинаковую высоту")
    print("🚪 Добавлен парадный вход с декоративным козырьком")
    print("🔳 Карнизы теперь прямоугольные с увеличенной глубиной")
    print("🏭 Дымовая труба увеличена до реалистичных размеров")
    print("🔧 Исправлена ошибка с индексами вершин крыши")

    return building


# Пример использования
if __name__ == "__main__":
    create_historical_building(
        name="grand_historical_building",
        description="Величественное историческое здание с парадным входом",
        floor_count=3,
        width=24.0,  # Увеличена ширина для лучшего размещения входа
        depth=18.0,
        ground_floor_height=4.5,  # Увеличена высота первого этажа
        typical_floor_height=3.4,
        roof_height=4.2,
        wall_color="#C19A6B",
        window_color="#B0E0E6",
        roof_color="#8B4513",
        chimney_color="#A52A2A",
        door_color="#8B4513",
    )
