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

#include <flash.h>
#include "ioreg.h"

#if !defined  (HSE_VALUE) 
    #warning HSE_VALUE not defined. Will be used HSE_VALUE equals 8 MHZ.
    #define HSE_VALUE    ((uint32_t)8000000)
#endif //! HSE_VALUE

#if !defined  (HSI_VALUE)
    #warning HSI_VALUE not defined. Will be used HSI_VALUE equals 8 MHZ.
    #define HSI_VALUE    ((uint32_t)8000000)
#endif //! HSI_VALUE

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
            static bool EnableClockSource(unsigned turnMask,  unsigned waitReadyMask)
            {
                uint32_t timeoutCounter = 0;
                _Regs::Or(turnMask);
                while(((_Regs::Get() & waitReadyMask) == 0) && (timeoutCounter < ClockStartTimeout))
                {
                    timeoutCounter++;
                }
                return (_Regs::Get() & waitReadyMask) != 0;
            }

            /**
             * @brief Disable clocking
             * 
             * @param [in] turnMask Mask
             * @param [in] waitReadyMask Wait for ready mask
             */
            static bool DisableClockSource(unsigned turnMask,  unsigned waitReadyMask)
            {
                uint32_t timeoutCounter = 0;
                _Regs::And(~turnMask);
                while(((_Regs::Get() & waitReadyMask) != 0) && (timeoutCounter < ClockStartTimeout))
                {
                    timeoutCounter++;
                }
                return (_Regs::Get() & waitReadyMask) == 0;
            }
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
            static ClockFrequenceT SrcClockFreq()
            {
                return HSE_VALUE;
            }

            /**
             * @brief Returns HSE divider
             * 
             * @returns Hse divider
             */
            static ClockFrequenceT GetDivider() { return 1; }

            /**
             * @brief Returns HSE multiplier
             * 
             * @returns Hse multiplier
             */
            static ClockFrequenceT GetMultipler() { return 1; }


            /**
             * @brief Returns final clock frequence
             * 
             * @deail
             * Final clock frequece = source clock frequence / divider * multiplier
             * 
             * @returns Final frequence
             */
            static ClockFrequenceT ClockFreq()
            {
                return SrcClockFreq();
            }

            /**
             * @brief Enables Hse source
             * 
             * @retval true Successful enable
             * @retval false Fail enable
             */
            static bool Enable()
            {
                return ClockBase::EnableClockSource(RCC_CR_HSEON, RCC_CR_HSERDY);
            }

            /**
             * @brief Disables Hse source
             * 
             * @retval true Successful disable
             * @retval false Fail disable
             */
            static bool Disable()
            {
                return ClockBase::DisableClockSource(RCC_CR_HSEON, RCC_CR_HSERDY);
            }
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
            static ClockFrequenceT SrcClockFreq()
            {
                return HSI_VALUE;
            }

            /**
             * @brief Returns HSI divider
             * 
             * @returns Hsi divider
             */
            static ClockFrequenceT GetDivider() { return 1; }

            /**
             * @brief Returns HSI multiplier
             * 
             * @returns Hsi multiplier
             */
            static ClockFrequenceT GetMultipler() { return 1; }


            /**
             * @brief Returns final clock frequence
             * 
             * @details
             * Final clock frequece = source clock frequence / divider * multiplier
             * 
             * @returns Final frequence
             */
            static ClockFrequenceT ClockFreq()
            {
                return SrcClockFreq();
            }

            /**
             * @brief Enables Hsi source
             * 
             * @retval true Successful enable
             * @retval false Fail enable
             */
            static bool Enable()
            {
                return ClockBase::EnableClockSource(RCC_CR_HSION, RCC_CR_HSIRDY);
            }

            /**
             * @brief Disables Hsi source
             * 
             * @retval true Successful disable
             * @retval false Fail disable
             */
            static bool Disable()
            {
                return ClockBase::DisableClockSource(RCC_CR_HSION, RCC_CR_HSIRDY);
            }
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
                Internal,   //< Use internal oscillator (HSI) divided 2
                External	//< Use external oscillator (HSE)
            };

            /**
             * @brief Set PLL frequence the most close to given
             * 
             * @param [in] freq Needed frequence
             * 
             * @returns Actual frequence 
             */
            static ClockFrequenceT SetClockFreq(ClockFrequenceT freq);
            
            /**
             * @brief Returns PLL source clock frequence (HSI or HSE)
             * 
             * @returns PLL source frequence
             */
            static ClockFrequenceT SrcClockFreq()
            {
                return GetClockSource() == External
                    ? HseClock::ClockFreq()
                    : HsiClock::ClockFreq();
            }

            /**
             * @brief Returns PLL divider
             * 
             * @returns Pll divider
             */
            static ClockFrequenceT GetDivider();

            /**
             * @brief Set PLL divider
             *
             * @param [in] divider Pll divider value
             * 
             * @par Returns
             *	Nothing
             */
            static void SetDivider(ClockFrequenceT divider);

            /**
             * @brief Returns PLL multiplier
             * 
             * @returns Pll multiplier
             */
            static ClockFrequenceT GetMultipler();

            /**
             * @brief Set PLL multiplier
             * 
             * @param [in] multiplier Pll Multiplier value
             * 
             * @par Returns
             *	Nothing
             */
            static void SetMultiplier(ClockFrequenceT multiplier);		

        #if defined(RCC_PLLCFGR_PLLP)
            /**
             * @brief Return PLLP value
             * 
             * @returns PLLP value
             */
            static ClockFrequenceT GetSystemOutputDivider();

            /**
             * @brief Set PLLP
             * 
             * @param [in] divider PLLP value
             * 
             * @par Returns
             *	Nothing
             */
            static void SetSystemOutputDivider(ClockFrequenceT divider);

            /**
             * @brief Return PLLQ value
             * 
             * @returns PLLQ value
             */
            static ClockFrequenceT GetUsbOutputDivider();

            /**
             * @brief Set PLLQ
             * 
             * @param [in] divider PLLQ value
             * 
             * @par Returns
             *	Nothing
             */
            static void SetUsbOutputDivider(ClockFrequenceT divider);
        #endif

            /**
             * @brief Set PLL clock source
             * 
             * @param [in] clockSource Source for Pll (HSE or HSI)
             * 
             * @par Returns
             *	Nothing
             */
            static void SelectClockSource(ClockSource clockSource);

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
            static ClockFrequenceT ClockFreq()
            {
                return SrcClockFreq() / GetDivider() * GetMultipler();
            }

            /**
             * @brief Enables PLL
             * 
             * @retval true Success
             * @retval false Fail
             */
            static bool Enable()
            {
                if (GetClockSource() == Internal)
                {
                    if (!HsiClock::Enable())
                        return false;
                }
                else
                {
                    if (!HseClock::Enable())
                        return false;
                }
                return ClockBase::EnableClockSource(RCC_CR_PLLON, RCC_CR_PLLRDY);
            }

            /**
             * @brief Disables PLL
             * 
             * @retval true Success
             * @retval false Fail
             */
            static void Disable()
            {
                ClockBase::DisableClockSource(RCC_CR_PLLON, RCC_CR_PLLRDY);
            }
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
            static ClockFrequenceT SrcClockFreq()
            {
                return 32768;
            }

            /**
             * @brief Returns LSI divider
             * 
             * @returns Lse divider
             */
            static ClockFrequenceT GetDivider() { return 1; }

            /**
             * @brief Returns LSI multiplier
             * 
             * @returns Hsi multiplier
             */
            static ClockFrequenceT GetMultipler() { return 1; }


            /**
             * @brief Returns final clock frequence
             * 
             * @returns Final frequence
             */
            static ClockFrequenceT ClockFreq()
            {
                return SrcClockFreq();
            }

            /**
             * @brief Enables LSI source
             * 
             * @retval true Successful enable
             * @retval false Fail enable
             */
            static bool Enable()
            {
                return ClockBase::EnableClockSource(RCC_CSR_LSION, RCC_CSR_LSIRDY);
            }

            /**
             * @brief Disables LSI source
             * 
             * @retval true Successful disable
             * @retval false Fail disable
             */
            static bool Disable()
            {
                return ClockBase::DisableClockSource(RCC_CSR_LSION, RCC_CSR_LSIRDY);
            }
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
                Internal,	//< HSI
                External,	//< HSE
                Pll,		//< PLL
            };

            /// Error codes for class methods
            enum ErrorCode
            {
                Success,			//< Success (no error)
                ClockSourceFailed,	//< Fail to enable clock source
                InvalidClockSource,	//< Invalid clock source (not HSI, HSE or PLL)
                ClockSelectFailed	//< Fail to select clock source
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
             * @param [in] clockSource Clock source (Internal, external or PLL)
             * 
             * @returns Select result
             */
            static ErrorCode SelectClockSource(ClockSource clockSource)
            {
                uint32_t clockStatusValue;
                uint32_t clockSelectMask;
                uint32_t sourceFrequence;

                if(clockSource == Internal)
                {
                    clockStatusValue = RCC_CFGR_SWS_HSI;
                    clockSelectMask = RCC_CFGR_SW_HSI;
                    if (!HsiClock::Enable())
                        return ClockSourceFailed;
                    sourceFrequence = HsiClock::ClockFreq();
                }
                else if(clockSource == External)
                {
                    clockStatusValue = RCC_CFGR_SWS_HSE;
                    clockSelectMask = RCC_CFGR_SW_HSE;
                    if (!HseClock::Enable())
                        return ClockSourceFailed;
                    sourceFrequence = HseClock::ClockFreq();
                }
                else if(clockSource == Pll)
                {
                    clockStatusValue = RCC_CFGR_SWS_PLL;
                    clockSelectMask = RCC_CFGR_SW_PLL;
                    if (!PllClock::Enable())
                        return ClockSourceFailed;
                    sourceFrequence = PllClock::ClockFreq();
                }
                else
                {
                    return InvalidClockSource;
                }

                Flash::ConfigureFrequence(sourceFrequence);

                RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | clockSelectMask;
                
                uint32_t timeout = 10000;
                while (((RCC->CFGR & RCC_CFGR_SWS) != clockStatusValue) && --timeout)
                    ;
                if(timeout == 0)
                {
                    return ClockSelectFailed;
                }
                return Success;
            }

            /**
             * @brief Returns system clock frequence
             * 
             * @returns Clock frequence
             */
            static ClockFrequenceT ClockFreq()
            {
                uint32_t clockSrc = RCC->CFGR & RCC_CFGR_SWS;
                switch (clockSrc)
                {
                    case 0:              return HsiClock::ClockFreq();
                    case RCC_CFGR_SWS_0: return HseClock::ClockFreq();
                    case RCC_CFGR_SWS_1: return PllClock::ClockFreq();
                }
                return 0;
            }

            /**
             * @brief Returns source clock frequence
             * 
             * @details
             * System clock frequence = source clock frequence
             * 
             * @returns Source clock frequence
             */
            static ClockFrequenceT SrcClockFreq()
            {
                return ClockFreq();
            }

            /**
             * @brief Set clock frequence
             * 
             * @param [in] freq New frequence
             * 
             * @returns New clock frequence
             */
            static ClockFrequenceT SetClockFreq(ClockFrequenceT freq)
            {
                SelectClockSource(Internal);
                PllClock::Disable();
                PllClock::SelectClockSource(PllClock::External);
                PllClock::SetClockFreq(freq);
                SelectClockSource(Pll);
                return ClockFreq();
            }
        };

        IO_REG_WRAPPER(RCC->APB1ENR, PeriphClockEnable1, uint32_t);
        IO_REG_WRAPPER(RCC->APB2ENR, PeriphClockEnable2, uint32_t);

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
            static ClockFrequenceT SrcClockFreq()
            {
                return _SrcClock::ClockFreq();
            }

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
            static void SetPrescaler(PrescalerType prescaler)
            {
                _PrescalerBitField::Set(static_cast<ClockFrequenceT>(prescaler));
            }

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
            static bool SelectClockSource(ClockSource source);
            
            /**
             * @brief Set prescaler for ADC
             * 
             * @par Returns
             *	Nothing
             */
            static void SetPrescaler(Prescaler prescaller);
            
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
            static void Enable()
            {
                _Reg::Or(_Mask);
            }
            
            /**
             * @brief Disable clock
             * 
             * @par Returns
             * 	Nothing
             */
            static void Disable()
            {
                _Reg::And(~_Mask);
            }
        };
    }
}

#endif //! ZHELE_CLOCK_COMMON_H