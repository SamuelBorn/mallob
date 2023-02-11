#pragma once

template <typename T>
class SynchronizedOrderedBuffer {
public:
    virtual ~SynchronizedOrderedBuffer() = default;

    // returns if there was enough space or if some item had to be kicked out
    virtual bool insert(T element) = 0;

    virtual T extract() = 0;

    virtual bool empty() = 0;

    virtual int size() = 0;
};