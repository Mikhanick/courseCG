from createBuilding import (
    Building,
    GeometrySegment,
    RectangleWithCutout,
    SimplePolygon,
    RoofGeometry,
)


def create_stepped_panel_building(
    name="stepped_panel_building",
    description="Панельное здание со ступенчатым фасадом и рамками окон как вырезы",
    floor_count=5,
    roof_type="flat",
    roof_height=1.2,
    window_color=None,
    center_panel_color_type="blue",  # "blue" или "orange"
):
    """
    Создает 3D-модель панельного здания с необычной геометрией:
    - Все стены создаются как единые панели с многоуровневыми вырезами
    - Рамки окон реализованы как дополнительные вырезы, соединяющие внешний и внутренний контуры
    - 6 панелей во фронтальной и задней части по 3.5 метра
    - Высота панелей 2.9 метра
    - Панели 3 и 4 утоплены внутрь на 1.5 метра
    - Панели 2 и 5 имеют наклонную поверхность
    - 4 панели в глубину
    - На первом этаже одна из боковых панелей имеет входную группу с вырезом под дверь
    - Центральные панели могут быть светло-голубыми или оранжевыми

    Параметры:
    name -- имя здания
    description -- описание здания
    floor_count -- количество этажей
    roof_type -- тип крыши ("flat", "simple", "custom")
    roof_height -- высота крыши в метрах
    window_color -- цвет окон (если None, будет случайный)
    center_panel_color_type -- цвет центральных панелей: "blue" или "orange"

    Возвращает:
    Building -- готовый объект здания
    """

    # Основные размеры здания
    panel_width = 3.5  # ширина одной панели
    panel_height = 2.9  # высота панели
    building_width = panel_width * 6  # общая ширина здания
    building_depth = panel_width * 4  # общая глубина здания (4 панели)

    # Инициализация здания
    building = Building(
        name=name,
        description=description,
        floor_count=floor_count,
        width=building_width,
        depth=building_depth,
        ground_floor_height=panel_height,
        typical_floor_height=panel_height,
        roof_type=roof_type,
        roof_height=roof_height,
    )

    # Если цвет окон не задан, используем голубоватый для контраста
    window_color = window_color or "#7ec0ee"  # Светло-голубой для окон

    # Цвета панелей в зависимости от типа
    if center_panel_color_type.lower() == "orange":
        center_panel_color = "#ffa07a"  # Светло-оранжевый (лосось)
        center_panel_description = "оранжевыми"
    else:
        center_panel_color = "#add8e6"  # Светло-голубой
        center_panel_description = "голубыми"

    side_panel_color = "#d3d3d3"  # Светло-серый для остальных панелей
    frame_color = "#505050"  # Цвет рамок окон
    deep_frame_color = "#3a3a3a"  # Более темный цвет для глубоких рамок

    # Параметры окон и рамок
    window_offset = 0.5  # Отступ от краев для окон
    window_depth = 0.03  # Глубина выступа окна
    frame_thickness = 0.03  # Толщина рамки окна
    frame_depth = 0.01  # Глубина выступа рамки относительно стены

    # Глубина утопления центральных панелей
    recess_depth = 1.5

    # =============== СОЗДАНИЕ ПАНЕЛЕЙ С МНОГОУРОВНЕВЫМИ ВЫРЕЗАМИ ===============

    # 1. Панель без смещения с многоуровневыми вырезами для окна и рамки
    def create_flat_panel_with_nested_cutouts(
        width, height, color=side_panel_color, with_window=True
    ):
        """Создает плоскую панель с многоуровневыми вырезами для окна и рамки"""
        panel = GeometrySegment()

        # Внешний контур панели (стена)
        outer_frame = [
            (0, 0, 0),  # Нижний левый
            (width, 0, 0),  # Нижний правый
            (width, height, 0),  # Верхний правый
            (0, height, 0),  # Верхний левый
        ]

        if with_window:
            # Внешний контур рамки (соединяется с основной стеной)
            outer_frame_frame = [
                (
                    window_offset - frame_thickness - 0.02,
                    window_offset - frame_thickness - 0.02,
                    0,
                ),
                (
                    width - window_offset + frame_thickness + 0.02,
                    window_offset - frame_thickness - 0.02,
                    0,
                ),
                (
                    width - window_offset + frame_thickness + 0.02,
                    height - window_offset + frame_thickness + 0.02,
                    0,
                ),
                (
                    window_offset - frame_thickness - 0.02,
                    height - window_offset + frame_thickness + 0.02,
                    0,
                ),
            ]

            # Внутренний контур рамки (граница со стеклом)
            inner_frame_frame = [
                (window_offset, window_offset, window_depth),
                (width - window_offset, window_offset, window_depth),
                (width - window_offset, height - window_offset, window_depth),
                (window_offset, height - window_offset, window_depth),
            ]

            # Создаем панель с двойным вырезом
            panel.add_child(
                RectangleWithCutout(
                    outer_points=outer_frame,
                    cutout_points=outer_frame_frame,
                    color=color,
                )
            )

            # Создаем рамку как отдельную панель с вырезом
            frame_panel = GeometrySegment()
            frame_panel.add_child(
                RectangleWithCutout(
                    outer_points=outer_frame_frame,
                    cutout_points=inner_frame_frame,
                    color=deep_frame_color,
                )
            )
            panel.add_child(frame_panel)

            # Стекло окна (заполняет внутренний вырез)
            panel.add_child(
                SimplePolygon(
                    points=inner_frame_frame,
                    color=window_color,
                )
            )
        else:
            # Если окна нет, создаем сплошную панель
            panel.add_child(
                SimplePolygon(
                    points=outer_frame,
                    color=color,
                )
            )

        return panel

    # 2. Утопленная панель с многоуровневыми вырезами
    def create_recessed_panel_with_nested_cutouts(
        width, height, recess=recess_depth, color=center_panel_color, with_window=True
    ):
        """Создает утопленную панель с многоуровневыми вырезами для окна и рамки"""
        panel = GeometrySegment()

        # Внешний контур панели (утопленная стена)
        outer_frame = [
            (0, 0, -recess),  # Нижний левый
            (width, 0, -recess),  # Нижний правый
            (width, height, -recess),  # Верхний правый
            (0, height, -recess),  # Верхний левый
        ]

        if with_window:
            # Внешний контур рамки
            outer_frame_frame = [
                (
                    window_offset - frame_thickness - 0.02,
                    window_offset - frame_thickness - 0.02,
                    -recess,
                ),
                (
                    width - window_offset + frame_thickness + 0.02,
                    window_offset - frame_thickness - 0.02,
                    -recess,
                ),
                (
                    width - window_offset + frame_thickness + 0.02,
                    height - window_offset + frame_thickness + 0.02,
                    -recess,
                ),
                (
                    window_offset - frame_thickness - 0.02,
                    height - window_offset + frame_thickness + 0.02,
                    -recess,
                ),
            ]

            # Внутренний контур рамки
            inner_frame_frame = [
                (window_offset, window_offset, -recess + window_depth),
                (width - window_offset, window_offset, -recess + window_depth),
                (width - window_offset, height - window_offset, -recess + window_depth),
                (window_offset, height - window_offset, -recess + window_depth),
            ]

            # Создаем утопленную панель с двойным вырезом
            panel.add_child(
                RectangleWithCutout(
                    outer_points=outer_frame,
                    cutout_points=outer_frame_frame,
                    color=color,
                )
            )

            # Создаем рамку
            frame_panel = GeometrySegment()
            frame_panel.add_child(
                RectangleWithCutout(
                    outer_points=outer_frame_frame,
                    cutout_points=inner_frame_frame,
                    color=deep_frame_color,
                )
            )
            panel.add_child(frame_panel)

            # Стекло окна
            panel.add_child(
                SimplePolygon(
                    points=inner_frame_frame,
                    color=window_color,
                )
            )
        else:
            # Сплошная утопленная панель
            panel.add_child(
                SimplePolygon(
                    points=outer_frame,
                    color=color,
                )
            )

        return panel

    # 3. Наклонная панель с многоуровневыми вырезами
    def create_sloped_panel_with_nested_cutouts(
        width,
        height,
        start_z=0,
        end_z=-recess_depth,
        color=side_panel_color,
        with_window=True,
    ):
        """Создает наклонную панель с многоуровневыми вырезами для окна и рамки"""
        panel = GeometrySegment()

        # Внешний контур панели (наклонная стена)
        outer_frame = [
            (0, 0, start_z),  # Нижний левый
            (width, 0, end_z),  # Нижний правый
            (width, height, end_z),  # Верхний правый
            (0, height, start_z),  # Верхний левый
        ]

        if with_window:
            # Рассчитываем наклон для правильного позиционирования вырезов
            slope = (end_z - start_z) / width

            # Внешний контур рамки (на поверхности стены)
            outer_left_x = window_offset - frame_thickness - 0.02
            outer_right_x = width - window_offset + frame_thickness + 0.02
            outer_top_y = height - window_offset + frame_thickness + 0.02
            outer_bottom_y = window_offset - frame_thickness - 0.02

            outer_left_bottom_z = start_z + slope * outer_left_x
            outer_right_bottom_z = start_z + slope * outer_right_x
            outer_left_top_z = start_z + slope * outer_left_x
            outer_right_top_z = start_z + slope * outer_right_x

            outer_frame_frame = [
                (outer_left_x, outer_bottom_y, outer_left_bottom_z),
                (outer_right_x, outer_bottom_y, outer_right_bottom_z),
                (outer_right_x, outer_top_y, outer_right_top_z),
                (outer_left_x, outer_top_y, outer_left_top_z),
            ]

            # Внутренний контур рамки (граница со стеклом)
            inner_left_x = window_offset
            inner_right_x = width - window_offset
            inner_top_y = height - window_offset
            inner_bottom_y = window_offset

            inner_left_bottom_z = start_z + slope * inner_left_x + window_depth
            inner_right_bottom_z = start_z + slope * inner_right_x + window_depth
            inner_left_top_z = start_z + slope * inner_left_x + window_depth
            inner_right_top_z = start_z + slope * inner_right_x + window_depth

            inner_frame_frame = [
                (inner_left_x, inner_bottom_y, inner_left_bottom_z),
                (inner_right_x, inner_bottom_y, inner_right_bottom_z),
                (inner_right_x, inner_top_y, inner_right_top_z),
                (inner_left_x, inner_top_y, inner_left_top_z),
            ]

            # Создаем наклонную панель с двойным вырезом
            panel.add_child(
                RectangleWithCutout(
                    outer_points=outer_frame,
                    cutout_points=outer_frame_frame,
                    color=color,
                )
            )

            # Создаем рамку как отдельную наклонную панель
            frame_panel = GeometrySegment()
            frame_panel.add_child(
                RectangleWithCutout(
                    outer_points=outer_frame_frame,
                    cutout_points=inner_frame_frame,
                    color=deep_frame_color,
                )
            )
            panel.add_child(frame_panel)

            # Стекло окна
            panel.add_child(
                SimplePolygon(
                    points=inner_frame_frame,
                    color=window_color,
                )
            )
        else:
            # Сплошная наклонная панель
            panel.add_child(
                SimplePolygon(
                    points=outer_frame,
                    color=color,
                )
            )

        return panel

    # 4. Панель для боковых стен с многоуровневыми вырезами
    def create_side_panel_with_nested_cutouts(
        width, height, depth=panel_width, color=side_panel_color, with_window=True
    ):
        """Создает панель для боковых стен с многоуровневыми вырезами для окна и рамки"""
        panel = GeometrySegment()

        # Внешний контур панели (боковая стена)
        outer_frame = [
            (0, 0, 0),  # Нижний левый
            (depth, 0, 0),  # Нижний правый (по глубине здания)
            (depth, height, 0),  # Верхний правый
            (0, height, 0),  # Верхний левый
        ]

        if with_window:
            # Внешний контур рамки
            outer_frame_frame = [
                (
                    window_offset - frame_thickness - 0.02,
                    window_offset - frame_thickness - 0.02,
                    0,
                ),
                (
                    depth - window_offset + frame_thickness + 0.02,
                    window_offset - frame_thickness - 0.02,
                    0,
                ),
                (
                    depth - window_offset + frame_thickness + 0.02,
                    height - window_offset + frame_thickness + 0.02,
                    0,
                ),
                (
                    window_offset - frame_thickness - 0.02,
                    height - window_offset + frame_thickness + 0.02,
                    0,
                ),
            ]

            # Внутренний контур рамки
            inner_frame_frame = [
                (window_offset, window_offset, window_depth),
                (depth - window_offset, window_offset, window_depth),
                (depth - window_offset, height - window_offset, window_depth),
                (window_offset, height - window_offset, window_depth),
            ]

            # Создаем боковую панель с двойным вырезом
            panel.add_child(
                RectangleWithCutout(
                    outer_points=outer_frame,
                    cutout_points=outer_frame_frame,
                    color=color,
                )
            )

            # Создаем рамку
            frame_panel = GeometrySegment()
            frame_panel.add_child(
                RectangleWithCutout(
                    outer_points=outer_frame_frame,
                    cutout_points=inner_frame_frame,
                    color=deep_frame_color,
                )
            )
            panel.add_child(frame_panel)

            # Стекло окна
            panel.add_child(
                SimplePolygon(
                    points=inner_frame_frame,
                    color=window_color,
                )
            )
        else:
            # Сплошная боковая панель
            panel.add_child(
                SimplePolygon(
                    points=outer_frame,
                    color=color,
                )
            )

        return panel

    # 5. Панель с входной группой (многоуровневые вырезы под дверь)
    def create_entrance_panel_with_nested_cutouts(
        width, height, depth=panel_width, color=side_panel_color
    ):
        """Создает панель с входной группой с многоуровневыми вырезами под дверь"""
        panel = GeometrySegment()

        # Внешний контур панели (стена)
        outer_frame = [
            (0, 0, 0),  # Нижний левый
            (depth, 0, 0),  # Нижний правый
            (depth, height, 0),  # Верхний правый
            (0, height, 0),  # Верхний левый
        ]

        # Дверной проем (шире и ниже, чем обычное окно)
        door_width = 1.8
        door_height = 2.2
        frame_thickness_door = 0.05
        door_offset_x = (depth - door_width) / 2  # По центру по ширине
        door_offset_y = 0.2  # Небольшой отступ от пола
        door_depth = 0.05  # Глубина выступа дверной рамы

        # Внешний контур дверной рамки
        outer_frame_frame = [
            (
                door_offset_x - frame_thickness_door - 0.03,
                door_offset_y - frame_thickness_door - 0.03,
                0,
            ),
            (
                door_offset_x + door_width + frame_thickness_door + 0.03,
                door_offset_y - frame_thickness_door - 0.03,
                0,
            ),
            (
                door_offset_x + door_width + frame_thickness_door + 0.03,
                door_offset_y + door_height + frame_thickness_door + 0.03,
                0,
            ),
            (
                door_offset_x - frame_thickness_door - 0.03,
                door_offset_y + door_height + frame_thickness_door + 0.03,
                0,
            ),
        ]

        # Внутренний контур дверной рамки (граница со стеклом)
        inner_frame_frame = [
            (door_offset_x, door_offset_y, door_depth),
            (door_offset_x + door_width, door_offset_y, door_depth),
            (door_offset_x + door_width, door_offset_y + door_height, door_depth),
            (door_offset_x, door_offset_y + door_height, door_depth),
        ]

        # Создаем панель с двойным вырезом для двери
        panel.add_child(
            RectangleWithCutout(
                outer_points=outer_frame,
                cutout_points=outer_frame_frame,
                color=color,
            )
        )

        # Создаем дверную рамку
        frame_panel = GeometrySegment()
        frame_panel.add_child(
            RectangleWithCutout(
                outer_points=outer_frame_frame,
                cutout_points=inner_frame_frame,
                color="#2a2a2a",  # Очень темная рамка для двери
            )
        )
        panel.add_child(frame_panel)

        # Стеклянная дверь
        panel.add_child(
            SimplePolygon(
                points=inner_frame_frame,
                color="#87cefa",  # Светло-голубое стекло
            )
        )

        # Навес над входом
        canopy_depth = 0.4  # Глубина навеса
        canopy_width = door_width + 0.6  # Шире двери
        canopy_height = door_offset_y + door_height + 0.1  # Над дверью

        canopy_points = [
            ((depth - canopy_width) / 2, canopy_height, 0),  # Нижний левый у стены
            (
                (depth - canopy_width) / 2 + canopy_width,
                canopy_height,
                0,
            ),  # Нижний правый у стены
            (
                (depth - canopy_width) / 2 + canopy_width,
                canopy_height,
                -canopy_depth,
            ),  # Верхний правый (выступает)
            ((depth - canopy_width) / 2, canopy_height, -canopy_depth),  # Верхний левый
        ]

        panel.add_child(
            SimplePolygon(
                points=canopy_points,
                color="#555555",  # Темно-серый навес
            )
        )

        # Ступени у входа (упрощенная версия)
        step_height = 0.15
        step_depth = 0.3
        step_width = door_width + 0.4

        for i in range(3):  # 3 ступени
            step_y = door_offset_y - (i + 1) * step_height
            if step_y < 0:
                break

            step_points = [
                ((depth - step_width) / 2, step_y, 0),
                ((depth - step_width) / 2 + step_width, step_y, 0),
                (
                    (depth - step_width) / 2 + step_width,
                    step_y + step_height,
                    step_depth,
                ),
                ((depth - step_width) / 2, step_y + step_height, step_depth),
            ]

            panel.add_child(
                SimplePolygon(
                    points=step_points,
                    color="#707070",  # Серые ступени
                )
            )

        return panel

    # =============== ФОРМИРОВАНИЕ СТЕН ===============

    # 1. Фронтальные стены (типовые этажи)
    flat_panel = create_flat_panel_with_nested_cutouts(
        panel_width, panel_height, color=side_panel_color
    )
    recessed_panel = create_recessed_panel_with_nested_cutouts(
        panel_width, panel_height, color=center_panel_color
    )
    sloped_panel_front_2 = create_sloped_panel_with_nested_cutouts(
        panel_width,
        panel_height,
        start_z=0,
        end_z=-recess_depth,
        color=side_panel_color,
    )  # Панель 2
    sloped_panel_front_5 = create_sloped_panel_with_nested_cutouts(
        panel_width,
        panel_height,
        start_z=-recess_depth,
        end_z=0,
        color=side_panel_color,
    )  # Панель 5

    # Формируем последовательность панелей для фронтальной стены
    front_panels = [
        flat_panel,  # Панель 1
        sloped_panel_front_2,  # Панель 2 (наклон от выступа к утоплению)
        recessed_panel,  # Панель 3 (утопленная, цвет зависит от параметра)
        recessed_panel,  # Панель 4 (утопленная, цвет зависит от параметра)
        sloped_panel_front_5,  # Панель 5 (наклон от утопления к выступу)
        flat_panel,  # Панель 6
    ]

    # Добавляем фронтальные стены для типовых этажей
    building.add_wall(
        wall_type="front",
        panels=front_panels,
        floor_type="typical",
        invert=False,
    )

    # 2. Задние стены (зеркальные фронтальным с корректными наклонами)
    flat_panel_back = create_flat_panel_with_nested_cutouts(
        panel_width, panel_height, color=side_panel_color
    )
    recessed_panel_back = create_recessed_panel_with_nested_cutouts(
        panel_width, panel_height, color=center_panel_color
    )
    sloped_panel_back_2 = create_sloped_panel_with_nested_cutouts(
        panel_width,
        panel_height,
        start_z=0,
        end_z=-recess_depth,
        color=side_panel_color,
    )  # Панель 2
    sloped_panel_back_5 = create_sloped_panel_with_nested_cutouts(
        panel_width,
        panel_height,
        start_z=-recess_depth,
        end_z=0,
        color=side_panel_color,
    )  # Панель 5

    back_panels = [
        flat_panel_back,  # Панель 1
        sloped_panel_back_2,  # Панель 2 (наклон от выступа к утоплению)
        recessed_panel_back,  # Панель 3
        recessed_panel_back,  # Панель 4
        sloped_panel_back_5,  # Панель 5 (наклон от утопления к выступу)
        flat_panel_back,  # Панель 6
    ]

    # Для задних стен ВСЕГДА инвертируем нормали
    building.add_wall(
        wall_type="back",
        panels=back_panels,
        floor_type="typical",
        invert=True,
    )

    # 3. Боковые стены с окнами на всех этажах

    # Левая стена (4 панели по глубине)
    side_panel_left = create_side_panel_with_nested_cutouts(
        panel_width,
        panel_height,
        depth=panel_width,
        color=side_panel_color,
        with_window=True,
    )
    left_panels = [side_panel_left] * 4  # 4 одинаковых панели с окнами

    building.add_wall(
        wall_type="left",
        panels=left_panels,
        floor_type="typical",
        invert=False,
    )

    # Правая стена (зеркальная левой, но с инвертированными нормалями)
    side_panel_right = create_side_panel_with_nested_cutouts(
        panel_width,
        panel_height,
        depth=panel_width,
        color=side_panel_color,
        with_window=True,
    )
    right_panels = [side_panel_right] * 4

    building.add_wall(
        wall_type="right",
        panels=right_panels,
        floor_type="typical",
        invert=True,
    )

    # 4. Стены первого этажа (ground floor)
    window_offset_ground = 0.6  # Больший отступ для первого этажа

    # Функции для первого этажа с другими параметрами окон
    def create_flat_panel_ground_with_nested_cutouts(
        width, height, color=side_panel_color
    ):
        panel = GeometrySegment()
        outer_frame = [(0, 0, 0), (width, 0, 0), (width, height, 0), (0, height, 0)]

        # Меньшие окна для первого этажа
        outer_frame_frame = [
            (
                window_offset_ground - frame_thickness - 0.02,
                window_offset_ground - frame_thickness - 0.02,
                0,
            ),
            (
                width - window_offset_ground + frame_thickness + 0.02,
                window_offset_ground - frame_thickness - 0.02,
                0,
            ),
            (
                width - window_offset_ground + frame_thickness + 0.02,
                height - window_offset_ground + frame_thickness + 0.02,
                0,
            ),
            (
                window_offset_ground - frame_thickness - 0.02,
                height - window_offset_ground + frame_thickness + 0.02,
                0,
            ),
        ]

        inner_frame_frame = [
            (window_offset_ground, window_offset_ground, window_depth),
            (width - window_offset_ground, window_offset_ground, window_depth),
            (width - window_offset_ground, height - window_offset_ground, window_depth),
            (window_offset_ground, height - window_offset_ground, window_depth),
        ]

        panel.add_child(
            RectangleWithCutout(
                outer_points=outer_frame,
                cutout_points=outer_frame_frame,
                color=color,
            )
        )

        frame_panel = GeometrySegment()
        frame_panel.add_child(
            RectangleWithCutout(
                outer_points=outer_frame_frame,
                cutout_points=inner_frame_frame,
                color=deep_frame_color,
            )
        )
        panel.add_child(frame_panel)

        panel.add_child(
            SimplePolygon(
                points=inner_frame_frame,
                color=window_color,
            )
        )

        return panel

    def create_recessed_panel_ground_with_nested_cutouts(
        width, height, recess=recess_depth, color=center_panel_color
    ):
        panel = GeometrySegment()
        outer_frame = [
            (0, 0, -recess),
            (width, 0, -recess),
            (width, height, -recess),
            (0, height, -recess),
        ]

        outer_frame_frame = [
            (
                window_offset_ground - frame_thickness - 0.02,
                window_offset_ground - frame_thickness - 0.02,
                -recess,
            ),
            (
                width - window_offset_ground + frame_thickness + 0.02,
                window_offset_ground - frame_thickness - 0.02,
                -recess,
            ),
            (
                width - window_offset_ground + frame_thickness + 0.02,
                height - window_offset_ground + frame_thickness + 0.02,
                -recess,
            ),
            (
                window_offset_ground - frame_thickness - 0.02,
                height - window_offset_ground + frame_thickness + 0.02,
                -recess,
            ),
        ]

        inner_frame_frame = [
            (window_offset_ground, window_offset_ground, -recess + window_depth),
            (
                width - window_offset_ground,
                window_offset_ground,
                -recess + window_depth,
            ),
            (
                width - window_offset_ground,
                height - window_offset_ground,
                -recess + window_depth,
            ),
            (
                window_offset_ground,
                height - window_offset_ground,
                -recess + window_depth,
            ),
        ]

        panel.add_child(
            RectangleWithCutout(
                outer_points=outer_frame,
                cutout_points=outer_frame_frame,
                color=color,
            )
        )

        frame_panel = GeometrySegment()
        frame_panel.add_child(
            RectangleWithCutout(
                outer_points=outer_frame_frame,
                cutout_points=inner_frame_frame,
                color=deep_frame_color,
            )
        )
        panel.add_child(frame_panel)

        panel.add_child(
            SimplePolygon(
                points=inner_frame_frame,
                color=window_color,
            )
        )

        return panel

    def create_sloped_panel_ground_with_nested_cutouts(
        width, height, start_z=0, end_z=-recess_depth, color=side_panel_color
    ):
        panel = GeometrySegment()
        outer_frame = [
            (0, 0, start_z),
            (width, 0, end_z),
            (width, height, end_z),
            (0, height, start_z),
        ]

        slope = (end_z - start_z) / width

        outer_left_x = window_offset_ground - frame_thickness - 0.02
        outer_right_x = width - window_offset_ground + frame_thickness + 0.02
        outer_top_y = height - window_offset_ground + frame_thickness + 0.02
        outer_bottom_y = window_offset_ground - frame_thickness - 0.02

        outer_left_bottom_z = start_z + slope * outer_left_x
        outer_right_bottom_z = start_z + slope * outer_right_x
        outer_left_top_z = start_z + slope * outer_left_x
        outer_right_top_z = start_z + slope * outer_right_x

        outer_frame_frame = [
            (outer_left_x, outer_bottom_y, outer_left_bottom_z),
            (outer_right_x, outer_bottom_y, outer_right_bottom_z),
            (outer_right_x, outer_top_y, outer_right_top_z),
            (outer_left_x, outer_top_y, outer_left_top_z),
        ]

        inner_left_x = window_offset_ground
        inner_right_x = width - window_offset_ground
        inner_top_y = height - window_offset_ground
        inner_bottom_y = window_offset_ground

        inner_left_bottom_z = start_z + slope * inner_left_x + window_depth
        inner_right_bottom_z = start_z + slope * inner_right_x + window_depth
        inner_left_top_z = start_z + slope * inner_left_x + window_depth
        inner_right_top_z = start_z + slope * inner_right_x + window_depth

        inner_frame_frame = [
            (inner_left_x, inner_bottom_y, inner_left_bottom_z),
            (inner_right_x, inner_bottom_y, inner_right_bottom_z),
            (inner_right_x, inner_top_y, inner_right_top_z),
            (inner_left_x, inner_top_y, inner_left_top_z),
        ]

        panel.add_child(
            RectangleWithCutout(
                outer_points=outer_frame,
                cutout_points=outer_frame_frame,
                color=color,
            )
        )

        frame_panel = GeometrySegment()
        frame_panel.add_child(
            RectangleWithCutout(
                outer_points=outer_frame_frame,
                cutout_points=inner_frame_frame,
                color=deep_frame_color,
            )
        )
        panel.add_child(frame_panel)

        panel.add_child(
            SimplePolygon(
                points=inner_frame_frame,
                color=window_color,
            )
        )

        return panel

    # Создаем панели для первого этажа
    flat_panel_g = create_flat_panel_ground_with_nested_cutouts(
        panel_width, panel_height, color=side_panel_color
    )
    recessed_panel_g = create_recessed_panel_ground_with_nested_cutouts(
        panel_width, panel_height, color=center_panel_color
    )
    sloped_panel_2_g = create_sloped_panel_ground_with_nested_cutouts(
        panel_width,
        panel_height,
        start_z=0,
        end_z=-recess_depth,
        color=side_panel_color,
    )
    sloped_panel_5_g = create_sloped_panel_ground_with_nested_cutouts(
        panel_width,
        panel_height,
        start_z=-recess_depth,
        end_z=0,
        color=side_panel_color,
    )

    # Фронтальные стены первого этажа
    front_panels_ground = [
        flat_panel_g,
        sloped_panel_2_g,
        recessed_panel_g,
        recessed_panel_g,
        sloped_panel_5_g,
        flat_panel_g,
    ]

    building.add_wall(
        wall_type="front",
        panels=front_panels_ground,
        floor_type="ground",
        invert=False,
    )

    # Задние стены первого этажа
    flat_panel_back_g = create_flat_panel_ground_with_nested_cutouts(
        panel_width, panel_height, color=side_panel_color
    )
    recessed_panel_back_g = create_recessed_panel_ground_with_nested_cutouts(
        panel_width, panel_height, color=center_panel_color
    )
    sloped_panel_back_2_g = create_sloped_panel_ground_with_nested_cutouts(
        panel_width,
        panel_height,
        start_z=0,
        end_z=-recess_depth,
        color=side_panel_color,
    )
    sloped_panel_back_5_g = create_sloped_panel_ground_with_nested_cutouts(
        panel_width,
        panel_height,
        start_z=-recess_depth,
        end_z=0,
        color=side_panel_color,
    )

    back_panels_ground = [
        flat_panel_back_g,
        sloped_panel_back_2_g,
        recessed_panel_back_g,
        recessed_panel_back_g,
        sloped_panel_back_5_g,
        flat_panel_back_g,
    ]

    building.add_wall(
        wall_type="back",
        panels=back_panels_ground,
        floor_type="ground",
        invert=True,
    )

    # Боковые стены первого этажа с входной группой
    side_panel_ground_normal = create_side_panel_with_nested_cutouts(
        panel_width,
        panel_height,
        depth=panel_width,
        color=side_panel_color,
        with_window=True,
    )

    # Панель с входом (третья панель левой стены)
    entrance_panel = create_entrance_panel_with_nested_cutouts(
        panel_width, panel_height, depth=panel_width, color=side_panel_color
    )

    # Левая стена первого этажа (панели: обычная, обычная, с входом, обычная)
    left_panels_ground = [
        side_panel_ground_normal,
        side_panel_ground_normal,
        entrance_panel,  # Третья панель с входом
        side_panel_ground_normal,
    ]

    building.add_wall(
        wall_type="left",
        panels=left_panels_ground,
        floor_type="ground",
        invert=False,
    )

    # Правая стена первого этажа (все обычные панели с окнами)
    right_panels_ground = [side_panel_ground_normal] * 4

    building.add_wall(
        wall_type="right",
        panels=right_panels_ground,
        floor_type="ground",
        invert=True,
    )

    # =============== ФИНАЛИЗАЦИЯ ===============

    building.finalize()
    return building


