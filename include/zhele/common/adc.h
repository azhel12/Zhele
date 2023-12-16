
/**
 * @file
 * @brief Implements ADC
 * @date 2016
 * @author Konstantin Chizhov
 * @license FreeBSD
 */
#ifndef ZHELE_ADC_COMMON_H
#define ZHELE_ADC_COMMON_H

#include <initializer_list>

namespace Zhele
{
    using AdcCallbackType = std::add_pointer_t<void(uint16_t* data, uint32_t count)>;

    namespace Private
    {
        /**
         * @brief Implements common code for ADC
         */
        class AdcCommon
        {
        public:
            // Data type (16bit for 12-bit ADC resolution)
            using DataT = uint16_t;

            static const unsigned MaxRegular = 16;
            static const unsigned MaxInjected = 4;

            /// Nominal reference voltage (3.3v)
            static const unsigned VRefNominal = 33000;
            
            /// Possible adc results
            enum class AdcError
            {
                NoError, //< No error
                Overflow, //< Overflow
                TransferError, //< Transfer error
                HardwareError, //< Hardware error
                ArgumentError, //< Argument error
                RegularError, //< Regular error
                NotReady //< Adc not ready
            };
            
            // Channels count (16 external + internal VRef and Temp)
            static const uint8_t ChannelCount = 18;

            // Reference voltage source
            enum class Reference
            {
                External = 0, //< External source
                VCC = 0 //< MCU power VCC
            };

            // Number of temperature sensor channel
            static const uint8_t TempSensorChannel = 16;
            // Number of referenve voltage channel
            static const uint8_t ReferenceChannel = 17;
        };

        /**
         * @brief
         * Implements ADC data structure.
         */
        struct AdcData
        {
            /**
             * @brief
             * Constructor
             */
            AdcData()
                : regularCallback(nullptr),
                injectedCallback(nullptr),
                regularData(0),
                injectedData(0),
                error(AdcCommon::AdcError::NoError),
                vRef(0)
            {
            }

            AdcCallbackType regularCallback;
            AdcCallbackType injectedCallback;
            uint16_t* regularData;
            uint16_t* injectedData;
            AdcCommon::AdcError error;
            uint16_t vRef;
        };

        template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
        class AdcBase : public AdcCommon
        {
            // Maximum ADC convertion time MaxCockDivider * AdcConvertionCycles / cpu cycle per wait loop
            static const unsigned AdcTimeoutCycles = 2048 * 28 / 4;
        public:
            static const uint8_t ResolutionBits = 12;

            using Clock = _ClockCtrl;
            using Pins = _InputPins;

            using AdcDivider = typename _ClockCtrl::Prescaler;
            using ClockSource = typename _ClockCtrl::ClockSource;

            /// Conversion mode
            enum class ConversionMode : uint8_t
            {
                Single, //< Single (discontinuous) conversion mode
                Continuous //< Continuous conversion mode
            };

            // Clock sources
            static const ClockSource AdcClock = _ClockCtrl::Apb2;
            static const ClockSource MainClock = _ClockCtrl::Apb2;

            /**
             * @brief Set adc clock source
             * 
             * @param [in] clockSource Clock source
             * 
             * @par Returns
             *  Nothing
             */
            static void SelectClockSource(ClockSource clockSource);

            /**
             * @brief Set adc referenve voltage source
             * 
             * @param [in] reference Reference voltage source
             * 
             * @par Returns
             *  Nothing
             */
            static void SetReference(Reference reference);

            /**
             * @brief Set adc clock divider
             * 
             * @param [in]  divider Clock divider
             * 
             * @par Returns
             *  Nothing
             */
            static void SetDivider(AdcDivider divider);

            /**
             * @brief Set sample time
             * 
             * @param [in] channel Channel
             * @param [in] sampleTime Sample time for given channel
             * 
             * @par Returns
             *  Nothing
             */
            static void SetSampleTime(uint8_t channel, unsigned sampleTime);

            /**
             * @brief Init adc
             * 
             * @param [in] divider Clock divider
             * @param [in] clockSource Clock source
             * @param [in] reference Reference volatage source
             * 
             * @par Returns
             *  Nothing
             */
            static void Init(AdcDivider divider = AdcDivider::Div2, ClockSource clockSource = AdcClock, Reference reference = Reference::External);
            
            /**
             * @brief Set adc resolution
             * 
             * @param [in] resolutino Resolution (bits count)
             * 
             * @returns Result resolution
             */
            static uint8_t SetResolution(uint8_t resolution);

            /**
             * @brief Set conversion mode
             * 
             * @param [in] mode Conversion mode
             * 
             * @par Returns
             *  Nothing
             */
            static void SetConversionMode(ConversionMode mode);

            /**
             * @brief Disable adc
             * 
             * @par Returns
             *  Nothing
             */
            static void Disable();

            /**
             * @brief Set injected trigger
             * 
             * @param [in] trigger Trigger
             * @param [in] mode Trigger mode
             * 
             * @par Returns
             *  Nothing
             */
            template<typename InjectedTrigger, typename TriggerMode>
            static void SetInjectedTrigger(InjectedTrigger trigger, TriggerMode mode);
            
            /**
             * @brief Returns state for injected measurement
             * 
             * @retval true ADC ready for measurement
             * @retval false ADC does not ready for measurement
             */
            static bool InjectedReady();

            /**
             * @brief Start injected measurement
             * 
             * @param [in] channels Channels
             * @param [out] data Buffer for result
             * @param [in] count Channels count
             * @param [in, opt] callback Callback
             * 
             * @retval true Successful measurement
             * @retval false Measurement fail
             */
            static bool StartInjected(const uint8_t* channels, uint16_t* data, uint8_t count, AdcCallbackType callback = nullptr);
            
