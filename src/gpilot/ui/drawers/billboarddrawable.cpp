#include "billboarddrawable.h"
#include "heightmapgriddrawer.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>
#include <QtMath>
#include <algorithm>

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
        qWarning() << "[Billboard] Atlas full";

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
    // Use pixelSize as height - all billboards with same pixelSize have same height
    float scale = billboard.pixelSize / pixelHeight;
    QVector2D size(pixelWidth * scale, pixelHeight * scale);

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
    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }

    static int atlasSize = 256; // Start with smallest size

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

        if (texRect.width() == 0) {
            // Jeśli atlas jest pełny to spróbujemy go powiększyć i zacząć od nowa
            atlasSize *= 2;
            if (atlasSize < 4096) {
                qWarning() << "[Billboard] Rebuilding billboard atlas with size" << atlasSize;
                rebuildAtlas(palette);
            } else {
                qWarning() << "[Billboard] Billboard atlas exceeded maximum size!";
            }

            return; // Exit if atlas is full, texture will be invalid
        }

        // Add geometry for this billboard
        addBillboardGeometry(billboard, texRect);
    }

    // Let's see how much of the atlas was used, to allow dynamic adjustment of atlas size in the future
    // If less than 25% is used, we decrease the atlas size
    float usedArea = (m_atlasY + m_atlasRowHeight) * atlasSize;
    float totalArea = atlasSize * atlasSize;
    float usageRatio = usedArea / totalArea;
    qDebug() << "[Billboard] Billboard atlas usage:" << usageRatio * 100.0f << "%" << "; size is " << atlasSize;
    if (usageRatio < 0.25f && atlasSize > 256) {
        // Next time, use smaller atlas
        atlasSize /= 2;
        qDebug() << "[Billboard] Decreasing billboard atlas size to" << atlasSize;
    }

    // Notify derived classes that atlas is ready, may be used to generate mipmaps or other processing
    atlasReady(m_atlasImage);

    m_texture = new QOpenGLTexture(m_atlasImage);
    m_texture->setMinificationFilter(QOpenGLTexture::Linear);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
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
    shaderProgram->setUniformValue("u_billboardTexture", 1);

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
    } else {
        qWarning() << "[BillboardDrawable] Shader attribute a_position not found!";
        assert(false);
    }
    offset += sizeof(QVector3D);

    pos = shaderProgram->attributeLocation("a_billboardSize");
    if (pos >= 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 2, sizeof(BillboardVertex));
    } else {
        qWarning() << "[BillboardDrawable] Shader attribute a_billboardSize not found!";
        assert(false);
    }
    offset += sizeof(QVector2D);

    pos = shaderProgram->attributeLocation("a_corner");
    if (pos >= 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 2, sizeof(BillboardVertex));
    } else {
        qWarning() << "[BillboardDrawable] Shader attribute a_corner not found!";
        assert(false);
    }
    offset += sizeof(QVector2D);

    pos = shaderProgram->attributeLocation("a_texCoord");
    if (pos >= 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 2, sizeof(BillboardVertex));
    } else {
        qWarning() << "[BillboardDrawable] Shader attribute a_texCoord not found!";
        assert(false);
    }
    offset += sizeof(QVector2D);

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

