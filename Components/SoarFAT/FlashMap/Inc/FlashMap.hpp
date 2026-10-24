/**
 ********************************************************************************
 * @file    FlashMap.hpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    October 24, 2026
 * @brief   This class should be used to handle all flash operations when using
 *          the SoarFAT filesystem. It dynamically maps logical sectors to
 *          physical sectors on the flash to minimize wear on the flash chip
 *          caused by frequent erase operations to update filesystem metadata.
 ********************************************************************************
 */

#ifndef FLASH_MAP_HPP
#define FLASH_MAP_HPP

/************************************
 * INCLUDES
 ************************************/

#include <stdint.h>

/************************************
 * MACROS AND DEFINES
 ************************************/

constexpr uint16_t NUM_PHYSICAL_SECTORS = 32768;

// The total size of data stored per-sector in the reserved area is
// represented by n sectors, where n is the number of bits used per-sector.
// Set NUM_MAP_SECTORS = n
constexpr uint16_t NUM_MAP_SECTORS = 18;
constexpr uint16_t NUM_LOGICAL_SECTORS = NUM_PHYSICAL_SECTORS - NUM_MAP_SECTORS;

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/

class FlashMap {
private:
    // Map sector metadata
    uint16_t physical_sector[NUM_LOGICAL_SECTORS];
    bool is_free[NUM_LOGICAL_SECTORS];
    bool is_dirty[NUM_LOGICAL_SECTORS];

    // Global flags
    bool initialized = false;
public:
    static FlashMap& Inst(void);
    void Init(void);
private:
    FlashMap(void);
    void SerializeMapSectorsToFlash(void);
    void DeserializeMapSectorsFromFlash(void);
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FLASH_MAP_HPP */
