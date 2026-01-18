#ifndef BOUNDINGBOXDRAWER_H
#define BOUNDINGBOXDRAWER_H

#include <QObject>
#include "shaderdrawable.h"
#include "core/gcode/parser/gcodeviewparser.h"

class BoundingBoxDrawer : public QObject, public ShaderDrawable
{
    Q_OBJECT

    public:
        explicit BoundingBoxDrawer();
        bool updateData(GLPalette &palette) override;
        void setViewParser(GCodeViewParser* viewParser);
        void setZoom(double zoom);

    private:
        double m_scale = 1.0;
        GCodeViewParser *m_viewParser = nullptr;
        QVector3D minimumExtremes() override;
        QVector3D maximumExtremes() override;
};

#endif // BOUNDINGBOXDRAWER_H
