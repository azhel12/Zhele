/**
 * @file
 * @brief Macros, templates for timers
 * @author Konstantin Chizhov
 * @date 2016
 * @license FreeBSD
 */

#ifndef ZHELE_TIMER_COMMON_H
#define ZHELE_TIMER_COMMON_H

#include "clock.h"
#include "ioreg.h"
#include "iopins.h"
#include "macro_utils/enum.h"
#include "template_utils/type_list.h"
#include "pinlist.h"

namespace Zhele::Timers
{
    namespace Private
    {
        /**
         * @brief Class implements STM32 base timer`s functional.
         * 
         * @tparam _Regs - timers`s registers wrapper.
         * Regs type must has operator(), which returns TIM_TypeDef* (so, it's registers).
         * This parameter should be created with IO_STRUCT_WRAPPER macro.
         * 
         * @tparam _ClockEnReg - timers`s clock.
         * ClockEnReg type must have static methods ClockFreq, Enable, Disable.
         * This template parameter is used for enable and disable timer`s clocking.
         * This parameter should be created as ClockControl instance (see @ref clock.h headerfile)
         * 
         * @tparam IRQNumber - number of timer`s IRQ
         *
         **/
        template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber>
        class BaseTimer
        {
        public:
            using Prescaler = uint16_t;
            using Counter = uint16_t;

            /// All timer`s interrupts
            enum class Interrupt
            {
                Update = TIM_DIER_UIE,	///< update event (overflow)
                CC1 = TIM_DIER_CC1IE,	///< Capture compare (1 channel)
                CC2 = TIM_DIER_CC2IE,	///< Capture compare (2 channel)
                CC3 = TIM_DIER_CC3IE,	///< Capture compare (3 channel)
                CC4 = TIM_DIER_CC4IE,	///< Capture compare (4 channel)
                COM = TIM_DIER_COMIE,	///< COM interrupt
                Trigger = TIM_DIER_TIE,	///< Trigger output
                Break = TIM_DIER_BIE,	///< Break
            };
            DECLARE_ENUM_OPERATIONS_IN_CLASS(Interrupt)

            /// Timer counter mode
            enum class CounterMode
            {
                Up = 0x00000000U,	///< count up
                Down = TIM_CR1_DIR,	///< count down
                CenterAligned1 = TIM_CR1_CMS_0,	///< center aligned, interrupt with counting down
                CenterAligned2 = TIM_CR1_CMS_1, ///< center aligned, interrupt with counting up
                CenterAligned3 = TIM_CR1_CMS ///< center aligned, interrupt with both

            };

            /**
             * @brief Returns frequence of timer`s clock (bus)
             * 
             * @return Timer`s clock frequence
             */
            static unsigned GetClockFreq();
            

            /**
             * @brief Enables timer`s clocking
             * 
             * Enables clocking of timer (timer`s bus).
             * Because it's static class without properties (and without state),
             * all configs writes to registers immediately, this method should be called
             * first, otherwise other methods (such as SetXxx/GetXxx) will work incorrect.
             * 
             * @par Returns
             *	Nothing
             */
            static void Enable();
         

            /**
             * @brief Disables timer`s clocking
             * 
             * @par Returns
             *	Nothing
             */
            static void Disable();
            

            /**
             * @brief Set timer`s counter value
             * 
             * @param [in] counter Timer`s counter value
             * 
             * @par Returns
             *	Nothing
             */
            static void SetCounterValue(Counter counter);
            

            /**
             * @brief Returns timer`s counter value
             * 
             * @returns Current counter value
             */
            static Counter GetCounterValue();
           

            /**
             * @brief Reset timer`s counter value (set to zero)
             *
             * @par Returns
             *	Nothing
                */
            static void ResetCounterValue();
            

            /**
             * @brief Set timer`s prescaler (PSC register) value
             * 
             * @param [in] prescaler New prescaler value
             * 
             * @par Returns
             *	Nothing
                */
            static void SetPrescaler(Prescaler prescaler);
           

            /**
             * @brief Returns timer`s prescaler value
             * 
             * @returns Current prescaler (PSC register value) value
             */
            static Counter GetPrescaler();
        

