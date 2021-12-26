/**
 * @file
 * Driver for bmp 280
 * 
 * bases on AlexGyver library (url: github.com/GyverLibs/GyverBME280) and Adafruit (url: github.com/adafruit/Adafruit_BMP280_Library)
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_BMP280_H
#define ZHELE_DRIVERS_BMP280_H

namespace Zhele::Drivers
{
    /**
     * @brief Class for DMP280 sensor
     * 
     * @tparam _I2CBus Target I2C type (See i2c.h)
     */
    template <typename _I2CBus>
    class Bmp280
    {
        const static uint8_t Bmp280Address = 0x76; ///< I2C address in 7-bit mode
        const static uint8_t ChipId = 0x58; ///< Chip ID

        /// Sensor registers
        enum class Register
        {
            DigT1 = 0x88, ///< Dig T1 register
            DigT2 = 0x8a, ///< Dig T2 register
            DigT3 = 0x8c, ///< Dig T3 register

            DigP1 = 0x8e, ///< Dig P1 register
            DigP2 = 0x90, ///< Dig P2 register
            DigP3 = 0x92, ///< Dig P3 register
            DigP4 = 0x94, ///< Dig P4 register
            DigP5 = 0x96, ///< Dig P5 register
            DigP6 = 0x98, ///< Dig P6 register
            DigP7 = 0x9a, ///< Dig P7 register
            DigP8 = 0x9c, ///< Dig P8 register
            DigP9 = 0x9e, ///< Dig P9 register

            ChipId = 0xd0, ///< Chip id register
            Version = 0xd1, ///< Version register
            SoftReset = 0xe0, ///< Soft reset register

            Calibration = 0xe1, ///< Calibration start register

            Status = 0xf3, ///< Status register
            Control = 0xf4, ///< Control register
            Config = 0xf5, ///< Config register
            PressureData = 0xf7, ///< Pressure data register
            TemperatureData = 0xfa, ///< Temperature data register
        };

        /// Sampling rates
        enum class Sampling : uint8_t
        {
            None = 0x00, ///< Sampling none
            X1 = 0x01, ///< Sampling x1
            X2 = 0x02, ///< Sampling x2
            X4 = 0x03, ///< Sampling x4
            X8 = 0x04, ///< Sampling x8
            X16 = 0x05 ///< Sampling x16
        };

        /// Power modes
        enum class Mode : uint8_t
        {
            Sleep = 0b00, ///< Sleep mode
            Forced = 0b01, ///< Forced mode
            Normal = 0b11 ///< Normal mode
        };

        /// Filter values
        enum class Filter : uint8_t
        {
            Off = 0x00, ///< No filtering
            X2 = 0x01, ///< 2x filtering
            X4 = 0x02, ///< 4x filtering
            X8 = 0x03, ///< 8x filtering
            X16 = 0x04 ///< 16x filtering
        };

        /// Standby duration in ms
        enum StandbyDuration
        {
            Ms1 = 0x00, ///< 1 ms standby
            Ms63 = 0x01, ///< 62.5 ms standby
            Ms125 = 0x02, ///< 125 ms standby
            Ms250 = 0x03, ///< 125 ms standby
            Ms500 = 0x04, ///< 500 ms standby
            Ms1000 = 0x05, ///< 1000 ms standby
            Ms2000 = 0x06, ///< 2000 ms standby
            Ms4000 = 0x0, ///< 4000 ms standby
        };

#pragma pack(push, 1)
        /// Calibration data
        struct CalibrationData
        {
            uint16_t T1; ///< T1 calibration value
            int16_t T2; ///< T2 calibration value
            int16_t T3; ///< T3 calibration value
            uint16_t P1; ///< P1 calibration value
            int16_t P2; ///< P2 calibration value
            int16_t P3; ///< P3 calibration value
            int16_t P4; ///< P4 calibration value
            int16_t P5; ///< P5 calibration value
            int16_t P6; ///< P6 calibration value
            int16_t P7; ///< P7 calibration value
            int16_t P8; ///< P8 calibration value
            int16_t P9; ///< P9 calibration value
        };
#pragma pack(pop)

        /// Control register format
        struct Control
        {
            uint8_t TemparatureOversampling : 3; ///< Temperature oversampling
            uint8_t PressureOversampling : 3; ///< Pressure oversampling
            uint8_t Mode : 2; ///< Device mode
        };

        /// Configuration register format
        struct Config
        {
            uint8_t StandbyDuration : 3; ///< Inactive duration (standby time) in normal mode
            uint8_t Filter : 3; ///< Filter settings

            uint8_t None : 1;     ///< Unused
            uint8_t SpiEnable : 1; ///< Unused
        };

        static CalibrationData _calibrationData;
        static Control _control;
        static Config _config;

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
            
            for(unsigned i = 0; i < 100000; ++i)
                __asm("nop");


            uint8_t chipId = ReadRegister(Register::ChipId);

            if(chipId != ChipId)
                return false;
            
            ReadCalibrationData();

            WriteRegister(Register::Control, *reinterpret_cast<uint8_t*>(&_control));
            WriteRegister(Register::Config, *reinterpret_cast<uint8_t*>(&_config));

            for(unsigned i = 0; i < 100000; ++i)
                __asm("nop");

            return true;
        }

        /**
         * @brief Read temperature
         * 
         * @returns Temperature
         */
        static float ReadTemperature()
        {
            int32_t raw = ReadRegister24(Register::TemperatureData);

            if (raw == 0x800000)
                return 0;

            raw >>= 4;

            int32_t firstTemp = ((((raw >> 3) - (static_cast<int32_t>(_calibrationData.T1) << 1))) *
                    (static_cast<int32_t>(_calibrationData.T2))) >>
                    11;

            int32_t secondTemp = (((((raw >> 4) - (static_cast<int32_t>(_calibrationData.T1))) *
                        ((raw >> 4) - (static_cast<int32_t>(_calibrationData.T1)))) >>
                    12) *
                    (static_cast<int32_t>(_calibrationData.T3))) >>
                    14;

            firstTemp += secondTemp;

            return static_cast<float>((firstTemp * 5) >> 8) / 100;
        }

    private:
        /**
         * @brief Software reset
         * 
         * @return true 
         * @return false 
         */
        static bool Reset()
        {
            return WriteRegister(Register::SoftReset, 0xB6) == I2cStatus::Success;
        }

        /**
         * @brief Read sensor calibration data
         * 
         * @par Returns
         *  Nothing
         */
        static void ReadCalibrationData()
        {
            _I2CBus::Read(Bmp280Address, static_cast<uint16_t>(Register::DigT1), reinterpret_cast<uint8_t*>(&_calibrationData), sizeof(CalibrationData));
        }

        /**
         * @brief Read 3-bytes register.
         * 
         * @param regAddress Register address
         * 
         * @return 3-bytes register value
         */
        static uint32_t ReadRegister24(Register regAddress)
        {  
            uint32_t value = 0;
        
            _I2CBus::Read(Bmp280Address, static_cast<uint16_t>(regAddress), (reinterpret_cast<uint8_t*>(&value)), 3);
            std::swap(reinterpret_cast<uint8_t*>(&value)[0], reinterpret_cast<uint8_t*>(&value)[2]);

            return value;
        }

        /**
         * @brief Read 1-byte register value
         * 
         * @param regAddress Register ADDRESS
         * @return 1-byte register value
         */
        static uint8_t ReadRegister(Register regAddress)
        {
            auto readResult = _I2CBus::ReadU8(Bmp280Address, static_cast<uint8_t>(regAddress));
            return readResult.Status == I2cStatus::Success
                ? readResult.Value
                : 0;
        }

        /**
         * @brief Writes value in register
         * 
         * @tparam ValueType Value type (template param, because type may be number, enums, bitfields)
         * 
         * @param regAddress Register address
         * @param value value
         * 
         * @return I2c bus status
         */
        template<typename ValueType>
        static I2cStatus WriteRegister(Register regAddress, ValueType value)
        {
            return _I2CBus::WriteU8(Bmp280Address, static_cast<uint8_t>(regAddress), static_cast<uint8_t>(value));
        }
    };

    template<typename _I2CBus>
    Bmp280<_I2CBus>::CalibrationData Bmp280<_I2CBus>::_calibrationData = {};
    template<typename _I2CBus>
    Bmp280<_I2CBus>::Control Bmp280<_I2CBus>::_control = {
        .TemparatureOversampling = static_cast<uint8_t>(Bmp280<_I2CBus>::Sampling::X4),
        .PressureOversampling  = static_cast<uint8_t>(Bmp280<_I2CBus>::Sampling::X2),
        .Mode = static_cast<uint8_t>(Bmp280<_I2CBus>::Mode::Normal)
    };
    template<typename _I2CBus>
    Bmp280<_I2CBus>::Config Bmp280<_I2CBus>::_config = {
        .StandbyDuration = static_cast<uint8_t>(Bmp280<_I2CBus>::StandbyDuration::Ms250),
        .Filter = static_cast<uint8_t>(Bmp280<_I2CBus>::Filter::X16)
    };
}
#endif // !ZHELE_DRIVERS_BMP280_H