#include "partmainvisualizer.h"
#include "ui_partmainvisualizer.h"

PartMainVisualizer::PartMainVisualizer(GCode& m_program, Heightmap& m_heightmap, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::partMainVisualizer)
    , m_heightmapBorderDrawer(m_heightmap)
    , m_heightmapGridDrawer(m_heightmap),
    m_heightmap(m_heightmap),
    m_program(m_program)
{
    ui->setupUi(this);

    connect(ui->gl, &GLContainer::resized, this, &PartMainVisualizer::placeVisualizerButtons);
}

PartMainVisualizer::~PartMainVisualizer()
{
    delete ui;
}

void PartMainVisualizer::placeVisualizerButtons()
{
    ui->buttons->setParent(ui->gl);
    ui->buttons->move(
        width() - ui->buttons->width() - 8,
        8
    );
}

void PartMainVisualizer::onVisualizerCursorPosChanged(QPointF pos)
{
   m_cursorDrawer.setPosition(pos);
}

void PartMainVisualizer::addDrawables()
{
    *ui->gl << &m_originDrawer << m_codeDrawer << m_probeDrawer
                       << &m_cursorDrawer << &m_heightmapBorderDrawer
                       << &m_heightmapGridDrawer << &m_heightmapInterpolationDrawer
                       << &m_selectionDrawer << &m_machineBoundsDrawer << &m_toolDrawer;

    ui->gl->fitDrawable(m_codeDrawer);
}
