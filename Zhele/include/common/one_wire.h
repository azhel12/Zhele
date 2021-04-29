/**
 * @file

 * @brief This file contains template class for communicate with
 * one-wire-based devices.
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @licence FreeBSD
 */

#ifndef ZHELE_ONE_WIRE_COMMON_H
#define ZHELE_ONE_WIRE_COMMON_H

#include <stdint.h>

namespace Zhele
{
    /**
     * @brief Class for one-wired communicate
     * 
     * @tparam Pin GPIO Pin
     */
    template<typename _Usart, typename _Pin>
    class OneWire
    {
    public:
        enum Commands : uint8_t
        {
            Read = 0x33,
            Match = 0x55,
            Skip = 0xcc,
            Search = 0xf0,
        };
        
        /**
         * @brief Initialize one-wire communicate line
         * 
         * @par Returns
         *	Nothing
         */
        static void Init();
        

        /**
         * @brief Reset line
         * 
         * @retval true Success (some device connected)
         * @retval false Error (no device connected)
         * 
         * @retval true Success reset (precense detected)
         * @retval false Fail reset (precense not detected)
         */
        static bool Reset();
    

        /**
         * @brief Writes byte to line
         * 
         * @param [in] byteToWrite Byte for write
         * 
         * @par Returns
         *	Nothing
         */
        static void WriteByte(uint8_t byteToWrite);
        

        /**
         * @brief Reads byte from line
         * 
         * @return Byte from line
         */
        static uint8_t ReadByte();
        

        /**
         * @brief Reads bytes from line
         * 
         * @param [out] data Output buffer
         * @param [in] size Size to read
         * 
         * @return
         *  Nothing
         */
        static void ReadBytes(void* data, uint8_t size);
       

        /**
         * @brief Select (match) device by ROM.
         * 
         * @param [in] rom 8-bytes array with device ROM
         * 
         * @par Returns
         *	Nothing
         */
        static void MatchRom(const uint8_t rom[8]);
       

        /**
         * @brief Send SKIP command to line;
         * 
         * @details
         * Method send "SKIP ROM" command to line.
         * If it is only one device on the line, protocol
         * allows to skip specifying device`s address.
         * 
         * @par Returns
         *	Nothing
         */
        static void SkipRom();
       

        /**
         * @brief Read single devices`s address
         * 
         * @detailss
         * This method write "Read ROM" command to line
         * and read 8 bytes
         * 
         * @param [out] rom Address of found device.
         * 
         * @retval true Precense bit was detected
         * @retval false No precense bit was detected
         */
        static bool ReadRom(uint8_t* rom);
       

        /**
         * @brief Start new search.
         * 
         * @param [out] rom Address of found device.
         * 
         * @todo Implement this method
         * 
         * @returns
         *	Nothing
         */
        static void SearchFirst(uint8_t* rom)
        {
        }

    private:
        /**
         * @brief Convert received array to one byte
         * 
         * @param [in] bits Receved array
         * 
         * @returns Matching byte
         */
        static uint8_t ConvertToByte(const uint8_t* bits);
       

        /**
         * @brief Convert byte to array for transmit
         * 
         * @param [in] byte Byte to decompose
         * @param [out] bits Out array
         * 
         * @returns
         *  Nothing
         */
        static void ConvertToBits(uint8_t byte, uint8_t* bits);
       
    private:
        // Dummy buffer for read operation
        static constexpr uint8_t _readDummyBuffer[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    };
}

#include "impl/one_wire.h"

#endif // !ZHELE_ONE_WIRE_COMMON_H