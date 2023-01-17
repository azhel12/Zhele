/**
 * @file
 * Implement UART protocol
 * 
 * @author Konstantin Chizhov
 * @date ??
 * @license FreeBSD
 */

#ifndef ZHELE_UART_COMMON_H
#define ZHELE_UART_COMMON_H

#include "clock.h"
#include "dma.h"
#include "iopins.h"
#include "ioreg.h"
#include "pinlist.h"

#include "./template_utils/data_transfer.h"

namespace Zhele
{
#if(USART_ISR_PE)
    #define USART_TYPE_1
    #define STATUS_REG ISR
    #define TRANSMIT_DATA_REG TDR
    #define RECEIVE_DATA_REG RDR
#endif
#if(USART_SR_PE)
    #define USART_TYPE_2
    #define STATUS_REG SR
    #define TRANSMIT_DATA_REG DR
    #define RECEIVE_DATA_REG DR
#endif
    class UsartBase
    {
    public:
        struct UsartMode
        {
            /**
             * @brief CR1 
             */
            enum _CR1 : uint32_t
            {
                DataBits8 = 0,
                DataBits9 = USART_CR1_M,

                NoneParity = 0,
                EvenParity = USART_CR1_PCE,
                OddParity  = USART_CR1_PS | USART_CR1_PCE,

                Disabled = 0,
                RxEnable = USART_CR1_RE,
                TxEnable = USART_CR1_TE,
                RxTxEnable  = USART_CR1_RE | USART_CR1_TE,
                Default = RxTxEnable,
            } CR1;

            enum _CR2 : uint32_t
            {
                NoClock = 0,
                Clock = USART_CR2_CLKEN,

                OneStopBit         = 0,
                HalfStopBit        = USART_CR2_STOP_0,
                TwoStopBits        = USART_CR2_STOP_1,
                OneAndHalfStopBits = (USART_CR2_STOP_0 | USART_CR2_STOP_1)
            } CR2;

            enum _CR3 : uint32_t
            {
                FullDuplex = 0,
                HalfDuplex = USART_CR3_HDSEL,
            
                OneSampleBitDisable = 0,
                OneSampleBitEnable =
                #if defined (USART_CR3_ONEBIT)
                    USART_CR3_ONEBIT
                #else
                    0
                #endif
            } CR3;

            UsartMode operator | (UsartMode::_CR1 flag)
            {
                return UsartMode{static_cast<UsartMode::_CR1>(CR1 | flag), CR2, CR3};
            }

            UsartMode operator | (UsartMode::_CR2 flag)
            {
                return UsartMode{CR1, static_cast<UsartMode::_CR2>(CR2 | flag), CR3};
            }

            UsartMode operator | (UsartMode::_CR3 flag)
            {
                return UsartMode{CR1, CR2, static_cast<UsartMode::_CR3>(CR3 | flag)};
            }
        };

        /**
         * @brief All possible interrupts
         */
        enum InterruptFlags
        {
            NoInterrupt = 0,
        #if defined (USART_SR_PE)
            ParityErrorInt = USART_SR_PE,	///< Parity error
            TxEmptyInt     = USART_SR_TXE,	///< Transmission register empty
            TxCompleteInt  = USART_SR_TC,	///< Transmission complete
            RxNotEmptyInt  = USART_SR_RXNE,	///< Read data register not empty
            IdleInt        = USART_SR_IDLE,	///< Idle mode (all operations finished)

            LineBreakInt   = USART_SR_LBD,

            ErrorInt       = USART_SR_FE | USART_SR_NE | USART_SR_ORE,	
            CtsInt         = USART_SR_CTS,
        #endif
        #if defined (USART_ISR_PE)
            ParityErrorInt = USART_ISR_PE,	///< Parity error
            TxEmptyInt     = USART_ISR_TXE,	///< Transmission register empty
            TxCompleteInt  = USART_ISR_TC,	///< Transmission complete
            RxNotEmptyInt  = USART_ISR_RXNE,	///< Read data register not empty
            IdleInt        = USART_ISR_IDLE,	///< Idle mode (all operations finished)
            #if defined(USART_ISR_LBD)
            LineBreakInt   = USART_ISR_LBD,
            #else
            LineBreakInt   = 0,
            #endif
            ErrorInt       = USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE,	
            CtsInt         = USART_ISR_CTS,
        #endif
            AllInterrupts  = ParityErrorInt | TxEmptyInt | TxCompleteInt | RxNotEmptyInt | IdleInt | LineBreakInt | ErrorInt | CtsInt
        };

        enum Error
        {
            NoError = 0,
        #if defined (USART_TYPE_1)
            OverrunError = USART_ISR_ORE,
            NoiseError = USART_ISR_NE,
            FramingError = USART_ISR_FE,
            ParityError = USART_ISR_PE
        #endif
        #if defined (USART_TYPE_2)
            OverrunError = USART_SR_ORE,
            NoiseError = USART_SR_NE,
            FramingError = USART_SR_FE,
            ParityError = USART_SR_PE
        #endif
        };

    protected:
        static const unsigned ErrorMask = OverrunError | NoiseError | FramingError | ParityError;

        static const unsigned InterruptMask = ParityErrorInt | TxEmptyInt |
                TxCompleteInt | RxNotEmptyInt | IdleInt | LineBreakInt |
                ErrorInt | CtsInt;
    };
    
