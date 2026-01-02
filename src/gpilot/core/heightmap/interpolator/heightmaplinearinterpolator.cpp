#include "heightmaplinearinterpolator.h"

HeightmapLinearInterpolator::HeightmapLinearInterpolator(const Heightmap& heightmap) : HeightmapInterpolator(heightmap)
{
}

// linear interpolation
double HeightmapLinearInterpolator::interpolate(QPointF point) const
{
    // if (!m_heightmap.isInside(point)) {
    //     return NAN;
    // }

    // return rand() % 500 / 100.0;

    QSize gridSize = m_heightmap.gridSize();

    // Get grid indices
    // Let's assume that point value is in grid units
    auto [x, y] = point;
        //m_heightmap.gridIndices(point);

    // gridSize is a size

    // double x0 = m_heightmap.startPos().x() + x * gridSize.width();
    // double y0 = m_heightmap.startPos().y() + y * gridSize.height();
    double x0 = x;
    double y0 = y;

    // Z values at the corners of the grid cell
    double z00 = m_heightmap.at((int) x, (int) y);
    double z10 = m_heightmap.at((int) x + 1, (int) y);
    double z01 = m_heightmap.at((int) x, (int) y + 1);
    double z11 = m_heightmap.at((int) x + 1, (int) y + 1);

    // Calculate fractional offset in the grid
    // double dx = (point.x() - x0) / gridSize.width();
    // double dy = (point.y() - y0) / gridSize.height();
    // double dx = point.x() - x0;
    double dx = x - (int) x;
    double dy = y - (int) y;

    // Determine which edge to interpolate along
    if (dx > dy) {
        // Interpolate along the bottom edge
        double z0 = z00 * (1 - dx) + z10 * dx;
        double z1 = z10 * (1 - dy) + z11 * dy;

        return z0 * (1 - dy) + z1 * dy;
    } else {
        // Interpolate along the left edge
        double z0 = z00 * (1 - dy) + z01 * dy;
        double z1 = z01 * (1 - dx) + z11 * dx;

        return z0 * (1 - dx) + z1 * dx;
    }
}
