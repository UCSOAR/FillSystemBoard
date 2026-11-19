/**
 ********************************************************************************
 * @file    FlashStrategy.hpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    November 18, 2026
 * @brief   Defines an interface that flash methods must conform to so that they
 *          can be used with the FlashMap module.
 ********************************************************************************
 */

#ifndef FLASH_STRATEGY_H
#define FLASH_STRATEGY_H

/************************************
 * INCLUDES
 ************************************/

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

class FlashStrategy {
public:
    FlashStrategy() { };

    virtual ~FlashStrategy() { };

    /**
     * Initializes the flash chip.
     */
    virtual int Init(void) = 0;

    /**
     * Writes data to the flash chip specified by the parameters below.
     *
     * @param buf               The data to be written to the flash
     * @param sector_address    The (erase) sector address for the sector to be written to.
     * @param byte_offset       The byte offset within the sector to write to.
     * @param n                 The number of bytes to write.
     * 
     * @return 0 on success, -1 on failure.
     */
    virtual int Write(const uint8_t *buf, uint32_t sector_address,
                      uint32_t byte_offset, uint32_t n) = 0;

    /**
     * Reads data from the flash chip specified by the parameters below.
     *
     * @param buf               The data buffer to read the flash data into.
     * @param sector_address    The (erase) sector address for the sector to be read from.
     * @param byte_offset       The byte offset within the sector to read from.
     * @param n                 The number of bytes to read.
     * 
     * @return 0 on success, -1 on failure.
     */
    virtual int Read(uint8_t *buf, uint32_t sector_address,
                     uint32_t byte_offset, uint32_t n) = 0;

    /**
     * Erases the sector specified by the sector address.
     *
     * @param sector_address    The sector address to erase.
     *
     * @return 0 on success, -1 on failure.
     */
    virtual int EraseSector(uint32_t sector_address) = 0;

    /**
     * Erases the entire flash chip.
     *
     * @return 0 on success, -1 on failure.
     */
    virtual int EraseChip(void) = 0;

    /**
     * Checks if the flash chip is currently writing.
     */
    virtual bool IsWriting(void) = 0;
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FLASH_STRATEGY_H */
