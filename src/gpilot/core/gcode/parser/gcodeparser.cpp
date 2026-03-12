// This file is a part of "Candle" application.
// This file was originally ported from "GcodeParser.java" class
// of "Universal GcodeSender" application written by Will Winder
// (https://github.com/winder/Universal-G-Code-Sender)

// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include <QListIterator>
#include <QDebug>
#include "gcodeparser.h"
#include "core/gcode/gcode.h"

GcodeParser::GcodeParser(QObject *parent) : QObject(parent)
{
    m_state.isMetric = true;
    m_state.inAbsoluteMode = true;
    m_state.inAbsoluteIJKMode = false;
    m_state.lastGcodeCommand = -1;
    m_state.commandNumber = 0;
    m_state.lastSpeed = 0;
    m_state.lastSpindleSpeed = 0;

    // Settings
    m_speedOverride = -1;
    m_truncateDecimalLength = 40;
    m_removeAllWhitespace = true;
    m_convertArcsToLines = false;
    m_smallArcThreshold = 1.0;
    m_smallArcSegmentLength = 0.3;
    m_traverseSpeed = 300;

    reset();
}

GcodeParser::~GcodeParser()
{
    foreach (PointSegment *ps, m_points) delete ps;
}

bool GcodeParser::getConvertArcsToLines() {
    return m_convertArcsToLines;
}

void GcodeParser::setConvertArcsToLines(bool convertArcsToLines) {
    m_convertArcsToLines = convertArcsToLines;
}

bool GcodeParser::getRemoveAllWhitespace() {
    return m_removeAllWhitespace;
}

void GcodeParser::setRemoveAllWhitespace(bool removeAllWhitespace) {
    m_removeAllWhitespace = removeAllWhitespace;
}

double GcodeParser::getSmallArcSegmentLength() {
    return m_smallArcSegmentLength;
}

void GcodeParser::setSmallArcSegmentLength(double smallArcSegmentLength) {
    m_smallArcSegmentLength = smallArcSegmentLength;
}

double GcodeParser::getSmallArcThreshold() {
    return m_smallArcThreshold;
}

void GcodeParser::setSmallArcThreshold(double smallArcThreshold) {
    m_smallArcThreshold = smallArcThreshold;
}

double GcodeParser::getSpeedOverride() {
    return m_speedOverride;
}

void GcodeParser::setSpeedOverride(double speedOverride) {
    m_speedOverride = speedOverride;
}

int GcodeParser::getTruncateDecimalLength() {
    return m_truncateDecimalLength;
}

void GcodeParser::setTruncateDecimalLength(int truncateDecimalLength) {
    m_truncateDecimalLength = truncateDecimalLength;
}

// Resets the current state.
void GcodeParser::reset(const QVector3D &initialPoint)
{
    foreach (PointSegment *ps, m_points) {
        delete ps;
    }
    m_points.clear();
    // The unspoken home location.
    m_state.currentPoint = initialPoint;
    m_state.currentPlane = PointSegment::XY;
    m_points.append(new PointSegment(&m_state.currentPoint, -1));
}

/**
* Add a command to be processed.
*/
PointSegment* GcodeParser::addCommand(QString command)
{
    QString stripped = GcodePreprocessorUtils::removeComment(command);
    auto args = GcodePreprocessorUtils::splitCommand(stripped);

    if (args.empty()) return nullptr;
    return processCommand(args);
}

PointSegment *GcodeParser::addCommand(const GCodeItem &gcodeItem)
{
    if (gcodeItem.args.empty()) {
        return nullptr;
    }

    return processCommand(gcodeItem.args);
}

/**
* Warning, this should only be used when modifying live gcode, such as when
* expanding an arc or canned cycle into line segments.
*/
void GcodeParser::setLastGcodeCommand(float num) {
    m_state.lastGcodeCommand = num;
}

/**
* Gets the point at the end of the list.
*/
QVector3D *GcodeParser::getCurrentPoint() {
    return &m_state.currentPoint;
}

/**
* Expands the last point in the list if it is an arc according to the
* the parsers settings.
*/
// QList<PointSegment*> GcodeParser::expandArc()
// {
//     PointSegment *startSegment = this->m_points[this->m_points.size() - 2];
//     PointSegment *lastSegment = this->m_points[this->m_points.size() - 1];

