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
        static void Init()
        {
            _Usart::Init(9600, _Usart::UsartMode::DataBits8
                            | _Usart::UsartMode::NoneParity
                            | _Usart::UsartMode::RxTxEnable
                            | _Usart::UsartMode::OneStopBit
                            | _Usart::UsartMode::OneSampleBitEnable
                            | _Usart::UsartMode::HalfDuplex);
            
            _Usart::template SelectTxRxPins<_Pin>();
            _Pin::SetDriverType(_Pin::DriverType::OpenDrain);
            _Pin::SetPullMode(_Pin::PullMode::PullUp);
            _Pin::SetSpeed(_Pin::Speed::Fast);
        }

        /**
         * @brief Reset line
         * 
         * @retval true Success (some device connected)
         * @retval false Error (no device connected)
         * 
         * @retval true Success reset (precense detected)
         * @retval false Fail reset (precense not detected)
         */
        static bool Reset()
        {
            _Usart::SetBaud(9600);

            volatile bool complete = false;
            uint8_t precenseBit = 0;

            _Usart::EnableAsyncRead(&precenseBit, 1, [&complete](void*, unsigned, bool){complete = true;});
            _Usart::Write(0xf0);
            while (!complete);

            _Usart::SetBaud(115200);

            return precenseBit != 0xf0;
        }

        /**
         * @brief Writes byte to line
         * 
         * @param [in] byteToWrite Byte for write
         * 
         * @par Returns
         *	Nothing
         */
        static void WriteByte(uint8_t byteToWrite)
        {
            uint8_t buffer[8];
            uint8_t dummyBuffer[8];
            ConvertToBits(byteToWrite, buffer);
            volatile bool complete = false;

            // Send byte async, receive because it's half-duplex
            _Usart::EnableAsyncRead(dummyBuffer, 8, [&complete](void*, unsigned, bool){complete = true;});
            _Usart::Write(buffer, 8, true);

            while(!complete);
        }

        /**
         * @brief Reads byte from line
         * 
         * @return Byte from line
         */
        static uint8_t ReadByte()
        {
            uint8_t buffer[8];
            volatile bool readComplete = false;            

            _Usart::EnableAsyncRead(buffer, 8,
                            [&readComplete](void* data, unsigned size, bool success){
                                readComplete = true;});
            
            _Usart::Write(_readDummyBuffer, 8, true);

            while (!readComplete){}

            return ConvertToByte(buffer);
        }

        /**
         * @brief Reads bytes from line
         * 
         * @param [out] data Output buffer
         * @param [in] size Size to read
         * 
         * @return
         *  Nothing
         */
        static void ReadBytes(void* data, uint8_t size)
        {
            uint8_t* buffer = reinterpret_cast<uint8_t*>(data);

            for(uint8_t i = 0; i < size; ++i)
            {
                buffer[i] = ReadByte();
            }
        }

        /**
         * @brief Select (match) device by ROM.
         * 
         * @param [in] rom 8-bytes array with device ROM
         * 
         * @par Returns
         *	Nothing
         */
        static void MatchRom(const uint8_t rom[8])
        {
            WriteByte(Commands::Match);
	
            for (uint8_t i = 0; i < 8; ++i) {
                WriteByte(rom[i]);
            }
        }

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
        static void SkipRom()
        {
            Reset();
            WriteByte(Commands::Skip);
        }

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
        static bool ReadRom(uint8_t* rom)
        {
            if(!Reset())
                return false;
            WriteByte(Commands::Read);

            for (uint8_t i = 0; i < 8; ++i)
            {
                rom[i] = ReadByte();
            }
            return true;
        }

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
        static uint8_t ConvertToByte(const uint8_t* bits)
        {
            uint8_t resultByte;
            resultByte = 0;
            for (uint8_t i = 0; i < 8; i++)
            {
                resultByte >>= 1;
                if (bits[i] == 0xff) {
                    resultByte |= 0x80;
                }
            }
            return resultByte;
        }

        /**
         * @brief Convert byte to array for transmit
         * 
         * @param [in] byte Byte to decompose
         * @param [out] bits Out array
         * 
         * @returns
         *  Nothing
         */
        static void ConvertToBits(uint8_t byte, uint8_t* bits)
        {
            for (uint8_t i = 0; i < 8; ++i)
            {
                bits[i] = (byte & 0x01) > 0
                            ? 0xff
                            : 0x00;

                byte >>= 1;
            }
        }
    private:
        // Dummy buffer for read operation
        static constexpr uint8_t _readDummyBuffer[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    };
}

#endif // !ZHELE_ONE_WIRE_COMMON_H