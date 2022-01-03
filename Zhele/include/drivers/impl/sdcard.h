/**
 * @file
 * Implements method of scdard class
 * 
 * @author Konstantin Chizhov
 * @date 2012
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_SDCARD_IMPL_H
#define ZHELE_DRIVERS_SDCARD_IMPL_H

namespace Zhele::Drivers
{
    template<typename _SpiModule, typename _CsPin>
    uint16_t SdCard<_SpiModule, _CsPin>::SpiCommand(uint8_t index, uint32_t arg, uint8_t crc)
    {
        _CsPin::Clear();
        //Spi.Read();
        Spi.Write(index | (1 << 6));
        Spi.WriteU32Be(arg);
        Spi.Write(crc | 1);
        uint16_t responce = Spi.IgnoreWhile(1000, 0xff);
        if(index == SendStatus && responce !=0xff)
            responce |= Spi.Read() << 8;
        _CsPin::Set();
        //Spi.Read();
        return responce;
    }


    template<class _SpiModule, class _CsPin>
    bool SdCard<_SpiModule, _CsPin>::CheckStatus()
    {
        return SpiCommand(SendStatus, 0) == 0;
    }

    template<class _SpiModule, class _CsPin>
    SdCardType SdCard<_SpiModule, _CsPin>::Detect()
    {
        _type = SdCardNone;

        _CsPin::SetDirWrite();
        _CsPin::Set();

        for(uint8_t i=0; i < 20; i++)
            Spi.Read();

        if(SpiCommand(GoIdleState, 0, 0x95) > SdR1Idle)
            return _type;

        uint8_t resp;
        uint16_t timeout = 10000;

        // test for SDCv2
        if(SpiCommand(SendIfCond, 0x1aa, 0x87) <= SdR1Idle)
        {
            _CsPin::Clear();
            uint32_t voltage = Spi.ReadU32Le();
            _CsPin::Set();

            // voltage check
            if((voltage & 0xffff0000) == 0xaa010000)
            {
                do
                {
                    if(SpiCommand(AppCmd, 0) > 0x01)
                        continue;
                        
                    resp = SpiCommand(SdSendOpCond, 1ul << 30);
                    delay_ms<50, F_CPU>();
                    
                }while(resp != 0 && --timeout);

                if(resp == 0 && !SpiCommand(ReadOcr, 0))
                {
                    _CsPin::Clear();
                    uint32_t ocr = Spi.ReadU32Le();
                    _CsPin::Set();
                     _type = ocr & 0x40 ? SdhcCard : SdCardV2;
                }
            }
        }
        else
        {
            // try SD_SEND_OP_COND for SDSC
            if(SpiCommand(AppCmd, 0) <= SdR1Idle && (resp = SpiCommand(SdSendOpCond, 0)) <= SdR1Idle)
            {
                while(resp != 0 && --timeout)
                {
                    resp = SpiCommand(SendOpCond, 0);
                    delay_ms<50, F_CPU>();
                }

                if(resp == 0)
                    _type = SdCardV1;
            }
            else //  MMC
            {
                do
                {
                    resp = SpiCommand(SendOpCond, 0);
                    delay_ms<50, F_CPU>();
                }while(resp != 0 && --timeout);

                if(resp == SdR1Idle)
                    _type = SdCardMmc;
            }
        }
        return _type;
    }

    template<class _SpiModule, class _CsPin>
    uint32_t SdCard<_SpiModule, _CsPin>::ReadBlocksCount()
    {
        uint8_t csd[16];
        if(!SpiCommand(SendCsd, 0) && ReadDataBlock(csd, 16))
        {
            if(csd[0] & 0xC0) // SD v2
            {
                uint32_t c_size = (((uint32_t)csd[7] & 0x3F) << 16) | ((uint32_t)csd[8] << 8) | csd[9];
                return (c_size + 1) * 1024u - 1u;
            }else // SD v1
            {
                uint32_t c_size = ((((uint32_t)csd[6] << 16) | ((uint32_t)csd[7] << 8) | csd[8]) & 0x0003FFC0) >> 6;
                uint16_t c_size_mult = ((uint16_t)((csd[9] & 0x03) << 1)) | ((uint16_t)((csd[10] & 0x80) >> 7));
                uint16_t block_len = csd[5] & 0x0F;
                block_len = 1u << (block_len - 9);
                return ((c_size + 1u) * (1u << (c_size_mult + 2u)) * block_len) - 1u;
            }
        }
        return 0;
    }

    template<class _SpiModule, class _CsPin>
    uint32_t SdCard<_SpiModule, _CsPin>::BlocksCount()
    {
        // TODO: cache this value
        return ReadBlocksCount();
    }

    template<class _SpiModule, class _CsPin>
    size_t SdCard<_SpiModule, _CsPin>::BlockSize()
    {
        return 512;
    }

    template<class _SpiModule, class _CsPin>
    bool SdCard<_SpiModule, _CsPin>::WaitWhileBusy()
    {
        _CsPin::Clear();
        return Spi.Ignore(10000u, 0xff) == 0xff;
    }
}

#endif //! ZHELE_DRIVERS_SDCARD_IMPL_H
