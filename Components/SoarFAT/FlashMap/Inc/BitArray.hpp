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

#include <atomic>

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
    std::atomic_uint32_t storage[(N + 31) / 32];

public:
    /**
     * Zero-initializes all bits.
     */
    BitArray(void) {
        for (size_t i = 0; i < ((N + 31) / 32); i++) {
            storage[i] = 0;
        }
    }

    /**
     * Atomically sets the nth bit.
     */
    void set(size_t n) {
        storage[n / 32].fetch_or(0b1U << (n % 32), std::memory_order_relaxed);
    }

    /**
     * Atomically resets the nth bit.
     */
    void reset(size_t n) {
        storage[n / 32].fetch_and(~(0b1U << (n % 32)), std::memory_order_relaxed);
    }

    /**
     * Returns the nth bit.
     */
    uint32_t operator[] (size_t i) const {
        return (uint32_t)storage[i / 32] & (0b1U << (i % 32));
    }
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* BIT_ARRAY_H */
