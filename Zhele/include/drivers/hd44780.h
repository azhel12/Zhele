/**
 * @file
 * Provides code for lcd based on HD44780 controller.
 * 
 * I2c version based on LiquidCrystal_I2C
 * 
 * @author Konstantin Chizhov
 * @date 2018
 * @license FreeBSD
 */
#ifndef ZHELE_DRIVERS_HD44780_H
#define ZHELE_DRIVERS_HD44780_H

#include <delay.h>
#include <pinlist.h>

namespace Zhele::Drivers
{
    /**
     * @brief Base class for lcd.
     */
    class LcdBase
    {
    protected:
        /**
         * HD44780 supported commands
         */
        enum Command
        {
            ClearDisplay   = 0x01, ///< Clear display
            ReturnHome     = 0x02, ///< Return cursor home
            EntryModeSet   = 0x04, ///< Set entry mode
            DisplayControl = 0x08, ///< Control display
            CursorShift    = 0x10, ///< Shift cursor
            FunctionSet    = 0x20, ///< Set function
            SetCGRamAddr   = 0x40, ///< Set CGRam address
            SetDDRamAddr   = 0x80, ///< Set DDram address
        };

        /**
         * @brief Entry modes
         */
        enum EntryMode
        {
            Right          = 0x00, ///< Right alignment
            Left           = 0x02, ///< Left alignment
            ShiftIncrement = 0x01, ///< Shit increment
            ShiftDecrement = 0x00 ///< Shit decrement
        };

        /**
         * @brief Display settings
         */
        enum FunctionSet : uint8_t
        {
            Bit8Mode = 0x10, ///< 8-bit mode
            Bit4Mode = 0x00, ///< 4-bit mode
            Line2    = 0x08, ///< 2-lines
            Line1    = 0x00, ///< 1 line
            Dots5x10 = 0x04, ///< Character size 5x10
            Dots5x8  = 0x00 ///< Character size 5x8
        };
    public:
        /**
         * @brief Display power settings.
         */
        enum PowerControl : uint8_t
        {
            DisplayOn  = 0x04, ///< On
            DisplayOff = 0x00, ///< Off
            CursorOn   = 0x02, ///< Cursor on
            CursorOff  = 0x00, ///< Cursor off
            BlinkOn    = 0x01, ///< Cursor blink on
            BlinkOff   = 0x00 ///< Cursor blink off
        };

        /**
         * @brief Delay for hd44780.
         * 
         * @par Returns
         * 	Nothing
         */
        static void Delay()
        {
            delay_us<200>();
        }
    };

    /**
     * @brief Class provides LCD displays with HD44780 mcu
     * 
     * @tparam RS RS pin
     * @tparam E Enable pin
     * @tparam D4 D4 pin
     * @tparam D5 D5 pin
     * @tparam D6 D6 pin
     * @tparam D7 D7 pin
     * @tparam LINE_WIDTH One line width (symbols count)
     * @tparam LINES Lines count
     */
    template<
        typename RS,
        typename E,
        typename D4,
        typename D5,
        typename D6,
        typename D7,
        uint8_t LINE_WIDTH = 8,
        uint8_t LINES = 2
        >
    class Lcd : public LcdBase
    {
    protected:
        using DataBus = IO::PinList<D4, D5, D6, D7>; // Data bus (4 bit mode)
        using LcdPins = IO::PinList<RS, E, D4, D5, D6, D7>; // All pins

        
    public:
        /**
         * @brief Returns line width.
         *
         * @returns Line width.
         */
        static uint8_t LineWidth()
        {
            return LINE_WIDTH;
        }

        /**
		 * @brief Returns lines count.
		 *
		 * @returns Lines count.
		 */
        static uint8_t Lines()
        {
            return LINES;
        }

        /**
         * @brief Init display.
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            LcdPins::Enable();
            LcdPins::template SetConfiguration<LcdPins::Configuration::Out>();
            RS::Clear();
            DataBus::template Write<0x03>();
            Strobe();
            Strobe();
            Strobe();
            delay_ms<60>();
            DataBus::template Write<0x02>();  // set 4 bit mode
            Strobe();

            Write(FunctionSet | Line2 | Dots5x8 | Bit4Mode);
            Write(DisplayControl | DisplayOn | CursorOn | BlinkOn);
            Write(EntryModeSet | Left | ShiftDecrement);
        }

        /**
         * @brief Clear display.
         * 
         * @par Returns
         *  Nothing
         */
        static void Clear()
        {
            RS::Clear();
            Write(ClearDisplay);
            delay_ms<10>();
        }

