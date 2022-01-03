/**
 * @file
 * FatFs library adapter
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_FILESYSTEM_FATFSADAPTER_H
#define ZHELE_DRIVERS_FILESYSTEM_FATFSADAPTER_H

#include <spi.h>
#include <drivers/sdcard.h>

#include "fatfs/diskio.h"

namespace Zhele::Drivers::Filesystem
{
    /**
     * @brief FatFs adapter class for sdcard
     * 
     * @tparam _SdCardInstance SdCard instance
     */
    template<typename _SdCardInstance>
    class SdCardFatFsAdapter
    {
    public:
        static DSTATUS DiskInitialize()
        {
            return _SdCardInstance::CheckStatus()
                ? 0
                : STA_NODISK;
        }

        static DSTATUS DiskStatus()
        {
            return _SdCardInstance::CheckStatus() ? 0 : STA_NOINIT;
        }

        static DRESULT DiskRead(BYTE* buff, LBA_t sector, UINT count)
        {
            if(count == 1)
            {
                return _SdCardInstance::ReadBlock(buff, static_cast<uint32_t>(sector))
                    ? DRESULT::RES_OK
                    : DRESULT::RES_ERROR;
            }
            else
            {
                return _SdCardInstance::ReadMultipleBlock(buff, static_cast<uint32_t>(sector), count)
                    ? DRESULT::RES_OK
                    : DRESULT::RES_ERROR;
            }
        }
    };

    using SdCardReader = Drivers::SdCard<Spi1, IO::Pa4>;
    
    using SdCardAdapter = SdCardFatFsAdapter<SdCardReader>;
} // namespace Zhele::Drivers::Filesystem

#endif //! ZHELE_DRIVERS_FILESYSTEM_FATFSADAPTER_H