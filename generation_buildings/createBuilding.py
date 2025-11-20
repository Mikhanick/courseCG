# createBuilding.py
import json
import numpy as np
import random
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
        invert_normals: bool = False,
    ):
        pass


@dataclass
class SimplePolygon(GeometryComponent):
    points: List[Tuple[float, float]]
    color: Union[str, callable]
    invert: bool = False
    margin_type: str = "relative"  # Новое свойство
    margin_value: float = 0.0  # Новое свойство

    def calculate_absolute_points(self, scale: float) -> List[Tuple[float, float]]:
        """Рассчитывает абсолютные координаты точек с учетом типа отступа"""
        if self.margin_type == "absolute" and self.margin_value > 0:
            # Преобразуем абсолютные отступы в относительные для текущего масштаба
            relative_margin = self.margin_value / scale
            # Применяем отступы ко всем точкам
            return [
                (
                    min(max(x * (1 - 2 * relative_margin) + relative_margin, 0), 1),
                    min(max(y * (1 - 2 * relative_margin) + relative_margin, 0), 1),
                )
                for (x, y) in self.points
            ]
        return self.points

    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        scale: float,
        invert_normals: bool = False,
    ):
        x0, y0, z0 = position
        # Используем пересчитанные точки с учетом абсолютных отступов
        actual_points = self.calculate_absolute_points(scale)

        indices = []
        for px, py in actual_points:
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
        # Комбинируем флаги инверсии: локальный и глобальный
        final_invert = self.invert ^ invert_normals
        section.add_polygon(indices, self.get_color(), invert=final_invert)

    def get_color(self):
        """Возвращает цвет, вычисляя его динамически если это функция"""
        if callable(self.color):
            return self.color()
        return self.color


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
    color: Union[str, callable]  # Может быть строкой или функцией, возвращающей цвет
    invert: bool = False  # Новое свойство для управления нормалями
    # Внешние точки по умолчанию - прямоугольник 1x1
    outer_points: List[Tuple[float, float]] = field(
        default_factory=lambda: [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)]
    )

    def get_color(self):
        """Возвращает цвет, вычисляя его динамически если это функция"""
        if callable(self.color):
            return self.color()
        return self.color

    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        scale: float,
        invert_normals: bool = False,
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
        final_invert = self.invert ^ invert_normals
        color = self.get_color()
        for polygon_indices in frame_polygons:
            section.add_polygon(polygon_indices, color, invert=final_invert)

    def get_cutout_points(self) -> List[Tuple[float, float]]:
        """Возвращает точки выреза для создания окна того же размера"""
        return self.cutout_points


@dataclass
class CompositeGeometry(GeometryComponent):
    """Композитный геометрический объект — контейнер для других компонентов"""

    children: List[GeometryComponent] = field(default_factory=list)
    invert: bool = False  # Новое свойство для управления нормалями

    def add_child(self, child: GeometryComponent):
        self.children.append(child)

    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        scale: float,
        invert_normals: bool = False,
    ):
        # Комбинируем флаги инверсии
        final_invert = self.invert ^ invert_normals
        for child in self.children:
            # Передаем комбинированный флаг инверсии детям
            child.add_to_section(section, position, orientation, scale, final_invert)


