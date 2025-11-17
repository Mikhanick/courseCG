import unittest
import numpy as np
import json
from generation_buildings.createBuuilding import (
    BuildingSection,
    GeometrySegment,
    BuildingModel,
    create_panel_segment,
    generate_soviet_apartment,
)


class TestBuildingSection(unittest.TestCase):
    def setUp(self):
        self.section = BuildingSection()

    def test_add_vertex_updates_dimensions(self):
        """Проверка, что добавление вершин обновляет размеры секции"""
        self.section.add_vertex(1.0, 2.0, 3.0)
        self.assertEqual(self.section.dimensions, [1.0, 2.0, 3.0])

        self.section.add_vertex(4.0, 1.0, 2.0)
        self.assertEqual(self.section.dimensions, [4.0, 2.0, 3.0])

    def test_simple_polygon_triangulation(self):
        """Проверка триангуляции простого прямоугольника"""
        # Создаем прямоугольник 2x2
        v0 = self.section.add_vertex(0, 0, 0)
        v1 = self.section.add_vertex(2, 0, 0)
        v2 = self.section.add_vertex(2, 2, 0)
        v3 = self.section.add_vertex(0, 2, 0)

        self.section.add_polygon([v0, v1, v2, v3], "#FF0000")

        # Проверяем, что создано 2 треугольника
        self.assertEqual(len(self.section.faces), 2)
        # Проверяем порядок вершин для первого треугольника
        self.assertEqual(self.section.faces[0][:3], [v0, v1, v2])
        # Проверяем порядок вершин для второго треугольника
        self.assertEqual(self.section.faces[1][:3], [v0, v2, v3])

    def test_geometry_segment_with_cutout(self):
        """Проверка добавления сегмента с внутренним вырезом"""
        # Создаем сегмент с внутренним вырезом
        segment = GeometrySegment(
            points=[(0, 0), (2, 0), (2, 2), (0, 2)],
            color="#8B7D6B",
            cutouts=[[(0.5, 0.5), (1.5, 0.5), (1.5, 1.5), (0.5, 1.5)]],
        )

        # Добавляем сегмент
        self.section.add_geometry_segment(segment, (0, 0, 0), "front", 1.0)

        # Проверяем количество вершин
        self.assertEqual(len(self.section.vertices), 8)  # 4 внешних + 4 внутренних

        # Проверяем, что есть хотя бы несколько граней
        self.assertTrue(len(self.section.faces) > 0)

    def test_connection_points_calculation(self):
        """Проверка вычисления точек соединения между внешним и внутренним контурами"""
        outer_points = [(0, 0), (2, 0), (2, 2), (0, 2)]
        cutout_points = [(0.5, 0.5), (1.5, 0.5), (1.5, 1.5), (0.5, 1.5)]

        # Создаем карты вершин
        vertex_map = {i: i for i in range(4)}
        cutout_map = {i: i + 4 for i in range(4)}

        connections = self.section._find_connection_points(
            outer_points, cutout_points, "front", vertex_map, cutout_map
        )

        # Проверяем количество соединений
        self.assertGreaterEqual(len(connections), 2)  # Должно быть минимум 2 соединения

    def test_cutout_creation(self):
        """Проверка корректного создания внутренних вырезов"""
        # Создаем сегмент с внутренним вырезом
        segment = GeometrySegment(
            points=[(0, 0), (3, 0), (3, 3), (0, 3)],
            color="#8B7D6B",
            cutouts=[[(1, 1), (2, 1), (2, 2), (1, 2)]],
        )

        # Добавляем сегмент
        self.section.add_geometry_segment(segment, (0, 0, 0), "front", 1.0)

        # Проверяем количество вершин (4 внешних + 4 внутренних)
        self.assertEqual(len(self.section.vertices), 8)

        # Проверяем координаты внутренних вершин (должны быть внутри внешнего контура)
        inner_vertices = self.section.vertices[4:]
        for vertex in inner_vertices:
            self.assertGreater(vertex[0], 0)
            self.assertLess(vertex[0], 3)
            self.assertGreater(vertex[1], 0)
            self.assertLess(vertex[1], 3)