            /**
             * @brief Set timer`s period (ARR register) value
             * 
             * @param [in] period New period
             * 
             * @par Returns
             *	Nothing
             *
             * @note Action of this method depends of ARPE
             * (auto-reload preload enable) bit in CR1 register
             */
            static void SetPeriod(Counter period);
           

            /**
             * @brief Set timer`s period immediately (and reset CNT)
             * 
             * @detailss
             * Writes new value to ARR register and set EGR_UG bit.
             * So, CNT will be zeroing, new ARR value will be written to shadow register (if no UDIS bit is set).
             * 
             * @par Returns
             *	Nothing
             */
            static void SetPeriodAndUpdate(Counter value);
          

            /**
             * @brief Returns timer`s period (ARR register value)
             * 
             * @returns Current period (ARR register value)
             * 
             * @note Action of this method depends of ARPE
             * (auto-reload preload enable) bit in CR1 register
             */
            static Counter GetPeriod();
           
            /**
             * @brief Stop timer
             * 
             * @details
             * Assign CR1 and CR2 to zero
             * 
             * @par Returns
             *	Nothing
             */
            static void Stop();
           
            /**
             * @brief Start timer
             * 
             * @details
             * Send UG bit to EGR register and
             * CEN and URS bits to CR1 register.
             * So, now you cannot assign initial CNT value,
             * because after start CNT register zeroing will occurs
             * (by EGR_UG bit).
             * 
             * @par Returns
             *	Nothing
             */
            static void Start();
           

            /**
             * @brief Enables update interrupt for timer
             * 
             * @detailss
             * Method does not have any parameters (such as interrupt type), because
             * it's base timer and it supports only update (overflow) event
             * 
             * @par Returns
             *	Nothing
             */
            static void EnableInterrupt();
            


            /**
             * @brief Disables update event interrupt
             * 
             * @par Returns
             *	Nothing
             */
            static void DisableInterrupt();
            

            /**
             * @brief Returns is interrupt was occured.
             * 
             * @retval false No interrupt was occured
             * @retval true Interrupt was occured (update event)
             */
            static bool IsInterrupt();
            


            /**
             * @brief Reset interrupt
             * 
             * @detailss Reset UIF in SR register (set SR to zero)
             * 
             * @par Returns
             *	Nothing
             */
            static void ClearInterruptFlag();
          


            /**
             * @brief Enable DMA requests
             * 
             * @detailss
             * Method set UDE flag in DIER register.
             * So, DMA requests will occur with update event
             * 
             * @par Returns
             * Nothing
             */
            static void DmaRequestEnable();
           

            /**
             * @brief Disable DMA requests
             * 
             * @par Returns
             * Nothing
             */
            static void DmaRequestDisable();
          
        };

