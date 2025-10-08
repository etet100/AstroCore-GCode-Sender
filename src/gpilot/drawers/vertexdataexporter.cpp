#include "vertexdataexporter.h"
#include <QFile>

VertexDataExporter::VertexDataExporter() {
}

void VertexDataExporter::exportToJsFile(const QString &fileName, const QVector<VertexData> &data)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream out(&file);

    out << "var vertexData = [\n";

    for (auto &vertex : data) {
        out << QString("%1, %2, %3, %4, %5, %6, %7,\n")
            .arg(vertex.position.x())
            .arg(vertex.position.y())
            .arg(vertex.position.z())
            .arg(vertex.color)
            .arg(vertex.start.x())
            .arg(vertex.start.y())
            .arg(vertex.start.z());
    }

    out << "];\n";

    file.close();
}
