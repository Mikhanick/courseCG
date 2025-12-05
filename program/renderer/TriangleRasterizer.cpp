#include "TriangleRasterizer.h"

bool TriangleRasterizer::IsTriangleOutsideViewport(
    const QVector3D &p0,
    const QVector3D &p1,
    const QVector3D &p2,
    const QRectF &viewport)
{
    // Проверяем по каждой границе
    // Левая граница: все вершины слева от viewport.left()
    if (p0.x() < viewport.left() && p1.x() < viewport.left() && p2.x() < viewport.left())
    {
        return true;
    }

    // Правая граница: все вершины справа от viewport.right()
    if (p0.x() > viewport.right() && p1.x() > viewport.right() && p2.x() > viewport.right())
    {
        return true;
    }

    // Верхняя граница: все вершины выше viewport.top() (Y вниз!)
    if (p0.y() < viewport.top() && p1.y() < viewport.top() && p2.y() < viewport.top())
    {
        return true;
    }

    // Нижняя граница: все вершины ниже viewport.bottom()
    if (p0.y() > viewport.bottom() && p1.y() > viewport.bottom() && p2.y() > viewport.bottom())
    {
        return true;
    }

    return false;
}
