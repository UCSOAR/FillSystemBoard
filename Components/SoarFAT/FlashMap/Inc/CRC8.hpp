/**
 ********************************************************************************
 * @file    CRC8.hpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    October 25, 2026
 * @brief   Fast standard CRC-8 algorithm that uses a polynomial of 0x07 and an
 *          initial value of 0x00.
 ********************************************************************************
 */

#ifndef CRC8_H
#define CRC8_H

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

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

uint8_t CRC8(const uint8_t *data, size_t n);

#endif /* CRC8_H */
