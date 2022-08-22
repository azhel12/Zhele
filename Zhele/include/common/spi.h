/**
 * @file
 * Implements SPI protocol.
 * @author Konstantin Chizhov
 * @date 2012
 * @license FreeBSD
 */

#ifndef ZHELE_SPI_COMMON_H
#define ZHELE_SPI_COMMON_H

#include "ioreg.h"
#include "template_utils/data_transfer.h"

#include <clock.h>
#include <iopins.h>
#include <pinlist.h>

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
             * @brief Defines divider for spi clock.
             * 
             * @details
             * SPI clock is equal to APB1 / divider.
             * STM32 has 3 bits in CR1 register (bits 7-9) to
             * write this value. So, spi module clock frequence
             * is equal to ARB1 / 2^(divider + 1)
             * 
             */
            enum ClockDivider
            {
                Div2	= 0, ///< Divide to 2
                Div4	= 1 << SPI_CR1_BR_Pos, ///< Divide to 4
                Div8	= 2 << SPI_CR1_BR_Pos, ///< Divide to 8
                Div16	= 3 << SPI_CR1_BR_Pos, ///< Divide to 16
                Div32	= 4 << SPI_CR1_BR_Pos, ///< Divide to 32
                Div64	= 5 << SPI_CR1_BR_Pos, ///< Divide to 64
                Div128	= 6 << SPI_CR1_BR_Pos, ///< Divide to 128
                Div256	= 7 << SPI_CR1_BR_Pos, ///< Divide to 256
                Fastest = Div2,
                Fast	= Div8,
                Medium	= Div32,
                Slow	= Div128,
                Slowest = Div256
            };

            /**
             * @brief Defines slave control (auto or manual)
             * 
             * @details
             * STM32 SPI module has NSS (SS, CS) pin.
             * In main, it is useless feature, this pin
             * can be used, when you have only one slave device
             * or if your device can be slave.
             * 
             * See articles about SPI and SSM/SSI control bits
             */
            enum SlaveControl
            {
                SoftSlaveControl	= SPI_CR1_SSM, ///< SS soft (manual) control
                AutoSlaveControl	= 0 ///< SS auto (device) control
            };

            /**
             * @brief SPI unit mode
             */
            enum Mode
            {
                Master				= SPI_CR1_MSTR | (SPI_CR2_SSOE << 16),	 ///< Master
                MultiMaster			= SPI_CR1_MSTR,	///< Multi master
                Slave				= 0	///< Slave
            };

            /**
             * @brief Data size for data transfer
             * 
             * @details
             * STM32 MCUs start with f3 series support data size settings by CR2_DS bits, which
             * allows to set any size of data between 2 and 16 bits.
             * More young platform (such as f103) has only one bit in CR1 register - DFF, which
             * defines size of data. If SPI_CR1_DFF = 0 then data size is 8bit, if = 1 then 16 bit.
             * 
             */
            enum DataSize : uint16_t
            {
            #if defined (SPI_CR1_DFF)
                DataSize8 = 0,
                DataSize16 = SPI_CR1_DFF
            #else
                DataSize4	= 0x03 << SPI_CR2_DS_Pos,
                DataSize5	= 0x04 << SPI_CR2_DS_Pos,
                DataSize6	= 0x05 << SPI_CR2_DS_Pos,
                DataSize7	= 0x06 << SPI_CR2_DS_Pos,
                DataSize8	= 0x07 << SPI_CR2_DS_Pos,
                DataSize9	= 0x08 << SPI_CR2_DS_Pos,
                DataSize10	= 0x09 << SPI_CR2_DS_Pos,
                DataSize11	= 0x0A << SPI_CR2_DS_Pos,
                DataSize12	= 0x0B << SPI_CR2_DS_Pos,
                DataSize13	= 0x0C << SPI_CR2_DS_Pos,
                DataSize14	= 0x0D << SPI_CR2_DS_Pos,
                DataSize15	= 0x0E << SPI_CR2_DS_Pos,
                DataSize16	= 0x0F << SPI_CR2_DS_Pos
            #endif
            };
            
            /**
             * @brief Clock polarity (CPOL)
             */
            enum ClockPolarity
            {
                ClockPolarityLow	= 0, ///< Low
                ClockPolarityHigh	= SPI_CR1_CPOL ///< High
            };
            
            /**
             * @brief Clock phase (CPHA)
             */
            enum ClockPhase
            {
                ClockPhaseLeadingEdge = 0, ///< Leading
                ClockPhaseFallingEdge	 = SPI_CR1_CPHA ///<Falling
            };
            
            /**
             * @brief Bit order
             */
            enum BitOrder
            {
                LsbFirst			= SPI_CR1_LSBFIRST, ///< LSB
                MsbFirst			= 0 ///< MSB
            };
        };
        

        /**
         * @brief Implements spi unit
         * 
         * @tparam _Regs Spi registers
         * @tparam _Clock Spi source clock class
         * @tparam _MosiPins Pinlist of MOSI pins
         * @tparam _MisoPins Pinlist of MISO pins
         * @tparam _ClockPins Pinlist of CLK pins
         * @tparam _SsPins Pinlist of SS pins
         */
        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        class Spi : public SpiBase
        {
        public:
            /**
             * @brief Enable SPI
             * 
             * @par Returns
             * 	Nothing
             */
            static void Enable();
          
            /**
             * @brief Disable SPI
             * 
             * @par Returns
             * 	Nothing
             */
            static void Disable();
            
            
            /**
             * @brief Init SPI interface
             * 
             * @param [in] divider Clock divider
             * @param [in] mode SPI mode
             * 
             * @par Returns
             * 	Nothing
             */
            static void Init(ClockDivider divider = Medium, Mode mode = Master);
           

            /**
             * @brief Set SPI clock divider
             * 
             * @param [in] divider Clock divider
             * 
             * @par Returns
             *	Nothing
             */
            static void SetDivider(ClockDivider divider);
           
            /**
             * @brief Set SPI clock polarity (CPOL)
             * 
             * @param [in] clockPolarity Polarity
             * 
             * @par Returns
             * 	Nothing
             */
            static void SetClockPolarity(ClockPolarity clockPolarity);
          
            
            /**
             * @brief Set SPI clock phase (CPHA)
             * 
             * @param [in] clockPhase Clock phase
             * 
             * @par Returns
             * 	Nothing
             */
            static void SetClockPhase(ClockPhase clockPhase);
           
            
            /**
             * @brief Set SPI bit order (LSB/MSB)
             * 
             * @param [in] bitOrder Bit order
             * 
             * @par Returns
             * 	Nothing
             */
            static void SetBitOrder(BitOrder bitOrder);
            
            
            /**
             * @brief Set SPI data size
             * 
             * @param [in] dataSize Data size
             * 
             * @note For some MCU (f3 series) you can choose any value (4..16), for other only 8 or 16
             * 
             * @par Returns
             * 	Nothing
             */
            static void SetDataSize(DataSize dataSize);
          
            /**
             * @brief Set slave control (NSS pin)
             * 
             * @param [in] slaveControl Slave control
             * 
             * @par Returns
             * 	Nothing
             */
            static void SetSlaveControl(SlaveControl slaveControl);
           
            /**
             * @brief Set slave select (set NSS pin)
             * 
             * @par Returns
             * 	Nothing
             */
            static void SetSS();
          
            /**
             * @brief Unset slave select (clear NSS pin)
             * 
             * @par Returns
             * 	Nothing
             */
            static void ClearSS();

            /**
             * @brief Returns busy state
             * 
             * @return true SPI is busy
             * @return false SPI is not busy
             */
            static bool Busy();
            
            /**
             * @brief Send and receive data
             * 
             * @param [in] outValue Data to send
             * 
             * @returns Received value
             */
            static uint16_t Send(uint16_t value);
           
            /**
             * @brief Send data async (by DMA)
             * 
             * @param [out] transmitBuffer Data to transmit
             * @param [out] receiveBuffer Output buffer
             * @param [in] bufferSize Data size
             * @param [in, opt] callback Transfer complete callback (optional parameter)
             * 
             * @par Returns
             *  Nothing
             */
            
            static void SendAsync(void* transmitBuffer, void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr);
            /**
             * @brief Send data with ignored receive
             * 
             * @param [in] data Data to send
             * 
             * @par Returns
             * 	Nothing
             */
            static void Write(uint16_t data);
            
    
            /**
             * @brief Send data async (by DMA) with ignored receive.
             * 
             * @param [in] data Data buffer
             * @param [in] size Buffer size (count of elements)
             * 
             * @par Returns
             * 	Nothing
             */
            static void WriteAsync(const void* data, uint16_t size, TransferCallback callback = nullptr);

            /**
             * @brief Send data async (by DMA) with ignored receive.
             * 
             * @param [in] data Data buffer
             * @param [in] size Buffer size (count of elements)
             * 
             * @par Returns
             * 	Nothing
             */
            static void WriteAsyncNoIncrement(const void* data, uint16_t size, TransferCallback callback = nullptr);

            /**
             * @brief Read data (via send 0xFF dummy value)
             * 
             * @returns Readed value
             */
            static uint16_t Read();
            
            
            /**
             * @brief Enable async read (by DMA)
             * 
             * @param [out] data Output buffer
             * @param [in] size Size to read
             * @param [in, opt] callback Transfer complete callback (optional parameter)
             * 
             * @par Returns
             *  Nothing
             */
            static void ReadAsync(void* receiveBuffer, size_t bufferSize, TransferCallback callback = nullptr);
         

            /**
             * @brief Select pins
             * 
             * @param [in] mosiPinNumber MOSI pin number
             * @param [in] misoPinNumber MISO pin number
             * @param [in] clockPinNumber CLK pin number
             * @param [in] ssPinNumber NSS pin number
             * 
             * @par Returns
             * 	Nothing
             */
            static void SelectPins(int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber);

            /**
             * @brief Select pins
             * 
             * @tparam mosiPinNumber MOSI pin number
             * @tparam misoPinNumber MISO pin number
             * @tparam clockPinNumber CLK pin number
             * @tparam ssPinNumber NSS pin number
             * 
             * @par Returns
             * 	Nothing
             */
            template<int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber>
            static void SelectPins();

            /**
             * @brief Select pins
             * 
             * @tparam mosiPin MOSI pin
             * @tparam misoPin MISO pin
             * @tparam clockPin CLK pin
             * @tparam ssPin NSS pin
             * 
             * @par Returns
             * 	Nothing
             */
            template<typename mosiPin, typename misoPin, typename clockPin, typename ssPin>
            static void SelectPins();
        };
    }
}

#include "impl/spi.h"

#endif //!ZHELE_SPI_COMMON_H