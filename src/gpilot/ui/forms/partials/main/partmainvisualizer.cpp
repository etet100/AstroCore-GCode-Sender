#include "partmainvisualizer.h"
#include "ui_partmainvisualizer.h"
#include "ui/config/configurationvisualizer.h"
#include "core/config/module/configurationmachine.h"
#include "core/gcode/gcode.h"
#include "ui/drawers/vertexdataexporter.h"
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include "styledtoolbutton.h"
#include "ui/utils/thememanager.h"
#include "core/utils/programtimeestimator.h"

PartMainVisualizer::PartMainVisualizer(QWidget* parent) : QWidget(parent)
    , ui(new Ui::partMainVisualizer)
    , m_heightmapBorderDrawer()
    , m_heightmapGridDrawer()
    , m_ignoreZ(false)
    , m_lastDrawnLineIndex(0)
{
    ui->setupUi(this);

    m_codeDrawer = new GcodeDrawer();

    m_toolDrawer.setVisible(ui->cmdToggleTool->isChecked());
    m_originDrawer.setVisible(ui->cmdToggleOrigin->isChecked());
    m_boundingBoxDrawer.setVisible(ui->cmdToggleBoundingBox->isChecked());
    m_tableSurfaceDrawer.setVisible(ui->cmdToggleGrid->isChecked());
    m_codeDrawer->setVisible(ui->cmdToggleToolpath->isChecked());
    m_codeDrawer->setHeightmapPreview(ui->cmdToggleHeightmapPreview->isChecked());
    m_heightmapGridDrawer.setVisible(ui->cmdToggleHeightmap->isChecked());
    m_heightmapGridDrawer.billboardDrawable()->setVisible(ui->cmdToggleHeightmapMarkers->isChecked());
    m_lightSourceDrawer.setVisible(ui->cmdToggleLight->isChecked());
    ui->visualizer->setRotationCubeVisible(ui->cmdToggleCube->isChecked());
    ui->visualizer->setLightEnabled(ui->cmdToggleLight->isChecked());

    connect(ui->visualizer, &GLContainer::cursorPosChanged, this, &PartMainVisualizer::updateCursorDrawer);

    // Handle heightmap billboards interaction - mouse moved, double click
    connect(ui->visualizer, &GLContainer::viewParametersChanged, this, &PartMainVisualizer::updateBillboardsScreenPositions);
    connect(ui->visualizer, &GLContainer::mouseMoved, this, [this](QPoint pos) {
        if (!m_heightmapGridDrawer.visible()) {
            return;
        }
        HeightMapGridBillboardContentData* cd = static_cast<HeightMapGridBillboardContentData*>(
            m_heightmapGridDrawer.billboardDrawable()->hitTest(pos)
        );
        if (cd != nullptr) {
            showInfoBar(QString("Height at %1, %2 = %3, dbl click to edit")
                .arg(cd->pos.x())
                .arg(cd->pos.y())
                .arg(cd->height, 0, 'f', 2)
            );
        } else {
            hideInfoBar();
        }
    });
    connect(ui->visualizer, &GLContainer::mouseDoubleClicked, this, [this](QPoint pos) {
        // Ignore clicks if heightmap markers are not visible
        if (m_heightmapGridDrawer.billboardDrawable()->visible()) {
            HeightMapGridBillboardContentData* contentData = static_cast<HeightMapGridBillboardContentData*>(
                m_heightmapGridDrawer.billboardDrawable()->hitTest(pos)
            );
            if (contentData != nullptr && m_lastHMGBContentData != contentData) {
                emit editHeightmapPoint(contentData->pos);
            } else if (contentData == nullptr) {
                hideInfoBar();
                m_lastHMGBContentData = nullptr;
            }
        }
    });
    //

    connect(ui->visualizer, &GLContainer::entered, this, [this]() {
        m_cursorDrawer.setVisible(true);
    });
    connect(ui->visualizer, &GLContainer::left, this, [this]() {
        m_cursorDrawer.setVisible(false);
        hideInfoBar();
        m_lastHMGBContentData = nullptr;
    });
    connect(ui->visualizer, &GLContainer::zoomChanged, this, [this](double zoom) {
        m_originDrawer.setZoom(zoom);
        m_boundingBoxDrawer.setZoom(zoom);
    });
    connect(ui->visualizer, &GLContainer::goToCursor, this, [this](QPointF pos) {
        // Do not go to cursor if heightmap markers are visible
        if (!m_heightmapGridDrawer.billboardDrawable()->visible()) {
            emit goToCursor(pos);
        }
    });
    connect(ui->visualizer, &GLContainer::viewModeChanged, this, [this](GLWidget::ViewMode mode) {
        emit viewModeChanged(mode);
    });

    connect(&ThemeManager::instance(), &ThemeManager::scaleChanged, [this]() {
        m_originDrawer.update();
        m_heightmapGridDrawer.update();
    });
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, [this]() {
        updateColors();
    });

    initializeButtons();
    initializeInfoBar();

    m_lightPosTimer.setInterval(100);
    connect(&m_lightPosTimer, &QTimer::timeout, this, [this]() {
        m_lightSourceDrawer.setPosition(ui->visualizer->lightPos());
    });
    m_lightPosTimer.start();
}

