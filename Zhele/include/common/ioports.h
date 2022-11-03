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
#include "template_utils/type_list.h"

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
                NoPull = 0,
                PullUp   = 1,
                PullDown = 2
            };

            // Port driver type
            enum DriverType
            {
                PushPull = 0,	///< Push-pull
                OpenDrain = 1	///< Open-drain
            };

            // Port speed
            enum Speed
            {
                Slow = 0, ///< Slow (< 2 MHz)
                Medium = 1, ///< Medium (< 10 MHz)
                Fast = 2, ///< Fast (< 50MHz)
                Fastest = 3 ///< Fastest (< 50MHz)
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

        class NullPort : public NativePortBase
        {
        public:
            typedef uint8_t DataT;
            static void Write(DataT)
            {}
            static void ClearAndSet(DataT, DataT)
            {}
            static DataT Read()
            {
                return 0;
            }
            static void Set(DataT)
            {}
            static void Clear(DataT)
            {}
            static void Toggle(DataT)
            {}
            static DataT PinRead()
            {
                return 0;
            }

            static void Enable()
            {}
            static void Disable()
            {}

            template<DataT clearMask, DataT>
            static void ClearAndSet()
            {}

            template<DataT>
            static void Toggle()
            {}

            template<DataT>
            static void Set()
            {}

            template<DataT>
            static void Clear()
            {}

            template<unsigned pin, class Config>
            static void SetPinConfiguration(Config)
            {}
            template<typename Config>
            static void SetConfiguration(DataT, Config)
            {}

            template<DataT mask, Configuration>
            static void SetConfiguration()
            {}

            static void SetSpeed(DataT, Speed)
            {}
            
            static void SetPullMode(DataT, PullMode)
            {}

            template<DataT mask, DriverType>
            static void SetDriverType()
            {}

            static void SetDriverType(DataT, DriverType)
            {}
            
            static void AltFuncNumber(DataT, uint8_t)
            {}

            enum{Id = '-'};
            enum{Width=sizeof(DataT)*8};
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
                static DataType Read();
                

                /**
                 * @brief Send value to port
                 * 
                 * @param [in] value Value to write
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Write(DataType value);
                

                /**
                 * @brief Clear and set bits
                 * 
                 * @param [in] clearMask [in] Clear mask
                 * @param [in] setMask [in] Set mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void ClearAndSet(DataType clearMask, DataType setMask);
               

                /**
                 * @brief Set bits by mask
                 * 
                 * @param [in] value Set mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Set(DataType value);
                

                /**
                 * @brief Clear (reset) bits by mask
                 * 
                 * @param [in] value Clear mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Clear(DataType value);
                

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
                static void Toggle(DataType value);
                

                /**
                 * @brief Returns input port value
                 * @details
                 * Read and return IDR register
                 * 
                 * @return IDR register value
                 */
                static DataType PinRead();
               

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
                static void ClearAndSet();
             

                /**
                 * @brief Template clone of Toggle function
                 * 
                 * @tparam value Toggle mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType value>
                static void Toggle();
               

                /**
                 * @brief Template clone of Set function
                 * 
                 * @tparam value Set mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType value>
                static void Set();
                

                /**
                 * @brief Template clone of Clear function
                 * 
                 * @tparam value Clear mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType value>
                static void Clear();
                

                /**
                 * @brief Set port configuration
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] configuration Selected pins configuration
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void SetConfiguration(DataType mask, Configuration configuration);
               

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
                static void SetConfiguration();
                

                /**
                 * @brief Set port speed
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] speed Selected pins speed
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void SetSpeed(DataType mask, Speed speed);
                

                /**
                 * @brief Template clone of SetSpeed function
                 * @tparam mask Pin mask
                 * @tparam speed Selected pins speed
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template<DataType mask, Speed speed>
                static void SetSpeed();
               

                /**
                 * @brief Set pull smode
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] mode Pull mode for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void SetPullMode(DataType mask, PullMode mode);
                

                /**
                 * @brief Template clone of SetPullMode function.
                 * 
                 * @tparam mask Pin mask
                 * @tparam mode Pull mode for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                template <DataType mask, PullMode mode>
                static void SetPullMode();
                

                /**
                 * @brief Set driver type
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] driver Driver type for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void SetDriverType(DataType mask, DriverType driver);
                

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
                static void SetDriverType();
                

                /**
                 * @brief Set alternate function number
                 * 
                 * @param [in] mask Pin mask
                 * @param [in] number Alternate function number for selected pins
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void AltFuncNumber(DataType mask, uint8_t number);
               

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
                static void AltFuncNumber();
                

                /**
                 * @brief Enable clock for port
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Enable();
               
                /**
                 * @brief Disable clock for port
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void Disable();
                
                enum { Id = ID };
            };
        }

        /**
         * @brief Implements IO ports list class.
         * 
         * @tparam _Ports IOPorts types.
         */
        template<typename... >
        class PortList;

        template<typename... _Ports>
        class PortList<TemplateUtils::TypeList<_Ports...> >
        {
        public:
            /**
             * @brief Enables all ports.
             * 
             * @par Returns
             *  Nothing
             */
            static void Enable()
            {
                (_Ports::Enable(), ...);
            }

            /**
             * @brief Disables all ports.
             * 
             * @par Returns
             *  Nothing
             */
            static void Disable()
            {
                (_Ports::Disable(), ...);
            }
        };

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

#include "impl/ioports.h"

#endif //ZHELE_IOPORTS_COMMON_H