//     QList<PointSegment*> empty;

//     // Can only expand arcs.
//     if (!lastSegment->isArc()) {
//         return empty;
//     }

//     // Get precalculated stuff.
//     QVector3D *start = startSegment->point();
//     QVector3D *end = lastSegment->point();
//     QVector3D *center = lastSegment->center();
//     double radius = lastSegment->getRadius();
//     bool clockwise = lastSegment->isClockwise();
//     PointSegment::planes plane = startSegment->plane();

//     // Start expansion.
//     QList<QVector3D> expandedPoints = GcodePreprocessorUtils::generatePointsAlongArcBDring(plane, *start, *end, *center, clockwise, radius, m_smallArcThreshold, m_smallArcSegmentLength, false);

//     // Validate output of expansion.
//     if (expandedPoints.length() == 0) {
//         return empty;
//     }

//     // Remove the last point now that we're about to expand it.
//     this->m_points.removeLast();
//     m_commandNumber--;

//     // Initialize return value
//     QList<PointSegment*> psl;

//     // Create line segments from points.
//     PointSegment *temp;

//     QListIterator<QVector3D> psi(expandedPoints);
//     // skip first element.
//     if (psi.hasNext()) psi.next();

//     while (psi.hasNext()) {
//         temp = new PointSegment(&psi.next(), m_commandNumber++);
//         temp->setIsMetric(lastSegment->isMetric());
//         this->m_points.append(temp);
//         psl.append(temp);
//     }

//     // Update the new endpoint.
//     this->m_currentPoint.setX(this->m_points.last()->point()->x());
//     this->m_currentPoint.setY(this->m_points.last()->point()->y());
//     this->m_currentPoint.setZ(this->m_points.last()->point()->z());

//     return psl;
// }

QList<PointSegment*> GcodeParser::getPointSegmentList() {
    return m_points;
}

double GcodeParser::getTraverseSpeed() const
{
    return m_traverseSpeed;
}

void GcodeParser::setTraverseSpeed(double traverseSpeed)
{
    m_traverseSpeed = traverseSpeed;
}

int GcodeParser::getCommandNumber() const
{
    return m_state.commandNumber - 1;
}

void GcodeParser::pushState()
{
    m_state.pointsCount = m_points.size();
    m_stateStack.append(m_state);
}

void GcodeParser::popState()
{
    if (!m_stateStack.isEmpty()) {
        GcodeParserState state = m_stateStack.takeLast();

        // Remove points added after the saved state and clean up memory
        while (m_points.size() > state.pointsCount) {
            delete m_points.takeLast();
        }

        m_state = state;
    }
}

// Legacy methods for backward compatibility
GcodeParserState GcodeParser::saveState() const
{
    GcodeParserState state = m_state;
    state.pointsCount = m_points.size();

    return state;
}

void GcodeParser::restoreState(const GcodeParserState &state)
{
    while (m_points.size() > state.pointsCount) {
        delete m_points.takeLast();
    }

    m_state = state;
}

PointSegment *GcodeParser::processCommand(const std::vector<std::string> &args)
{
    QList<float> gCodes;
    PointSegment *ps = nullptr;

    // Handle F code
    double speed = GcodePreprocessorUtils::parseCoord(args, 'F');
    if (!qIsNaN(speed)) m_state.lastSpeed = m_state.isMetric ? speed : speed * 25.4;

    // Handle S code
    double spindleSpeed = GcodePreprocessorUtils::parseCoord(args, 'S');
    if (!qIsNaN(spindleSpeed)) m_state.lastSpindleSpeed = spindleSpeed;

    // Handle P code
    double dwell = GcodePreprocessorUtils::parseCoord(args, 'P');
    if (!qIsNaN(dwell)) m_points.last()->setDwell(dwell);

    // handle G codes.
    gCodes = GcodePreprocessorUtils::parseCodes(args, 'G');

    // If there was no command, add the implicit one to the party.
    if (gCodes.isEmpty() && m_state.lastGcodeCommand != -1) {
        gCodes.append(m_state.lastGcodeCommand);
    }

    // Only one G command should generate a PointSegment???
    foreach (float code, gCodes) {
        PointSegment *ps2 = handleGCode(code, args);
        assert(ps2 == nullptr || ps == nullptr);
        ps = ps2;
    }

    return ps;
}

