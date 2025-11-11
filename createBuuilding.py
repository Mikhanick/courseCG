# createBuilding.py
import json
import numpy as np
from typing import List, Tuple, Dict, Any, Optional, Union
from dataclasses import dataclass, field
from abc import ABC, abstractmethod


class GeometryComponent(ABC):
    """Базовый класс для геометрических компонентов (паттерн Компоновщик)"""

    @abstractmethod
    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        scale: float,
        invert_normals: bool,
    ):
        pass


@dataclass
class SimplePolygon(GeometryComponent):
    """
    Простой полигон без вырезов.
    Все вершины перечислены по порядку (против часовой стрелки).
    """

    points: List[
        Tuple[float, float]
    ]  # (x, y) в локальной системе координат [0..1] x [0..1]
    color: str

    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        scale: float,
        invert_normals: bool,
    ):
        x0, y0, z0 = position
        indices = []
        for px, py in self.points:
            # Масштабируем относительные координаты до размеров панели
            px_s = px * scale
            py_s = py * scale
            if orientation == "front":
                wx, wy, wz = x0 + px_s, y0 + py_s, z0
            elif orientation == "back":
                wx, wy, wz = x0 + (scale - px_s), y0 + py_s, z0
            elif orientation == "left":
                wx, wy, wz = x0, y0 + py_s, z0 + (scale - px_s)
            elif orientation == "right":
                wx, wy, wz = x0, y0 + py_s, z0 + px_s
            else:
                raise ValueError(f"Invalid orientation: {orientation}")
            idx = section.add_vertex(wx, wy, wz)
            indices.append(idx)
        section.add_polygon(indices, self.color, invert=invert_normals)


@dataclass
class RectangleWithCutout(GeometryComponent):
    """
    Полигон с вырезом произвольной формы.
    Внешняя граница задается вершинами в порядке их соединения.
    Вырез задается вершинами в порядке их соединения.
    Все координаты задаются в относительных единицах [0..1] x [0..1].
    """

    cutout_points: List[
        Tuple[float, float]
    ]  # Вершины выреза в относительных координатах [0..1] x [0..1]
    color: str
    # Внешние точки по умолчанию - прямоугольник 1x1
    outer_points: List[Tuple[float, float]] = field(
        default_factory=lambda: [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)]
    )

    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        scale: float,
        invert_normals: bool,
    ):
        x0, y0, z0 = position

        # Преобразуем внешние вершины
        outer_indices = []
        for px, py in self.outer_points:
            px_s = px * scale
            py_s = py * scale
            if orientation == "front":
                wx, wy, wz = x0 + px_s, y0 + py_s, z0
            elif orientation == "back":
                wx, wy, wz = x0 + (scale - px_s), y0 + py_s, z0
            elif orientation == "left":
                wx, wy, wz = x0, y0 + py_s, z0 + (scale - px_s)
            elif orientation == "right":
                wx, wy, wz = x0, y0 + py_s, z0 + px_s
            else:
                raise ValueError(f"Invalid orientation: {orientation}")
            idx = section.add_vertex(wx, wy, wz)
            outer_indices.append(idx)

        # Преобразуем вершины выреза
        cutout_indices = []
        for px, py in self.cutout_points:
            px_s = px * scale
            py_s = py * scale
            if orientation == "front":
                wx, wy, wz = x0 + px_s, y0 + py_s, z0
            elif orientation == "back":
                wx, wy, wz = x0 + (scale - px_s), y0 + py_s, z0
            elif orientation == "left":
                wx, wy, wz = x0, y0 + py_s, z0 + (scale - px_s)
            elif orientation == "right":
                wx, wy, wz = x0, y0 + py_s, z0 + px_s
            else:
                raise ValueError(f"Invalid orientation: {orientation}")
            idx = section.add_vertex(wx, wy, wz)
            cutout_indices.append(idx)

        # Триангуляция с вырезом
        # Создаем полигоны, соединяющие внешние и внутренние вершины
        # Предполагаем, что оба контура имеют одинаковое количество вершин
        if len(outer_indices) != len(cutout_indices):
            raise ValueError(
                "Outer and cutout contours must have the same number of vertices for simple triangulation"
            )

        frame_polygons = []
        n = len(outer_indices)
        for i in range(n):
            next_i = (i + 1) % n
            # Создаем четырехугольник между внешним и внутренним контурами
            frame_polygons.append(
                [
                    outer_indices[i],
                    outer_indices[next_i],
                    cutout_indices[next_i],
                    cutout_indices[i],
                ]
            )

        # Добавляем полигоны рамки
        for polygon_indices in frame_polygons:
            section.add_polygon(polygon_indices, self.color, invert=invert_normals)

    def get_cutout_points(self) -> List[Tuple[float, float]]:
        """Возвращает точки выреза для создания окна того же размера"""
        return self.cutout_points