PartMainVisualizer::~PartMainVisualizer()
{
    delete m_infoAnimation;
    delete m_infoOpacityEffect;
    delete m_codeDrawer;
    delete ui;
}

void PartMainVisualizer::initializeInfoBar()
{
    ui->info->setParent(ui->visualizer);
    ui->info->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_infoOpacityEffect = new QGraphicsOpacityEffect(ui->lblInfo);
    m_infoOpacityEffect->setOpacity(0);
    ui->lblInfo->setGraphicsEffect(m_infoOpacityEffect);

    m_infoAnimation = new QPropertyAnimation(m_infoOpacityEffect, "opacity");
    m_infoAnimation->setDuration(300);
}

void PartMainVisualizer::initializeButtons()
{
    ui->buttons->setParent(ui->visualizer);

    for (auto& button : ui->buttons->findChildren<StyledToolButton*>(Qt::FindDirectChildrenOnly)) {
        connect(button, &StyledToolButton::hoverChanged, this, &PartMainVisualizer::showButtonInfo);
    }
}

void PartMainVisualizer::placeButtons()
{
    ui->buttons->move(width() - ui->buttons->width() - 8, 8);
}

void PartMainVisualizer::updateCursorDrawer(QPointF pos)
{
   m_cursorDrawer.setPosition(pos);
}

// Do not init drawables before the parser is set (setProgram)
void PartMainVisualizer::initDrawables()
{
    *ui->visualizer << &m_tableSurfaceDrawer
                    << m_codeDrawer
                    << &m_boundingBoxDrawer
                    << m_boundingBoxDrawer.billboardDrawable()
                    << &m_cursorDrawer
                    << &m_heightmapBorderDrawer
                    << m_heightmapBorderDrawer.billboardDrawable()
                    << &m_heightmapGridDrawer
                    << &m_heightmapInterpolationDrawer
                    << m_heightmapGridDrawer.billboardDrawable()
                    << &m_selectionDrawer
                    << &m_machineBoundsDrawer
                    << &m_toolDrawer
                    << &m_originDrawer
                    << &m_originDrawer.billboardDrawable()
                    << &m_noGcodeDefaultDrawer
                    << &m_lightSourceDrawer
    ;

    ui->visualizer->fitDrawable(&m_noGcodeDefaultDrawer);
}