        /**
         * @brief Class implements STM32 General-Purpose timer`s functional.
         * 
         * @tparam _Regs Timer`s register wrapper
         * @tparam _ClockEnReg Timer`s clock
         * @tparam _IRQNumber Timer`s IRQ number
         * @tparam _ChPins Channel`s pins
         */
        template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        class GPTimer :public BaseTimer<_Regs, _ClockEnReg, _IRQNumber>
        {
            IO_BITFIELD_WRAPPER(_Regs()->CCMR1, Channel1Mode, uint32_t, 0, 8);
            IO_BITFIELD_WRAPPER(_Regs()->CCMR1, Channel2Mode, uint32_t, 8, 8);
            IO_BITFIELD_WRAPPER(_Regs()->CCMR2, Channel3Mode, uint32_t, 0, 8);
            IO_BITFIELD_WRAPPER(_Regs()->CCMR2, Channel4Mode, uint32_t, 8, 8);
            using ModeList = TypeList<Channel1Mode, Channel2Mode, Channel3Mode, Channel4Mode>;
            using Base = BaseTimer<_Regs, _ClockEnReg, _IRQNumber>;

        public:
            /**
             * @brief Internal class for output compare feature
             * 
             * @tparam _ChannelNumber Channel number
             */
            template<unsigned _ChannelNumber>
            class OutputCompare
            {
                using ModeBitField = typename GetType<_ChannelNumber, ModeList>::type;
            public:
                /// Output polarity
                enum OutputPolarity
                {
                    ActiveHigh = 0,
                    ActiveLow  = TIM_CCER_CC1P
                };

                /// Output mode
                enum OutputMode
                {
                    Timing         = 0,
                    Active         = TIM_CCMR1_OC1M_0,
                    Inactive       = TIM_CCMR1_OC1M_1,
                    Toggle         = TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1,
                    PWM1           = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2,
                    PWM2           = TIM_CCMR1_OC1M,
                    ForcedActive   = TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_2,
                    ForcedInactive = TIM_CCMR1_OC1M_2
                };
                static_assert(_ChannelNumber < 4);
                using Pins = typename _ChPins<_ChannelNumber>::Pins::Key;
                using PinsAltFuncNumber = typename _ChPins<_ChannelNumber>::Pins::Value;
            
                /**
                 * @brief Set OC pulse
                 * 
                 * @param [in] 
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void SetPulse(typename Base::Counter pulse);
                

                /**
                 * @brief Returns current pulse value
                 * 
                 * @returns Pulse value
                 */
                static typename Base::Counter GetPulse();
                

                /**
                 * @brief Enable interrupt
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void EnableInterrupt();
               
            
                /**
                 * @brief Disable interrupt
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void DisableInterrupt();
               

                /**
                 * @brief Check that interrupt is enabled
                 * 
                 * @retval true Interrupt enabled
                 * @retval false Interrupt disabled
                 */
                static bool IsInterrupt();
                
        
                /**
                 * @brief Clear interrupt flag
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void ClearInterruptFlag();
               
            
                /**
                 * @brief Enable OC channel
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void Enable();
               
            
                /**
                 * @brief Disable OC channel
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void Disable();
                
            
                /**
                 * @brief Set output polarity
                 * 
                 * @param [in] polarity Output polarity
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void SetOutputPolarity(OutputPolarity polarity);
               
            
                /**
                 * @brief Set output mode
                 * 
                 * @param [in] mode Output mode
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void SetOutputMode(OutputMode mode);
               
            
                /**
                 * @brief Select channel pin
                 * 
                 * @param [in] pinNumber Pin number
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void SelectPins(int pinNumber);
            
                /**
                 * @brief Select channel pin by number (template method)
                 * 
                 * @tparam PinNumber Pin number
                 */
                template<unsigned PinNumber>
                static void SelectPins();

                /**
                 * @brief Select channel pin (template method)
                 * 
                 * @tparam Pin Pin class
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename Pin>
                static void SelectPins();
            };

            /**
             * @brief Internal class for PWM feature
             * 
             * @tparam _ChannelNumber Channel number
             */
            template<unsigned _ChannelNumber>
            class PWMGeneration : public OutputCompare<_ChannelNumber>
            {
                using Base = OutputCompare<_ChannelNumber>;
                using Pins = typename Base::Pins;
            public:
                /// PWM Fast mod
                enum class FastMode
                {
                    Disable = 0x00000000U,
                    Enable  = TIM_CCMR1_OC1FE
                };

                /**
                 * @brief Set output fast mode (enable or disable)
                 * 
                 * @param[in] mode Fast mode
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void SetOutputFastMode(FastMode mode);
               

                /**
                 * @brief Select channel pin
                 * 
                 * @param [in] pinNumber Pin number
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                static void SelectPins(int pinNumber);
               

                /**
                 * @brief Select channel pin (template method by number)
                 * 
                 * @param [in] pinNumber Pin number
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<unsigned PinNumber>
                static void SelectPins();
              

                /**
                 * @brief Select channel pin (template method)
                 * 
                 * @tparam Pin Pin class
                 * 
                 * @par Returns
                 * 	Nothing
                 */
                template<typename Pin>
                static void SelectPins();
               
            };
        };
        
        /**
         * @brief Class implements STM32 advanced timer`s functional.
         * 
         * @tparam _Regs Timer`s register wrapper
         * @tparam _ClockEnReg Timer`s clock
         * @tparam _IRQNumber Timer`s IRQ number
         * @tparam _ChPins Channel`s pins
         * 
         * @todo Implements this class!
         */
        template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        class AdvancedTimer : public GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>
        {
            using Base = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>;
            // Not implemented yet
        };
    }
}

#include "impl/timer.h"

#endif //! ZHELE_TIMER_COMMON_H