@dataclass
class CompositeGeometry(GeometryComponent):
    """Композитный геометрический объект — контейнер для других компонентов"""

    children: List[GeometryComponent] = field(default_factory=list)

    def add_child(self, child: GeometryComponent):
        self.children.append(child)

    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        scale: float,
        invert_normals: bool,
    ):
        for child in self.children:
            child.add_to_section(section, position, orientation, scale, invert_normals)


@dataclass
class GeometrySegment(CompositeGeometry):
    """
    Сегмент геометрии — композитная панель.
    Состоит из рамки с вырезом и окна на месте выреза.
    """

    pass


class BuildingSection:
    """Секция здания с автоматическим вычислением размеров и триангуляцией"""

    def __init__(self):
        self.vertices: List[List[float]] = []
        self.faces: List[List] = []  # [v0, v1, v2, color]
        self.dimensions = [0.0, 0.0, 0.0]  # [width, height, depth]
        self.walls_created = {
            "front": False,
            "back": False,
            "left": False,
            "right": False,
        }
        self.center = np.array([0.0, 0.0, 0.0])
        self.auto_fix_normals = True

    def set_center(self, center: Tuple[float, float, float]):
        self.center = np.array(center)

    def calculate_center(self):
        if not self.vertices:
            self.center = np.array([0.0, 0.0, 0.0])
            return
        vertices_array = np.array(self.vertices)
        self.center = np.mean(vertices_array, axis=0)

    def add_vertex(self, x: float, y: float, z: float) -> int:
        self.vertices.append([x, y, z])
        self.dimensions[0] = max(self.dimensions[0], x)
        self.dimensions[1] = max(self.dimensions[1], y)
        self.dimensions[2] = max(self.dimensions[2], z)
        return len(self.vertices) - 1

    def _calculate_face_normal(
        self, v0_idx: int, v1_idx: int, v2_idx: int
    ) -> np.ndarray:
        v0 = np.array(self.vertices[v0_idx])
        v1 = np.array(self.vertices[v1_idx])
        v2 = np.array(self.vertices[v2_idx])
        edge1 = v1 - v0
        edge2 = v2 - v0
        normal = np.cross(edge1, -edge2)
        norm = np.linalg.norm(normal)
        if norm > 1e-8:
            normal = normal / norm
        return normal

    def _calculate_face_center(
        self, v0_idx: int, v1_idx: int, v2_idx: int
    ) -> np.ndarray:
        v0 = np.array(self.vertices[v0_idx])
        v1 = np.array(self.vertices[v1_idx])
        v2 = np.array(self.vertices[v2_idx])
        return (v0 + v1 + v2) / 3.0

    def _should_invert_normal(
        self, normal: np.ndarray, face_center: np.ndarray
    ) -> bool:
        to_face_vector = face_center - self.center
        if np.linalg.norm(to_face_vector) < 1e-8:
            return False
        to_face_vector = to_face_vector / np.linalg.norm(to_face_vector)
        dot_product = np.dot(normal, to_face_vector)
        return dot_product < 0

    def add_face(self, v0: int, v1: int, v2: int, color: str, invert: bool = False):
        if invert:
            v1, v2 = v2, v1
        face = [v0, v1, v2, color]
        self.faces.append(face)

        if self.auto_fix_normals:
            normal = self._calculate_face_normal(v0, v1, v2)
            face_center = self._calculate_face_center(v0, v1, v2)
            if self._should_invert_normal(normal, face_center):
                self.faces[-1] = [v0, v2, v1, color]

    def add_polygon(
        self,
        vertex_indices: List[int],
        color: str,
        orientation: Optional[str] = None,
        invert: bool = False,
    ):
        if len(vertex_indices) < 3:
            return
        if len(vertex_indices) == 3:
            self.add_face(
                vertex_indices[0], vertex_indices[1], vertex_indices[2], color, invert
            )
            return
        if len(vertex_indices) == 4:
            self.add_face(
                vertex_indices[0], vertex_indices[1], vertex_indices[2], color, invert
            )
            self.add_face(
                vertex_indices[0], vertex_indices[2], vertex_indices[3], color, invert
            )
            return
        # Общая триангуляция "вентилятором"
        for i in range(1, len(vertex_indices) - 1):
            self.add_face(
                vertex_indices[0],
                vertex_indices[i],
                vertex_indices[i + 1],
                color,
                invert,
            )

    def add_geometry_segment(
        self,
        segment: GeometryComponent,
        position: Tuple[float, float, float],
        orientation: str = "front",
        scale: float = 1.0,
        invert_normals: bool = False,
    ):
        segment.add_to_section(self, position, orientation, scale, invert_normals)
        self.walls_created[orientation] = True

    def copy_front_wall_to_back(self, width: float, depth: float, invert: bool = False):
        front_vertices = []
        for i, v in enumerate(self.vertices):
            if abs(v[2] - 0) < 0.001:
                front_vertices.append((i, v))
        if not front_vertices:
            return
        vertex_map = {}
        for orig_idx, v in front_vertices:
            new_idx = self.add_vertex(v[0], v[1], depth)
            vertex_map[orig_idx] = new_idx
        for face in self.faces:
            v0, v1, v2, color = face
            if (
                v0 in vertex_map
                and v1 in vertex_map
                and v2 in vertex_map
                and abs(self.vertices[v0][2]) < 0.001
                and abs(self.vertices[v1][2]) < 0.001
                and abs(self.vertices[v2][2]) < 0.001
            ):
                self.add_face(
                    vertex_map[v0], vertex_map[v1], vertex_map[v2], color, invert
                )
        self.walls_created["back"] = True

    def copy_left_wall_to_right(self, width: float, depth: float, invert: bool = False):
        left_vertices = []
        for i, v in enumerate(self.vertices):
            if abs(v[0] - 0) < 0.001:
                left_vertices.append((i, v))
        if not left_vertices:
            return
        vertex_map = {}
        for orig_idx, v in left_vertices:
            new_idx = self.add_vertex(width, v[1], v[2])
            vertex_map[orig_idx] = new_idx
        for face in self.faces:
            v0, v1, v2, color = face
            if (
                v0 in vertex_map
                and v1 in vertex_map
                and v2 in vertex_map
                and abs(self.vertices[v0][0]) < 0.001
                and abs(self.vertices[v1][0]) < 0.001
                and abs(self.vertices[v2][0]) < 0.001
            ):
                self.add_face(
                    vertex_map[v0], vertex_map[v1], vertex_map[v2], color, invert
                )
        self.walls_created["right"] = True

    def fix_normals(self):
        self.calculate_center()
        fixed = []
        for face in self.faces:
            v0, v1, v2, color = face
            normal = self._calculate_face_normal(v0, v1, v2)
            center = self._calculate_face_center(v0, v1, v2)
            if self._should_invert_normal(normal, center):
                fixed.append([v0, v2, v1, color])
            else:
                fixed.append(face)
        self.faces = fixed

    def disable_auto_normal_fix(self):
        self.auto_fix_normals = False

    def enable_auto_normal_fix(self):
        self.auto_fix_normals = True


