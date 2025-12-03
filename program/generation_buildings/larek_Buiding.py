from generation_buildings.createBuilding import *
def generate_building_kiosk_pechat():
    model = Building(
        name="kiosk_pechat",
        description="Круглый ларек с вывеской ПЕЧАТЬ",
        author="UrbanAI",
        version="1.0",
        floor_count=1,
        panels_per_row=3,
        panels_per_row_depth=1,
        panel_width=2.0,
        panel_height=3.5,
        ground_floor_height=4.0,
    )

    # Боковые панели (левая и правая части фасада)
    window_margin = 0.05
    window_points = [
        (window_margin, window_margin),
        (1 - window_margin, window_margin),
        (1 - window_margin, 1 - window_margin),
        (window_margin, 1 - window_margin),
    ]

    glass_panel = GeometrySegment()
    glass_panel.add_child(
        RectangleWithCutout(cutout_points=window_points, color="#303030")
    )
    glass_panel.add_child(SimplePolygon(points=window_points, color="#88B6CC"))

    # Центральная панель с вывеской и витриной
    vitriane_height = 0.7
    vitriane_margin = 0.05
    vitriane_points = [
        (vitriane_margin, vitriane_margin),
        (1 - vitriane_margin, vitriane_margin),
        (1 - vitriane_margin, vitriane_height - vitriane_margin),
        (vitriane_margin, vitriane_height - vitriane_margin),
    ]

    central_vitrine = GeometrySegment()
    central_vitrine.add_child(
        RectangleWithCutout(cutout_points=vitriane_points, color="#303030")
    )
    central_vitrine.add_child(SimplePolygon(points=vitriane_points, color="#88B6CC"))

    sign_panel = SimplePolygon(
        points=[(0, vitriane_height), (1, vitriane_height), (1, 1), (0, 1)],
        color="#008000",
    )

    central_panel = GeometrySegment()
    central_panel.add_child(sign_panel)
    central_panel.add_child(central_vitrine)

    # Сборка фасада
    front_segments = [glass_panel, central_panel, glass_panel]
    left_segments = [glass_panel]

    model.add_wall(model.ground_floor, "front", front_segments)
    model.add_wall(model.ground_floor, "left", left_segments)
    model.auto_complete_walls(model.ground_floor)
    model.ground_floor.fix_normals()

    # Добавляем крышу
    roof_width = model.panels_per_row * model.panel_width
    roof_depth = model.panels_per_row_depth * model.panel_width

    # Вершины крыши (плоская крыша)
    roof_vertices = [
        (0, model.ground_floor_height, 0),  # передний левый угол
        (roof_width, model.ground_floor_height, 0),  # передний правый угол
        (roof_width, model.ground_floor_height, roof_depth),  # задний правый угол
        (0, model.ground_floor_height, roof_depth),  # задний левый угол
    ]

    # Создаем полигон крыши
    roof_polygon = SimplePolygon(
        points=[
            (0, 0),  # левый нижний
            (1, 0),  # правый нижний
            (1, 1),  # правый верхний
            (0, 1),  # левый верхний
        ],
        color="#A4A4A4",
    )

    # Добавляем крышу как отдельный элемент
    model.roof.add_child(roof_polygon)

    # Применяем исправление нормалей к крыше
    model.roof.fix_normals()

    return model

if __name__ == "__main__":
    model = generate_building_kiosk_pechat()
    model.save_to_file("/drive_d/Documents/CG_curs/program/buildings/larek.json")
    print("Здание сохранено в файл ../buildings/larek.json")