            /**
             * @brief Start injected measurment for one channel
             * 
             * @param [in] channel Channel
             * 
             * @retval true Successful measurement
             * @retval false Measurement fail
             */
            static bool StartInjected(uint8_t channel);

            /**
             * @brief Returns injected measurement results
             * 
             * @param [in] channels Channels
             * @param [out] data Buffer for result
             * @param [in] count Channels count
             */
            static bool ReadInjected(const uint8_t* channels, uint16_t* data, uint8_t count);

            /**
             * @brief Returns injected measurement result for one channel
             * 
             * @param [in] channel Channel
             * 
             * @returns Measurement result
             */
            static uint16_t ReadInjected(uint8_t channel);
            
            /**
             * @brief Returns injected measurement
             * 
             * @returns Measurement result
             */
            static uint16_t ReadInjected();

            /**
             * @brief Returns injected measurement result for one channel
             * 
             * @tparam _Pin Pin (channel)
             * 
             * @returns Measurement result
             */
            template <typename _Pin>
            static uint16_t ReadInjected();

            /**
             * @brief Stop injected measurement
             * 
             * @par Returns
             * 	Nothing
             */
            static void StopInjected();

            /**
             * @brief Set trigger for start regular measurement
             * 
             * @param [in] trigger Trigger source
             * @param [in] mode Trigger mode
             * 
             * @par Returns
             * 	Nothing
             */
            template<typename RegularTrigger, typename TriggerMode>
            static void SetRegularTrigger(RegularTrigger trigger, TriggerMode mode);

            /**
             * @brief Set callback for regular measurement complete
             * 
             * @param [in] callback Pointer to callback function
             * 
             * @par Returns
             * 	Nothing
             */
            static void SetRegularCallback(AdcCallbackType callback);

            /**
             * @brief Returns state for regular measurement
             * 
             * @retval true ADC ready for regular measurement
             * @retval false ADC does not ready for regular measurement
             */
            static bool RegularReady();

            /**
             * @brief Start regular measurement
             * 
             * @param [in] channels Array with channels
             * @param [in] channelsCount Channels count
             * @param [out] dataBuffer Buffer. Must has hannelsCount * scanCount elements.
             * @param [in] scanCount Scan count
             * @param [in] discontinuous Discontinuous number (0 for off)
             * 
             * @retval true Success measurement
             * @retval false Measurement fail
             */
            static bool StartRegular(const uint8_t* channels, uint8_t channelsCount, uint16_t* dataBuffer, uint16_t scanCount, uint8_t discontinuous = 0);
            /**
             * @brief Start regular measurement ()
             * 
             * @param [in] channels Channels as initializer_list
             * @param [in] channelsCount Channels count
             * @param [out] dataBuffer Buffer. Must has hannelsCount * scanCount elements.
             * @param [in] scanCount Scan count
             * @param [in] discontinuous Discontinuous number (0 for off)
             * 
             * @retval true Success measurement
             * @retval false Measurement fail
             */
            static bool StartRegular(std::initializer_list<uint8_t> channels, uint16_t* dataBuffer, uint16_t scanCount, uint8_t discontinuous = 0);

            /**
             * @brief Start regular measurement
             * 
             * @tparam _Pins Variadic templates with inputs pins
             * @param [in] dataBuffer Buffer
             * @param [in] scanCount Scan count
             * 
             * @retval true Success measurement
             * @retval false Measurement fail
             */
            template <typename... _Pins>
            static bool StartRegular(uint16_t* dataBuffer, uint16_t scanCount);

            /**
             * @brief Stop regular measurement
             * 
             * @par Returns
             * 	Nothing
             */
            static void StopRegular();

            /**
             * @brief Convert GPIO pin to ADC channel number
             * 
             * @tparam [in] Pin GPIO pin
             * 
             * @returns Appropriate channel number
             */
            template <typename _Pin>
            constexpr static unsigned ChannelNum();

            /**
             * @brief Get error of last operation
             * 
             * @returns Error code
             */
            static AdcError GetError();

            /**
             * @brief Converts measurement result to voltage
             * 
             * @param [in] value Measurement result
             * 
             * @returns Result as voltage in 10E-4 uints
             */
            static unsigned ToVolts(uint16_t value);

            /**
             * @brief Returns ADC source clock frequence
             * 
             * @returns Source clock frequence
             */
            static unsigned ClockFreq();

            /**
             * @brief Returns convertion times (in cycles) for channel.
             * 
             * @param [in] channel Target channel
             * 
             * @returns Convertion time (in cycles).
             */
            static unsigned ConvertionTimeCycles(uint8_t channel);

            /**
             * @brief Returns convertion time (in 10us units) for channel.
             * 
             * @param [in] channel Target channel
             * 
             * @returns Convertion time (in cycles).
             */
            static unsigned AdcPeriodUs10(uint8_t channel);

            /**
             * @brief Measure temperature from temp channel
             * 
             * @returns Temperature
             */
            static int16_t ReadTemperature();

            /**
             * @brief Dma handler
             * 
             * @param [in] data Data
             * @param [in] size Data size
             * @param [in] success Is dma operation success.
             * 
             * @par Returns
             *  Nothing
             */
            static void DmaHandler(void *data, size_t size, bool success);

            /**
             * @brief Adc irq handler
             * 
             * @par Returns
             *  Nothing
             */
            static void IrqHandler();
            
        protected:
            static bool VerifyReady(unsigned);
            static unsigned SampleTimeToReg(unsigned sampleTime);
            static AdcData _adcData;
        };
    } // namespace Private
} // namespace Zhele

#include "impl/adc.h"

#endif //! ZHELE_ADC_COMMON_H