/**
 * @file
 * Containing template classes, which implements MCU clock.
 * 
 * @author Konstantin Chizhov
 * @date 2018
 * @license FreeBSD
 */

#ifndef ZHELE_CLOCK_COMMON_H
#define ZHELE_CLOCK_COMMON_H

#include "ioreg.h"

#if !defined  (HSE_VALUE) 
	#warning HSE_VALUE not defined. Will be used HSE_VALUE equals 8 MHZ.
	#define HSE_VALUE    ((uint32_t)8000000)
#endif //! HSE_VALUE

#if !defined  (HSI_VALUE)
	#warning HSI_VALUE not defined. Will be used HSI_VALUE equals 8 MHZ.
	#define HSI_VALUE    ((uint32_t)8000000)
#endif //! HSI_VALUE

namespace Zhele
{
	using ClockFrequenceT = uint32_t;
	namespace Clock
	{
		/**
		 * @brief Base class for clock sources (Hsi, Hse, Pll)
		 */
		class ClockBase
		{
		protected:
			static const uint32_t ClockStartTimeout = 4000;

			/**
			 * @brief Enable clocking
			 * 
			 * @param [in] turnMask Mask
			 * @param [in] waitReadyMask Wait for ready mask
			 */
			static bool EnableClockSource(unsigned turnMask,  unsigned waitReadyMask);

			/**
			 * @brief Disable clocking
			 * 
			 * @param [in] turnMask Mask
			 * @param [in] waitReadyMask Wait for ready mask
			 */
			static bool DisableClockSource(unsigned turnMask,  unsigned waitReadyMask);
		};

		/**
		 * @brief Implements external clock source
		 */
		class HseClock :public ClockBase
		{
		public:
			/**
			 * @brief Returns external oscillator clock frequence
			 * 
			 * @returns External source (HSE) clock frequence
			 */
			static ClockFrequenceT SrcClockFreq()
			{
				return HSE_VALUE;
			}

			/**
			 * @brief Returns HSE divider
			 * 
			 * @returns Hse divider
			 */
			static ClockFrequenceT GetDivider() { return 1; }

			/**
			 * @brief Returns HSE multiplier
			 * 
			 * @returns Hse multiplier
			 */
			static ClockFrequenceT GetMultipler() { return 1; }


			/**
			 * @brief Returns final clock frequence
			 * 
			 * @deail
			 * Final clock frequece = source clock frequence / divider * multiplier
			 * 
			 * @returns Final frequence
			 */
			static ClockFrequenceT ClockFreq()
			{
				return SrcClockFreq();
			}

			/**
			 * @brief Enables Hse source
			 * 
			 * @retval true Successful enable
			 * @retval false Fail enable
			 */
			static bool Enable()
			{
				return ClockBase::EnableClockSource(RCC_CR_HSEON, RCC_CR_HSERDY);
			}

			/**
			 * @brief Disables Hse source
			 * 
			 * @retval true Successful disable
			 * @retval false Fail disable
			 */
			static bool Disable()
			{
				return ClockBase::DisableClockSource(RCC_CR_HSEON, RCC_CR_HSERDY);
			}
		};

		/**
		 * @brief Implements internal clock source
		 */
		class HsiClock :public ClockBase
		{
		public:
			/**
			 * @brief Returns internal oscillator clock frequence
			 * 
			 * @returns External source (HSE) clock frequence
			 */
			static ClockFrequenceT SrcClockFreq()
			{
				return HSI_VALUE;
			}

			/**
			 * @brief Returns HSI divider
			 * 
			 * @returns Hsi divider
			 */
			static ClockFrequenceT GetDivider() { return 1; }

			/**
			 * @brief Returns HSI multiplier
			 * 
			 * @returns Hsi multiplier
			 */
			static ClockFrequenceT GetMultipler() { return 1; }