PointSegment *GcodeParser::addLinearPointSegment(const QVector3D &nextPoint, bool fastTraverse)
{
    PointSegment *ps = new PointSegment(&nextPoint, m_state.commandNumber++);

    bool zOnly = false;

    // Check for z-only
    if ((m_state.currentPoint.x() == nextPoint.x()) &&
            (m_state.currentPoint.y() == nextPoint.y()) &&
            (m_state.currentPoint.z() != nextPoint.z())) {
        zOnly = true;
    }

    ps->setIsMetric(m_state.isMetric);
    ps->setIsZMovement(zOnly);
    ps->setIsFastTraverse(fastTraverse);
    ps->setIsAbsolute(m_state.inAbsoluteMode);
    ps->setSpeed(fastTraverse ? m_traverseSpeed : m_state.lastSpeed);
    ps->setSpindleSpeed(m_state.lastSpindleSpeed);
    m_points.append(ps);

    // Save off the endpoint.
    m_state.currentPoint = nextPoint;

    return ps;
}

PointSegment *GcodeParser::addArcPointSegment(const QVector3D &nextPoint, bool clockwise, const std::vector<std::string> &args)
{
    PointSegment *ps = new PointSegment(&nextPoint, m_state.commandNumber++);

    QVector3D center = GcodePreprocessorUtils::updateCenterWithCommand(args, m_state.currentPoint, nextPoint, m_state.inAbsoluteIJKMode, clockwise);
    double radius = GcodePreprocessorUtils::parseCoord(args, 'R');

    // Calculate radius if necessary.
    if (qIsNaN(radius)) {

        QMatrix4x4 m;
        m.setToIdentity();
        switch (m_state.currentPlane) {
        case PointSegment::XY:
            break;
        case PointSegment::ZX:
            m.rotate(90, 1.0, 0.0, 0.0);
            break;
        case PointSegment::YZ:
            m.rotate(-90, 0.0, 1.0, 0.0);
            break;
        }

        radius = sqrt(pow((double)((m.map(m_state.currentPoint)).x() - (m.map(center)).x()), 2.0)
                        + pow((double)((m.map(m_state.currentPoint)).y() - (m.map(center)).y()), 2.0));
    }

    ps->setIsMetric(m_state.isMetric);
    ps->setArcCenter(&center);
    ps->setIsArc(true);
    ps->setRadius(radius);
    ps->setIsClockwise(clockwise);
    ps->setIsAbsolute(m_state.inAbsoluteMode);
    ps->setSpeed(m_state.lastSpeed);
    ps->setSpindleSpeed(m_state.lastSpindleSpeed);
    ps->setPlane(m_state.currentPlane);
    m_points.append(ps);

    // Save off the endpoint.
    m_state.currentPoint = nextPoint;
    return ps;
}

void GcodeParser::handleMCode(float code, const std::vector<std::string> &args)
{
    double spindleSpeed = GcodePreprocessorUtils::parseCoord(args, 'S');
    if (!qIsNaN(spindleSpeed)) m_state.lastSpindleSpeed = spindleSpeed;
}

