#include "billboarddrawable.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

BillboardDrawable::BillboardDrawable()
    : m_texture(nullptr)
    , m_atlasX(0)
    , m_atlasY(0)
    , m_atlasRowHeight(0)
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

void BillboardDrawable::addBillboard(const QVector3D &position, BillboardContentData* cdata, float pixelSize)
{
    m_billboards.append(BillboardData(position, cdata, pixelSize));
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

QRectF BillboardDrawable::addBillboardToAtlas(const BillboardData& data)
{
    QString cacheKey = buildCacheKey(data.contentData.data());
    if (m_textCache.contains(cacheKey)) {
        return m_textCache[cacheKey];
    }

    QSize size = measureBillboard(data.contentData.data());

    // Check if we need to move to next row
    if (m_atlasX + size.width() > m_atlasImage.width()) {
        m_atlasX = 0;
        m_atlasY += m_atlasRowHeight;
        m_atlasRowHeight = 0;
    }

    // Check if we have space
    if (m_atlasY + size.height() > m_atlasImage.height()) {
        qWarning() << "Billboard atlas full!";
        return QRectF(0, 0, 0, 0);
    }

    // Draw billboard to atlas using virtual method
    QPainter painter(&m_atlasImage);
    QRect rect(m_atlasX, m_atlasY, size.width(), size.height());
    drawBillboard(painter, rect, data.contentData.data());
    painter.end();

    // Store texture coordinates (normalized) - width/height in texRect are normalized but we need pixel ratio
    // Store actual pixel dimensions in width/height for correct aspect ratio calculation
    QRectF texRect(
        (float)m_atlasX / m_atlasImage.width(),
        (float)m_atlasY / m_atlasImage.height(),
        (float)size.width(),  // Store actual pixel width
        (float)size.height()  // Store actual pixel height
    );

    m_textCache[cacheKey] = texRect;

    m_atlasX += size.width();
    m_atlasRowHeight = qMax(m_atlasRowHeight, size.height());

    return texRect;
}

void BillboardDrawable::addBillboardGeometry(const BillboardData& billboard, const QRectF& texRect)
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
        QVector2D(normalizedLeft, normalizedBottom)));

    m_billboardVertices.append(BillboardVertex(
        billboard.position, size, QVector2D(1.0, 0.0),
        QVector2D(normalizedRight, normalizedBottom)));

    m_billboardVertices.append(BillboardVertex(
        billboard.position, size, QVector2D(1.0, 1.0),
        QVector2D(normalizedRight, normalizedTop)));

    m_billboardVertices.append(BillboardVertex(
        billboard.position, size, QVector2D(0.0, 1.0),
        QVector2D(normalizedLeft, normalizedTop)));
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

    // Build vertex data
    m_billboardVertices.clear();
    m_billboardVertices.reserve(m_billboards.size() * 4);

    for (const BillboardData &billboard : m_billboards) {
        QRectF texRect = addBillboardToAtlas(billboard);

        if (texRect.width() == 0) continue; // Skip if atlas is full

        // Add geometry for this billboard
        addBillboardGeometry(billboard, texRect);
    }

    // Update texture
    if (m_texture) {
        delete m_texture;
    }

    m_texture = new QOpenGLTexture(m_atlasImage);
    m_texture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture->setMagnificationFilter(QOpenGLTexture::Nearest);
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

    shaderProgram->setUniformValue("u_scaleWithDistance", m_scaleWithDistance ? 1 : 0);
    shaderProgram->setUniformValue("u_globalScale", m_globalScale);

    if (!m_vao.isCreated() || !m_vbo.isCreated()) {
        init();
    }

    if (!m_indexBuffer.isCreated()) {
        m_indexBuffer.create();
    }

    m_vao.bind();
    m_vbo.bind();

    m_vbo.allocate(m_billboardVertices.constData(),
                   m_billboardVertices.count() * sizeof(BillboardVertex));

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

    if (m_depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);

    m_indexBuffer.release();
    m_texture->release();
    m_vbo.release();
    m_vao.release();

    // Restore texture unit 0 for other drawables!
    glActiveTexture(GL_TEXTURE0);
}
