/**
 * @file
 * Implement i2c protocol
 * 
 * @author Aleksei Zhelonkin (based on i2c implementation by Konstantin Chizhov)
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_I2C_COMMON_H
#define ZHELE_I2C_COMMON_H

#include "macro_utils/enum.h"
#include "template_utils/type_list.h"

#include <clock.h>
#include <iopins.h>
#include <pinlist.h>

#include <functional>

#if defined(I2C_ISR_BUSY)
    #define I2C_TYPE_1
#elif defined(I2C_SR2_BUSY)
    #define I2C_TYPE_2
#endif

namespace Zhele
{
    // Result of i2c class operation
    enum class I2cStatus :uint8_t
    {
        Success,	///< Success (no error)
        Overflow,	///< Buffer overflow
        Timeout,	///< Operation timeout
        BusError,	///< I2c bus error
        ArbitrationError,	///< Bus arbitration error
        ArgumentError,	///< Invalid arguments
        Nack,	///< Nack recieved
        Busy, ///< Bus is busy
    };

    // I2C state
    enum class I2cState : uint8_t
    {
        Idle = 0, ///< Idle
        Start = 1, ///< Start operation 
        DevAddr = 2, ///< Writing device address ti bus
        RegAddrNextByte = 4, // I2C unit waits for next register address
        RegAddr = 5, ///< Register address has been written
        Restart = 8, ///< Restart
        DevAddrRead = 16, ///< Device address read
        Data = 32, ///< Data
        Stop = 64, ///< Stop
    };

    /**
     * @brief I2C mode
     */
    enum class I2cMode :uint8_t
    {
        Idle = 0, ///< Idles
        Read = 1, ///< Read data
        Write = 2, ///< Write data
    };

    /**
     * @brief I2C options
     */
    enum class I2cOpts :uint8_t
    {
        None = 0, ///< None
        
        DevAddr7Bit = 0, ///< 7-bit device address
        DevAddr10Bit = 1, ///< 10-bit device address
        
        RegAddr8Bit = 0, ///< 7-bit register address
        RegAddr16Bit = 2, ///< 16-bit register address
        RegAddrNone = 3, ///< No register (read/write in device direct)
    };

    /**
     * @brief I2C events
     */
    enum Events : uint32_t
    {
        Error = (0x1UL << 31),

    #if defined(I2C_TYPE_1)
        TxEmpty = I2C_ISR_TXE,
        TxInterrupt = I2C_ISR_TXIS,
        RxNotEmpty = I2C_ISR_RXNE,
        AddressMatched = I2C_ISR_ADDR,
        AckFailure = I2C_ISR_NACKF,
        StopDetection = I2C_ISR_STOPF,
        TransfertComplete = I2C_ISR_TC,
        TransfertCompleteReload = I2C_ISR_TCR,
        BusError = I2C_ISR_BERR,
        ArbitrationLost = I2C_ISR_ARLO,
        Overrun = I2C_ISR_OVR,
        PecError = I2C_ISR_PECERR,
        Timeout = I2C_ISR_TIMEOUT,
        SMBusAlert = I2C_ISR_ALERT,
        BusBusy = I2C_ISR_BUSY,
        TransferDirection = I2C_ISR_DIR,
        AddressCode = I2C_ISR_ADDCODE
    #else
        // SR1
        StartBit = I2C_SR1_SB,
        AddressSent = I2C_SR1_ADDR,
        ByteTransferFinished = I2C_SR1_BTF,
        Address10BitSent = I2C_SR1_ADD10,
        StopDetection = I2C_SR1_STOPF,
        RxNotEmpty = I2C_SR1_RXNE,
        TxEmpty = I2C_SR1_TXE,
        BusError = I2C_SR1_BERR,
        ArbitrationLost = I2C_SR1_ARLO,
        AckFailure = I2C_SR1_AF,
        Overrun = I2C_SR1_OVR,
        PecError = I2C_SR1_PECERR,
        Timeout = I2C_SR1_TIMEOUT,
        SMBusAlert = I2C_SR1_SMBALERT,
        // SR2
        MasterSlave = I2C_SR2_MSL << 16,
        BusBusy = I2C_SR2_BUSY << 16,
        TransmitterReceiver = I2C_SR2_TRA << 16,
        GeneralCall = I2C_SR2_GENCALL << 16,
        SMBusDefaultAddress = I2C_SR2_SMBDEFAULT << 16,
        SMBusHostHeader = I2C_SR2_SMBHOST << 16,
        DualFlag = I2C_SR2_DUALF << 16
    #endif
    };
    DECLARE_ENUM_OPERATIONS(Events);
    
    struct ReadResult
    {
        uint8_t Value;        
        I2cStatus Status;        
    };

    using I2cCallback = std::add_pointer_t<void(I2cStatus status)>;

    namespace Private
    {
        /**
         * @brief Implements I2C protocol.
         * 
         * @tparam _Regs Register.
         * @tparam _EventIrqNumber Event IRQ number.
         * @tparam _ErrorIrqNumber Error IRQ number.
         * @tparam _ClockCtrl Clock.
         * @tparam _SclPins Clock pins.
         * @tparam _SdaPins Data pins.
         * @tparam _DmaTx Dma Tx
         * @tparam _DmaRx Dma Rx
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
            #if defined (I2C_TYPE_1)
                uint8_t* Buffer;
                uint16_t Size;
            #endif
                I2cCallback Callback;
            };

            static AsyncTransferData _transferData;
        public:
            using SclPins = _SclPins;
            using SdaPins = _SdaPins;
            
            /**
             * @brief Initialize I2C
             * 
             * @param [in] i2cClockSpeed I2C speed
             * @param [in] dutyCycle2 Enable duty cycle 16/9 (not for all MCU)
             * 
             * @par Returns
             * 	Nothing
             */
        #if defined (I2C_TYPE_1)
            static void Init(uint32_t i2cClockSpeed = 100000U);
        #endif
        #if defined (I2C_TYPE_2)
            static void Init(uint32_t i2cClockSpeed = 100000U, bool dutyCycle2 = false);
        #endif
            
            /**
             * @brief Write 8-bit unsigned to register.
             * 
             * @param [in] devAddr Device address.
             * @param [in] regAddr Register address.
             * @param [in] data Data to write.
             * @param [in] opts Options.
             * 
             * @returns Write status.
             */
            static I2cStatus WriteU8(uint16_t devAddr, uint16_t regAddr, uint8_t data, I2cOpts opts = I2cOpts::None);
            
            /**
             * @brief Write data to register.
             * 
             * @param [in] devAddr Device address.
             * @param [in] regAddr Register address.
             * @param [in] data Data to write.
             * @param [in] size Data size.
             * @param [in] opts Options.
             * 
             * @returns Write status.
             */
            static I2cStatus Write(uint16_t devAddr, uint16_t regAddr, const uint8_t *data, uint16_t size, I2cOpts opts = I2cOpts::None);

            /**
             * @brief Write data to register async.
             * 
             * @param [in] devAddr Device address.
             * @param [in] regAddr Register address.
             * @param [in] data Data to write.
             * @param [in] size Data size.
             * @param [in] opts Options.
             * @param [in] callback Complete (or error) callabck.
             * 
             * @returns Status.
             */
            static I2cStatus WriteAsync(uint16_t devAddr, uint16_t regAddr, const uint8_t *data, uint16_t size, I2cOpts opts = I2cOpts::None, I2cCallback callback = nullptr);

            /**
             * @brief Read 8-bit unsigned.
             * 
             * @param [in] devAddr Device address.
             * @param [in] regAddr Register address.
             * @param [in] opts Options.
             * 
             * @returns If return value is positive - it is read result (cast it to uint8_t!), if negative - it is error code.
             */
            static ReadResult ReadU8(uint16_t devAddr, uint16_t regAddr, I2cOpts opts = I2cOpts::None);

            /**
             * @brief Read some bytes.
             * 
             * @param [in] devAddr Device address.
             * @param [in] regAddr Register address.
             * @param [in] data Data buffer.
             * @param [in] size Data size.
             * @param [in] opts Options.
             * 
             * @returns Operation status.
             */
            static I2cStatus Read(uint16_t devAddr, uint16_t regAddr, uint8_t *data, uint16_t size, I2cOpts opts = I2cOpts::None);

            /**
             * @brief Read some bytes async.
             * 
             * @param [in] devAddr Device address.
             * @param [in] regAddr Register address.
             * @param [out] data Output buffer.
             * @param [in] size Data size to read.
             * @param [in] opts Options.
             * @param [in] callback Complete (or error) callback.
             * 
             * @returns Operation status.
             */
            static I2cStatus EnableAsyncRead(uint16_t devAddr, uint16_t regAddr, uint8_t *data, uint16_t size, I2cOpts opts = I2cOpts::None, I2cCallback callback = nullptr);

            /**
             * @brief Write register address.
             * 
             * @param [in] regAddr Register address.
             * @param [in] opts Options
             * 
             * @retval true Success.
             * @retval false Fail.
             */
            static bool WriteRegAddr(uint16_t regAddr, I2cOpts opts);
            
            /**
             * @brief Wait for events.
             * 
             * @param [in] Events mask.
             * 
             * @retval true Events was occured.
             * @retval false Events was not occured.
             */
            static bool WaitEvent(uint32_t i2c_event);
            
            /**
             * @brief Is bus busy.
             * 
             * @retval true Bus is busy.
             * @retval false Bus is free.
             */
            static bool Busy();
            
            /**
             * @brief Wait while bus busy.
             * 
             * @retval true Bus is free.
             * @retval false Timeout.
             */
            static bool WaitWhileBusy();
            
            /**
             * @brief Event IRQ handler
             * 
             * @par Returns
             *  Nothing
             */
            static void EventIrqHandler();
            
            /**
             * @brief Error IRQ handler
             * 
             * @par Returns
             *  Nothing
             */
            static void ErrorIrqHandler();
            
            /**
             * @brief Returns error from event.
             * 
             * @returns Status from event.
             */
            static I2cStatus GetErorFromEvent(uint32_t lastevent);
            
            /**
             * @brief Select pins
             * 
             * @tparam SclPinNumber SCL pin number
             * @tparam SdaPinNumber SDA pin number
             * 
             * @par Returns
             *  Nothing
             */
            template<unsigned sclPinNumber, unsigned sdaPinNumber>
            static void SelectPins();

            /**
             * @brief Select pins.
             * 
             * @param [in] sclPinNumber SCL pin number
             * @param [in] sdaPinNumber SDA pin number
             * 
             * @par Returns
             *  Nothing
             */
            static void SelectPins(uint8_t sclPinNumber, uint8_t sdaPinNumber);
            
            /**
             * @brief Select pins
             * 
             * @tparam SclPin SCL pin
             * @tparam SdaPin SDA pin
             * 
             * @par Returns
             *  Nothing
             */
            template<typename SclPin, typename SdaPin>
            static void SelectPins();

        private:
            #if defined(I2C_TYPE_1)
            /**
             * @brief Write device address for write operation.
             * 
             * @param [in] devAddr Device address.
             * @param [in] opts Options
             * 
             * @retval true Success.
             * @retval false Fail.
             */
            static bool WriteDevAddrForWrite(uint16_t devAddr, I2cOpts opts);
            
            /**
             * @brief Write device address for read operation.
             * 
             * @param [in] devAddr Device address.
             * @param [in] opts Options.
             * @param [in] bytesToRead Nbytes value.
             * @param [in] reload Need reload (if need read > 255 bytes)
             * 
             * @retval true Success.
             * @retval false Fail.
             */
            static bool WriteDevAddrForRead(uint16_t devAddr, I2cOpts opts, uint8_t bytesToRead = 1, bool reload = false);

            /**
             * @brief Set transfer size (only for MCUs with NBYTES in CR2)
             * 
             * @param [in] size Nbytes 
             * @param [in] isLast Is this transfer last
             */
            static void SetTransferSize(uint8_t size, bool isLast = true);
            #endif

            #if defined (I2C_TYPE_2)
            /**
             * @brief Write device address for write operation.
             * 
             * @param [in] devAddr Device address.
             * @param [in] read Is read
             * @param [in] opts Options
             * 
             * @retval true Success.
             * @retval false Fail.
             */        
            static bool WriteDevAddr(uint16_t devAddr, bool read, I2cOpts opts);

            /**
             * @brief Start transfer.
             * 
             * @retval true Success.
             * @retval false Fail.
             */
            static bool Start();
            #endif

            /**
             * @brief Returns last event (SR register value)
             * 
             * @returns Event bitmask
             */
            static uint32_t GetLastEvent();
        };
    }
}

#include "impl/i2c.h"

#endif //! ZHELE_I2C_COMMON_H