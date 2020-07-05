/**
 * @file
 * Implement DMA protocol
 * 
 * @author Konstantin Chizhov
 * @date ??
 * @license FreeBSD
 */

#ifndef ZHELE_DMA_COMMON_H
#define ZHELE_DMA_COMMON_H

#include "./template_utils/data_transfer.h"
#include "./macro_utils/enum.h"
#include "ioreg.h"
#include "clock.h"

#include <cstddef>

namespace Zhele
{
	class DmaBase
	{
	public:
		/**
		 * @brief DMA settings
		 */
		enum Mode
		{
			Mem2Mem = DMA_CCR_MEM2MEM, //< Memory to memory transfer
			
			PriorityLow = 0, //< Low priority
			PriorityMedium = DMA_CCR_PL_0, //< Medium priority
			PriorityHigh = DMA_CCR_PL_1, //< High priority
			PriorityVeryHigh = DMA_CCR_PL_1 | DMA_CCR_PL_0, //< Very high priority
			
			MSize8Bits = 0,	//< Memory size 8 bits
			MSize16Bits = DMA_CCR_MSIZE_0, //< Memory size 16 bits
			MSize32Bits = DMA_CCR_MSIZE_1, //< Memory size 32 bits
			
			PSize8Bits = 0,	//< Peripheral size 8 bits
			PSize16Bits = DMA_CCR_PSIZE_0, //< Peripheral size 16 bits
			PSize32Bits = DMA_CCR_PSIZE_1, //< Peripheral size 32 bits
			
			MemIncrement = DMA_CCR_MINC, //< Enable memory increment
			PeriphIncrement = DMA_CCR_PINC,	//< Peripheral increment
			Circular = DMA_CCR_CIRC, //< Circular transfer
			
			Periph2Mem = 0,	//< Peripheral to memory transfer
			Mem2Periph = DMA_CCR_DIR, //< Memory to peripheral transfer
			
			TransferErrorInterrupt = DMA_CCR_TEIE,	//< Transfer error interrupt
			HalfTransferInterrupt = DMA_CCR_HTIE,	//< Half transfer interrupt
			TransferCompleteInterrupt = DMA_CCR_TCIE	//< Transfer complete interrupt
		};
	};

	/**
	 * @brief DMA mode wrapper for support logic operations (AND, OR and more)
	 */
	DECLARE_ENUM_OPERATIONS(DmaBase::Mode)

	/**
	 * @brief DMA channel data
	 */
	struct DmaChannelData
	{
		/**
		 * @brief Default constructor
		 * 
		 * @par Returns
		 *	Nothing
		 */
		DmaChannelData()
			:transferCallback(nullptr),
            data(nullptr),
            size(0)
		{}

		TransferCallback transferCallback; //< Transfer complete/error callback pointer

		void *data;	//< Data buffer
		uint16_t size; //< Data buffer size

		/**
		 * @brief Transfer complete handler. Call user`s callback if it has been set.
		 * 
		 * @par Returns
		 *	Nothing
		 */
		inline void NotifyTransferComplete()
		{
			TransferCallback callback = transferCallback;

			if(callback)
			{
				callback(data, size, true);
			}
		}

		/**
		 * @brief Transfer error handler. Call user`s callback if it has been set.
		 * 
		 * @par Returns
		 *	Nothing
		 */
		inline void NotifyError()
		{
			TransferCallback callback = transferCallback;
			
			if(callback)
			{
				callback(data, size, false);
			}
		}
	};

	/**
	 * @brief Implements DMA channel
	 * 
	 * @tparam _Module Parent DMA module
	 * @tparam _ChannelRegs Pointer to associated DMA_Channel_TypeDef structure
	 * @tparam _Channel Channel number (start with 1)
	 * @tparam _IRQNumber DMA channel IRQ number
	 */
	template<typename _Module, typename _ChannelRegs, unsigned _Channel, IRQn_Type _IRQNumber>
	class DmaChannel : public DmaBase
	{
		static_assert(_Channel <= _Module::Channels);
		static DmaChannelData ChannelData;

	public:
		using DmaBase::Mode;