void PartMainVisualizer::applyVisualizerConfiguration(
    ConfigurationVisualizer& visualizerConfiguration,
    ConfigurationMachine& machineConfiguration
) {
    m_ignoreZ = visualizerConfiguration.ignoreZ();

    ui->visualizer->setLineWidth(visualizerConfiguration.lineWidth());
    ui->visualizer->setAntialiasing(visualizerConfiguration.antialiasing());
    ui->visualizer->setMsaa(visualizerConfiguration.msaa());
    ui->visualizer->setZBuffer(visualizerConfiguration.zBuffer());
    ui->visualizer->setFov(visualizerConfiguration.fieldOfView());
    ui->visualizer->setNearPlane(visualizerConfiguration.nearPlane());
    ui->visualizer->setFarPlane(visualizerConfiguration.farPlane());
    ui->visualizer->setVsync(visualizerConfiguration.vsync());
    ui->visualizer->setFps(visualizerConfiguration.fpsLock());

    m_colors = visualizerConfiguration.colors();
    updateColors();

    m_cursorDrawer.setVisible(visualizerConfiguration.show3dCursor());

    applyCursorDrawerConfiguration(visualizerConfiguration);
    applyTableSurfaceDrawerConfiguration(visualizerConfiguration);
    applyHeightmapDrawerConfiguration(visualizerConfiguration);
    applyOriginDrawerConfiguration(visualizerConfiguration);
    applySelectionDrawerConfiguration(visualizerConfiguration);
    applyToolDrawerConfiguration(visualizerConfiguration);
    applyCodeDrawerConfiguration(visualizerConfiguration, machineConfiguration);

    switch (visualizerConfiguration.viewMode()) {
        case ConfigurationVisualizer::ViewMode::Perspective:
            ui->visualizer->setViewMode(GLWidget::ViewMode::Perspective);
            break;
        case ConfigurationVisualizer::ViewMode::Orthogonal:
            ui->visualizer->setViewMode(GLWidget::ViewMode::Orthogonal);
            break;
        case ConfigurationVisualizer::ViewMode::View2D:
            ui->visualizer->setViewMode(GLWidget::ViewMode::View2D);
            break;
        default:
            qWarning() << "[PartMainVisualizer] Unknown view mode in visualizer configuration" << visualizerConfiguration.viewMode();
            break;
    }
}

void PartMainVisualizer::updateGCodeExtremes()
{
    ui->visualizer->updateExtremes(m_codeDrawer);
}

void PartMainVisualizer::fitDrawable()
{
    ui->visualizer->fitDrawable(&m_noGcodeDefaultDrawer);
}

void PartMainVisualizer::fitCodeDrawer()
{
    ui->visualizer->fitDrawable(m_codeDrawer);
}

/*
 * program should not be null
 * parser will be nullptr if new file is selected - program is empty
 */
void PartMainVisualizer::setProgram(GCode* program, GCodeViewParser* parser)
{
    m_program = program;
    m_codeDrawer->setViewParser(parser);
    m_boundingBoxDrawer.setViewParser(parser);

    if (parser != nullptr) {
        QVector3D minEx = parser->getMinimumExtremes();
        QVector3D maxEx = parser->getMaximumExtremes();
        ui->visualizer->setLightCenter(QVector3D(
            (minEx.x() + maxEx.x()) / 2.0f,
            (minEx.y() + maxEx.y()) / 2.0f,
            maxEx.z()
        ));
    }

    updateDefaultDrawerVisibility();
}

void PartMainVisualizer::updateCodeDrawer(const QList<int>& indexes)
{
    m_codeDrawer->update(indexes);
    ui->visualizer->update();
}

void PartMainVisualizer::updateCodeDrawer()
{
    m_codeDrawer->update();
    ui->visualizer->update();
}

void PartMainVisualizer::setToolPosition(QVector3D pos)
{
    m_toolDrawer.setToolPosition(pos);
}

/*
 * Unload program and heightmap, reset drawers and visualizer state
 */
void PartMainVisualizer::close()
{
    m_heightmap = nullptr;
    m_program = nullptr;
    m_codeDrawer->setViewParser(nullptr);
    m_boundingBoxDrawer.setViewParser(nullptr);
    m_heightmapGridDrawer.setVisible(false);
    m_heightmapBorderDrawer.setVisible(false);
    m_heightmapInterpolationDrawer.setVisible(false);
    m_boundingBoxDrawer.setVisible(false);
    m_noGcodeDefaultDrawer.setVisible(true);
}

