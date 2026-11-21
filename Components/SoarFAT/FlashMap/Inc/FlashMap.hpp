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

#include "BitArray.hpp"
#include "CircularBuffer.hpp"
#include "Flash.hpp"

#include <stdint.h>
#include <atomic>

/************************************
 * MACROS AND DEFINES
 ************************************/

#define DIV_CEIL(x,y) (((x) + (y) - 1) / (y))

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/

template <class F>
class FlashMap {
private:
    /**
     * Two separate map sections are stored in the flash to aid with data
     * integrity.
     */
    enum class MapLabel: uint8_t {
        MapA,
        MapB,
        NumMaps,
    };

    /**
     * Indicates the operation performed on the flash chip for the purpose of
     * logging.
     */
    enum class LogType : uint8_t {
        LogWrite,
        LogErase,
    };

    /**
     * Each log entry consists of a type, 48 bits of metadata, and an 8 bit CRC.
     */
    struct LogEntry {
        LogType type;
        uint8_t bytes[6];
        uint8_t crc;
    } __attribute__((packed));

    /**
     * Number stored in sector zero to verify if flash has been formatted.
     */
    static constexpr uint32_t MAGIC_NUMBER = 0xAFA7FACE;

    /**
     * The real number of sectors on the flash device.
     */
    static constexpr size_t NUM_PHYSICAL_SECTORS =
        F::CAPACITY_BYTES / F::SECTOR_SIZE_BYTES;

    /**
     * The total size of metadata stored per sector, in bits.
     * NOTE: This must be updated whenever the metadata requirements change.
     *
     * Currently we have:
     *  - Array of physical_sector (16 bits)
     *  - Array of is_free (1 bit)
     *  - Array of is_stale (1 bit)
     *  - uint16_t CircularBuffer of sector_seq (16 bit)
     */
    static constexpr uint32_t CLASS_BITS_PER_SECTOR = 34;

    /**
     * The number of sectors at the start of flash allocated for the sector
     * mapping metadata, including the per-sector class bits and a 32-bit
     * sequence number.
     */
    static constexpr size_t NUM_SECTORS_PER_MAP = DIV_CEIL(
        NUM_PHYSICAL_SECTORS * CLASS_BITS_PER_SECTOR + 32,
        F::SECTOR_SIZE_BYTES * 8);

    /**
     * The number of sectors to using for tracking changes made to the map
     * before committing them.
     * The number of sectors used is to level wear between them and the data
     * sectors.
     */
    static constexpr size_t NUM_LOG_SECTORS = DIV_CEIL(
        NUM_PHYSICAL_SECTORS, DIV_CEIL(F::SECTOR_SIZE_BYTES, sizeof(LogEntry)));

    /**
     * The sector number of the first log sector.
     */
    static constexpr size_t LOG_SECTOR_OFFSET =
        1 + NUM_SECTORS_PER_MAP * static_cast<uint8_t>(MapLabel::NumMaps);

    /**
     * The sector number of the first data sector.
     */
    static constexpr size_t DATA_SECTOR_OFFSET =
        LOG_SECTOR_OFFSET + NUM_LOG_SECTORS;

    /**
     * The number of data sectors to reserve as empty space to increase
     * performance with wear leveling.
     */
    static constexpr size_t NUM_OVERPROVISIONED_SECTORS = 0;

    /**
     * The number of physical sectors used for data storage.
     */
    static constexpr size_t NUM_DATA_SECTORS_PHYSICAL =
        NUM_PHYSICAL_SECTORS -
        DATA_SECTOR_OFFSET;

    /**
     * The number of user-indexable logical sectors for data storage. Used for the index of
     * logical sectors and their physical sector equivalents.
     */
    static constexpr size_t NUM_DATA_SECTORS_LOGICAL =
        NUM_DATA_SECTORS_PHYSICAL - NUM_OVERPROVISIONED_SECTORS;

    /**
     * Physical sector associated with the logical index. Has 0 if there is
     * currently no association.
     * Indexed by logical sector.
     */
    std::atomic_uint16_t physical_sector[NUM_DATA_SECTORS_LOGICAL];

    /**
     * Flag that marks whether a physical sector is currently referenced by a
     * logical sector.
     * Indexed by physical sector.
     */
    BitArray<NUM_DATA_SECTORS_PHYSICAL> is_free;

    /**
     * Flag that marks whether the sector sector contains stale data and needs
     * to be erased.
     * Indexed by physical sector.
     */
    BitArray<NUM_DATA_SECTORS_PHYSICAL> is_stale;

    /**
     * A queue used for maintaining near round-robin distribution when selecting
     * a physical sector to write to.
     * Indexed by physical sector.
     */
    CircularBuffer<std::atomic_uint16_t, NUM_DATA_SECTORS_PHYSICAL> sector_seq;

    /**
     * Indicates which copy of the map is currently being use. Swaps every time
     * the logs are commited to the map.
     */
    std::atomic<MapLabel> cur_map;

    /**
     * The next index in the log sectors where a log must be written.
     */
    std::atomic_uint16_t log_idx;

    /**
     * The flash device to use.
     */
    Flash<F::CAPACITY_BYTES, F::SECTOR_SIZE_BYTES>& dev;

    /**
     * Buffer used for sector reads/writes
     */
    uint8_t buf[F::SECTOR_SIZE_BYTES];

    // Global variables
    bool initialized = false;

public:
    FlashMap(Flash<F::CAPACITY_BYTES, F::SECTOR_SIZE_BYTES>& dev) : dev(dev) { };

    /**
     * Initializes the flash chip. Must be called before anything else.
     * Confirms the validity of sector 0, and loads the sector metadata into
     * RAM if valid.
     */
    void Init(void);

    /**
     * Erases the flash chip and writes the required static metadata to sector 0.
     */
    void Format(void);

    /**
     * Checks that the flash chip is formatted correctly by comparing the actual
     * and expected Sector 0 bytes.
     */
    bool IsValid(void);

    /**
     * Erases the flash chip.
     */
    void Erase(void);
private:
    /**
     * Finds a new random physical sector using the MurmurHash3 mixer finalizer.
     * See https://en.wikipedia.org/wiki/MurmurHash
     */
    uint32_t NewSector(void);

    int WriteMapDatum(size_t byte_offset, size_t datum_size_bytes, uint32_t datum);

    int ReadMapDatum(size_t byte_offset, size_t datum_size_bytes, void *ret);

    void WriteInitialMap(MapLabel map);

    void PrintBuf(void);

    /**
     * Returns the physical sector number of the starting sector of a map.
     * The sector number will always be at least 1 because sector 0 is reserved
     * for static flash metadata.
     */
    static constexpr size_t MapOffset(MapLabel map);
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#include "FlashMap.ipp"

#endif /* FLASH_MAP_HPP */
