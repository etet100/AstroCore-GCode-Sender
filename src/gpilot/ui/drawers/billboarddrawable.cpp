#include "billboarddrawable.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

BillboardDrawable::BillboardDrawable()
    : m_texture(nullptr)
    , m_atlasX(0)
    , m_atlasY(0)
    , m_atlasRowHeight(0)
{
    m_indexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
}

BillboardDrawable::~BillboardDrawable()
{
    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }
    if (m_indexBuffer.isCreated()) {
        m_indexBuffer.destroy();
    }
}

void BillboardDrawable::addBillboard(const QVector3D &position, const QString &text, const QColor &color, float pixelSize)
{
    m_billboards.append(BillboardData(position, text, color, pixelSize));
    m_needsUpdateGeometry = true;
}

void BillboardDrawable::clearBillboards()
{
    m_billboards.clear();
    m_billboardVertices.clear();
    m_textCache.clear();
    m_atlasX = 0;
    m_atlasY = 0;
    m_atlasRowHeight = 0;
}

void BillboardDrawable::init()
{
    // Initialize OpenGL functions (required!)
    initializeOpenGLFunctions();

    // Create VAO and VBO for billboard rendering
    if (!m_vao.isCreated()) m_vao.create();
    if (!m_vbo.isCreated()) m_vbo.create();

    m_vao.bind();

    m_vao.release();
    m_vbo.release();
}

QRectF BillboardDrawable::addTextToAtlas(const QString &text, const QFont &font)
{
    if (m_textCache.contains(text)) {
        return m_textCache[text];
    }

    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(text) + 4;
    int textHeight = fm.height() + 4;

    // Check if we need to move to next row
    if (m_atlasX + textWidth > m_atlasImage.width()) {
        m_atlasX = 0;
        m_atlasY += m_atlasRowHeight;
        m_atlasRowHeight = 0;
    }

    // Check if we have space
    if (m_atlasY + textHeight > m_atlasImage.height()) {
        qWarning() << "Billboard atlas full!";
        return QRectF(0, 0, 0, 0);
    }

    // Draw text to atlas
    QPainter painter(&m_atlasImage);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(font);

    // Draw semi-transparent background
    painter.fillRect(m_atlasX, m_atlasY, textWidth, textHeight, QColor(0, 0, 0, 180));

    // Draw text
    painter.setPen(Qt::white);
    painter.drawText(m_atlasX + 2, m_atlasY + 2 + fm.ascent(), text);
    painter.end();

    // Store texture coordinates (normalized)
    QRectF texRect(
        (float)m_atlasX / m_atlasImage.width(),
        (float)m_atlasY / m_atlasImage.height(),
        (float)textWidth / m_atlasImage.width(),
        (float)textHeight / m_atlasImage.height()
    );

    m_textCache[text] = texRect;

    m_atlasX += textWidth;
    m_atlasRowHeight = qMax(m_atlasRowHeight, textHeight);

    return texRect;
}

void BillboardDrawable::rebuildAtlas(GLPalette &palette)
{
    // Create atlas image
    const int atlasSize = 1024;
    m_atlasImage = QImage(atlasSize, atlasSize, QImage::Format_RGBA8888);
    m_atlasImage.fill(Qt::transparent);

    m_textCache.clear();
    m_atlasX = 0;
    m_atlasY = 0;
    m_atlasRowHeight = 0;

    QFont font("Arial", 8);

    // Build vertex data
    m_billboardVertices.clear();
    m_billboardVertices.reserve(m_billboards.size() * 4);

    for (const BillboardData &billboard : m_billboards) {
        QRectF texRect = addTextToAtlas(billboard.text, font);

        if (texRect.width() == 0) continue; // Skip if atlas is full

        GLuint color = palette.color(billboard.color);
        // Use desired pixel size instead of texture size
        float aspectRatio = texRect.width() / texRect.height();
        QVector2D size(billboard.pixelSize * aspectRatio, billboard.pixelSize);

        if (m_billboardVertices.isEmpty()) {
            qDebug() << "[BillboardDrawable] First billboard:"
                     << "text=" << billboard.text
                     << "texRect=" << texRect
                     << "texRect in pixels: (" << (texRect.x() * 1024) << "," << (texRect.y() * 1024)
                     << texRect.width() * 1024 << "x" << texRect.height() * 1024 << ")"
                     << "size=" << size;
        }

        if (m_billboardVertices.isEmpty()) {
            qDebug() << "[BillboardDrawable] First billboard: pos=" << billboard.position
                     << "size=" << size << "texRect=" << texRect
                     << "aspectRatio=" << aspectRatio
                     << "pixelSize=" << billboard.pixelSize;
        }

        // Four corners of billboard quad with texture coordinates
        // Swap top/bottom because OpenGL Y goes bottom-to-top, Qt Y goes top-to-bottom
        m_billboardVertices.append(BillboardVertex(
            billboard.position, size, QVector2D(0.0, 0.0),
            QVector2D(texRect.left(), texRect.bottom()), color));

        m_billboardVertices.append(BillboardVertex(
            billboard.position, size, QVector2D(1.0, 0.0),
            QVector2D(texRect.right(), texRect.bottom()), color));

        m_billboardVertices.append(BillboardVertex(
            billboard.position, size, QVector2D(1.0, 1.0),
            QVector2D(texRect.right(), texRect.top()), color));

        m_billboardVertices.append(BillboardVertex(
            billboard.position, size, QVector2D(0.0, 1.0),
            QVector2D(texRect.left(), texRect.top()), color));
            
        if (m_billboards.size() == 1) {
            qDebug() << "[BillboardDrawable] First billboard texCoords:"
                     << "TL:" << QVector2D(texRect.left(), texRect.top())
                     << "TR:" << QVector2D(texRect.right(), texRect.top())
                     << "BR:" << QVector2D(texRect.right(), texRect.bottom())
                     << "BL:" << QVector2D(texRect.left(), texRect.bottom());
        }
    }

    // Update texture
    if (m_texture) {
        delete m_texture;
    }
    // Don't mirror - Qt and OpenGL have same Y coordinate system for textures
    m_texture = new QOpenGLTexture(m_atlasImage);
    m_texture->setMinificationFilter(QOpenGLTexture::Linear);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // DEBUG: Save atlas to file to inspect
    static bool saved = false;
    if (!saved) {
        m_atlasImage.save("billboard_atlas_debug.png");
        qDebug() << "[BillboardDrawable] Saved atlas to billboard_atlas_debug.png";
        saved = true;
    }
}

