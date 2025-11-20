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
    Каждый дом: 7 этажей, 5 панелей в длину (20 м), 2 панели в глубину (8 м), высота панели 2.8 м.
    Основной цвет стен циклически перебирает 5 пастельных оттенков.
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
            width_panels=5,
            depth_panels=2,
            panel_width=4.0,
            panel_depth=4.0,
            floor_height=2.8,
            ground_floor_height=None,
            roof_type="flat",
            roof_height=None,
        )

        # === ШАГ 2: СОЗДАНИЕ ГЕОМЕТРИЧЕСКИХ ПАНЕЛЕЙ ВРУЧНУЮ ===
        # Создаем фасадную панель первого этажа с окном
        front_panel = GeometrySegment()
        margin = 0.15  # Отступ окна от краев панели (15%)
        window_points = [
            (margin, margin),
            (1 - margin, margin),
            (1 - margin, 1 - margin),
            (margin, 1 - margin),
        ]

        # Добавляем рамку с вырезом
        # RectangleWithCutout аргументы:
        #   cutout_points: List[Tuple[float, float]] - координаты выреза в относительных единицах [0..1]
        #   color: str - цвет рамки в формате #RRGGBB
        #   invert: bool - инвертировать нормали (True для задних/боковых стен)
        front_panel.add_child(
            RectangleWithCutout(
                cutout_points=window_points,
                color=color,
                invert=False,  # Нормали "наружу" для фасада
            )
        )

        # Определяем цвет стекла (Building.get_window_color генерирует случайный темный цвет)
        glass_color = Building.get_window_color

        # Добавляем стекло
        # SimplePolygon аргументы:
        #   points: List[Tuple[float, float]] - координаты контура в относительных единицах [0..1]
        #   color: str - цвет полигона в формате #RRGGBB
        #   invert: bool - инвертировать нормали
        #   margin_type: str - тип отступа ("relative" или "absolute")
        #   margin_value: float - значение отступа (относительное или абсолютное)
        front_panel.add_child(
            SimplePolygon(
                points=window_points,
                color=glass_color,
                invert=False,
                margin_type="relative",
                margin_value=margin,
            )
        )

        # Создаем глухую панель для задней стены
        back_panel = Building.create_solid_panel(color=color, invert=True)

        # === ШАГ 3: ДОБАВЛЕНИЕ СТЕН ПЕРВОГО ЭТАЖА ===
        building.add_wall(
            wall_type="front",
            panels=[front_panel] * 5,
            floor_type="ground",
            panels_count=5,
            invert=False,
        )

        building.add_wall(
            wall_type="back",
            panels=[back_panel] * 5,
            floor_type="ground",
            panels_count=5,
            invert=True,
        )

        building.add_wall(
            wall_type="left",
            panels=[front_panel] * 2,
            floor_type="ground",
            panels_count=2,
            invert=False,
        )

        building.complete_walls("ground")

        # === ШАГ 4: ТИПОВЫЕ ЭТАЖИ (этажи 2-7) ===
        if building.floor_count > 1:
            typical_panel = GeometrySegment()
            margin = 0.20  # Отступ для типовых этажей (20%)
            window_points = [
                (margin, margin),
                (1 - margin, margin),
                (1 - margin, 1 - margin),
                (margin, 1 - margin),
            ]

            typical_panel.add_child(
                RectangleWithCutout(
                    cutout_points=window_points, color=color, invert=False
                )
            )

            typical_panel.add_child(
                SimplePolygon(
                    points=window_points,
                    color=Building.get_window_color,
                    invert=False,
                    margin_type="relative",
                    margin_value=margin,
                )
            )

            building.add_wall(
                "front", [typical_panel] * 5, "typical", panels_count=5, invert=False
            )
            building.add_wall(
                "back", [back_panel] * 5, "typical", panels_count=5, invert=True
            )
            building.add_wall(
                "left", [typical_panel] * 2, "typical", panels_count=2, invert=False
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
