# createBuilding.py
import json
import numpy as np
import random
from typing import List, Tuple, Dict, Any, Optional, Union
from dataclasses import dataclass, field
from abc import ABC, abstractmethod


class GeometryComponent(ABC):
    """Базовый класс для геометрических компонентов с поддержкой 3D-координат"""

    @abstractmethod
    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        invert_normals: bool = False,
    ):
        pass


@dataclass
class SimplePolygon(GeometryComponent):
    """Простой полигон с поддержкой абсолютных 3D-координат"""

    points: List[
        Tuple[float, float, float]
    ]  # Абсолютные координаты в локальной системе панели (x, y, z)
    color: Union[str, callable]
    invert: bool = False

    def get_color(self):
        """Возвращает цвет, вычисляя его динамически если это функция"""
        if callable(self.color):
            return self.color()
        return self.color

    # НАЙДИТЕ ЭТИ МЕТОДЫ В КЛАССАХ SimplePolygon И RectangleWithCutout:


    # В классе SimplePolygon, метод add_to_section:
    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        invert_normals: bool = False,
    ):
        x0, y0, z0 = position
        indices = []

        for local_x, local_y, local_z in self.points:
            # ИЗМЕНИТЕ ЭТУ ЧАСТЬ КОДА:
            if orientation == "front":
                # Фасадная стена на z=0, наружу - отрицательное направление z
                wx = x0 + local_x
                wy = y0 + local_y
                wz = z0 - local_z  # БЫЛО: z0 + local_z

            elif orientation == "back":
                # Задняя стена на z=depth, наружу - положительное направление z
                wx = x0 + local_x
                wy = y0 + local_y
                wz = z0 + local_z  # БЫЛО: z0 - local_z

            elif orientation == "left":
                # Левая стена на x=0, наружу - отрицательное направление x
                wx = x0 - local_z
                wy = y0 + local_y
                wz = z0 + local_x

            elif orientation == "right":
                # Правая стена на x=width, наружу - положительное направление x
                wx = x0 + local_z
                wy = y0 + local_y
                wz = z0 + local_x

            else:
                raise ValueError(f"Invalid orientation: {orientation}")

            idx = section.add_vertex(wx, wy, wz)
            indices.append(idx)

        final_invert = self.invert ^ invert_normals
        section.add_polygon(indices, self.get_color(), invert=final_invert)