			/**
			 * @brief Returns final clock frequence
			 * 
			 * @deail
			 * Final clock frequece = source clock frequence / divider * multiplier
			 * 
			 * @returns Final frequence
			 */
			static ClockFrequenceT ClockFreq()
			{
				return SrcClockFreq();
			}

			/**
			 * @brief Enables Hsi source
			 * 
			 * @retval true Successful enable
			 * @retval false Fail enable
			 */
			static bool Enable()
			{
				return ClockBase::EnableClockSource(RCC_CR_HSION, RCC_CR_HSIRDY);
			}

			/**
			 * @brief Disables Hsi source
			 * 
			 * @retval true Successful disable
			 * @retval false Fail disable
			 */
			static bool Disable()
			{
				return ClockBase::DisableClockSource(RCC_CR_HSION, RCC_CR_HSIRDY);
			}
		};

		/**
		 * @brief Implements Pll clock source
		 */
		class PllClock :public ClockBase
		{
		public:
			/**
			 * @brief Clock source for PLL
			 * 
			 * @todo Check this place for new CMSIS (early "RCC_CFGR_PLLSRC_HSI" and "RCC_CFGR_PLLSRC_HSE_DIV2" defines was used)
			 */
			enum ClockSource
			{
				Internal = 0,   //< Use internal oscillator (HSI) divided 2
				External = RCC_CFGR_PLLSRC
			};

			/**
			 * @brief Set 
			 */
			static uint32_t SetClockFreq(uint32_t freq);

			/**
			 * @brief Returns PLL source clock frequence (HSI or HSE)
			 * 
			 * @returns PLL source frequence
			 */
			static ClockFrequenceT SrcClockFreq();

			/**
			 * @brief Returns PLL divider
			 * 
			 * @returns Pll divider
			 */
			static ClockFrequenceT GetDivider();

			/**
			 * @brief Set PLL divider
			 *
			 * @param [in] divider Pll divider value
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetDivider(uint8_t divider);

			/**
			 * @brief Returns PLL multiplier
			 * 
			 * @returns Pll multiplier
			 */
			static ClockFrequenceT GetMultipler();

			/**
			 * @brief Set PLL multiplier
			 * 
			 * @param [in] multiplier Pll Multiplier value
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetMultipler(uint8_t multiplier);		

			/**
			 * @brief Set PLL clock source
			 * 
			 * @param [in] clockSource Source for Pll (HSE or HSI)
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SelectClockSource(ClockSource clockSource);


			/**
			 * @brief Returns result frequence
			 * 
			 * @returns Pll result frequence
			 */
			static ClockFrequenceT ClockFreq();

			/**
			 * @brief Enables PLL as clock source
			 * 
			 * @retval true Success
			 * @retval false Fail
			 */
			static bool Enable();

			/**
			 * @brief Disables PLL as clock source
			 * 
			 * @retval true Success
			 * @retval false Fail
			 */
			static void Disable();
		};

		/**
		 * @brief Implements system clock
		 */
		class SysClock
		{
		public:
			/// Clock source
			enum ClockSource
			{
				Internal = 0,	//< HSI
				External = 1,	//< HSE
				Pll = 2	//< PLL
			};

			/// Error codes for class methods
			enum ErrorCode
			{
				Success = 0,	//< Success (no error)
				ClockSourceFailed = 1,	//< Fail to enable clock source
				InvalidClockSource = 2,	//< Invalid clock source (not HSI, HSE or PLL)
				ClockSelectFailed = 3	//< Fail to select clock source
			};
			
		public:
			/**
			 * @brief Returns max allowed system clock frequence
			 * 
			 * @returns Max clock frequence
			 */
			static ClockFrequenceT MaxFreq();

			/**
			 * Select source for system clock
			 * 
			 * @param [in] clockSource Clock source (Internal, external or PLL)
			 * 
			 * @returns Select result
			 */
			static ErrorCode SelectClockSource(ClockSource clockSource);

