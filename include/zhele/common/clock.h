/**
 * @file
 * Containing template classes, which implements MCU clock.
 * 
 * @author Konstantin Chizhov
 * @date 2018
 * @license FreeBSD
 */

#ifndef ZHELE_CLOCK_COMMON_H
#define ZHELE_CLOCK_COMMON_H

#include "ioreg.h"

#include <zhele/flash.h>

#include <stdint.h>

namespace Zhele
{
    using ClockFrequenceT = uint32_t;
    namespace Clock
    {
        IO_REG_WRAPPER(RCC->CR, RccCrReg, uint32_t);
        IO_REG_WRAPPER(RCC->CSR, RccCsrReg, uint32_t);
        IO_REG_WRAPPER(RCC->BDCR, RccBdcrReg, uint32_t);

        /**
         * @brief Base class for clock sources (Hsi, Hse, Pll, Lsi)
         */
        template <typename _Regs = RccCrReg>
        class ClockBase
        {
        protected:
            static const uint32_t ClockStartTimeout = 4000;

            /**
             * @brief Enable clocking
             * 
             * @param [in] turnMask Mask
             * @param [in] waitReadyMask Wait for ready mask
             */
            static bool EnableClockSource(unsigned turnMask,  unsigned waitReadyMask);
        
            /**
             * @brief Disable clocking
             * 
             * @param [in] turnMask Mask
             * @param [in] waitReadyMask Wait for ready mask
             */
            static bool DisableClockSource(unsigned turnMask,  unsigned waitReadyMask);
           
        };

        /**
         * @brief Implements external clock source
         */
        class HseClock : public ClockBase<>
        {
            using Base = ClockBase<>;
        public:
            /**
             * @brief Returns external oscillator clock frequence
             * 
             * @returns External source (HSE) clock frequence
             */
            static constexpr ClockFrequenceT SrcClockFreq();

            /**
             * @brief Returns HSE divider
             * 
             * @returns Hse divider
             */
            static constexpr unsigned GetDivider();

            /**
             * @brief Returns HSE multiplier
             * 
             * @returns Hse multiplier
             */
            static constexpr unsigned GetMultipler();

            /**
             * @brief Returns final clock frequence
             * 
             * @deail
             * Final clock frequece = source clock frequence / divider * multiplier
             * 
             * @returns Final frequence
             */
            static constexpr ClockFrequenceT ClockFreq();

            /**
             * @brief Enables Hse source
             * 
             * @retval true Successful enable
             * @retval false Fail enable
             */
            static bool Enable();

            /**
             * @brief Disables Hse source
             * 
             * @retval true Successful disable
             * @retval false Fail disable
             */
            static bool Disable();
        };

        /**
         * @brief Implements internal clock source
         */
        class HsiClock : public ClockBase<>
        {
        public:
            /**
             * @brief Returns internal oscillator clock frequence
             * 
             * @returns External source (HSE) clock frequence
             */
            static constexpr ClockFrequenceT SrcClockFreq();

            /**
             * @brief Returns HSI divider
             * 
             * @returns Hsi divider
             */
            static constexpr unsigned GetDivider();

            /**
             * @brief Returns HSI multiplier
             * 
             * @returns Hsi multiplier
             */
            static constexpr unsigned GetMultipler();

            /**
             * @brief Returns final clock frequence
             * 
             * @details
             * Final clock frequece = source clock frequence / divider * multiplier
             * 
             * @returns Final frequence
             */
            static constexpr ClockFrequenceT ClockFreq();

            /**
             * @brief Enables Hsi source
             * 
             * @retval true Successful enable
             * @retval false Fail enable
             */
            static bool Enable();

            /**
             * @brief Disables Hsi source
             * 
             * @retval true Successful disable
             * @retval false Fail disable
             */
            static bool Disable();
        };

