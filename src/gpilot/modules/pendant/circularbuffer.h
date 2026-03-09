#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include "stdint.h"
#include <cstring>

template<size_t S>
class CircularBuffer {
    public:
        CircularBuffer();
        ~CircularBuffer();
        bool push(uint8_t *data, size_t len);
        void get(uint8_t *output, size_t len);
        void skip(size_t len);
        uint8_t operator [] (size_t index) const;
        size_t free();
        size_t size();
        int skipped();

    private:
        size_t free_ = S;
        size_t head = 0;
        size_t tail = 0;
        uint8_t buffer[S];
        int skipped_ = 0;
};

template<size_t S>
CircularBuffer<S>::CircularBuffer() {
}

template<size_t S>
CircularBuffer<S>::~CircularBuffer() {
}

template<size_t S>
bool CircularBuffer<S>::push(uint8_t *data, size_t len) {
    if (free_ < len) {
        return false;
    }
    size_t toEnd = S - head;
    if (toEnd >= len) {
        memcpy(buffer + head, data, len);
        head += len;
    } else {
        memcpy(buffer + head, data, toEnd);
        memcpy(buffer, data + toEnd, len - toEnd);
        head = len - toEnd;
    }
    head = head % S;
    free_ -= len;
    return true;
}

template<size_t S>
void CircularBuffer<S>::get(uint8_t *output, size_t len) {
    if (size() < len) {
        return;
    }

    size_t toEnd = S - tail;
    if (toEnd >= len) {
        memcpy(output, buffer + tail, len);
        tail += len;
    } else {
        memcpy(output, buffer + tail, toEnd);
        memcpy(output + toEnd, buffer, len - toEnd);
        tail = len - toEnd;
    }
    tail = tail % S;
    free_ += len;
}

template<size_t S>
void CircularBuffer<S>::skip(size_t len) {
    if (size() < len) {
        return;
    }

    tail = (tail + len) % S;
    free_ += len;
    skipped_ += len;
}

template<size_t S>
uint8_t CircularBuffer<S>::operator [](size_t index) const {
    return buffer[(tail + index) % S];
}

template<size_t S>
size_t CircularBuffer<S>::free() {
    return free_;
}

template<size_t S>
size_t CircularBuffer<S>::size() {
    return S - free_;
}

template<size_t S>
int CircularBuffer<S>::skipped() {
    return skipped_;
}

#endif // CIRCULARBUFFER_H
