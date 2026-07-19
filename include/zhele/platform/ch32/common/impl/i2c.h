/**
 * @file
 * I2c methods implementation for CH32 (WCH I2C peripheral)
 *
 * @author Aleksei Zhelonkin (based on the STM32 i2c implementation)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_IMPL_I2C_H
#define ZHELE_PLATFORM_CH32_COMMON_IMPL_I2C_H

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

        I2C_TEMPLATE_ARGS
        void I2C_TEMPLATE_QUALIFIER::Init(uint32_t i2cClockSpeed, bool dutyCycle2)
        {
            _ClockCtrl::Enable();

            _Regs()->CTLR1 &= ~I2C_CTLR1_PE;
            while (_Regs()->CTLR1 & I2C_CTLR1_PE) {}

            uint32_t sourceClock = _ClockCtrl::ClockFreq();

            // CTLR2.FREQ = source clock in MHz (peripheral needs it to time the bus).
            _Regs()->CTLR2 = (_Regs()->CTLR2 & ~I2C_CTLR2_FREQ)
                           | static_cast<uint16_t>(sourceClock / 1000000U);

            uint16_t ckcfgr = 0;
            if (i2cClockSpeed <= 100000U)
            {
                uint16_t result = static_cast<uint16_t>(sourceClock / (i2cClockSpeed << 1));
                if (result < 0x04)
                    result = 0x04;
                ckcfgr = result;
            }
            else
            {
                uint16_t result;
                if (dutyCycle2)
                {
                    result = static_cast<uint16_t>(sourceClock / (i2cClockSpeed * 3));
                }
                else
                {
                    result = static_cast<uint16_t>(sourceClock / (i2cClockSpeed * 25));
                    result |= I2C_CKCFGR_DUTY;
                }
                if ((result & I2C_CKCFGR_CCR) == 0)
                    result |= 0x0001;
                ckcfgr = static_cast<uint16_t>(result | I2C_CKCFGR_FS);
            }
            _Regs()->CKCFGR = ckcfgr;

            _Regs()->CTLR1 |= (I2C_CTLR1_ACK | I2C_CTLR1_PE);
            while ((_Regs()->CTLR1 & I2C_CTLR1_PE) == 0) {}

            _Regs()->OADDR1 = 0x4000; // bit 14 must stay 1 on this peripheral
            _Regs()->OADDR2 = 0;
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::WriteU8(uint16_t devAddr, uint16_t regAddr, uint8_t data, I2cOpts opts)
        {
            if (!WaitWhileBusy())
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CTLR1 |= I2C_CTLR1_ACK;

            if (!Start())
                return GetErorFromEvent(GetLastEvent());

            if (!WriteDevAddr(devAddr, false, opts))
                return GetErorFromEvent(GetLastEvent());

            if (!HasAnyFlag(opts, I2cOpts::RegAddrNone))
            {
                if (!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }

            _Regs()->DATAR = data;

            if (!WaitEvent(Events::ByteTransferFinished | Events::TxEmpty | Events::MasterSlave | Events::BusBusy | Events::TransmitterReceiver))
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CTLR1 &= ~I2C_CTLR1_ACK;
            _Regs()->CTLR1 |= I2C_CTLR1_STOP;

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::Write(uint16_t devAddr, uint16_t regAddr, const uint8_t* data, uint16_t size, I2cOpts opts)
        {
            if (!WaitWhileBusy())
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CTLR1 |= I2C_CTLR1_ACK;

            if (!Start())
                return GetErorFromEvent(GetLastEvent());

            if (!WriteDevAddr(devAddr, false, opts))
                return GetErorFromEvent(GetLastEvent());

            if (!HasAnyFlag(opts, I2cOpts::RegAddrNone))
            {
                if (!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }

            for (uint16_t i = 0; i < size; ++i)
            {
                _Regs()->DATAR = data[i];
                if (!WaitEvent(Events::ByteTransferFinished | Events::TxEmpty | Events::MasterSlave | Events::BusBusy))
                    return GetErorFromEvent(GetLastEvent());
            }

            _Regs()->CTLR1 &= ~I2C_CTLR1_ACK;
            _Regs()->CTLR1 |= I2C_CTLR1_STOP;

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::WriteAsync(uint16_t devAddr, uint16_t regAddr, const uint8_t* data, uint16_t size, I2cOpts opts, I2cCallback callback)
        {
            if (!WaitWhileBusy())
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CTLR1 |= I2C_CTLR1_ACK;

            if (!Start())
                return GetErorFromEvent(GetLastEvent());

            if (!WriteDevAddr(devAddr, false, opts))
                return GetErorFromEvent(GetLastEvent());

            if (!HasAnyFlag(opts, I2cOpts::RegAddrNone))
            {
                if (!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }

            _transferData.Callback = callback;

            _DmaTx::ClearTransferComplete();
            _Regs()->CTLR2 |= I2C_CTLR2_DMAEN;

            _DmaTx::SetTransferCallback([](void* /*buffer*/, unsigned /*size*/, bool success)
            {
                if (!WaitEvent(Events::ByteTransferFinished))
                {
                    if (_transferData.Callback != nullptr)
                        _transferData.Callback(GetErorFromEvent(GetLastEvent()));
                }

                _Regs()->CTLR2 &= ~I2C_CTLR2_DMAEN;
                _Regs()->CTLR1 &= ~I2C_CTLR1_ACK;
                _Regs()->CTLR1 |= I2C_CTLR1_STOP;

                if (_transferData.Callback != nullptr)
                    _transferData.Callback(success ? I2cStatus::Success : GetErorFromEvent(GetLastEvent()));
            });

            _DmaTx::Transfer(_DmaTx::Mem2Periph | _DmaTx::MemIncrement, data, &_Regs()->DATAR, size);

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        ReadResult I2C_TEMPLATE_QUALIFIER::ReadU8(uint16_t devAddr, uint16_t regAddr, I2cOpts opts)
        {
            if (!WaitWhileBusy())
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            if (!Start())
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            if (!WriteDevAddr(devAddr, false, opts))
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            if (!HasAnyFlag(opts, I2cOpts::RegAddrNone))
            {
                if (!WriteRegAddr(regAddr, opts))
                    return ReadResult {0, GetErorFromEvent(GetLastEvent())};
            }

            if (!Start())
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            if (!WriteDevAddr(devAddr, true, opts))
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            _Regs()->CTLR1 = (_Regs()->CTLR1 & ~I2C_CTLR1_ACK) | I2C_CTLR1_STOP;

            if (!WaitEvent(Events::RxNotEmpty | Events::MasterSlave | Events::BusBusy))
                return ReadResult {0, GetErorFromEvent(GetLastEvent())};

            uint8_t value = static_cast<uint8_t>(_Regs()->DATAR);
            return ReadResult {value, I2cStatus::Success};
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::Read(uint16_t devAddr, uint16_t regAddr, uint8_t* data, uint16_t size, I2cOpts opts)
        {
            if (!WaitWhileBusy())
                return GetErorFromEvent(GetLastEvent());

            if (!Start())
                return GetErorFromEvent(GetLastEvent());

            if (!WriteDevAddr(devAddr, false, opts))
                return GetErorFromEvent(GetLastEvent());

            if (!HasAnyFlag(opts, I2cOpts::RegAddrNone))
            {
                if (!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }

            if (!Start())
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CTLR1 |= I2C_CTLR1_ACK;

            if (!WriteDevAddr(devAddr, true, opts))
                return GetErorFromEvent(GetLastEvent());

            for (uint16_t i = 0; i + 1 < size; ++i)
            {
                if (!WaitEvent(Events::RxNotEmpty | Events::MasterSlave | Events::BusBusy))
                    return GetErorFromEvent(GetLastEvent());
                data[i] = static_cast<uint8_t>(_Regs()->DATAR);
            }

            _Regs()->CTLR1 &= ~I2C_CTLR1_ACK;

            if (!WaitEvent(Events::RxNotEmpty | Events::MasterSlave | Events::BusBusy))
                return GetErorFromEvent(GetLastEvent());

            data[size - 1] = static_cast<uint8_t>(_Regs()->DATAR);
            _Regs()->CTLR1 |= I2C_CTLR1_STOP;

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::EnableAsyncRead(uint16_t devAddr, uint16_t regAddr, uint8_t* data, uint16_t size, I2cOpts opts, I2cCallback callback)
        {
            if (!WaitWhileBusy())
                return GetErorFromEvent(GetLastEvent());

            if (!Start())
                return GetErorFromEvent(GetLastEvent());

            if (!WriteDevAddr(devAddr, false, opts))
                return GetErorFromEvent(GetLastEvent());

            if (!HasAnyFlag(opts, I2cOpts::RegAddrNone))
            {
                if (!WriteRegAddr(regAddr, opts))
                    return GetErorFromEvent(GetLastEvent());
            }

            if (!Start())
                return GetErorFromEvent(GetLastEvent());

            _Regs()->CTLR1 |= I2C_CTLR1_ACK;

            if (!WriteDevAddr(devAddr, true, opts))
                return GetErorFromEvent(GetLastEvent());

            _transferData.Callback = callback;

            _DmaRx::ClearTransferComplete();
            // LAST signals the DMA controller to send NACK on the final byte.
            _Regs()->CTLR2 |= I2C_CTLR2_DMAEN | I2C_CTLR2_LAST;

            _DmaRx::SetTransferCallback([](void* /*buffer*/, unsigned /*size*/, bool success)
            {
                _Regs()->CTLR2 &= ~(I2C_CTLR2_DMAEN | I2C_CTLR2_LAST);
                _Regs()->CTLR1 &= ~I2C_CTLR1_ACK;
                _Regs()->CTLR1 |= I2C_CTLR1_STOP;

                if (_transferData.Callback != nullptr)
                    _transferData.Callback(success ? I2cStatus::Success : GetErorFromEvent(GetLastEvent()));
            });

            _DmaRx::Transfer(_DmaRx::Periph2Mem | _DmaRx::MemIncrement, data, &_Regs()->DATAR, size);

            return I2cStatus::Success;
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::WriteRegAddr(uint16_t regAddr, I2cOpts opts)
        {
            if (HasAnyFlag(opts, I2cOpts::RegAddr16Bit))
            {
                _Regs()->DATAR = static_cast<uint8_t>(regAddr >> 8);
                if (!WaitEvent(Events::ByteTransferFinished | Events::TxEmpty | Events::MasterSlave | Events::BusBusy | Events::TransmitterReceiver))
                    return false;
            }
            _Regs()->DATAR = static_cast<uint8_t>(regAddr);
            return WaitEvent(Events::ByteTransferFinished | Events::TxEmpty | Events::MasterSlave | Events::BusBusy | Events::TransmitterReceiver);
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::Busy()
        {
            return (_Regs()->STAR2 & I2C_STAR2_BUSY) > 0;
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::WaitWhileBusy()
        {
            uint32_t timer = _timeout;
            while (Busy() && --timer > 0) {}
            return !Busy();
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::WaitEvent(uint32_t i2cEvent)
        {
            uint32_t timer = _timeout;
            uint32_t lastEvent;
            bool result = false;
            do
            {
                lastEvent = GetLastEvent();
                result = (lastEvent & i2cEvent) == i2cEvent;
            } while (!result && --timer > 0);
            return result;
        }

        I2C_TEMPLATE_ARGS
        I2cStatus I2C_TEMPLATE_QUALIFIER::GetErorFromEvent(uint32_t lastEvent)
        {
            if (lastEvent & Overrun)
                return I2cStatus::Overflow;
            if (lastEvent & AckFailure)
                return I2cStatus::Nack;
            if (lastEvent & ArbitrationLost)
                return I2cStatus::ArbitrationError;
            if (lastEvent & BusError)
                return I2cStatus::BusError;
            return I2cStatus::Timeout;
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::Start()
        {
            _Regs()->STAR1 = 0;
            _Regs()->CTLR1 |= I2C_CTLR1_START;
            return WaitEvent(Events::MasterSlave | Events::BusBusy | Events::StartBit);
        }

        I2C_TEMPLATE_ARGS
        bool I2C_TEMPLATE_QUALIFIER::WriteDevAddr(uint16_t devAddr, bool read, I2cOpts /*opts*/)
        {
            _Regs()->DATAR = static_cast<uint8_t>((devAddr << 1) | (read ? 1 : 0));
            return WaitEvent(read
                ? Events::AddressSent | Events::MasterSlave | Events::BusBusy
                : Events::AddressSent | Events::TxEmpty | Events::MasterSlave | Events::BusBusy | Events::TransmitterReceiver);
        }

        I2C_TEMPLATE_ARGS
        uint32_t I2C_TEMPLATE_QUALIFIER::GetLastEvent()
        {
            return (_Regs()->STAR1 | (_Regs()->STAR2 << 16)) & 0x00ffffff;
        }

        #undef I2C_TEMPLATE_ARGS
        #undef I2C_TEMPLATE_QUALIFIER
    }
}

#endif // ZHELE_PLATFORM_CH32_COMMON_IMPL_I2C_H