# =============== ПРИМЕР ИСПОЛЬЗОВАНИЯ ===============
if __name__ == "__main__":
    # Создаем здание со светло-голубыми центральными панелями
    blue_building = create_stepped_panel_building(
        name="blue_stepped_building_with_nested_frames",
     floor_count=27,
        roof_type="flat",
        roof_height=1.0,
        window_color=Building.get_window_color,
        center_panel_color_type="blue",
    )

    # Сохраняем в файл
    blue_building.save_to_file(
        "buildings/blue_stepped_building_with_nested_frames.json"
    )
    print(
        "✅ Здание с голубыми центральными панелями и рамками как вырезы успешно создано!"
    )

    # Создаем здание с оранжевыми центральными панелями
    orange_building = create_stepped_panel_building(
        name="orange_stepped_building_with_nested_frames",
        description="Здание с оранжевыми центральными панелями и рамками окон как вырезы",
        floor_count=32,
        roof_type="flat",
        roof_height=1.0,
        window_color=Building.get_window_color,
        center_panel_color_type="orange",
    )

    # Сохраняем в файл
    orange_building.save_to_file(
        "buildings/orange_stepped_building_with_nested_frames.json"
    )
    print(
        "✅ Здание с оранжевыми центральными панелями и рамками как вырезы успешно создано!"
    )