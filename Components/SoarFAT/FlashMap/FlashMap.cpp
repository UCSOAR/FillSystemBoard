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

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

static uint8_t *BufPut(uint8_t *buf, void *src, size_t n);

/************************************
 * FUNCTION DEFINITIONS
 ************************************/

FlashMap& FlashMap::Inst(void) {
    static FlashMap inst;
    return inst;
}

FlashMap::FlashMap(void) { }

void FlashMap::Init(void) {
    mutex.Lock();

    InitSequence();

    initialized = false;

    if (!MX66xxQSPI_Init()) {
        SOAR_PRINT("FlashMap::InitFlash() - MX66 init failed\n");
        return;
    }

    uint8_t status = MX66xxQSPI_ReadStatusRegister();
    SOAR_PRINT("FlashMap::Init() - Status1: 0x%02x\n", status);

    uint32_t id = MX66xxQSPI_QPIReadID();
    SOAR_PRINT("FlashMap::Init() - MX66 ReadID: 0x%06lx\n", id);

    // Load map into RAM and patch it with the log contents
    
    initialized = true;

    mutex.Unlock();
}

void FlashMap::InitSequence(void) {
    MX66xxQSPI_ReleaseFromDeepPowerDown();
    MX66xxQSPI_RSTEN();
    MX66xxQSPI_RST();
    MX66xxQSPI_EQIO_1LINE();
    MX66xxQSPI_EN4B();
}

void FlashMap::Format(void) {
    mutex.Lock();

    SOAR_PRINT("FlashMap::Format() - Chip erase started\n");
    MX66xxQSPI_EraseChip();
    SOAR_PRINT("FlashMap::Format() - Chip erase finished\n");

    uint8_t buf[SECTOR_SIZE_BYTES] = { 0 };

    uint8_t num_maps = NUM_MAPS;

    uint8_t *ptr = buf;
    ptr = BufPut(ptr, (void *)&MAGIC_NUMBER, sizeof(MAGIC_NUMBER));
    ptr = BufPut(ptr, (void *)&num_maps, sizeof(num_maps));
    ptr = BufPut(ptr, (void *)&NUM_SECTORS_PER_MAP, sizeof(NUM_SECTORS_PER_MAP));

    uint8_t crc = CRC8(buf, ptr - buf);
    BufPut(ptr, (void *)&crc, sizeof(crc));

    SOAR_PRINT("FlashMap::Format() - Writing sector 0\n");
    MX66xxQSPI_WriteSector(buf, 0, 0, SECTOR_SIZE_BYTES);
    SOAR_PRINT("FlashMap::Format() - Finished writing sector 0\n");

    mutex.Unlock();
}

void Trim(uint32_t timeout_ms) {
    // Erase sectors on the flash chip with that are flagged as free and dirty

    SOAR_PRINT("FlashMap::Trim() - Not implemented\n");
}

void FlashMap::SerializeMapSectorsToFlash(void) {
    if (!initialized) {
        SOAR_PRINT("FlashMap::SerializeMapSectorsToFlash() - MX66 not initialized\n");
        return;
    }

    SOAR_PRINT("FlashMap::SerializeMapSectorsToFlash() - Not implemented\n");
}

void FlashMap::DeserializeMapSectorsFromFlash(void) {
    if (!initialized) {
        SOAR_PRINT("FlashMap::DeserializeMapSectorsFromFlash() - MX66 not initialized\n");
        return;
    }

    SOAR_PRINT("FlashMap::DeserializeMapSectorsFromFlash() - Not implemented\n");
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

constexpr size_t FlashMap::MapOffset(map_label_e map) {
    return 1 + static_cast<size_t>(map) * NUM_SECTORS_PER_MAP;
}

static uint8_t *BufPut(uint8_t *buf, void *src, size_t n) {
    return (uint8_t *)memcpy(buf, src, n) + n;
}
