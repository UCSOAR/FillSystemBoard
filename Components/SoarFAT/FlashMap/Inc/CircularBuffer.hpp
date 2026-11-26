/**
 ********************************************************************************
 * @file    CircularBuffer.hpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    October 28, 2026
 * @brief   Implements a queue using a CircularBuffer as a template for variable
 *          types and sizes.
 ********************************************************************************
 */

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

/************************************
 * INCLUDES
 ************************************/

#include <stddef.h>

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/

template <class T, size_t N>
class CircularBuffer {
private:
    static constexpr size_t capacity = N + 1;

    T storage[capacity];
    size_t head;
    size_t tail;
public:
    CircularBuffer(void): head(0), tail(0) { };

    bool IsFull(void) const {
        return ((head + 1) % capacity) == tail;
    }

    bool IsEmpty(void) const {
        return head == tail;
    }

    void Enqueue(const T& datum) {
        storage[head] = datum;
        head = (head + 1) % capacity;
    }

    T& Dequeue(void) {
        T& ret = storage[tail];
        tail = (tail + 1) % capacity;
        return ret;
    }

    T& operator[] (size_t i) {
        return const_cast<T&>(static_cast<const CircularBuffer&>(*this)[i]);
    }

    const T& operator[] (size_t i) const {
        return storage[i];
    }
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* CIRCULAR_BUFFER_H */
