/**
 * @file
 * Driver for encoder
 * 
 * @author Aleksei Zhelonkin
 * @date 2022
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_ENCODER_H
#define ZHELE_DRIVERS_ENCODER_H

#include <common/template_utils/type_list.h>

#include <cstdint>

namespace Zhele::Drivers
{
    /**
     * @brief Class for encoder
     * 
     * @tparam _Timer GP timer instance
     * @tparam _PinA Input pin A (for 1 channel)
     * @tparam _PinB Input pin B (for 2 channel)
     */
    template <typename _Timer,
            typename _PinA = Zhele::TemplateUtils::GetType_t<0, typename _Timer::InputCapture<0>::Pins>,
            typename _PinB = Zhele::TemplateUtils::GetType_t<0, typename _Timer::InputCapture<1>::Pins>,
            uint16_t _MaxValue = 0xffff>
    class Encoder
    {
        using InputA = typename _Timer::InputCapture<0>;
        using InputB = typename _Timer::InputCapture<1>;
    public:
        /**
         * @brief Init encoder
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            _Timer::Enable();
            _Timer::SetPeriod(_MaxValue * 2 + 1);

            _Timer::SlaveMode::EnableSlaveMode(_Timer::SlaveMode::Mode::EncoderMode2);

            InputA::Enable();
            InputA::SetCaptureMode(InputA::CaptureMode::Direct);
            InputA::template SelectPins<_PinA>();

            InputB::Enable();
            InputB::SetCaptureMode(InputB::CaptureMode::Direct);
            InputB::template SelectPins<_PinB>();

            _Timer::Start();
        }

        /**
         * @brief Enable interrupt
         * 
         */
        static void EnableInterrupt()
        {
            _Timer::SlaveMode::SelectTrigger(_Timer::SlaveMode::Trigger::Ti1EdgeDetector);
            _Timer::EnableInterrupt(_Timer::Interrupt::Trigger);
        }

        /**
         * @brief Returns current encoder value
         * 
         * @return uint16_t Current value
         */
        static uint16_t GetValue()
        {
            return _Timer::GetCounterValue() >> 1;
        }

        /**
         * @brief Returns current encoder value (interrupt mode)
         * 
         * @details For encoder mode 2 in interrupt counter value less by 2 for CW
         *  and more by 1 for CWW
         * 
         * @return uint16_t Current value
         */
        static uint16_t GetValueInterrupt()
        {
            uint16_t counter = _Timer::GetCounterValue();

            return (counter & 1) > 0
                ? (counter - 1) >> 1
                : ((counter + 2) >> 1) % (_MaxValue + 1);
        }
    };
}
#endif // !ZHELE_DRIVERS_ENCODER_H