        /**
         * @brief Returns cursor home.
         * 
         * @par Returns
         *  Nothing
         */
        static void Home()
        {
            RS::Clear();
            Write(ReturnHome);
        }

        /**
         * @brief Set cursor's position.
         * 
         * @param [in] position New position.
         * 
         * @par Returns
         *  Nothing
         */
        static void Goto(uint8_t position)
        {
            RS::Clear();
            Write(SetDDRamAddr | position);
        }

        /**
         * @brief Set cursor's position.
         * 
         * @param [in] x New X-position.
         * @param [in] y New Y-position.
         * 
         * @par Returns
         *  Nothing
         */
        static void Goto(uint8_t x, uint8_t y)
        {
            RS::Clear();
            if (y == 1)
                x += 0x40;
            Write(SetDDRamAddr + x);
            Delay();
        }

        /**
         * @brief Print text.
         * 
         * @param [in] text Text
         * 
         * @par Returns
         *  Nothing
         */
        static void Puts(const char* text)
        {
            RS::Set();
            while(*text) {
                Write(*text++);
            }
        }

        /**
         * @brief Print one character.
         * 
         * @param [in] symbol Symbol
         * 
         * @par Returns
         *  Nothing
         */
        static void Putch(char symbol)
        {
            RS::Set();
            Write(symbol);
        }

        /**
         * @brief Control display power settings.
         * 
         * @tparam DisplayState Display state (on/off).
         * @tparam CursorState Cursor state.
         * @tparam BlinkState Cursor blink state.
         * 
         * @par Returns
         *  Nothing
         */
        template<PowerControl DisplayState, PowerControl CursorState, PowerControl BlinkState>
        static void PowerControl()
        {
            RS::Clear();
            Write(DisplayControl | DisplayState | CursorState | BlinkState);
        }

    protected:
        static void Strobe()
        {
            E::Set();
            Delay();
            E::Clear();
            Delay();
        }

        static void Write(uint8_t c)
        {
            DataBus::Write(c >> 4);
            Strobe();
            DataBus::Write(c);
            Strobe();
        }
    };

    /**
     * @brief Implements Lcd with read support
     * @tparam RS RS pin
     * @tparam RW RW pin
     * @tparam E Enable pin
     * @tparam D4 D4 pin
     * @tparam D5 D5 pin
     * @tparam D6 D6 pin
     * @tparam D7 D7 pin
     * @tparam LINE_WIDTH One line width (symbols count)
     * @tparam LINES Lines count
     */
    template<
    class RS,
    class RW,
    class E,
    class D4,
    class D5,
    class D6,
    class D7,
    uint8_t LINE_WIDTH = 8,
    uint8_t LINES = 2
    >
    class LcdExt : protected Lcd<RS, E, D4, D5, D6, D7, LINE_WIDTH, LINES>
    {
        using Base = Lcd<RS, E, D4, D5, D6, D7, LINE_WIDTH, LINES>;
    public:
        /**
         * @brief Init display.
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            Base::Init();
            RW::template SetConfiguration<RW::Out>();
        }

        /**
         * @brief Check display is busy.
         * 
         * @retval true Display busy.
         * @retval false Display not busy.
         */
        static bool Busy()
        {
            RS::Clear();
            return Read() & 0x80;
        }

    private:
        /**
         * @brief Read value from display.
         * 
         * @returns Readed value.
         */
        static uint8_t Read()
        {
            Base::DataBus::template SetConfiguration<Base::DataBus::In>();
            RW::Set();
            E::Set();
            uint8_t res = Base::DataBus::Read() << 4;
            E::Clear();
            Base::Delay();
            E::Set();
            res |= Base::DataBus::Read();
            E::Clear();
            RW::Clear();
            Base::DataBus::template SetConfiguration<Base::DataBus::Out>();
            return res;
        }
    };

