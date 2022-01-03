/**
 * @file
 * Driver for SD card
 * 
 * @author Konstantin Chizhov
 * @date 2012
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_SDCARD_H
#define ZHELE_DRIVERS_SDCARD_H

#include <delay.h>
#include <binary_stream.h>

namespace Zhele::Drivers
{
    /// SD card command
    enum SdCardCommand
    {
        GoIdleState = 0, ///< Software reset
        SendOpCond = 1, ///< Initiate initialization process
        SendIfCond = 8, ///< Check voltage range
        SendCsd = 9, ///< Read CSD register
        SendCid  = 10, ///< Read CID register
        StopTransmission = 12, ///< Stop to read data
        SendStatus = 13, ///< Ask card's status register
        SetBlockLength = 16, ///< Change R/W block size
        ReadSingleBlock = 17, ///< Read block
        ReadMultipleBlock = 18, ///< Read multiple blocks
        WriteBlock = 24, ///< Write block
        WriteMultipleBlock = 25, ///< Write multiple blocks
        ProgramCsd = 27, ///< Program CSD register
        SetWriteProt = 28, ///< Set write protection (if card support this feature)
        ClrWriteProt = 29, ///< Clear write protection (if card support this feature)
        SendWriteProt = 30, ///< Ask card write protection status (if card support this feature)
        EraseWrBlkStartAddr = 32, ///< Set the address of the first write block to be erased
        EraseWrBlkEndAddr = 33, ///< Set the address of the last write block of the continuous range to be erased
        Erase = 38, ///< Erases all previously selected write blocks
        AppCmd  = 55, ///< Leading command of ACMD<n> command
        GenCmd = 56, ///< Transfer data block
        ReadOcr = 58, ///< Read OCR register
        CrcOnOff = 59, ///< Turns CRC option on or off
        SdSendOpCond = 41 // ACMD41
    };

    /// R1 & R2 response bits
    enum SdR1R2ResponseBits
    {
        SdR1Idle           = (1 << 0), ///< Idle state
        SdR1EraseReset     = (1 << 1), ///< Erase reset
        SdR1IllegalCommand = (1 << 2), ///< Illegal command
        SdR1CrcError       = (1 << 3), ///< Crc error
        SdR1EraseSeqError  = (1 << 4), ///< Erase sequence error
        SdR1AddressError   = (1 << 5), ///< Address error
        SdR1ParameterError = (1 << 6), ///< Parameter error

        CardIsLocked  = (1 << 0), ///< Card locked
        WPEraseSkip   = (1 << 1), ///< WP erase skip
        Error         = (1 << 2), ///< Error
        CCError       = (1 << 3), ///< CC error
        CardECCFailed = (1 << 4), ///< Card ecc failed
        WPViolation   = (1 << 5), ///< WP violation
        EraseParam    = (1 << 6), ///< Erase param
        OutofRange    = (1 << 7) ///< Out of range
    };

    /// SD card type
    enum SdCardType
    {
        SdCardNone = 0, ///< None
        SdCardMmc  = 0x01, ///< MMC card
        SdCardV1   = 0x02, ///< SD card v1
        SdCardV2   = 0x04, ///< SD card v2
        SdhcCard   = 0x06 ///< SDHC card
    };

    /// SD card data error
    enum SdCardDataError
    {
        SdDataError = 0x01, ///< Data error
        SdDataCCError  = 0x02, ///< CC error
        SdDataECCError   = 0x04, ///< ECC error
        SdDataOutOfRangeError   = 0x08 ///< Out of range error
    };

    /**
     * @brief Implements SD card
     * 
     * @tparam _SpiModule SPI module
     * @tparam _CsPin Chip select pin
     */
    template<typename _SpiModule, typename _CsPin>
    class SdCard
    {
        static const uint16_t CommandTimeoutValue = 100; ///< Command timeout
        static const bool useCrc = false; ///< CRC using flag
        static SdCardType _type; ///< SD card type
        static BinaryStream<_SpiModule> Spi; ///< Binary stream
    
    protected:
        /**
         * @brief Execute spi command
         * 
         * @param index Index
         * @param arg Argument
         * @param crc Crc (default 0)
         * @return uint16_t Command result
         */
        static uint16_t SpiCommand(uint8_t index, uint32_t arg, uint8_t crc = 0);

        /**
         * @brief Read card blocks count
         * 
         * @return uint32_t Blocks count
         */
        static uint32_t ReadBlocksCount();

        /**
         * @brief Wait while bus is busy
         * 
         * @return true Bus free
         * @return false Timeout
         */
        static bool WaitWhileBusy();

        /**
         * @brief Read data block
         * 
         * @tparam ReadIterator Iterator type
         * 
         * @param iter Iterator
         * @param size Size to read
         * @return true Read success
         * @return false Read failed
         */
        template<typename ReadIterator>
        static bool ReadDataBlock(ReadIterator iter, size_t size)
        {
            _CsPin::Clear();

            uint8_t resp;
            resp = Spi.IgnoreWhile(1000, 0xFF);
            if(resp != 0xFE)
            {
                _CsPin::Set();
                return false;
            }
            Spi. template Read<ReadIterator>(iter, size);
            uint16_t crc = Spi.ReadU16Le();
            if(useCrc)
            {
                // TBD
            }
            else
            {
                (void)crc;
            }
            _CsPin::Set();
            Spi.Read();
            return true;
        }

    public:
        /**
         * @brief Check card status
         * 
         * @return true OK
         * @return false Error
         */
        static bool CheckStatus();

        /**
         * @brief Detect sd card type
         * 
         * @return SdCardType Card type
         */
        static SdCardType Detect();

        /**
         * @brief Returns card's blocks count
         * 
         * @return uint32_t Blocks count
         */
        static uint32_t BlocksCount();

        /**
         * @brief Returns card's block size
         * 
         * @return size_t Block size
         */
        static size_t BlockSize();

        /**
         * @brief Writes block to card
         * 
         * @tparam WriteIterator Iterator type
         * 
         * @param iter Iterator
         * @param logicalBlockAddress Block address
         * 
         * @return true Success
         * @return false Fail
         */
        template<typename WriteIterator>
        static bool WriteBlock(WriteIterator iter, uint32_t logicalBlockAddress)
        {
            if(_type != SdhcCard)
                logicalBlockAddress <<= 9;
            if(SpiCommand(SdCardCommand::WriteBlock, logicalBlockAddress) == 0)
            {
                _CsPin::Clear();
                if(Spi.Ignore(10000u, 0xff) != 0xff)
                {
                    return false;
                }

                Spi.Write(0xFE);
                Spi.template Write<WriteIterator>(iter, 512);
                Spi.ReadU16Be();
                uint8_t resp;
                if((resp = Spi.Read() & 0x1F) != 0x05)
                {
                    return false;
                }
                _CsPin::Set();
                Spi.Read();
                return true;
            }
            return false;
        }

        /**
         * @brief Read block from card
         * 
         * @tparam ReadIterator Iterator type
         * 
         * @param iter Iterator
         * @param logicalBlockAddress Block address
         * 
         * @return true Success
         * @return false Fail
         */
        template<typename ReadIterator>
        static bool ReadBlock(ReadIterator iter, uint32_t logicalBlockAddress)
        {
            if(_type != SdhcCard)
                logicalBlockAddress <<= 9;
            if(!WaitWhileBusy())
                return false;
            if(SpiCommand(ReadSingleBlock, logicalBlockAddress) == 0)
            {
                return ReadDataBlock<ReadIterator>(iter, 512);
            }
            return false;
        }

        /**
         * @brief Read multiple blocks from card
         * 
         * @tparam ReadIterator Iterator type
         * 
         * @param iter [in] Iterator
         * @param logicalBlockAddress [in] Block address
         * @param [in] blocksCount Block to read count
         * 
         * @return true Success
         * @return false Fail
         */
        template<typename ReadIterator>
        static bool ReadMultipleBlock(ReadIterator iter, uint32_t logicalBlockAddress, uint32_t blocksCount)
        {
            if(_type != SdhcCard)
                logicalBlockAddress <<= 9;
            if(!WaitWhileBusy())
                return false;
            if(SpiCommand(SdCardCommand::ReadMultipleBlock, logicalBlockAddress) == 0)
            {
                ReadDataBlock<ReadIterator>(iter, 512 * blocksCount);
            }
            return false;
        }
    };

    template<typename _SpiModule, typename _CsPin>
    SdCardType SdCard<_SpiModule, _CsPin>::_type;
    template<typename _SpiModule, typename _CsPin>        
    BinaryStream<_SpiModule> SdCard<_SpiModule, _CsPin>::Spi;
} // namespace Zhele::Drivers

#include "impl/sdcard.h"


#endif //! ZHELE_DRIVERS_SDCARD_H
