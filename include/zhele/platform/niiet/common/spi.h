/**
 * @file
 * SPI master/slave driver for NIIET K1921VG5T / K1921VG7T.
 *
 * The block is an ARM PrimeCell PL022 (SSP) with one difference from the classic
 * part — and from the K1921VG015, which has the classic layout: CR0 and CR1 are
 * fused into a single 32-bit CR (DSS, SCR, FRF, SPO, SPH, LBM, SSE, MS, SOD plus
 * the FIFO level selects). Everything else is stock SSP: a 16-bit DR behind
 * 8-entry FIFOs, the SR flags (TFE/TNF/RNE/RFF/BSY), and the two-stage clock
 * divider
 *
 *   fSCK = fSPICLK / (CPSDVSR × (1 + SCR))      (РП формула 22.2)
 *
 * where CPSDVSR is an even 2…254 in CPSR and SCR is 0…255 in CR. fSPICLK is the
 * per-instance reference clock selected in RCU->SPICFG[n] (Clock::SpiNClock).
 *
 * Data frames are 4…16 bits (CR.DSS = bits-1), so the data register is 16-bit —
 * the driver keeps the uint16_t frame API used by the other Zhele SPI back-ends.
 *
 * @author Alexey Zhelonkin (NIIET port)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_NIIET_COMMON_SPI_H
#define ZHELE_PLATFORM_NIIET_COMMON_SPI_H

#include "ioreg.h"

#include <zhele/clock.h>
#include <zhele/dma.h>
#include <zhele/iopins.h>
#include <zhele/pinlist.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    namespace Private
    {
        /// Base class with SPI configuration enums (PL022 CR/CPSR bits).
        class SpiBase
        {
        public:
            /**
             * @brief Baud-rate divider.
             *
             * Encoded as the SCR value used with CPSDVSR = 2, i.e. DivN divides the
             * reference clock by N. Use SetDivider(uint32_t) for an arbitrary ratio.
             */
            enum ClockDivider : uint32_t
            {
                Div2    = 0,
                Div4    = 1,
                Div8    = 3,
                Div16   = 7,
                Div32   = 15,
                Div64   = 31,
                Div128  = 63,
                Div256  = 127,
                Fastest = Div2,
                Fast    = Div8,
                Medium  = Div32,
                Slow    = Div128,
                Slowest = Div256,
            };

            /// SPI unit mode (CR.MS).
            enum Mode : uint32_t
            {
                Master = 0,
                Slave  = SPI_CR_MS_Msk,
            };

            /// Frame format (CR.FRF).
            enum FrameFormat : uint32_t
            {
                Motorola         = 0u << SPI_CR_FRF_Pos,
                TexasInstruments = 1u << SPI_CR_FRF_Pos,
                Microwire        = 2u << SPI_CR_FRF_Pos,
            };

            /// Data frame width (CR.DSS holds bits-1; 4…16 bits are supported).
            enum DataSize : uint32_t
            {
                DataSize4  = 3,
                DataSize8  = 7,
                DataSize16 = 15,
            };

            enum ClockPolarity : uint32_t
            {
                ClockPolarityLow  = 0,
                ClockPolarityHigh = SPI_CR_SPO_Msk,
            };

            enum ClockPhase : uint32_t
            {
                ClockPhaseLeadingEdge  = 0,
                ClockPhaseFallingEdge  = SPI_CR_SPH_Msk,
            };
        };

        /**
         * @brief PL022 SPI instance.
         *
         * @tparam _Regs      IO_STRUCT_WRAPPER over the SPI_TypeDef.
         * @tparam _Clock     Clock control (Enable / Disable / ClockFreq).
         * @tparam _DmaTx     DMA TX channel (or void to disable async write).
         * @tparam _DmaRx     DMA RX channel (or void to disable async read).
         * @tparam _MosiPins  Pin map for the TX line.
         * @tparam _MisoPins  Pin map for the RX line.
         * @tparam _ClkPins   Pin map for the CLK line.
         * @tparam _SsPins    Pin map for the FSS (slave select) line.
         */
        template<typename _Regs, typename _Clock, typename _DmaTx, typename _DmaRx,
                 typename _MosiPins, typename _MisoPins, typename _ClkPins, typename _SsPins>
        class Spi : public SpiBase
        {
            // CPSDVSR must be even; 2 is the fastest and what the ClockDivider enum assumes.
            static constexpr uint32_t DefaultPrescaler = 2;

        public:
            using Regs = _Regs;
            using DmaTx = _DmaTx;
            using DmaRx = _DmaRx;
            using TransferCallback = DmaChannelData::TransferCallback;

            static void Enable()  { _Regs()->CR |= SPI_CR_SSE_Msk; }
            static void Disable() { _Regs()->CR &= ~SPI_CR_SSE_Msk; }

            /**
             * @brief Enable the block and configure it as 8-bit Motorola SPI.
             *
             * @param divider Baud-rate divider (see ClockDivider).
             * @param mode    Master or Slave.
             */
            static void Init(ClockDivider divider = Medium, Mode mode = Master)
            {
                _Clock::Enable();
                _Regs()->CR = 0; // SSE = 0 while reconfiguring
                _Regs()->CPSR = DefaultPrescaler;
                _Regs()->CR = static_cast<uint32_t>(DataSize8)
                            | (static_cast<uint32_t>(divider) << SPI_CR_SCR_Pos)
                            | static_cast<uint32_t>(Motorola)
                            | static_cast<uint32_t>(mode)
                            | SPI_CR_SSE_Msk;
            }

            static void SetDivider(ClockDivider divider)
            {
                const bool wasEnabled = (_Regs()->CR & SPI_CR_SSE_Msk) != 0;
                Disable();
                _Regs()->CPSR = DefaultPrescaler;
                _Regs()->CR = (_Regs()->CR & ~SPI_CR_SCR_Msk)
                            | (static_cast<uint32_t>(divider) << SPI_CR_SCR_Pos);
                if (wasEnabled)
                    Enable();
            }

            /**
             * @brief Set an arbitrary divider: fSCK = fSPICLK / @p divider.
             *
             * Splits @p divider into CPSDVSR × (1 + SCR), keeping CPSDVSR as small
             * as possible. The split rounds *up*, so the resulting SCK is never
             * faster than fSPICLK / @p divider. Values are clamped to the hardware
             * range 2…254×256.
             */
            static void SetDivider(uint32_t divider)
            {
                if (divider < 2)
                    divider = 2;

                uint32_t prescaler = 2;
                uint32_t scr = 0;
                for (; prescaler <= 254; prescaler += 2)
                {
                    scr = (divider + prescaler - 1) / prescaler; // round up
                    if (scr - 1 <= 255)
                    {
                        scr -= 1;
                        break;
                    }
                }
                if (prescaler > 254)
                {
                    prescaler = 254;
                    scr = 255;
                }

                const bool wasEnabled = (_Regs()->CR & SPI_CR_SSE_Msk) != 0;
                Disable();
                _Regs()->CPSR = prescaler;
                _Regs()->CR = (_Regs()->CR & ~SPI_CR_SCR_Msk) | (scr << SPI_CR_SCR_Pos);
                if (wasEnabled)
                    Enable();
            }

            /// Set SCK frequency as close as possible to (but not above) @p speed.
            static void SetSpeed(uint32_t speed)
            {
                const uint32_t clock = _Clock::ClockFreq();
                SetDivider(speed != 0 ? (clock + speed - 1) / speed : 0xFFFFu);
            }

            static void SetClockPolarity(ClockPolarity clockPolarity)
            {
                UpdateCr(SPI_CR_SPO_Msk, static_cast<uint32_t>(clockPolarity));
            }

            static void SetClockPhase(ClockPhase clockPhase)
            {
                UpdateCr(SPI_CR_SPH_Msk, static_cast<uint32_t>(clockPhase));
            }

            static void SetFrameFormat(FrameFormat format)
            {
                UpdateCr(SPI_CR_FRF_Msk, static_cast<uint32_t>(format));
            }

            static void SetDataSize(DataSize dataSize)
            {
                UpdateCr(SPI_CR_DSS_Msk, static_cast<uint32_t>(dataSize));
            }

            /// True while a frame is in flight or a FIFO still holds data.
            static bool Busy() { return (_Regs()->SR & SPI_SR_BSY_Msk) != 0; }

            static bool WriteReady() { return (_Regs()->SR & SPI_SR_TNF_Msk) != 0; }
            static bool ReadReady()  { return (_Regs()->SR & SPI_SR_RNE_Msk) != 0; }

            /// Send and receive one frame (blocking).
            static uint16_t Send(uint16_t value)
            {
                while (!WriteReady()) { }
                _Regs()->DR = value;
                while (!ReadReady()) { }
                return static_cast<uint16_t>(_Regs()->DR);
            }

            /// Send with ignored receive.
            static void Write(uint16_t data) { Send(data); }

            /// Read one frame (sends a dummy 0xFFFF).
            static uint16_t Read() { return Send(0xFFFF); }

            /// Send a buffer, discarding what comes back (blocking).
            static void Write(const void* data, size_t size)
            {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                for (size_t i = 0; i < size; ++i)
                    Send(bytes[i]);
            }

            /**
             * @brief Send a buffer via DMA, ignoring receive (non-blocking).
             * @note The TX DMA channel IRQ must route to DmaTx::IrqHandler().
             */
            static void WriteAsync(const void* data, size_t size, TransferCallback callback = nullptr)
            {
                if constexpr (!std::is_same_v<_DmaTx, void>)
                {
                    _DmaTx::SetTransferCallback(callback);
                    _Regs()->DMACR |= SPI_DMACR_TXDMAE_Msk;
                    _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement | _DmaTx::MSize8Bits | _DmaTx::PSize8Bits,
                                     data, &_Regs()->DR, size);
                }
            }

            /**
             * @brief Read a buffer via DMA (non-blocking).
             * @note The RX DMA channel IRQ must route to DmaRx::IrqHandler().
             */
            static void ReadAsync(void* receiveBuffer, size_t size, TransferCallback callback = nullptr)
            {
                if constexpr (!std::is_same_v<_DmaRx, void>)
                {
                    _DmaRx::SetTransferCallback(callback);
                    _Regs()->DMACR |= SPI_DMACR_RXDMAE_Msk;
                    _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::MSize8Bits | _DmaRx::PSize8Bits,
                                     receiveBuffer, &_Regs()->DR, size);
                }
            }

            /**
             * @brief Route the SPI signals to the given pins.
             *
             * Pass IO::NullPin for a line the board does not use (a software-driven
             * chip select, or a write-only display with no MISO).
             */
            template<typename MosiPin, typename MisoPin = IO::NullPin,
                     typename SckPin = IO::NullPin, typename SsPin = IO::NullPin>
            static void SelectPins()
            {
                SelectPin<_MosiPins, MosiPin>();
                SelectPin<_MisoPins, MisoPin>();
                SelectPin<_ClkPins, SckPin>();
                SelectPin<_SsPins, SsPin>();
            }

            /// Route the SPI signals to the default pins (first entry of each map).
            static void SelectPins()
            {
                SelectPins<typename _MosiPins::io_pins::template Pin<0>,
                           typename _MisoPins::io_pins::template Pin<0>,
                           typename _ClkPins::io_pins::template Pin<0>,
                           typename _SsPins::io_pins::template Pin<0>>();
            }

        private:
            static void UpdateCr(uint32_t mask, uint32_t value)
            {
                const bool wasEnabled = (_Regs()->CR & SPI_CR_SSE_Msk) != 0;
                Disable();
                _Regs()->CR = (_Regs()->CR & ~mask) | (value & mask);
                if (wasEnabled)
                    Enable();
            }

            template<typename PinMap, typename Pin>
            static void SelectPin()
            {
                if constexpr (!std::is_same_v<Pin, IO::NullPin>)
                {
                    constexpr int index = PinMap::io_pins::template IndexOf<Pin>;
                    static_assert(index >= 0, "Pin is not a valid pin for this SPI signal");

                    Pin::Port::Enable();
                    Pin::template SetConfiguration<Pin::Port::Configuration::AltFunc>();
                    Pin::template AltFuncNumber<PinMap::alt_functions[index]>();
                }
            }
        };
    } // namespace Private
} // namespace Zhele

#endif // ZHELE_PLATFORM_NIIET_COMMON_SPI_H
