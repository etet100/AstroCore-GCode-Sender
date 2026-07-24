// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODEPROGRAM_H
#define GCODEPROGRAM_H

#include <QList>
#include <QHash>
#include <QMap>
#include <QString>
#include "gcodeitem.h"

// Pure data model of a G-code program: item storage, overlays, response map
// and content hash. Holds no Qt signals and no execution cursor, so it can be
// unit tested in isolation. GCode wraps it and emits the view-update signals.
class GCodeProgram
{
    public:
        GCodeProgram() = default;

        // Item access
        GCodeItem& operator [] (int index) { return m_data[index]; }
        const GCodeItem& operator [] (int index) const { return m_data[index]; }
        GCodeItem& at(int index) { return m_data[index]; }
        int count() const { return m_data.count(); }
        bool empty() const { return m_data.isEmpty(); }
        const QList<GCodeItem>& items() const { return m_data; }

        QList<GCodeItem>::iterator begin() { return m_data.begin(); }
        QList<GCodeItem>::iterator end() { return m_data.end(); }

        void reserve(int size) { m_data.reserve(size); }
        void clear();

        // offset is number of lines ahead (1 = next line, 2 = line after next, etc.)
        GCodeItem* lookAhead(int fromIndex, int offset) {
            if (fromIndex + offset >= m_data.count() || offset < 1) {
                return nullptr;
            }

            return &m_data[fromIndex + offset];
        }

        // offset is number of lines behind (1 = previous line, 2 = line before that, etc.)
        GCodeItem* lookBehind(int fromIndex, int offset) {
            if (fromIndex - offset < 0 || offset < 1) {
                return nullptr;
            }

            return &m_data[fromIndex - offset];
        }

        GCodeItem* getLine(int index) {
            if (index < 0 || index >= m_data.count()) {
                return nullptr;
            }

            return &m_data[index];
        }

        // Mutations. These keep m_mainCount in sync and drop the response map on
        // structural edits (index keys become invalid). They emit nothing —
        // callers decide which signal fits.
        int append(const GCodeItem& item);
        int append(GCodeItem&& item);
        void appendProgram(const GCodeProgram& source);
        void insert(int index, const GCodeItem& item);
        void removeAt(int index);
        void erase(int begin, int end);
        void deleteLines(int from, int to);
        // Returns true if the replacement had the same number of lines as the
        // removed block (so callers can pick a data-only vs structural update).
        bool replace(int from, int to, const QList<GCodeItem>& items);
        QString linesAsText(int from, int to) const;

        // Resets every item to its initial run state (Comment lines stay Comment).
        void resetItemStates();

        // Overlay support. insertOverlay takes an explicit insert position; the
        // caller (GCode) derives it from the execution cursor. Returns the new
        // overlay id. resetOverlays returns true if it actually changed data.
        int insertOverlay(const QString& name, const QList<GCodeItem>& commands, int insertAt);
        const OverlayInfo* overlayInfo(int overlayId) const;
        bool resetOverlays(int fromIndex);
        bool isOverlayItem(int index) const;
        int mainCount() const { return m_mainCount; }

        // Response storage. Kept in a sparse hash indexed by position to avoid
        // paying a QString header per item. "ok" responses are not stored; they
        // are inferred from the Processed state. Only error/custom responses
        // occupy memory.
        QString response(int index) const;
        void setResponse(int index, const QString& response);
        void clearResponses() { m_responses.clear(); }

        // Content hash of the main program (overlays excluded).
        void updateHash();
        QString calculateHash() const;
        bool isModified() const;
        void markAsNotModified();

    private:
        QList<GCodeItem> m_data;
        QHash<int, QString> m_responses;
        int m_mainCount = 0;
        QMap<int, OverlayInfo> m_overlays;
        int m_nextOverlayId = 0;
        QString m_contentHash;
};

#endif // GCODEPROGRAM_H
