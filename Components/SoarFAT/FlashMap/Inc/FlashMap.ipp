/**
 ********************************************************************************
 * @file    FlashMap.ipp
 * @author  Jacob Plourde (Jacobplo)
 * @date    October 24, 2026
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/

#include "FlashMap.hpp"
#include "CRC8.hpp"

#include "CubeDefines.hpp"
#include "stm32h7xx_hal.h"
#include "mx66xx_qspi.hpp"

#include <cstring>
#include <stdio.h>

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

static inline uint8_t *BufPut(uint8_t *buf, void *src, size_t n);
#define BufPush(buf, src) BufPut((buf), (void *)&(src), sizeof(src))

/************************************
 * FUNCTION DEFINITIONS
 ************************************/

template <class F>
void FlashMap<F>::Init(void) {
    initialized = false;

    if (dev.Init() < 0) {
        SOAR_PRINT("FlashMap::InitFlash() - init failed\n");
        return;
    }

    // Read Sector 0 and confirm contents are as expected
    if (!IsValid()) {
      SOAR_PRINT("FlashMap::Init() - Unrecognized flash format. Call the "
                 "Format() method once before initalizing\n");
      return;
    };
    
    initialized = true;
}

template <class F>
void FlashMap<F>::Format(void) {
    if (dev.Init() < 0) {
        SOAR_PRINT("FlashMap::Format() - init failed\n");
        return;
    }

    EraseChip();

    uint8_t num_maps = static_cast<uint8_t>(MapLabel::NumMaps);

    // Write static metadata to physical sector 0 of the flash for verification.
    uint8_t *ptr = buf;
    ptr = BufPush(ptr, MAGIC_NUMBER);
    ptr = BufPush(ptr, NUM_PHYSICAL_SECTORS);
    ptr = BufPush(ptr, num_maps);
    ptr = BufPush(ptr, NUM_SECTORS_PER_MAP);
    ptr = BufPush(ptr, NUM_LOG_SECTORS);
    ptr = BufPush(ptr, NUM_DATA_SECTORS_PHYSICAL);
    ptr = BufPush(ptr, NUM_OVERPROVISIONED_SECTORS);

    uint8_t crc = CRC8(buf, ptr - buf);
    BufPush(ptr, crc);

    SOAR_PRINT("FlashMap::Format() - Writing sector 0\n");
    dev.Write(buf, 0, 0, F::SECTOR_SIZE_BYTES);
    SOAR_PRINT("FlashMap::Format() - Finished writing sector 0\n");

    // Write base metadata the the map sector
    SOAR_PRINT("FlashMap::Format() - Writing map metadata\n");
    WriteInitialMap(MapLabel::MapA);
    WriteInitialMap(MapLabel::MapB);
    SOAR_PRINT("FlashMap::Format() - Finished writing map metadata\n");
}

template <class F>
bool FlashMap<F>::IsValid(void) {
    //for (size_t i = 0; i < 2 * NUM_SECTORS_PER_MAP + 1; i++) {
    //    dev.Read(buf, i, 0, F::SECTOR_SIZE_BYTES); 
    //    PrintBuf();
    //}
    dev.Read(buf, 0, 0, F::SECTOR_SIZE_BYTES);
    PrintBuf();

    uint32_t read_magic_number = (buf[3] << 24) |
                                 (buf[2] << 16) |
                                 (buf[1] << 8)  |
                                 (buf[0]);

    uint8_t read_crc = buf[25];
    uint8_t calculated_crc = CRC8(buf, 25);

    SOAR_PRINT("NUM_DATA_SECTORS_LOGICAL: %d\n", NUM_DATA_SECTORS_LOGICAL);
    SOAR_PRINT("NUM_SECTORS_PER_MAP: %d\n", NUM_SECTORS_PER_MAP);
    SOAR_PRINT("Read CRC: %02X\n", read_crc);
    SOAR_PRINT("Caclculated CRC: %02X\n", calculated_crc);

    if ((read_magic_number != MAGIC_NUMBER) || (read_crc != calculated_crc)) {
        return false;
    }

    return true;
}

template <class F>
void FlashMap<F>::EraseChip(void) {
    SOAR_PRINT("FlashMap::EraseChip() - Chip erase started\n");
    dev.EraseChip();
    SOAR_PRINT("FlashMap::EraseChip() - Chip erase finished\n");
}

template <class F>
uint32_t FlashMap<F>::NewSector(void) {
    uint32_t h = HAL_GetTick();

    h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

    return h % NUM_DATA_SECTORS_PHYSICAL;
}

template <class F>
void FlashMap<F>::WriteInitialMap(MapLabel map) {
    size_t write_sector = MapOffset(map);

    uint16_t u16;
    uint32_t u32;
    uint8_t *ptr = buf;

// Writes the buffer to flash when it is full
#define check_and_flush_buf() do {                                  \
    if (static_cast<size_t>(ptr - buf) >= F::SECTOR_SIZE_BYTES) {   \
        dev.Write(buf, write_sector++, 0, F::SECTOR_SIZE_BYTES);    \
        ptr = buf;                                                  \
    }                                                               \
} while (0)
   
    // Write initial map sequence number
    u32 = static_cast<uint32_t>(map);
    ptr = BufPush(ptr, u32);

    // Write logical to physical sector mappings to 0.
    u16 = 0;
    for (size_t i = 0; i < NUM_DATA_SECTORS_LOGICAL; i++) { 
        check_and_flush_buf();

        ptr = BufPush(ptr, u16);
    }

    // Mark all data sectors free
    u32 = 0xFFFFFFFF;
    for (size_t i = 0; i < is_free.BackingArraySize(); i++) {
        check_and_flush_buf();

        ptr = BufPush(ptr, u32);
    }

    // Mark data sectors not stale.
    u32 = 0;
    for (size_t i = 0; i < is_stale.BackingArraySize(); i++) {
        check_and_flush_buf();

        ptr = BufPush(ptr, u32);
    }

    // Write sector initial sector queue
    for (u16 = 0; u16 < NUM_DATA_SECTORS_PHYSICAL; u16++) {
        check_and_flush_buf();

        ptr = BufPush(ptr, u16);
    }

    // Write remaining data.
    if (ptr - buf > 0) {
       dev.Write(buf, write_sector++, 0, ptr - buf); 
    }

#undef check_and_flush_buf
}

template <class F>
void FlashMap<F>::PrintBuf(void) {
    size_t i = 0;
    while (i < F::SECTOR_SIZE_BYTES) {
        for (size_t j = 0; j < 32; j++) {
            SOAR_PRINT("%02X ", buf[i++]);
        }
        SOAR_PRINT("\n");
    }
}

template <class F>
constexpr size_t FlashMap<F>::MapOffset(MapLabel map) {
    return 1 + static_cast<size_t>(map) * NUM_SECTORS_PER_MAP;
}

static inline uint8_t *BufPut(uint8_t *buf, void *src, size_t n) {
    return (uint8_t *)memcpy(buf, src, n) + n;
}