class BuildingModel:
    def __init__(
        self,
        name: str,
        description: str,
        author: str,
        version: str,
        floor_count: int,
        fixed_scale: bool = True,
        texture_scale: float = 1.0,
        panels_per_row: int = 5,
        panels_per_row_depth: int = 3,
        panel_width: float = 5.0,
        panel_height: float = 3.5,
        ground_floor_height: float = 5.0,
    ):
        self.metadata = {
            "name": name,
            "description": description,
            "author": author,
            "version": version,
        }
        self.dimensions = {
            "min_width": 0.0,
            "max_width": 0.0,
            "min_depth": 0.0,
            "max_depth": 0.0,
            "fixed_scale": fixed_scale,
        }
        self.floors = {"count": floor_count, "texture_scale": texture_scale}
        self.ground_floor = BuildingSection()
        self.typical_floor = BuildingSection()
        self.roof = BuildingSection()

        self.panels_per_row = panels_per_row
        self.panel_width = panel_width
        self.panel_height = panel_height
        self.ground_floor_height = ground_floor_height
        self.typical_floor_height = panel_height
        self.roof_height = panel_height * 0.43

        self.width = panels_per_row * panel_width
        self.depth = panels_per_row_depth * panel_width

        self.dimensions["min_width"] = self.width * 0.95
        self.dimensions["max_width"] = self.width * 1.05
        self.dimensions["min_depth"] = self.depth * 0.95
        self.dimensions["max_depth"] = self.depth * 1.05

        self._set_floor_centers()

    def _set_floor_centers(self):
        gf_center = (self.width / 2, self.ground_floor_height / 2, self.depth / 2)
        self.ground_floor.set_center(gf_center)

        tf_center = (
            self.width / 2,
            self.ground_floor_height + self.typical_floor_height / 2,
            self.depth / 2,
        )
        self.typical_floor.set_center(tf_center)

        roof_center = (
            self.width / 2,
            self.ground_floor_height + self.typical_floor_height + self.roof_height / 2,
            self.depth / 2,
        )
        self.roof.set_center(roof_center)

    def add_wall(
        self,
        section: BuildingSection,
        wall_type: str,
        segments: List[GeometryComponent],
        panels_count: Optional[int] = None,
        invert_normals: bool = False,
    ):
        if panels_count is None:
            panels_count = len(segments)

        total_width = self.width if wall_type in ["front", "back"] else self.depth
        panel_width = total_width / panels_count

        for i in range(panels_count):
            segment = segments[i % len(segments)]
            if wall_type == "front":
                pos = (i * panel_width, 0, 0)
                section.add_geometry_segment(
                    segment, pos, "front", panel_width, invert_normals
                )
            elif wall_type == "back":
                pos = (i * panel_width, 0, self.depth)
                section.add_geometry_segment(
                    segment, pos, "back", panel_width, invert_normals
                )
            elif wall_type == "left":
                pos = (0, 0, i * panel_width)
                section.add_geometry_segment(
                    segment, pos, "left", panel_width, invert_normals
                )
            elif wall_type == "right":
                pos = (self.width, 0, i * panel_width)
                section.add_geometry_segment(
                    segment, pos, "right", panel_width, invert_normals
                )

    def auto_complete_walls(
        self,
        section: BuildingSection,
        invert_back: bool = False,
        invert_right: bool = False,
    ):
        if section.walls_created["front"] and not section.walls_created["back"]:
            section.copy_front_wall_to_back(self.width, self.depth, invert=invert_back)
        if section.walls_created["left"] and not section.walls_created["right"]:
            section.copy_left_wall_to_right(self.width, self.depth, invert=invert_right)

    def to_json(self) -> str:
        sections = [self.ground_floor, self.typical_floor, self.roof]
        max_width = max(sec.dimensions[0] for sec in sections)
        max_height = max(sec.dimensions[1] for sec in sections)
        max_depth = max(sec.dimensions[2] for sec in sections)

        self.dimensions["min_width"] = max_width * 0.95
        self.dimensions["max_width"] = max_width * 1.05
        self.dimensions["min_depth"] = max_depth * 0.95
        self.dimensions["max_depth"] = max_depth * 1.05

        model = {
            "metadata": self.metadata,
            "dimensions": self.dimensions,
            "floors": self.floors,
            "ground_floor": {
                "vertices": self.ground_floor.vertices,
                "faces": self.ground_floor.faces,
            },
            "typical_floor": {
                "vertices": self.typical_floor.vertices,
                "faces": self.typical_floor.faces,
            },
            "roof": {
                "vertices": self.roof.vertices,
                "faces": self.roof.faces,
            },
        }
        return json.dumps(model, indent=2)


