/**
 * @file
 * Implements classes for ST77xx based TFT display
 * @author Alexey Zhelonkin (based on afiskon port)
 * @author bakatrouble (ST7789 support, based on https://github.com/deividAlfa/ST7789-STM32-uGUI)
 * @date 2022
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_ST7735_H
#define ZHELE_DRIVERS_ST7735_H

#include <delay.h>
#include <common/template_utils/data_transfer.h>

#include <cstdint>
#include <initializer_list>

namespace Zhele::Drivers
{
    namespace Private {
        enum MadCtl : uint8_t
        {
            My = 0x80,
            Mx = 0x40,
            Mv = 0x20,
            Ml = 0x10,
            Rgb = 0x00,
            Bgr = 0x08,
            Mh = 0x04
        };

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
        template <typename _SpiBus, typename _SsPin, typename _DcPin, typename _ResetPin, uint8_t _Rotation, uint8_t _TextRotation, uint8_t _Width = 128, uint8_t _Height = 160>
        class St77xx
        {
        protected:
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
                PorCtrl = 0xb2,
                FrmCtr2 = 0xb2,
                FrmCtr3 = 0xb3,
                InvCtr = 0xb4,
                DisSet5 = 0xb6,
                GCtrl = 0xb7,
                VComs = 0xbb,

                LcmCtrl = 0xc0,
                PwCtr1 = 0xc0,
                PwCtr2 = 0xc1,
                VdvVreh = 0xc2,
                PwCtr3 = 0xc2,
                Vrhs = 0xc3,
                PwCtr4 = 0xc3,
                Vdvs = 0xc4,
                PwCtr5 = 0xc4,
                FrCtrl2 = 0xc6,
                VmCtrl2 = 0xc6,
                VmCtr1 = 0xc5,

                PwCtrl1 = 0xd0,
                RdId1 = 0xda,
                RdId2 = 0xdb,
                RdId3 = 0xdc,
                RdId4 = 0xdd,

                PwCtr6 = 0xfc,

                GmctrP1 = 0xe0,
                GmctrN1 = 0xe1,
            };

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
            static void Init();

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
             * @brief Write char to display
             *
             * @tparam Font Font
             *
             * @param x X coordinate
             * @param y Y coordinate
             * @param symbol Symbol
             * @param color Symbol color
             * @param background Background color
             *
             * @par Returns
             *  Nothing
             */
            template<typename Font>
            static void WriteChar(uint8_t x, uint8_t y, char symbol, uint16_t color, uint16_t background)
            {
                _SsPin::Clear();

                // For compatibility with old fonts I need to fill display by column not by row
                // So, temporarily change the X and Y axes
                WriteCommand(Command::MadCtl);
                WriteData(_TextRotation);

                uint8_t width;
                if constexpr (Font::MonoSpace)
                    width = Font::Width;
                else
                    width = Font::GetWidth(symbol);

                SetAddressWindow(y, x, y + Font::Height - 1, x + width - 1);

                _SpiBus::SetDataSize(_SpiBus::DataSize::DataSize16);
                _DcPin::Set();

                const uint8_t extraBits = Font::Height % 8;

                for (uint8_t column = 0; column < width; ++column) {

                    uint8_t page = 0;
                    for(page = 0; page < Font::Height / 8; ++page) {
                        uint8_t temp = Font::Get(symbol)[page * width + column];

                        for(uint8_t i = 0; i < 8; ++i) {
                            _SpiBus::Write((temp & 0x01) > 0 ? color : background);
                            temp >>= 1;
                        }
                    }

                    if constexpr (extraBits > 0) {
                        uint8_t temp = Font::Get(symbol)[page * width + column] >> (8 - extraBits);

                        for(uint8_t i = 0; i < extraBits; ++i) {
                            _SpiBus::Write((temp & 0x01) > 0 ? color : background);
                            temp >>= 1;
                        }
                    }
                }

                _SpiBus::SetDataSize(_SpiBus::DataSize::DataSize8);

                // Put back
                WriteCommand(Command::MadCtl);
                WriteData(_Rotation);

                _SsPin::Set();
            }

            /**
             * @brief Write string
             *
             * @tparam Font Font
             *
             * @param x X coordinate
             * @param y Y coordinate
             * @param str String
             * @param color Symbol color
             * @param background Background color
             *
             * @par Returns
             *  Nothing
             */
            template<typename Font>
            static void WriteString(uint8_t x, uint8_t y, const char* str, uint16_t color, uint16_t background)
            {
                while(*str) {
                    uint8_t width;
                    if constexpr (Font::MonoSpace)
                        width = Font::Width;
                    else
                        width = Font::GetWidth(*str);

                    if(x + width >= _Width) {
                        x = 0;
                        y += Font::Height;

                        if(y + Font::Height >= _Height)
                            break;

                        if(*str == ' ') {
                            // skip spaces in the beginning of the new line
                            ++str;
                            continue;
                        }
                    }
                    WriteChar<Font>(x, y, *str, color, background);

                    x += width;

                    ++str;
                }
            }

            /**
             * @brief Reset controller
             *
             * @par Returns
             * Nothing
             */
            static void Reset()
            {
                _SsPin::Set();

                _ResetPin::Clear();
                delay_ms<1>();

                _ResetPin::Set();
                delay_ms<200>();

                _SsPin::Clear();
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

        protected:
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
             * @brief Write data to display (1 or 2 bytes)
             *
             * @param data Data
             *
             * @par Returns
             *  Nothing
             */
            static void WriteData(uint16_t data)
            {
                _DcPin::Set();
                _SpiBus::Write(data);
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

        template <typename _SpiBus, typename _SsPin, typename _DcPin, typename _ResetPin, uint8_t _Rotation, uint8_t _TextRotation, uint8_t _Width, uint8_t _Height>
        bool St77xx<_SpiBus, _SsPin, _DcPin, _ResetPin, _Rotation, _TextRotation, _Width, _Height>::_busy = false;
    }

    template <typename _SpiBus, typename _SsPin, typename _DcPin, typename _ResetPin, uint8_t _Width = 128, uint8_t _Height = 160>
    class St7735 : public Private::St77xx<
            _SpiBus, _SsPin, _DcPin, _ResetPin,
            Private::MadCtl::Rgb | (_Width < _Height ? Private::MadCtl::Mx | Private::MadCtl::My : Private::MadCtl::My | Private::MadCtl::Mv),
            (Private::MadCtl::Rgb | (_Width < _Height ? Private::MadCtl::Mx | Private::MadCtl::My : Private::MadCtl::My | Private::MadCtl::Mv) ^ Private::MadCtl::Mv),
            _Width, _Height> {
    private:
        static const uint8_t Rotation = St7735::MadCtl::Rgb
                                        | (_Width < _Height
                                           ? St7735::MadCtl::Mx | St7735::MadCtl::My
                                           : St7735::MadCtl::My | St7735::MadCtl::Mv);
    public:
        static void Init() {
            St7735::Reset();

            St7735::WriteCommand(St7735::Command::SoftwareReset);
            delay_ms<150>();

            St7735::WriteCommand(St7735::Command::SlpOut);
            delay_ms<500>();

            St7735::WriteCommand(St7735::Command::FrmCtr1);
            St7735::WriteData({0x01, 0x2c, 0x2d});

            St7735::WriteCommand(St7735::Command::FrmCtr2);
            St7735::WriteData({0x01, 0x2c, 0x2d});

            St7735::WriteCommand(St7735::Command::FrmCtr3);
            St7735::WriteData({0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d});

            St7735::WriteCommand(St7735::Command::InvCtr);
            St7735::WriteData({0x07});

            St7735::WriteCommand(St7735::Command::PwCtr1);
            St7735::WriteData({0xa2, 0x02, 0x84});

            St7735::WriteCommand(St7735::Command::PwCtr2);
            St7735::WriteData({0xc5});

            St7735::WriteCommand(St7735::Command::PwCtr3);
            St7735::WriteData({0x0a, 0x00});

            St7735::WriteCommand(St7735::Command::PwCtr4);
            St7735::WriteData({0x8a, 0x2a});

            St7735::WriteCommand(St7735::Command::PwCtr5);
            St7735::WriteData({0x8a, 0xee});

            St7735::WriteCommand(St7735::Command::VmCtr1);
            St7735::WriteData({0x0e});

            St7735::WriteCommand(St7735::Command::InvOff);
            St7735::WriteData({0x0e});

            St7735::WriteCommand(St7735::Command::MadCtl);
            St7735::WriteData({St7735::Rotation});

            St7735::WriteCommand(St7735::Command::ColMod);
            St7735::WriteData({0x05});

            if constexpr (_Width == 80 || _Height == 80)
            {
                St7735::WriteCommand(St7735::Command::CaSet);
                St7735::WriteData({0x00, 0x00, 0x00, 0x4f});

                St7735::WriteCommand(St7735::Command::RaSet);
                St7735::WriteData({0x00, 0x00, 0x00, 0x9f});

                St7735::WriteCommand(St7735::Command::InvOn);
            }
            else
            {
                St7735::WriteCommand(St7735::Command::CaSet);
                St7735::WriteData({0x00, 0x00, 0x00, 0x7f});

                St7735::WriteCommand(St7735::Command::RaSet);
                St7735::WriteData({0x00, 0x00, 0x00, 0x7f});
            }

            St7735::WriteCommand(St7735::Command::GmctrP1);
            St7735::WriteData({
                                      0x02, 0x1c, 0x07, 0x12,
                                      0x37, 0x32, 0x29, 0x2d,
                                      0x29, 0x25, 0x2B, 0x39,
                                      0x00, 0x01, 0x03, 0x10});

            St7735::WriteCommand(St7735::Command::GmctrN1);
            St7735::WriteData({
                                      0x03, 0x1d, 0x07, 0x06,
                                      0x2E, 0x2C, 0x29, 0x2D,
                                      0x2E, 0x2E, 0x37, 0x3F,
                                      0x00, 0x00, 0x02, 0x10});

            St7735::WriteCommand(St7735::Command::NorOn);
            delay_ms<10>();

            St7735::WriteCommand(St7735::Command::DispOn);
            delay_ms<100>();

            _SsPin::Set();
        }
    };

    template <typename _SpiBus, typename _SsPin, typename _DcPin, typename _ResetPin, uint8_t _Width = 240, uint8_t _Height = 240>
    class St7789 : public Private::St77xx<
            _SpiBus, _SsPin, _DcPin, _ResetPin,
            Private::MadCtl::Rgb,
            Private::MadCtl::Rgb ^ Private::MadCtl::Mv,
            _Width, _Height> {
    public:
        static void Init() {
            St7789::Reset();

            St7789::WriteCommand(St7789::Command::SlpOut);

            St7789::WriteCommand(St7789::Command::ColMod);
            St7789::WriteData({0x55});

            St7789::WriteCommand(St7789::Command::PorCtrl);
            St7789::WriteData({0x0C, 0x0C, 0x00, 0x33, 0x33});

            St7789::WriteCommand(St7789::Command::GCtrl);
            St7789::WriteData({0x35});

            St7789::WriteCommand(St7789::Command::VComs);
            St7789::WriteData({0x19});

            St7789::WriteCommand(St7789::Command::LcmCtrl);
            St7789::WriteData({0X2C});

            St7789::WriteCommand(St7789::Command::VdvVreh);
            St7789::WriteData({0x01});

            St7789::WriteCommand(St7789::Command::Vrhs);
            St7789::WriteData({0x12});

            St7789::WriteCommand(St7789::Command::Vdvs);
            St7789::WriteData({0x20});

            St7789::WriteCommand(St7789::Command::FrCtrl2);
            St7789::WriteData({0x0F});

            St7789::WriteCommand(St7789::Command::PwCtrl1);
            St7789::WriteData({0xA4, 0xA1});

            St7789::WriteCommand(St7789::Command::MadCtl);
            St7789::WriteData({ 0x00 });

            St7789::WriteCommand(St7789::Command::GmctrP1);
            St7789::WriteData({0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23});

            St7789::WriteCommand(St7789::Command::GmctrN1);

            St7789::WriteData({0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23});

            St7789::WriteCommand(St7789::Command::InvOn);

            St7789::WriteCommand(St7789::Command::NorOn);

            St7789::WriteCommand(St7789::Command::DispOn);
            delay_ms<100>();

            _SsPin::Set();
        }
    };
}

#endif //! ZHELE_DRIVERS_ST7735_H
