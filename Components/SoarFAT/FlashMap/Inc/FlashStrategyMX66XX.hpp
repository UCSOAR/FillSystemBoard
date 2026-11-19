/**
 ********************************************************************************
 * @file    FlashStrategyMX66XX.hpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    November 18, 2026
 * @brief   Flash strategy adapter for the existing MX66XX flash driver.
 ********************************************************************************
 */

#ifndef FLASH_STRATEGY_MX66XX
#define FLASH_STRATEGY_MX66XX

/************************************
 * INCLUDES
 ************************************/

#include "FlashStrategy.hpp"

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

class FlashStrategyMX66XX: public FlashStrategy {
public:
    /**
     * Returns a singleton for the flash driver strategy because the base driver
     * is written without instancing.
     */
    static FlashStrategyMX66XX& Inst(void);

    int Init(void) override;

    int Write(const uint8_t *buf, uint32_t sector_address, uint32_t byte_offset,
              uint32_t n) override;

    int Read(uint8_t *buf, uint32_t sector_address, uint32_t byte_offset,
             uint32_t n) override;

    int EraseSector(uint32_t sector_address) override;

    int EraseChip(void) override;

    bool IsWriting(void) override;

private:
    FlashStrategyMX66XX(void) { };
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FLASH_STRATEGY_MX66XX */