class TestBuildingModel(unittest.TestCase):
    def test_automatic_dimensions_calculation(self):
        """Проверка автоматического расчета размеров здания"""
        model = BuildingModel(
            name="test_building",
            description="Test building",
            author="Test",
            version="1.0",
            floor_count=3,
            panels_per_row=4,
            panel_width=3.0,
            panel_height=2.5,
        )

        # Проверяем рассчитанные размеры с учетом плавающей точки
        self.assertEqual(model.width, 12.0)  # 4 панели × 3.0 м
        self.assertAlmostEqual(model.depth, 7.2, places=6)  # 12.0 × 0.6
        self.assertEqual(model.typical_floor_height, 2.5)

    def test_wall_creation(self):
        """Проверка создания стен"""
        model = BuildingModel(
            name="test_building",
            description="Test building",
            author="Test",
            version="1.0",
            floor_count=1,
            panels_per_row=2,
            panel_width=3.0,
            panel_height=2.5,
        )

        # Создаем стандартную панель
        panel = GeometrySegment(
            points=[(0, 0), (1, 0), (1, 1), (0, 1)], color="#8B7D6B"
        )

        # Добавляем переднюю стену
        model.add_wall(model.ground_floor, "front", [panel] * 2, panels_count=2)

        # Проверяем количество вершин
        self.assertTrue(len(model.ground_floor.vertices) > 0)
        # Проверяем, что вершины имеют правильные координаты
        for vertex in model.ground_floor.vertices:
            self.assertGreaterEqual(vertex[0], 0)
            self.assertGreaterEqual(vertex[1], 0)
            self.assertEqual(vertex[2], 0)  # Передняя стена должна быть на Z=0

    def test_add_wall_with_different_orientations(self):
        """Проверка добавления стен с разными ориентациями"""
        model = BuildingModel(
            name="test_building",
            description="Test building",
            author="Test",
            version="1.0",
            floor_count=1,
            panels_per_row=1,
            panel_width=3.0,
            panel_height=2.5,
        )

        panel = GeometrySegment(
            points=[(0, 0), (1, 0), (1, 1), (0, 1)], color="#8B7D6B"
        )

        # Добавляем стены всех ориентаций
        model.add_wall(model.ground_floor, "front", [panel], panels_count=1)
        model.add_wall(model.ground_floor, "back", [panel], panels_count=1)
        model.add_wall(model.ground_floor, "left", [panel], panels_count=1)
        model.add_wall(model.ground_floor, "right", [panel], panels_count=1)

        # Проверяем, что все стены добавлены
        self.assertTrue(len(model.ground_floor.vertices) > 0)
        self.assertTrue(len(model.ground_floor.faces) > 0)

        # Проверяем координаты для разных ориентаций
        front_vertices = [v for v in model.ground_floor.vertices if v[2] == 0]
        back_vertices = [v for v in model.ground_floor.vertices if v[2] == model.depth]
        left_vertices = [v for v in model.ground_floor.vertices if v[0] == 0]
        right_vertices = [v for v in model.ground_floor.vertices if v[0] == model.width]

        self.assertTrue(len(front_vertices) > 0)
        self.assertTrue(len(back_vertices) > 0)
        self.assertTrue(len(left_vertices) > 0)
        self.assertTrue(len(right_vertices) > 0)


