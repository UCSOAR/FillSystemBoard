/**
 ********************************************************************************
 * @file    FlashMap.cpp
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

uint8_t FlashMap::buf[SECTOR_SIZE_BYTES] = { 0 };

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

static inline uint8_t *BufPut(uint8_t *buf, void *src, size_t n);
#define BufPush(buf, src) BufPut((buf), (void *)&(src), sizeof(src))

static void MX66xxQSPI_ReadSector_Chunked(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead);
static void MX66xxQSPI_WriteSector_Chunked(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite);

/************************************
 * FUNCTION DEFINITIONS
 ************************************/

FlashMap& FlashMap::Inst(void) {
    static FlashMap inst;
    return inst;
}

void FlashMap::Init(void) {
    initialized = false;

    if (!MX66xxQSPI_Init()) {
        SOAR_PRINT("FlashMap::InitFlash() - MX66 init failed\n");
        return;
    }

    uint8_t status = MX66xxQSPI_ReadStatusRegister();
    SOAR_PRINT("FlashMap::Init() - Status1: 0x%02x\n", status);

    uint32_t id = MX66xxQSPI_QPIReadID();
    SOAR_PRINT("FlashMap::Init() - MX66 ReadID: 0x%06lx\n", id);

    // Read Sector 0 and confirm contents are as expected
    if (!IsValid()) {
      SOAR_PRINT("FlashMap::Init() - Unrecognized flash format. Call the "
                 "Format() method once before initalizing\n");
      return;
    };
    
    initialized = true;
}

void FlashMap::Format(void) {
    if (!MX66xxQSPI_Init()) {
        SOAR_PRINT("FlashMap::Format() - MX66 init failed\n");
        return;
    }

    Erase();

    uint8_t num_maps = NUM_MAPS;

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
    MX66xxQSPI_WriteSector(buf, 0, 0, SECTOR_SIZE_BYTES);
    SOAR_PRINT("FlashMap::Format() - Finished writing sector 0\n");

    // Write base metadata the the map sector
    SOAR_PRINT("FlashMap::Format() - Writing map metadata\n");
    WriteInitialMap(MAP_A);
    WriteInitialMap(MAP_B);
    SOAR_PRINT("FlashMap::Format() - Finished writing map metadata\n");
}

bool FlashMap::IsValid(void) {
    MX66xxQSPI_ReadSector(buf, 0, 0, SECTOR_SIZE_BYTES); 
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

void FlashMap::Erase(void) {
    SOAR_PRINT("FlashMap::Erase() - Chip erase started\n");
    MX66xxQSPI_EraseChip();
    SOAR_PRINT("FlashMap::Erase() - Chip erase finished\n");
}

uint32_t FlashMap::NewSector(void) {
    uint32_t h = HAL_GetTick();

    h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

    return h % NUM_DATA_SECTORS_PHYSICAL;
}

int FlashMap::WriteMapDatum(size_t byte_offset, size_t datum_size_bytes, uint32_t datum) {
    size_t total_bytes = NUM_SECTORS_PER_MAP * SECTOR_SIZE_BYTES;
    if (byte_offset >= total_bytes) return -1;

    size_t map_sector = byte_offset / SECTOR_SIZE_BYTES;
    size_t map_offset = byte_offset % SECTOR_SIZE_BYTES;

    MX66xxQSPI_WriteSector((uint8_t *)&datum, map_sector, map_offset, datum_size_bytes);

    return 0;
}

int FlashMap::ReadMapDatum(size_t byte_offset, size_t datum_size_bytes, void *ret) {
    size_t total_bytes = NUM_SECTORS_PER_MAP * SECTOR_SIZE_BYTES;
    if (byte_offset >= total_bytes) return -1;

    size_t map_sector = byte_offset / SECTOR_SIZE_BYTES;
    size_t map_offset = byte_offset % SECTOR_SIZE_BYTES;

    MX66xxQSPI_ReadSector((uint8_t *)ret, map_sector, map_offset, datum_size_bytes);

    return 0;
}

void FlashMap::WriteInitialMap(map_label_e map) {
    size_t map_offset = MapOffset(map);
   
    // Write initial map sequence number
    size_t cur_byte = map_offset;
    WriteMapDatum(cur_byte, 4, map);
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

void FlashMap::PrintBuf(void) {
    size_t i = 0;
    while (i < SECTOR_SIZE_BYTES) {
        for (size_t j = 0; j < 32; j++) {
            SOAR_PRINT("%02X ", buf[i++]);
        }
        SOAR_PRINT("\n");
    }
}

constexpr size_t FlashMap::MapOffset(map_label_e map) {
    return 1 + static_cast<size_t>(map) * NUM_SECTORS_PER_MAP;
}

static inline uint8_t *BufPut(uint8_t *buf, void *src, size_t n) {
    return (uint8_t *)memcpy(buf, src, n) + n;
}

static void MX66xxQSPI_ReadSector_Chunked(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead) {
    uint32_t num_chunks = NumByteToRead / 256;
    uint32_t remaining = NumByteToRead % 256;

    uint32_t i;
    for (i = 0; i < num_chunks * 256; i += 256) {
        MX66xxQSPI_ReadSector(pBuffer + i, Sector_Address + (i / SECTOR_SIZE_BYTES), i % SECTOR_SIZE_BYTES, 256);
    }

    MX66xxQSPI_ReadSector(pBuffer + i, Sector_Address + (i / SECTOR_SIZE_BYTES), i % SECTOR_SIZE_BYTES, remaining);
}

static void MX66xxQSPI_WriteSector_Chunked(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite) {
    uint32_t num_chunks = NumByteToWrite / 256;
    uint32_t remaining = NumByteToWrite % 256;

    uint32_t i;
    for (i = 0; i < num_chunks * 256; i += 256) {
        MX66xxQSPI_WriteSector(pBuffer + i, Sector_Address + (i / SECTOR_SIZE_BYTES), i % SECTOR_SIZE_BYTES, 256);
    }

    if (remaining > 0) {
        MX66xxQSPI_WriteSector(pBuffer + i, Sector_Address + (i / SECTOR_SIZE_BYTES), i % SECTOR_SIZE_BYTES, remaining);
    }
}
