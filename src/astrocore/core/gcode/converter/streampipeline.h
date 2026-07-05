// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef STREAMPIPELINE_H
#define STREAMPIPELINE_H

#include "streamconverter.h"

/**
 * Composes multiple StreamConverter stages into a single stream operator.
 * Output of stage k becomes the input of stage k+1.
 *
 * Ownership: the pipeline takes ownership of every stage passed in via
 * operator<<; they are deleted in the pipeline's destructor.
 *
 * flush() is cascading — each stage is flushed in order, and the items it
 * emits are pushed through all downstream stages before the next stage is
 * flushed.
 */
class StreamPipeline : public StreamConverter
{
    public:
        StreamPipeline() = default;
        ~StreamPipeline() override;

        StreamPipeline(const StreamPipeline&) = delete;
        StreamPipeline& operator=(const StreamPipeline&) = delete;

        StreamPipeline& operator<<(StreamConverter *stage);

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override;
        void reset() override;

    private:
        QList<StreamConverter*> m_stages;
};

#endif // STREAMPIPELINE_H
