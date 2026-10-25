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

#include "Mutex.hpp"
#include "BitArray.hpp"

#include <stdint.h>
#include <atomic>

/************************************
 * MACROS AND DEFINES
 ************************************/

constexpr size_t NUM_PHYSICAL_SECTORS = 32768;

// The total size of data stored per-sector in the reserved area is
// represented by n sectors, where n is the number of bits used per-sector in
// the map. Set NUM_MAP_SECTORS = n
constexpr size_t NUM_MAP_SECTORS = 19;
constexpr size_t NUM_LOGICAL_SECTORS = NUM_PHYSICAL_SECTORS - NUM_MAP_SECTORS;

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/

class FlashMap {
private:
    /**
     * Physical sector associated with the logical index. Has 0 if there is
     * currently no association. 
     * Indexed by logical sector.
     */
    std::atomic_uint16_t physical_sector[NUM_LOGICAL_SECTORS];

    /**
     * Flag that marks whether the sector has been modified and needs to be
     * written to the flash.
     * Indexed by logical sector.
     */
    BitArray<NUM_LOGICAL_SECTORS> is_dirty_logical;

    /**
     * Flag that marks whether a physical sector is currently referenced by a
     * logical sector.
     * Indexed by physical sector.
     */
    BitArray<NUM_LOGICAL_SECTORS> is_free;

    /**
     * Flag that marks whether the sector sector needs to be erased.
     * Indexed by physical sector.
     */
    BitArray<NUM_LOGICAL_SECTORS> is_dirty_physical;

    // Global variables
    bool initialized = false;
    Mutex mutex;

public:
    static FlashMap& Inst(void);
    void Init(void);
    void Trim(uint32_t timeout_ms);
private:
    FlashMap(void);
    void SerializeMapSectorsToFlash(void);
    void DeserializeMapSectorsFromFlash(void);
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FLASH_MAP_HPP */