@dataclass
class RectangleWithCutout(GeometryComponent):
    """
    Полигон с вырезом в абсолютных 3D-координатах
    """

    outer_points: List[Tuple[float, float, float]]  # Внешний контур в метрах (x, y, z)
    cutout_points: List[Tuple[float, float, float]]  # Контур выреза в метрах (x, y, z)
    color: Union[str, callable]
    invert: bool = False

    def get_color(self):
        """Возвращает цвет, вычисляя его динамически если это функция"""
        if callable(self.color):
            return self.color()
        return self.color

    # В классе RectangleWithCutout, метод add_to_section:
    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        invert_normals: bool = False,
    ):
        x0, y0, z0 = position
        outer_indices = []
        cutout_indices = []

        # ИЗМЕНИТЕ ЭТУ ЧАСТЬ ДЛЯ ВНЕШНЕГО КОНТУРА:
        for local_x, local_y, local_z in self.outer_points:
            if orientation == "front":
                wx = x0 + local_x
                wy = y0 + local_y
                wz = z0 - local_z  # БЫЛО: z0 + local_z
            elif orientation == "back":
                wx = x0 + local_x
                wy = y0 + local_y
                wz = z0 + local_z  # БЫЛО: z0 - local_z
            elif orientation == "left":
                wx = x0 - local_z
                wy = y0 + local_y
                wz = z0 + local_x
            elif orientation == "right":
                wx = x0 + local_z
                wy = y0 + local_y
                wz = z0 + local_x
            else:
                raise ValueError(f"Invalid orientation: {orientation}")

            idx = section.add_vertex(wx, wy, wz)
            outer_indices.append(idx)

        # ИЗМЕНИТЕ ЭТУ ЧАСТЬ ДЛЯ КОНТУРА ВЫРЕЗА:
        for local_x, local_y, local_z in self.cutout_points:
            if orientation == "front":
                wx = x0 + local_x
                wy = y0 + local_y
                wz = z0 - local_z  # БЫЛО: z0 + local_z
            elif orientation == "back":
                wx = x0 + local_x
                wy = y0 + local_y
                wz = z0 + local_z  # БЫЛО: z0 - local_z
            elif orientation == "left":
                wx = x0 - local_z
                wy = y0 + local_y
                wz = z0 + local_x
            elif orientation == "right":
                wx = x0 + local_z
                wy = y0 + local_y
                wz = z0 + local_x
            else:
                raise ValueError(f"Invalid orientation: {orientation}")

            idx = section.add_vertex(wx, wy, wz)
            cutout_indices.append(idx)

        # Создаем грани для рамки (между внешним и внутренним контурами)
        frame_polygons = []
        n_outer = len(outer_indices)
        n_cutout = len(cutout_indices)

        # Триангуляция для прямоугольной рамки
        if n_outer == 4 and n_cutout == 4:
            # Нижняя грань рамки
            frame_polygons.append(
                [
                    outer_indices[0],
                    outer_indices[1],
                    cutout_indices[1],
                    cutout_indices[0],
                ]
            )
            # Правая грань рамки
            frame_polygons.append(
                [
                    outer_indices[1],
                    outer_indices[2],
                    cutout_indices[2],
                    cutout_indices[1],
                ]
            )
            # Верхняя грань рамки
            frame_polygons.append(
                [
                    outer_indices[2],
                    outer_indices[3],
                    cutout_indices[3],
                    cutout_indices[2],
                ]
            )
            # Левая грань рамки
            frame_polygons.append(
                [
                    outer_indices[3],
                    outer_indices[0],
                    cutout_indices[0],
                    cutout_indices[3],
                ]
            )
        else:
            # Общая триангуляция для произвольных контуров
            for i in range(min(n_outer, n_cutout)):
                next_i = (i + 1) % min(n_outer, n_cutout)
                frame_polygons.append(
                    [
                        outer_indices[i],
                        outer_indices[next_i],
                        cutout_indices[next_i],
                        cutout_indices[i],
                    ]
                )

        final_invert = self.invert ^ invert_normals
        color = self.get_color()
        for polygon_indices in frame_polygons:
            section.add_polygon(polygon_indices, color, invert=final_invert)


@dataclass
class CompositeGeometry(GeometryComponent):
    """Композитный геометрический объект — контейнер для других компонентов"""

    children: List[GeometryComponent] = field(default_factory=list)
    invert: bool = False

    def add_child(self, child: GeometryComponent):
        self.children.append(child)

    def add_to_section(
        self,
        section: "BuildingSection",
        position: Tuple[float, float, float],
        orientation: str,
        invert_normals: bool = False,
    ):
        final_invert = self.invert ^ invert_normals
        for child in self.children:
            child.add_to_section(section, position, orientation, final_invert)


