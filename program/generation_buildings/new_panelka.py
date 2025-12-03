# generate_houses.py
# Импорты — только необходимые из фреймворка
from createBuilding import (
    Building,
    GeometrySegment,
    RectangleWithCutout,
    SimplePolygon,
    RoofGeometry,
)


def generate_seven_story_house():
    """
    Генерирует 5 3D-моделей семиэтажных панельных домов с разными пастельными цветами.
    Каждый дом: 7 этажей, общая ширина 20 м (5 панелей), глубина 8 м (2 панели), высота этажа 2.8 м.
    Основной цвет стен циклически перебирает 5 пастельных оттенков.
    Все координаты задаются в абсолютных значениях относительно левого нижнего угла панели.
    """
    # Список пастельных цветов для основного цвета фасадов
    pastel_colors = [
        "#FFD1DC",  # Пастельно-розовый
        "#E6E6FA",  # Лаванда
        "#B0E0E6",  # Пастельно-голубой
        "#98FB98",  # Мятный
        "#FFFACD",  # Светло-желтый
    ]

    for i, color in enumerate(pastel_colors):
        # === ШАГ 1: ИНИЦИАЛИЗАЦИЯ ЗДАНИЯ ===
        building = Building(
            name=f"seven_story_house_{i + 1}",
            description="Семиэтажный панельный дом с пастельным цветом фасада",
            floor_count=7,
            width=20.0,  # Общая ширина здания
            depth=8.0,  # Общая глубина здания
            ground_floor_height=2.8,
            typical_floor_height=2.8,
            roof_type="flat",
            roof_height=1.0,  # Высота крыши
        )

        # === ШАГ 2: СОЗДАНИЕ ГЕОМЕТРИЧЕСКИХ ПАНЕЛЕЙ ВРУЧНУЮ ===
        # Размеры одной панели
        panel_width = 4.0  # 20 м / 5 панелей
        panel_height_ground = 2.8  # Высота первого этажа

        # Создаем фасадную панель первого этажа с окном и кондиционером
        front_panel = GeometrySegment()

        # 1. Рамка окна
        window_margin = 0.6  # 60 см отступ от краев
        # Внешний контур рамки (x, y, z) относительно левого нижнего угла панели
        outer_frame = [
            (0, 0, 0),  # Нижний левый угол
            (panel_width, 0, 0),  # Нижний правый угол
            (panel_width, panel_height_ground, 0),  # Верхний правый угол
            (0, panel_height_ground, 0),  # Верхний левый угол
        ]

        # Контур окна (x, y, z) относительно левого нижнего угла панели
        window_frame = [
            (
                window_margin,
                window_margin,
                0.1,
            ),  # Нижний левый угол окна (слегка выступает)
            (panel_width - window_margin, window_margin, 0.1),  # Нижний правый угол
            (
                panel_width - window_margin,
                panel_height_ground - window_margin,
                0.1,
            ),  # Верхний правый угол
            (
                window_margin,
                panel_height_ground - window_margin,
                0.1,
            ),  # Верхний левый угол
        ]

        # Добавляем рамку с вырезом
        front_panel.add_child(
            RectangleWithCutout(
                outer_points=outer_frame,
                cutout_points=window_frame,
                color=color,
            )
        )

        # 2. Стекло (в том же контуре, что и окно)
        front_panel.add_child(
            SimplePolygon(
                points=window_frame,
                color=Building.get_window_color,
            )
        )

        # 3. Кондиционер под окном
        ac_width = 1.2
        ac_height = 0.4
        ac_depth = 0.25  # Глубина кондиционера
        ac_x = panel_width / 2 - ac_width / 2  # По центру панели по X
        ac_y = window_margin - ac_height - 0.15  # Под окном с отступом по Y
        ac_z = 0.3  # Отступ от стены по Z

        # Точки кондиционера (x, y, z) относительно левого нижнего угла панели
        ac_points = [
            (ac_x, ac_y, ac_z),  # Нижний левый
            (ac_x + ac_width, ac_y, ac_z),  # Нижний правый
            (ac_x + ac_width, ac_y + ac_height, ac_z),  # Верхний правый
            (ac_x, ac_y + ac_height, ac_z),  # Верхний левый
        ]

        front_panel.add_child(
            SimplePolygon(
                points=ac_points,
                color="#444444",  # Темно-серый
            )
        )

        # 4. Подоконник (внутри стены для объема)
        sill_width = panel_width - 1.0
        sill_height = 0.1
        sill_depth = 0.15  # Глубина подоконника
        sill_x = (panel_width - sill_width) / 2
        sill_y = window_margin - sill_height
        sill_z = -0.05  # Внутрь стены на 5 см

        sill_points = [
            (sill_x, sill_y, sill_z),
            (sill_x + sill_width, sill_y, sill_z),
            (sill_x + sill_width, sill_y + sill_height, sill_z),
            (sill_x, sill_y + sill_height, sill_z),
        ]

        front_panel.add_child(
            SimplePolygon(
                points=sill_points,
                color="#8B4513",  # Коричневый как дерево
            )
        )

        # Создаем глухую панель для задней стены
        back_panel = GeometrySegment()
        back_panel_points = [
            (0, 0, 0),
            (panel_width, 0, 0),
            (panel_width, panel_height_ground, 0),
            (0, panel_height_ground, 0),
        ]
        back_panel.add_child(
            SimplePolygon(
                points=back_panel_points,
                color=color,
                invert=True,  # Нормали внутрь
            )
        )

        # === ШАГ 3: ДОБАВЛЕНИЕ СТЕН ПЕРВОГО ЭТАЖА ===
        building.add_wall(
            wall_type="front",
            panels=[front_panel] * 5,  # 5 одинаковых панелей
            floor_type="ground",
            invert=False,
        )

        building.add_wall(
            wall_type="back",
            panels=[back_panel] * 5,  # 5 одинаковых глухих панелей
            floor_type="ground",
            invert=True,  # Инвертируем нормали для задней стены
        )

        building.add_wall(
            wall_type="left",
            panels=[front_panel]
            * 2,  # 2 панели для левой стены (глубина 8 м / 2 = 4 м на панель)
            floor_type="ground",
            invert=False,
        )

        building.add_wall(
            wall_type="right",
            panels=[back_panel] * 2,  # 2 панели для правой стены
            floor_type="ground",
            invert=True,
        )

        building.complete_walls("ground")

        # === ШАГ 4: ТИПОВЫЕ ЭТАЖИ (этажи 2-7) ===
        if building.floor_count > 1:
            panel_height_typical = 2.8

            # Создаем панель типового этажа
            typical_panel = GeometrySegment()

            # Окно с другим отступом
            window_margin_typical = 0.7
            # Внешний контур рамки
            outer_frame_typical = [
                (0, 0, 0),
                (panel_width, 0, 0),
                (panel_width, panel_height_typical, 0),
                (0, panel_height_typical, 0),
            ]

            # Контур окна
            window_frame_typical = [
                (window_margin_typical, window_margin_typical, 0.1),
                (panel_width - window_margin_typical, window_margin_typical, 0.1),
                (
                    panel_width - window_margin_typical,
                    panel_height_typical - window_margin_typical,
                    0.1,
                ),
                (
                    window_margin_typical,
                    panel_height_typical - window_margin_typical,
                    0.1,
                ),
            ]

            # Рамка окна
            typical_panel.add_child(
                RectangleWithCutout(
                    outer_points=outer_frame_typical,
                    cutout_points=window_frame_typical,
                    color=color,
                )
            )

            # Стекло
            typical_panel.add_child(
                SimplePolygon(
                    points=window_frame_typical,
                    color=Building.get_window_color,
                )
            )

            # === ДОБАВЛЕНИЕ СТЕН ТИПОВОГО ЭТАЖА ===
            building.add_wall(
                wall_type="front",
                panels=[typical_panel] * 5,
                floor_type="typical",
                invert=False,
            )

            building.add_wall(
                wall_type="back",
                panels=[back_panel] * 5,
                floor_type="typical",
                invert=True,
            )

            building.add_wall(
                wall_type="left",
                panels=[typical_panel] * 2,
                floor_type="typical",
                invert=False,
            )

            building.add_wall(
                wall_type="right",
                panels=[back_panel] * 2,
                floor_type="typical",
                invert=True,
            )

            building.complete_walls("typical")

        # === ШАГ 5: ФИНАЛИЗАЦИЯ И СОХРАНЕНИЕ ===
        building.finalize()

        filename = f"buildings/seven_story_house_{i + 1}.json"
        building.save_to_file(filename)
        print(f"✅ Дом {i + 1}/5 сохранен в {filename}")

    return f"seven_story_house_5.json"


# Запуск генерации при выполнении скрипта
if __name__ == "__main__":
    generate_seven_story_house()
