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

#include <clock.h>

#include <cstddef>

#define COMMA ,

#if defined (DMA_CCR_EN)
    #define ONLY_FOR_CCR(TEXT) TEXT
#else
    #define ONLY_FOR_CCR(TEXT)
#endif
#if defined (DMA_SxCR_EN)
    #define ONLY_FOR_SXCR(TEXT) TEXT
#else
    #define ONLY_FOR_SXCR(TEXT)
#endif

// Dummy function for fix compiler problem.
// Without this hook compiler does not overload weak irq handlers (not include cpp files with handlers).
// So, I have placed declaration here and empty definition in each cpp file.
// It should be fixed (but I dont know how do it).
void DmaDummy();

namespace Zhele
{
    class DmaBase
    {
    public:
        /**
         * @brief DMA settings
         * 
         * @note There are only two DMA types
         */
        enum Mode
        {
        #if defined (DMA_CCR_EN)
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
            Mem2Mem = DMA_CCR_MEM2MEM, //< Memory to memory transfer
            
            TransferErrorInterrupt = DMA_CCR_TEIE,	//< Transfer error interrupt
            HalfTransferInterrupt = DMA_CCR_HTIE,	//< Half transfer interrupt
            TransferCompleteInterrupt = DMA_CCR_TCIE	//< Transfer complete interrupt
        #endif
        #if defined(DMA_SxCR_EN)
            PriorityLow = 0,
			PriorityMedium = DMA_SxCR_PL_0,
			PriorityHigh = DMA_SxCR_PL_1,
			PriorityVeryHigh = DMA_SxCR_PL_1 | DMA_SxCR_PL_0,
			
			MSize8Bits = 0,
			MSize16Bits = DMA_SxCR_MSIZE_0,
			MSize32Bits = DMA_SxCR_MSIZE_1,
			
			PSize8Bits = 0,
			PSize16Bits = DMA_SxCR_PSIZE_0,
			PSize32Bits = DMA_SxCR_PSIZE_1,
			
			MemIncrement = DMA_SxCR_MINC,
			PeriphIncrement = DMA_SxCR_PINC,
			Circular = DMA_SxCR_CIRC,
			
			Periph2Mem = 0,
			Mem2Periph = DMA_SxCR_DIR_0,
			Mem2Mem = DMA_SxCR_DIR_1,
			