@dataclass
class GeometrySegment(CompositeGeometry):
    """
    Сегмент геометрии — композитная панель.
    Состоит из рамки с вырезом и окна на месте выреза.
    """

    points: List[Tuple[float, float]] = field(default_factory=list)
    color: Union[str, callable] = "#8B7D6B"  # Может быть строкой или функцией
    cutouts: List[List[Tuple[float, float]]] = field(default_factory=list)
    invert: bool = False  # Новое свойство для управления нормалями

    def __post_init__(self):
        # Если заданы точки и цвет, создаем соответствующие геометрические компоненты
        if self.points and not self.children:
            if self.cutouts:
                # Если есть вырезы, создаем RectangleWithCutout для каждого выреза
                for cutout in self.cutouts:
                    self.add_child(
                        RectangleWithCutout(
                            cutout_points=cutout,
                            color=self.color,
                            invert=self.invert,  # Передаем флаг инверсии
                        )
                    )
            else:
                # Если нет вырезов, создаем SimplePolygon
                self.add_child(
                    SimplePolygon(
                        points=self.points,
                        color=self.color,
                        invert=self.invert,  # Передаем флаг инверсии
                    )
                )

    def get_color(self):
        """Возвращает цвет, вычисляя его динамически если это функция"""
        if callable(self.color):
            return self.color()
        return self.color


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
                # Инвертируем нормали для задней стены
                self.add_face(
                    vertex_map[v0], vertex_map[v1], vertex_map[v2], color, invert=True
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
                # Инвертируем нормали для правой стены
                self.add_face(
                    vertex_map[v0], vertex_map[v1], vertex_map[v2], color, invert=True
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

    def clear(self):
        """Очищает все вершины и грани секции"""
        self.vertices = []
        self.faces = []
        self.dimensions = [0.0, 0.0, 0.0]
        self.walls_created = {
            "front": False,
            "back": False,
            "left": False,
            "right": False,
        }


class RoofGeometry:
    """Специальный класс для создания сложной геометрии крыши"""

    @staticmethod
    def create_simple_roof(
        section: "BuildingSection",
        width: float,
        depth: float,
        height: float,
        base_height: float = 0.0,  # Добавлен параметр базовой высоты
        color: str = "#222222",
        invert: bool = False,
    ):
        """Создает простую двускатную крышу"""
        # Вершины основания крыши (с учетом базовой высоты)
        v0 = section.add_vertex(0, base_height, 0)
        v1 = section.add_vertex(width, base_height, 0)
        v2 = section.add_vertex(width, base_height, depth)
        v3 = section.add_vertex(0, base_height, depth)

        # Вершина конька крыши
        v4 = section.add_vertex(width / 2, base_height + height, depth / 2)

        # Грани крыши
        section.add_polygon([v0, v1, v4], color, invert=invert)
        section.add_polygon([v1, v2, v4], color, invert=invert)
        section.add_polygon([v2, v3, v4], color, invert=invert)
        section.add_polygon([v3, v0, v4], color, invert=invert)
        return section

    @staticmethod
    def create_flat_roof(
        section: "BuildingSection",
        width: float,
        depth: float,
        height: float,
        base_height: float = 0.0,  # Добавлен параметр базовой высоты
        color: str = "#444444",
        border_width: float = 0.5,
        invert: bool = False,
    ):
        """Создает плоскую крышу с бортиком"""
        # Нижние вершины крыши (уровень верха здания)
        v0 = section.add_vertex(0, base_height, 0)
        v1 = section.add_vertex(width, base_height, 0)
        v2 = section.add_vertex(width, base_height, depth)
        v3 = section.add_vertex(0, base_height, depth)

        # Верхние вершины бортика
        v4 = section.add_vertex(border_width, base_height + height, border_width)
        v5 = section.add_vertex(
            width - border_width, base_height + height, border_width
        )
        v6 = section.add_vertex(
            width - border_width, base_height + height, depth - border_width
        )
        v7 = section.add_vertex(
            border_width, base_height + height, depth - border_width
        )

        # Верхняя плоскость крыши
        section.add_polygon([v4, v5, v6, v7], "#333333", invert=invert)

        # Боковые стенки бортика
        section.add_polygon([v0, v1, v5, v4], color, invert=invert)
        section.add_polygon([v1, v2, v6, v5], color, invert=invert)
        section.add_polygon([v2, v3, v7, v6], color, invert=invert)
        section.add_polygon([v3, v0, v4, v7], color, invert=invert)

        return section

    @staticmethod
    def create_custom_roof(
        section: "BuildingSection",
        vertices: List[Tuple[float, float, float]],
        faces: List[List[int]],
        colors: List[str],
        base_height: float = 0.0,  # Добавлен параметр базовой высоты
        invert_flags: Optional[List[bool]] = None,
    ):
        """Создает кастомную крышу по заданным вершинам и граням"""
        if invert_flags is None:
            invert_flags = [False] * len(faces)

        vertex_indices = []
        for x, y, z in vertices:
            # Сдвигаем вершины на базовую высоту
            vertex_indices.append(section.add_vertex(x, y + base_height, z))

        for i, face in enumerate(faces):
            polygon_indices = [vertex_indices[idx] for idx in face]
            invert = invert_flags[i] if i < len(invert_flags) else False
            section.add_polygon(polygon_indices, colors[i % len(colors)], invert=invert)

        return section


class Building:
    """
    Упрощенный интерфейс для создания зданий
    """

    def __init__(
        self,
        name: str,
        description: str,
        author: str = "UrbanSim3D",
        version: str = "1.0",
        floor_count: int = 1,
        width_panels: int = 1,
        depth_panels: int = 1,
        panel_width: float = 5.0,
        panel_depth: float = 5.0,
        floor_height: float = 3.5,
        ground_floor_height: Optional[float] = None,
        roof_type: str = "simple",
        roof_height: Optional[float] = None,
    ):
        """
        Создает новое здание с автоматическим расчетом размеров

        :param width_panels: количество панелей по ширине здания
        :param depth_panels: количество панелей по глубине здания
        :param panel_width: ширина одной панели в метрах
        :param panel_depth: глубина одной панели в метрах
        :param floor_height: высота типового этажа в метрах
        :param ground_floor_height: высота первого этажа (если None, используется floor_height)
        """
        self.name = name
        self.description = description
        self.author = author
        self.version = version
        self.floor_count = floor_count

        # Расчет размеров здания
        self.width = width_panels * panel_width
        self.depth = depth_panels * panel_depth
        self.panel_width = panel_width
        self.panel_depth = panel_depth
        self.floor_height = floor_height
        self.ground_floor_height = ground_floor_height or floor_height
        self.roof_type = roof_type
        self.roof_height = roof_height or (floor_height * 0.43)

        # Инициализация секций
        self.ground_floor = BuildingSection()
        self.typical_floor = BuildingSection() if floor_count > 1 else None
        self.roof = BuildingSection()

        # Установка центров секций
        self._set_section_centers()

        # Список для хранения оконных компонентов
        self.window_panels = []

    def _set_section_centers(self):
        """Устанавливает центры для всех секций здания"""
        # Центр первого этажа
        gf_center = (self.width / 2, self.ground_floor_height / 2, self.depth / 2)
        self.ground_floor.set_center(gf_center)

        # Центр типового этажа (если есть)
        if self.typical_floor:
            tf_center = (
                self.width / 2,
                self.ground_floor_height + self.floor_height / 2,
                self.depth / 2,
            )
            self.typical_floor.set_center(tf_center)

        # Центр крыши
        roof_base_height = (
            self.ground_floor_height + max(0, self.floor_count - 1) * self.floor_height
        )
        roof_center = (
            self.width / 2,
            roof_base_height + self.roof_height / 2,
            self.depth / 2,
        )
        self.roof.set_center(roof_center)

    @staticmethod
    def create_window_frame(
        margin: float = 0.23,
        margin_type: str = "relative",  # "relative" или "absolute"
        frame_color: str = "#A4A4A4",
        glass_color: Optional[Union[str, callable]] = None,
        invert_frame: bool = False,
        invert_glass: bool = False,
    ) -> GeometrySegment:
        """
        Создает стандартную панель с окном
        
        :param margin: отступ от края панели до окна
        :param margin_type: "relative" (доля от размера панели) или "absolute" (в метрах)
        :param frame_color: цвет рамки окна
        :param glass_color: цвет стекла (если None, используется случайный цвет)
        :param invert_frame: инвертировать нормали для рамки
        :param invert_glass: инвертировать нормали для стекла
        :return: Готовая панель с окном
        """
        # Создаем сегмент
        panel = GeometrySegment(invert=invert_frame)

        # Если margin задан в абсолютных единицах, преобразуем в относительные
        # (это будет сделано при добавлении в секцию с учетом scale)
        # Для геометрических компонентов всегда используем относительные координаты [0..1]
        window_points = [
            (margin if margin_type == "relative" else 0.0, 
            margin if margin_type == "relative" else 0.0),
            (1.0 - (margin if margin_type == "relative" else 0.0), 
            margin if margin_type == "relative" else 0.0),
            (1.0 - (margin if margin_type == "relative" else 0.0), 
            1.0 - (margin if margin_type == "relative" else 0.0)),
            (margin if margin_type == "relative" else 0.0, 
            1.0 - (margin if margin_type == "relative" else 0.0)),
        ]

        # Добавляем рамку с вырезом
        panel.add_child(
            RectangleWithCutout(
                cutout_points=window_points, 
                color=frame_color, 
                invert=invert_frame,
            )
        )

        # Определяем цвет стекла
        if glass_color is None:
            glass_color = Building.get_window_color  

        # Добавляем стекло
        panel.add_child(
            SimplePolygon(
                points=window_points, 
                color=glass_color, 
                invert=invert_glass,
                margin_type=margin_type,
                margin_value=margin
            )
        )

        return panel

    @staticmethod
    def create_solid_panel(
        color: Union[str, callable] = "#A4A4A4", invert: bool = False
    ) -> GeometrySegment:
        """
        Создает сплошную панель без окон

        :param color: цвет панели (строка или функция)
        :param invert: инвертировать нормали
        :return: Готовая сплошная панель
        """
        panel = GeometrySegment(invert=invert)
        panel.add_child(
            SimplePolygon(
                points=[(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)],
                color=color,
                invert=invert,
            )
        )
        return panel

    @staticmethod
    def get_window_color():
        r = random.random()

        # Приоритет: особый цвет (2%)
        if r < 0.02:
            return "#3E007D"  # deep electric purple — тонированное/декоративное стекло

        # Освещённые окна: суммарно 10% (8% + 2%)
        elif r < 0.10:  # [0.02, 0.10) → 8%
            return "#FFD700"  # светло-жёлтый, открытое освещённое окно

        elif r < 0.12:  # [0.10, 0.12) → 2%
            # Свет сквозь плотные шторы: тёплый, приглушённый
            return random.choice(
                [
                    "#B8860B",  # DarkGoldenrod — золотисто-коричневый
                    "#A0522D",  # Sienna — тёплая глубокая земля
                    "#8B4513",  # SaddleBrown — более тёмный вариант
                    "#C19A6B",  # Camel — мягкий бежевый (для полупрозрачных штор)
                ]
            )

        # Иначе — неосвещённое окно: 88%
        else:
            dark_and_mid_colors = [
                # Deep blues
                "#393951",
                "#4C5157",
                "#243457",
                "#13171F",
                "#48657C",
                # Deep greens
                "#00271E",
                "#0B3D2E",
                "#023020",
                "#123C29",
                "#003B36",
                # Deep reds/maroons
                "#1A0000",
                "#3D0C02",
                "#4C0C0C",
                "#5D0C0C",
                "#610B0B",
                # Charcoal/grays (cool/warm)
                "#161528",
                "#20202C",
                "#25243B",
                "#2D2B3F",
                "#2B2B2B",
                "#333333",
                "#3A3A3A",
                "#424245",
                # Muted midtones (still 'dark', but readable as unlit windows)
                "#4A4A5A",
                "#505060",
                "#4E5566",
                "#5A586D",
                "#58586A",
                "#3B3B4F",
                "#40404F",
                "#4C4C5D",
                # Deep browns (brick/panels reflection)
                "#281E15",
                "#3B2F2F",
                "#2E1F17",
                "#3C2A21",
                "#4B3621",
            ]
            return random.choice(dark_and_mid_colors)

    def add_wall(
        self,
        wall_type: str,
        panels: List[GeometryComponent],
        floor_type: str = "ground",
        panels_count: Optional[int] = None,
        invert: bool = False,
    ):
        """
        Добавляет стену к зданию

        :param wall_type: тип стены ('front', 'back', 'left', 'right')
        :param panels: список панелей для стены
        :param floor_type: тип этажа ('ground' или 'typical')
        :param panels_count: количество панелей (если None, используется длина списка panels)
        :param invert: инвертировать нормали для всей стены
        """
        # Определяем секцию
        if floor_type == "ground":
            section = self.ground_floor
        elif floor_type == "typical" and self.typical_floor:
            section = self.typical_floor
        else:
            raise ValueError("Invalid floor type or typical floor not available")

        # Определяем количество панелей
        if panels_count is None:
            panels_count = len(panels)

        # Определяем размер панели в зависимости от ориентации стены
        panel_size = (
            self.panel_width if wall_type in ["front", "back"] else self.panel_depth
        )
        total_size = self.width if wall_type in ["front", "back"] else self.depth

        # Добавляем панели
        for i in range(panels_count):
            panel = panels[i % len(panels)]
            if wall_type == "front":
                pos = (i * panel_size, 0, 0)
                orientation = "front"
            elif wall_type == "back":
                pos = (i * panel_size, 0, self.depth)
                orientation = "back"
            elif wall_type == "left":
                pos = (0, 0, i * panel_size)
                orientation = "left"
            elif wall_type == "right":
                pos = (self.width, 0, i * panel_size)
                orientation = "right"
            else:
                raise ValueError(f"Invalid wall type: {wall_type}")

            section.add_geometry_segment(panel, pos, orientation, panel_size, invert)

    def complete_walls(self, floor_type: str = "ground"):
        """
        Автоматически завершает недостающие стены

        :param floor_type: тип этажа ('ground' или 'typical')
        """
        if floor_type == "ground":
            section = self.ground_floor
        elif floor_type == "typical" and self.typical_floor:
            section = self.typical_floor
        else:
            raise ValueError("Invalid floor type or typical floor not available")

        # Завершаем стены (с инверсией нормалей для противоположных стен)
        if section.walls_created["front"] and not section.walls_created["back"]:
            section.copy_front_wall_to_back(self.width, self.depth, invert=True)

        if section.walls_created["left"] and not section.walls_created["right"]:
            section.copy_left_wall_to_right(self.width, self.depth, invert=True)

    def create_roof(self):
        """Создает крышу в соответствии с выбранным типом"""
        self.roof.clear()

        roof_base_height = (
            self.ground_floor_height + max(0, self.floor_count - 1) * self.floor_height
        )

        if self.roof_type == "simple":
            RoofGeometry.create_simple_roof(
                self.roof,
                self.width,
                self.depth,
                self.roof_height,
                base_height=roof_base_height,  # Передаем базовую высоту
            )
        elif self.roof_type == "flat":
            RoofGeometry.create_flat_roof(
                self.roof,
                self.width,
                self.depth,
                self.roof_height,
                base_height=roof_base_height,  # Передаем базовую высоту
                border_width=min(self.panel_width, self.panel_depth) * 0.1,
            )
        elif self.roof_type == "custom":
            # Для кастомной крыши пользователь должен вызвать add_custom_roof
            pass
        else:
            # По умолчанию создаем простую крышу
            RoofGeometry.create_simple_roof(
                self.roof,
                self.width,
                self.depth,
                self.roof_height,
                base_height=roof_base_height,  # Передаем базовую высоту
            )

    def add_custom_roof(
        self,
        vertices: List[Tuple[float, float, float]],
        faces: List[List[int]],
        colors: List[str],
        invert_flags: Optional[List[bool]] = None,
    ):
        """Добавляет кастомную геометрию крыши"""
        self.roof_type = "custom"
        self.roof.clear()

        roof_base_height = (
            self.ground_floor_height + max(0, self.floor_count - 1) * self.floor_height
        )

        RoofGeometry.create_custom_roof(
            self.roof,
            vertices,
            faces,
            colors,
            base_height=roof_base_height,  # Передаем базовую высоту
            invert_flags=invert_flags,
        )

    def finalize(self):
        """Завершает создание здания и корректирует нормали"""
        if self.roof_type != "custom":
            self.create_roof()

        self.ground_floor.fix_normals()
        if self.typical_floor:
            self.typical_floor.fix_normals()
        self.roof.fix_normals()

    def to_json(self) -> str:
        """Преобразует здание в JSON"""
        # Создаем структуру для экспорта
        model = {
            "metadata": {
                "name": self.name,
                "description": self.description,
                "author": self.author,
                "version": self.version,
            },
            "dimensions": {
                "min_width": self.width * 1.05,
                "max_width": self.width * 2.05,
                "min_depth": self.depth * 1.05,
                "max_depth": self.depth * 2.05,
                "fixed_scale": True,
            },
            "floors": {
                "count": self.floor_count,
                "texture_scale": 1.0,
            },
            "ground_floor": {
                "vertices": self.ground_floor.vertices,
                "faces": self.ground_floor.faces,
            },
        }

        if self.typical_floor:
            model["typical_floor"] = {
                "vertices": self.typical_floor.vertices,
                "faces": self.typical_floor.faces,
            }

        model["roof"] = {
            "vertices": self.roof.vertices,
            "faces": self.roof.faces,
        }

        return json.dumps(model, indent=2)

    def save_to_file(self, filename: str):
        """Сохраняет здание в файл"""
        with open(filename, "w", encoding="utf-8") as f:
            f.write(self.to_json())


# Примеры использования упрощенного интерфейса:


def generate_soviet_apartment() -> str:
    """Генерирует советский панельный дом"""
    # Создаем здание с упрощенным интерфейсом
    building = Building(
        name="apartment_soviet_automatic",
        description="Советский панельный дом с автоматическими размерами",
        floor_count=9,
        width_panels=10,  # 10 панелей по ширине
        depth_panels=4,  # 4 панели по глубине
        panel_width=5.0,  # ширина панели 5 метров
        panel_depth=7.5,  # глубина панели 7.5 метров
        floor_height=3.5,  # высота типового этажа
        ground_floor_height=5.0,  # высота первого этажа
        roof_type="simple",
    )

    # Создаем панель для входа (без окна)
    entrance_panel = Building.create_solid_panel(color="#A4A4A4")

    # Добавляем стены первого этажа
    # Каждая панель создается отдельно для уникального цвета окна
    front_panels = []
    for i in range(10):
        if i == 2:  # центральная панель - вход
            front_panels.append(entrance_panel)
        else:
            front_panels.append(
                Building.create_window_frame(
                    margin=0.23,
                    frame_color="#A4A4A4",
                    # glass_color будет вызван при добавлении панели
                )
            )

    building.add_wall("front", front_panels, "ground")

    # Для боковых стен создаем отдельные панели
    left_panels = [
        Building.create_window_frame(
            margin=0.23,
            frame_color="#A4A4A4",
        )
        for _ in range(4)
    ]
    building.add_wall("left", left_panels, "ground")
    building.complete_walls("ground")

    # Добавляем стены типовых этажей
    if building.typical_floor:
        typical_front_panels = [
            Building.create_window_frame(
                margin=0.23,
                frame_color="#A4A4A4",
            )
            for _ in range(10)
        ]
        building.add_wall("front", typical_front_panels, "typical")

        typical_left_panels = [
            Building.create_window_frame(
                margin=0.23,
                frame_color="#A4A4A4",
            )
            for _ in range(4)
        ]
        building.add_wall("left", typical_left_panels, "typical")
        building.complete_walls("typical")

    # Завершаем создание здания
    building.finalize()

    return building.to_json()


def generate_building_kiosk_pechat() -> str:
    """Генерирует киоск печати с вывеской"""
    # Создаем киоск с упрощенным интерфейсом
    building = Building(
        name="kiosk_pechat",
        description="Киоск печати с вывеской",
        floor_count=1,
        width_panels=3,  # 3 панели по ширине
        depth_panels=2,  # 2 панели по глубине
        panel_width=2.0,  # ширина панели 2 метра
        panel_depth=2.0,  # глубина панели 2 метра
        floor_height=3.0,  # высота этажа
        roof_type="custom",  # кастомная крыша для вывески
    )

    # Создаем панели с уникальными цветами окон
    front_panels = [
        Building.create_window_frame(
            margin=0.15,
            frame_color="#964B00",  # Коричневый цвет для киоска
            glass_color="#336699",  # Голубое стекло (фиксированный цвет)
        )
        for _ in range(3)
    ]

    # Создаем панель для задней стены (без окна)
    back_panels = [
        Building.create_solid_panel(color="#8B4513")
        for _ in range(3)  # Темно-коричневый
    ]

    left_panels = [
        Building.create_window_frame(
            margin=0.15,
            frame_color="#964B00",
            glass_color="#336699",
        )
        for _ in range(2)
    ]

    # Добавляем стены
    building.add_wall("front", front_panels, "ground")
    building.add_wall("left", left_panels, "ground")
    building.add_wall("back", back_panels, "ground")
    building.complete_walls("ground")

    # Создаем кастомную крышу с вывеской
    roof_base_height = building.ground_floor_height  # Высота основания крыши

    roof_vertices = [
        # Основание крыши (уровень верха здания)
        (0, 0, 0),
        (building.width, 0, 0),
        (building.width, 0, building.depth),
        (0, 0, building.depth),
        # Верхняя часть бортика
        (0.2, 0.5, 0.2),
        (building.width - 0.2, 0.5, 0.2),
        (building.width - 0.2, 0.5, building.depth - 0.2),
        (0.2, 0.5, building.depth - 0.2),
        # Вывеска над входом
        (building.width * 0.2, 0.6, -0.3),
        (building.width * 0.8, 0.6, -0.3),
        (building.width * 0.8, 0.9, -0.3),
        (building.width * 0.2, 0.9, -0.3),
        (building.width * 0.2, 0.6, 0),
        (building.width * 0.8, 0.6, 0),
        (building.width * 0.8, 0.9, 0),
        (building.width * 0.2, 0.9, 0),
    ]

    roof_faces = [
        [4, 5, 6, 7],  # Верх крыши
        [0, 1, 5, 4],
        [1, 2, 6, 5],
        [2, 3, 7, 6],
        [3, 0, 4, 7],  # Бортики
        [8, 9, 10, 11],
        [11, 10, 14, 15],
        [12, 13, 14, 15],  # Вывеска спереди/сверху/сзади
        [8, 12, 15, 11],
        [9, 13, 14, 10],  # Боковые части вывески
    ]

    roof_colors = [
        "#333333",
        "#444444",
        "#444444",
        "#444444",
        "#444444",
        "#AA0000",
        "#880000",
        "#660000",
        "#990000",
        "#990000",
    ]

    # Флаги инверсии для каждого полигона
    invert_flags = [
        False,  # Верх крыши
        False,
        False,
        False,
        False,  # Бортики
        False,
        False,
        True,  # Вывеска сзади (инвертируем)
        False,
        False,  # Боковые части вывески
    ]

    # Добавляем кастомную крышу
    building.add_custom_roof(roof_vertices, roof_faces, roof_colors, invert_flags)

    # Завершаем создание здания
    building.finalize()

    return building.to_json()


if __name__ == "__main__":
    # Генерация зданий
    soviet_apartment = generate_soviet_apartment()
    kiosk_pechat = generate_building_kiosk_pechat()

    # Сохранение в файлы
    with open("soviet_apartment.json", "w", encoding="utf-8") as f:
        f.write(soviet_apartment)

    with open("kiosk_pechat.json", "w", encoding="utf-8") as f:
        f.write(kiosk_pechat)

    print("Здания успешно сгенерированы!")
    print("- soviet_apartment.json")
    print("- kiosk_pechat.json")
