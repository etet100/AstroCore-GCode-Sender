// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef ABSTRACTBATCHCONVERTER_H
#define ABSTRACTBATCHCONVERTER_H

class GCode;

class AbstractBatchConverter
{
public:
    virtual ~AbstractBatchConverter() = default;

    virtual void setGCode(GCode *gcode) = 0;
    virtual int convertNext(int count) = 0;
    virtual void reset() = 0;
    virtual bool hasMore() const = 0;
    virtual int currentPosition() const = 0;
    virtual int totalLines() const = 0;

    /**
     * Convert entire G-Code and return new GCode object.
     * Source gcode must be set via setGCode() first.
     */
    virtual GCode* convertAll() = 0;
};

#endif // ABSTRACTBATCHCONVERTER_H
