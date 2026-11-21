/**
 ********************************************************************************
 * @file    FlashMX66XX.hpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    November 18, 2026
 * @brief   Flash interface adapter for the existing MX66XX flash driver.
 ********************************************************************************
 */

#ifndef FLASH_MX66XX
#define FLASH_MX66XX

/************************************
 * INCLUDES
 ************************************/

#include "Flash.hpp"

#include <stdint.h>

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/

class FlashMX66XX: public Flash<128 * 1024 * 1024, 4096> {
public:
    /**
     * Returns a singleton for the flash driver strategy because the base driver
     * is written without instancing.
     */
    static FlashMX66XX& Inst(void);

    int Init(void) override;

    int Write(const uint8_t *buf, uint32_t sector_address, uint32_t byte_offset,
              uint32_t n) override;

    int Read(uint8_t *buf, uint32_t sector_address, uint32_t byte_offset,
             uint32_t n) override;

    int EraseSector(uint32_t sector_address) override;

    int EraseChip(void) override;

    bool IsBusy(void) override;

private:
    FlashMX66XX(void) { };
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FLASH_MX66XX */