        /**
         * @brief Implements Pll clock source
         */
        class PllClock : public ClockBase<>
        {
        public:
            /**
             * @brief Clock source for PLL
             */
            enum ClockSource
            {
                Internal,   ///< Use internal oscillator (HSI) divided 2
                External	///< Use external oscillator (HSE)
            };

            /**
             * @brief Returns PLL source clock frequence (HSI or HSE)
             * 
             * @returns PLL source frequence
             */
            static ClockFrequenceT SrcClockFreq();
           
            /**
             * @brief Returns PLL divider
             * 
             * @returns Pll divider
             */
            static unsigned GetDivider();

            /**
             * @brief Set PLL divider
             *
             * @param divider Pll divider value
             * 
             * @par Returns
             *	Nothing
             */
            template<unsigned divider>
            static void SetDivider();

            /**
             * @brief Returns PLL multiplier
             * 
             * @returns Pll multiplier
             */
            static unsigned GetMultipler();

            /**
             * @brief Set PLL multiplier
             * 
             * @tparam multiplier Pll multiplier value
             * 
             * @par Returns
             *	Nothing
             */
            template<unsigned multiplier>
            static void SetMultiplier();

            /**
             * @brief Return PLLP value
             * 
             * @returns PLLP value
             */
            static unsigned GetSystemOutputDivider();

            /**
             * @brief Set PLLP
             * 
             * @tparam divider Divider for sysclock output
             * 
             * @par Returns
             *	Nothing
             */
            template<unsigned divider>
            static void SetSystemOutputDivider();

            /**
             * @brief Return USB divider value
             * 
             * @returns USB divider value
             */
            static unsigned GetUsbOutputDivider();

            /**
             * @brief Set USB divider value
             * 
             * @tparam divider USB divider value
             * 
             * @par Returns
             *	Nothing
             */
            template<unsigned divider>
            static void SetUsbOutputDivider();

            /**
             * @brief Return I2S divider value
             *
             * @returns I2S divider value
             */
            static unsigned GetI2SOutputDivider();

            /**
             * @brief Set I2S divider value
             *
             * @tparam divider I2S divider value
             *
             * @par Returns
             *	Nothing
             */
            template<unsigned divider>
            static void SetI2SOutputDivider();

            /**
             * @brief Set PLL clock source
             * 
             * @tparam clockSource Source for Pll (HSE or HSI)
             * 
             * @par Returns
             *	Nothing
             */
            template<ClockSource clockSource>
            static void SelectClockSource();

            /**
             * @brief Return current PLL source
             * 
             * @retval Internal PLL source is HSI
             * @retval External PLL source is HSI
             */
            static ClockSource GetClockSource();

            /**
             * @brief Returns result frequence
             * 
             * @returns Pll result frequence
             */
            static ClockFrequenceT ClockFreq();

            /**
             * @brief Enables PLL
             * 
             * @retval true Success
             * @retval false Fail
             */
            static bool Enable();

            /**
             * @brief Disables PLL
             * 
             * @retval true Success
             * @retval false Fail
             */
            static void Disable();
        };

    #if defined (RCC_CSR_LSION)
        /**
         * @brief Implements internal low source
         */
        class LsiClock : public ClockBase<RccCsrReg>
        {
        public:
            /**
             * @brief Returns internal low oscillator clock frequence
             * 
             * @returns Internal low source (LSI) clock frequence
             */
            static constexpr ClockFrequenceT SrcClockFreq();
         

            /**
             * @brief Returns LSI divider
             * 
             * @returns Lse divider
             */
            static constexpr unsigned GetDivider();

            /**
             * @brief Returns LSI multiplier
             * 
             * @returns Hsi multiplier
             */
            static constexpr unsigned GetMultipler();


            /**
             * @brief Returns final clock frequence
             * 
             * @returns Final frequence
             */
            static constexpr ClockFrequenceT ClockFreq();
            

            /**
             * @brief Enables LSI source
             * 
             * @retval true Successful enable
             * @retval false Fail enable
             */
            static bool Enable();

