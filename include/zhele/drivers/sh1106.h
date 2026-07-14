/**
 * @file
 * Implements class for Sh1106 OLED display
 * @author Alexey Zhelonkin (based on Tilen Majerle library)
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_DRIVERS_SH1106_H
#define ZHELE_DRIVERS_SH1106_H

#include <zhele/i2c.h>

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace Zhele::Drivers
{
    /**
     * @brief Implements driver for SH1106-based OLED display
     *
     * Unlike SSD1306, SH1106 has no horizontal addressing mode, so a frame is
     * sent as one I2C transaction per page: the page addressing commands ride
     * in front of the pixel data using Co-bit framing, and the next page is
     * chained from the DMA completion callback.
     *
     * @tparam I2CBus I2C bus
     * @tparam Width Display width
     * @tparam Height Display height
     * @tparam ColumnOffset Horizontal RAM offset of the visible area: SH1106
     *  RAM is 132 px wide and typical 1.3" modules map visible x=0 to RAM
     *  column 2; set to 0 if the image comes out shifted by two pixels.
     */
    template <typename I2CBus, unsigned Width = 128, unsigned Height = 64, uint8_t ColumnOffset = 2>
    class Sh1106
    {
        static const uint8_t I2cAddress = (0x78 >> 1);

    public:
        enum class Pixel : bool
        {
            Off = false, ///< Pixel off (black)
            On = true ///< Pixel on (color depends of LCD used)
        };

        /**
         * Initialize display
         *
         * @retval true - Display success initialized.
         * @retval false - Display not detected.
         */
        static bool Init();

        /**
         * Fill display with given color (state)
         *
         * @param [in] state Pixel state
         *
         * @par Returns
         *  Nothing
         */
        static void Fill(Pixel state);

        /**
         * Update LCD (async: pages are chained from the DMA completion
         * callback; the framebuffer must not be touched while Busy())
         *
         * @par Returns
         *  Nothing
         */
        static void Update();

        /**
         * Check whether a frame transfer is still in progress
         *
         * @retval true - Transfer in progress
         * @retval false - Idle
         */
        static bool Busy();

        /**
         * Draws pixel (x; y);
         *
         * @param [in] x X coordinate
         * @param [in] y Y coordinate
         * @param [in] state Pixel color (on or off)
         *
         * @par Returns
         *  Nothing
         */
        static void DrawPixel(uint16_t x, uint16_t y, Pixel state = Pixel::On);

        /**
         * Sets cursor to given position
         *
         * @param [in] x X coordinate
         * @param [in] y Y coordinate
         *
         * @par Returns
         *  Nothing
         */
        static void Goto(uint16_t x, uint16_t y);

        /**
         * Puts char on current position with monospace font
         *
         * @tparam Font Font for symbol definition
         * @param [in] symbol Character
         */
        template <typename Font>
        static std::enable_if_t<Font::MonoSpace, bool> Putc(char symbol);

        /**
         * Puts char on current position with no monospace font
         *
         * @tparam Font Font for symbol definition
         * @param [in] symbol Character
         */
        template <typename Font>
        static std::enable_if_t<!Font::MonoSpace, bool> Putc(char symbol);

        /**
         * Puts string on current position
         *
         * @tparam Font Font for symbol definition
         * @param [in] str String
         */
        template <typename Font>
        static bool Puts(const char* str);

    private:
        /**
         * Write command to LCD
         *
         * @param [in] command Command
         *
         * @par Returns
         *  Nothing
         */
        static void WriteCommand(uint8_t command);

        /**
         * Start the async transfer of the current page (_page), chaining the
         * next one from the completion callback
         *
         * @par Returns
         *  Nothing
         */
        static void SendPage();

    private:
        static uint8_t _buffer[Width * Height / 8];
        static uint8_t _pageBuffer[6 + Width]; // page addressing prefix + pixels
        static uint16_t _x;
        static uint16_t _y;
        static volatile uint8_t _page;
        static volatile bool _busy;
    };

    #define SH1106_TEMPLATE_ARGS template <typename I2CBus, unsigned Width, unsigned Height, uint8_t ColumnOffset>
    #define SH1106_TEMPLATE_QUALIFIER Sh1106<I2CBus, Width, Height, ColumnOffset>

    SH1106_TEMPLATE_ARGS
    bool SH1106_TEMPLATE_QUALIFIER::Init()
    {
        constexpr uint8_t initSequence[] = {
            0xAE, // display off
            0xA8, // set multiplex ratio (1 to 64)
            0x3F,
            0x81, // set contrast control register
            0x40,
            0xA1, // segment remap on
            0xC8, // reverse COM direction
            0xDA, // set COM pins config
            0x12,
            0xD3, // display offset = 0
            0x00,
            0x40, // start line = 0
            0xD9, // set pre-charge period
            0x22,
            0xAF, // turn on Sh1106 panel
        };

        if (I2CBus::Write(I2cAddress, 0x00, initSequence, sizeof(initSequence)) != I2cStatus::Success) {
            return false;
        }

        Fill(Pixel::Off);
        Update();

        _x = 0;
        _y = 0;

        return true;
    }

    SH1106_TEMPLATE_ARGS
    void SH1106_TEMPLATE_QUALIFIER::Fill(Pixel state)
    {
        memset(_buffer, state == Pixel::Off ? 0x00 : 0xff, sizeof(_buffer));
    }

    SH1106_TEMPLATE_ARGS
    void SH1106_TEMPLATE_QUALIFIER::Update()
    {
        while(_busy) continue;

        _busy = true;
        _page = 0;
        SendPage();
    }

    SH1106_TEMPLATE_ARGS
    bool SH1106_TEMPLATE_QUALIFIER::Busy()
    {
        return _busy;
    }

    SH1106_TEMPLATE_ARGS
    void SH1106_TEMPLATE_QUALIFIER::SendPage()
    {
        // The whole page is one I2C transaction: the addressing commands ride
        // in front of the pixel data using Co-bit framing (0x80 = one command
        // follows, 0x40 = the rest is data).
        _pageBuffer[0] = 0xB0 + _page; // Set the current RAM page address.
        _pageBuffer[1] = 0x80;
        _pageBuffer[2] = 0x00 | (ColumnOffset & 0x0F);
        _pageBuffer[3] = 0x80;
        _pageBuffer[4] = 0x10 | (ColumnOffset >> 4);
        _pageBuffer[5] = 0x40;
        memcpy(&_pageBuffer[6], &_buffer[_page * Width], Width);

        auto status = I2CBus::WriteAsync(I2cAddress, 0x80, _pageBuffer, sizeof(_pageBuffer), I2cOpts::None,
            [](I2cStatus status) {
                if(!_busy)
                    return;

                if(status != I2cStatus::Success || ++_page >= Height / 8) {
                    _busy = false;
                    return;
                }
                SendPage();
            });

        if(status != I2cStatus::Success)
            _busy = false;
    }

    SH1106_TEMPLATE_ARGS
    void SH1106_TEMPLATE_QUALIFIER::DrawPixel(uint16_t x, uint16_t y, Pixel state)
    {
        if(x >= Width || y >= Height)
            return;
        if(state == Pixel::On)
        {
            _buffer[(y / 8) * Width + x] |= 1 << (y % 8);
        }
        else
        {
            _buffer[(y / 8) * Width + x] &= ~(1 << (y % 8));
        }
    }

    SH1106_TEMPLATE_ARGS
    void SH1106_TEMPLATE_QUALIFIER::Goto(uint16_t x, uint16_t y)
    {
        _x = x;
        _y = y;
    }

    SH1106_TEMPLATE_ARGS
    template <typename Font>
    std::enable_if_t<Font::MonoSpace, bool> SH1106_TEMPLATE_QUALIFIER::Putc(char symbol)
    {
        if (Width <= (_x + Font::Width) || Height <= (_y + Font::Height))
        {
            return false;
        }

        uint8_t page = 0;
        for(; page < Font::Height / 8; ++page)
        {
            for (uint8_t column = 0; column < Font::Width; ++column)
            {
                _buffer[(_y / 8 + page) * Width + _x + column] = Font::Get(symbol)[page * Font::Width + column];
            }
        }
        const uint8_t extraBits = Font::Height % 8;
        if constexpr (extraBits > 0)
        {
            for (uint8_t column = 0; column < Font::Width; ++column)
            {
                _buffer[(_y + Font::Height) / 8 * Width + _x + column] =
                    (_buffer[(_y + Font::Height) / 8 * Width + _x + column] & (0xff << extraBits))
                    | (Font::Get(symbol)[page * Font::Width + column] & (0xff >> (8 - extraBits)));
            }
        }

        _x += Font::Width + 1;

        return true;
    }

    SH1106_TEMPLATE_ARGS
    template <typename Font>
    std::enable_if_t<!Font::MonoSpace, bool> SH1106_TEMPLATE_QUALIFIER::Putc(char symbol)
    {
        volatile uint8_t width = Font::GetWidth(symbol);
        if (Width <= (_x + width) || Height <= (_y + Font::Height))
        {
            return false;
        }

        uint8_t page = 0;
        for(; page < Font::Height / 8; ++page)
        {
            for (uint8_t column = 0; column < width; ++column)
            {
                _buffer[(_y / 8 + page) * Width + _x + column] = Font::Get(symbol)[page * width + column];
            }
        }
        const uint8_t extraBits = Font::Height % 8;
        if constexpr (extraBits > 0)
        {
            for (uint8_t column = 0; column < width; ++column)
            {
                _buffer[(_y + Font::Height) / 8 * Width + _x + column] =
                    (_buffer[(_y + Font::Height) / 8 * Width + _x + column] & (0xff << extraBits))
                    | (Font::Get(symbol)[page * width + column] >> (8 - extraBits));
            }
        }

        _x += width + 1;

        return true;
    }

    SH1106_TEMPLATE_ARGS
    template <typename Font>
    bool SH1106_TEMPLATE_QUALIFIER::Puts(const char* str)
    {
        while (*str)
        {
            if (!Putc<Font>(*str))
            {
                return false;
            }
            ++str;
        }
        return true;
    }

    SH1106_TEMPLATE_ARGS
    void SH1106_TEMPLATE_QUALIFIER::WriteCommand(uint8_t command)
    {
        I2CBus::WriteU8(I2cAddress, 0x00, command);
    }

    SH1106_TEMPLATE_ARGS
    uint8_t SH1106_TEMPLATE_QUALIFIER::_buffer[Width * Height / 8];
    SH1106_TEMPLATE_ARGS
    uint8_t SH1106_TEMPLATE_QUALIFIER::_pageBuffer[6 + Width];
    SH1106_TEMPLATE_ARGS
    uint16_t SH1106_TEMPLATE_QUALIFIER::_x = 0;
    SH1106_TEMPLATE_ARGS
    uint16_t SH1106_TEMPLATE_QUALIFIER::_y = 0;
    SH1106_TEMPLATE_ARGS
    volatile uint8_t SH1106_TEMPLATE_QUALIFIER::_page = 0;
    SH1106_TEMPLATE_ARGS
    volatile bool SH1106_TEMPLATE_QUALIFIER::_busy = false;

    #undef SH1106_TEMPLATE_ARGS
    #undef SH1106_TEMPLATE_QUALIFIER
}

#endif //! ZHELE_DRIVERS_SH1106_H
