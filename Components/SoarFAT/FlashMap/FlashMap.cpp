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

#include "mx66xx_qspi.hpp"
#include "CubeDefines.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

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

    MX66xxQSPI_ReleaseFromDeepPowerDown();
    MX66xxQSPI_RSTEN();
    MX66xxQSPI_RST();
    MX66xxQSPI_EQIO_1LINE();
    MX66xxQSPI_EN4B();

    initialized = false;

    if (!MX66xxQSPI_Init()) {
        SOAR_PRINT("FlashMap::InitFlash() - MX66 init failed\n");
        return;
    }

    uint8_t status = MX66xxQSPI_ReadStatusRegister();
    SOAR_PRINT("FlashMap::Init() - Status1: 0x%02x\n", status);

    uint32_t id = MX66xxQSPI_QPIReadID();
    SOAR_PRINT("FlashMap::Init() - MX66 ReadID: 0x%06lx\n", id);

    // Load map into RAM
    
    // Mark all sectors as not dirty.    
    for (uint16_t i = 0; i < NUM_LOGICAL_SECTORS; i++) {
        is_dirty_logical[i] = false; 
    }

    initialized = true;
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
