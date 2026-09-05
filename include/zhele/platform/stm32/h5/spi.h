/**
 * @file
 * Implements SPI protocol for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_SPI_H
#define ZHELE_PLATFORM_STM32_H5_SPI_H

#include <stm32h5xx.h>

#include <zhele/common/template_utils/enum.h>

#include "clock.h"
#include "dma.h"
#include "iopins.h"
#include "pinlist.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    namespace Private
    {
        /**
         * @brief Base class for SPI interface
         */
        class SpiBase
        {
        public:
            /**
             * @brief Defines divider for spi clock
             *
             * @details
             * The SPI kernel clock is divided by 2^(MBR + 1).
             */
            enum ClockDivider
            {
                Div2 = 0 << SPI_CFG1_MBR_Pos,   ///< Divide to 2
                Div4 = 1 << SPI_CFG1_MBR_Pos,   ///< Divide to 4
                Div8 = 2 << SPI_CFG1_MBR_Pos,   ///< Divide to 8
                Div16 = 3 << SPI_CFG1_MBR_Pos,  ///< Divide to 16
                Div32 = 4 << SPI_CFG1_MBR_Pos,  ///< Divide to 32
                Div64 = 5 << SPI_CFG1_MBR_Pos,  ///< Divide to 64
                Div128 = 6 << SPI_CFG1_MBR_Pos, ///< Divide to 128
                Div256 = 7 << SPI_CFG1_MBR_Pos, ///< Divide to 256
                Fastest = Div2,
                Fast = Div8,
                Medium = Div32,
                Slow = Div128,
                Slowest = Div256
            };

            /**
             * @brief Defines slave control (auto or manual)
             */
            enum SlaveControl
            {
                SoftSlaveControl = SPI_CFG2_SSM, ///< SS soft (manual) control
                AutoSlaveControl = 0             ///< SS auto (device) control
            };

            /**
             * @brief SPI unit mode
             *
             * @details
             * The SSOE half lives in CFG2 next to MASTER, so unlike the CR1/CR2
             * families no bit shuffling is needed here.
             */
            enum Mode
            {
                Master = SPI_CFG2_MASTER | SPI_CFG2_SSOE, ///< Master
                MultiMaster = SPI_CFG2_MASTER,            ///< Multi master
                Slave = 0                                 ///< Slave
            };

            /**
             * @brief Data size for data transfer
             *
             * @details
             * DSIZE holds "bits - 1"; H5 supports 4..32 bits, this enum exposes
             * the 4..16 range the framework uses.
             */
            enum DataSize : uint32_t
            {
                DataSize4 = 0x03 << SPI_CFG1_DSIZE_Pos,
                DataSize5 = 0x04 << SPI_CFG1_DSIZE_Pos,
                DataSize6 = 0x05 << SPI_CFG1_DSIZE_Pos,
                DataSize7 = 0x06 << SPI_CFG1_DSIZE_Pos,
                DataSize8 = 0x07 << SPI_CFG1_DSIZE_Pos,
                DataSize9 = 0x08 << SPI_CFG1_DSIZE_Pos,
                DataSize10 = 0x09 << SPI_CFG1_DSIZE_Pos,
                DataSize11 = 0x0A << SPI_CFG1_DSIZE_Pos,
                DataSize12 = 0x0B << SPI_CFG1_DSIZE_Pos,
                DataSize13 = 0x0C << SPI_CFG1_DSIZE_Pos,
                DataSize14 = 0x0D << SPI_CFG1_DSIZE_Pos,
                DataSize15 = 0x0E << SPI_CFG1_DSIZE_Pos,
                DataSize16 = 0x0F << SPI_CFG1_DSIZE_Pos
            };

            /**
             * @brief Clock polarity (CPOL)
             */
            enum ClockPolarity
            {
                ClockPolarityLow = 0,             ///< Low
                ClockPolarityHigh = SPI_CFG2_CPOL ///< High
            };

            /**
             * @brief Clock phase (CPHA)
             */
            enum ClockPhase
            {
                ClockPhaseLeadingEdge = 0,           ///< Leading
                ClockPhaseFallingEdge = SPI_CFG2_CPHA ///< Falling
            };

            /**
             * @brief Bit order
             */
            enum BitOrder
            {
                LsbFirst = SPI_CFG2_LSBFRST, ///< LSB
                MsbFirst = 0                 ///< MSB
            };
        };

        /**
         * @brief Implements spi unit
         *
         * @tparam _Regs Spi registers
         * @tparam _Clock Spi bus clock control class
         * @tparam _MosiPins Pinlist of MOSI pins
         * @tparam _MisoPins Pinlist of MISO pins
         * @tparam _ClockPins Pinlist of CLK pins
         * @tparam _SsPins Pinlist of SS pins
         * @tparam _DmaTx Tx DMA channel
         * @tparam _DmaRx Rx DMA channel
         */
        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        class Spi : public SpiBase
        {
            /**
             * @brief Returns true when the configured data size needs 16-bit accesses
             */
            static bool IsWideData()
            {
                return ((_Regs()->CFG1 & SPI_CFG1_DSIZE_Msk) >> SPI_CFG1_DSIZE_Pos) > 0x07;
            }

            /**
             * @brief Re-arm the endless transaction after a configuration change
             */
            static void Restart()
            {
                Disable();
                _Regs()->IFCR = 0xFFFFFFFFu;
                Enable();
                _Regs()->CR1 |= SPI_CR1_CSTART;
            }

        public:
            using DmaTx = _DmaTx;
            using DmaRx = _DmaRx;
            using TransferCallback = DmaChannelData::TransferCallback;

            /**
             * @brief Enable SPI
             *
             * @par Returns
             *  Nothing
             */
            static void Enable()
            {
                _Regs()->CR1 |= SPI_CR1_SPE;
            }

            /**
             * @brief Disable SPI
             *
             * @par Returns
             *  Nothing
             */
            static void Disable()
            {
                _Regs()->CR1 &= ~SPI_CR1_SPE;
            }

            /**
             * @brief Init SPI interface
             *
             * @param [in] divider Clock divider
             * @param [in] mode SPI mode
             *
             * @par Returns
             *  Nothing
             */
            static void Init(ClockDivider divider = Medium, Mode mode = Master)
            {
                _Clock::Enable();

                Disable();

                // TSIZE = 0: endless transaction, so a single CSTART covers every
                // later Send()/Write() call, like the classic SPI does.
                _Regs()->CR2 = 0;
                _Regs()->CFG1 = static_cast<uint32_t>(divider) | DataSize::DataSize8;
                _Regs()->CFG2 = static_cast<uint32_t>(mode) | SPI_CFG2_AFCNTR;
                _Regs()->IFCR = 0xFFFFFFFFu;

                Enable();

                if ((mode & SPI_CFG2_MASTER) != 0)
                    _Regs()->CR1 |= SPI_CR1_CSTART;
            }

            /**
             * @brief Set SPI clock divider
             *
             * @param [in] divider Clock divider
             *
             * @par Returns
             *  Nothing
             */
            static void SetDivider(ClockDivider divider)
            {
                bool enabled = (_Regs()->CR1 & SPI_CR1_SPE) != 0;
                Disable();
                _Regs()->CFG1 = (_Regs()->CFG1 & ~SPI_CFG1_MBR_Msk) | divider;
                if (enabled)
                    Restart();
            }

            /**
             * @brief Set SPI clock polarity (CPOL)
             *
             * @param [in] clockPolarity Polarity
             *
             * @par Returns
             *  Nothing
             */
            static void SetClockPolarity(ClockPolarity clockPolarity)
            {
                bool enabled = (_Regs()->CR1 & SPI_CR1_SPE) != 0;
                Disable();
                _Regs()->CFG2 = (_Regs()->CFG2 & ~SPI_CFG2_CPOL) | clockPolarity;
                if (enabled)
                    Restart();
            }

            /**
             * @brief Set SPI clock phase (CPHA)
             *
             * @param [in] clockPhase Clock phase
             *
             * @par Returns
             *  Nothing
             */
            static void SetClockPhase(ClockPhase clockPhase)
            {
                bool enabled = (_Regs()->CR1 & SPI_CR1_SPE) != 0;
                Disable();
                _Regs()->CFG2 = (_Regs()->CFG2 & ~SPI_CFG2_CPHA) | clockPhase;
                if (enabled)
                    Restart();
            }

            /**
             * @brief Set SPI bit order (LSB/MSB)
             *
             * @param [in] bitOrder Bit order
             *
             * @par Returns
             *  Nothing
             */
            static void SetBitOrder(BitOrder bitOrder)
            {
                bool enabled = (_Regs()->CR1 & SPI_CR1_SPE) != 0;
                Disable();
                _Regs()->CFG2 = (_Regs()->CFG2 & ~SPI_CFG2_LSBFRST) | bitOrder;
                if (enabled)
                    Restart();
            }

            /**
             * @brief Set SPI data size
             *
             * @details
             * DSIZE is only writable while the peripheral is disabled, so the
             * endless transaction is torn down and re-armed around the change.
             *
             * @param [in] dataSize Data size
             *
             * @par Returns
             *  Nothing
             */
            static void SetDataSize(DataSize dataSize)
            {
                while (Busy())
                    ;

                bool enabled = (_Regs()->CR1 & SPI_CR1_SPE) != 0;
                Disable();
                _Regs()->CFG1 = (_Regs()->CFG1 & ~SPI_CFG1_DSIZE_Msk) | dataSize;
                if (enabled)
                    Restart();
            }

            /**
             * @brief Set slave control (NSS pin)
             *
             * @param [in] slaveControl Slave control
             *
             * @par Returns
             *  Nothing
             */
            static void SetSlaveControl(SlaveControl slaveControl)
            {
                bool enabled = (_Regs()->CR1 & SPI_CR1_SPE) != 0;
                Disable();
                _Regs()->CFG2 = (_Regs()->CFG2 & ~SPI_CFG2_SSM) | slaveControl;
                if (enabled)
                    Restart();
            }

            /**
             * @brief Set slave select (set NSS pin)
             *
             * @par Returns
             *  Nothing
             */
            static void SetSS()
            {
                _Regs()->CR1 |= SPI_CR1_SSI;
            }

            /**
             * @brief Unset slave select (clear NSS pin)
             *
             * @par Returns
             *  Nothing
             */
            static void ClearSS()
            {
                _Regs()->CR1 &= ~SPI_CR1_SSI;
            }

            /**
             * @brief Returns busy state
             *
             * @details
             * There is no BSY flag on this IP: the transmission is finished once
             * the shift register has drained, which TXC reports.
             *
             * @retval true SPI is busy
             * @retval false SPI is not busy
             */
            static bool Busy()
            {
                return (_Regs()->SR & SPI_SR_TXC) == 0;
            }

            /**
             * @brief Send and receive data
             *
             * @param [in] value Data to send
             *
             * @returns Received value
             */
            static uint16_t Send(uint16_t value)
            {
                while ((_Regs()->SR & SPI_SR_TXP) == 0)
                    ;

                if (IsWideData())
                    *reinterpret_cast<volatile uint16_t*>(&_Regs()->TXDR) = value;
                else
                    *reinterpret_cast<volatile uint8_t*>(&_Regs()->TXDR) = static_cast<uint8_t>(value);

                while ((_Regs()->SR & SPI_SR_RXP) == 0)
                    ;

                return IsWideData()
                    ? *reinterpret_cast<volatile uint16_t*>(&_Regs()->RXDR)
                    : *reinterpret_cast<volatile uint8_t*>(&_Regs()->RXDR);
            }

            /**
             * @brief Send data async (by DMA)
             *
             * @param [in] transmitBuffer Data to transmit
             * @param [out] receiveBuffer Output buffer
             * @param [in] bufferSize Data size
             * @param [in, opt] callback Transfer complete callback
             *
             * @par Returns
             *  Nothing
             */
            static void SendAsync(void* transmitBuffer, void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr)
            {
                _DmaRx::ClearTransferComplete();
                _Regs()->CFG1 |= (SPI_CFG1_RXDMAEN | SPI_CFG1_TXDMAEN);

                auto dataSize = IsWideData()
                    ? (_DmaTx::PSize16Bits | _DmaTx::MSize16Bits)
                    : (_DmaTx::PSize8Bits | _DmaTx::MSize8Bits);

                _DmaRx::SetTransferCallback(callback);
                _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | dataSize, receiveBuffer, &_Regs()->RXDR, bufferSize);
                _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement | dataSize, transmitBuffer, &_Regs()->TXDR, bufferSize);
            }

            /**
             * @brief Send data with ignored receive
             *
             * @param [in] data Data to send
             *
             * @par Returns
             *  Nothing
             */
            static void Write(uint16_t data)
            {
                Send(data);
            }

            /**
             * @brief Send data async (by DMA) with ignored receive
             *
             * @param [in] data Data buffer
             * @param [in] size Buffer size (count of elements)
             * @param [in, opt] callback Transfer complete callback
             *
             * @par Returns
             *  Nothing
             */
            static void WriteAsync(const void* data, uint16_t size, TransferCallback callback = nullptr)
            {
                _DmaTx::ClearTransferComplete();
                _Regs()->CFG1 |= SPI_CFG1_TXDMAEN;

                auto dataSize = IsWideData()
                    ? (_DmaTx::PSize16Bits | _DmaTx::MSize16Bits)
                    : (_DmaTx::PSize8Bits | _DmaTx::MSize8Bits);

                _DmaTx::SetTransferCallback(callback);
                _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement | dataSize, data, &_Regs()->TXDR, size);
            }

            /**
             * @brief Send the same value repeatedly async (by DMA)
             *
             * @param [in] data Value to send
             * @param [in] size Repeat count
             * @param [in, opt] callback Transfer complete callback
             *
             * @par Returns
             *  Nothing
             */
            static void WriteAsyncNoIncrement(const void* data, uint16_t size, TransferCallback callback = nullptr)
            {
                _DmaTx::ClearTransferComplete();
                _Regs()->CFG1 |= SPI_CFG1_TXDMAEN;

                auto dataSize = IsWideData()
                    ? (_DmaTx::PSize16Bits | _DmaTx::MSize16Bits)
                    : (_DmaTx::PSize8Bits | _DmaTx::MSize8Bits);

                _DmaTx::SetTransferCallback(callback);
                _DmaTx::Transfer(_DmaTx::Mem2Periph | dataSize, data, &_Regs()->TXDR, size);
            }

            /**
             * @brief Read data (via send 0xFF dummy value)
             *
             * @returns Read value
             */
            static uint16_t Read()
            {
                return Send(0xffff);
            }

            /**
             * @brief Enable async read (by DMA)
             *
             * @param [out] receiveBuffer Output buffer
             * @param [in] bufferSize Size to read
             * @param [in, opt] callback Transfer complete callback
             *
             * @par Returns
             *  Nothing
             */
            static void ReadAsync(void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr)
            {
                static uint16_t dummy = 0xffff;

                _DmaRx::ClearTransferComplete();
                _Regs()->CFG1 |= (SPI_CFG1_RXDMAEN | SPI_CFG1_TXDMAEN);

                auto dataSize = IsWideData()
                    ? (_DmaTx::PSize16Bits | _DmaTx::MSize16Bits)
                    : (_DmaTx::PSize8Bits | _DmaTx::MSize8Bits);

                _DmaRx::SetTransferCallback(callback);
                _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | dataSize, receiveBuffer, &_Regs()->RXDR, bufferSize);
                _DmaTx::Transfer(_DmaTx::Mem2Periph | dataSize, &dummy, &_Regs()->TXDR, bufferSize);
            }

            /**
             * @brief Select pins
             *
             * @param [in] mosiPinNumber MOSI pin number
             * @param [in] misoPinNumber MISO pin number
             * @param [in] clockPinNumber CLK pin number
             * @param [in] ssPinNumber NSS pin number
             *
             * @par Returns
             *  Nothing
             */
            static void SelectPins(int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber)
            {
                using MosiPins = typename _MosiPins::io_pins;
                using MisoPins = typename _MisoPins::io_pins;
                using ClockPins = typename _ClockPins::io_pins;
                using SsPins = typename _SsPins::io_pins;

                using Type = typename MosiPins::DataType;

                if (mosiPinNumber >= 0)
                {
                    MosiPins::Enable();
                    Type mask(1 << mosiPinNumber);
                    MosiPins::SetConfiguration(MosiPins::Configuration::AltFunc, mask);
                    MosiPins::AltFuncNumber(_MosiPins::alt_functions[static_cast<size_t>(mosiPinNumber)], mask);
                }

                if (misoPinNumber >= 0)
                {
                    MisoPins::Enable();
                    Type mask(1 << misoPinNumber);
                    MisoPins::SetConfiguration(MisoPins::Configuration::AltFunc, mask);
                    MisoPins::AltFuncNumber(_MisoPins::alt_functions[static_cast<size_t>(misoPinNumber)], mask);
                }

                if (clockPinNumber >= 0)
                {
                    ClockPins::Enable();
                    Type mask(1 << clockPinNumber);
                    ClockPins::SetConfiguration(ClockPins::Configuration::AltFunc, mask);
                    ClockPins::AltFuncNumber(_ClockPins::alt_functions[static_cast<size_t>(clockPinNumber)], mask);
                }

                if (ssPinNumber >= 0)
                {
                    SsPins::Enable();
                    Type mask(1 << ssPinNumber);
                    SsPins::SetConfiguration(SsPins::Configuration::AltFunc, mask);
                    SsPins::AltFuncNumber(_SsPins::alt_functions[static_cast<size_t>(ssPinNumber)], mask);
                }
            }

            /**
             * @brief Select pins
             *
             * @tparam mosiPinNumber MOSI pin number
             * @tparam misoPinNumber MISO pin number
             * @tparam clockPinNumber CLK pin number
             * @tparam ssPinNumber NSS pin number
             *
             * @par Returns
             *  Nothing
             */
            template<int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber>
            static void SelectPins()
            {
                if constexpr (mosiPinNumber >= 0)
                {
                    using MosiPin = typename _MosiPins::io_pins::template Pin<mosiPinNumber>;
                    MosiPin::Port::Enable();
                    MosiPin::template SetConfiguration<MosiPin::Port::Configuration::AltFunc>();
                    MosiPin::template AltFuncNumber<_MosiPins::alt_functions[mosiPinNumber]>();
                }

                if constexpr (misoPinNumber >= 0)
                {
                    using MisoPin = typename _MisoPins::io_pins::template Pin<misoPinNumber>;
                    MisoPin::Port::Enable();
                    MisoPin::template SetConfiguration<MisoPin::Port::Configuration::AltFunc>();
                    MisoPin::template AltFuncNumber<_MisoPins::alt_functions[misoPinNumber]>();
                }

                if constexpr (clockPinNumber >= 0)
                {
                    using ClockPin = typename _ClockPins::io_pins::template Pin<clockPinNumber>;
                    ClockPin::Port::Enable();
                    ClockPin::template SetConfiguration<ClockPin::Port::Configuration::AltFunc>();
                    ClockPin::template AltFuncNumber<_ClockPins::alt_functions[clockPinNumber]>();
                }

                if constexpr (ssPinNumber >= 0)
                {
                    using SsPin = typename _SsPins::io_pins::template Pin<ssPinNumber>;
                    SsPin::Port::Enable();
                    SsPin::template SetConfiguration<SsPin::Port::Configuration::AltFunc>();
                    SsPin::template AltFuncNumber<_SsPins::alt_functions[ssPinNumber]>();
                }
            }

            /**
             * @brief Select pins
             *
             * @tparam MosiPin MOSI pin
             * @tparam MisoPin MISO pin
             * @tparam ClockPin CLK pin
             * @tparam SsPin NSS pin
             *
             * @par Returns
             *  Nothing
             */
            template<typename MosiPin, typename MisoPin, typename ClockPin, typename SsPin>
            static void SelectPins()
            {
                const int8_t mosiPinIndex = !std::is_same_v<MosiPin, IO::NullPin>
                    ? _MosiPins::io_pins::template IndexOf<MosiPin>
                    : -1;
                const int8_t misoPinIndex = !std::is_same_v<MisoPin, IO::NullPin>
                    ? _MisoPins::io_pins::template IndexOf<MisoPin>
                    : -1;
                const int8_t clockPinIndex = !std::is_same_v<ClockPin, IO::NullPin>
                    ? _ClockPins::io_pins::template IndexOf<ClockPin>
                    : -1;
                const int8_t ssPinIndex = !std::is_same_v<SsPin, IO::NullPin>
                    ? _SsPins::io_pins::template IndexOf<SsPin>
                    : -1;

                static_assert(mosiPinIndex >= -1);
                static_assert(misoPinIndex >= -1);
                static_assert(clockPinIndex >= -1);
                static_assert(ssPinIndex >= -1);

                SelectPins<mosiPinIndex, misoPinIndex, clockPinIndex, ssPinIndex>();
            }
        };

        struct Spi1MosiPins
        {
            using io_pins = IO::PinList<IO::Pa4, IO::Pa7, IO::Pb5, IO::Pc3, IO::Pc7>;
            static constexpr std::array<uint8_t, 5> alt_functions{4, 5, 5, 4, 5};
        };
        struct Spi1MisoPins
        {
            using io_pins = IO::PinList<IO::Pa0, IO::Pa3, IO::Pa6, IO::Pa9, IO::Pb4, IO::Pc2, IO::Pc10>;
            static constexpr std::array<uint8_t, 7> alt_functions{12, 4, 5, 4, 5, 4, 5};
        };
        struct Spi1ClockPins
        {
            using io_pins = IO::PinList<IO::Pa2, IO::Pa5, IO::Pa8, IO::Pb3, IO::Pc0, IO::Pc5>;
            static constexpr std::array<uint8_t, 6> alt_functions{4, 5, 12, 5, 5, 5};
        };
        struct Spi1SsPins
        {
            using io_pins = IO::PinList<IO::Pa1, IO::Pa4, IO::Pa15, IO::Pb8, IO::Pc1, IO::Pc8>;
            static constexpr std::array<uint8_t, 6> alt_functions{4, 5, 5, 12, 4, 5};
        };

        struct Spi2MosiPins
        {
            using io_pins = IO::PinList<IO::Pa8, IO::Pb1, IO::Pb15, IO::Pc1, IO::Pc3>;
            static constexpr std::array<uint8_t, 5> alt_functions{6, 6, 5, 5, 5};
        };
        struct Spi2MisoPins
        {
            using io_pins = IO::PinList<IO::Pa7, IO::Pa15, IO::Pb5, IO::Pb14, IO::Pc2>;
            static constexpr std::array<uint8_t, 5> alt_functions{11, 7, 6, 5, 5};
        };
        struct Spi2ClockPins
        {
            using io_pins = IO::PinList<IO::Pa5, IO::Pa9, IO::Pa12, IO::Pb2, IO::Pb10, IO::Pb13>;
            static constexpr std::array<uint8_t, 6> alt_functions{7, 5, 5, 6, 5, 5};
        };
        struct Spi2SsPins
        {
            using io_pins = IO::PinList<IO::Pa3, IO::Pa8, IO::Pa11, IO::Pb4, IO::Pb12>;
            static constexpr std::array<uint8_t, 5> alt_functions{5, 11, 5, 7, 5};
        };

        struct Spi3MosiPins
        {
            using io_pins = IO::PinList<IO::Pa3, IO::Pa5, IO::Pa9, IO::Pb2, IO::Pb5, IO::Pc12>;
            static constexpr std::array<uint8_t, 6> alt_functions{6, 10, 10, 7, 7, 6};
        };
        struct Spi3MisoPins
        {
            using io_pins = IO::PinList<IO::Pa2, IO::Pa4, IO::Pb4, IO::Pb15, IO::Pc11>;
            static constexpr std::array<uint8_t, 5> alt_functions{6, 10, 6, 6, 6};
        };
        struct Spi3ClockPins
        {
            using io_pins = IO::PinList<IO::Pa1, IO::Pa15, IO::Pb3, IO::Pb7, IO::Pc10>;
            static constexpr std::array<uint8_t, 5> alt_functions{6, 10, 6, 6, 6};
        };
        struct Spi3SsPins
        {
            using io_pins = IO::PinList<IO::Pa0, IO::Pa4, IO::Pa15, IO::Pb10, IO::Pd2>;
            static constexpr std::array<uint8_t, 5> alt_functions{10, 6, 6, 6, 6};
        };

        IO_STRUCT_WRAPPER(SPI1, Spi1Regs, SPI_TypeDef);
    #if defined (SPI2)
        IO_STRUCT_WRAPPER(SPI2, Spi2Regs, SPI_TypeDef);
    #endif
    #if defined (SPI3)
        IO_STRUCT_WRAPPER(SPI3, Spi3Regs, SPI_TypeDef);
    #endif
    }

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi1 = Private::Spi<Private::Spi1Regs, Clock::Spi1Clock, Private::Spi1MosiPins, Private::Spi1MisoPins, Private::Spi1ClockPins, Private::Spi1SsPins, _DmaTx, _DmaRx>;
    using Spi1NoDma = Spi1<>;

#if defined (SPI2)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi2 = Private::Spi<Private::Spi2Regs, Clock::Spi2Clock, Private::Spi2MosiPins, Private::Spi2MisoPins, Private::Spi2ClockPins, Private::Spi2SsPins, _DmaTx, _DmaRx>;
    using Spi2NoDma = Spi2<>;
#endif

#if defined (SPI3)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi3 = Private::Spi<Private::Spi3Regs, Clock::Spi3Clock, Private::Spi3MosiPins, Private::Spi3MisoPins, Private::Spi3ClockPins, Private::Spi3SsPins, _DmaTx, _DmaRx>;
    using Spi3NoDma = Spi3<>;
#endif
}

#endif //! ZHELE_PLATFORM_STM32_H5_SPI_H
