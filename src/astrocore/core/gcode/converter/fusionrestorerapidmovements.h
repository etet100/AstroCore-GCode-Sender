// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef FUSIONRESTORERAPIDMOVEMENTS_H
#define FUSIONRESTORERAPIDMOVEMENTS_H

#include "streamconverter.h"
#include <vector>
#include <string>

/**
 * Detects G1 moves that Fusion 360 generated as feed moves but should be G0,
 * and converts them. Ported from Tim Paterson's PostProcessAll.py "Restore rapid moves".
 *
 * Background
 * ----------
 * Fusion 360 post-processors output all moves (including retract and reposition moves)
 * as G1 (feed rate) moves. This is safe but slow. Most of those moves happen above
 * the material surface and could be G0 (rapid). This converter detects them and
 * replaces them with G0.
 *
 * The converter is stateful: it observes lines one by one and tracks position and
 * feed height. Modified lines get a (Changed from: "...") inline comment.
 *
 * Algorithm
 * ---------
 * The converter estimates a "feed height" (Zfeed) — the Z level above which all
 * moves are repositioning moves and can be rapids. It then converts G1 to G0 for
 * any move that is at or above Zfeed, and restores G1 for moves that descend below it.
 *
 * Step A — Learn Zfeed.
 *   The first Z-only move (no XY) seen after a G0 sets Zfeed to that Z value.
 *   If a second Z-only move follows, Zfeed is updated again and marked as confirmed.
 *   A G1 Z-only move used to learn Zfeed is converted to G0 (it is a retract).
 *
 * Step B — Convert G1 to G0.
 *   For a G1 move with Z but no XY (Z-only retract/plunge):
 *     Convert to G0 if: Z goes upward, OR Z is at/above Zfeed, OR feed rate is zero.
 *   For a G1 move with XY but no Z (horizontal repositioning):
 *     Convert to G0 if: current Z is at or above Zfeed.
 *
 * Step C — Restore G1 on modal lines after a G0 conversion.
 *   After a G0 conversion, the next line has no explicit G-code (modal, inherits G0).
 *   If that modal line moves below Zfeed, it is a real cutting move — restore G1
 *   and add an explicit F parameter so the feed rate is not lost.
 *
 * Step D — Add missing F parameter.
 *   If a G1 line after a G0 conversion is missing an F value, append it.
 *   This handles cases not covered by step C (e.g. mixed XY+Z cutting moves).
 *
 * Step E — Adjust Zfeed upward.
 *   If a cutting move (G1 with XY) is found at or above Zfeed, Zfeed was estimated
 *   too low. Raise it slightly above the current Z so future moves are not wrongly
 *   converted.
 *
 * State variables
 * ---------------
 *   m_lastMotionGcode  Last motion G-code seen (0, 1, 2, 3, or -1 if none yet).
 *   m_zcur             Current Z position (NaN until first Z seen).
 *   m_zlast            Z position before the last Z change (NaN until second Z seen).
 *   m_zfeed            Estimated feed height (NaN until step A runs).
 *   m_zfeedNotSet      True while Zfeed is only from a single G0 (not yet confirmed).
 *   m_feedcur          Last feed rate seen (0 until first F seen).
 *   m_needFeed         True when the next non-G0 line needs an explicit F appended.
 *   m_lockSpeed        True when M49 is active (speed overrides disabled — skip B/C).
 *
 * Limitations
 * -----------
 *   - G90 (absolute mode) only. G91 (incremental) is not supported because
 *     the algorithm stores Z values directly from the line (m_zcur = zVal)
 *     instead of accumulating deltas. Fusion 360 always outputs G90, so
 *     this was never a concern for the original Python author either.
 *   - Experimental — may misidentify moves near the feed height boundary.
 *   - With only one clearance-level Z-move before a plunge, the plunge is converted
 *     to G0 (tool crash risk). At least two Z-only moves above material are needed
 *     for Zfeed to be confirmed before the first plunge.
 */
class FusionRestoreRapidMovements : public StreamConverter
{
    public:
        static QString parameterSchema();

        FusionRestoreRapidMovements();

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override { return {}; }
        void reset() override;

    private:
        // -1 = none seen yet, 0 = G0, 1 = G1, 2/3 = arc
        int    m_lastMotionGcode;
        double m_zcur;
        double m_zlast;
        double m_zfeed;
        bool   m_zfeedNotSet;
        double m_feedcur;
        bool   m_needFeed;
        bool   m_lockSpeed;

        static QString formatZ(double z);
        static QString formatF(double f);
        static QString extractXY(const std::vector<std::string> &args);
};

#endif // FUSIONRESTORERAPIDMOVEMENTS_H
