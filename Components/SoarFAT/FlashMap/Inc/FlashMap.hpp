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

#define FLASH_CAPACITY_BYTES 134217728
#define SECTOR_SIZE_BYTES    4096

#define DIV_CEIL(x,y) (((x) + (y) - 1) / (y))

/************************************
 * TYPEDEFS
 ************************************/

static constexpr size_t NUM_PHYSICAL_SECTORS = FLASH_CAPACITY_BYTES / SECTOR_SIZE_BYTES;

/************************************
 * CLASS DEFINITIONS
 ************************************/

class FlashMap {
private:
    /**
     * The total size of metadata stored per sector, in bits.
     * NOTE: This must be updated whenever the metadata requirements change.
     */
    static constexpr uint32_t CLASS_BITS_PER_SECTOR = 19;

    /**
     * The number of sectors at the start of flash allocated for sector
     * metadata.
     */
    static constexpr size_t NUM_MAP_SECTORS =
        DIV_CEIL(NUM_PHYSICAL_SECTORS * CLASS_BITS_PER_SECTOR, SECTOR_SIZE_BYTES * 8);

    /**
     * The number of available sectors for data storage. Used for the index of
     * logical sectors and their physical sector equivalents.
     */
    static constexpr size_t NUM_LOGICAL_SECTORS =
        NUM_PHYSICAL_SECTORS - NUM_MAP_SECTORS;

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

    /**
     * Finds a new random physical sector using the MurmurHash3 mixer finalizer.
     * See https://en.wikipedia.org/wiki/MurmurHash
     */
    static uint32_t NewSector(void);
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FLASH_MAP_HPP */
