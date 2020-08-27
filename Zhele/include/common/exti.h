/**
 * @file
 * Implements EXTI
 * 
 * @author Alexey Zhelonkin
 * @date 2020
 * @license FreeBSD
 */
#ifndef ZHELE_EXTI_COMMON_H
#define ZHELE_EXTI_COMMON_H

#include <ioports.h>

namespace Zhele
{
    /**
     * @brief Implements EXTI
     * 
     * @tparam _Line EXTI line
     * @tparam _IRQn IRQ number
     */
    template<uint8_t _Line, IRQn_Type _IRQn>
    class Exti
    {
    public:
        enum Trigger
        {
            Rising = 1,
            Falling = 2,
            RisingFalling = Rising | Falling
        };

        /**
         * @brief Init EXTI
         * 
         * @tparam trigger Trigger type
         * @tparam port GPIO port
         * 
         * @par Returns
         *  Nothing
         */
        template<Trigger trigger = Trigger::Rising, typename port = IO::Porta>
        static void Init();

        /**
         * @brief Init EXTI
         * 
         * @param [in] trigger Trigger type
         * @param [in] portId GPIO port (as char: 'A', 'B', ...)
         * 
         * @par Returns
         *  Nothing
         */
        static void Init(Trigger trigger = Trigger::Rising, char portId = 'A');

        /**
         * @brief Select port for this EXTI
         * 
         * @tparam port GPIO port
         * 
         * @par Returns
         *  Nothing
         */
        template<typename port>
        static void SelectPort();

        /**
         * @brief Select port for this EXTI
         * 
         * @param [in] PortID GPIO port (as char: 'A', 'B', ...)
         * 
         * @par Returns
         *  Nothing
         */
        static void SelectPort(uint8_t portID);

        /**
         * @brief Init pin for use it as exti input
         * 
         * @tparam Pin GPIO pin
         * 
         * @param [in] pullMode Pull mode
         * @param [in] driverType Driver type
         * @param [in] speed Speed
         * 
         * @par Returns
         *  Nothing
         */
        template <typename pin>
        static void InitPin(
            typename pin::PullMode pullMode = pin::PullMode::PullDown,
            typename pin::DriverType driverType = pin::DriverType::PushPull,
            typename pin::Speed speed = pin::Speed::Slow);

        /**
         * @brief Enable interrupt for this EXTI line
         * 
         * @par Returns
         *  Nothing
         */
        static void EnableInterrupt();

        /**
         * @brief Disable interrupt for this EXTI line
         * 
         * @par Returns
         *  Nothing
         */
        static void DisableInterrupt();

        /**
         * @brief Clear interrupt flag for this EXTI line
         * 
         * @par Returns
         *  Nothing
         */
        static void ClearInterruptFlag();

    private:
        /**
         * @brief Enable clock for use EXTI.
         * 
         * @par Returns
         *  Nothing
         */
        static void EnableClock();
    };
}

#include "impl/exti.h"

#endif //! ZHELE_EXTI_COMMON_H