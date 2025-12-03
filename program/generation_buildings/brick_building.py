from generation_buildings.createBuilding import *
def generate_brick_7_story():
    model = BuildingModel(
        name="brick_7_story",
        description="7-этажное кирпичное здание с классическими окнами",
        author="Architect",
        version="1.0",
        floor_count=7,
        panels_per_row=4,
        panels_per_row_depth=2,
        panel_width=4.0,
        panel_height=3.2,
        ground_floor_height=4.0,
    )

    # Настройка кирпичного фасада и окон
    window_margin = 0.25
    window_points = [
        (window_margin, window_margin),
        (1 - window_margin, window_margin),
        (1 - window_margin, 1 - window_margin),
        (window_margin, 1 - window_margin),
    ]

    # Создаем кирпичную панель с окном
    brick_panel = GeometrySegment()
    brick_panel.add_child(
        RectangleWithCutout(
            cutout_points=window_points,
            color="#B23A48",  # Кирпичный цвет
        )
    )
    brick_panel.add_child(
        SimplePolygon(
            points=window_points,
            color=BuildingModel.get_window_color(),  # Случайный цвет окна
        )
    )

    # Добавляем стены для первого этажа
    model.add_wall(model.ground_floor, "front", [brick_panel] * 4)
    model.add_wall(model.ground_floor, "left", [brick_panel] * 2)
    model.auto_complete_walls(model.ground_floor)

    # Добавляем стены для типовых этажей
    model.add_wall(model.typical_floor, "front", [brick_panel] * 4)
    model.add_wall(model.typical_floor, "left", [brick_panel] * 2)
    model.auto_complete_walls(model.typical_floor)

    # Коррекция нормалей для всех секций
    model.ground_floor.fix_normals()
    model.typical_floor.fix_normals()
    model.roof.fix_normals()

    return model


if __name__ == "__main__":
    model = generate_brick_7_story()
    model.save_to_file("../buildings/brick_7_story.json")
    print("Здание сохранено в файл ../buildings/brick_7_story.json")