    /**
     * @brief Class provides LCD displays with HD44780 mcu
     * 
     * @tparam _I2cBus I2c bus
     * @tparam _Address I2c address
     * @tparam LINE_WIDTH One line width (symbols count)
     * @tparam LINES Lines count
     */
    template<
        typename _I2cBus,
        uint8_t _Address = 0x27,
        uint8_t LINE_WIDTH = 8,
        uint8_t LINES = 2
        >
    class LcdI2c : public LcdBase
    {
        using DataBus = _I2cBus;
        static const uint8_t BackLight = 0x08;
        static const uint8_t Enable = 0x04;

        enum Mode : uint8_t
        {
            Command = 0x00,
            Data = 0x01
        };

    public:
        /**
         * @brief Returns line width.
         *
         * @returns Line width.
         */
        static uint8_t LineWidth()
        {
            return LINE_WIDTH;
        }

        /**
		 * @brief Returns lines count.
		 *
		 * @returns Lines count.
		 */
        static uint8_t Lines()
        {
            return LINES;
        }

        /**
         * @brief Init display.
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            delay_ms<50>();
            Write(BackLight);
            delay_ms<1000>();

            WriteU4(0x03 << 4);
            delay_ms<5>();
            WriteU4(0x02 << 4);

            WriteU8(FunctionSet | Line2 | Dots5x8 | Bit4Mode);
            WriteU8(DisplayControl | DisplayOn | CursorOn | BlinkOn);
            WriteU8(EntryModeSet | Left | ShiftDecrement);

            Home();
        }

        /**
         * @brief Clear display.
         * 
         * @par Returns
         *  Nothing
         */
        static void Clear()
        {
            WriteU8(ClearDisplay);
            delay_ms<10>();
            Home();
        }

        /**
         * @brief Returns cursor home.
         * 
         * @par Returns
         *  Nothing
         */
        static void Home()
        {
            WriteU8(ReturnHome);
            delay_ms<10>();
        }

        /**
         * @brief Set cursor's position.
         * 
         * @param [in] position New position.
         * 
         * @par Returns
         *  Nothing
         */
        static void Goto(uint8_t position)
        {
            WriteU8(SetDDRamAddr | position);
        }

        /**
         * @brief Set cursor's position.
         * 
         * @param [in] x New X-position.
         * @param [in] y New Y-position.
         * 
         * @par Returns
         *  Nothing
         */
        static void Goto(uint8_t x, uint8_t y)
        {
            if (y == 1)
                x += 0x40;
            WriteU8(SetDDRamAddr | x);
        }

        /**
         * @brief Print text.
         * 
         * @param [in] text Text
         * 
         * @par Returns
         *  Nothing
         */
        static void Puts(const char* text)
        {
            while(*text) {
                WriteU8(*text++, Mode::Data);
            }
        }

        /**
         * @brief Print one character.
         * 
         * @param [in] symbol Symbol
         * 
         * @par Returns
         *  Nothing
         */
        static void Putch(char symbol)
        {
            WriteU8(symbol, Mode::Data);
        }

        /**
         * @brief Control display power settings.
         * 
         * @tparam DisplayState Display state (on/off).
         * @tparam CursorState Cursor state.
         * @tparam BlinkState Cursor blink state.
         * 
         * @par Returns
         *  Nothing
         */
        template<PowerControl DisplayState, PowerControl CursorState, PowerControl BlinkState>
        static void PowerControl()
        {
            WriteU8(DisplayControl | DisplayState | CursorState | BlinkState);
        }

    protected:
        static void WriteU8(uint8_t data, Mode mode = Mode::Command)
        {
            WriteU4((data & 0xf0) | static_cast<uint8_t>(mode));
            WriteU4(((data << 4) & 0xf0) | static_cast<uint8_t>(mode));
        }

        static void WriteU4(uint8_t data)
        {
            Write(data);
            Strobe(data);
        }

        static void Strobe(uint8_t data)
        {
            Write(data | Enable);
            delay_us<1>();
            Write(data & ~Enable);
            delay_us<50>();
        }

        static void Write(uint8_t data)
        {
            DataBus::WriteU8(_Address, 0x00, data | BackLight, I2cOpts::RegAddrNone);
        }
    };
}
#endif //! ZHELE_DRIVERS_HD44780_H
