/**
 ********************************************************************************
 * @file    FlashStrategyMX66XX.cpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    November 18, 2026
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/

#include "FlashStrategyMX66XX.hpp"

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

FlashStrategyMX66XX& FlashStrategyMX66XX::Inst(void) {
    static FlashStrategyMX66XX inst;
    return inst;
}

int FlashStrategyMX66XX::Init(void) 
{
    if (!MX66xxQSPI_Init()) {
        return -1;
    }

    uint8_t status = MX66xxQSPI_ReadStatusRegister();
    SOAR_PRINT("FlashMap::Init() - Status1: 0x%02x\n", status);

    uint32_t id = MX66xxQSPI_QPIReadID();
    SOAR_PRINT("FlashMap::Init() - MX66 ReadID: 0x%06lx\n", id);

    return 0;
}

int FlashStrategyMX66XX::Write(const uint8_t *buf, uint32_t sector_address,
                               uint32_t byte_offset, uint32_t n)
{

}

int FlashStrategyMX66XX::Read(uint8_t *buf, uint32_t sector_address,
                              uint32_t byte_offset, uint32_t n)
{

}


int FlashStrategyMX66XX::EraseSector(uint32_t sector_address)
{

}

int FlashStrategyMX66XX::EraseChip(void)
{

}

bool FlashStrategyMX66XX::IsWriting(void)
{

}
