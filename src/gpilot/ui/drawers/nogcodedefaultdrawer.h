#ifndef NOGCODEDEFAULTDRAWER_H
#define NOGCODEDEFAULTDRAWER_H

#include "shaderdrawable.h"

class NoGcodeDefaultDrawer : public ShaderDrawable
{
    public:
        NoGcodeDefaultDrawer();

    protected:
        bool updateData(GLPalette &palette) override;
        QVector3D maximumExtremes() override;
};

#endif // NOGCODEDEFAULTDRAWER_H
