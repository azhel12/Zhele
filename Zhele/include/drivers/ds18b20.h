/**
 * @file
 * Contains class for Dallas DS18B20 temperature sensor
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_DS18B20_H
#define ZHELE_DS18B20_H

#include <one_wire.h>

namespace Zhele::Drivers
{
    /**
     * @brief Implements driver for DS18B20 sensor over One-wire protocol
     * 
     * @tparam Pin GPIO pin for One-wire
     */
    template <typename _Bus>
    class Ds18b20
    {
        /**
         * @brief Sensor commands
         */
        enum Commands : uint8_t
        {
            ConvertTemperature = 0x44, ///< Start temperature convert (measure)
            ReadScratchPad = 0xbe, ///< Read sensor memory
            WriteScratchPad = 0x4e, ///< Write sensor memory (resolution, etc)
        };

        /**
         * @brief Ds18b20 scratchpad structure.
         */
        struct Scratchpad
        {
            uint8_t Lsb;
            uint8_t Msb;
            uint8_t Th;
            uint8_t Tl;
            uint8_t Configuration;
            uint8_t Reserved[3];
            uint8_t Crc;
        };

        static constexpr float Resolution = 0.0625;
    public:
        /**
         * @brief Convert result
         */
        struct ConvertResult
        {
            enum ConvertErrors : uint8_t
            {
                PrecenseError,
                CrcError
            };
            bool Success;
            union
            {
                float Temperature;
                ConvertErrors Error;
            };
        };

        /**
         * @brief Initialize communicate line and sensor
         * 
         * @retval true If some device send response for reset
         * @retval true If no devices on line
         */
        static bool Init()
        {
            _Bus::Init();
            return _Bus::Reset();
        }

        /**
         * @brief Send command to start temperature convert for device with given ROM or for all devices (via SKIP ROM)
         * 
         * @param [in, opt] rom ROM
         * 
         * @par Returns
         *	Nothing
         */
        static bool Start(const uint8_t* rom = nullptr)
        {
            if(!_Bus::Reset())
                return false;

            rom == nullptr
                ? _Bus::SkipRom()
                : _Bus::MatchRom(rom);

            _Bus::WriteByte(Commands::ConvertTemperature);

            return true;
        }

        /**
         * @brief Read convert (measure) result
         * 
         * @param [in, opt] rom Sensor ROM
         * 
         * @returns Conversion result
         */
        static ConvertResult Read(const uint8_t* rom = nullptr)
        {
            Scratchpad scratchpad;

            if(!_Bus::Reset())
                return ConvertResult{false, ConvertResult::ConvertErrors::PrecenseError};

            rom == nullptr
                ? _Bus::SkipRom()
                : _Bus::MatchRom(rom);
            
            _Bus::WriteByte(Commands::ReadScratchPad);
            _Bus::ReadBytes(&scratchpad, sizeof(scratchpad));

            if(CalculateCrc(&scratchpad, 8) != scratchpad.Crc)
                return ConvertResult{false, ConvertResult::ConvertErrors::CrcError};

            uint8_t resolution = (scratchpad.Configuration >> 5) & 0b11;
            scratchpad.Lsb &= 0xff << (3 - resolution);
            float temperature = static_cast<float>(*(reinterpret_cast<int16_t*>(&scratchpad.Lsb))) * Resolution;

            return ConvertResult{true, temperature};
        }

        /**
         * @brief Check that all devices complete temperature convert (release line)
         * 
         * @retval true All sensors complete convert (line has been released)
         * @retval false Not all sensor complete convert (line not released yet)
         */
        static bool AllDone()
        {
            _Bus::WriteByte(_Bus::Commands::Read);
            return _Bus::ReadByte() == 0xff;
        }

    private:
        static uint8_t CalculateCrc(const void* data, uint8_t size)
        {
            const uint8_t* castedData = reinterpret_cast<const uint8_t*>(data);
            uint8_t crc = 0;
            for(uint8_t i = 0; i < size; ++i)
            {
                uint8_t byte = castedData[i];
                for (uint8_t j = 0; j < 8; ++j)
                {
                    uint8_t tmp = crc ^ byte;
                    crc >>= 1;
                    byte >>= 1;
                    if ((tmp & 1) > 0)
                        crc ^= 0x8c;
                }
            }
            return crc;
        }
    };
}

#endif //! ZHELE_DS18B20_H