    static const UsartBase::UsartMode DefaultUsartMode = {
        .CR1 = static_cast<UsartBase::UsartMode::_CR1>(UsartBase::UsartMode::RxTxEnable),
        .CR2 = static_cast<UsartBase::UsartMode::_CR2>(0),
        .CR3 = static_cast<UsartBase::UsartMode::_CR3>(0)};

    namespace Private
    {
        template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl, typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        class Usart : public UsartBase
        {
        public:
            /**
             * @brief Initialize USART
             * 
             * @tparam baud Baud rate
             * @param [in] mode Mode
             * 
             * @par Returns
             *	Nothing
             */
            template<unsigned long baud>
            static inline void Init(UsartMode mode = DefaultUsartMode);
            

            /**
             * @brief Initialize USART
             * 
             * @param [in] baud Baud rate
             * @param[in] mode Mode
             * 
             * @par Returns
             *	Nothing
             */
            static void Init(unsigned baud, UsartMode mode = DefaultUsartMode);
            

            /**
             * @brief Set config
             * 
             * @param [in] modeMask Mode mask
             */
            static void SetConfig(UsartMode modeMask);
           

            /**
             * @brief Clear config
             * 
             * @param [in] modeMask Mode mask
             */
            static void ClearConfig(UsartMode modeMask);
            

            /**
             * @brief Set baud rate
             * 
             * @param [in] baud Baud rate
             * 
             * @par Returns
             *  Nothing
             */
            static void SetBaud(unsigned baud);           

            /**
             * @brief Check that USART ready to read
             * 
             * @retval true USART ready for read
             * @retval false USART is not ready for read
             */
            static bool ReadReady();

            /**
             * @brief Synch read data
             * 
             * @returns Readed byte
             */
            static uint8_t Read();

            /**
             * @brief Enable async read (by DMA)
             * 
             * @param [out] receiveBuffer Output buffer
             * @param [in] bufferSize Output buffer size
             * @param [in] callback Transfer complete callback (optional parameter)
             * 
             * @par Returns
             * 	Nothing
             */
            static void EnableAsyncRead(void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr);
           

            /**
             * @brief Check that USART ready to write
             * 
             * @retval true USART ready for write
             * @retval false USART is not ready for write
             */
            static bool WriteReady();
         
            /**
             * @brief Write data to USART
             * 
             * @param [in] data Data to write
             * @param [in] size Data size
             * @param [in] async Write async or no
             * 
             * @par Returns
             * 	Nothing
             */
            [[deprecated("Replaced by Write/WriteAsync methods")]]
            static void Write(const void* data, size_t size, bool async);

            /**
             * @brief Write data to USART
             * 
             * @param [in] data Data to write
             * @param [in] size Data size
             * 
             * @par Returns
             * 	Nothing
             */
            static void Write(const void* data, size_t size);
            
            /**
             * @brief Write data to USART async (via DMA)
             * 
             * @param [in] data Data to write
             * @param [in] size Data size
             * @param [in] callback Transfer complete callback
             * 
             * @par Returns
             * 	Nothing
             */
            static void WriteAsync(const void* data, size_t size, TransferCallback callback = nullptr);

            /**
             * @brief Synch write byte
             * 
             * @param [in] data Byte to write
             * 
             * @par Returns
             *	Nothing
             */
            static void Write(uint8_t data);
           
            /**
             * @brief Enables one or more interrupts
             * 
             * @param [in] interruptFlags Interrupt flags (supports OR ("||") logic operation)
             * 
             * @par Returns
             *	Nothing
             */
            static void EnableInterrupt(InterruptFlags interruptFlags);
            

            /**
             * @brief Disables one or more interrupts
             * 
             * @param [in] interruptFlags Interrupt flags (supports OR ("||") logic operation)
             * 
             * @par Returns
             *	Nothing
             */
            static void DisableInterrupt(InterruptFlags interruptFlags);
            

            /**
             * @brief Returns caused interrupts
             * 
             * @returns Mask of interrupts
             */
            static InterruptFlags InterruptSource();
           

            /**
             * @brief Returns caused errors
             * 
             * @returns Mask of errors
             */
            static Error GetError();
           

            /**
             * @brief Clears interrupts
             * 
             * @param [in] interruptFlags Interrupts mask
             * 
             * @par Returns
             *	Nothing
             */
            static void ClearInterruptFlag(InterruptFlags interruptFlags);
           
            /**
             * @brief Select RX and TX pins (set settings)
             * 
             * @param [in] txPinNumber pin number in Txs PinList
             * @param [in] rxPinNumber pin number in Rxs PinList
             * 
             * @par Returns
             *	Nothing
             */
            static void SelectTxRxPins(int8_t txPinNumber, int8_t rxPinNumber = -1);

            /**
             * @brief Template clone of SelectTxRxPins method
             * 
             * @tparam TxPinNumber pin number in Txs PinList
             * @tparam RxPinNumber pin number in Rxs PinList
             * 
             * @par Returns
             *	Nothing
             */
            template<int8_t TxPinNumber, int8_t RxPinNumber = -1>
            static void SelectTxRxPins();

            /**
             * @brief Template clone of SelectTxRxPins method (params are TPin instances)
             * 
             * @tparam TxPin TxPin
             * @tparam RxPin RxPin
             * 
             * @par Returns
             *	Nothing
             */
            template<typename TxPin, typename RxPin = typename IO::NullPin>
            static void SelectTxRxPins();
        };
    }
}

#include "impl/usart.h"

#endif