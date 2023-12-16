/**
 * @file
 * I2c methods implementation
 * 
 * @author Aleksei Zhelonkin (based on i2c implementation by Konstantin Chizhov)
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_I2C_IMPL_COMMON_H
#define ZHELE_I2C_IMPL_COMMON_H

namespace Zhele
{
    namespace Private
    {
        #define I2C_TEMPLATE_ARGS template< \
            typename _Regs, \
            IRQn_Type _EventIrqNumber, \
            IRQn_Type _ErrorIrqNumber, \
            typename _ClockCtrl, \
            typename _SclPins, \
            typename _SdaPins, \
            typename _DmaTx, \
            typename _DmaRx>

        #define I2C_TEMPLATE_QUALIFIER I2cBase<\
                _Regs, \
                _EventIrqNumber, \
                _ErrorIrqNumber, \
                _ClockCtrl, \
                _SclPins, \
                _SdaPins, \
                _DmaTx, \
                _DmaRx>

        I2C_TEMPLATE_ARGS
        typename I2C_TEMPLATE_QUALIFIER::AsyncTransferData I2C_TEMPLATE_QUALIFIER::_transferData;
    #if defined (I2C_TYPE_1)
    static inline uint32_t CalcTiming (uint32_t sourceClock, uint32_t sclClock)
    {
        uint32_t tClk = 4000000000u / sourceClock;	// 4*nsec
        uint32_t t2Scl = 2000000000u / sclClock;	// 4*nsec
        bool stdMode = sclClock <= 100000;
        bool fstMode = sclClock <= 400000;
        uint32_t riseTime = (stdMode ? 1000 : fstMode ? 300 : 120) * 4;	// 4*nsec
        uint32_t fallTime = (fstMode ? 300 : 120) * 4;	// 4*nsec
        uint32_t tsudat = (stdMode ? 250 : fstMode ? 100 : 50) * 4;	// 4*nsec
        uint32_t tLow = t2Scl - (stdMode ? fallTime : 0) - 3 * tClk;
        uint32_t tHi = t2Scl - (stdMode ? 0 : fallTime) - riseTime - 3 * tClk;
        uint32_t sclh = tLow / tClk;
        uint32_t scll = tHi / tClk;
        uint32_t scldel = tsudat / tClk;

        sclh -= sclh > 0 ? 1 : 0;
        scll -= scll > 0 ? 1 : 0;
        scldel -= scldel > 0 ? 1 : 0;

        uint32_t presc = scll / 256;
        if (presc > 0)
        {
            sclh /= (presc + 1);
            scll /= (presc + 1);
            scldel /= (presc + 1);
        }

        return (scll << I2C_TIMINGR_SCLL_Pos) | (sclh << I2C_TIMINGR_SCLH_Pos) | (scldel << I2C_TIMINGR_SCLDEL_Pos) | (presc << I2C_TIMINGR_PRESC_Pos);
    }

    I2C_TEMPLATE_ARGS
    void I2C_TEMPLATE_QUALIFIER::Init(uint32_t i2cClockSpeed)
    {
        _ClockCtrl::Enable();
        
        _Regs()->CR1 &= ~I2C_CR1_PE;
        while (_Regs()->CR1 & I2C_CR1_PE) {};

        _Regs()->TIMINGR = CalcTiming(_ClockCtrl::ClockFreq(), i2cClockSpeed);
        _Regs()->CR1 |= I2C_CR1_PE;

        while ((_Regs()->CR1 & I2C_CR1_PE) == 0) {};

        _Regs()->OAR1 = 2;
        _Regs()->OAR2 = 0;
    }

    I2C_TEMPLATE_ARGS
    I2cStatus I2C_TEMPLATE_QUALIFIER::WriteU8(uint16_t devAddr, uint16_t regAddr, uint8_t data, I2cOpts opts)
    {
        if(!WaitWhileBusy())
            return GetErorFromEvent(GetLastEvent());
            
        _Regs()->ICR = _Regs()->ISR;

        if(!WriteDevAddrForWrite(devAddr, opts))
            GetErorFromEvent(GetLastEvent());

        if(!HasAnyFlag(opts, I2cOpts::RegAddrNone))
        {
            if(!WriteRegAddr(regAddr, opts))
                GetErorFromEvent(GetLastEvent());
        }

        SetTransferSize(1);
        _Regs()->TXDR = data;
        if(!WaitEvent(TransfertComplete))
            GetErorFromEvent(GetLastEvent());

        return I2cStatus::Success;
    }

    I2C_TEMPLATE_ARGS
    I2cStatus I2C_TEMPLATE_QUALIFIER::Write(uint16_t devAddr, uint16_t regAddr, const uint8_t* data, uint16_t size, I2cOpts opts)
    {
        if(!WaitWhileBusy())
            GetErorFromEvent(GetLastEvent());

        if(!WriteDevAddrForWrite(devAddr, opts))
            GetErorFromEvent(GetLastEvent());

        if(!HasAnyFlag(opts, I2cOpts::RegAddrNone))
        {
            if(!WriteRegAddr(regAddr, opts))
                GetErorFromEvent(GetLastEvent());
        }

        while (size > 255)
        {
            SetTransferSize(255, false);

            for(uint8_t i = 0; i < 254; ++i)
            {
                _Regs()->TXDR = *data;
                if(!WaitEvent(TxInterrupt))
                    GetErorFromEvent(GetLastEvent());
                ++data;
            }
            size -= 255;
            _Regs()->TXDR = *data;
            ++data;

            if(!WaitEvent(TransfertCompleteReload))
                GetErorFromEvent(GetLastEvent());
        }
            
        SetTransferSize(size & 0xff);

        for(uint8_t i = 0; i < size - 1; ++i)
        {
            _Regs()->TXDR = *data;
            if(!WaitEvent(TxInterrupt))
                GetErorFromEvent(GetLastEvent());
            ++data;
        }
        _Regs()->TXDR = *data;
        ++data;

        if(!WaitEvent(TransfertComplete))
            GetErorFromEvent(GetLastEvent());

        return I2cStatus::Success;
    }

    I2C_TEMPLATE_ARGS
    I2cStatus I2C_TEMPLATE_QUALIFIER::WriteAsync(uint16_t devAddr, uint16_t regAddr, const uint8_t* data, uint16_t size, I2cOpts opts, I2cCallback callback)
    {
        if(!WaitWhileBusy())
            return I2cStatus::Busy;
        
        if(!WriteDevAddrForWrite(devAddr, opts))
            return GetErorFromEvent(GetLastEvent());

        if(!HasAnyFlag(opts, I2cOpts::RegAddrNone))
        {
            if(!WriteRegAddr(regAddr, opts))
                return GetErorFromEvent(GetLastEvent());
        }

        _transferData.Buffer = const_cast<uint8_t*>(data);
        _transferData.Size = size;
        _transferData.Callback = callback;

        SetTransferSize(size > 255 ? 255 : size, size <= 255);
        _DmaTx::ClearTransferComplete();
        _Regs()->CR1 |= I2C_CR1_TXDMAEN;
        _DmaTx::SetTransferCallback([](void* buffer, unsigned bytesTransmit, bool success)
        {
            if (!success)
            {
                if(_transferData.Callback != nullptr)
                    _transferData.Callback(GetErorFromEvent(GetLastEvent()));
                return;
            }

            _transferData.Size -= bytesTransmit;
            _transferData.Buffer += bytesTransmit;

            if(!WaitEvent(_transferData.Size > 0 ? TransfertCompleteReload : TransfertComplete))
            {
                if(_transferData.Callback != nullptr)
                    _transferData.Callback(GetErorFromEvent(GetLastEvent()));
                return;
            }

            if(_transferData.Size > 255)
            {
                SetTransferSize(255, false);
                _DmaTx::ClearTransferComplete();
                _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement, _transferData.Buffer, &_Regs()->TXDR, 255);
                return;
            }
            if(_transferData.Size > 0)
            {
                SetTransferSize(_transferData.Size & 0xff);
                _DmaTx::ClearTransferComplete();
                _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement, _transferData.Buffer, &_Regs()->TXDR, _transferData.Size);
            }
            else
            {
                _Regs()->CR1 &= ~I2C_CR1_TXDMAEN;
                if(_transferData.Callback != nullptr)
                    _transferData.Callback(I2cStatus::Success);
            }
        });
        
        _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement, data, &_Regs()->TXDR, (size > 255 ? 255 : size));

        return I2cStatus::Success;
    }

    I2C_TEMPLATE_ARGS
    ReadResult I2C_TEMPLATE_QUALIFIER::ReadU8(uint16_t devAddr, uint16_t regAddr, I2cOpts opts)
    {
        if(!WaitWhileBusy())
            return ReadResult {0, I2cStatus::Busy};

        if(!WriteDevAddrForWrite(devAddr, opts))
            return ReadResult{0, GetErorFromEvent(GetLastEvent())};

        _Regs()->CR2 &= ~(I2C_CR2_AUTOEND | I2C_CR2_RELOAD);
        if(!HasAnyFlag(opts, I2cOpts::RegAddrNone))
        {
            if(!WriteRegAddr(regAddr, opts))
                ReadResult{0, GetErorFromEvent(GetLastEvent())};
        }

        if(!WriteDevAddrForRead(devAddr, opts, 1, false))
            return ReadResult{0, GetErorFromEvent(GetLastEvent())};

        uint8_t readed = _Regs()->RXDR;
        
        if(!WaitEvent(TransfertComplete))
            return ReadResult{0, GetErorFromEvent(GetLastEvent())};

        return ReadResult{readed, I2cStatus::Success};
    }

    I2C_TEMPLATE_ARGS
    I2cStatus I2C_TEMPLATE_QUALIFIER::Read(uint16_t devAddr, uint16_t regAddr, uint8_t* data, uint16_t size, I2cOpts opts)
    {
        if(!WaitWhileBusy())
            return I2cStatus::Busy;

        if(!WriteDevAddrForWrite(devAddr, opts))
            return GetErorFromEvent(GetLastEvent());

        _Regs()->CR2 &= ~(I2C_CR2_AUTOEND | I2C_CR2_RELOAD);
        if(!HasAnyFlag(opts, I2cOpts::RegAddrNone))
        {
            if(!WriteRegAddr(regAddr, opts))
                GetErorFromEvent(GetLastEvent());
        }

        if(!WriteDevAddrForRead(devAddr, opts, size > 255 ? 255 : size, size > 255))
            return GetErorFromEvent(GetLastEvent());
        
        while (size > 255)
        {
            SetTransferSize(255, false);
            for(uint16_t i = 0; i < 255; ++i)
            {
                if(!WaitEvent(RxNotEmpty))
                    return GetErorFromEvent(GetLastEvent());

                *data = static_cast<uint8_t>(_Regs()->RXDR);
                ++data;
            }
            if(!WaitEvent(TransfertCompleteReload))
                return GetErorFromEvent(GetLastEvent());

            size -= 255;
        }
        SetTransferSize(size);
        for(uint16_t i = 0; i < size; ++i)
        {
            if(!WaitEvent(RxNotEmpty))
                return GetErorFromEvent(GetLastEvent());

            *data = static_cast<uint8_t>(_Regs()->RXDR);
            ++data;
        }
        if(!WaitEvent(TransfertComplete))
            return GetErorFromEvent(GetLastEvent());

        size = 0;

        return I2cStatus::Success;
    }

    I2C_TEMPLATE_ARGS
    I2cStatus I2C_TEMPLATE_QUALIFIER::EnableAsyncRead(uint16_t devAddr, uint16_t regAddr, uint8_t* data, uint16_t size, I2cOpts opts, I2cCallback callback)
    {
        if(!WaitWhileBusy())
            return I2cStatus::Busy;

        if(!WriteDevAddrForWrite(devAddr, opts))
            return GetErorFromEvent(GetLastEvent());

        _Regs()->CR2 &= ~(I2C_CR2_AUTOEND | I2C_CR2_RELOAD);
        if(!HasAnyFlag(opts, I2cOpts::RegAddrNone))
        {
            if(!WriteRegAddr(regAddr, opts))
                GetErorFromEvent(GetLastEvent());
        }

        if(!WriteDevAddrForRead(devAddr, opts, size > 255 ? 255 : size, size > 255))
            return GetErorFromEvent(GetLastEvent());

        _transferData.Buffer = data;
        _transferData.Size = size;
        _transferData.Callback = callback;

        _DmaRx::ClearTransferComplete();
        _Regs()->CR1 |= I2C_CR1_RXDMAEN;

        _DmaRx::SetTransferCallback([](void* buffer, unsigned bytesReceived, bool success)
        {
            if (!success)
            {
                if(_transferData.Callback != nullptr)
                    _transferData.Callback(GetErorFromEvent(GetLastEvent()));
                return;
            }

            _transferData.Size -= bytesReceived;
            _transferData.Buffer += bytesReceived;

            if(!WaitEvent(_transferData.Size > 0 ? TransfertCompleteReload : TransfertComplete))
            {
                if(_transferData.Callback != nullptr)
                    _transferData.Callback(GetErorFromEvent(GetLastEvent()));
                return;
            }

            if(_transferData.Size > 255)
            {
                SetTransferSize(255, false);
                _DmaRx::ClearTransferComplete();
                _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::Circular, _transferData.Buffer, &_Regs()->RXDR, 255);
                return;
            }
            if(_transferData.Size > 0)
            {
                SetTransferSize(_transferData.Size & 0xff);
                _DmaRx::ClearTransferComplete();
                _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::Circular, _transferData.Buffer, &_Regs()->RXDR, _transferData.Size);
            }
            else
            {
                _Regs()->CR1 &= ~I2C_CR1_RXDMAEN;
                if(_transferData.Callback != nullptr)
                    _transferData.Callback(I2cStatus::Success);
            }
        });

        _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::Circular, data, &_Regs()->RXDR, (size > 255 ? 255 : size));

        return I2cStatus::Success;
    }

    I2C_TEMPLATE_ARGS
    bool I2C_TEMPLATE_QUALIFIER::WriteDevAddrForWrite(uint16_t devAddr,I2cOpts opts)
    {
        _Regs()->CR2 = (devAddr << 1)
            | (HasAnyFlag(opts, I2cOpts::RegAddr16Bit)
                ? 2
                : HasAnyFlag(opts, I2cOpts::RegAddrNone)
                    ? 0
                    : 1) << I2C_CR2_NBYTES_Pos
            | I2C_CR2_RELOAD
            | I2C_CR2_AUTOEND
            | (HasAnyFlag(opts, I2cOpts::DevAddr10Bit) ? I2C_CR2_ADD10 : 0)
            | I2C_CR2_START;

        return WaitEvent(TxInterrupt);
    }

    I2C_TEMPLATE_ARGS
    bool I2C_TEMPLATE_QUALIFIER::WriteDevAddrForRead(uint16_t devAddr, I2cOpts opts, uint8_t bytesToRead, bool reload)
    {
        _Regs()->CR2 = (devAddr << 1)
            | I2C_CR2_RD_WRN
            | (bytesToRead << I2C_CR2_NBYTES_Pos)
            | (reload ? I2C_CR2_RELOAD : 0)
            | I2C_CR2_AUTOEND
            | (HasAnyFlag(opts, I2cOpts::DevAddr10Bit) ? I2C_CR2_ADD10 : 0)
            | I2C_CR2_START;

        return WaitEvent(RxNotEmpty);
    }

    I2C_TEMPLATE_ARGS
    bool I2C_TEMPLATE_QUALIFIER::WriteRegAddr(uint16_t regAddr, I2cOpts opts)
    {
        if(HasAnyFlag(opts, I2cOpts::RegAddr16Bit))
        {
            _Regs()->TXDR = static_cast<uint8_t>(regAddr);
            if(!WaitEvent(TxInterrupt))
                return false;

            _Regs()->TXDR = static_cast<uint8_t>(regAddr >> 8);
            return WaitEvent(TransfertCompleteReload);
        }
        else
        {
            _Regs()->TXDR = static_cast<uint8_t>(regAddr);
            return WaitEvent(TransfertCompleteReload);
        }
        return true;
    }

    I2C_TEMPLATE_ARGS
    bool I2C_TEMPLATE_QUALIFIER::Busy()
    {
        return (_Regs()->ISR & I2C_ISR_BUSY) > 0;
    }

    I2C_TEMPLATE_ARGS
    uint32_t I2C_TEMPLATE_QUALIFIER::GetLastEvent()
    {
        return _Regs()->ISR;   
    }

    I2C_TEMPLATE_ARGS
    void I2C_TEMPLATE_QUALIFIER::SetTransferSize(uint8_t size, bool isLast)
    {
        _Regs()->CR2 = (_Regs()->CR2 & ~(I2C_CR2_NBYTES | I2C_CR2_RELOAD))
            | (size << I2C_CR2_NBYTES_Pos)
            | (isLast ? 0 : I2C_CR2_RELOAD);
    }
    #endif
    #if defined (I2C_TYPE_2)
        template<typename _Regs>
        static void CalcTiming(uint32_t sourceClock, uint32_t i2cClockSpeed, bool dutyCycle2)
        {
            uint32_t sourceClockInMhz = sourceClock / 1000000;
            uint32_t result = 0;
            uint32_t tmpreg = 0;

            if (i2cClockSpeed <= 100000)
            {
                result = (uint16_t)(sourceClock / (i2cClockSpeed << 1));

                if (result < 0x04)
                {
                    result = 0x04;
                }
                tmpreg |= result;
                _Regs()->TRISE = sourceClockInMhz + 1;
            }
            else
            {
                if (dutyCycle2)
                {
                    result = static_cast<uint16_t>(sourceClock / (i2cClockSpeed * 3));
                }
                else
                {
                    result = static_cast<uint16_t>(sourceClock / (i2cClockSpeed * 25));
                    result |= 0x4000;
                }


                if ((result & I2C_CCR_CCR) == 0)
                {
                    result |= static_cast<uint16_t>(0x0001);
                }
                tmpreg |= static_cast<uint16_t>(result | I2C_CCR_FS);
                _Regs()->TRISE = static_cast<uint16_t>(((sourceClockInMhz * static_cast<uint16_t>(300)) / static_cast<uint16_t>(1000)) + static_cast<uint16_t>(1));  
            }
            _Regs()->CCR = tmpreg;
        }

        I2C_TEMPLATE_ARGS
        void I2C_TEMPLATE_QUALIFIER::Init(uint32_t i2cClockSpeed, bool dutyCycle2)
        {
            _ClockCtrl::Enable();
            
            _Regs()->CR1 = 0;
            while (_Regs()->CR1 & I2C_CR1_PE) {};
            
            uint32_t sourceClock = _ClockCtrl::ClockFreq();
            CalcTiming<_Regs>(sourceClock, i2cClockSpeed, dutyCycle2);

            _Regs()->CR1 |= (I2C_CR1_ACK | I2C_CR1_PE);

            while ((_Regs()->CR1 & I2C_CR1_PE) == 0) {};

            _Regs()->OAR1 = 2;
            _Regs()->OAR2 = 0;

            NVIC_EnableIRQ(_EventIrqNumber);
            if constexpr(_EventIrqNumber != _ErrorIrqNumber)
            {
                NVIC_EnableIRQ(_ErrorIrqNumber);
            }
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::WriteU8(uint16_t devAddr, uint16_t regAddr, uint8_t data, I2cOpts opts)
        {
            _Regs()->SR1 = 0;
            _Regs()->SR2 = 0;

            if(!WaitWhileBusy())
                return GetErorFromEvent(GetLastEvent());

            if(!Start())
                return GetErorFromEvent(GetLastEvent());
            
            if(!WriteDevAddr(devAddr, false, opts))
                return GetErorFromEvent(GetLastEvent());

            if(!HasAnyFlag(opts, I2cOpts::RegAddrNone) )
            {
                if(!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }
            _Regs()->DR = data;

            if(!WaitEvent(Events::ByteTransferFinished | Events::TxEmpty | Events::MasterSlave | Events::BusBusy | Events::TransmitterReceiver))
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CR1 &= ~I2C_CR1_ACK;
            _Regs()->CR1 |= I2C_CR1_STOP;

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::Write(uint16_t devAddr, uint16_t regAddr, const uint8_t *data, uint16_t size, I2cOpts opts)
        {
            _Regs()->SR1 = 0;
            _Regs()->SR2 = 0;

            if(!WaitWhileBusy())
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CR1 |= I2C_CR1_ACK;
            
            if(!Start())
                return GetErorFromEvent(GetLastEvent());
            
           if(!WriteDevAddr(devAddr, false, opts))
                return GetErorFromEvent(GetLastEvent());

            if(!HasAnyFlag(opts, I2cOpts::RegAddrNone) )
            {
                if(!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }
            
            for(uint16_t i = 0; i < size; ++i)
            {
                _Regs()->DR = data[i];

                if(!WaitEvent(Events::ByteTransferFinished | Events::TxEmpty | Events::MasterSlave | Events::BusBusy))
                    return GetErorFromEvent(GetLastEvent());
            }
            
            _Regs()->CR1 &= ~I2C_CR1_ACK;
            _Regs()->CR1 |= I2C_CR1_STOP;

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::WriteAsync(uint16_t devAddr, uint16_t regAddr, const uint8_t *data, uint16_t size, I2cOpts opts, I2cCallback callback)
        {
            _Regs()->SR1 = 0;
            _Regs()->SR2 = 0;

            if(!WaitWhileBusy())
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CR1 |= I2C_CR1_ACK;
            
            if(!Start())
                return GetErorFromEvent(GetLastEvent());
            
           if(!WriteDevAddr(devAddr, false, opts))
                return GetErorFromEvent(GetLastEvent());

            if(!HasAnyFlag(opts, I2cOpts::RegAddrNone) )
            {
                if(!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }
            
            _transferData.Callback = callback;

            _DmaTx::ClearTransferComplete();
            _Regs()->CR2 |= I2C_CR2_DMAEN;

            _DmaTx::SetTransferCallback([](void* buffer, unsigned size, bool success)
            {
                if(!WaitEvent(Events::ByteTransferFinished))
                {
                    if (_transferData.Callback != nullptr)
                    {
                        _transferData.Callback(GetErorFromEvent(GetLastEvent()));
                    }
                }

                _Regs()->CR1 &= ~I2C_CR1_ACK;
                _Regs()->CR1 |= I2C_CR1_STOP;

                if (_transferData.Callback != nullptr)
                {
                    _transferData.Callback(success ? I2cStatus::Success : GetErorFromEvent(GetLastEvent()));
                }
            });

            _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement, data, &_Regs()->DR, size);

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        ReadResult I2C_TEMPLATE_QUALIFIER::ReadU8(uint16_t devAddr, uint16_t regAddr, I2cOpts opts)
        {
            if(!WaitWhileBusy())
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            if(!Start())
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            if(!WriteDevAddr(devAddr, false, opts))
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            if(!HasAnyFlag(opts, I2cOpts::RegAddrNone))
            {
                if(!WriteRegAddr(regAddr, opts))
                    return ReadResult {0, GetErorFromEvent(GetLastEvent())};
            }

            if(!Start())
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            if(!WriteDevAddr(devAddr, true, opts))
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            _Regs()->CR1 = (_Regs()->CR1 & (~I2C_CR1_ACK)) | I2C_CR1_STOP;
            uint8_t readedValue = static_cast<uint8_t>(_Regs()->DR);

            return ReadResult {readedValue, I2cStatus::Success};
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::Read(uint16_t devAddr, uint16_t regAddr, uint8_t *data, uint16_t size, I2cOpts opts)
        {            
            if(!WaitWhileBusy())
                GetErorFromEvent(GetLastEvent());
            
            if(!Start())
                GetErorFromEvent(GetLastEvent());

            if(!WriteDevAddr(devAddr, false, opts))
                GetErorFromEvent(GetLastEvent());

            if(!HasAnyFlag(opts, I2cOpts::RegAddrNone))
            {
                if(!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }
            
            if(!Start())
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CR1 |= I2C_CR1_ACK;

            if(!WriteDevAddr(devAddr, true, opts))
                GetErorFromEvent(GetLastEvent());
            
            for(int i = 0; i < size - 1; ++i)
            {
                if(!WaitEvent(Events::RxNotEmpty | Events::MasterSlave | Events::BusBusy))
                    return GetErorFromEvent(GetLastEvent());

                data[i] = static_cast<uint8_t>(_Regs()->DR);
            }

            _Regs()->CR1 &= ~I2C_CR1_ACK;

            if(!WaitEvent(Events::RxNotEmpty | Events::MasterSlave | Events::BusBusy))
                    return GetErorFromEvent(GetLastEvent());

            data[size - 1] = static_cast<uint8_t>(_Regs()->DR);

            _Regs()->CR1 |= I2C_CR1_STOP;

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::EnableAsyncRead(uint16_t devAddr, uint16_t regAddr, uint8_t *data, uint16_t size, I2cOpts opts, I2cCallback callback)
        {            
            if(!WaitWhileBusy())
                GetErorFromEvent(GetLastEvent());
            
            if(!Start())
                GetErorFromEvent(GetLastEvent());

            if(!WriteDevAddr(devAddr, false, opts))
                GetErorFromEvent(GetLastEvent());

            if(!HasAnyFlag(opts, I2cOpts::RegAddrNone))
            {
                if(!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }
            
            if(!Start())
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CR1 |= I2C_CR1_ACK;

            if(!WriteDevAddr(devAddr, true, opts))
                GetErorFromEvent(GetLastEvent());
            
            _transferData.Callback = callback;

            _DmaRx::ClearTransferComplete();
            _Regs()->CR2 |= I2C_CR2_DMAEN;

            _DmaRx::SetTransferCallback([](void* buffer, unsigned size, bool success)
            {
                _Regs()->CR1 &= ~I2C_CR1_ACK;

                if(!WaitEvent(Events::RxNotEmpty | Events::MasterSlave | Events::BusBusy))
                {
                    if (_transferData.Callback != nullptr)
                    {
                        _transferData.Callback(GetErorFromEvent(GetLastEvent()));
                    }
                }

                static_cast<uint8_t*>(buffer)[size] = static_cast<uint8_t>(_Regs()->DR);

                _Regs()->CR1 |= I2C_CR1_STOP;

                if (_transferData.Callback != nullptr)
                {
                    _transferData.Callback(success ? I2cStatus::Success : GetErorFromEvent(GetLastEvent()));
                }
            });

            _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement | _DmaRx::Circular, data, &_Regs()->DR, size - 1);

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::WriteDevAddr(uint16_t devAddr, bool read, I2cOpts opts)
        {
            _Regs()->DR = (devAddr << 1) | (read ? 1 : 0);
            return WaitEvent(read
                ? Events::RxNotEmpty | Events::MasterSlave | Events::BusBusy
                : Events::AddressSent | Events::TxEmpty | Events::MasterSlave | Events::BusBusy | Events::TransmitterReceiver);
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::WriteRegAddr(uint16_t regAddr, I2cOpts opts)
        {
            if(HasAnyFlag(opts, I2cOpts::RegAddr16Bit))
            {
                _Regs()->DR = static_cast<uint8_t>(regAddr);
                if(!WaitEvent(Events::RxNotEmpty | Events::MasterSlave | Events::BusBusy))
                    return false;
            
                _Regs()->DR = static_cast<uint8_t>(regAddr >> 8);
                return WaitEvent(Events::ByteTransferFinished | Events::TxEmpty | Events::MasterSlave | Events::BusBusy | Events::TransmitterReceiver);
            }
            else
            {
                _Regs()->DR = static_cast<uint8_t>(regAddr);
                return WaitEvent(Events::ByteTransferFinished | Events::TxEmpty | Events::MasterSlave | Events::BusBusy | Events::TransmitterReceiver);
            }
            return true;
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::Start()
        {
            _Regs()->SR1 = 0;
            _Regs()->SR2 = 0;
            _Regs()->CR1 |= I2C_CR1_START;
            return WaitEvent(Events::MasterSlave | Events::BusBusy | Events::StartBit);
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::Busy()
        {
            return (_Regs()->SR2 & I2C_SR2_BUSY) > 0;
        }

        I2C_TEMPLATE_ARGS
        uint32_t I2C_TEMPLATE_QUALIFIER::GetLastEvent()
        {
            return (_Regs()->SR1 | _Regs()->SR2 << 16) & 0x00ffffff;
        }
    #endif
        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::WaitWhileBusy()
        {
            for(uint32_t i = _timeout; i >= 0 && Busy(); --i);

            return !Busy();
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::WaitEvent(uint32_t i2c_event)
        {
            bool result = false;
            auto timer = _timeout;
            uint32_t lastevent;
            do
            {
                lastevent = GetLastEvent();
                result = (lastevent & i2c_event) == i2c_event;
            } while(!result && --timer > 0);

            return result;
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::GetErorFromEvent(uint32_t lastevent)
        {
            if(lastevent & Timeout)
            {
                return I2cStatus::Timeout;
            }
            if(lastevent & Overrun)
            {
                return I2cStatus::Overflow;
            }
            if(lastevent & AckFailure)
            {
                return I2cStatus::Nack;
            }
            if(lastevent & ArbitrationLost)
            {
                return I2cStatus::ArbitrationError;
            }
            if(lastevent & BusError)
            {
                return I2cStatus::BusError;
            }
            return I2cStatus::Timeout;
        }
    }
}
#endif //! ZHELE_I2C_IMPL_COMMON_H