void GcodeParser::expandCannedCycle(const QVector3D &position)
{
    if (m_state.activeCannedCycle < 0) {
        return;
    }

    // Move to XY position at current Z (or retract plane)
    QVector3D xyPosition(position.x(), position.y(), m_state.currentPoint.z());
    if (xyPosition != m_state.currentPoint) {
        addLinearPointSegment(xyPosition, true);
    }

    // Move to R plane (retract/rapid plane)
    QVector3D rPlane(position.x(), position.y(), m_state.cannedR);
    if (rPlane.z() != m_state.currentPoint.z()) {
        addLinearPointSegment(rPlane, true);
    }

    if (m_state.activeCannedCycle == 81.0f) {
        // G81 - Simple drilling cycle
        // 1. Rapid to R plane (already done)
        // 2. Feed to Z depth
        QVector3D zDepth(position.x(), position.y(), m_state.cannedZ);
        addLinearPointSegment(zDepth, false);
        // 3. Rapid back to R plane
        addLinearPointSegment(rPlane, true);
    }
    else if (m_state.activeCannedCycle == 82.0f) {
        // G82 - Drilling cycle with dwell
        // 1. Rapid to R plane (already done)
        // 2. Feed to Z depth
        QVector3D zDepth(position.x(), position.y(), m_state.cannedZ);
        PointSegment *ps = addLinearPointSegment(zDepth, false);
        // 3. Dwell at bottom
        if (ps && m_state.cannedP > 0) {
            ps->setDwell(m_state.cannedP);
        }
        // 4. Rapid back to R plane
        addLinearPointSegment(rPlane, true);
    }
    else if (m_state.activeCannedCycle == 83.0f) {
        // G83 - Peck drilling cycle
        double currentZ = m_state.cannedR;
        double targetZ = m_state.cannedZ;
        double peckIncrement = m_state.cannedQ > 0 ? m_state.cannedQ : (m_state.cannedR - m_state.cannedZ) / 5.0;

        // Peck down in increments
        while (currentZ > targetZ + 0.001) { // small tolerance
            currentZ -= peckIncrement;
            if (currentZ < targetZ) currentZ = targetZ;

            // Feed down
            QVector3D peckDepth(position.x(), position.y(), currentZ);
            addLinearPointSegment(peckDepth, false);

            // If not at final depth, retract and rapid back
            if (currentZ > targetZ + 0.001) {
                // Retract slightly
                QVector3D retract(position.x(), position.y(), currentZ + 1.0);
                addLinearPointSegment(retract, true);
                // Rapid back to just above current depth
                QVector3D reentry(position.x(), position.y(), currentZ + 0.5);
                addLinearPointSegment(reentry, true);
            }
        }
        // Final retract to R plane
        addLinearPointSegment(rPlane, true);
    }
}

// Why we use float here? Because of such G-codes as G32.2
PointSegment * GcodeParser::handleGCode(float code, const std::vector<std::string> &args)
{
    PointSegment *ps = nullptr;

    QVector3D nextPoint = GcodePreprocessorUtils::updatePointWithCommand(args, m_state.currentPoint, m_state.inAbsoluteMode);

    if (code == 0.0f) ps = addLinearPointSegment(nextPoint, true);
    else if (code == 1.0f) ps = addLinearPointSegment(nextPoint, false);
    else if (code == 2.0f) ps = addArcPointSegment(nextPoint, true, args);
    else if (code == 3.0f) ps = addArcPointSegment(nextPoint, false, args);
    else if (code == 4.0f) {} // G4 - dwell, handled by P code in processCommand
    else if (code == 17.0f) m_state.currentPlane = PointSegment::XY;
    else if (code == 18.0f) m_state.currentPlane = PointSegment::ZX;
    else if (code == 19.0f) m_state.currentPlane = PointSegment::YZ;
    else if (code == 20.0f) m_state.isMetric = false;
    else if (code == 21.0f) m_state.isMetric = true;
    else if (code == 28.0f) ps = addLinearPointSegment(nextPoint, true); // G28 - return to home
    else if (code == 30.0f) ps = addLinearPointSegment(nextPoint, true); // G30 - return to secondary home
    // Probing
    else if (code == 38.1f) ps = addLinearPointSegment(nextPoint, false); // probe toward, error if no contact
    else if (code == 38.2f) ps = addLinearPointSegment(nextPoint, false); // probe toward, stop on contact
    else if (code == 38.3f) ps = addLinearPointSegment(nextPoint, false); // probe away, stop on loss of contact
    else if (code == 38.4f) ps = addLinearPointSegment(nextPoint, false); // probe away, stop on loss of contact
    else if (code == 38.5f) ps = addLinearPointSegment(nextPoint, false); // probe toward, stop on contact
    // Canned cycles
    else if (code == 80.0f) {
        m_state.activeCannedCycle = -1; // Cancel canned cycle
    }
    else if (code >= 81.0f && code <= 83.0f) {
        // Update canned cycle parameters
        m_state.activeCannedCycle = code;
        double r = GcodePreprocessorUtils::parseCoord(args, 'R');
        double z = GcodePreprocessorUtils::parseCoord(args, 'Z');
        double q = GcodePreprocessorUtils::parseCoord(args, 'Q');
        double p = GcodePreprocessorUtils::parseCoord(args, 'P');

        if (!qIsNaN(r)) m_state.cannedR = r;
        if (!qIsNaN(z)) m_state.cannedZ = z;
        if (!qIsNaN(q)) m_state.cannedQ = q;
        if (!qIsNaN(p)) m_state.cannedP = p;

        // Execute the cycle at the current/specified position
        expandCannedCycle(nextPoint);
    }
    else if (code == 90.0f) m_state.inAbsoluteMode = true;
    else if (code == 90.1f) m_state.inAbsoluteIJKMode = true;
    else if (code == 91.0f) m_state.inAbsoluteMode = false;
    else if (code == 91.1f) m_state.inAbsoluteIJKMode = false;

    // Update last G-code command for modal commands
    if (code == 0.0f || code == 1.0f || code == 2.0f || code == 3.0f ||
        (code >= 38.1f && code <= 38.5f)) {
        m_state.lastGcodeCommand = code;
    }
    // Canned cycles are modal - they repeat for each XY position
    if (code >= 81.0f && code <= 89.0f) {
        m_state.lastGcodeCommand = code;
    }
    // Execute active canned cycle if we have XY movement and active cycle
    if (m_state.activeCannedCycle > 0 && (code == 0.0f || code == 1.0f)) {
        if (nextPoint.x() != m_state.currentPoint.x() || nextPoint.y() != m_state.currentPoint.y()) {
            expandCannedCycle(nextPoint);
        }
    }

    return ps;
}

