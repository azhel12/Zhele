/**
 * @file
 * Base for GPIO ports
 * @author Konstantin Chizhov
 * @date 2012
 * @license FreeBSD
 */

#ifndef ZHELE_IOPORTS_COMMON_H
#define ZHELE_IOPORTS_COMMON_H

#include <clock.h>
#include "ioreg.h"


namespace Zhele
{
    namespace IO
    {
        /**
         * @brief
         * Base class for GPIO port
         */
        class NativePortBase
        {
        public:
            using DataType = uint16_t;
            // Port configuartion
            enum Configuration
            {
                In = 0,
                Out = 1,
                AltFunc = 2,
                Analog = 3
            };

            // Pull mode
            enum PullMode
            {
                NoPullUp = 0,
                PullUp   = 1,
                PullDown = 2
            };

            // Port driver type
            enum DriverType
            {
                PushPull = 0,	//< Push-pull
                OpenDrain = 1	//< Open-drain
            };

            // Port speed
            enum Speed
            {
                Slow = 0, //< Slow (< 2 MHz)
                Medium = 1, //< Medium (< 10 MHz)
                Fast = 2, //< Fast (< 50MHz)
                Fastest = 3 //< Fastest (< 50MHz)
            };

            static constexpr inline unsigned UnpackConfig2bits(unsigned mask, unsigned value, unsigned configuration)
			{
				mask = (mask & 0xff00)     << 8 | (mask & 0x00ff);
				mask = (mask & 0x00f000f0) << 4 | (mask & 0x000f000f);
				mask = (mask & 0x0C0C0C0C) << 2 | (mask & 0x03030303);
				mask = (mask & 0x22222222) << 1 | (mask & 0x11111111);
				return (value & ~(mask * 0x03)) | mask * configuration;
			}
			
			static constexpr inline unsigned UnpackConfig4Bit(unsigned mask, unsigned value, unsigned configuration)
			{
				mask = (mask & 0xf0) << 12 | (mask & 0x0f);
				mask = (mask & 0x000C000C) << 6 | (mask & 0x00030003);
				mask = (mask & 0x02020202) << 3 | (mask & 0x01010101);
				return (value & ~(mask * 0x0f)) | mask * configuration;
			}
        };

        namespace Private
        {
            /**
             * @brief Implement GPIO port
             */
            template<typename _Regs, typename _ClkEnReg, int ID>
            class PortImplementation :public NativePortBase
            {
            public:
                /**
                 * @brief Read output port value
                 * @details
                 * Method returns ODR register value
                 * 
                 * @return Port value
                 */
                static DataType Read()
                {
                    return _Regs()->ODR;
                }

                /**
                 * @brief Send value to port
                 * 
                 * @param [in] value Value to write
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Write(DataType value)
                {
                    _Regs()->ODR = value;
                }

                /**
                 * @brief Clear and set bits
                 * 
                 * @param [in] clearMask [in] Clear mask
                 * @param [in] setMask [in] Set mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void ClearAndSet(DataType clearMask, DataType setMask)
                {
                    Clear(clearMask);
                    Set(setMask);
                }

                /**
                 * @brief Set bits by mask
                 * 
                 * @param [in] value Set mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Set(DataType value)
                {
                    _Regs()->BSRR = value;
                }

                /**
                 * @brief Clear (reset) bits by mask
                 * 
                 * @param [in] value Clear mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Clear(DataType value)
                {
                    _Regs()->BSRR = value << 16;
                }

                /**
                 * @brief Toggle output bits
                 * @details
                 * Reverse (xor) ODR register
                 * 
                 * @param [in] value Toggle mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Toggle(DataType value)
                {
                    _Regs()->ODR ^= value;
                }

                /**
                 * @brief Returns input port value
                 * @details
                 * Read and return IDR register
                 * 
                 * @return IDR register value
                 */
                static DataType PinRead()
                {
                    return _Regs()->IDR;
                }

                /**
                 * @brief Template clone of ClearAndSet function
                 * @details
                 * Template methods may be more effective.
                 * It require less instructions, so requires less ROM, less time for execute
                 * If you know parameters before run-time, you should use template methods.
                 * 
                 * @tparam clearMask Clear mask
                 * @tparam setMask Set mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType clearMask, DataType setMask>
                static void ClearAndSet()
                {
                    Clear<clearMask>();
                    Set<setMask>();
                }

                /**
                 * @brief Template clone of Toggle function
                 * 
                 * @tparam value Toggle mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType value>
                static void Toggle()
                {
                    _Regs()->ODR ^= value;
                }

                /**
                 * @brief Template clone of Set function
                 * 
                 * @tparam value Set mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType value>
                static void Set()
                {
                    _Regs()->BSRR = value;
                }


                /**
                 * @brief Template clone of Clear function
                 * 
                 * @tparam value Clear mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType value>
                static void Clear()
                {
                    _Regs()->BSRR = value << 16;
                }

                /**
                 * @brief Set ping configuration
                 * 
                 * @tparam pin Pin number (0.. 15)
                 * 
                 * @param [in] configuration Pin configuration
                 * 
                 * @par Returns
                 *	Nothings
                 */
                template<unsigned pin>
                static void SetPinConfiguration(Configuration configuration)
                {
					_Regs()->MODER = UnpackConfig2bits(1 << pin, _Regs()->MODER, configuration);
                }

