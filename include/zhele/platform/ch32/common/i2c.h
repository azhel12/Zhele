/**
 * @file
 * I2C master for CH32 (WCH I2C peripheral)
 * @author Aleksei Zhelonkin (based on the STM32 i2c implementation)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_I2C_H
#define ZHELE_PLATFORM_CH32_COMMON_I2C_H

#include <zhele/common/template_utils/enum.h>

#include <zhele/clock.h>
#include <zhele/dma.h>
#include <zhele/iopins.h>
#include <zhele/pinlist.h>

#include <type_traits>

namespace Zhele
{
    /// Result of an I2C operation.
    enum class I2cStatus : uint8_t
    {
        Success,          ///< Success (no error)
        Overflow,         ///< Buffer overflow
        Timeout,          ///< Operation timeout
        BusError,         ///< I2C bus error
        ArbitrationError, ///< Bus arbitration error
        ArgumentError,    ///< Invalid arguments
        Nack,             ///< NACK received
        Busy,             ///< Bus is busy
    };

    /// I2C transfer options.
    enum class I2cOpts : uint8_t
    {
        None = 0,           ///< None

        DevAddr7Bit  = 0,   ///< 7-bit device address
        DevAddr10Bit = 1,   ///< 10-bit device address

        RegAddr8Bit  = 0,   ///< 8-bit register address
        RegAddr16Bit = 2,   ///< 16-bit register address
        RegAddrNone  = 3,   ///< No register address (talk to device directly)
    };

    /// I2C status-register events (STAR1 in the low 16 bits, STAR2 in the high 16 bits).
    enum Events : uint32_t
    {
        // STAR1
        StartBit             = I2C_STAR1_SB,
        AddressSent          = I2C_STAR1_ADDR,
        ByteTransferFinished = I2C_STAR1_BTF,
        Address10BitSent     = I2C_STAR1_ADD10,
        StopDetection        = I2C_STAR1_STOPF,
        RxNotEmpty           = I2C_STAR1_RXNE,
        TxEmpty              = I2C_STAR1_TXE,
        BusError             = I2C_STAR1_BERR,
        ArbitrationLost      = I2C_STAR1_ARLO,
        AckFailure           = I2C_STAR1_AF,
        Overrun              = I2C_STAR1_OVR,
        // STAR2
        MasterSlave          = I2C_STAR2_MSL << 16,
        BusBusy              = I2C_STAR2_BUSY << 16,
        TransmitterReceiver  = I2C_STAR2_TRA << 16,
        GeneralCall          = I2C_STAR2_GENCALL << 16,
        DualFlag             = I2C_STAR2_DUALF << 16,
    };

    struct ReadResult
    {
        uint8_t Value;
        I2cStatus Status;
    };

    using I2cCallback = std::add_pointer_t<void(I2cStatus status)>;

    namespace Private
    {
        /**
         * @brief Implements the I2C master protocol.
         *
         * @tparam _Regs Peripheral register wrapper
         * @tparam _EventIrqNumber Event IRQ number
         * @tparam _ErrorIrqNumber Error IRQ number
         * @tparam _ClockCtrl Clock control class
         * @tparam _SclPins SCL candidate pin list
         * @tparam _SdaPins SDA candidate pin list
         * @tparam _DmaTx TX DMA channel
         * @tparam _DmaRx RX DMA channel
         */
        template<
            typename _Regs,
            IRQn_Type _EventIrqNumber,
            IRQn_Type _ErrorIrqNumber,
            typename _ClockCtrl,
            typename _SclPins,
            typename _SdaPins,
            typename _DmaTx,
            typename _DmaRx>
        class I2cBase
        {
            static const uint16_t _timeout = 10000;

            struct AsyncTransferData
            {
                I2cCallback Callback;
            };

            static AsyncTransferData _transferData;

        public:
            using SclPins = _SclPins;
            using SdaPins = _SdaPins;
            using DmaTx = _DmaTx;
            using DmaRx = _DmaRx;

            /**
             * @brief Initialize I2C in master mode.
             *
             * @param [in] i2cClockSpeed Bus speed, Hz
             * @param [in] dutyCycle2 Use 16/9 duty cycle in fast mode (else 2/1)
             */
            static void Init(uint32_t i2cClockSpeed = 100000U, bool dutyCycle2 = false);

            /**
             * @brief Write a single byte to a register.
             */
            static I2cStatus WriteU8(uint16_t devAddr, uint16_t regAddr, uint8_t data, I2cOpts opts = I2cOpts::None);

            /**
             * @brief Write a buffer to a register (blocking).
             */
            static I2cStatus Write(uint16_t devAddr, uint16_t regAddr, const uint8_t* data, uint16_t size, I2cOpts opts = I2cOpts::None);

            /**
             * @brief Write a buffer to a register via DMA (non-blocking).
             *
             * @note The TX DMA channel IRQ must route to _DmaTx::IrqHandler() so
             *       the STOP condition and callback fire on completion.
             */
            static I2cStatus WriteAsync(uint16_t devAddr, uint16_t regAddr, const uint8_t* data, uint16_t size, I2cOpts opts = I2cOpts::None, I2cCallback callback = nullptr);

            /**
             * @brief Read a single byte from a register.
             */
            static ReadResult ReadU8(uint16_t devAddr, uint16_t regAddr, I2cOpts opts = I2cOpts::None);

            /**
             * @brief Read a buffer from a register (blocking).
             */
            static I2cStatus Read(uint16_t devAddr, uint16_t regAddr, uint8_t* data, uint16_t size, I2cOpts opts = I2cOpts::None);

            /**
             * @brief Read a buffer from a register via DMA (non-blocking).
             *
             * @note The RX DMA channel IRQ must route to _DmaRx::IrqHandler().
             */
            static I2cStatus EnableAsyncRead(uint16_t devAddr, uint16_t regAddr, uint8_t* data, uint16_t size, I2cOpts opts = I2cOpts::None, I2cCallback callback = nullptr);

            /**
             * @brief Write the register address (8- or 16-bit).
             */
            static bool WriteRegAddr(uint16_t regAddr, I2cOpts opts);

            static bool Busy();

            static bool WaitWhileBusy();

            static bool WaitEvent(uint32_t i2cEvent);

            static I2cStatus GetErorFromEvent(uint32_t lastEvent);

            /**
             * @brief Configure the SCL/SDA pins (alternate function, open-drain) and remap.
             */
            template<typename SclPin, typename SdaPin>
            static void SelectPins()
            {
                SclPin::Port::Enable();
                SclPin::template SetConfiguration<SclPin::Port::Configuration::AltFunc>();
                SclPin::template SetDriverType<SclPin::Port::DriverType::OpenDrain>();

                if constexpr (!std::is_same_v<typename SdaPin::Port, typename SclPin::Port>)
                    SdaPin::Port::Enable();
                SdaPin::template SetConfiguration<SdaPin::Port::Configuration::AltFunc>();
                SdaPin::template SetDriverType<SdaPin::Port::DriverType::OpenDrain>();

                // Apply I2C1 remap for the chosen SCL pin (PD1/PD0 or PC5/PC6; default PC2/PC1).
#if defined(AFIO_PCFR1_I2C1_HIGH_BIT_REMAP)
                constexpr char sclPort = static_cast<char>(SclPin::Port::Id);
                constexpr unsigned sclNum = SclPin::Number;
                if constexpr (sclPort == 'D')
                {
                    Clock::Apb2PeriphClockEnable::Or(RCC_AFIOEN);
                    AFIO->PCFR1 |= AFIO_PCFR1_I2C1_REMAP;
                }
                else if constexpr (sclPort == 'C' && sclNum == 5)
                {
                    Clock::Apb2PeriphClockEnable::Or(RCC_AFIOEN);
                    AFIO->PCFR1 |= AFIO_PCFR1_I2C1_HIGH_BIT_REMAP;
                }
#endif
            }

        private:
            static bool Start();

            static bool WriteDevAddr(uint16_t devAddr, bool read, I2cOpts opts);

            static uint32_t GetLastEvent();
        };
    }
}

#include "impl/i2c.h"

#endif // ZHELE_PLATFORM_CH32_COMMON_I2C_H
