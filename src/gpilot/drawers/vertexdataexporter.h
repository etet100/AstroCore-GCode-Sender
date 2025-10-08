#ifndef VERTEXDATAEXPORTER_H
#define VERTEXDATAEXPORTER_H

#include "shaderdrawable.h"

class VertexDataExporter
{
public:
    VertexDataExporter();

    static void exportToJsFile(const QString &fileName, const QVector<VertexData> &data);
};

#endif // VERTEXDATAEXPORTER_H