class TestWallNormals(unittest.TestCase):
    """Тесты для проверки правильности нормалей стен"""

    def setUp(self):
        self.model = BuildingModel(
            name="test_building",
            description="Test building",
            author="Test",
            version="1.0",
            floor_count=1,
            panels_per_row=1,
            panel_width=3.0,
            panel_height=2.5,
        )

        # Создаем простую панель
        self.panel = GeometrySegment(
            points=[(0, 0), (1, 0), (1, 1), (0, 1)], color="#8B7D6B"
        )

    def _calculate_normal(self, p0, p1, p2):
        """Вычисляет нормаль для треугольника"""
        edge1 = np.array(p1) - np.array(p0)
        edge2 = np.array(p2) - np.array(p0)
        normal = np.cross(edge1, edge2)
        return normal / np.linalg.norm(normal) if np.linalg.norm(normal) > 0 else normal

    def test_front_wall_normals(self):
        """Проверка нормалей передней стены (должны указывать в отрицательном Z)"""
        self.model.add_wall(
            self.model.ground_floor, "front", [self.panel], panels_count=1
        )

        # Берем первые три вершины для проверки нормали
        face = self.model.ground_floor.faces[0]
        v0, v1, v2 = face[0:3]
        p0 = self.model.ground_floor.vertices[v0]
        p1 = self.model.ground_floor.vertices[v1]
        p2 = self.model.ground_floor.vertices[v2]

        normal = self._calculate_normal(p0, p1, p2)
        # Нормаль должна указывать в отрицательном Z (от здания наружу)
        self.assertLess(
            normal[2],
            0,
            f"Front wall normal should point in negative Z direction: {normal}",
        )

    def test_back_wall_normals(self):
        """Проверка нормалей задней стены (должны указывать в положительном Z)"""
        self.model.add_wall(
            self.model.ground_floor, "back", [self.panel], panels_count=1
        )

        # Берем первые три вершины для проверки нормали
        face = self.model.ground_floor.faces[0]
        v0, v1, v2 = face[0:3]
        p0 = self.model.ground_floor.vertices[v0]
        p1 = self.model.ground_floor.vertices[v1]
        p2 = self.model.ground_floor.vertices[v2]

        normal = self._calculate_normal(p0, p1, p2)
        # Нормаль должна указывать в положительном Z (от здания наружу)
        self.assertGreater(
            normal[2],
            0,
            f"Back wall normal should point in positive Z direction: {normal}",
        )

    def test_left_wall_normals(self):
        """Проверка нормалей левой стены (должны указывать в отрицательном X)"""
        self.model.add_wall(
            self.model.ground_floor, "left", [self.panel], panels_count=1
        )

        # Берем первые три вершины для проверки нормали
        face = self.model.ground_floor.faces[0]
        v0, v1, v2 = face[0:3]
        p0 = self.model.ground_floor.vertices[v0]
        p1 = self.model.ground_floor.vertices[v1]
        p2 = self.model.ground_floor.vertices[v2]

        normal = self._calculate_normal(p0, p1, p2)
        # Нормаль должна указывать в отрицательном X (от здания наружу)
        self.assertLess(
            normal[0],
            0,
            f"Left wall normal should point in negative X direction: {normal}",
        )

    def test_right_wall_normals(self):
        """Проверка нормалей правой стены (должны указывать в положительном X)"""
        self.model.add_wall(
            self.model.ground_floor, "right", [self.panel], panels_count=1
        )

        # Берем первые три вершины для проверки нормали
        face = self.model.ground_floor.faces[0]
        v0, v1, v2 = face[0:3]
        p0 = self.model.ground_floor.vertices[v0]
        p1 = self.model.ground_floor.vertices[v1]
        p2 = self.model.ground_floor.vertices[v2]

        normal = self._calculate_normal(p0, p1, p2)
        # Нормаль должна указывать в положительном X (от здания наружу)
        self.assertGreater(
            normal[0],
            0,
            f"Right wall normal should point in positive X direction: {normal}",
        )


class TestAutoCompleteWalls(unittest.TestCase):
    """Тесты для автоматического создания недостающих стен"""

    def setUp(self):
        self.model = BuildingModel(
            name="test_building",
            description="Test building",
            author="Test",
            version="1.0",
            floor_count=1,
            panels_per_row=2,
            panel_width=3.0,
            panel_height=2.5,
        )

        # Создаем стандартную панель
        self.panel = GeometrySegment(
            points=[(0, 0), (1, 0), (1, 1), (0, 1)], color="#8B7D6B"
        )

    def test_auto_complete_missing_walls(self):
        """Проверка автоматического создания недостающих стен"""
        # Добавляем только переднюю и левую стены
        self.model.add_wall(
            self.model.ground_floor, "front", [self.panel] * 2, panels_count=2
        )
        self.model.add_wall(
            self.model.ground_floor, "left", [self.panel] * 2, panels_count=2
        )

        # Сохраняем количество вершин и граней до автозавершения
        vertices_before = len(self.model.ground_floor.vertices)
        faces_before = len(self.model.ground_floor.faces)

        # Автоматически создаем недостающие стены
        self.model.auto_complete_walls(self.model.ground_floor)

        # Проверяем, что добавлены новые вершины и грани
        self.assertGreater(len(self.model.ground_floor.vertices), vertices_before)
        self.assertGreater(len(self.model.ground_floor.faces), faces_before)

        # Проверяем наличие всех четырех стен
        z_values = [v[2] for v in self.model.ground_floor.vertices]
        x_values = [v[0] for v in self.model.ground_floor.vertices]

        # Должны быть вершины с Z=0 (передняя стена) и Z=depth (задняя стена)
        self.assertIn(0, z_values)
        self.assertIn(self.model.depth, z_values)

        # Должны быть вершины с X=0 (левая стена) и X=width (правая стена)
        self.assertIn(0, x_values)
        self.assertIn(self.model.width, x_values)

    def test_wall_completion_with_cutouts(self):
        """Проверка автоматического создания стен с внутренними вырезами"""
        # Создаем панель с вырезом
        panel_with_cutout = GeometrySegment(
            points=[(0, 0), (1, 0), (1, 1), (0, 1)],
            color="#8B7D6B",
            cutouts=[[(0.3, 0.3), (0.7, 0.3), (0.7, 0.7), (0.3, 0.7)]],
        )

        # Добавляем только переднюю стену с вырезами
        self.model.add_wall(
            self.model.ground_floor, "front", [panel_with_cutout] * 2, panels_count=2
        )

        # Автоматически создаем недостающие стены
        self.model.auto_complete_walls(self.model.ground_floor)

        # Проверяем, что в задней стене тоже есть вырезы (через анализ геометрии)
        # В реальной реализации здесь будет более сложная проверка
        self.assertTrue(len(self.model.ground_floor.faces) > 0)