			/**
			 * @brief Returns system clock frequence
			 * 
			 * @returns Clock frequence
			 */
			static ClockFrequenceT ClockFreq();


			/**
			 * @brief Returns source clock frequence
			 * 
			 * @details
			 * System clock frequence = source clock frequence
			 * 
			 * @returns Source clock frequence
			 */
			static ClockFrequenceT SrcClockFreq()
			{
				return ClockFreq();
			}

			/**
			 * @brief Set clock frequence
			 * 
			 * @param [in] freq New frequence
			 * 
			 * @returns New clock frequence
			 */
			static ClockFrequenceT SetClockFreq(ClockFrequenceT freq);
		};

		IO_REG_WRAPPER(RCC->AHBENR, AhbClockEnableReg, uint32_t);	
		IO_REG_WRAPPER(RCC->APB1ENR, PeriphClockEnable1, uint32_t);
		IO_REG_WRAPPER(RCC->APB2ENR, PeriphClockEnable2, uint32_t);

		const static unsigned AhbPrescalrBitFieldOffset = 4;
		const static unsigned AhbPrescalrBitFieldLength = 4;
		IO_BITFIELD_WRAPPER(RCC->CFGR, AhbPrescalerBitField, uint32_t, 4, 4);

		const static unsigned Apb1PrescalrBitFieldOffset = 8;
		const static unsigned Apb1PrescalrBitFieldLength = 3;
		IO_BITFIELD_WRAPPER(RCC->CFGR, Apb1PrescalerBitField, uint32_t, 8, 3);

		const static unsigned Apb2PrescalrBitFieldOffset = 11;
		const static unsigned Apb2PrescalrBitFieldLength = 3;
		IO_BITFIELD_WRAPPER(RCC->CFGR, Apb2PrescalerBitField, uint32_t, 11, 3);

		const static unsigned AdcPrescalrBitFieldOffset = 14;
		const static unsigned AdcPrescalrBitFieldLength = 2;		
		IO_BITFIELD_WRAPPER(RCC->CFGR, AdcPrescaller, uint32_t, 14, 2);

		const static unsigned McoPrescalrBitFieldOffset = 24;
		const static unsigned McoPrescalrBitFieldLength = 3;
		IO_BITFIELD_WRAPPER(RCC->CFGR, McoBitField, uint32_t, 24, 3);

		/**
		 * @brief Implements AHB clock
		 */
		class AhbClock
		{
		public:
			// AHB prescaller values
			enum Prescaller
			{
				Div1 = RCC_CFGR_HPRE_DIV1 >> AhbPrescalrBitFieldOffset, //< No divide (prescaler = 1)
				Div2 = RCC_CFGR_HPRE_DIV2 >> AhbPrescalrBitFieldOffset, //< Prescaler = 2
				Div4 = RCC_CFGR_HPRE_DIV4 >> AhbPrescalrBitFieldOffset, //< Prescaler = 4
				Div8 = RCC_CFGR_HPRE_DIV8 >> AhbPrescalrBitFieldOffset, //< Prescaler = 8
				Div16 = RCC_CFGR_HPRE_DIV16 >> AhbPrescalrBitFieldOffset, //< Prescaler = 16
				Div64 = RCC_CFGR_HPRE_DIV64 >> AhbPrescalrBitFieldOffset, //< Prescaler = 64
				Div128 = RCC_CFGR_HPRE_DIV128 >> AhbPrescalrBitFieldOffset, //< Prescaler = 128
				Div256 = RCC_CFGR_HPRE_DIV256 >> AhbPrescalrBitFieldOffset, //< Prescaler = 256
				Div512 = RCC_CFGR_HPRE_DIV512 >> AhbPrescalrBitFieldOffset //< Prescaler = 512
			};

			/**
			 * @brief Returns source clock frequence of AHB
			 * 
			 * @returns Clock frequence
			 */
			static ClockFrequenceT SrcClockFreq()
			{
				return SysClock::ClockFreq();
			}