		/**
		 * @brief Initialize DMA channel
		 * 
		 * @param [in] mode Channel mode (support logic operations, OR ("||") for example)
		 * @param [in] buffer Memory buffer
		 * @param [in] periph Peripheral address (or second memory buffer in Mem2Mem case)
		 * @param [in] bufferSize Memory buffer size
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void Init(Mode mode, const void* buffer, volatile void* periph, uint32_t bufferSize)
		{
			_Module::Enable();
			_ChannelRegs()->CCR = 0;
			_ChannelRegs()->CNDTR = bufferSize;
			_ChannelRegs()->CPAR = reinterpret_cast<uint32_t>(periph);
			_ChannelRegs()->CMAR = reinterpret_cast<uint32_t>(buffer);
			
			ChannelData.data = const_cast<void*>(buffer);
			ChannelData.size = bufferSize;
			if(ChannelData.transferCallback)
				mode = mode | DmaBase::TransferCompleteInterrupt | DmaBase::TransferErrorInterrupt;
			NVIC_EnableIRQ(_IRQNumber);
			
			_ChannelRegs()->CCR = mode | DMA_CCR_EN;
		}

		/**
		 * @brief Set transfer callback function 
		 * 
		 * @par [in] callback Pointer to callback function
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void SetTransferCallback(TransferCallback callback)
		{
			ChannelData.transferCallback = callback;
		}

		/**
		 * @brief Check DMA channel for state
		 * 
		 * @retval true DMA channel enabled
		 * @retval false DMA channel disabled
		 */
		static bool Enabled()
		{
			return _ChannelRegs()->CCR & DMA_CCR_EN;
		}

		/**
		 * @brief Enable DMA channel
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void Enable()
		{
			_ChannelRegs()->CCR |= DMA_CCR_EN;
		}

		/**
		 * @brief Disable DMA channel
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void Disable()
		{
			_ChannelRegs()->CCR &= ~DMA_CCR_EN;
		}

		/**
		 * @brief Returns remaining bytes to transfer
		 * 
		 * @details
		 *	Bytes remaining = initial size (initial CNDTR value) - current CNDTR value
		 *	
		 *	@returns Byte to transfer remaining
		 */
		static uint32_t RemainingTransfers()
		{
			return _ChannelRegs()->CNDTR;
		}

		/**
		 * @brief Returns peripheral address
		 * 
		 * @returns Peripheral address
		 */
		static void* PeriphAddress()
		{
			return reinterpret_cast<void *>(_ChannelRegs()->CPAR);
		}

		/**
		 * @brief Returns memory buffer address
		 * 
		 * @returns Memory buffer address
		 */
		static void* MemAddress()
		{
			return reinterpret_cast<void *>(_ChannelRegs()->CMAR);
		}

		/**
		 * @brief Returns transfer error state
		 * 
		 * @retval true Transfer error was occured (flag is set)
		 * @retval false No transfer error was occured (flag is reset)
		 */
		static bool TransferError()
		{
			return _Module::template TransferError<_Channel>();
		}

		/**
		 * @brief Returns transfer half of data
		 * 
		 * @retval true If half of data was transfered (flag is set)
		 * @retval false If not half of data was transfered (flag is reset)
		 */
		static bool HalfTransfer()
		{
			return _Module::template HalfTrasfer<_Channel>();
		}

		/**
		 * @brief Returns transfer complete
		 * 
		 * @retval true If transfer complete (flag is set)
		 * @retval false If transfer not complete (flag is reset)
		 */
		static bool TransferComplete()
		{
			return _Module::template TransferComplete<_Channel>();
		}

		/**
		 * @brief Returns interrupt state (occured or not)
		 * 
		 * @retval true Interrupt was occured
		 * @retval false Interrupt was not occured
		 */
		static bool Interrupt()
		{
			return _Module::template Interrupt<_Channel>();
		}