void PartMainVisualizer::setHeightmap(Heightmap& heightmap)
{
    m_heightmap = &heightmap;
    m_heightmapBorderDrawer.setModel(heightmap);
    m_heightmapGridDrawer.setModel(heightmap);
    m_codeDrawer->setHeightmapView(&heightmap, 1);
    connect(&heightmap, &Heightmap::changed, this, [this]() {
        updateHeightmap();
    });
}

void PartMainVisualizer::setSelectionEndPosition(QVector3D pos)
{
    m_selectionDrawer.setEndPosition(pos);
    m_selectionDrawer.update();
}

void PartMainVisualizer::updateSelection()
{
    m_selectionDrawer.update();
}

void PartMainVisualizer::placeInfoBar()
{
    ui->info->setGeometry(QRect(0, 8, width(), ui->info->height()));
}

void PartMainVisualizer::setHeightmapMode(bool enabled)
{
    m_heightmapInterpolationDrawer.setVisible(ui->visualizer->property("showInterpolation").toBool() && enabled);
    // m_heightmapBorderDrawer.setVisible(ui->visualizer->property("showBorder").toBool() && enabled);
    // m_heightmapGridDrawer.setVisible(ui->visualizer->property("showGrid").toBool() && enabled);

    m_selectionDrawer.setVisible(!enabled);
    updateDefaultDrawerVisibility();
}

void PartMainVisualizer::updateHeightmapGrid()
{
    m_heightmapGridDrawer.update();
}

void PartMainVisualizer::updateHeightmapInterpolation(bool reset)
{
    if (reset) m_heightmapInterpolationDrawer.setData(nullptr);
    else m_heightmapInterpolationDrawer.update();
}

void PartMainVisualizer::setInterpolationData(QVector<QVector<double>> *data, QRectF borderRect)
{
    m_heightmapInterpolationDrawer.setBorderRect(borderRect);
    m_heightmapInterpolationDrawer.setData(data);
}

void PartMainVisualizer::setHeightmapInterpolationVisible(bool visible)
{
    m_heightmapInterpolationDrawer.setVisible(visible);
    updateDefaultDrawerVisibility();
}

void PartMainVisualizer::setSelectionVisible(bool visible)
{
    m_selectionDrawer.setVisible(visible);
}

void PartMainVisualizer::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    placeButtons();
    placeInfoBar();
    updateBillboardsScreenPositions();
}

void PartMainVisualizer::topClicked()
{
    ui->visualizer->setTopView();
}

void PartMainVisualizer::frontClicked()
{
    ui->visualizer->setFrontView();
}

void PartMainVisualizer::leftClicked()
{
    ui->visualizer->setLeftView();
}

void PartMainVisualizer::rightClicked()
{
    ui->visualizer->setRightView();
}

void PartMainVisualizer::isometricClicked()
{
    ui->visualizer->setIsometricView();
}

void PartMainVisualizer::rotationCubeClicked(bool checked)
{
    ui->visualizer->setRotationCubeVisible(checked);
}

void PartMainVisualizer::heightmapClicked(bool checked)
{
    m_heightmapGridDrawer.setVisible(checked);
    if (checked) {
        updateBillboardsScreenPositions();
    }
    updateDefaultDrawerVisibility();
}

void PartMainVisualizer::heightmapMarkersClicked(bool checked)
{
    if (checked && !m_heightmapGridDrawer.visible()) {
        ui->cmdToggleHeightmap->setChecked(true);
    }
    m_heightmapGridDrawer.billboardDrawable()->setVisible(checked);
    if (checked) {
        updateBillboardsScreenPositions();
    }
}

void PartMainVisualizer::toggleProjectionClicked()
{
    ui->visualizer->toggleProjectionType();
}

