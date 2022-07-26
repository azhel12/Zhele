/**
 * @file
 * Driver for AHT10 sensor
 * 
 * bases on Adafruit library (url: https://github.com/adafruit/Adafruit_AHTX0/)
 * 
 * @author Aleksei Zhelonkin
 * @date 2022
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_AHT10_H
#define ZHELE_DRIVERS_AHT10_H

#include <delay.h>
#include <limits.h>

namespace Zhele::Drivers
{
    /**
     * @brief Class for AHT10 sensor
     * 
     * @tparam _I2CBus Target I2C type (See i2c.h)
     */
    template <typename _I2CBus>
    class Aht10
    {
        const static uint8_t AHT10Address = 0x38; ///< I2C address in 7-bit mode

        /// Sensor commands
        enum class Commands : uint8_t
        {
            Calibrate = 0xe1, ///< Calibrate
            Trigger = 0xac, ///< Trigger
            SoftReset = 0xba, ///< Soft reset
            StartMeasurement = 0x33, ///< Start measurement
        };

        /// Sensor status
        enum class Status : uint8_t
        {
            Busy = 0x80, ///< Busy
            Calibrated = 0x08, ///< Calibrated
        };

    public:
        /**
         * @brief Init sensor
         * 
         * @return true Init success
         * @return false Init fail
         */
        static bool Init()
        {
            if(!Reset())
                return false;

            if(!Calibrate())
                return false;

            return true;
        }

        /**
         * @brief  Gets the status (first byte) from AHT10/AHT20
         *
         * @returns 8 bits of status data, or 0xFF if failed
         */
        static uint8_t GetStatus()
        {
            auto readResult = ReadU8();
            if (readResult.Status != I2cStatus::Success) {
                return 0xff;
            }
            return readResult.Value;
        }

        /**
         * @brief Read temperature
         * 
         * @returns Temperature
         */
        static float ReadTemperature()
        {
            uint8_t data[6];

            if (!ReadRaw(data))
                return false;

            uint32_t temperature = data[3] & 0x0f;
            temperature <<= 8;
            temperature |= data[4];
            temperature <<= 8;
            temperature |= data[5];
            return (static_cast<float>(temperature) * 200 / 0x100000) - 50;
        }

        /**
         * @brief Read temperature
         * 
         * @returns Temperature
         */
        static float ReadHumidity()
        {
            uint8_t data[6];

            if (!ReadRaw(data))
                return false;

            uint32_t humidity = data[1];
            humidity <<= 8;
            humidity |= data[2];
            humidity <<= 4;
            humidity |= data[3] >> 4;
            return (static_cast<float>(humidity) * 100) / 0x100000;
        }

        /**
         * @brief Read temperature and humidity both
         * 
         * @returns Temperature and humidity pair
         */
        static std::pair<float, float> ReadTemperatureAndHumidity()
        {
            uint8_t data[6];

            if (!ReadRaw(data))
                return std::make_pair(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN());

            uint32_t temperature = data[3] & 0x0f;
            temperature <<= 8;
            temperature |= data[4];
            temperature <<= 8;
            temperature |= data[5];

            uint32_t humidity = data[1];
            humidity <<= 8;
            humidity |= data[2];
            humidity <<= 4;
            humidity |= data[3] >> 4;

            return std::make_pair((static_cast<float>(temperature) * 200 / 0x100000) - 50, (static_cast<float>(humidity) * 100) / 0x100000);
        }

    private:
        /**
         * @brief Read raw 6 bytes
         * 
         * @param [out] data Buffer
         * 
         * @return true Read success
         * @return false Read fail
         */
        static bool ReadRaw(uint8_t* data)
        {
            uint8_t cmd[] = {static_cast<uint8_t>(Commands::Trigger), static_cast<uint8_t>(Commands::StartMeasurement), 0};

            if (!Write(cmd)) {
                return false;
            }

            while (GetStatus() & static_cast<uint8_t>(Status::Busy)) {
                delay_ms<10>();
            }

            return Read(data);
        }

        /**
         * @brief Software reset
         * 
         * @return true 
         * @return false 
         */
        static bool Reset()
        {
            return WriteU8(static_cast<uint8_t>(Commands::SoftReset));
        }

        /**
         * @brief Read sensor calibration data
         * 
         * @par Returns
         *  Nothing
         */
        static bool Calibrate()
        {
            uint8_t data[] = {static_cast<uint8_t>(Commands::SoftReset), 0x08, 0x00};

            if(!Write(data))
                return false;

            while(GetStatus() & static_cast<uint8_t>(Status::Busy)) {
                delay_ms<10>();
            }

            return GetStatus() & static_cast<uint8_t>(Status::Calibrated);
        }

        /**
         * @brief Read
         * 
         * @param regAddress Register ADDRESS
         * @return ReadResult
         */
        template<uint8_t Size = 6>
        static bool Read(uint8_t* data)
        {
            return _I2CBus::Read(AHT10Address, 0, data, Size, I2cOpts::RegAddrNone) == I2cStatus::Success;
        }

        /**
         * @brief Read 1-byte register value
         * 
         * @param regAddress Register ADDRESS
         * @return ReadResult
         */
        static ReadResult ReadU8()
        {
            return _I2CBus::ReadU8(AHT10Address, 0, I2cOpts::RegAddrNone);
        }

        /**
         * @brief Writes byte
         * 
         * @param value value
         * 
         * @return I2c bus status
         */
        static bool WriteU8(uint8_t value)
        {
            return _I2CBus::WriteU8(AHT10Address, 0, value, I2cOpts::RegAddrNone) == I2cStatus::Success;
        }

        /**
         * @brief Writes data
         * 
         * @tparam Size data size
         * 
         * @param data data
         * 
         * @return I2c bus status
         */
        template<uint8_t Size>
        static bool Write(uint8_t (&data)[Size])
        {
            return _I2CBus::Write(AHT10Address, 0, data, Size, I2cOpts::RegAddrNone) == I2cStatus::Success;
        }
    };
}
#endif // !ZHELE_DRIVERS_AHT10_H