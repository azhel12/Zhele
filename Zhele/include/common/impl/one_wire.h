/**
 * @file
 * One-wire methods implementation
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @licence FreeBSD
 */

#ifndef ZHELE_ONE_WIRE_IMPL_COMMON_H
#define ZHELE_ONE_WIRE_IMPL_COMMON_H

namespace Zhele 
{
    template<typename _Usart, typename _Pin>
    void OneWire<_Usart, _Pin>::Init()
    {
        _Usart::Init(9600, _Usart::UsartMode::DataBits8
                        | _Usart::UsartMode::NoneParity
                        | _Usart::UsartMode::RxTxEnable
                        | _Usart::UsartMode::OneStopBit
                        | _Usart::UsartMode::OneSampleBitEnable
                        | _Usart::UsartMode::HalfDuplex);
        
        _Usart::template SelectTxRxPins<_Pin>();
        _Pin::SetDriverType(_Pin::DriverType::OpenDrain);
        _Pin::SetPullMode(_Pin::PullMode::PullUp);
        _Pin::SetSpeed(_Pin::Speed::Fast);
    }

    template<typename _Usart, typename _Pin>
    bool OneWire<_Usart, _Pin>::Reset()
    {
        _Usart::SetBaud(9600);

        volatile bool complete = false;
        uint8_t precenseBit = 0;

        _Usart::EnableAsyncRead(&precenseBit, 1, [&complete](void*, unsigned, bool){complete = true;});
        _Usart::Write(0xf0);
        while (!complete);

        _Usart::SetBaud(115200);

        return precenseBit != 0xf0;
    }

    template<typename _Usart, typename _Pin>
    void OneWire<_Usart, _Pin>::WriteByte(uint8_t byteToWrite)
    {
        uint8_t buffer[8];
        uint8_t dummyBuffer[8];
        ConvertToBits(byteToWrite, buffer);
        volatile bool complete = false;

        // Send byte async, receive because it's half-duplex
        _Usart::EnableAsyncRead(dummyBuffer, 8, [&complete](void*, unsigned, bool){complete = true;});
        _Usart::Write(buffer, 8, true);

        while(!complete);
    }

    template<typename _Usart, typename _Pin>
    uint8_t OneWire<_Usart, _Pin>::ReadByte()
    {
        uint8_t buffer[8];
        volatile bool readComplete = false;            

        _Usart::EnableAsyncRead(buffer, 8,
                        [&readComplete](void* data, unsigned size, bool success){
                            readComplete = true;});
        
        _Usart::Write(_readDummyBuffer, 8, true);

        while (!readComplete){}

        return ConvertToByte(buffer);
    }

    template<typename _Usart, typename _Pin>
    void OneWire<_Usart, _Pin>::ReadBytes(void* data, uint8_t size)
    {
        uint8_t* buffer = reinterpret_cast<uint8_t*>(data);

        for(uint8_t i = 0; i < size; ++i)
        {
            buffer[i] = ReadByte();
        }
    }

    template<typename _Usart, typename _Pin>
    void OneWire<_Usart, _Pin>::MatchRom(const uint8_t rom[8])
    {
        WriteByte(Commands::Match);

        for (uint8_t i = 0; i < 8; ++i) {
            WriteByte(rom[i]);
        }
    }

    template<typename _Usart, typename _Pin>
    void OneWire<_Usart, _Pin>::SkipRom()
    {
        Reset();
        WriteByte(Commands::Skip);
    }

    template<typename _Usart, typename _Pin>
    bool OneWire<_Usart, _Pin>::ReadRom(uint8_t* rom)
    {
        if(!Reset())
            return false;
        WriteByte(Commands::Read);

        for (uint8_t i = 0; i < 8; ++i)
        {
            rom[i] = ReadByte();
        }
        return true;
    }

    template<typename _Usart, typename _Pin>
    uint8_t OneWire<_Usart, _Pin>::ConvertToByte(const uint8_t* bits)
    {
        uint8_t resultByte;
        resultByte = 0;
        for (uint8_t i = 0; i < 8; i++)
        {
            resultByte >>= 1;
            if (bits[i] == 0xff) {
                resultByte |= 0x80;
            }
        }
        return resultByte;
    }

    template<typename _Usart, typename _Pin>
    void OneWire<_Usart, _Pin>::ConvertToBits(uint8_t byte, uint8_t* bits)
    {
        for (uint8_t i = 0; i < 8; ++i)
        {
            bits[i] = (byte & 0x01) > 0
                        ? 0xff
                        : 0x00;

            byte >>= 1;
        }
    }
}
#endif // !ZHELE_ONE_WIRE_IMPL_COMMON_H