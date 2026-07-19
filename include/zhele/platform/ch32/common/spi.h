/**
 * @file
 * SPI master for CH32 (WCH SPI peripheral)
 * @author Aleksei Zhelonkin (based on the STM32 spi implementation by Konstantin Chizhov)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_SPI_H
#define ZHELE_PLATFORM_CH32_COMMON_SPI_H

#include <zhele/common/template_utils/enum.h>

#include <zhele/clock.h>
#include <zhele/dma.h>
#include <zhele/iopins.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    namespace Private
    {
        /// Base class with SPI configuration enums (WCH register bits).
        class SpiBase
        {
        public:
            /// Baud-rate divider (BR field sits at bits 5:3 of CTLR1).
            enum ClockDivider
            {
                Div2    = 0,
                Div4    = 1 << 3,
                Div8    = 2 << 3,
                Div16   = 3 << 3,
                Div32   = 4 << 3,
                Div64   = 5 << 3,
                Div128  = 6 << 3,
                Div256  = 7 << 3,
                Fastest = Div2,
                Fast    = Div8,
                Medium  = Div32,
                Slow    = Div128,
                Slowest = Div256,
            };

            /// NSS (slave-select) management.
            enum SlaveControl
            {
                SoftSlaveControl = SPI_CTLR1_SSM,
                AutoSlaveControl = 0,
            };

            /// SPI unit mode (Master uses software NSS management: SSM|SSI).
            enum Mode
            {
                Master      = SPI_CTLR1_MSTR | SPI_CTLR1_SSM | SPI_CTLR1_SSI,
                MultiMaster = SPI_CTLR1_MSTR,
                Slave       = 0,
            };

            /// Data frame width.
            enum DataSize : uint16_t
            {
                DataSize8  = 0,
                DataSize16 = SPI_CTLR1_DFF,
            };

            enum ClockPolarity
            {
                ClockPolarityLow  = 0,
                ClockPolarityHigh = SPI_CTLR1_CPOL,
            };

            enum ClockPhase
            {
                ClockPhaseLeadingEdge = 0,
                ClockPhaseFallingEdge = SPI_CTLR1_CPHA,
            };

            enum BitOrder
            {
                LsbFirst = SPI_CTLR1_LSBFIRST,
                MsbFirst = 0,
            };
        };

        /**
         * @brief Implements the SPI master.
         *
         * @tparam _Regs Peripheral register wrapper
         * @tparam _Clock Source clock class
         * @tparam _DmaTx TX DMA channel
         * @tparam _DmaRx RX DMA channel
         * @tparam _RemapMask AFIO->PCFR1 remap bit for this SPI (0 if none)
         */
        template<typename _Regs, typename _Clock, typename _DmaTx, typename _DmaRx, uint32_t _RemapMask = 0>
        class Spi : public SpiBase
        {
        public:
            using DmaTx = _DmaTx;
            using DmaRx = _DmaRx;
            using TransferCallback = DmaChannelData::TransferCallback;
            using Regs = _Regs;

            static void Enable();
            static void Disable();

            static void Init(ClockDivider divider = Medium, Mode mode = Master);

            static void SetDivider(ClockDivider divider);

            static void SetClockPolarity(ClockPolarity clockPolarity);

            static void SetClockPhase(ClockPhase clockPhase);

            static void SetBitOrder(BitOrder bitOrder);

            static void SetDataSize(DataSize dataSize);

            static void SetSlaveControl(SlaveControl slaveControl);

            static void SetSS();
            static void ClearSS();

            static bool Busy();

            /// Send and receive one frame (blocking).
            static uint16_t Send(uint16_t value);

            /// Send with ignored receive.
            static void Write(uint16_t data);

            /// Read one frame (send a dummy 0xFFFF).
            static uint16_t Read();

            /**
             * @brief Send + receive a buffer via DMA (non-blocking).
             * @note The RX DMA channel IRQ must route to DmaRx::IrqHandler().
             */
            static void SendAsync(const void* transmitBuffer, void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr);

            /**
             * @brief Send a buffer via DMA, ignoring receive (non-blocking).
             * @note The TX DMA channel IRQ must route to DmaTx::IrqHandler().
             */
            static void WriteAsync(const void* data, uint16_t size, TransferCallback callback = nullptr);

            /// Like WriteAsync but sends a single value @p size times (no memory increment).
            static void WriteAsyncNoIncrement(const void* data, uint16_t size, TransferCallback callback = nullptr);

            /**
             * @brief Read a buffer via DMA (non-blocking).
             * @note The RX DMA channel IRQ must route to DmaRx::IrqHandler().
             */
            static void ReadAsync(void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr);

            /**
             * @brief Configure the SPI pins and apply the AFIO remap (NullPin skips MISO/SS).
             */
            template<typename MosiPin, typename MisoPin, typename SckPin, typename SsPin = IO::NullPin, uint8_t Remap = 0>
            static void SelectPins();

        private:
            static typename _DmaTx::Mode DmaDataSize();
        };
    }
}

#include "impl/spi.h"

#endif // ZHELE_PLATFORM_CH32_COMMON_SPI_H