void PartMainVisualizer::toggleOriginClicked(bool checked)
{
    m_originDrawer.setVisible(checked);
}

void PartMainVisualizer::fitClicked()
{
    if (m_codeDrawer->viewParser() != nullptr) {
        ui->visualizer->fitDrawable(m_codeDrawer);
    } else {
        ui->visualizer->fitDrawable(&m_noGcodeDefaultDrawer);
    }
}

void PartMainVisualizer::_2dClicked()
{
    ui->visualizer->setViewMode(GLWidget::ViewMode::View2D);
}

void PartMainVisualizer::showInfoBar(QString text)
{
    m_infoAnimation->stop();
    ui->lblInfo->setText(text);
    if (m_infoOpacityEffect->opacity() > 0.99) {
        return;
    }
    m_infoAnimation->setStartValue(m_infoOpacityEffect->opacity());
    m_infoAnimation->setEndValue(1.0);
    m_infoAnimation->start();
}

void PartMainVisualizer::hideInfoBar()
{
    m_infoAnimation->stop();
    if (m_infoOpacityEffect->opacity() < 0.01) {
        return;
    }
    m_infoAnimation->setStartValue(m_infoOpacityEffect->opacity());
    m_infoAnimation->setEndValue(0.0);
    m_infoAnimation->start();
}

void PartMainVisualizer::setTimeEstimation(ProgramTimeEstimator& estimator)
{
    ui->visualizer->setEstimatedTime(estimator.estimatedRemainingTimeWithCorrection());
    ui->visualizer->setSpendTime(estimator.elapsedTime());
}

void PartMainVisualizer::showButtonInfo(bool hovered)
{
    StyledToolButton* button = qobject_cast<StyledToolButton*>(sender());
    if (hovered) {
        showInfoBar(button->toolTip());
    } else {
        hideInfoBar();
    }
}

void PartMainVisualizer::setUpdatesEnabled2(bool updatesEnabled)
{
    ui->visualizer->setUpdatesEnabled(updatesEnabled);
}

void PartMainVisualizer::updateColors()
{
    const int LIGHTBOUND = 140;

    bool dark = ThemeManager::instance().dark();
    ConfigurationVisualizer::Colors colors = dark ? m_colors.dark : m_colors.light;

    QColor bgColor = colors.background;

    ui->visualizer->setColorBackground(bgColor);
    ui->visualizer->setColorText(bgColor.value() > LIGHTBOUND ? Qt::black : Qt::white);

    m_codeDrawer->setColorNormal(colors.normalToolpath);
    m_codeDrawer->setColorDrawn(colors.drawnToolpath);
    m_codeDrawer->setColorHighlight(colors.hightlightToolpath);
    m_codeDrawer->setColorZMovement(colors.zMovement);
    m_codeDrawer->setColorRapidMovement(colors.rapidMovement);
    m_codeDrawer->setColorStart(colors.startPoint);
    m_codeDrawer->setColorEnd(colors.endPoint);

    m_codeDrawer->update();

    m_toolDrawer.setColor(colors.tool);
    m_toolDrawer.update();

    m_cursorDrawer.setColor(colors.cursor);
    m_cursorDrawer.update();

    m_tableSurfaceDrawer.setGridColor(colors.tableSurfaceGrid);
    m_tableSurfaceDrawer.update();

    m_selectionDrawer.setColor(colors.hightlightToolpath);
}

void PartMainVisualizer::applyCodeDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration, ConfigurationMachine &machineConfiguration)
{
    m_codeDrawer->setLineWidth(visualizerConfiguration.lineWidth());
    m_codeDrawer->setSimplify(visualizerConfiguration.simplifyGeometry());
    m_codeDrawer->setSimplifyPrecision(visualizerConfiguration.simplifyGeometryPrecision());
    m_codeDrawer->setIgnoreZ(visualizerConfiguration.ignoreZ());
    m_codeDrawer->setGrayscaleSegments(visualizerConfiguration.grayscaleSegments());
    m_codeDrawer->setGrayscaleCode(visualizerConfiguration.grayscaleSegmentsBySCode() ? GcodeDrawer::S : GcodeDrawer::Z);
    m_codeDrawer->setGrayscaleMin(machineConfiguration.laserPowerRange().min);
    m_codeDrawer->setGrayscaleMax(machineConfiguration.laserPowerRange().max);
    m_codeDrawer->update();
}