bool BillboardDrawable::updateData(GLPalette &palette)
{
    if (m_billboards.isEmpty()) {
        return false;
    }

    rebuildAtlas(palette);
    return true;
}

void BillboardDrawable::updateGeometry(QOpenGLShaderProgram *shaderProgram, GLPalette &palette)
{
    // BillboardDrawable handles its own geometry updates in draw()
    // Don't call base class version which uses bindAttributes()
    if (updateData(palette)) {
        m_needsUpdateGeometry = false;
    }
}

void BillboardDrawable::draw(QOpenGLShaderProgram *shaderProgram)
{
    if (!m_visible || m_billboardVertices.isEmpty() || !m_texture) {
        return;
    }

    // Initialize buffers if needed
    if (!m_vao.isCreated() || !m_vbo.isCreated()) {
        init();
    }

    if (!m_indexBuffer.isCreated()) {
        m_indexBuffer.create();
    }

    m_vao.bind();
    m_vbo.bind();

    // Upload vertex data
    m_vbo.allocate(m_billboardVertices.constData(),
                   m_billboardVertices.count() * sizeof(BillboardVertex));

    // Bind attributes
    quintptr offset = 0;
    int pos;

    pos = shaderProgram->attributeLocation("a_position");
    if (pos >= 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 3, sizeof(BillboardVertex));
    }
    offset += sizeof(QVector3D);

    pos = shaderProgram->attributeLocation("a_billboardSize");
    if (pos >= 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 2, sizeof(BillboardVertex));
    }
    offset += sizeof(QVector2D);

    pos = shaderProgram->attributeLocation("a_corner");
    if (pos >= 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 2, sizeof(BillboardVertex));
    }
    offset += sizeof(QVector2D);

    pos = shaderProgram->attributeLocation("a_texCoord");
    if (pos >= 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 2, sizeof(BillboardVertex));
    }
    offset += sizeof(QVector2D);

    pos = shaderProgram->attributeLocation("a_color");
    if (pos >= 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 1, sizeof(BillboardVertex));
    }

    // Bind texture
    glActiveTexture(GL_TEXTURE1);
    m_texture->bind();

    // Create index buffer
    QVector<GLushort> indices;
    indices.reserve(m_billboards.size() * 6);
    for (int i = 0; i < m_billboards.size(); ++i) {
        GLushort base = i * 4;
        indices.append(base + 0);
        indices.append(base + 1);
        indices.append(base + 2);
        indices.append(base + 0);
        indices.append(base + 2);
        indices.append(base + 3);
    }

    m_indexBuffer.bind();
    m_indexBuffer.allocate(indices.constData(), indices.size() * sizeof(GLushort));

    // Draw without depth test temporarily for debugging
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Also draw triangles
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);

    // Cleanup
    m_indexBuffer.release();
    m_texture->release();
    m_vbo.release();
    m_vao.release();

    // Restore texture unit 0 for other drawables
    glActiveTexture(GL_TEXTURE0);
}