			/**
			 * @brief Returns current clock frequence of AHB
			 * 
			 * @returns Current frequence
			 */
			static ClockFrequenceT ClockFreq()
			{
				ClockFrequenceT clock = SysClock::ClockFreq();
				uint8_t clockPrescShift[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
				uint8_t shiftBits = clockPrescShift[AhbPrescalerBitField::Get()];
				clock >>= shiftBits;
				return clock;
			}

			/**
			 * @brief Set prescaler for AHB
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetPrescaler(Prescaller prescaler)
			{
				AhbPrescalerBitField::Set(static_cast<ClockFrequenceT>(prescaler));
			}
		};

		/**
		 * @brief Implements APB1 clock
		 */
		class Apb1Clock
		{
		public:
			/**
			 * @brief APB1 clock prescalers
			 */
			enum Prescaler
			{
				Div1 = RCC_CFGR_PPRE1_DIV1 >> Apb1PrescalrBitFieldOffset, //< No divide (prescaler = 1)
				Div2 = RCC_CFGR_PPRE1_DIV2 >> Apb1PrescalrBitFieldOffset, //< Prescaler = 2
				Div4 = RCC_CFGR_PPRE1_DIV4 >> Apb1PrescalrBitFieldOffset, //< Prescaler = 4
				Div8 = RCC_CFGR_PPRE1_DIV8 >> Apb1PrescalrBitFieldOffset, //< Prescaler = 8
				Div16 = RCC_CFGR_PPRE1_DIV16 >> Apb1PrescalrBitFieldOffset, //< Prescaler = 16
			};

			/**
			 * @brief Returns source clock frequence of APB1
			 * 
			 * @returns Clock frequence
			 */
			static ClockFrequenceT SrcClockFreq()
			{
				return AhbClock::ClockFreq();
			}
			
			/**
			 * @brief Returns current clock frequence of APB1
			 * 
			 * @returns Current frequence
			 */
			static ClockFrequenceT ClockFreq()
			{
				ClockFrequenceT clock = AhbClock::ClockFreq();
				uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};
				uint8_t shiftBits = clockPrescShift[Apb1PrescalerBitField::Get()];
				clock >>= shiftBits;
				return clock;
			}
			
			/**
			 * @brief Set prescaler for APB1
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetPrescaler(Prescaler prescaller)
			{
				Apb1PrescalerBitField::Set((uint32_t)prescaller);
			}
		};
		
		/**
		 * @brief Implements APB2 clock
		 */
		class Apb2Clock
		{
		public:
			/**
			 * @brief APB2 clock prescalers
			 */
			enum Prescaller
			{
				Div1 = RCC_CFGR_PPRE2_DIV1 >> Apb2PrescalrBitFieldOffset, //< No divide (prescaler = 1)
				Div2 = RCC_CFGR_PPRE2_DIV2 >> Apb2PrescalrBitFieldOffset, //< Prescaler = 2
				Div4 = RCC_CFGR_PPRE2_DIV4 >> Apb2PrescalrBitFieldOffset, //< Prescaler = 4
				Div8 = RCC_CFGR_PPRE2_DIV8 >> Apb2PrescalrBitFieldOffset, //< Prescaler = 8
				Div16 = RCC_CFGR_PPRE2_DIV16 >> Apb2PrescalrBitFieldOffset, //< Prescaler = 16
			};
			
			/**
			 * @brief Returns source clock frequence of APB2
			 * 
			 * @returns Clock frequence
			 */
			static ClockFrequenceT SrcClockFreq()
			{
				return AhbClock::ClockFreq();
			}
			
			/**
			 * @brief Returns current clock frequence of APB2
			 * 
			 * @returns Current frequence
			 */
			static ClockFrequenceT ClockFreq()
			{
				ClockFrequenceT clock = AhbClock::ClockFreq();
				const uint8_t clockPrescShift[] = {0, 0, 0, 0, 1, 2, 3, 4};
				uint8_t shiftBits = clockPrescShift[Apb2PrescalerBitField::Get()];
				clock >>= shiftBits;
				return clock;
			}
			
			/**
			 * @brief Set prescaler for APB2
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetPrescaler(Prescaller prescaller)
			{
				Apb2PrescalerBitField::Set((uint32_t)prescaller);
			}
		};
		
		/**
		 * @brief Implements ADC clock
		 */
		class AdcClockSource 
		{
		public:
			/**
			 * @brief ADC clock sources
			 */
			enum ClockSource
			{
				Apb2 = 0, //< APB2
			};
			
			/**
			 * @brief ADC prescaler
			 */
			enum Prescaler
			{
				Div2 = RCC_CFGR_ADCPRE_DIV2 >> AdcPrescalrBitFieldOffset, //< Prescaler = 2
				Div4 = RCC_CFGR_ADCPRE_DIV4 >> AdcPrescalrBitFieldOffset, //< Prescaler = 4
				Div6 = RCC_CFGR_ADCPRE_DIV6 >> AdcPrescalrBitFieldOffset, //< Prescaler = 6
				Div8 = RCC_CFGR_ADCPRE_DIV8 >> AdcPrescalrBitFieldOffset, //< Prescaler = 8
			};
			
			/**
			 * @brief Select clock source for ADC (like Belarus presidential election)
			 * 
			 * @retval true Always
			 */
			static bool SelectClockSource(ClockSource)
			{
				return true;
			}
			
			/**
			 * @brief Set prescaler for ADC
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void SetPrescaler(Prescaler prescaller)
			{
				AdcPrescaller::Set((uint32_t)prescaller);
			}
			
			/**
			 * @brief Returns source clock frequence of ADC
			 * 
			 * @returns Clock frequence
			 */
			static ClockFrequenceT SrcClockFreq()
			{
				return Apb2Clock::ClockFreq();
			}
			
			/**
			 * @brief Returns current clock frequence of ADC
			 * 
			 * @returns Current frequence
			 */
			static ClockFrequenceT ClockFreq()
			{
				return SrcClockFreq() / ((AdcPrescaller::Get() + 1) * 2);
			}
		};
		
		/**
		 * @brief Implements clock control
		 * 
		 * @tparam _Reg register address
		 * @tparam _Mask Clock enable mask
		 * @tparam _ClockSrc Clock source
		 */
		template<typename _Reg, unsigned _Mask, typename _ClockSrc>
		class ClockControl :public _ClockSrc
		{
		public:
			/**
			 * @brief Enable clock
			 * 
			 * @par Returns
			 * 	Nothing
			 */
			static void Enable()
			{
				_Reg::Or(_Mask);
			}
			
			/**
			 * @brief Disable clock
			 * 
			 * @par Returns
			 * 	Nothing
			 */
			static void Disable()
			{
				_Reg::And(~_Mask);
			}
		};

		/// Common *ENR clocks for all Stm32 MCU (was get as intersect of *ENR defines of all mcuheaders from CMSIS)
		using CrcClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_CRCEN, AhbClock>;
		using Dma1Clock = ClockControl<AhbClockEnableReg, RCC_AHBENR_DMA1EN, AhbClock>;
		using FlitfClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_FLITFEN, AhbClock>;
		using SramClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_SRAMEN, AhbClock>;

		using BackupClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_BKPEN, Apb1Clock>;
		using I2c1Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C1EN, Apb1Clock>;
		using PowerClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_PWREN, Apb1Clock>;
		using Tim2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM2EN, Apb1Clock>;
		using Tim3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM3EN, Apb1Clock>;
		using Usart2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART2EN, Apb1Clock>;
		using WatchDogClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_WWDGEN, Apb1Clock>;

		using AfioClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_AFIOEN, Apb2Clock>;
		using Adc1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC1EN, AdcClockSource>;
		using PortaClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPAEN, Apb2Clock>;
		using PortbClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPBEN, Apb2Clock>;
		using PortcClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPCEN, Apb2Clock>;
		using PortdClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPDEN, Apb2Clock>;
		using Spi1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_SPI1EN, Apb2Clock>;
		using Tim1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM1EN, Apb2Clock>;
		using Usart1Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_USART1EN, Apb2Clock>;

		/// Specific *ENR clocks
		#if defined (RCC_AHBENR_DMA2EN)
			using Dma2Clock = ClockControl<AhbClockEnableReg, RCC_AHBENR_DMA2EN, AhbClock>;
		#endif
		#if defined (RCC_AHBENR_ETHMACEN)
			using EthernetClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_ETHMACEN, AhbClock>;
		#endif
		#if defined (RCC_AHBENR_ETHMACRXEN)
			using EthernetRxClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_ETHMACRXEN, AhbClock>;
		#endif
		#if defined (RCC_AHBENR_ETHMACTXEN)
			using EthernetTxClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_ETHMACTXEN, AhbClock>;
		#endif
		#if defined (RCC_AHBENR_FSMCEN)
			using FsmcClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_FSMCEN, AhbClock>;
		#endif
		#if defined (RCC_AHBENR_OTGFSEN)
			using OtgFsClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_OTGFSEN, AhbClock>;
		#endif
		#if defined (RCC_AHBENR_SDIOEN)
			using SdioClock = ClockControl<AhbClockEnableReg, RCC_AHBENR_SDIOEN, AhbClock>;
		#endif

		#if defined (RCC_APB1ENR_CAN1EN)
			using Can1Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CAN1EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_CAN2EN)
			using Can2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CAN2EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_CECEN)
			using CecClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_CECEN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_DACEN)
			using DacClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_DACEN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_I2C2EN)
			using I2c2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_I2C2EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_SPI2EN)
			using Spi2Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPI2EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_SPI3EN)
			using Spi3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_SPI3EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_TIM4EN)
			using Tim4Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM4EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_TIM5EN)
			using Tim5Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM5EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_TIM6EN)
			using Tim6Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM6EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_TIM7EN)
			using Tim7Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM7EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_TIM12EN)
			using Tim12Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM12EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_TIM13EN)
			using Tim13Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM13EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_TIM14EN)
			using Tim14Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_TIM14EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_USART3EN)
			using Usart3Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USART3EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_UART4EN)
			using Uart4Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART4EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_UART5EN)
			using Uart5Clock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_UART5EN, Apb1Clock>;
		#endif
		#if defined (RCC_APB1ENR_USBEN)
			using UsbClock = ClockControl<PeriphClockEnable1, RCC_APB1ENR_USBEN, Apb1Clock>;
		#endif

		#if defined (RCC_APB2ENR_ADC2EN)
			using Adc2Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC2EN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_ADC3EN)
			using Adc3Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC3EN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_ADC3EN)
			using Adc3Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_ADC3EN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_IOPEEN)
			using PorteClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPEEN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_IOPFEN)
			using PortfClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPFEN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_IOPGEN)
			using PortgClock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_IOPGEN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_TIM8EN)
			using Tim8Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM8EN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_TIM9EN)
			using Tim9Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM9EN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_TIM10EN)
			using Tim10Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM10EN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_TIM11EN)
			using Tim11Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM11EN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_TIM15EN)
			using Tim15Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM15EN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_TIM16EN)
			using Tim16Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM16EN, Apb2Clock>;
		#endif
		#if defined (RCC_APB2ENR_TIM17EN)
			using Tim17Clock = ClockControl<PeriphClockEnable2, RCC_APB2ENR_TIM17EN, Apb2Clock>;
		#endif
	}
}

#endif //! ZHELE_CLOCK_COMMON_H