            /**
             * @brief Disables LSI source
             * 
             * @retval true Successful disable
             * @retval false Fail disable
             */
            static bool Disable();
        };
    #endif

        /**
         * @brief Implements system clock
         */
        class SysClock
        {
        public:
            /// Clock source
            enum ClockSource
            {
                Internal,	///< HSI
                External,	///< HSE
                Pll,		///< PLL
            };

            /// Error codes for class methods
            enum ErrorCode
            {
                Success,			///< Success (no error)
                ClockSourceFailed,	///< Fail to enable clock source
                InvalidClockSource,	///< Invalid clock source (not HSI, HSE or PLL)
                ClockSelectFailed	///< Fail to select clock source
            };
            
        public:
            /**
             * @brief Returns max allowed system clock frequence
             * 
             * @returns Max clock frequence
             */
            static ClockFrequenceT MaxFreq();

            /**
             * Select source for system clock
             * 
             * @tparam clockSource Clock source (Internal, external or PLL)
             * 
             * @returns Select result
             */
            template<ClockSource clockSource>
            static ErrorCode SelectClockSource();

            /**
             * @brief Returns system clock frequence
             * 
             * @returns Clock frequence
             */
            static ClockFrequenceT ClockFreq();
            
            /**
             * @brief Returns source clock frequence
             * 
             * @details
             * System clock frequence = source clock frequence
             * 
             * @returns Source clock frequence
             */
            static ClockFrequenceT SrcClockFreq();
        };

        /**
         * @brief Base class for bus clock
         */
        template<typename _SrcClock, typename _PrescalerBitField>
        class BusClock
        {
        public:
            /**
             * @brief Returns source clock frequence of AHB
             * 
             * @returns Clock frequence
             */
            static ClockFrequenceT SrcClockFreq();
            
            /**
             * @brief Returns current clock frequence of AHB
             * 
             * @returns Current frequence
             */
            static ClockFrequenceT ClockFreq();

            /**
             * @brief Set prescaler for Bus
             * 
             * @param [in] prescaler Prescaler
             * 
             * @par Returns
             *	Nothing
             */
            template<typename PrescalerType>
            static void SetPrescaler(PrescalerType prescaler);
        };
        
        /**
         * @brief Implements ADC clock
         */
        class _AdcClockSource
        {
        public:
            /**
             * @brief ADC clock sources
             */
            enum ClockSource
            {};
            
            /**
             * @brief ADC prescaler
             */
            enum Prescaler
            {};
            
            /**
             * @brief Select clock source for ADC
             * 
             * @retval true Success select source
             * @retval false Fail select source
             */
            template<ClockSource source>
            static bool SelectClockSource();
            
            /**
             * @brief Set prescaler for ADC
             * 
             * @par Returns
             *	Nothing
             */
            template<Prescaler prescaler>
            static void SetPrescaler();
            
            /**
             * @brief Returns source clock frequence of ADC
             * 
             * @returns Clock frequence
             */
            static ClockFrequenceT SrcClockFreq();
            
            /**
             * @brief Returns current clock frequence of ADC
             * 
             * @returns Current frequence
             */
            static ClockFrequenceT ClockFreq();
        };
        
        /**
         * @brief Implements clock control
         * 
         * @tparam _Reg register address
         * @tparam _Mask Clock enable mask
         * @tparam _ClockSrc Clock source
         */
        template<typename _Reg, unsigned _Mask, typename _ClockSrc>
        class ClockControl :public _ClockSrc
        {
        public:
            /**
             * @brief Enable clock
             * 
             * @par Returns
             * 	Nothing
             */
            static void Enable();
           
            
            /**
             * @brief Disable clock
             * 
             * @par Returns
             * 	Nothing
             */
            static void Disable();
        };
    }
}

#include "impl/clock.h"

#endif //! ZHELE_CLOCK_COMMON_H