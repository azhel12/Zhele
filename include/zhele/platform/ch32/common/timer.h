/**
 * @file
 * Timers for CH32 (WCH TIM peripheral)
 *
 * @note Counting, update interrupt/DMA and PWM / output-compare only; input
 *       capture and slave/encoder modes are not implemented yet.
 *
 * @author Aleksei Zhelonkin (based on the STM32 timer implementation by Konstantin Chizhov)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_TIMER_H
#define ZHELE_PLATFORM_CH32_COMMON_TIMER_H

#include <zhele/common/template_utils/enum.h>

#include <zhele/clock.h>
#include <zhele/iopins.h>

#include <cstdint>
#include <type_traits>

namespace Zhele::Timers
{
    namespace Private
    {
        /**
         * @brief Basic (counting) timer.
         *
         * @tparam _Regs Timer register wrapper (returns TIM_TypeDef*)
         * @tparam _ClockEnReg Timer clock control (Enable/Disable/ClockFreq)
         * @tparam _IRQNumber Timer update IRQ number
         */
        template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber>
        class BaseTimer
        {
        public:
            using Prescaler = uint16_t;
            using Counter = uint16_t;

            /// Update / capture-compare interrupt sources (DMAINTENR bits).
            enum class Interrupt : uint32_t
            {
                Update  = TIM_UIE,
                CC1     = TIM_CC1IE,
                CC2     = TIM_CC2IE,
                CC3     = TIM_CC3IE,
                CC4     = TIM_CC4IE,
                COM     = TIM_COMIE,
                Trigger = TIM_TIE,
                Break   = TIM_BIE,
            };

            /// Counter direction / alignment (CTLR1 bits).
            enum class CounterMode : uint32_t
            {
                Up             = 0,
                Down           = TIM_DIR,
                CenterAligned1 = TIM_CMS_0,
                CenterAligned2 = TIM_CMS_1,
                CenterAligned3 = TIM_CMS,
            };

            /// Master mode (TRGO source, CTLR2 MMS field at bits 4:6).
            enum class MasterMode : uint16_t
            {
                Reset        = 0u << 4,
                Enable       = 1u << 4,
                Update       = 2u << 4,
                ComparePulse = 3u << 4,
                CompareCh1   = 4u << 4,
                CompareCh2   = 5u << 4,
                CompareCh3   = 6u << 4,
                CompareCh4   = 7u << 4,
            };

            static unsigned GetClockFreq();

            static void Enable();
            static void Disable();

            static void SetCounterMode(CounterMode mode);

            static void SetCounterValue(Counter counter);
            static Counter GetCounterValue();
            static void ResetCounterValue();

            static void SetPrescaler(Prescaler prescaler);
            static Counter GetPrescaler();

            static void SetPeriod(Counter period);
            static Counter GetPeriod();

            /// Set the period and force an update (reloads shadow regs, zeroes CNT).
            static void SetPeriodAndUpdate(Counter period);

            static void EnableOnePulseMode();
            static void DisableOnePulseMode();

            static void Stop();

            /// Start the timer (generate an update to load the prescaler, then run).
            static void Start();

            static void EnableInterrupt(Interrupt interrupt = Interrupt::Update);

            static void DisableInterrupt(Interrupt interrupt = Interrupt::Update);

            static bool IsInterrupt();
            static void ClearInterruptFlag();

            static void SetMasterMode(MasterMode mode);

            static void DmaRequestEnable();
            static void DmaRequestDisable();
        };

        /**
         * @brief General-purpose timer: adds PWM / output-compare on 4 channels.
         *
         * @tparam _ChPins Channel-pin trait: _ChPins<N>::Pins is a PinList of the
         *         candidate output pins for channel N (0-based).
         */
        template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        class GPTimer : public BaseTimer<_Regs, _ClockEnReg, _IRQNumber>
        {
            using Base = BaseTimer<_Regs, _ClockEnReg, _IRQNumber>;

        public:
            /**
             * @brief Output-compare / PWM control for one channel.
             *
             * @tparam _Channel Channel index (0..3)
             */
            template<unsigned _Channel>
            class OutputCompare
            {
                static_assert(_Channel < 4);

                // Odd channel of each CHCTLR pair sits in the high byte (+8).
                static constexpr uint32_t ByteShift = (_Channel & 1u) ? 8u : 0u;
                // CCxE/CCxP live in CCER, 4 bits per channel.
                static constexpr uint32_t CcerShift = 4u * _Channel;

                static volatile uint16_t& Ccr()
                {
                    if constexpr (_Channel == 0) return _Regs()->CH1CVR;
                    else if constexpr (_Channel == 1) return _Regs()->CH2CVR;
                    else if constexpr (_Channel == 2) return _Regs()->CH3CVR;
                    else return _Regs()->CH4CVR;
                }

                static volatile uint16_t& Chctlr()
                {
                    if constexpr (_Channel < 2) return _Regs()->CHCTLR1;
                    else return _Regs()->CHCTLR2;
                }

            public:
                using Pins = typename _ChPins<_Channel>::Pins;

                /// Output-compare mode (values are the channel-1 field, shifted per channel).
                enum OutputMode : uint32_t
                {
                    Timing         = 0,
                    Active         = TIM_OC1M_0,
                    Inactive       = TIM_OC1M_1,
                    Toggle         = TIM_OC1M_0 | TIM_OC1M_1,
                    PWM1           = TIM_OC1M_1 | TIM_OC1M_2,
                    PWM2           = TIM_OC1M,
                    ForcedActive   = TIM_OC1M_0 | TIM_OC1M_2,
                    ForcedInactive = TIM_OC1M_2,
                };

                enum OutputPolarity : uint32_t
                {
                    ActiveHigh = 0,
                    ActiveLow  = TIM_CC1P,
                };

                static void SetOutputMode(OutputMode mode)
                {
                    // Configure as output (CCxS = 0) with preload enabled + the OCxM mode.
                    uint32_t clearMask = (TIM_CC1S | TIM_OC1M | TIM_OC1PE) << ByteShift;
                    uint32_t setMask   = (static_cast<uint32_t>(mode) | TIM_OC1PE) << ByteShift;
                    Chctlr() = (Chctlr() & ~clearMask) | setMask;
                }

                static void SetOutputPolarity(OutputPolarity polarity)
                {
                    _Regs()->CCER = (_Regs()->CCER & ~(TIM_CC1P << CcerShift))
                                  | (static_cast<uint32_t>(polarity) << CcerShift);
                }

                static void SetPulse(typename Base::Counter pulse) { Ccr() = pulse; }
                static typename Base::Counter GetPulse() { return static_cast<typename Base::Counter>(Ccr()); }

                static void Enable()  { _Regs()->CCER |= (TIM_CC1E << CcerShift); }
                static void Disable() { _Regs()->CCER &= ~(TIM_CC1E << CcerShift); }

                /// Configure a channel output pin (alternate-function push-pull).
                template<typename Pin>
                static void SelectPins()
                {
                    Pin::Port::Enable();
                    Pin::template SetConfiguration<Pin::Port::Configuration::AltFunc>();
                    Pin::template SetDriverType<Pin::Port::DriverType::PushPull>();
                    Pin::template SetSpeed<Pin::Port::Speed::Fast>();
                }

                /// Configure the default (first) output pin of this channel.
                static void SelectPins()
                {
                    SelectPins<typename Pins::template Pin<0>>();
                }
            };

            /// PWM generation is output-compare in a PWM mode; alias for clarity.
            template<unsigned _Channel>
            using PWMGeneration = OutputCompare<_Channel>;
        };

        /**
         * @brief Advanced timer (TIM1): adds the main-output enable and repetition counter.
         */
        template<typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        class AdvancedTimer : public GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>
        {
        public:
            /// Channel outputs on an advanced timer are gated by BDTR.MOE — call this to drive them.
            static void EnableMainOutput();
            static void DisableMainOutput();

            static void SetRepetitionCounter(uint8_t value);
            static uint8_t GetRepetitionCounter();
        };
    }
}

#include "impl/timer.h"

#endif // ZHELE_PLATFORM_CH32_COMMON_TIMER_H
