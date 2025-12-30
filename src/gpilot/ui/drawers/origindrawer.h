#ifndef ORIGINDRAWER_H
#define ORIGINDRAWER_H

#include "shaderdrawable.h"
#include "originbillboarddrawer.h"

class OriginDrawer : public ShaderDrawable
{
public:
    OriginDrawer();

    void setZoom(double);

    OriginBillboardDrawer &billboardDrawable() { return m_billboardDrawable; }

protected:
    bool updateData(GLPalette &palette) override;

private:
    double m_scale;
    OriginBillboardDrawer m_billboardDrawable;
};

#endif // ORIGINDRAWER_H
