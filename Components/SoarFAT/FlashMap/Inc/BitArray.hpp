/**
 ********************************************************************************
 * @file    BitArray.hpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    October 24, 2026
 * @brief   This provides a bit array implementation backed by an atomic 32-bit
 *          unsigned integer array. This class does not provide bounds checking
 *          to maximize speed.
 ********************************************************************************
 */

#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H

/************************************
 * INCLUDES
 ************************************/

#include <stdint.h>
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

template <size_t N>
class BitArray {
private:
    static constexpr size_t capacity = (N + 31) / 32;
    uint32_t storage[capacity];

public:
    /**
     * Zero-initializes all bits.
     */
    BitArray(void) {
        for (size_t i = 0; i < capacity; i++) {
            storage[i] = 0;
        }
    }

    /**
     * Atomically sets the nth bit.
     */
    void Set(size_t n) {
        storage[n / 32] |= (0b1U << (n % 32));
    }

    /**
     * Atomically resets the nth bit.
     */
    void Reset(size_t n) {
        storage[n / 32] &= ~(0b1U << (n % 32));
    }

    /**
     * Returns the nth bit.
     */
    uint32_t Get(size_t i) const {
        return (storage[i / 32] >> (i % 32)) & 0b1U;
    }

    constexpr size_t BackingArraySize(void) const {
        return capacity;
    }

    void BackingArraySetByte(size_t idx, uint32_t byte) {
        storage[idx] = byte; 
    }

    uint32_t BackingArrayGetByte(size_t idx) const {
        return storage[idx]; 
    }
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* BIT_ARRAY_H */
