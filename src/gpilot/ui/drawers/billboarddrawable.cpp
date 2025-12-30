#include "billboarddrawable.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

BillboardDrawable::BillboardDrawable()
    : m_texture(nullptr)
    , m_atlasX(0)
    , m_atlasY(0)
    , m_atlasRowHeight(0)
    , m_scaleWithDistance(false)  // Default: constant screen size
    , m_globalScale(1.0f)          // Default: no additional scaling
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

QRectF BillboardDrawable::addTextToAtlas(const QString &text, const QColor &textColor, const QFont &font)
{
    // Create cache key from text and color
    QString cacheKey = text + "_" + textColor.name();

    if (m_textCache.contains(cacheKey)) {
        return m_textCache[cacheKey];
    }

    // Split text into two lines
    QStringList lines = text.split('\n');
    if (lines.isEmpty()) {
        return QRectF(0, 0, 0, 0);
    }

    // Two fonts: smaller for coordinates, larger for value
    QFont smallFont;
    smallFont.setPointSize(20);
    QFont largeFont;
    largeFont.setPointSize(28);

    QFontMetrics fmSmall(smallFont);
    QFontMetrics fmLarge(largeFont);

    // Calculate dimensions
    int maxWidth = 0;
    int totalHeight = 0;

    if (lines.size() > 0) {
        maxWidth = qMax(maxWidth, fmSmall.horizontalAdvance(lines[0]));
        totalHeight += fmSmall.height();
    }
    if (lines.size() > 1) {
        maxWidth = qMax(maxWidth, fmLarge.horizontalAdvance(lines[1]));
        totalHeight += fmLarge.height();
    }

    int textWidth = maxWidth + 8;
    int textHeight = totalHeight + 8;

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

    // Draw billboard to atlas using virtual method
    QPainter painter(&m_atlasImage);
    QRect rect(m_atlasX, m_atlasY, textWidth, textHeight);
    drawBillboard(painter, rect, text, textColor);
    painter.end();

    // Store texture coordinates (normalized) - width/height in texRect are normalized but we need pixel ratio
    // Store actual pixel dimensions in width/height for correct aspect ratio calculation
    QRectF texRect(
        (float)m_atlasX / m_atlasImage.width(),
        (float)m_atlasY / m_atlasImage.height(),
        (float)textWidth,  // Store actual pixel width
        (float)textHeight  // Store actual pixel height
    );

    m_textCache[cacheKey] = texRect;

    m_atlasX += textWidth;
    m_atlasRowHeight = qMax(m_atlasRowHeight, textHeight);

    return texRect;
}

void BillboardDrawable::addBillboardGeometry(const BillboardData &billboard, const QRectF &texRect, GLuint color)
{
    // texRect contains: x,y = normalized coords, width/height = pixel dimensions
    float pixelWidth = texRect.width();
    float pixelHeight = texRect.height();

    // Scale billboard to maintain text proportions while being reasonably sized
    // Use pixelSize as a base scale factor for the larger dimension
    float maxDimension = qMax(pixelWidth, pixelHeight);
    float scale = billboard.pixelSize / maxDimension;
    QVector2D size(pixelWidth * scale, pixelHeight * scale * 2.5f);  // 2.5x taller to fix aspect ratio

    // Normalize texRect coordinates for texture sampling
    float normalizedLeft = texRect.x();
    float normalizedTop = texRect.y();
    float normalizedWidth = pixelWidth / m_atlasImage.width();
    float normalizedHeight = pixelHeight / m_atlasImage.height();
    float normalizedRight = normalizedLeft + normalizedWidth;
    float normalizedBottom = normalizedTop + normalizedHeight;

    // Four corners of billboard quad with texture coordinates
    // Swap top/bottom because OpenGL Y goes bottom-to-top, Qt Y goes top-to-bottom
    m_billboardVertices.append(BillboardVertex(
        billboard.position, size, QVector2D(0.0, 0.0),
        QVector2D(normalizedLeft, normalizedBottom), color));

    m_billboardVertices.append(BillboardVertex(
        billboard.position, size, QVector2D(1.0, 0.0),
        QVector2D(normalizedRight, normalizedBottom), color));

    m_billboardVertices.append(BillboardVertex(
        billboard.position, size, QVector2D(1.0, 1.0),
        QVector2D(normalizedRight, normalizedTop), color));

    m_billboardVertices.append(BillboardVertex(
        billboard.position, size, QVector2D(0.0, 1.0),
        QVector2D(normalizedLeft, normalizedTop), color));
}

void BillboardDrawable::drawBillboard(QPainter &painter, const QRect &rect, const QString &text, const QColor &textColor)
{
    // Split text into lines
    QStringList lines = text.split('\n');

    // Two fonts: smaller for coordinates, larger for value
    QFont smallFont;
    smallFont.setPointSize(20);
    QFont largeFont;
    largeFont.setPointSize(28);

    QFontMetrics fmSmall(smallFont);
    QFontMetrics fmLarge(largeFont);

    // Draw semi-transparent background
    painter.fillRect(rect, QColor(0, 0, 0, 180));

    // Draw text centered
    painter.setPen(Qt::white);
    int yPos = rect.y() + 4;

    if (lines.size() > 0) {
        painter.setFont(smallFont);
        int xPos = rect.x() + (rect.width() - fmSmall.horizontalAdvance(lines[0])) / 2;
        painter.drawText(xPos, yPos + fmSmall.ascent(), lines[0]);
        yPos += fmSmall.height();
    }

    if (lines.size() > 1) {
        painter.setFont(largeFont);
        int xPos = rect.x() + (rect.width() - fmLarge.horizontalAdvance(lines[1])) / 2;
        painter.drawText(xPos, yPos + fmLarge.ascent(), lines[1]);
    }
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

    QFont font("Arial", 16);

    // Build vertex data
    m_billboardVertices.clear();
    m_billboardVertices.reserve(m_billboards.size() * 4);

    for (const BillboardData &billboard : m_billboards) {
        QRectF texRect = addTextToAtlas(billboard.text, billboard.color, font);

        if (texRect.width() == 0) continue; // Skip if atlas is full

        GLuint color = palette.color(billboard.color);

        // Add geometry for this billboard
        addBillboardGeometry(billboard, texRect, color);

        if (m_billboardVertices.size() == 4) {
            qDebug() << "[BillboardDrawable] First billboard:"
                     << "text=" << billboard.text
                     << "position=" << billboard.position
                     << "pixelSize=" << billboard.pixelSize;
        }
    }

    // Update texture
    if (m_texture) {
        delete m_texture;
    }
    // Don't mirror - Qt and OpenGL have same Y coordinate system for textures
    m_texture = new QOpenGLTexture(m_atlasImage);
    m_texture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture->setMagnificationFilter(QOpenGLTexture::Nearest);

    // DEBUG: Save atlas to file to inspect
    m_atlasImage.save("billboard_atlas_debug.png");
    qDebug() << "[BillboardDrawable] Saved atlas to billboard_atlas_debug.png";
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

    // Set billboard-specific uniforms
    shaderProgram->setUniformValue("u_scaleWithDistance", m_scaleWithDistance ? 1 : 0);
    shaderProgram->setUniformValue("u_globalScale", m_globalScale);

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

    // Enable depth test and blending
    glEnable(GL_DEPTH_TEST);
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