		/**
		 * @brief Clear channel flags
		 * 
		 * @details
		 * Clear channel`s flags in IFCR register
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void ClearFlags()
		{
			_Module::template ClearChannelFlags<_Channel>();
		}

		/**
		 * @brief Clear transfer error flag
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void ClearTransferError()
		{
			_Module::template ClearTransferError<_Channel>();
		}

		/**
		 * @brief Clear half transfer complete flag
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void ClearHalfTrasfer()
		{
			_Module::template ClearHalfTrasfer<_Channel>();
		}

		/**
		 * @brief Clear transfer complete flag
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void ClearTransferComplete()
		{
			_Module::template ClearTransferComplete<_Channel>();
		}

		/**
		 * @brief Clear interrupt flag
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void ClearInterrupt()
		{
			_Module::template ClearInterrupt<_Channel>();
		}

		/**
		 * @brief DMA channel IRQ handler
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void IrqHandler()
		{
			if(TransferComplete())
			{
				ClearFlags();
				Disable();
				ChannelData.NotifyTransferComplete();
			}
			if(TransferError())
			{
				ClearFlags();
				Disable();
				ChannelData.NotifyError();
			}
		}
	};

	/**
	 * @brief Implements DMA module
	 * 
	 * @tparam DmaRegs Pointer to associated DMA_TypeDef structure
	 * @tparam Clock Source clock class
	 * @tparam _Channels DMA channels count
	 */
	template<typename _DmaRegs, typename _Clock, unsigned _Channels>
	class DmaModule : public DmaBase
	{
		static_assert(_Channels <= 7);

		/**
		 * @brief Returns channel flag state
		 * 
		 * @tparam ChannelNum Target channel number
		 * @tparam Flag Flag (bit) number
		 * 
		 * @retval true If flag is set
		 * @retval false If flag is reset
		 */
		template<int ChannelNum, int Flag>
		static bool ChannelFlag()
		{
			static_assert(ChannelNum > 0 && ChannelNum <= Channels);
			return _DmaRegs()->ISR & (1 << ((ChannelNum - 1) * 4 + Flag));
		}


		/**
		 * @brief Clear channel flag
		 * 
		 * @tparam ChannelNum Target channel number
		 * @tparam Flag Flag (bit) number
		 * 
		 * @par Returns
		 *	Nothing
		 */
		template<int ChannelNum, int Flag>
		static void ClearChannelFlag()
		{
			static_assert(ChannelNum > 0 && ChannelNum <= Channels);
			_DmaRegs()->IFCR |= (1 << ((ChannelNum - 1) * 4 + Flag));
		}
		
	public:
		static const int Channels = _Channels;		

		/**
		 * @brief Returns transfer error state
		 * 
		 * @tparam ChannelNum Target channel number
		 * 
		 * @retval true Transfer error was occured (flag is set)
		 * @retval false No transfer error was occured (flag is reset)
		 */
		template<int ChannelNum>
		static bool TransferError()
		{
			return ChannelFlag<ChannelNum, 3>();
		}


		/**
		 * @brief Returns transfer half of data
		 * 
		 * @tparam ChannelNum Target channel number
		 * 
		 * @retval true If half of data was transfered (flag is set)
		 * @retval false If not half of data was transfered (flag is reset)
		 */
		template<int ChannelNum>
		static bool HalfTransfer()
		{
			return ChannelFlag<ChannelNum, 2>();
		}

		/**
		 * @brief Returns transfer complete
		 * 
		 * @tparam ChannelNum Target channel number
		 * 
		 * @retval true If transfer complete (flag is set)
		 * @retval false If transfer not complete (flag is reset)
		 */
		template<int ChannelNum>
		static bool TransferComplete()
		{
			return ChannelFlag<ChannelNum, 1>();
		}

		/**
		 * @brief Returns interrupt state (occured or not)
		 * 
		 * @tparam ChannelNum Target channel number
		 * 
		 * @retval true Interrupt was occured
		 * @retval false Interrupt was not occured
		 */
		template<int ChannelNum>
		static bool Interrupt()
		{
			return ChannelFlag<ChannelNum, 0>();
		}

		/**
		 * @brief Clear channel flags
		 * 
		 * @tparam ChannelNum Target channel number
		 * 
		 * @par Returns
		 *	Nothing
		 */
		template<int ChannelNum>
		static void ClearChannelFlags()
		{
			static_assert(ChannelNum > 0 && ChannelNum <= Channels);
			_DmaRegs()->IFCR |= (0x0f << (ChannelNum - 1) * 4);
		}

		/**
		 * @brief Clear transfer error flag
		 * 
		 * @tparam ChannelNum Target channel number
		 * 
		 * @par Returns
		 *	Nothing
		 */
		template<int ChannelNum>
		static void ClearTransferError()
		{
			ClearChannelFlag<ChannelNum, 3>();
		}

