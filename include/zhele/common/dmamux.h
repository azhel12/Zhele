/**
 * @file
 * Implement DMAMUX module
 * 
 * @author Alexey Zhelonkin
 * @date 2024
 * @license FreeBSD
 */

#ifndef ZHELE_DMAMUX_COMMON_H
#define ZHELE_DMAMUX_COMMON_H

#include "ioreg.h"

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace Zhele
{
    /**
     * @brief Implements DMAMUX module
     * 
     * @tparam _Regs DMAMUX base regs
    */
    template<typename _BaseRegs, std::convertible_to<uint32_t> _SyncInput, std::convertible_to<uint32_t> _RequestInput>
    class DmaMux
    {
        static constexpr uint32_t ChannelsOffset = 0x00;
        static constexpr uint32_t ChannelStatusOffset = 0x080;
        static constexpr uint32_t RequestGeneratorOffset = 0x100;
        static constexpr uint32_t RequestGeneratorStatusOffset = 0x140;

        IO_STRUCT_WRAPPER((reinterpret_cast<std::byte*>(_BaseRegs::Get()) + ChannelsOffset), ChannelRegs, DMAMUX_Channel_TypeDef);
        IO_STRUCT_WRAPPER((reinterpret_cast<std::byte*>(_BaseRegs::Get()) + ChannelStatusOffset), ChannelStatusReg, DMAMUX_ChannelStatus_TypeDef);
        IO_STRUCT_WRAPPER((reinterpret_cast<std::byte*>(_BaseRegs::Get()) + RequestGeneratorOffset), RequestGeneratorRegs, DMAMUX_RequestGen_TypeDef);
        IO_STRUCT_WRAPPER((reinterpret_cast<std::byte*>(_BaseRegs::Get()) + RequestGeneratorStatusOffset), RequestGeneratorStatusReg, DMAMUX_RequestGenStatus_TypeDef);

    public:
        using RequestInput = _RequestInput;
        using SyncInput = _SyncInput;

        /**
         * @brief Implements DMAMUX channel
         * 
         * @tparam _ChannelNumber Channel number
        */
        template<unsigned _ChannelNumber>
        class Channel
        {
            DECLARE_IO_BITFIELD_WRAPPER(ChannelRegs::Get()[_ChannelNumber].CCR, SyncInputBitfield, DMAMUX_CxCR_SYNC_ID)
            DECLARE_IO_BITFIELD_WRAPPER(ChannelRegs::Get()[_ChannelNumber].CCR, SyncPolarityBitfield, DMAMUX_CxCR_SPOL)
            DECLARE_IO_BITFIELD_WRAPPER(ChannelRegs::Get()[_ChannelNumber].CCR, RequestPolarityBitfield, DMAMUX_CxCR_DMAREQ_ID)
        public:
            /// @brief Synchronization polarity choise
            enum class SyncPolarity
            {
                None = 0b00,
                Rising = 0b01,
                Falling = 0b10,
                Both = 0b11
            };

            /**
             * @brief Select the synchronization input
             * 
             * @param [in] input Input
             * 
             * @par Returns
             *  Nothing
            */
            static void SetSyncInput(SyncInput input);

            /**
             * @brief Returns current synchronization input
             * 
             * @returns Synchronization input
            */
            static SyncInput GetSyncInput();

            /**
             * @brief Set synchronization polarity for channel
             * 
             * @param [in] polarity Target polarity
             * 
             * @par Returns
             *  Nothing
            */
            static void SetSyncPolarity(SyncPolarity polarity);

            /**
             * @brief Returns current synchronization polarity for channel
             * 
             * @returns Synchronization polarity
            */
            static SyncPolarity GetSyncPolarity();

            /**
             * @brief Enables synchronization
             * 
             * @par Returns
             *  Nothing
            */
            static void EnableSynchronization();

            /**
             * @brief Disables synchronization
             * 
             * @par Returns
             *  Nothing
            */
            static void DisableSynchronization();

            /**
             * @brief Set request input
             * 
             * @param [in] input Input value
             * 
             * @par Returns
             *  Nothing
            */
            static void SelectRequestInput(RequestInput input);

            /**
             * @brief Returns current request input
             * 
             * @returns Current request input
            */
            static RequestInput GetRequestInput();
        };
    };
}

#include "impl/dmamux.h"

#endif //! ZHELE_DMAMUX_COMMON_H