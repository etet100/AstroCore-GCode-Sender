#include "partmainvisualizer.h"
#include "ui_partmainvisualizer.h"
#include "core/config/module/configurationvisualizer.h"
#include "core/config/module/configurationmachine.h"
#include "core/gcode/gcode.h"
#include "ui/drawers/vertexdataexporter.h"
#include <QRegularExpression>
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
    m_probeDrawer = new GcodeDrawer();
    m_probeDrawer->setVisible(false);
    m_currentDrawer = m_codeDrawer;

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
        if (m_heightmapGridDrawer.visible()) {
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
    });
    // connect(ui->visualizer, &GLContainer::goToCursor, this, &PartMainVisualizer::goToCursor);
    connect(ui->visualizer, &GLContainer::viewModeChanged, this, [this](GLWidget::ViewMode mode) {
        emit viewModeChanged(mode);
    });

    connect(&ThemeManager::instance(), &ThemeManager::scaleChanged, [this]() {
        m_originDrawer.update();
        m_heightmapGridDrawer.update();
    });

    initializeButtons();
    initializeInfoBar();
}

PartMainVisualizer::~PartMainVisualizer()
{
    delete m_infoAnimation;
    delete m_infoOpacityEffect;
    delete m_codeDrawer;
    delete m_probeDrawer;
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
    m_infoAnimation->setDuration(100);
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

// Do not init drawables before the parsers are set
// (setCodeParser, setProbeParser)
void PartMainVisualizer::initDrawables()
{
    *ui->visualizer << m_codeDrawer << m_probeDrawer
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
    ;

    ui->visualizer->fitDrawable(m_codeDrawer);
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
    ui->visualizer->setColorBackground(visualizerConfiguration.backgroundColor());
    ui->visualizer->setColorText(visualizerConfiguration.textColor());

    // Adapt visualizer buttons colors
    const int LIGHTBOUND = 140;
    const int NORMALSHIFT = 40;
    const int HIGHLIGHTSHIFT = 80;

    QColor base = visualizerConfiguration.backgroundColor();
    bool light = base.value() > LIGHTBOUND;

    // Use background color with some transparency for buttons background
    ui->buttons->setStyleSheet(
        ui->buttons->styleSheet().replace(
            QRegularExpression("/\\* bbg \\*/ background-color: rgba\\([^;^\\}]+\\)"),
                        QString("/* bbg */ background-color: rgba(%1,%2,%3,%4)").arg(base.red())
                                                   .arg(base.green())
                                                   .arg(base.blue())
                .arg(std::max(0, base.alpha() - 100))
            )
        );

    // ui->cmdToggleProjection->setIcon(QIcon(":/images/visualizer_toggle_view_mode.png"));
    // ui->cmdFit->setIcon(QIcon(":/images/fit_1.png"));
    // ui->cmdIsometric->setIcon(QIcon(":/images/visualizer_isometric.png"));
    // ui->cmdFront->setIcon(QIcon(":/images/visualizer_front.png"));
    // ui->cmdRight->setIcon(QIcon(":/images/visualizer_left.png"));
    // ui->cmdTop->setIcon(QIcon(":/images/visualizer_top.png"));

    QColor normal, highlight;

    normal.setHsv(base.hue(), base.saturation(), base.value() + (light ? -NORMALSHIFT : NORMALSHIFT));
    highlight.setHsv(base.hue(), base.saturation(), base.value() + (light ? -HIGHLIGHTSHIFT : HIGHLIGHTSHIFT));

    ui->visualizer->setStyleSheet(QString("QToolButton {border: 1px solid %1; \
                background-color: %3} QToolButton:hover {border: 1px solid %2;}")
                .arg(normal.name()).arg(highlight.name())
                .arg(base.name()));

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
    ui->visualizer->updateExtremes(m_currentDrawer);
}

void PartMainVisualizer::fitDrawable()
{
    ui->visualizer->fitDrawable();
}

void PartMainVisualizer::fitCodeDrawer()
{
    ui->visualizer->fitDrawable(m_codeDrawer);
}

void PartMainVisualizer::setProgram(GCode* program, GCodeViewParser* parser)
{
    m_program = program;
    m_codeDrawer->setViewParser(parser);
}

void PartMainVisualizer::setProbeParser(GCodeViewParser* parser)
{
    m_probeDrawer->setViewParser(parser);
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

void PartMainVisualizer::updateCurrentDrawer(const QList<int>& indexes)
{
    m_currentDrawer->update(indexes);
}

void PartMainVisualizer::setToolPosition(QVector3D pos)
{
    m_toolDrawer.setToolPosition(pos);
}

void PartMainVisualizer::reset()
{
    m_codeDrawer->update();
    m_currentDrawer = m_codeDrawer;
    ui->visualizer->fitDrawable();

    m_selectionDrawer.resetEndPosition();
    m_selectionDrawer.update();
}

void PartMainVisualizer::setHeightmap(Heightmap& heightmap)
{
    m_heightmap = &heightmap;
    m_heightmapBorderDrawer.setModel(heightmap);
    m_heightmapGridDrawer.setModel(heightmap);
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
}

void PartMainVisualizer::setSelectionVisible(bool visible)
{
    m_selectionDrawer.setVisible(visible);
}

void PartMainVisualizer::useCodeDrawer()
{
    m_currentDrawer = m_codeDrawer;
}

void PartMainVisualizer::useProbeDrawer()
{
    m_currentDrawer = m_probeDrawer;
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

void PartMainVisualizer::rotationCubeClicked()
{
    ui->visualizer->toggleRotationCube();
}

void PartMainVisualizer::heightmapClicked()
{
    m_heightmapGridDrawer.toggleVisible();
    if (m_heightmapGridDrawer.visible()) {
        updateBillboardsScreenPositions();
    }
}

void PartMainVisualizer::heightmapMarkersClicked()
{
    if (!m_heightmapGridDrawer.visible()) {
        // show grid too
        m_heightmapGridDrawer.toggleVisible();
        updateBillboardsScreenPositions();
    } else {
        m_heightmapGridDrawer.billboardDrawable()->toggleVisible();
        if (m_heightmapGridDrawer.billboardDrawable()->visible()) {
            updateBillboardsScreenPositions();
        }
    }
}

void PartMainVisualizer::toggleProjectionClicked()
{
    ui->visualizer->toggleProjectionType();
}

void PartMainVisualizer::toggleOriginClicked()
{
    m_originDrawer.toggleVisible();
}

void PartMainVisualizer::fitClicked()
{
    ui->visualizer->fitDrawable(m_currentDrawer);
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

void PartMainVisualizer::applyCodeDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration, ConfigurationMachine &machineConfiguration)
{
    m_codeDrawer->setLineWidth(visualizerConfiguration.lineWidth());
    m_codeDrawer->setSimplify(visualizerConfiguration.simplifyGeometry());
    m_codeDrawer->setSimplifyPrecision(visualizerConfiguration.simplifyGeometryPrecision());
    m_codeDrawer->setColorNormal(visualizerConfiguration.normalToolpathColor());
    m_codeDrawer->setColorDrawn(visualizerConfiguration.drawnToolpathColor());
    m_codeDrawer->setColorHighlight(visualizerConfiguration.hightlightToolpathColor());
    m_codeDrawer->setColorZMovement(visualizerConfiguration.zMovementColor());
    m_codeDrawer->setColorRapidMovement(visualizerConfiguration.rapidMovementColor());
    m_codeDrawer->setColorStart(visualizerConfiguration.startPointColor());
    m_codeDrawer->setColorEnd(visualizerConfiguration.endPointColor());
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
    m_toolDrawer.setColor(visualizerConfiguration.toolColor());
    m_toolDrawer.update();
}

void PartMainVisualizer::applyCursorDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_cursorDrawer.setVisible(visualizerConfiguration.show3dCursor());
    m_cursorDrawer.setColor(visualizerConfiguration.cursorColor());
    m_cursorDrawer.update();
}

void PartMainVisualizer::applyTableSurfaceDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_tableSurfaceDrawer.setGridColor(visualizerConfiguration.tableSurfaceGridColor());
    m_tableSurfaceDrawer.update();
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
    m_selectionDrawer.setColor(visualizerConfiguration.hightlightToolpathColor());
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
    m_currentDrawer = m_codeDrawer;
    ui->visualizer->fitDrawable(m_codeDrawer);

    m_selectionDrawer.resetEndPosition();
    m_selectionDrawer.update();
}

void PartMainVisualizer::resetVisualization()
{
    m_lastDrawnLineIndex = 0;
    m_codeDrawer->update();
    m_currentDrawer = m_codeDrawer;
    ui->visualizer->fitDrawable();

    m_selectionDrawer.resetEndPosition();
    m_selectionDrawer.update();
}

void PartMainVisualizer::updateToolpathHighlighting(int currentRow, int previousRow)
{
    if (!m_program || m_program->empty()) {
        return;
    }

    int rowCurrent = qMin(currentRow, m_program->lastCommandIndex());
    int rowPrevious = qMax(qMin(previousRow, m_program->lastCommandIndex()), 0);

    qDebug() << "[PartMainVisualizer] Updating toolpath highlighting from row"
             << rowPrevious << "to" << rowCurrent;

    GCodeViewParser *parser = m_currentDrawer->viewParser();
    QList<LineSegment>& list = parser->getLineSegmentList();
    QVector<QList<int>>& lineIndexes = parser->getLinesIndexes();

    GCodeItem& currentItem = m_program->at(rowCurrent);
    GCodeItem& previousItem = m_program->at(rowPrevious);

    qDebug() << "[PartMainVisualizer] Is movment:" << currentItem.isMovement;

    // Update linesegments on cell changed
    if (!m_currentDrawer->geometryUpdated()) {
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
            m_currentDrawer->update(indexes);
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

    GCodeViewParser *parser = m_currentDrawer->viewParser();
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
            m_currentDrawer->update(drawnLines);
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
    GCodeViewParser *parser = m_currentDrawer->viewParser();
    QList<LineSegment>& list = parser->getLineSegmentList();

    if (m_lastDrawnLineIndex < list.count()) {
        list[m_lastDrawnLineIndex].setDrawn(true);
        m_currentDrawer->update(QList<int>() << m_lastDrawnLineIndex);
        ui->visualizer->update();
    }

    m_lastDrawnLineIndex = 0;
}

GCodeViewParser* PartMainVisualizer::getCurrentParser()
{
    return m_currentDrawer->viewParser();
}

bool PartMainVisualizer::isCurrentDrawerProbeMode() const
{
    return m_currentDrawer == m_probeDrawer;
}

void PartMainVisualizer::updateCurrentDrawerGeometry()
{
    m_currentDrawer->update();
    ui->visualizer->update();
}

void PartMainVisualizer::exportCodeDrawerToFile(const QString& filename)
{
    VertexDataExporter::exportToJsFile(filename, m_codeDrawer->lines());
}

void PartMainVisualizer::showHeightmapBorder(bool show)
{
    m_heightmapBorderDrawer.setVisible(show);
}

void PartMainVisualizer::showHeightmapProbeGrid(bool show)
{
    m_heightmapGridDrawer.setVisible(show);
}

void PartMainVisualizer::showHeightmapInterpolationGrid(bool show)
{
    m_heightmapInterpolationDrawer.setVisible(show);
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
}

PartMainVisualizer::SegmentInfo PartMainVisualizer::getSegmentInfoForLine(int lineNumber)
{
    SegmentInfo info = {nullptr, nullptr, nullptr, nullptr};

    GCodeViewParser *parser = m_currentDrawer->viewParser();
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

void PartMainVisualizer::toggleToolClicked()
{
    m_toolDrawer.toggleVisible();
}

void PartMainVisualizer::toggleLightClicked()
{
    ui->visualizer->toggleLight();
}
