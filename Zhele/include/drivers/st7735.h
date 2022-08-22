/**
 * @file
 * Implements class for ST7735 based TFT display
 * @author Alexey Zhelonkin (based on afiskon port)
 * @date 2022
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_ST7735_H
#define ZHELE_DRIVERS_ST7735_H

#include <delay.h>

#include <cstdint>
#include <initializer_list>

namespace Zhele::Drivers
{
    /**
     * @brief Implements driver for TFT based on ST7735
     * 
     * @tparam _SpiBus Spi bus
     * @tparam _SsPin Chip select pin
     * @tparam _DcPin Data/Control pin
     * @tparam _ResetPin Reset pin
     * @tparam _Width Width
     * @tparam _Height Height
     */
    template <typename _SpiBus, typename _SsPin, typename _DcPin, typename _ResetPin, uint8_t _Width = 128, uint8_t _Height = 160>
    class St7735
    {
        enum MadCtl : uint8_t
        {
            My = 0x80,
            Mx = 0x40,
            Mv = 0x20, 
            Ml = 0x10,
            Rfb = 0x00,
            Bgr = 0x08,
            Mh = 0x04
        };

        /// Display commands
        enum class Command : uint8_t
        {
            Nop = 0x00, ///< No operation
            SoftwareReset = 0x01, ///< Software reset
            RddId = 0x04,
            RddSt = 0x09,

            SlpIn = 0x10,
            SlpOut = 0x11,
            PtlOn = 0x12, 
            NorOn = 0x13,

            InvOff = 0x20,
            InvOn = 0x21,
            GamSet = 0x26,
            DispOff = 0x28,
            DispOn = 0x29,
            CaSet = 0x2a,
            RaSet = 0x2b,
            RamWr = 0x2c,
            RamRd = 0x2e,

            PrlAr = 0x30,
            ColMod = 0x3a,
            MadCtl = 0x36,

            FrmCtr1 = 0xb1,
            FrmCtr2 = 0xb2,
            FrmCtr3 = 0xb3,
            InvCtr = 0xb4,
            DisSet5 = 0xb6,

            PwCtr1 = 0xc0,
            PwCtr2 = 0xc1,
            PwCtr3 = 0xc2,
            PwCtr4 = 0xc3,
            PwCtr5 = 0xc4,
            VmCtr1 = 0xc5,

            RdId1 = 0xda,
            RdId2 = 0xdb,
            RdId3 = 0xdc,
            RdId4 = 0xdd,

            PwCtr6 = 0xfc,

            GmctrP1 = 0xe0,
            GmctrN1 = 0xe1
        };

        static const uint8_t Rotation = _Width < _Height
            ? MadCtl::Mx | MadCtl::My
            : MadCtl::My | MadCtl::Mv;

        static bool _busy;
    public:
        /// Color
        enum Color : uint16_t
        {
            Black = 0x0000,
            Blue = 0x001f,
            Red = 0xf800,
            Green = 0x07e0,
            Cyan = 0x07ff,
            Magenta = 0xf81f,
            Yellow = 0xffe0,
            White = 0xffff
        };

        /**
         * @brief Init display
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            _SsPin::Clear();
            
            Reset();

            WriteCommand(Command::SoftwareReset);
            delay_ms<150>();

            WriteCommand(Command::SlpOut);
            delay_ms<500>();

            WriteCommand(Command::FrmCtr1);
            WriteData({0x01, 0x2c, 0x2d});

            WriteCommand(Command::FrmCtr2);
            WriteData({0x01, 0x2c, 0x2d});

            WriteCommand(Command::FrmCtr3);
            WriteData({0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d});

            WriteCommand(Command::InvCtr);
            WriteData({0x07});

            WriteCommand(Command::PwCtr1);
            WriteData({0xa2, 0x02, 0x84});

            WriteCommand(Command::PwCtr2);
            WriteData({0xc5});

            WriteCommand(Command::PwCtr3);
            WriteData({0x0a, 0x00});

            WriteCommand(Command::PwCtr4);
            WriteData({0x8a, 0x2a});

            WriteCommand(Command::PwCtr5);
            WriteData({0x8a, 0xee});

            WriteCommand(Command::VmCtr1);
            WriteData({0x0e});

            WriteCommand(Command::InvOff);
            WriteData({0x0e});

            WriteCommand(Command::MadCtl);
            WriteData({Rotation});

            WriteCommand(Command::ColMod);
            WriteData({0x05});

            if constexpr (_Width == 80 || _Height == 80)
            {
                WriteCommand(Command::CaSet);
                WriteData({0x00, 0x00, 0x00, 0x4f});

                WriteCommand(Command::RaSet);
                WriteData({0x00, 0x00, 0x00, 0x9f});

                WriteCommand(Command::InvOn);
            }
            else
            {
                WriteCommand(Command::CaSet);
                WriteData({0x00, 0x00, 0x00, 0x7f});

                WriteCommand(Command::RaSet);
                WriteData({0x00, 0x00, 0x00, 0x7f});
            }

            WriteCommand(Command::GmctrP1);
            WriteData({
                0x02, 0x1c, 0x07, 0x12,
                0x37, 0x32, 0x29, 0x2d,
                0x29, 0x25, 0x2B, 0x39,
                0x00, 0x01, 0x03, 0x10});

            WriteCommand(Command::GmctrN1);
            WriteData({
                0x03, 0x1d, 0x07, 0x06,
                0x2E, 0x2C, 0x29, 0x2D,
                0x2E, 0x2E, 0x37, 0x3F,
                0x00, 0x00, 0x02, 0x10});

            WriteCommand(Command::NorOn);
            delay_ms<10>();

            WriteCommand(Command::DispOn);
            delay_ms<100>();

            _SsPin::Set();
            _SsPin::WaitForSet();
        }

        /**
         * @brief Set the Address Window
         * 
         * @param x0 x0
         * @param y0 y0
         * @param x1 x1
         * @param y1 y1
         */
        static void SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
        {
            WriteCommand(Command::CaSet);
            WriteData({ 0x00, x0, 0x00, x1});

            WriteCommand(Command::RaSet);
            WriteData({0x00, y0, 0x00, y1});

            WriteCommand(Command::RamWr);
        }

        /**
         * @brief Draw pixel
         * 
         * @param x X coordinate
         * @param y Y coordinate
         * @param color Pixel color
         * 
         * @par Returns
         *  Nothing
         */
        static void DrawPixel(uint8_t x, uint8_t y, uint16_t color)
        {
            _SsPin::Clear();

            SetAddressWindow(x, y, x + 1, y + 1);
            WriteData({color >> 8, color & 0xff});
            
            _SsPin::Set();
        }

        /**
         * @brief Fill rectangle
         * 
         * @param x X coordinate
         * @param y Y coordinate
         * @param width Width
         * @param height Height
         * @param color Color
         * 
         * @par Returns
         * Nothing
         */
        static void FillRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)
        {
            _busy = true;

            _SsPin::Clear();

            SetAddressWindow(x, y, x + width - 1, y + height - 1);

            _SpiBus::SetDataSize(_SpiBus::DataSize::DataSize16);

            _DcPin::Set();
            
            _SpiBus::WriteAsyncNoIncrement(&color, height * width, [](void* data, unsigned size, bool success){
                while(_SpiBus::Busy()) continue;
                _SsPin::Set();
                _SpiBus::SetDataSize(_SpiBus::DataSize::DataSize8);
                _busy = false;
            });
        }

        /**
         * @brief Foll screen with given color
         * 
         * @param color Color
         * 
         * @par Returns
         *  Nothing
         */
        static void FillScreen(uint16_t color)
        {
            FillRectangle(0, 0, _Width, _Height, color);
        }

        /**
         * @brief Draw image
         * 
         * @param x X coordinate
         * @param y Y coordinate
         * @param width Image width
         * @param height Image height
         * @param data Image data source
         * 
         * @par Returns
         *  Nothing
         */
        static void DrawImage(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint16_t* data)
        {
            _busy = true;

            _SsPin::Clear();
            
            SetAddressWindow(x, y, x + width - 1, y + height - 1);

            WriteDataAsync(data, sizeof(uint16_t) * width * height);
        }

        /**
         * @brief Reset controller
         * 
         * @par Returns
         * Nothing
         */
        static void Reset()
        {
            _ResetPin::Clear();
            
            delay_ms<5>();

            _ResetPin::Set();
        }

        /**
         * @brief Indicates driver busy state
         * 
         * @return true Display busy
         * @return false Display not busy
         */
        static bool Busy()
        {
            return _busy;
        }

    private:
        /**
         * @brief Write command to display
         * 
         * @param command Command
         * 
         * @par Returns
         *  Nothing
         */
        static void WriteCommand(Command command)
        {
            _DcPin::Clear();

            _SpiBus::Write(static_cast<uint8_t>(command));
        }
        
        /**
         * @brief Write data to display
         * 
         * @param data Data
         * 
         * @par Returns
         *  Nothing
         */
        static void WriteData(std::initializer_list<uint8_t> data)
        {
            _DcPin::Set();

            for (uint8_t dataByte : data)
                _SpiBus::Write(dataByte);
        }

        /**
         * @brief Write data to display async (via DMA)
         * 
         * @param data Data
         * @param size Size
         * @param callback Complete callback
         */
        static void WriteDataAsync(const void* data, uint32_t size, TransferCallback callback = [](void* data, unsigned size, bool success){_SsPin::Set(); _busy = false;})
        {
            _DcPin::Set();

            _SpiBus::WriteAsync(data, size, callback);
        }
    };

    template <typename _SpiBus, typename _SsPin, typename _DcPin, typename _ResetPin, uint8_t _Width, uint8_t _Height>
    bool St7735<_SpiBus, _SsPin, _DcPin, _ResetPin, _Width, _Height>::_busy = false;
}

#endif //! ZHELE_DRIVERS_ST7735_H