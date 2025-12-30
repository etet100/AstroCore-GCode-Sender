#ifndef ORIGINDRAWER_H
#define ORIGINDRAWER_H

#include "shaderdrawable.h"
#include "originbillboarddrawable.h"

class OriginDrawer : public ShaderDrawable
{
public:
    OriginDrawer();

    void setZoom(double);

    OriginBillboardDrawable &billboardDrawable() { return m_billboardDrawable; }

protected:
    bool updateData(GLPalette &palette) override;

private:
    double m_scale;
    OriginBillboardDrawable m_billboardDrawable;
};

#endif // ORIGINDRAWER_H
