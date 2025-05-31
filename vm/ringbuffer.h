#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <memory>
#include "instruction.h"

#define VM_COMMAND_BUFFER_SIZE 100

template <typename T>
class RingBuffer
{
private:
    std::unique_ptr<T> buffer[VM_COMMAND_BUFFER_SIZE];
    int head;
    int tail;

public:
    RingBuffer() : head(0), tail(0) {}

    bool isEmpty() const
    {
        return head == tail;
    }
    bool isFull() const
    {
        return (head + 1) % VM_COMMAND_BUFFER_SIZE == tail;
    }

    void push(std::unique_ptr<T> value)
    {
        buffer[head] = std::move(value);
        head = (head + 1) % VM_COMMAND_BUFFER_SIZE;
        if (head == tail)
        {
            tail = (tail + 1) % VM_COMMAND_BUFFER_SIZE;
        }
    }

    std::unique_ptr<T> pop()
    {
        if (head == tail)
        {
            return nullptr;
        }
        std::unique_ptr<T> value = std::move(buffer[tail]);
        tail = (tail + 1) % VM_COMMAND_BUFFER_SIZE;
        return value;
    }
};

#endif