		/**
		 * @brief Clear half transfer complete flag
		 * 
		 * @tparam ChannelNum Target channel number
		 * 
		 * @par Returns
		 *	Nothing
		 */
		template<int ChannelNum>
		static void ClearHalfTrasfer()
		{
			ClearChannelFlag<ChannelNum, 2>();
		}

		/**
		 * @brief Clear transfer complete flag
		 * 
		 * @tparam ChannelNum Target channel number
		 * 
		 * @par Returns
		 *	Nothing
		 */
		template<int ChannelNum>
		static void ClearTransferComplete()
		{
			ClearChannelFlag<ChannelNum, 1>();
		}

		/**
		 * @brief Clear interrupt flag
		 * 
		 * @tparam ChannelNum Target channel number
		 * 
		 * @par Returns
		 *	Nothing
		 */
		template<int ChannelNum>
		static void ClearInterrupt()
		{
			ClearChannelFlag<ChannelNum, 0>();
		}

		/**
		 * @brief Enable DMA module clocking
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void Enable()
		{
			_Clock::Enable();
		}

		/**
		 * @brief Disable DMA module clocking
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void Disable()
		{
			_Clock::Disable();
		}
	};
	
	
	namespace Private
	{
		IO_STRUCT_WRAPPER(DMA1, Dma1, DMA_TypeDef);
		
		IO_STRUCT_WRAPPER(DMA1_Channel1, Dma1Channel1, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA1_Channel2, Dma1Channel2, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA1_Channel3, Dma1Channel3, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA1_Channel4, Dma1Channel4, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA1_Channel5, Dma1Channel5, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA1_Channel6, Dma1Channel6, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA1_Channel7, Dma1Channel7, DMA_Channel_TypeDef);	
	}

	using Dma1 = DmaModule<Private::Dma1, Clock::Dma1Clock, 7>;
	using Dma1Channel1 = DmaChannel<Dma1, Private::Dma1Channel1, 1, DMA1_Channel1_IRQn>;
	using Dma1Channel2 = DmaChannel<Dma1, Private::Dma1Channel2, 2, DMA1_Channel2_IRQn>;
	using Dma1Channel3 = DmaChannel<Dma1, Private::Dma1Channel3, 3, DMA1_Channel3_IRQn>;
	using Dma1Channel4 = DmaChannel<Dma1, Private::Dma1Channel4, 4, DMA1_Channel4_IRQn>;
	using Dma1Channel5 = DmaChannel<Dma1, Private::Dma1Channel5, 5, DMA1_Channel5_IRQn>;
	using Dma1Channel6 = DmaChannel<Dma1, Private::Dma1Channel6, 6, DMA1_Channel6_IRQn>;
	using Dma1Channel7 = DmaChannel<Dma1, Private::Dma1Channel7, 7, DMA1_Channel7_IRQn>;

#if defined (RCC_AHBENR_DMA2EN)
	namespace Private
	{
		IO_STRUCT_WRAPPER(DMA2, Dma2, DMA_TypeDef);
		
		IO_STRUCT_WRAPPER(DMA2_Channel1, Dma2Channel1, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA2_Channel2, Dma2Channel2, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA2_Channel3, Dma2Channel3, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA2_Channel4, Dma2Channel4, DMA_Channel_TypeDef);
		IO_STRUCT_WRAPPER(DMA2_Channel5, Dma2Channel5, DMA_Channel_TypeDef);	
	}
	using Dma2 = DmaModule<Private::Dma2, Clock::Dma2Clock, 5>;
	using Dma2Channel1 = DmaChannel<Dma2, Private::Dma2Channel1, 1, DMA2_Channel1_IRQn>;
	using Dma2Channel2 = DmaChannel<Dma2, Private::Dma2Channel2, 2, DMA2_Channel2_IRQn>;
	using Dma2Channel3 = DmaChannel<Dma2, Private::Dma2Channel3, 3, DMA2_Channel3_IRQn>;
	using Dma2Channel4 = DmaChannel<Dma2, Private::Dma2Channel4, 4, DMA2_Channel4_IRQn>;
	using Dma2Channel5 = DmaChannel<Dma2, Private::Dma2Channel5, 5, DMA2_Channel5_IRQn>;
#endif
}

#endif //! ZHELE_DMA_COMMON_H