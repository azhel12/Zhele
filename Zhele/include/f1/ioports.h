/**
 * @file
 * Implement GPIO port for stm32f1 series
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_IOPORTS_H
#define ZHELE_IOPORTS_H

#include <stm32f1xx.h>

#include "../common/template_utils/type_list.h"

#include "clock.h"

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
                Analog = 0,	///< Analog
                In = 0x04,	///< In (read)
                Out = 0x03, ///< Out (write)
                AltFunc = 0x0B ///< Alternate function
            };

            // Pull mode
            enum PullMode
            {
                NoPull = 0,	///< No pull
                PullUp = 0x08,	///< Pull up
                PullDown = 0x18	///< Pull down
            };

            // Port driver type
            enum DriverType
            {
                PushPull = 0,	///< Push-pull
                OpenDrain = 4	///< Open-drain
            };

            // Port speed
            enum Speed
            {
                Slow = 2, ///< Slow (< 2 MHz)
                Medium = 1, ///< Medium (< 10 MHz)
                Fast = 3 ///< Fast (< 50MHz)
            };

            static constexpr unsigned int ConfigurationMask(unsigned mask)
            {
                const unsigned mask1 = (mask & 0xf0) << 12 | (mask & 0x0f);
                const unsigned mask2 = (mask1 & 0x000C000C) << 6 | (mask1 & 0x00030003);
                return (mask2 & 0x02020202) << 3 | (mask2 & 0x01010101);
            }

            static constexpr unsigned UnpackConfig(unsigned mask, unsigned value, unsigned configuration, unsigned configMask)
            {
                mask = ConfigurationMask(mask);
                return (value & ~(mask * configMask)) | mask * configuration;
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
            template<class Config>
            static void SetConfiguration(DataT, Config)
            {}

            template<DataT mask, Configuration>
            static void SetConfiguration()
            {}

            static void SetSpeed(DataT, Speed)
            {}
            
            static void SetPullMode(DataT, PullMode)
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
            template<typename _Regs, typename _ClkEnReg, uint8_t ID>
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
                 * @details
                 * Send in BSRR set mask and clear mask << 16
                 * 
                 * @param [in] clearMask [in] Clear mask
                 * @param [in] setMask [in] Set mask
                 * 
                 * @par Returns
                 *	Nothing
                 */
                static void ClearAndSet(DataType clearMask, DataType setMask)
                {
                    _Regs()->BSRR = (setMask | static_cast<uint32_t>(clearMask << 16));
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
                    _Regs()->BSRR = (static_cast<uint32_t>(value) << 16);
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
                    _Regs()->BSRR = (setMask | static_cast<uint32_t>(clearMask) << 16);
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
                    _Regs()->BSRR = (static_cast<uint32_t>(value) << 16);
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
                    _Regs()->CRL = UnpackConfig(mask, _Regs()->CRL, configuration, 0x0f);
                    _Regs()->CRH = UnpackConfig((mask >> 8), _Regs()->CRH, configuration, 0x0f);
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
                    constexpr unsigned lowMaskPart = ConfigurationMask(mask);
                    constexpr unsigned highMaskPart = ConfigurationMask(mask >> 8);

                    _Regs()->CRL = (_Regs()->CRL & ~(lowMaskPart * 0x0f)) | lowMaskPart * configuration;
                    _Regs()->CRH = (_Regs()->CRH & ~(highMaskPart * 0x0f)) | highMaskPart * configuration;
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
                    _Regs()->CRL = NativePortBase::UnpackConfig(mask, _Regs()->CRL, speed, 0x03);
                    _Regs()->CRH = NativePortBase::UnpackConfig((mask >> 8), _Regs()->CRH, speed, 0x03);
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
                static void SetSpeed()
                {
                    constexpr unsigned lowMaskPart = ConfigurationMask(mask);
                    constexpr unsigned highMaskPart = ConfigurationMask(mask);
                    _Regs()->CRL = (_Regs()->CRL & ~(lowMaskPart * 0x03)) | lowMaskPart * speed;
                    _Regs()->CRH = (_Regs()->CRH & ~(highMaskPart * 0x03)) | highMaskPart * speed;
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
                static void SetPullMode(DataType mask, PullMode mode)
                {
                    _Regs()->CRL = NativePortBase::UnpackConfig(mask, _Regs()->CRL, mode & 0x08, 0x0f);
                    _Regs()->CRH = NativePortBase::UnpackConfig((mask >> 8), _Regs()->CRH, mode & 0x08, 0x0f);
                    
                    if (mode & 0x10) // pulldown
                    {
                        Clear(mask);
                    }
                    else
                    {
                        Set(mask);
                    }
                }

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
                static void SetPullMode()
                {
                    _Regs()->CRL = NativePortBase::UnpackConfig(mask, _Regs()->CRL, mode & 0x08, 0x0f);
                    _Regs()->CRH = NativePortBase::UnpackConfig((mask >> 8), _Regs()->CRH, mode & 0x08, 0x0f);
                    
                    if constexpr (mode & 0x10) // pulldown
                    {
                        Clear(mask);
                    }
                    else
                    {
                        Set(mask);
                    }
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
                    _Regs()->CRL = NativePortBase::UnpackConfig(mask, _Regs()->CRL, driver, 0x04);
                    _Regs()->CRH = NativePortBase::UnpackConfig((mask >> 8), _Regs()->CRH, driver, 0x04);
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
                    constexpr unsigned lowMaskPart = ConfigurationMask(mask);
                    constexpr unsigned highMaskPart = ConfigurationMask(mask);
                    _Regs()->CRL = (_Regs()->CRL & ~(lowMaskPart * 0x04)) | lowMaskPart * driver;
                    _Regs()->CRH = (_Regs()->CRH & ~(highMaskPart * 0x04)) | highMaskPart * driver;
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
                    // Not supported
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

        template<typename... >
        class PortList;

        template<typename... _Ports>
        class PortList<TemplateUtils::TypeList<_Ports...> >
        {
        public:
            static void Enable()
            {
                (_Ports::Enable(), ...);
            }

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

#endif //! ZHELE_IOPORTS_H