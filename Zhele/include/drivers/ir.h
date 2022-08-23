/**
 * @file
 * Driver for IR receiver
 * 
 * Based on http://we.easyelectronics.ru/STM32/ir-usb-hid-ocherednoy-pult-dlya-kompa-chast-1.html
 * 
 * @author Aleksei Zhelonkin
 * @date 2022
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_IR_H
#define ZHELE_DRIVERS_IR_H

namespace Zhele::Drivers
{
    /**
     * @brief Class for IR receiver
     * 
     * @tparam _Timer GP timer instance
     * @tparam _Pin Input pin
     * @tparam _Decoder Decoder
     */
    template <typename _Timer, typename _Pin, typename _Decoder>
    class IrReceiver
    {
        using InputCaptureFalling = typename _Timer::InputCapture<0>;
        using InputCaptureRising = typename _Timer::InputCapture<1>;
        using TimeoutOCChannel = typename _Timer::OutputCompare<2>;

    public:
        /**
         * @brief Init receiver
         * 
         * @par Returns
         *  Nothing
         */
        static void Init()
        {
            _Timer::Enable();
            _Timer::SetPrescaler(_Timer::GetClockFreq() / 1000000 * 2 - 1); // 1us period
            _Timer::SetPeriod(0xffff);

            _Timer::SlaveMode::SelectTrigger(_Timer::SlaveMode::Trigger::FilteredTimerInput1);
            _Timer::SlaveMode::EnableSlaveMode(_Timer::SlaveMode::Mode::ResetMode);

            InputCaptureFalling::SetCapturePolarity(InputCaptureFalling::CapturePolarity::FallingEdge);
            InputCaptureFalling::SetCaptureMode(InputCaptureFalling::CaptureMode::Direct);
            InputCaptureFalling::EnableInterrupt();
            InputCaptureFalling::Enable();

            InputCaptureRising::SetCapturePolarity(InputCaptureRising::CapturePolarity::RisingEdge);
            InputCaptureRising::SetCaptureMode(InputCaptureRising::CaptureMode::Indirect);
            InputCaptureRising::EnableInterrupt();
            InputCaptureRising::Enable();

            TimeoutOCChannel::SetPulse(15'000);

            _Pin::Port::Enable();
            _Pin::template SetConfiguration<_Pin::Configuration::In>();
            _Pin::template SetPullMode<_Pin::PullMode::PullUp>();

            _Timer::Start();
        }

        /**
         * @brief Timer IRQ handler (call this method in TIMx_IRQHandler)
         * 
         * @par Returns
         *  Nothing
         */
        static void IRQHandler()
        {
            static bool idleState = true;

            if(InputCaptureFalling::IsInterrupt()) {
                InputCaptureFalling::ClearInterruptFlag();

                uint16_t width = InputCaptureFalling::GetValue();
                uint16_t pulse = InputCaptureRising::GetValue();

                if(idleState) {
                    idleState = false;
                    TimeoutOCChannel::EnableInterrupt();
                }
                else {
                    if(IsSimilar<_Decoder::StartWidth>(width) && IsSimilar<_Decoder::StartPulse>(pulse)) {
                        _Decoder::Start();
                    }
                    else if(IsSimilar<_Decoder::Width0>(width) && IsSimilar<_Decoder::Pulse0>(pulse)) {
                        _Decoder::Add0();
                    }
                    else if(IsSimilar<_Decoder::Width1>(width) && IsSimilar<_Decoder::Pulse1>(pulse)) {
                        _Decoder::Add1();
                    }
                    else {
                        idleState = true;
                    }
                }
            }

            if (TimeoutOCChannel::IsInterrupt()) {
                TimeoutOCChannel::DisableInterrupt();
                TimeoutOCChannel::ClearInterruptFlag();

                if(!idleState) {
                    idleState = true;
                    _Decoder::Handle();
                }
            }
        }
    private:
        /**
         * @brief Compare received value with decoder constant with given accuracy
         * 
         * @tparam _TargetValue Constant
         * @param value Received value
         * 
         * @return true If value ~ _TargetValue
         * @return false If value != _TargetValue
         */
        template<uint16_t _TargetValue>
        inline static bool IsSimilar(uint16_t value)
        {
            return ((_TargetValue * (100 - _Decoder::EpsilonInPercent) / 100) < value) && (value < (_TargetValue * (100 + _Decoder::EpsilonInPercent) / 100));
        }
    };

    /**
     * @brief NEC decoder
     */
    class NecDecoder
    {
    public:
        static const uint16_t StartWidth = 13500;
        static const uint16_t StartPulse = 9000;

        static const uint16_t Width0 = 1125;
        static const uint16_t Pulse0 = 562;

        static const uint16_t Width1 = 2250;
        static const uint16_t Pulse1 = 562;

        static const uint16_t EpsilonInPercent = 20;

        /// Command
        /// @todo Add commands
        enum Command : uint16_t
        {
            NoCommand = 0
        };

        using Callback = std::add_pointer_t<void(Command command)>;

        /**
         * @brief Set the callback for command receive
         * 
         * @param callback Callback
         * 
         * @par Returns
         *  Nothing
         */
        static void SetCallback(Callback callback)
        {
            _callback = callback;
        }

        /**
         * @brief Start new frame
         * 
         * @par Returns
         *  Nothing
         */
        static void Start()
        {
            _frame = 0;
        }

        /**
         * @brief Add bit 0 to frame
         * 
         * @par Returns
         *  Nothing
         */
        static void Add0()
        {
            _frame >>= 1;
        }

        /**
         * @brief Add bit 1 to frame
         * 
         * @par Returns
         *  Nothing
         */
        static void Add1()
        {
            _frame = (_frame >> 1) | 0x80000000;
        }

        /**
         * @brief End of frame (pause detect) handler
         * 
         * @par Returns
         *  Nothing
         */
        static void Handle()
        {
            if ((_frame & 0xff000000) != ( (~(_frame) & 0x00ff0000) << 8)) {
                return;
            }

            if ((_frame & 0xff00) != (((~_frame) & 0x00ff) << 8)) {
                return;
            }

            uint16_t command = ((_frame & 0xff000000) >> 16) | ((_frame & 0xff00) >> 8);
            
            if(_callback)
                _callback(static_cast<Command>(command));
        }

    private:
        static uint32_t _frame;
        static Callback _callback;
    };

    uint32_t NecDecoder::_frame;
    NecDecoder::Callback NecDecoder::_callback;
}
#endif // !ZHELE_DRIVERS_IR_H