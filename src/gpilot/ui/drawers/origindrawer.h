#ifndef ORIGINDRAWER_H
#define ORIGINDRAWER_H

#include "shaderdrawable.h"

class OriginDrawer : public ShaderDrawable
{
public:
    OriginDrawer();
    
    void setZoom(double);

protected:
    bool updateData(GLPalette &palette) override;

private:
    double m_scale;
};

#endif // ORIGINDRAWER_H