def generate_soviet_apartment() -> str:
    model = BuildingModel(
        name="apartment_soviet_automatic",
        description="Советский панельный дом с композитными панелями и вырезами",
        author="UrbanSim3D_v5",
        version="3.2",
        floor_count=9,
        panels_per_row=10,
        panels_per_row_depth=4,
        panel_width=5.0,
        panel_height=3.5,
        ground_floor_height=5.0,
    )

    # === СОЗДАНИЕ СТАНДАРТНОЙ ПАНЕЛИ С ОКНОМ ===
    # Определяем точки выреза (окна) в относительных координатах [0..1]
    window_margin = 0.23
    standard_window_points = [
        (window_margin, window_margin),  # нижний левый
        (1 - window_margin, window_margin),  # нижний правый
        (1 - window_margin, 1 - window_margin),  # верхний правый
        (window_margin, 1 - window_margin),  # верхний левый
    ]

    standard_panel = GeometrySegment()
    # Добавляем рамку с вырезом (используем внешние точки по умолчанию - прямоугольник)
    standard_panel.add_child(
        RectangleWithCutout(
            cutout_points=standard_window_points,
            color="#A4A4A4",  # цвет стены
        )
    )
    # Добавляем окно на место выреза
    standard_panel.add_child(
        SimplePolygon(
            points=standard_window_points,
            color="#254843",  # цвет окна
        )
    )

    # === СОЗДАНИЕ ВХОДНОЙ ПАНЕЛИ (БЕЗ ОКНА) ===
    entrance_panel = GeometrySegment()
    entrance_panel.add_child(
        SimplePolygon(
            points=[(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)],
            color="#A4A4A4",  # сплошная стена
        )
    )

    # === СОЗДАНИЕ БОКОВЫХ ПАНЕЛЕЙ ===
    side_window_points = [
        (window_margin, window_margin),
        (1 - window_margin, window_margin),
        (1 - window_margin, 1 - window_margin),
        (window_margin, 1 - window_margin),
    ]

    left_panels = []
    for _ in range(4):
        panel = GeometrySegment()
        panel.add_child(
            RectangleWithCutout(
                cutout_points=side_window_points,
                color="#A4A4A4",  # темнее для боковых стен
            )
        )
        panel.add_child(SimplePolygon(points=side_window_points, color="#254843"))
        left_panels.append(panel)

    # === ПЕРВЫЙ ЭТАЖ ===
    gf = model.ground_floor
    front_panels = [standard_panel] * 10
    front_panels[2] = entrance_panel  # центральная панель - вход
    model.add_wall(gf, "front", front_panels)
    model.add_wall(gf, "left", left_panels)
    model.auto_complete_walls(gf)

    # === ТИПОВЫЕ ЭТАЖИ ===
    tf = model.typical_floor
    model.add_wall(tf, "front", [standard_panel] * 10)
    model.add_wall(tf, "left", left_panels)
    model.auto_complete_walls(tf)

    # === КРЫША ===
    rf = model.roof
    v0 = rf.add_vertex(0, 0, 0)
    v1 = rf.add_vertex(model.width, 0, 0)
    v2 = rf.add_vertex(model.width, 0, model.depth)
    v3 = rf.add_vertex(0, 0, model.depth)
    v4 = rf.add_vertex(model.width / 2, model.roof_height, model.depth / 2)
    rf.add_polygon([v0, v1, v4], "#222222")
    rf.add_polygon([v1, v2, v4], "#222222")
    rf.add_polygon([v2, v3, v4], "#222222")
    rf.add_polygon([v3, v0, v4], "#222222")

    # Финальная коррекция нормалей
    gf.fix_normals()
    tf.fix_normals()
    rf.fix_normals()

    return model.to_json()


if __name__ == "__main__":
    print(generate_soviet_apartment())
