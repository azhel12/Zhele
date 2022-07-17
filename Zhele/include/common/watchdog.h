/**
 * @file

 * @brief Contains implementation of watchdog.
 * 
 * @author Aleksei Zhelonkin (based on Konstantin Chizhov mcucpp )
 * @date 2022
 * @licence FreeBSD
 */

#ifndef ZHELE_WATCHDOG_COMMON_H
#define ZHELE_WATCHDOG_COMMON_H

#include <cstdint>

#include <clock.h>

namespace Zhele::Timers
{
    class IWdg
    {
        static const uint32_t ReloadValue = 0x0000aaaa;
        static const uint32_t AccessValue = 0x00005555;
        static const uint32_t InitValue = 0x0000cccc;

    public:
        static constexpr unsigned ClockFreq = Zhele::Clock::LsiClock::ClockFreq();

        /**
         * @brief Prescaler
         */
        enum class Prescaler : uint8_t
        {
            Div4 = 0b000, ///< Divide by 4
            Div8 = 0b001, ///< Divide by 8
            Div16 = 0b010, ///< Divide by 16
            Div32 = 0b011, ///< Divide by 32
            Div64 = 0b100, ///< Divide by 64
            Div128 = 0b101, ///< Divide by 128
            Div256 = 0b110 ///< Divide by 256
        };

        /**
         * @brief Start watchdog
         * 
         * @par Returns
         *  Nothing
         */
        static void Start()
        {
            IWDG->KR = ReloadValue;
            IWDG->KR = InitValue;
        }

        /**
         * @brief Start watchdog with given period
         * 
         * @param [in] period Period in milliseconds
         * 
         * @par Returns
         *  Nothing
         */
        static void Start(uint16_t period)
        {
            auto prescalerAndReload = CalculatePrescalerAndReload(period);

            IWDG->KR = AccessValue;
            IWDG->PR = prescalerAndReload.first;
            IWDG->RLR = prescalerAndReload.second;
            IWDG->KR = ReloadValue;
            IWDG->KR = InitValue;
        }

        /**
         * @brief Start watchdog with given period (template version)
         * 
         * @tparam period Period in milliseconds
         * 
         * @par Returns
         *  Nothing
         */
        template<uint16_t period>
        static void Start()
        {
            auto prescalerAndReload = CalculatePrescalerAndReload(period);

            IWDG->KR = AccessValue;
            IWDG->PR = prescalerAndReload.first;
            IWDG->RLR = prescalerAndReload.second;
            IWDG->KR = ReloadValue;
            IWDG->KR = InitValue;
        }

        /**
         * @brief Set WDG prescaler
         * 
         * @param prescaler Prescaler value
         * 
         * @par Returns
         *  Nothing
         */
        static void SetPrescaler(Prescaler prescaler)
        {
            IWDG->KR = AccessValue;
            IWDG->PR = static_cast<uint8_t>(prescaler);
        }

        /**
         * @brief Set WDG prescaler (template version)
         * 
         * @tparam prescaler Prescaler value
         * 
         * @par Returns
         *  Nothing
         */
        template<Prescaler prescaler>
        static void SetPrescaler()
        {
            IWDG->KR = AccessValue;
            IWDG->PR = static_cast<uint8_t>(prescaler);
        }

        /**
         * @brief Set WDG period
         * 
         * @param period Period value
         * 
         * @par Returns
         *  Nothing
         */
        static void SetPeriod(uint16_t period)
        {
            IWDG->KR = AccessValue;
            IWDG->RLR = period;
            IWDG->KR = ReloadValue;
        }

        /**
         * @brief Set WDG period (template version)
         * 
         * @tparam period Period value
         * 
         * @par Returns
         *  Nothing
         */
        template<uint16_t period>
        static void SetPeriod()
        {
            IWDG->KR = AccessValue;
            IWDG->RLR = period;
            IWDG->KR = ReloadValue;
        }

        /**
         * @brief Reset watchdog counter
         * 
         * @par Returns
         *  Nothing
         */
        static void Reset()
        {
            IWDG->KR = ReloadValue;
        }

    private:
        static constexpr std::pair<uint8_t, uint16_t> CalculatePrescalerAndReload(uint16_t period)
        {
            const unsigned maxReloadValue = (1 << 12) - 1;
            auto reload = (period * ClockFreq) / 1000;
            
            uint8_t prescaler = 0;
            while(reload / (4 << prescaler) > maxReloadValue) {
                ++prescaler;
            }

            reload /= (4 << prescaler);

            if(prescaler > 0b110) {
                prescaler = static_cast<uint8_t>(Prescaler::Div256);
                reload = maxReloadValue;
            }

            return std::make_pair(prescaler, reload);
        }
    };
}

#endif //! ZHELE_WATCHDOG_COMMON_H