                /**
                 * @brief Set port configuration
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] configuration Selected pins configuration
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void SetConfiguration(DataType mask, Configuration configuration)
                {
                    _Regs()->MODER = UnpackConfig2bits(mask, _Regs()->MODER, configuration);
                }

                /**
                 * @brief Template clone of SetConfiguration function
                 * 
                 * @tparam mask Pin mask
                 * @tparam configuration Selected pins configuration
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType mask, Configuration configuration>
                static void SetConfiguration()
                {
                    _Regs()->MODER = UnpackConfig2bits(mask, _Regs()->MODER, configuration);
                }

                /**
                 * @brief Set port speed
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] speed Selected pins speed
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void SetSpeed(DataType mask, Speed speed)
                {
                    _Regs()->OSPEEDR = UnpackConfig2bits(mask, _Regs()->OSPEEDR, speed);
                }

                /**
                 * @brief Template clone of SetSpeed function
                 * @tparam mask Pin mask
                 * @tparam speed Selected pins speed
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType mask, Speed speed>
                void SetSpeed()
                {
                    _Regs()->OSPEEDR = UnpackConfig2bits(mask, _Regs()->OSPEEDR, speed);
                }

                /**
                 * @brief Set pull smode
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] mode Pull mode for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void SetPullUp(DataType mask, PullMode mode)
                {
                    _Regs()->PUPDR = UnpackConfig2bits(mask, _Regs()->PUPDR, mode);
                }

                /**
                 * @brief Template clone of SetPullUp function.
                 * 
                 * @tparam mask Pin mask
                 * @tparam mode Pull mode for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template <DataType mask, PullMode mode>
                static void SetPullUp()
                {
                    _Regs()->PUPDR = UnpackConfig2bits(mask, _Regs()->PUPDR, mode);
                }

                /**
                 * @brief Set driver type
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] driver Driver type for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void SetDriverType(DataType mask, DriverType driver)
                {
                    _Regs()->OTYPER = (_Regs()->OTYPER & ~mask) | mask * driver;
                }

                /**
                 * @brief Template clone of SetDriverType function
                 * 
                 * @tparam mask Pin mask
                 * @tparam driver Driver type for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template <DataType mask, DriverType driver>
                static void SetDriverType()
                {
                    _Regs()->OTYPER = (_Regs()->OTYPER & ~mask) | mask * driver;
                }

                /**
                 * @brief Set alternate function number
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] number Alternate function number for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void AltFuncNumber(DataType mask, uint8_t number)
                {
                    _Regs()->AFR[0] = UnpackConfig4Bit(mask & 0xff, _Regs()->AFR[0], number);
					_Regs()->AFR[1] = UnpackConfig4Bit((mask >> 8) & 0xff, _Regs()->AFR[1], number);
                }

                /**
                 * @brief Template clone of AltFuncNumber function
                 * 
                 * @tparam mask Pin mask
                 * @tparam number Alternate function number for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType mask, uint8_t number>
                static void AltFuncNumber()
                {
                    _Regs()->AFR[0] = UnpackConfig4Bit(mask & 0xff, _Regs()->AFR[0], number);
					_Regs()->AFR[1] = UnpackConfig4Bit((mask >> 8) & 0xff, _Regs()->AFR[1], number);
                }

                /**
                 * @brief Enable clock for port
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Enable()
                {
                    _ClkEnReg::Enable();
                }

                /**
                 * @brief Disable clock for port
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Disable()
                {
                    _ClkEnReg::Disable();
                }
                enum { Id = ID };
            };
        }
#define MAKE_PORT(REGS, ClkEnReg, className, ID) \
       namespace Private{\
            IO_STRUCT_WRAPPER(REGS, className ## Regs, GPIO_TypeDef);\
        }\
        using className = Private::PortImplementation<\
            Private::className ## Regs, \
            ClkEnReg,\
            ID>; \

        #if defined(GPIOA)
            MAKE_PORT(GPIOA, Zhele::Clock::PortaClock, Porta, 'A')
        #endif

        #if defined(GPIOB)
            MAKE_PORT(GPIOB, Zhele::Clock::PortbClock, Portb, 'B')
        #endif

        #if defined(GPIOC)
            MAKE_PORT(GPIOC, Zhele::Clock::PortcClock, Portc, 'C')
        #endif

        #if defined(GPIOD)
            MAKE_PORT(GPIOD, Clock::PortdClock, Portd, 'D')
        #endif

        #if defined(GPIOE)
            MAKE_PORT(GPIOE, Clock::PorteClock, Porte, 'E')
        #endif

        #if defined(GPIOF)
            MAKE_PORT(GPIOF, Clock::PortfClock, Portf, 'F')
        #endif

        #if defined(GPIOG)
            MAKE_PORT(GPIOG, Clock::PortgClock, Portg, 'G')
        #endif
    }
}
#endif //ZHELE_IOPORTS_COMMON_H