// QStringList GcodeParser::preprocessCommands(QStringList commands) {

//     QStringList result;

//     foreach (QString command, commands) {
//         result.append(preprocessCommand(command));
//     }

//     return result;
// }

// QStringList GcodeParser::preprocessCommand(QString command) {

//     QStringList result;
//     bool hasComment = false;

//     // Remove comments from command.
//     QString newCommand = GcodePreprocessorUtils::removeComment(command);
//     QString rawCommand = newCommand;
//     hasComment = (newCommand.length() != command.length());

//     if (m_removeAllWhitespace) {
//         newCommand = GcodePreprocessorUtils::removeAllWhitespace(newCommand);
//     }

//     if (newCommand.length() > 0) {
//         // Override feed speed
//         if (m_speedOverride > 0) {
//             newCommand = GcodePreprocessorUtils::overrideSpeed(newCommand, m_speedOverride);
//         }

//         if (m_truncateDecimalLength > 0) {
//             newCommand = GcodePreprocessorUtils::truncateDecimals(m_truncateDecimalLength, newCommand);
//         }

//         // If this is enabled we need to parse the gcode as we go along.
//         if (m_convertArcsToLines) { // || this.expandCannedCycles) {
//             QStringList arcLines = convertArcsToLines(newCommand);
//             if (arcLines.length() > 0) {
//                 result.append(arcLines);
//             } else {
//                 result.append(newCommand);
//             }
//         } else if (hasComment) {
//             // Maintain line level comment.
//             result.append(command.replace(rawCommand, newCommand));
//         } else {
//             result.append(newCommand);
//         }
//     } else if (hasComment) {
//         // Reinsert comment-only lines.
//         result.append(command);
//     }

//     return result;
// }

// QStringList GcodeParser::convertArcsToLines(QString command) {

//     QStringList result;

//     QVector3D start = this->m_currentPoint;

//     PointSegment *ps = addCommand(command);

//     if (ps == NULL || !ps->isArc()) {
//         return result;
//     }

//     QList<PointSegment*> psl = expandArc();

//     if (psl.length() == 0) {
//         return result;
//     }

//     // Create an array of new commands out of the of the segments in psl.
//     // Don't add them to the gcode parser since it is who expanded them.
//     foreach (PointSegment* segment, psl) {
//         //Point3d end = segment.point();
//         QVector3D end = *segment->point();
//         result.append(GcodePreprocessorUtils::generateG1FromPoints(start, end, this->m_inAbsoluteMode, m_truncateDecimalLength));
//         start = *segment->point();
//     }

//     return result;

// }
