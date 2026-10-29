/**
 ********************************************************************************
 * @file    CircularBuffer.hpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    October 28, 2026
 * @brief   
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
    T storage[N];
    size_t head;
    size_t tail;
public:
    CircularBuffer(void): head(0), tail(0) { };

    bool IsFull(void) {
        return ((head + 1) % N) == tail;
    }

    bool IsEmpty(void) {
        return head == tail;
    }

    int Enqueue(T& datum) {
        if (IsFull()) return -1;    

        storage[head] = datum;
        head = (head + 1) % N;

        return 0;
    }

    T& Dequeue(void) {
        if (IsEmpty()) return nullptr;

        T& ret = storage[tail];
        tail = (tail + 1) % N;

        return ret;
    } 
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* CIRCULAR_BUFFER_H */