void PartMainVisualizer::applyToolDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_toolDrawer.setToolDiameter(visualizerConfiguration.toolDiameter());
    m_toolDrawer.setToolLength(visualizerConfiguration.toolLength());
    m_toolDrawer.setLineWidth(visualizerConfiguration.lineWidth());
    m_toolDrawer.setMode(visualizerConfiguration.toolType());
    m_toolDrawer.setToolAngle(visualizerConfiguration.toolAngle());
    m_toolDrawer.update();
}

void PartMainVisualizer::applyCursorDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_cursorDrawer.setVisible(visualizerConfiguration.show3dCursor());
    m_cursorDrawer.update();
}

void PartMainVisualizer::applyTableSurfaceDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    Q_UNUSED(visualizerConfiguration)
}

void PartMainVisualizer::applyHeightmapDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_heightmapBorderDrawer.setLineWidth(visualizerConfiguration.lineWidth());
    m_heightmapBorderDrawer.setVisible(false);
    m_heightmapGridDrawer.setLineWidth(0.1);
    m_heightmapInterpolationDrawer.setLineWidth(visualizerConfiguration.lineWidth());
}

void PartMainVisualizer::applyOriginDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_originDrawer.setLineWidth(visualizerConfiguration.lineWidth());
}

void PartMainVisualizer::applySelectionDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    Q_UNUSED(visualizerConfiguration)
}

void PartMainVisualizer::setParserState(QString state)
{
    ui->visualizer->setParserState(state);
}

void PartMainVisualizer::setPinState(QString state)
{
    ui->visualizer->setPinState(state);
}

void PartMainVisualizer::setSpeedState(QString state)
{
    ui->visualizer->setSpeedState(state);
}

bool PartMainVisualizer::isIgnoreZ() const
{
    return m_ignoreZ;
}

void PartMainVisualizer::loadNewProgram()
{
    m_lastDrawnLineIndex = 0;
    m_codeDrawer->update();
    ui->visualizer->fitDrawable(m_codeDrawer);

    m_selectionDrawer.resetEndPosition();
    m_selectionDrawer.update();
}

void PartMainVisualizer::resetVisualization()
{
    m_lastDrawnLineIndex = 0;
    m_codeDrawer->update();
    ui->visualizer->fitDrawable();

    m_selectionDrawer.resetEndPosition();
    m_selectionDrawer.update();
}

