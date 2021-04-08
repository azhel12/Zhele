/**
 * @file
 * Implements class for Ssd1306 OLED display
 * @author Alexey Zhelonkin (based on Tilen Majerle library)
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_SSD1306_H
#define ZHELE_DRIVERS_SSD1306_H

#include <delay.h>

#include <cstring>

namespace Zhele::Drivers
{
    template <typename I2CBus>
    class Ssd1306
    {
        static const uint8_t I2cAddress = (0x78 >> 1);
        static const uint8_t Width = 128;
        static const uint8_t Height = 64;

        // Ssd1306 commands
        enum Commands : uint8_t
        {
            SetMemoryMode = 0x20, ///< Set Memory Addressing Mode
            On = 0xAF, ///< Display On
            Off = 0xAE, ///< Display off
        };       

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
         * Update LCD
         * 
         * @par Returns
         *  Nothing
         */
        static void Update();

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
        void WriteCommand(uint8_t command);

    private:
        static uint8_t _buffer[Width * Height / 8];
        static uint16_t _x;
        static uint16_t _y;
    };

    template <typename I2CBus>
    bool Ssd1306<I2CBus>::Init()
    {
        const uint8_t initSequence[] = {
            Commands::Off,
            Commands::SetMemoryMode,
            0x00, // Horizontal Addressing Mode.

            0x21, // Set columns = 0..127
            0x00,
            0x7f,
            0x22, // Set pages = 0..127
            0x00,
            0x07,

            0x40, // Start line = 0
            0xa1, // segment remap on
            0xc8, // reverse direction
            0xda, // Set com pins config
            0x12, // Disable remap
            0xa6, // Normal mode
            0xa4, // Entire display on
            0x8d, // Enable charge pump regulator
            0x14,
            Commands::On,
        };

        I2CBus::Write(I2cAddress, 0x00, initSequence, sizeof(initSequence));

        Fill(Pixel::Off);
        Update();

        _x = 0;
        _y = 0;

        return true;
    }

    template <typename I2CBus>
    void Ssd1306<I2CBus>::Fill(Pixel state)
    {
        memset(_buffer, state == Pixel::Off ? 0x00 : 0xff, sizeof(_buffer));
    }

    template <typename I2CBus>
    void Ssd1306<I2CBus>::Update()
    {
        I2CBus::WriteAsync(I2cAddress, 0x40, _buffer, Ssd1306<I2CBus>::Width * Ssd1306<I2CBus>::Height / 8);
    }

    template <typename I2CBus>
    void Ssd1306<I2CBus>::DrawPixel(uint16_t x, uint16_t y, Pixel state)
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

    template <typename I2CBus>
    void Ssd1306<I2CBus>::Goto(uint16_t x, uint16_t y)
    {
        _x = x;
        _y = y;
    }

    template <typename I2CBus>
    template <typename Font>
    std::enable_if_t<Font::MonoSpace, bool> Ssd1306<I2CBus>::Putc(char symbol)
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

    template <typename I2CBus>
    template <typename Font>
    std::enable_if_t<!Font::MonoSpace, bool> Ssd1306<I2CBus>::Putc(char symbol)
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

    template <typename I2CBus>
    template <typename Font>
    bool Ssd1306<I2CBus>::Puts(const char* str)
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

    template <typename I2CBus>
    void Ssd1306<I2CBus>::WriteCommand(uint8_t command)
    {
        I2CBus::WriteU8(I2cAddress, 0x00, command);
    }

    template <typename I2CBus>
    uint8_t Ssd1306<I2CBus>::_buffer[Ssd1306<I2CBus>::Width * Ssd1306<I2CBus>::Height / 8];
    template <typename I2CBus>
    uint16_t Ssd1306<I2CBus>::_x = 0;
    template <typename I2CBus>
    uint16_t Ssd1306<I2CBus>::_y = 0;
}

#endif //! ZHELE_DRIVERS_SSD1306_H