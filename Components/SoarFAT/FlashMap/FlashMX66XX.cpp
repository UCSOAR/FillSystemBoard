/**
 ********************************************************************************
 * @file    FlashMX66XX.cpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    November 18, 2026
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/

#include "FlashMX66XX.hpp"

#include "CubeDefines.hpp"
#include "mx66xx_qspi.hpp"

#include <stdint.h>

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

FlashMX66XX& FlashMX66XX::Inst(void) {
    static FlashMX66XX inst;
    return inst;
}

int FlashMX66XX::Init(void) 
{
    if (!MX66xxQSPI_Init()) {
        return -1;
    }

    uint8_t status = MX66xxQSPI_ReadStatusRegister();
    SOAR_PRINT("FlashMX66XX::Init() - Status1: 0x%02x\n", status);

    uint32_t id = MX66xxQSPI_QPIReadID();
    SOAR_PRINT("FlashMX66XX::Init() - MX66 ReadID: 0x%06lx\n", id);

    return 0;
}

int FlashMX66XX::Write(const uint8_t *buf, uint32_t sector_address,
                               uint32_t byte_offset, uint32_t n)
{
    static int i = 0;
    MX66xxQSPI_WriteSector(const_cast<uint8_t *>(buf), sector_address, byte_offset, n);
    SOAR_PRINT("%d\n", i++);
    return 0;
}

int FlashMX66XX::Read(uint8_t *buf, uint32_t sector_address,
                              uint32_t byte_offset, uint32_t n)
{
    MX66xxQSPI_ReadSector(buf, sector_address, byte_offset, n);
    return 0;
}


int FlashMX66XX::EraseSector(uint32_t sector_address)
{
    MX66xxQSPI_EraseSector(sector_address);
    return 0;
}

int FlashMX66XX::EraseChip(void)
{
    MX66xxQSPI_EraseChip();
    return 0;
}

bool FlashMX66XX::IsBusy(void)
{
    return false;
}
