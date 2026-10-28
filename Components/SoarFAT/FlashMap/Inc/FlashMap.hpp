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

/************************************
 * CLASS DEFINITIONS
 ************************************/

class FlashMap {
private:
    /**
     * Two separate map sections are stored in the flash to aid with data
     * integrity.
     */
    typedef enum mab_label_e {
        MAP_A,
        MAP_B,
        NUM_MAPS,
    } map_label_e;

    /**
     * Number stored in sector zero to verify if flash has been formatted.
     */
    static constexpr uint32_t MAGIC_NUMBER = 0xAFA7FACE;

    /**
     * The real number of sectors on the flash device.
     */
    static constexpr size_t NUM_PHYSICAL_SECTORS =
        FLASH_CAPACITY_BYTES / SECTOR_SIZE_BYTES;

    /**
     * The total size of metadata stored per sector, in bits.
     * NOTE: This must be updated whenever the metadata requirements change.
     *
     * Currently we have:
     *  - Array of physical_sector (16 bits)
     *  - Array of is_free (1 bit)
     *  - Array of is_dirty (1 bit)
     */
    static constexpr uint32_t CLASS_BITS_PER_SECTOR = 18;

    /**
     * The number of sectors at the start of flash allocated for the sector
     * mapping metadata, including the per-sector class bits and a 32-bit
     * sequence number.
     */
    static constexpr size_t NUM_SECTORS_PER_MAP = DIV_CEIL(
        NUM_PHYSICAL_SECTORS * CLASS_BITS_PER_SECTOR + 32,
        SECTOR_SIZE_BYTES * 8);

    /**
     * The number of sectors to using for tracking changes made to the map
     * before committing them.
     */
    static constexpr size_t NUM_LOG_SECTORS = 1;

    /**
     * The sector number of the first log sector.
     */
    static constexpr size_t LOG_SECTOR_OFFSET =
        1 + NUM_SECTORS_PER_MAP * NUM_MAPS;

    /**
     * The sector number of the first data sector.
     */
    static constexpr size_t DATA_SECTOR_OFFSET =
        LOG_SECTOR_OFFSET + NUM_LOG_SECTORS;

    /**
     * The number of available sectors for data storage. Used for the index of
     * logical sectors and their physical sector equivalents.
     */
    static constexpr size_t NUM_DATA_SECTORS =
        NUM_PHYSICAL_SECTORS - DATA_SECTOR_OFFSET;

    /**
     * Physical sector associated with the logical index. Has 0 if there is
     * currently no association.
     * Indexed by logical sector.
     */
    std::atomic_uint16_t physical_sector[NUM_DATA_SECTORS];

    /**
     * Flag that marks whether a physical sector is currently referenced by a
     * logical sector.
     * Indexed by physical sector.
     */
    BitArray<NUM_DATA_SECTORS> is_free;

    /**
     * Flag that marks whether the sector sector needs to be erased.
     * Indexed by physical sector.
     */
    BitArray<NUM_DATA_SECTORS> is_dirty;

    // Global variables
    bool initialized = false;
    Mutex mutex;

    map_label_e cur_map;
    uint32_t seq_number;
    size_t log_idx;

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

    /**
     * Returns the physical sector number of the starting sector of a map.
     * The sector number will always be at least 1 because sector 0 is reserved
     * for static flash metadata.
     */
    static constexpr size_t MapOffset(map_label_e map);
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FLASH_MAP_HPP */
