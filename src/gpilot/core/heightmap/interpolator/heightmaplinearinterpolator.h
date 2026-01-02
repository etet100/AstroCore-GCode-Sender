#ifndef HEIGHTMAPLINEARINTERPOLATOR_H
#define HEIGHTMAPLINEARINTERPOLATOR_H

#include "heightmapinterpolator.h"

class HeightmapLinearInterpolator : public HeightmapInterpolator
{
    public:
        HeightmapLinearInterpolator(const Heightmap& heightmap);
        double interpolate(QPointF point) const override;
};

#endif // HEIGHTMAPLINEARINTERPOLATOR_H