void PartMainVisualizer::updateToolpathHighlighting(int currentRow, int previousRow)
{
    if (!m_program || m_program->empty() || m_codeDrawer == nullptr) {
        return;
    }

    int rowCurrent = qMin(currentRow, m_program->lastCommandIndex());
    int rowPrevious = qMax(qMin(previousRow, m_program->lastCommandIndex()), 0);

    qDebug() << "[PartMainVisualizer] Updating toolpath highlighting from row"
             << rowPrevious << "to" << rowCurrent;

    GCodeViewParser *parser = m_codeDrawer->viewParser();
    if (parser == nullptr) {
        return;
    }
    QList<LineSegment>& list = parser->getLineSegmentList();
    QVector<QList<int>>& lineIndexes = parser->getLinesIndexes();

    GCodeItem& currentItem = m_program->at(rowCurrent);
    GCodeItem& previousItem = m_program->at(rowPrevious);

    qDebug() << "[PartMainVisualizer] Is movment:" << currentItem.isMovement;

    // Update linesegments on cell changed
    if (!m_codeDrawer->geometryUpdated()) {
        int lineCurrent = currentItem.commandNumber;
        for (int i = 0; i < list.count(); i++) {
            list[i].setIsHightlight(list[i].getLineNumber() <= lineCurrent);
        }
    } else {
        // Update vertices on current cell changed
        int lineCurrent = currentItem.commandNumber;
        int linePrevious = previousItem.commandNumber;
        if (linePrevious < lineCurrent) qSwap(linePrevious, lineCurrent);

        QList<int> indexes;
        for (int i = lineCurrent + 1; i <= linePrevious; i++) {
            foreach (int l, lineIndexes.at(i)) {
                list[l].setIsHightlight(rowCurrent > rowPrevious);
                indexes.append(l);
            }
        }

        if (indexes.isEmpty()) {
            m_selectionDrawer.resetEndPosition();
        } else {
            QVector3D pos = list[indexes.first()].getEnd();
            if (m_ignoreZ) {
                pos.setZ(0);
            }
            m_selectionDrawer.setEndPosition(pos);
        }
        m_selectionDrawer.update();

        if (!indexes.isEmpty()) {
            m_codeDrawer->update(indexes);
        }
    }

    // Update selection marker
    int line = currentItem.commandNumber;
    if (line > 0 && line < lineIndexes.count() && !lineIndexes.at(line).isEmpty()) {
        QVector3D pos = list[lineIndexes.at(line).last()].getEnd();
        m_selectionDrawer.setEndPosition(m_ignoreZ ? QVector3D(pos.x(), pos.y(), 0) : pos);
    } else {
        m_selectionDrawer.resetEndPosition();
    }
    m_selectionDrawer.update();
}

void PartMainVisualizer::updateToolTracking(QVector3D toolPosition, int processedLineIndex)
{
    m_toolDrawer.setToolPosition(m_ignoreZ ? QVector3D(toolPosition.x(), toolPosition.y(), 0) : toolPosition);

    GCodeViewParser *parser = m_codeDrawer->viewParser();
    bool toolOntoolpath = false;

    QList<int> drawnLines;
    QList<LineSegment>& list = parser->getLineSegmentList();

    for (
        int i = m_lastDrawnLineIndex;
        i < list.count() && list[i].getLineNumber() <= (processedLineIndex + 1);
        i++
    ) {
        if (list[i].contains(toolPosition)) {
            toolOntoolpath = true;
            m_lastDrawnLineIndex = i;
            break;
        }
        drawnLines << i;
    }

    if (toolOntoolpath) {
        foreach (int i, drawnLines) {
            list[i].setDrawn(true);
        }
        if (!drawnLines.isEmpty()) {
            m_codeDrawer->update(drawnLines);
            ui->visualizer->update();
        }
    }
}

QRectF PartMainVisualizer::getCodeDrawerBounds() const
{
    QRectF rect;
    rect.setX(m_codeDrawer->minimumExtremes().x());
    rect.setY(m_codeDrawer->minimumExtremes().y());
    rect.setWidth(m_codeDrawer->sizes().x());
    rect.setHeight(m_codeDrawer->sizes().y());
    return rect;
}

void PartMainVisualizer::resetLastDrawnLine()
{
    m_lastDrawnLineIndex = 0;
}

void PartMainVisualizer::finalizeTransfer()
{
    // Shadow last segment
    GCodeViewParser *parser = m_codeDrawer->viewParser();
    QList<LineSegment>& list = parser->getLineSegmentList();

    if (m_lastDrawnLineIndex < list.count()) {
        list[m_lastDrawnLineIndex].setDrawn(true);
        m_codeDrawer->update(QList<int>() << m_lastDrawnLineIndex);
        ui->visualizer->update();
    }

    m_lastDrawnLineIndex = 0;
}

void PartMainVisualizer::exportCodeDrawerToFile(const QString& filename)
{
    VertexDataExporter::exportToJsFile(filename, m_codeDrawer->lines());
}