@dataclass
class GeometrySegment(CompositeGeometry):
    """Сегмент геометрии — композитная панель с поддержкой 3D-элементов"""

    children: List[GeometryComponent] = field(default_factory=list)
    invert: bool = False

    def __post_init__(self):
        pass  # Никакой автоматической геометрии по умолчанию


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
        invert_normals: bool = False,
    ):
        """Добавляет сегмент с абсолютными координатами"""
        segment.add_to_section(self, position, orientation, invert_normals)
        self.walls_created[orientation] = True

    def copy_front_wall_to_back(self, width: float, depth: float, invert: bool = False):
        """Копирует фасадную стену на заднюю с правильной ориентацией"""
        front_vertices = []
        for i, v in enumerate(self.vertices):
            if abs(v[2] - 0) < 0.001:  # Находим вершины фасадной стены (z=0)
                front_vertices.append((i, v))
        if not front_vertices:
            return
        vertex_map = {}
        for orig_idx, v in front_vertices:
            # Копируем вершины на заднюю стену (z=depth)
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
        """Копирует левую стену на правую с правильной ориентацией"""
        left_vertices = []
        for i, v in enumerate(self.vertices):
            if abs(v[0] - 0) < 0.001:  # Находим вершины левой стены (x=0)
                left_vertices.append((i, v))
        if not left_vertices:
            return
        vertex_map = {}
        for orig_idx, v in left_vertices:
            # Копируем вершины на правую стену (x=width)
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
        base_height: float = 0.0,  # Всегда 0 по требованию
        color: str = "#222222",
        invert: bool = False,
    ):
        """Создает простую двускатную крышу начинающуюся с высоты 0"""
        # Вершины основания крыши на высоте 0
        v0 = section.add_vertex(0, 0, 0)
        v1 = section.add_vertex(width, 0, 0)
        v2 = section.add_vertex(width, 0, depth)
        v3 = section.add_vertex(0, 0, depth)

        # Вершина конька крыши
        v4 = section.add_vertex(width / 2, height, depth / 2)

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
        base_height: float = 0.0,  # Всегда 0 по требованию
        color: str = "#444444",
        border_width: float = 0.5,
        invert: bool = False,
    ):
        """Создает плоскую крышу с бортиком начинающуюся с высоты 0"""
        # Нижние вершины крыши (на высоте 0)
        v0 = section.add_vertex(0, 0, 0)
        v1 = section.add_vertex(width, 0, 0)
        v2 = section.add_vertex(width, 0, depth)
        v3 = section.add_vertex(0, 0, depth)

        # Верхние вершины бортика
        v4 = section.add_vertex(border_width, height, border_width)
        v5 = section.add_vertex(width - border_width, height, border_width)
        v6 = section.add_vertex(width - border_width, height, depth - border_width)
        v7 = section.add_vertex(border_width, height, depth - border_width)

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
        base_height: float = 0.0,  # Всегда 0 по требованию
        invert_flags: Optional[List[bool]] = None,
    ):
        """Создает кастомную крышу по заданным вершинам и граням начинающуюся с высоты 0"""
        if invert_flags is None:
            invert_flags = [False] * len(faces)

        vertex_indices = []
        for x, y, z in vertices:
            # Сдвигаем вершины на базовую высоту (всегда 0)
            vertex_indices.append(section.add_vertex(x, y + base_height, z))

        for i, face in enumerate(faces):
            polygon_indices = [vertex_indices[idx] for idx in face]
            invert = invert_flags[i] if i < len(invert_flags) else False
            section.add_polygon(polygon_indices, colors[i % len(colors)], invert=invert)

        return section