			TransferErrorInterrupt = DMA_SxCR_TEIE,
			HalfTransferInterrupt = DMA_SxCR_HTIE,
			TransferCompleteInterrupt = DMA_SxCR_TCIE,
            DirectModeErrorInterrupt = DMA_SxCR_DMEIE,
        #endif
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
            if(transferCallback)
            {
                transferCallback(data, size, true);
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
            if(transferCallback)
            {
                transferCallback(data, size, false);
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
        static DmaChannelData Data;
    public:
        using DmaBase::Mode;

        /**
         * @brief Initialize DMA channel
         * 
         * @param [in] mode Channel mode (support logic operations, OR ("||") for example)
         * @param [in] buffer Memory buffer
         * @param [in] periph Peripheral address (or second memory buffer in Mem2Mem case)
         * @param [in] bufferSize Memory buffer size
         * @param [in] channel Channel (for DMA with streams)
         * @par Returns
         *	Nothing
         */
        static void Transfer(Mode mode, const void* buffer, volatile void* periph, uint32_t bufferSize
        ONLY_FOR_SXCR(COMMA uint8_t channel = 0))
        {
            _Module::Enable();
            if(!TransferError())
			{
				while(!Ready())
					;
			}
        #if defined (DMA_CCR_EN)
            _ChannelRegs()->CCR = 0;
            _ChannelRegs()->CNDTR = bufferSize;
            _ChannelRegs()->CPAR = reinterpret_cast<uint32_t>(periph);
            _ChannelRegs()->CMAR = reinterpret_cast<uint32_t>(buffer);
        #endif
        #if defined (DMA_SxCR_EN)
            _ChannelRegs()->CR = 0;
			_ChannelRegs()->NDTR = bufferSize;
			_ChannelRegs()->PAR = reinterpret_cast<uint32_t>(periph);
			_ChannelRegs()->M0AR = reinterpret_cast<uint32_t>(buffer);
        #endif
        Data.data = const_cast<void*>(buffer);
        Data.size = bufferSize;

        if(Data.transferCallback)
            mode = mode | DmaBase::TransferCompleteInterrupt | DmaBase::TransferErrorInterrupt;
            
        NVIC_EnableIRQ(_IRQNumber);

        #if defined (DMA_CCR_EN)
            _ChannelRegs()->CCR = mode | DMA_CCR_EN;
        #endif
        #if defined (DMA_SxCR_EN)
            _ChannelRegs()->CR = mode | ((channel & 0x07) << 25) | DMA_SxCR_EN;
        #endif
            DmaDummy();
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
            Data.transferCallback = callback;
        }

        /**
         * @brief Check that DMA ready to transfer data
         * 
         * @retval true DMA ready
         * @retval false DMA not ready
         */
        static bool Ready()
		{
			return RemainingTransfers() == 0 || !Enabled() || TransferComplete();
		}

        /**
         * @brief Check DMA channel for state
         * 
         * @retval true DMA channel enabled
         * @retval false DMA channel disabled
         */
        static bool Enabled()
        {
            return _ChannelRegs()->ONLY_FOR_CCR(CCR)ONLY_FOR_SXCR(CR) & ONLY_FOR_CCR(DMA_CCR_EN)ONLY_FOR_SXCR(DMA_SxCR_EN);
        }

        /**
         * @brief Enable DMA channel
         * 
         * @par Returns
         *	Nothing
         */
        static void Enable()
        {
            _ChannelRegs()->ONLY_FOR_CCR(CCR)ONLY_FOR_SXCR(CR) |= ONLY_FOR_CCR(DMA_CCR_EN)ONLY_FOR_SXCR(DMA_SxCR_EN);
        }

        /**
         * @brief Disable DMA channel
         * 
         * @par Returns
         *	Nothing
         */
        static void Disable()
        {
            _ChannelRegs()->ONLY_FOR_CCR(CCR)ONLY_FOR_SXCR(CR) &= ~ONLY_FOR_CCR(DMA_CCR_EN)ONLY_FOR_SXCR(DMA_SxCR_EN);
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
            return _ChannelRegs()->ONLY_FOR_CCR(CNDTR)ONLY_FOR_SXCR(NDTR);
        }

        /**
         * @brief Returns peripheral address
         * 
         * @returns Peripheral address
         */
        static void* PeriphAddress()
        {
            return reinterpret_cast<void *>(_ChannelRegs()->ONLY_FOR_CCR(CPAR)ONLY_FOR_SXCR(PAR));
        }

        /**
         * @brief Returns memory buffer address
         * 
         * @returns Memory buffer address
         */
        static void* MemAddress()
        {
            return reinterpret_cast<void *>(_ChannelRegs()->ONLY_FOR_CCR(CMAR)ONLY_FOR_SXCR(M0AR));
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
                Data.NotifyTransferComplete();
            }
            if(TransferError())
            {
                ClearFlags();
                Disable();
                Data.NotifyError();
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
        #if defined(DMA_CCR_EN)
            return _DmaRegs()->ISR & (1 << ((ChannelNum - 1) * 4 + Flag));
        #endif
        #if defined(DMA_SxCR_EN)
            if constexpr(ChannelNum <= 1)
            {
                return _DmaRegs()->LISR & (1 << (ChannelNum * 6 + Flag));
            }
            if constexpr(2 <= ChannelNum && ChannelNum <= 3)
            {
                return _DmaRegs()->LISR & (1 << (4 + ChannelNum * 6 + Flag));
            }
            if constexpr(4 <= ChannelNum && ChannelNum <= 5)
            {
                return _DmaRegs()->HISR & (1 << ((ChannelNum - 4) * 6 + Flag));
            }
            if constexpr(6 <= ChannelNum && ChannelNum <= 7)
            {
                return _DmaRegs()->HISR & (1 << (4 + (ChannelNum - 4) * 6 + Flag));
            }
			return false;
        #endif
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
        #if defined(DMA_CCR_EN)
            _DmaRegs()->IFCR |= (static_cast<uint32_t>(1) << ((ChannelNum - 1) * 4 + Flag));
        #endif
        #if defined(DMA_SxCR_EN)
            if constexpr(ChannelNum <= 1)
            {
                _DmaRegs()->LIFCR |= (1 << (ChannelNum * 6 + Flag));
            }
            if constexpr(2 <= ChannelNum && ChannelNum <= 3)
            {
                _DmaRegs()->LIFCR |= (1 << (4 + ChannelNum * 6 + Flag));
            }
            if constexpr(4 <= ChannelNum && ChannelNum <= 5)
            {
                _DmaRegs()->HIFCR |= (1 << ((ChannelNum - 4) * 6 + Flag));
            }
            if constexpr(6 <= ChannelNum && ChannelNum <= 7)
            {
                _DmaRegs()->HIFCR |= (1 << (4 + (ChannelNum - 4) * 6 + Flag));
            }
        #endif
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
            return ChannelFlag<ChannelNum, ONLY_FOR_CCR(3)ONLY_FOR_SXCR((1 << 3))>();
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
            return ChannelFlag<ChannelNum, ONLY_FOR_CCR(2)ONLY_FOR_SXCR(4)>();
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
            return ChannelFlag<ChannelNum, ONLY_FOR_CCR(1)ONLY_FOR_SXCR(5)>();
        }

    #if defined(DMA_SxCR_EN)
        template<int ChannelNum>
		static bool FifoError()
		{
			return ChannelFlag<ChannelNum, (0)>();
		}
		
		template<int ChannelNum>
		static bool DirectError()
		{
			return ChannelFlag<ChannelNum, (2)>();
		}
    #endif

    #if defined(DMA_CCR_EN)
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
    #endif
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
        #if defined(DMA_CCR_EN)
            ClearChannelFlag<ChannelNum, 0xf>();
        #endif
        #if defined(DMA_SxCR_EN)
            ClearChannelFlag<ChannelNum, 0x3d>();
        #endif
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
            ClearChannelFlag<ChannelNum, ONLY_FOR_CCR(2)ONLY_FOR_SXCR(4)>();
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
            ClearChannelFlag<ChannelNum, ONLY_FOR_CCR(1)ONLY_FOR_SXCR(5)>();
        }

    #if defined(DMA_CCR_EN)
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
    #endif
    #if defined(DMA_SxCR_EN)
        template<int ChannelNum>
		static void ClearFifoError()
		{
			ClearChannelFlag<ChannelNum, 0>();
		}
		
		template<int ChannelNum>
		static void ClearDirectError()
		{
			ClearChannelFlag<ChannelNum, 2>();
		}
    #endif
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
}

#endif //! ZHELE_DMA_COMMON_H