void PartMainVisualizer::updateDefaultDrawerVisibility()
{
    bool hasContent = m_codeDrawer->viewParser() != nullptr
                   || m_heightmapGridDrawer.visible()
                   || m_heightmapBorderDrawer.visible()
                   || m_heightmapInterpolationDrawer.visible();
    m_noGcodeDefaultDrawer.setVisible(!hasContent);
}

void PartMainVisualizer::showHeightmapBorder(bool show)
{
    m_heightmapBorderDrawer.setVisible(show);
    updateDefaultDrawerVisibility();
}

void PartMainVisualizer::showHeightmapProbeGrid(bool show)
{
    m_heightmapGridDrawer.setVisible(show);
    updateDefaultDrawerVisibility();
}

void PartMainVisualizer::showHeightmapInterpolationGrid(bool show)
{
    m_heightmapInterpolationDrawer.setVisible(show);
    updateDefaultDrawerVisibility();
}

void PartMainVisualizer::setHeightmapInterpolationMode(Heightmap::InterpolationMode mode)
{
    m_heightmapInterpolationDrawer.setInterpolationMode(mode);
    m_heightmapGridDrawer.setInterpolationMode(mode);
}

void PartMainVisualizer::updateHeightmap()
{
    m_heightmapBorderDrawer.update();
    m_heightmapGridDrawer.update();
    m_heightmapInterpolationDrawer.update();
    m_codeDrawer->update();
}

PartMainVisualizer::SegmentInfo PartMainVisualizer::getSegmentInfoForLine(int lineNumber)
{
    SegmentInfo info = {nullptr, nullptr, nullptr, nullptr};

    GCodeViewParser *parser = m_codeDrawer->viewParser();
    QList<LineSegment>& list = parser->getLineSegmentList();
    QVector<QList<int>> lineIndexes = parser->getLinesIndexes();

    if (lineNumber == -1 || lineNumber >= lineIndexes.count()) {
        return info;
    }

    if (lineIndexes.at(lineNumber).isEmpty()) {
        return info;
    }

    int firstIdx = lineIndexes.at(lineNumber).first();
    int lastIdx = lineIndexes.at(lineNumber).last();

    info.firstSegment = &list[firstIdx];
    info.lastSegment = &list[lastIdx];
    info.feedSegment = info.lastSegment;
    info.plungeSegment = info.lastSegment;

    int segmentIndex = list.indexOf(*info.feedSegment);
    while (info.feedSegment->isFastTraverse() && (segmentIndex > 0)) {
        info.feedSegment = &list[--segmentIndex];
    }

    while (!(info.plungeSegment->isZMovement() && !info.plungeSegment->isFastTraverse()) && (segmentIndex > 0)) {
        info.plungeSegment = &list[--segmentIndex];
    }

    return info;
}

void PartMainVisualizer::updateBillboardsScreenPositions()
{
    m_heightmapGridDrawer.billboardDrawable()->updateScreenPositions(
        ui->visualizer->viewMatrix(),
        ui->visualizer->projectionMatrix(),
        ui->visualizer->size()
    );
}

void PartMainVisualizer::toggleToolClicked(bool checked)
{
    m_toolDrawer.setVisible(checked);
}

void PartMainVisualizer::toggleLightClicked(bool checked)
{
    ui->visualizer->setLightEnabled(checked);
    m_lightSourceDrawer.setVisible(checked);
}

void PartMainVisualizer::toggleBoundingBoxClicked(bool checked)
{
    m_boundingBoxDrawer.setVisible(checked);
}

void PartMainVisualizer::toggleToolpathClicked(bool checked)
{
    m_codeDrawer->setVisible(checked);
}

void PartMainVisualizer::toggleHeightmapPreviewClicked(bool checked)
{
    m_codeDrawer->setHeightmapPreview(checked);
}

void PartMainVisualizer::toggleGridClicked(bool checked)
{
    m_tableSurfaceDrawer.setVisible(checked);
}
