/**
 * @file
 * Driver for WS2812B (and compatible) addressable LEDs over SPI
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_DRIVERS_WS2812B_H
#define ZHELE_DRIVERS_WS2812B_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <type_traits>

namespace Zhele::Drivers {
    /// @brief Order in which the LED expects color bytes on the wire
    enum class Ws2812bOrder {
        Grb, ///< WS2812B, WS2812, SK6812
        Rgb, ///< WS2811, some WS2812 clones
        Brg, ///< Rare clones
        Bgr ///< Rare clones
    };

    /// @brief How many SPI bits encode one LED bit
    enum class Ws2812bEncoding
    {
        Auto, ///< Pick whatever the source clock can be divided to
        ThreeBit, ///< 3 SPI bits per LED bit
        FourBit, ///< 4 SPI bits per LED bit
        EightBit ///< 8 SPI bits per LED bit (one byte per LED bit)
    };

    /**
     * @brief Implements driver for WS2812B addressable LED strip over SPI
     *
     * @tparam _SpiBus Target SPI type, must have a TX DMA channel (see spi.h)
     * @tparam _LedsCount LEDs count in the strip
     * @tparam _SpiSourceClockHz SPI source (APB) clock the application sets up. The
     * driver picks the prescaler itself, so this is the only place a frequence is stated
     * @tparam _Order Color byte order on the wire (WS2812B is GRB)
     * @tparam _Encoding SPI bits per LED bit, derived from the source clock by default
     * @tparam _ResetUs Reset (latch) time
     */
    template <typename _SpiBus,
        unsigned _LedsCount,
        unsigned _SpiSourceClockHz,
        Ws2812bOrder _Order = Ws2812bOrder::Grb,
        Ws2812bEncoding _Encoding = Ws2812bEncoding::Auto,
        unsigned _ResetUs = 60>
    class Ws2812b {
        static_assert(_LedsCount > 0, "Strip must contain at least one LED");
        static_assert(!std::is_same_v<typename _SpiBus::DmaTx, void>, "WS2812B driver needs a SPI with TX DMA channel");

        /// Datasheet limits, nanoseconds
        static constexpr unsigned MinZeroHighNs = 200; ///< T0H = 400 ns - 150, and V5 dies want 220
        static constexpr unsigned MaxZeroHighNs = 550; ///< T0H = 400 ns + 150
        static constexpr unsigned MinOneHighNs = 650, MaxOneHighNs = 950; ///< T1H = 800 ns +/- 150
        static constexpr unsigned NominalZeroHighNs = 300, NominalOneHighNs = 800;
        static constexpr unsigned MinSlotNs = 650, MaxSlotNs = 1850; ///< TH + TL = 1250 ns +/- 600
        static constexpr unsigned ThresholdNs = 550;

        /// Prescalers the SPI peripheral offers, from /2 to /256
        static constexpr std::array Dividers {_SpiBus::Div2, _SpiBus::Div4, _SpiBus::Div8, _SpiBus::Div16, _SpiBus::Div32, _SpiBus::Div64, _SpiBus::Div128, _SpiBus::Div256};

        /**
         * @brief SPI bits per LED bit for an encoding
         *
         * @param [in] encoding Encoding (Auto is resolved before this is called)
         *
         * @returns Bits count
         */
        static constexpr unsigned BitsFor(Ws2812bEncoding encoding) {
            switch (encoding) {
                case Ws2812bEncoding::ThreeBit: return 3;
                case Ws2812bEncoding::FourBit: return 4;
                default: return 8;
            }
        }

        /**
         * @brief Everything the prescaler search decided
         */
        struct Setup {
            typename _SpiBus::ClockDivider Divider {}; ///< Prescaler to write
            unsigned ClockHz = 0; ///< Baud rate it produces
            unsigned ZeroHighBits = 0; ///< High SPI bits in the "0" pattern
            unsigned OneHighBits = 0; ///< High SPI bits in the "1" pattern
            unsigned Margin = 0; ///< Worst case distance of a pulse from ThresholdNs, nanoseconds
            bool Fits = false; ///< Whether the LED can read this at all
        };

        /**
         * @brief Fit the two patterns into one encoding clocked at a given baud rate
         *
         * @param [in] bits SPI bits per LED bit
         * @param [in] clockHz SPI baud rate
         *
         * @returns Setup (Fits = false when this combination cannot be read)
         */
        static constexpr Setup EvaluateClock(unsigned bits, unsigned clockHz) {
            const auto bitNs = static_cast<unsigned>(1'000'000'000ull / clockHz);

            if (bits * bitNs < MinSlotNs || bits * bitNs > MaxSlotNs)
                return Setup {};

            Setup setup {};

            unsigned bestError = 0xffffffff;
            for (unsigned high = 1; high < bits; ++high) {
                const unsigned highNs = high * bitNs;

                if (highNs < MinZeroHighNs || highNs > MaxZeroHighNs)
                    continue;

                const unsigned error = highNs > NominalZeroHighNs ? highNs - NominalZeroHighNs : NominalZeroHighNs - highNs;
                if (error < bestError) {
                    bestError = error;
                    setup.ZeroHighBits = high;
                }
            }

            bestError = 0xffffffff;
            for (unsigned high = setup.ZeroHighBits + 1; high < bits; ++high) {
                const unsigned highNs = high * bitNs;

                if (setup.ZeroHighBits == 0 || highNs < MinOneHighNs || highNs > MaxOneHighNs)
                    continue;

                const unsigned error = highNs > NominalOneHighNs ? highNs - NominalOneHighNs : NominalOneHighNs - highNs;
                if (error < bestError) {
                    bestError = error;
                    setup.OneHighBits = high;
                }
            }

            if (setup.ZeroHighBits == 0 || setup.OneHighBits == 0)
                return Setup {};

            const unsigned zeroHighNs = setup.ZeroHighBits * bitNs;
            const unsigned oneHighNs = setup.OneHighBits * bitNs;

            setup.ClockHz = clockHz;
            setup.Margin = std::min(zeroHighNs < ThresholdNs ? ThresholdNs - zeroHighNs : 0u,
                oneHighNs > ThresholdNs ? oneHighNs - ThresholdNs : 0u);
            setup.Fits = setup.Margin > 0;

            return setup;
        }

        /**
         * @brief Best prescaler for an encoding
         *
         * @param [in] bits SPI bits per LED bit
         *
         * @returns Setup (Fits = false when no prescaler suits this encoding)
         */
        static constexpr Setup ChooseSetup(unsigned bits) {
            Setup best {};

            for (unsigned i = 0; i < Dividers.size(); ++i) {
                Setup candidate = EvaluateClock(bits, _SpiSourceClockHz >> (i + 1));

                if (candidate.Fits && candidate.Margin > best.Margin) {
                    candidate.Divider = Dividers[i];
                    best = candidate;
                }
            }

            return best;
        }

        /**
         * @brief Encoding the source clock can actually be divided to
         *
         * @returns Encoding
         */
        static constexpr Ws2812bEncoding ResolveEncoding() {
            if constexpr (_Encoding != Ws2812bEncoding::Auto) {
                return _Encoding;
            }
            else {
                const Setup three = ChooseSetup(3);
                const Setup four = ChooseSetup(4);
                const Setup eight = ChooseSetup(8);

                if (three.Fits && three.Margin >= four.Margin && three.Margin >= eight.Margin)
                    return Ws2812bEncoding::ThreeBit;
                if (four.Fits && four.Margin >= eight.Margin)
                    return Ws2812bEncoding::FourBit;

                return Ws2812bEncoding::EightBit;
            }
        }

        static constexpr Ws2812bEncoding Encoding = ResolveEncoding();
        static constexpr unsigned SpiBitsPerLedBit = BitsFor(Encoding);
        static constexpr Setup Choice = ChooseSetup(SpiBitsPerLedBit);
        static_assert(Choice.Fits, "No SPI prescaler lets this encoding read on a WS2812B");

        static constexpr unsigned ZeroPattern = ((1u << Choice.ZeroHighBits) - 1) << (SpiBitsPerLedBit - Choice.ZeroHighBits);
        static constexpr unsigned OnePattern = ((1u << Choice.OneHighBits) - 1) << (SpiBitsPerLedBit - Choice.OneHighBits);
        static constexpr unsigned DataBitIndex = Choice.ZeroHighBits;
        static constexpr unsigned SpiClockHz = Choice.ClockHz;
        static constexpr typename _SpiBus::ClockDivider Divider = Choice.Divider;
        static constexpr unsigned SpiBitNs = SpiClockHz > 0 ? static_cast<unsigned>(1'000'000'000ull / SpiClockHz) : 0;

    public:
        /// High pulse of a LED "0", nanoseconds (datasheet T0H = 400 ns +/- 150)
        static constexpr unsigned ZeroHighNs = Choice.ZeroHighBits * SpiBitNs;
        /// High pulse of a LED "1", nanoseconds (datasheet T1H = 800 ns +/- 150)
        static constexpr unsigned OneHighNs = Choice.OneHighBits * SpiBitNs;
        /// Whole bit slot, nanoseconds (datasheet TH + TL = 1250 ns +/- 600)
        static constexpr unsigned BitSlotNs = SpiBitsPerLedBit * SpiBitNs;

    private:

        static constexpr unsigned BytesPerColorByte = SpiBitsPerLedBit;
        static constexpr unsigned BytesPerLed = 3 * BytesPerColorByte;
        static constexpr unsigned ResetBytes = static_cast<unsigned>((static_cast<uint64_t>(_ResetUs) * SpiClockHz + 7'999'999ull) / 8'000'000ull);
        static constexpr unsigned BufferSize = _LedsCount * BytesPerLed + ResetBytes;
        /// DMA counts transfers in a 16-bit register, so one frame must fit into it
        static_assert(BufferSize <= 65535, "Strip is too long: encoded frame does not fit into a single DMA transfer");

        /**
         * @brief Build encoded patterns for every nibble (4 LED bits -> 4 * SpiBitsPerLedBit SPI bits)
         *
         * @returns Patterns table
         */
        static constexpr std::array<uint32_t, 16> BuildNibblePatterns() {
            std::array<uint32_t, 16> patterns {};

            for (unsigned nibble = 0; nibble < patterns.size(); ++nibble) {
                uint32_t pattern = 0;
                for (int bit = 3; bit >= 0; --bit) {
                    pattern = (pattern << SpiBitsPerLedBit)
                        | ((nibble & (1u << bit)) != 0 ? OnePattern : ZeroPattern);
                }
                patterns[nibble] = pattern;
            }

            return patterns;
        }

        /// Encoded patterns for every nibble (up to 4 * 8 = 32 SPI bits)
        static constexpr std::array<uint32_t, 16> NibblePatterns = BuildNibblePatterns();

    public:
        /**
         * @brief Color of one LED
         */
        struct Color {
            uint8_t Red = 0; ///< Red component
            uint8_t Green = 0; ///< Green component
            uint8_t Blue = 0; ///< Blue component

            /**
             * @brief Build color from HSV (handy for rainbows and fades)
             *
             * @param [in] hue Hue (0..255 instead of 0..359 degrees)
             * @param [in] saturation Saturation
             * @param [in] value Value (brightness)
             *
             * @returns Color
             */
            static constexpr Color FromHsv(uint8_t hue, uint8_t saturation = 255, uint8_t value = 255) {
                if (saturation == 0)
                    return Color {value, value, value};

                const unsigned sector = (hue * 6u) >> 8;
                const unsigned offset = (hue * 6u) & 0xff;

                const auto p = static_cast<uint8_t>((value * (255u - saturation)) >> 8);
                const auto q = static_cast<uint8_t>((value * (255u - ((saturation * offset) >> 8))) >> 8);
                const auto t = static_cast<uint8_t>((value * (255u - ((saturation * (255u - offset)) >> 8))) >> 8);

                switch (sector) {
                    case 0: return Color {value, t, p};
                    case 1: return Color {q, value, p};
                    case 2: return Color {p, value, t};
                    case 3: return Color {p, q, value};
                    case 4: return Color {t, p, value};
                    default: return Color {value, p, q};
                }
            }
        };

        using TransferCallback = typename _SpiBus::TransferCallback;

        /**
         * @brief Init SPI for the strip and blank it
         *
         * @par Returns
         *  Nothing
         */
        static void Init() {
            _SpiBus::Init(Divider, _SpiBus::Master);
            _SpiBus::SetDataSize(_SpiBus::DataSize8);
            _SpiBus::SetBitOrder(_SpiBus::MsbFirst);
            _SpiBus::SetClockPolarity(_SpiBus::ClockPolarityLow);
            _SpiBus::SetClockPhase(_SpiBus::ClockPhaseLeadingEdge);

            Clear();
        }

        /**
         * @brief LEDs count in the strip
         *
         * @returns LEDs count
         */
        static constexpr unsigned Count() {
            return _LedsCount;
        }

        /**
         * @brief Set color of one LED
         *
         * @param [in] index LED index
         * @param [in] color Color
         *
         * @par Returns
         *  Nothing
         */
        static void SetPixel(unsigned index, Color color) {
            if (index >= _LedsCount)
                return;

            uint8_t* destination = &_buffer[index * BytesPerLed];

            for (uint8_t value : ToWireOrder(color)) {
                EncodeByte(destination, value);
                destination += BytesPerColorByte;
            }
        }

        /**
         * @brief Returns color of one LED
         *
         * @param [in] index LED index
         *
         * @returns Color (black for out of range index)
         */
        static Color GetPixel(unsigned index) {
            if (index >= _LedsCount)
                return Color {};

            const uint8_t* source = &_buffer[index * BytesPerLed];

            const uint8_t first = DecodeByte(source);
            const uint8_t second = DecodeByte(source + BytesPerColorByte);
            const uint8_t third = DecodeByte(source + 2 * BytesPerColorByte);

            if constexpr (_Order == Ws2812bOrder::Grb) {
                return Color {second, first, third};
            }
            if constexpr (_Order == Ws2812bOrder::Rgb) {
                return Color {first, second, third};
            }
            if constexpr (_Order == Ws2812bOrder::Brg) {
                return Color {second, third, first};
            }
            return Color {third, second, first};
        }

        /**
         * @brief Set the whole strip to one color
         *
         * @param [in] color Color
         *
         * @par Returns
         *  Nothing
         */
        static void Fill(Color color) {
            SetPixel(0, color);

            for (unsigned index = 1; index < _LedsCount; ++index) {
                for (unsigned i = 0; i < BytesPerLed; ++i) {
                    _buffer[index * BytesPerLed + i] = _buffer[i];
                }
            }
        }

        /**
         * @brief Turn the whole strip off
         *
         * @par Returns
         *  Nothing
         */
        static void Clear() {
            Fill(Color {});
        }

        /**
         * @brief Push the buffer to the strip and wait for latch
         *
         * @par Returns
         *  Nothing
         */
        static void Flush() {
            FlushAsync();

            while (Busy())
                continue;
        }

        /**
         * @brief Push the buffer to the strip by DMA
         *
         * @param [in, opt] callback Transfer complete callback
         *
         * @par Returns
         *  Nothing
         */
        static void FlushAsync(TransferCallback callback = nullptr) {
            _SpiBus::WriteAsync(_buffer, BufferSize, callback);
        }

        /**
         * @brief Returns transfer state
         *
         * @return true Transfer (or reset pause after it) is in progress
         * @return false Strip has latched the colors and is ready for the next frame
         */
        static bool Busy() {
            return !_SpiBus::DmaTx::Ready() || _SpiBus::Busy();
        }

    private:
        /**
         * @brief Returns color bytes in the order the LED expects them
         *
         * @param [in] color Color
         *
         * @returns Three color bytes
         */
        static constexpr std::array<uint8_t, 3> ToWireOrder(Color color) {
            if constexpr (_Order == Ws2812bOrder::Grb) {
                return {color.Green, color.Red, color.Blue};
            }
            else if constexpr (_Order == Ws2812bOrder::Rgb) {
                return {color.Red, color.Green, color.Blue};
            }
            else if constexpr (_Order == Ws2812bOrder::Brg) {
                return {color.Blue, color.Red, color.Green};
            }
            else {
                return {color.Blue, color.Green, color.Red};
            }
        }

        /**
         * @brief Encode one color byte into SPI bytes
         *
         * @param [out] destination Destination (BytesPerColorByte bytes)
         * @param [in] value Color byte
         *
         * @par Returns
         *  Nothing
         */
        static constexpr void EncodeByte(uint8_t* destination, uint8_t value) {
            const uint64_t pattern = (static_cast<uint64_t>(NibblePatterns[value >> 4]) << (4 * SpiBitsPerLedBit))
                | NibblePatterns[value & 0x0f];

            for (unsigned i = 0; i < BytesPerColorByte; ++i) {
                destination[i] = static_cast<uint8_t>(pattern >> (8 * (BytesPerColorByte - 1 - i)));
            }
        }

        /**
         * @brief Decode one color byte back from SPI bytes
         *
         * @param [in] source Source (BytesPerColorByte bytes)
         *
         * @returns Color byte
         */
        static constexpr uint8_t DecodeByte(const uint8_t* source) {
            uint64_t pattern = 0;
            for (unsigned i = 0; i < BytesPerColorByte; ++i) {
                pattern = (pattern << 8) | source[i];
            }

            // The patterns differ from DataBitIndex on, so that bit is the LED bit itself
            uint8_t value = 0;
            for (unsigned bit = 0; bit < 8; ++bit) {
                const unsigned shift = 8 * BytesPerColorByte - 1 - bit * SpiBitsPerLedBit - DataBitIndex;
                value = static_cast<uint8_t>((value << 1) | ((pattern >> shift) & 1));
            }

            return value;
        }

        static inline uint8_t _buffer[BufferSize] {};
    };
}

#endif //! ZHELE_DRIVERS_WS2812B_H