class TestFullBuildingGeneration(unittest.TestCase):
    def test_soviet_apartment_generation(self):
        """Проверка генерации полного здания"""
        json_output = generate_soviet_apartment()

        # Проверяем, что результат является валидным JSON
        try:
            model_data = json.loads(json_output)
        except json.JSONDecodeError as e:
            self.fail(f"Invalid JSON output: {e}")

        # Проверяем структуру JSON
        required_sections = [
            "metadata",
            "dimensions",
            "floors",
            "ground_floor",
            "typical_floor",
            "roof",
        ]
        for section in required_sections:
            self.assertIn(section, model_data)

        # Проверяем ground_floor
        self.assertIn("vertices", model_data["ground_floor"])
        self.assertIn("faces", model_data["ground_floor"])
        self.assertGreater(len(model_data["ground_floor"]["vertices"]), 0)
        self.assertGreater(len(model_data["ground_floor"]["faces"]), 0)

        # Проверяем, что все координаты неотрицательные
        for section_name in ["ground_floor", "typical_floor", "roof"]:
            for vertex in model_data[section_name]["vertices"]:
                for coord in vertex:
                    self.assertGreaterEqual(
                        coord,
                        0,
                        f"Negative coordinate found in {section_name}: {vertex}",
                    )

            # Проверяем, что все грани имеют ровно 3 вершины + цвет
            for face in model_data[section_name]["faces"]:
                self.assertEqual(
                    len(face), 4, f"Invalid face format in {section_name}: {face}"
                )
                self.assertIsInstance(
                    face[3], str, f"Invalid color format in {section_name}: {face[3]}"
                )
                # Проверяем, что индексы вершин валидны
                for idx in face[:3]:
                    self.assertLess(
                        idx,
                        len(model_data[section_name]["vertices"]),
                        f"Invalid vertex index {idx} in {section_name}",
                    )


def run_tests():
    """Запуск всех тестов с использованием современного подхода"""
    # Создаем загрузчик тестов
    loader = unittest.TestLoader()

    # Загружаем тестовые классы
    suite = unittest.TestSuite()
    suite.addTests(loader.loadTestsFromTestCase(TestBuildingSection))
    suite.addTests(loader.loadTestsFromTestCase(TestBuildingModel))
    suite.addTests(loader.loadTestsFromTestCase(TestWallNormals))
    suite.addTests(loader.loadTestsFromTestCase(TestAutoCompleteWalls))
    suite.addTests(loader.loadTestsFromTestCase(TestFullBuildingGeneration))

    # Создаем runner для вывода результатов
    runner = unittest.TextTestRunner(verbosity=2)

    # Запускаем тесты
    result = runner.run(suite)

    return result.wasSuccessful()


if __name__ == "__main__":
    # Сначала запускаем тесты
    print("Запуск тестов фреймворка...")
    success = run_tests()

    if success:
        print("\nВсе тесты пройдены успешно! Генерация здания:")
        # Если тесты прошли, генерируем здание
        print(generate_soviet_apartment())
    else:
        print(
            "\nНекоторые тесты не пройдены. Исправьте ошибки перед генерацией здания."
        )
