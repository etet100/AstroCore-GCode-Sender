// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "streampipeline.h"

StreamPipeline::~StreamPipeline()
{
    qDeleteAll(m_stages);
}

StreamPipeline& StreamPipeline::operator<<(StreamConverter *stage)
{
    if (stage) {
        m_stages << stage;
    }

    return *this;
}

QList<GCodeItem> StreamPipeline::push(const GCodeItem &input)
{
    QList<GCodeItem> wave = { input };

    for (auto *stage : m_stages) {
        QList<GCodeItem> next;
        for (const auto &item : wave) {
            next.append(stage->push(item));
        }
        wave = std::move(next);
    }

    return wave;
}

QList<GCodeItem> StreamPipeline::flush()
{
    // Cascading flush: flush stage k, push the result through stages [k+1..n-1],
    // then move on to k+1. By the time we flush stage k+1, it has seen everything
    // that earlier stages emitted during their own flush — this lets late-bound
    // items propagate correctly down the chain.
    QList<GCodeItem> tail;

    for (int k = 0; k < m_stages.size(); ++k) {
        QList<GCodeItem> wave = m_stages[k]->flush();

        for (int j = k + 1; j < m_stages.size(); ++j) {
            QList<GCodeItem> next;
            for (const auto &item : wave) {
                next.append(m_stages[j]->push(item));
            }
            wave = std::move(next);
        }

        tail.append(wave);
    }

    return tail;
}

void StreamPipeline::reset()
{
    for (auto *stage : m_stages) {
        stage->reset();
    }
}
