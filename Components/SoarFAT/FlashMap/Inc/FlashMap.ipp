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

    Erase();

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
    dev.Read(buf, 0, 0, F::SECTOR_SIZE_BYTES); 
    PrintBuf();

    uint32_t read_magic_number = (buf[3] << 24) |
                                 (buf[2] << 16) |
                                 (buf[1] << 8)  |
                                 (buf[0]);

    uint8_t read_crc = buf[25];
    uint8_t calculated_crc = CRC8(buf, 25);

    SOAR_PRINT("Read CRC: %02X\n", read_crc);
    SOAR_PRINT("Caclculated CRC: %02X\n", calculated_crc);

    if ((read_magic_number != MAGIC_NUMBER) || (read_crc != calculated_crc)) {
        return false;
    }

    return true;
}

template <class F>
void FlashMap<F>::Erase(void) {
    SOAR_PRINT("FlashMap::Erase() - Chip erase started\n");
    dev.EraseChip();
    SOAR_PRINT("FlashMap::Erase() - Chip erase finished\n");
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
int FlashMap<F>::WriteMapDatum(size_t byte_offset, size_t datum_size_bytes, uint32_t datum) {
    size_t total_bytes = NUM_SECTORS_PER_MAP * F::SECTOR_SIZE_BYTES;
    if (byte_offset >= total_bytes) return -1;

    size_t map_sector = byte_offset / F::SECTOR_SIZE_BYTES;
    size_t map_offset = byte_offset % F::SECTOR_SIZE_BYTES;

    dev.Write((uint8_t *)&datum, map_sector, map_offset, datum_size_bytes);

    return 0;
}

template <class F>
int FlashMap<F>::ReadMapDatum(size_t byte_offset, size_t datum_size_bytes, void *ret) {
    size_t total_bytes = NUM_SECTORS_PER_MAP * F::SECTOR_SIZE_BYTES;
    if (byte_offset >= total_bytes) return -1;

    size_t map_sector = byte_offset / F::SECTOR_SIZE_BYTES;
    size_t map_offset = byte_offset % F::SECTOR_SIZE_BYTES;

    dev.Read((uint8_t *)ret, map_sector, map_offset, datum_size_bytes);

    return 0;
}

template <class F>
void FlashMap<F>::WriteInitialMap(MapLabel map) {
    size_t map_offset = MapOffset(map);
   
    // Write initial map sequence number
    size_t cur_byte = map_offset;
    WriteMapDatum(cur_byte, 4, static_cast<uint8_t>(map));
    cur_byte += 4;

    // Write logical to physical sector mappings to 0.
    for (size_t i = 0; i < NUM_DATA_SECTORS_LOGICAL; i++) {
        WriteMapDatum(cur_byte, 2, 0);
        cur_byte += 2;
    }

    // Mark all data sectors free
    for (size_t i = 0; i < is_free.BackingArraySize(); i++) {
        WriteMapDatum(cur_byte, 4, 0xFFFFFFFF);
        cur_byte += 4;
    }

    // Mark data sectors not stale.
    for (size_t i = 0; i < is_stale.BackingArraySize(); i++) {
        WriteMapDatum(cur_byte, 4, 0);
        cur_byte += 4;
    }

    // Write sector initial sector queue
    for (size_t i = 0; i < NUM_DATA_SECTORS_PHYSICAL; i--) {
        WriteMapDatum(cur_byte, 2, i);
        cur_byte += 2;
    }
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