void BillboardDrawable::updateScreenPositions(const QMatrix4x4& viewMatrix, const QMatrix4x4& projectionMatrix, const QSize& viewportSize, bool isOrthographic)
{
    m_screenPositions.clear();
    m_screenPositions.reserve(m_billboards.size());

    QMatrix4x4 mvp = projectionMatrix * viewMatrix;

    for (auto& billboard : m_billboards) {
    // for (int i = 0; i < m_billboards.size(); ++i) {
        // const BillboardData& billboard = m_billboards[i];

        // Transform 3D position to clip space
        QVector4D clipPos = mvp * QVector4D(billboard.position, 1.0f);

        if (clipPos.w() <= 0.0f) {
            // Behind camera or outside clip space
            continue;
        }

        // Calculate screen size by simulating shader behavior
        // Get billboard size in pixels
        QString cacheKey = buildCacheKey(billboard.contentData.data());
        QRectF texRect = m_textCache.value(cacheKey);

        float pixelWidth = texRect.width();
        float pixelHeight = texRect.height();
        float maxDimension = qMax(pixelWidth, pixelHeight);
        float scale = billboard.pixelSize / maxDimension;

        QVector2D billboardSize(pixelWidth * scale, pixelHeight * scale);

        // Perspective divide
        QVector3D ndcCenter = clipPos.toVector3D() / clipPos.w();

        // Skip billboards outside NDC range [-1, 1]
        if (qAbs(ndcCenter.x()) > 1.0f || qAbs(ndcCenter.y()) > 1.0f || qAbs(ndcCenter.z()) > 1.0f) {
            continue;
        }

        // Convert center to screen coordinates
        QVector2D screenCenter(
            (ndcCenter.x() * 0.5f + 0.5f) * viewportSize.width(),
            (1.0f - (ndcCenter.y() * 0.5f + 0.5f)) * viewportSize.height()
        );

        // Calculate screen size using the same shader logic
        QVector2D halfSize = billboardSize * 0.5f * m_globalScale;

        // Apply shader multipliers - must match shader values exactly!
        float shaderMultiplier;
        if (isOrthographic) {
            shaderMultiplier = 0.004f;  // Match shader: offset * 0.004
        } else {
            if (m_scaleWithDistance) {
                shaderMultiplier = 0.9f;  // Match shader: offset * 0.9
            } else {
                // Constant screen size: multiply by w
                shaderMultiplier = 0.005f * clipPos.w();  // Match shader: offset * 0.005 * w
            }
        }

        // Calculate screen-space size
        // The shader adds offset in CLIP space, then GPU does perspective divide
        // So: clipPos.xy += offset * multiplier / aspectRatio
        // After divide by w: ndcPos = clipPos.xy / clipPos.w()
        float aspectRatio = viewportSize.width() / (float)viewportSize.height();

        // Clip space offset (before perspective divide)
        float clipOffsetX = halfSize.x() * shaderMultiplier / aspectRatio;
        float clipOffsetY = halfSize.y() * shaderMultiplier;

        // After perspective divide, NDC offset is: clipOffset / w
        float ndcOffsetX = clipOffsetX / clipPos.w();
        float ndcOffsetY = clipOffsetY / clipPos.w();

        // Full size is 2x offset (from -offset to +offset)
        // NDC to screen: size in NDC * viewport dimension / 2 (because NDC is [-1,1] = range of 2)
        QVector2D screenSize(
            qAbs(ndcOffsetX * 2.0f) * viewportSize.width() * 0.5f,
            qAbs(ndcOffsetY * 2.0f) * viewportSize.height() * 0.5f
        );

        BillboardScreenPosition screenData;
        screenData.screenPos = screenCenter;
        screenData.screenSize = screenSize;
        screenData.contentData = billboard.contentData;
        screenData.zDepth = ndcCenter.z();

        m_screenPositions.append(screenData);
    }

    // No need to sort - we'll find the closest hit in hitTest
}

BillboardContentData* BillboardDrawable::hitTest(const QPoint& screenPos) const
{
    // Find billboard closest to camera (smallest depth) among all hits
    BillboardContentData* closestHit = nullptr;
    float closestDepth = 1.0f;
    int hitCount = 0;

    if (m_debugBounds) {
        qDebug() << "[Billboard] HIT TEST at" << screenPos;
    }

    for (int i = 0; i < m_screenPositions.size(); ++i) {
        const BillboardScreenPosition& pos = m_screenPositions[i];

        // Use calculated size directly without correction
        float hitWidth = pos.screenSize.x();
        float hitHeight = pos.screenSize.y();

        QRectF bounds(
            pos.screenPos.x() - hitWidth * 0.5f,
            pos.screenPos.y() - hitHeight * 0.5f,
            hitWidth,
            hitHeight
        );

        if (bounds.contains(screenPos)) {
            hitCount++;

            if (m_debugBounds) {
                const HeightMapGridBillboardContentData* hmData =
                    dynamic_cast<const HeightMapGridBillboardContentData*>(pos.contentData.data());
                QString gridInfo;
                if (hmData) {
                    gridInfo = QString(" grid(%1,%2)").arg(hmData->pos.x()).arg(hmData->pos.y());
                }
                qDebug() << "[Billboard] Hit" << hitCount << gridInfo << ":"
                         << "center=" << pos.screenPos
                         << "size=" << pos.screenSize
                         << "bounds=" << bounds
                         << "depth=" << pos.zDepth;
            }

            if (pos.zDepth < closestDepth) {
                closestDepth = pos.zDepth;
                closestHit = pos.contentData.data();
            }
        }
    }

    if (m_debugBounds && hitCount > 0) {
        qDebug() << "[Billboard] Selected closest with depth" << closestDepth << "from" << hitCount << "hits";
    }

    return closestHit;
}