class Building:
    """
    Упрощенный интерфейс для создания зданий с абсолютными координатами
    """

    def __init__(
        self,
        name: str,
        description: str,
        author: str = "UrbanSim3D",
        version: str = "1.0",
        floor_count: int = 1,
        width: float = 20.0,  # Общая ширина здания в метрах
        depth: float = 8.0,  # Общая глубина здания в метрах
        ground_floor_height: float = 2.8,
        typical_floor_height: float = 2.8,
        roof_type: str = "flat",
        roof_height: Optional[float] = None,
    ):
        """
        Создает новое здание с абсолютными размерами

        :param width: общая ширина здания в метрах (по оси X)
        :param depth: общая глубина здания в метрах (по оси Z)
        :param ground_floor_height: высота первого этажа в метрах
        :param typical_floor_height: высота типовых этажей в метрах
        """
        self.name = name
        self.description = description
        self.author = author
        self.version = version
        self.floor_count = floor_count

        # Абсолютные размеры здания
        self.width = width
        self.depth = depth
        self.ground_floor_height = ground_floor_height
        self.typical_floor_height = typical_floor_height
        self.roof_type = roof_type
        self.roof_height = roof_height or (
            max(ground_floor_height, typical_floor_height) * 0.4
        )

        # Инициализация секций
        self.ground_floor = BuildingSection()
        self.typical_floor = BuildingSection() if floor_count > 1 else None
        self.roof = BuildingSection()

        # Установка центров секций
        self._set_section_centers()

    def _set_section_centers(self):
        """Устанавливает центры для всех секций здания"""
        # Центр первого этажа
        gf_center = (self.width / 2, self.ground_floor_height / 2, self.depth / 2)
        self.ground_floor.set_center(gf_center)

        # Центр типового этажа (если есть)
        if self.typical_floor:
            tf_center = (
                self.width / 2,
                self.ground_floor_height + self.typical_floor_height / 2,
                self.depth / 2,
            )
            self.typical_floor.set_center(tf_center)

        # Центр крыши (всегда начинается с высоты 0)
        roof_center = (
            self.width / 2,
            self.roof_height / 2,  # Не добавляем высоту этажей
            self.depth / 2,
        )
        self.roof.set_center(roof_center)

    @staticmethod
    def get_window_color():
        """Генерирует случайный цвет для окна"""
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
                "#5D4C4C",
                "#623C3C",
                "#431C1C",
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
        start_position: Optional[Tuple[float, float, float]] = None,
        invert: bool = False,
    ):
        """
        Добавляет стену к зданию с абсолютными координатами

        :param wall_type: тип стены ('front', 'back', 'left', 'right')
        :param panels: список панелей для стены
        :param floor_type: тип этажа ('ground' или 'typical')
        :param start_position: начальная позиция (x, y, z) для первой панели
        :param invert: инвертировать нормали для всей стены
        """
        # Определяем секцию
        if floor_type == "ground":
            section = self.ground_floor
        elif floor_type == "typical" and self.typical_floor:
            section = self.typical_floor
        else:
            raise ValueError("Invalid floor type or typical floor not available")

        # Определяем текущую высоту этажа
        current_height = (
            self.ground_floor_height
            if floor_type == "ground"
            else self.typical_floor_height
        )

        # Определяем позицию по умолчанию
        if start_position is None:
            y_pos = 0
            if wall_type == "front":
                start_position = (0, y_pos, 0)
            elif wall_type == "back":
                start_position = (0, y_pos, self.depth)
            elif wall_type == "left":
                start_position = (0, y_pos, 0)
            elif wall_type == "right":
                start_position = (self.width, y_pos, 0)
            else:
                raise ValueError(f"Invalid wall type: {wall_type}")

        x0, y0, z0 = start_position

        # Добавляем каждую панель
        for i, panel in enumerate(panels):
            if wall_type == "front":
                # Для фасада: двигаемся по оси X
                pos = (x0 + i * self.width / len(panels), y0, z0)
                orientation = "front"

            elif wall_type == "back":
                # Для задней стены: двигаемся по оси X
                pos = (x0 + i * self.width / len(panels), y0, z0)
                orientation = "back"

            elif wall_type == "left":
                # Для левой стены: двигаемся по оси Z
                pos = (x0, y0, z0 + i * self.depth / len(panels))
                orientation = "left"

            elif wall_type == "right":
                # Для правой стены: двигаемся по оси Z
                pos = (x0, y0, z0 + i * self.depth / len(panels))
                orientation = "right"

            else:
                raise ValueError(f"Invalid wall type: {wall_type}")

            section.add_geometry_segment(panel, pos, orientation, invert)

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

        # Для левой и правой стен используем высоту текущего этажа
        current_height = (
            self.ground_floor_height
            if floor_type == "ground"
            else self.typical_floor_height
        )

        if section.walls_created["left"] and not section.walls_created["right"]:
            section.copy_left_wall_to_right(self.width, self.depth, invert=True)

    def create_roof(self):
        """Создает крышу в соответствии с выбранным типом, начинающуюся с высоты 0"""
        self.roof.clear()

        if self.roof_type == "simple":
            RoofGeometry.create_simple_roof(
                self.roof,
                self.width,
                self.depth,
                self.roof_height,
                base_height=0.0,  # Всегда 0 по требованию
            )
        elif self.roof_type == "flat":
            RoofGeometry.create_flat_roof(
                self.roof,
                self.width,
                self.depth,
                self.roof_height,
                base_height=0.0,  # Всегда 0 по требованию
                border_width=min(self.width, self.depth)
                * 0.025,  # 2.5% от меньшего размера
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
                base_height=0.0,  # Всегда 0 по требованию
            )

    def add_custom_roof(
        self,
        vertices: List[Tuple[float, float, float]],
        faces: List[List[int]],
        colors: List[str],
        invert_flags: Optional[List[bool]] = None,
    ):
        """Добавляет кастомную геометрию крыши начинающуюся с высоты 0"""
        self.roof_type = "custom"
        self.roof.clear()

        RoofGeometry.create_custom_roof(
            self.roof,
            vertices,
            faces,
            colors,
            base_height=0.0,  # Всегда 0 по требованию
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
        import os
        directory = os.path.dirname(filename)
        if directory and not os.path.exists(directory):
            os.makedirs(directory)

        with open(filename, "w", encoding="utf-8") as f:
            f.write(self.to_json())
