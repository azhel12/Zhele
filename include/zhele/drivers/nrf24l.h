/**
 * @file
 * Contains class for NRF24L transeiver.
 * 
 * @author Aleksei Zhelonkin (based on Tilen Majerle library)
 * @date 2021
 * @licence FreeBSD
 */

#ifndef ZHELE_DRIVERS_NRF24L_H
#define ZHELE_DRIVERS_NRF24L_H

namespace Zhele::Drivers
{	
    /**
     * @brief Base class for Nrf24l modules. Contains enums, constants, etc.
     */
    class Nrf24lBase
    {
    public:
        /**
         * @brief NRF24L registers
         */
        enum class Registers : uint8_t
        {
            Configuration = 0x00, ///< Configuration register
            EnableAutoAcknowledgment = 0x01, ///< Enable "Auto Acknowledgment" Function
            EnableRxAddresses = 0x02, ///< Enable RX Addresses
            AddressWidth = 0x03, ///< Address width settings
            AutoRetransmission = 0x04, ///< Automatic retransmission settings
            RfChannel = 0x05, ///< RF channel
            RfSetup = 0x06, ///< RF settings
            Status = 0x07, ///< Status register
            ObserveTx = 0x08, ///< Transmit observe
            CarrierDetect = 0x09, ///< CarrierDetect
            RxAddress0 = 0x0a, ///< Receive address data pipe 0. 5 Bytes maximum length
            RxAddress1 = 0x0b, ///< Receive address data pipe 1. 5 Bytes maximum length
            RxAddress2 = 0x0c, ///< Receive address data pipe 2. 5 Only LSB
            RxAddress3 = 0x0d, ///< Receive address data pipe 3. 5 Only LSB
            RxAddress4 = 0x0e, ///< Receive address data pipe 4. 5 Only LSB
            RxAddress5 = 0x0f, ///< Receive address data pipe 5. 5 Only LSB
            TxAddress = 0x10, ///< Transmit address. Used for PTX device
            RxPayload0 = 0x11, ///< Number of bytes in RX payload id data pipe 0 (0 - pipe no used, 1..32 - data size in bytes)
            RxPayload1 = 0x12, ///< Number of bytes in RX payload id data pipe 1 (0 - pipe no used, 1..32 - data size in bytes)
            RxPayload2 = 0x13, ///< Number of bytes in RX payload id data pipe 2 (0 - pipe no used, 1..32 - data size in bytes)
            RxPayload3 = 0x14, ///< Number of bytes in RX payload id data pipe 3 (0 - pipe no used, 1..32 - data size in bytes)
            RxPayload4 = 0x15, ///< Number of bytes in RX payload id data pipe 4 (0 - pipe no used, 1..32 - data size in bytes)
            RxPayload5 = 0x16, ///< Number of bytes in RX payload id data pipe 5 (0 - pipe no used, 1..32 - data size in bytes)
            FifoStatus = 0x17, ///< FIFO status register
            DynamicPayload = 0x1c, ///< Dynamic payload setting
            Feature = 0x1d ///< Reserved
        };

        /**
         * @brief Default values for registers (need for init/reset)
         */
        enum RegistersDefault : uint8_t
        {
            Configuration            = 0x08,
            EnableAutoAcknowledgment = 0x3f,
            EnableRxAddresses        = 0x03,
            AddressWidth             = 0x03,
            AutoRetransmission       = 0x03,
            RfChannel                = 0x02,
            RfSetup                  = 0x0e,
            Status                   = 0x0e,
            ObserveTx                = 0x00,
            CarrierDetect            = 0x00,
            RxAddress0               = 0xe7,
            RxAddress1               = 0xc2,
            RxAddress2               = 0xc3,
            RxAddress3               = 0xc4,
            RxAddress4               = 0xc5,
            RxAddress5               = 0xc6,
            TxAddress                = 0xe7,
            RxPayload0               = 0x00,
            RxPayload1               = 0x00,
            RxPayload2               = 0x00,
            RxPayload3               = 0x00,
            RxPayload4               = 0x00,
            RxPayload5               = 0x00,
            FifoStatus               = 0x11,
            DynamicPayload           = 0x00,
            Feature                  = 0x00
        };

        /**
         * @brief Configuration register wrapper
         */
        enum ConfigurationRegister : uint8_t
        {
            /// RX/TX control
            PrimRxPos = 0,
            PrimRxMask = 1 << PrimRxPos,
            PRX = PrimRxMask, ///< Receive
            PTX = 0, ///< Transmit

            /// Power up
            PowerUpPos = 1,
            PowerUpMask = 1 << PowerUpPos,
            PowerUp = PowerUpMask, ///< Power up
            PowerDown = 0, ///< Power down

            /// CRC encoding scheme.
            CRCOPos = 2,
            CRCOMask = 1 << CRCOPos,
            CRCScheme2Bytes = CRCOMask, ///< 2-bytes CRC
            CRCScheme1Byte = 0, ///< 1-byte CRC

            /// Enable CRC
            EnableCRCPos = 3,
            EnableCRCMask = 1 << EnableCRCPos,
            EnableCRC = EnableCRCMask, ///< Enable CRC
            DisableCRC = 0, ///< Disable CRC

            /// MAX_RT (maximum retransmit count) interrupt setup
            MaxRtInterruptPos = 4,
            MaxRtInterruptMask = 1 << MaxRtInterruptPos,
            MaxRtInterruptDisable = MaxRtInterruptMask, ///< Max retransmission count interrupt enable
            MaxRtInterruptEnable = 0, ///< Max retransmission count interrupt disable

            /// TX_DS (ACK packet is received, no payload) interrupt setup
            TxDsInterruptPos = 5,
            TxDsInterruptMask = 1 << TxDsInterruptPos,
            TxDsInterruptDisable = TxDsInterruptMask,
            TxDsInterruptEnable = 0,
            
            /// RX_DR interrupt (ACK packet with payload) interrupt setup
            RxDrInterruptPos = 6,
            RxDrInterruptMask = 1 << RxDrInterruptPos,
            RxDrInterruptDisable = RxDrInterruptMask,	
            RxDrInterruptEnable = 0,
        };

        /**
         * @brief EN_AA register wrapper
         */
        enum EnableAutoAcknowledgmentRegister : uint8_t
        {
            EnableAutoAcknowledgmentPipe0Pos = 0,
            EnableAutoAcknowledgmentPipe0Mask = 1 << EnableAutoAcknowledgmentPipe0Pos,
            EnableAutoAcknowledgmentPipe0 = EnableAutoAcknowledgmentPipe0Mask, 	///< Enable auto acknowledgment data pipe 0
            DisableAutoAcknowledgmentPipe0 = 0, ///< Disable auto acknowledgment data pipe 0

            EnableAutoAcknowledgmentPipe1Pos = 0,
            EnableAutoAcknowledgmentPipe1Mask = 1 << EnableAutoAcknowledgmentPipe1Pos,
            EnableAutoAcknowledgmentPipe1 = EnableAutoAcknowledgmentPipe1Mask, 	///< Enable auto acknowledgment data pipe 1
            DisableAutoAcknowledgmentPipe1 = 0, ///< Disable auto acknowledgment data pipe 1

            EnableAutoAcknowledgmentPipe2Pos = 0,
            EnableAutoAcknowledgmentPipe2Mask = 1 << EnableAutoAcknowledgmentPipe2Pos,
            EnableAutoAcknowledgmentPipe2 = EnableAutoAcknowledgmentPipe2Mask, 	///< Enable auto acknowledgment data pipe 2
            DisableAutoAcknowledgmentPipe2 = 0, ///< Disable auto acknowledgment data pipe 2

            EnableAutoAcknowledgmentPipe3Pos = 0,
            EnableAutoAcknowledgmentPipe3Mask = 1 << EnableAutoAcknowledgmentPipe3Pos,
            EnableAutoAcknowledgmentPipe3 = EnableAutoAcknowledgmentPipe3Mask, 	///< Enable auto acknowledgment data pipe 3
            DisableAutoAcknowledgmentPipe3 = 0, ///< Disable auto acknowledgment data pipe 3

            EnableAutoAcknowledgmentPipe4Pos = 0,
            EnableAutoAcknowledgmentPipe4Mask = 1 << EnableAutoAcknowledgmentPipe4Pos,
            EnableAutoAcknowledgmentPipe4 = EnableAutoAcknowledgmentPipe4Mask, 	///< Enable auto acknowledgment data pipe 4
            DisableAutoAcknowledgmentPipe4 = 0, ///< Disable auto acknowledgment data pipe 4

            EnableAutoAcknowledgmentPipe5Pos = 0,
            EnableAutoAcknowledgmentPipe5Mask = 1 << EnableAutoAcknowledgmentPipe5Pos,
            EnableAutoAcknowledgmentPipe5 = EnableAutoAcknowledgmentPipe5Mask, 	///< Enable auto acknowledgment data pipe 5
            DisableAutoAcknowledgmentPipe5 = 0, ///< Disable auto acknowledgment data pipe 5
        };

        /**
         * @brief EN_RXADDR register wrapper
         */
        enum EnableRxAddressesRegister : uint8_t
        {
            EnableRxPipe0Pos = 0,
            EnableRxPipe0Mask = 1 << EnableRxPipe0Pos,
            EnableRxPipe0 = EnableRxPipe0Mask,  	///< Enable auto acknowledgment data pipe 0
            DisableRxPipe0 = 0,  ///< Disable auto acknowledgment data pipe 0

            EnableRxPipe1Pos = 0,
            EnableRxPipe1Mask = 1 << EnableRxPipe1Pos,
            EnableRxPipe1 = EnableRxPipe1Mask,  	///< Enable auto acknowledgment data pipe 1
            DisableRxPipe1 = 0,  ///< Disable auto acknowledgment data pipe 1

            EnableRxPipe2Pos = 0,
            EnableRxPipe2Mask = 1 << EnableRxPipe2Pos,
            EnableRxPipe2 = EnableRxPipe2Mask,  	///< Enable auto acknowledgment data pipe 2
            DisableRxPipe2 = 0,  ///< Disable auto acknowledgment data pipe 2

            EnableRxPipe3Pos = 0,
            EnableRxPipe3Mask = 1 << EnableRxPipe3Pos,
            EnableRxPipe3 = EnableRxPipe3Mask,  	///< Enable auto acknowledgment data pipe 3
            DisableRxPipe3 = 0,  ///< Disable auto acknowledgment data pipe 3

            EnableRxPipe4Pos = 0,
            EnableRxPipe4Mask = 1 << EnableRxPipe4Pos,
            EnableRxPipe4 = EnableRxPipe4Mask,  	///< Enable auto acknowledgment data pipe 4
            DisableRxPipe4 = 0,  ///< Disable auto acknowledgment data pipe 4

            EnableRxPipe5Pos = 0,
            EnableRxPipe5Mask = 1 << EnableRxPipe5Pos,
            EnableRxPipe5 = EnableRxPipe5Mask,  	///< Enable auto acknowledgment data pipe 5
            DisableRxPipe5 = 0,  ///< Disable auto acknowledgment data pipe 5
        };

        /**
         * @brief SETUP_AW (setup of address width) register wrapper
         */
        enum SetupAddressWidthRegister : uint8_t
        {
            AddressWidthPos = 0,
            AddressWidthMask = 2 << AddressWidthPos,
            AddressWidth3Bytes = 0x01, ///< Address width = 3 bytes
            AddressWidth4Bytes = 0x02, ///< Address width = 4 bytes
            AddressWidth5Bytes = 0x03, ///< Address width = 5 bytes
        };

        /**
         * @brief Status register wrapper
         */
        enum FifoStatusRegister : uint8_t
        {
            RxEmptyPos = 0,
            RxEmptyMask = 1 << RxEmptyPos,
            RxEmpty = RxEmptyMask, ///< RX FIFO empty
            RxNotEmpty = 0,	///< Data in RX FIFO

            RxFullPos = 1,
            RxFullMask = 1 << RxFullPos,
            RxFull = RxFullMask, ///< RX FIFO full
            RxNotFull = 0, ///< Available locations in RX FIFO

            TxEmptyPos = 4,
            TxEmptyMask = 1 << TxEmptyPos,
            TxEmpty = TxEmptyMask, ///< TX FIFO empty
            TxNotEmpty = 0, ///< Data in TX FIFO

            FifoFullPos = 5,
            FifoFullMask = 1 << FifoFullPos,
            FifoFull = FifoFullMask, ///< TX FIFO full
            FifoNotFull = 0, ///< Available locations in TX FIFO

            TxReusePos = 6,
            TxReuseMask = 1 << TxReusePos,
            TxReuse = TxReuseMask,
            TxNotReuse = 0
        };

        enum RfSetupRegister : uint8_t
        {
            RfPowerPos = 1,
            RfPowerMask = 0b11 << RfPowerPos, // 2 bits
            RfPowerM18dBm = 0b00 << RfPowerPos, ///< Power -18dBm
            RfPowerM12dBm = 0b01 << RfPowerPos, ///< Power -12dBm
            RfPowerM6dBm = 0b10 << RfPowerPos, ///< Power -6dBm
            RfPower0dBm = 0b11 << RfPowerPos, ///< Power 0dBm

            RfDataRatePos = 3,
            RfDataRateMask = 0b101 << RfDataRatePos,	// Its strange that Data rate field has two bits, but it is 3 and 5 O_O
            RfDataRate1Mbps = 0b000 << RfDataRatePos, ///< Data rate 1Mbps
            RfDataRate2Mbps = 0b001 << RfDataRatePos, ///< Data rate 2Mbps
            RfDataRate250Kbps = 0b100 << RfDataRatePos, ///< Data rate 250Kbps
        };

        /**
         * @brief Status register wrapper
         */
        enum StatusRegister : uint8_t
        {
            TxFullPos = 0,
            TxFullMask = 1 << TxFullPos,
            TxFull = TxFullMask, ///< TX FIFO full
            TxNotFull = 0, ///< Available locations in TX FIFO

            RxPipeNumberPos = 1,
            RxPipeNumberMask = 0b111 << RxPipeNumberPos,
            RxFifoEmpty = 0b111 < RxPipeNumberPos,

            MaxRtPos = 4,
            MaxRtMask = 1 << MaxRtPos,
            MaxRt = MaxRtMask, ///< Maximum number of retransmit interrupt caused
            NoMaxRt = 0, ///< No maximum number of retransmit interrupt caused

            TxDataSendPos = 5,
            TxDataSendMask = 1 << TxDataSendPos,
            TxDataSend = TxDataSendMask, ///< Data send interrupt. If auto-ack enabled, this bit is set high only when ACK is received. Write 1 to clear
            TxDataNotSend = 0, ///< No data send interrupt

            RxDrPos = 6,
            RxDrMask = 1 << RxDrPos,
            RxDr = RxDrMask, ///< RX data ready
            NoRxDr = 0, ///< RX data not ready


        };

        enum class TransmitStatus : uint8_t
        {
            Lost = 0x00, ///< Message is lost
            Ok = 0x01, ///< Success sent
            Sending = 0xff ///< Still sending
        };

        /**
         * @brief Data rate
         */
        enum class DataRate : uint8_t
        {
            DataRate2M = RfDataRate2Mbps, ///< 2Mbps
            DataRate1M = RfDataRate1Mbps, ///< 1Mbps
            DataRate250K = RfDataRate250Kbps ///< 250Kbps
        };

        /**
         * @brief Output power
         */
        enum class OutputPower : uint8_t
        {
            OutputPowerM18dBm = RfPowerM18dBm, ///< -18dBm
            OutputPowerM12dBm = RfPowerM12dBm, ///< -12dBm
            OutputPowerM6dBm = RfPowerM6dBm, ///< -6dBm
            OutputPower0dBm = RfPower0dBm, ///< 0dBm
        };
    };

    /**
     * @brief Operator "OR" for template type
     * 
     * @details
     * Some enums are declared as "class" for more strict methods arguments, but
     * at same time it's necessary to combine them. Method casts arguments to unsigned, makes "OR" and
     * casts result vice conversely.
     * 
     * @tparam T type
     * 
     * @param [in] left First value
     * @param [in] right Second value
     * 
     * @returns left OR right
     */
    template<typename T>
    inline T operator|(T left, T right)
    {	return static_cast<T>(static_cast<unsigned>(left) | static_cast<unsigned>(right));	}


    /**
     * @brief Implements NRF24L+ functional
     * 
     * @tparam SpiBus Spi bus class
     * @tparam SSPin GPIO pin for SS
     * @tparam CEPin GPIO pin for CE
     * @tparam EXTIPin Pin for extern interrupt. Use it if you want to connect IRQ 
     */
    template<typename SpiBus, typename SSPin, typename CEPin, typename EXTIPin = IO::NullPin>
    class Nrf24l : public Nrf24lBase
    {		
    public:
        /**
         * @brief Init module
         * 
         * @param [in] channel Channel
         * @param [in] payloadSize Payload size
         * 
         * @par Returns
         *	Nothing
         */
        static void Init(uint8_t channel = 0, uint8_t payloadSize = 32)
        {
            SpiBus::Init(SpiBus::ClockDivider::Medium);

            InitPins();
            if (payloadSize > 32)
                payloadSize = 32;

            _payloadSize = payloadSize;
            Reset();

            SetChannel(channel);

            WriteRegister(Registers::RxPayload0, payloadSize);
            WriteRegister(Registers::RxPayload1, payloadSize);
            WriteRegister(Registers::RxPayload2, payloadSize);
            WriteRegister(Registers::RxPayload3, payloadSize);
            WriteRegister(Registers::RxPayload4, payloadSize);
            WriteRegister(Registers::RxPayload5, payloadSize);

            SetRf(DataRate::DataRate2M, OutputPower::OutputPowerM18dBm);
            WriteRegister(Registers::Configuration, RegistersDefault::Configuration);
            WriteRegister(Registers::EnableAutoAcknowledgment, RegistersDefault::EnableAutoAcknowledgment);
            WriteRegister(Registers::EnableRxAddresses, 0x3f);
            WriteRegister(Registers::AutoRetransmission, 0x4f);
            WriteRegister(Registers::DynamicPayload, RegistersDefault::DynamicPayload);

            FlushTx();
            FlushRx();

            ClearInterrupts();
            PowerUpRx();
        }

        /**
         * @brief Turns power ON in transmit mode
         * 
         * @par Returns	
         *	Nothing
         */
        static void PowerUpTx()
        {
            ClearInterrupts();
            WriteRegister(Registers::Configuration, EnableCRC | CRCScheme1Byte | PTX | PowerUp);
        }

        /**
         * @brief Turns power ON in receive mode
         * 
         * @par Returns	
         *	Nothing
         */
        static void PowerUpRx()
        {
            CEPin::Clear();
            FlushRx();
            ClearInterrupts();
            WriteRegister(Registers::Configuration, EnableCRC | CRCScheme1Byte | PRX | PowerUp);
            CEPin::Set();
        }

        /**
         * @brief Sets channel
         * 
         * @par Returns
         *	Nothing
         */
        static void SetChannel(uint8_t channel)
        {
            if (channel > 125)
                return;
            WriteRegister(Registers::RfChannel, channel);
        }

        /**
         * @brief Sets module address (RX0)
         * 
         * @par Returns
         *	Nothing
         */
        static void SetMyAddress(uint8_t* address)
        {
            CEPin::Clear();
            WriteRegister(Registers::RxAddress1, address, 5);	// TODO:: Module allows to set address width as 3,4 or 5 bytes. This code should be modified
            CEPin::Set();
        }

        /**
         * @brief Set transmit address (TX)
         * 
         * @par Returns
         *	Nothing
         */
        static void SetTxAddress(uint8_t* address)
        {
            WriteRegister(Registers::RxAddress0, address, 5);
            WriteRegister(Registers::TxAddress, address, 5);
        }

        /**
         * @brief Set RF settings: data rate and power
         * 
         * @param [in] rate Data rate
         * @param [in] power Power
         * 
         * @par Returns
         *	Nothing
         */
        static void SetRf(DataRate rate, OutputPower power)
        {
            WriteRegister(Registers::RfSetup, static_cast <uint8_t>(rate) | static_cast <uint8_t >(power));
        }

        /**
         * @brief Transmit data
         * 
         * @param [in] data Data to transmit (module will transmit payloadSize bytes)
         * 
         * @par Returns
         *	Nothing
         */
        static void Transmit(const uint8_t* data)
        {
            CEPin::Clear();
            PowerUpTx();
            FlushTx();

            SSPin::Clear();
            SpiBus::Send(0xa0);
            for (int i = 0; i < _payloadSize; ++i)
            {
                SpiBus::Send(data[i]);
            }

            SSPin::Set();
            CEPin::Set();
        }

        /**
         * @brief Get data (read) from module
         * 
         * @param [out] data Buffer
         * 
         * @par Returns
         *	Nothing
         */
        static void GetData(uint8_t* data)
        {
            SSPin::Clear();
            SpiBus::Send(0x61);
            for (int i = 0; i < _payloadSize; ++i)
                data[i] = SpiBus::Send(0x00);
            SSPin::Set();

            WriteRegister(Registers::Status, StatusRegister::RxDr);
        }

        /**
         * @brief Check for data ready (RX buffer not empty)
         * 
         * @retval true Data ready for read
         * @retval false No data has been received
         */
        static bool DataReady()
        {
            return (GetStatus() & RxDr) != 0;
        }

        /**
         * @brief Check for FIFO is empty
         * 
         * @retval true RX FIFO is empty
         * @retval false RX FIFO is not empty
         */
        static bool RxFifoEmpty()
        {
            return (ReadRegister(Registers::Status) & FifoStatusRegister::RxEmptyMask) > 0;
        }

        /**
         * @brief Returns module status register
         * 
         * @returns Status register value
         */
        static uint8_t GetStatus()
        {
            SSPin::Clear();
            uint8_t status = SpiBus::Send(0xff);
            SSPin::Set();

            return status;
        }

        /**
         * @brief Returns transmission status
         * 
         * @retval TransmitStatus::Ok Success transmit
         * @retval TransmitStatus::Lost Transmission fail (MAX_RT count occured)
         * @retval TransmitStatus::Sending Transmission is in progress
         */
        static TransmitStatus GetTransmissionStatus()
        {
            uint8_t status = GetStatus();

            if (status & TxDsInterruptMask > 0)
            {
                return TransmitStatus::Ok;
            }
            if (status & MaxRtInterruptMask > 0)
            {
                return TransmitStatus::Lost;
            }
            return TransmitStatus::Sending;
        }

        /**
         * @brief Returns transmission try count (OBSERVE TX register value)
         * 
         * @returns Transmission try count
         */
        static uint8_t GetRetransmissionsCount()
        {
            return ReadRegister(Registers::ObserveTx) & 0x0f;
        }
    private:
        static void InitPins()
        {
            SSPin::Port::Enable();
            SSPin::SetConfiguration(SSPin::Configuration::Out);
            SSPin::SetDriverType(SSPin::DriverType::PushPull);
            SSPin::SetSpeed(SSPin::Speed::Fast);
            SSPin::Set();

            CEPin::Port::Enable();
            CEPin::SetConfiguration(CEPin::Configuration::Out);
            CEPin::SetDriverType(CEPin::DriverType::PushPull);
            CEPin::SetSpeed(CEPin::Speed::Fast);
            CEPin::Clear();
        }
        static void Reset()
        {
            uint8_t data[5];

            WriteRegister(Registers::Configuration, RegistersDefault::Configuration);
            WriteRegister(Registers::EnableAutoAcknowledgment, RegistersDefault::EnableAutoAcknowledgment);
            WriteRegister(Registers::EnableRxAddresses, RegistersDefault::EnableRxAddresses);
            WriteRegister(Registers::AddressWidth, RegistersDefault::AddressWidth);
            WriteRegister(Registers::AutoRetransmission, RegistersDefault::AutoRetransmission);
            WriteRegister(Registers::RfChannel, RegistersDefault::RfChannel);
            WriteRegister(Registers::RfSetup, RegistersDefault::RfSetup);
            WriteRegister(Registers::Status, RegistersDefault::Status);
            WriteRegister(Registers::ObserveTx, RegistersDefault::ObserveTx);
            WriteRegister(Registers::CarrierDetect, RegistersDefault::CarrierDetect);

            data[0] = data[1] = data [2] = data[3] = data[4] = RegistersDefault::RxAddress0;
            WriteRegister(Registers::RxAddress0, data, 5);

            data[0] = data[1] = data[2] = data[3] = data[4] = RegistersDefault::RxAddress1;
            WriteRegister(Registers::RxAddress1, data, 5);

            WriteRegister(Registers::RxAddress2, RegistersDefault::RxAddress2);
            WriteRegister(Registers::RxAddress3, RegistersDefault::RxAddress3);
            WriteRegister(Registers::RxAddress4, RegistersDefault::RxAddress4);
            WriteRegister(Registers::RxAddress5, RegistersDefault::RxAddress5);


            data[0] = data[1] = data[2] = data[3] = data[4] = RegistersDefault::TxAddress;
            WriteRegister(Registers::TxAddress, data, 5);

            WriteRegister(Registers::RxPayload0, RegistersDefault::RxPayload0);
            WriteRegister(Registers::RxPayload1, RegistersDefault::RxPayload1);
            WriteRegister(Registers::RxPayload2, RegistersDefault::RxPayload2);
            WriteRegister(Registers::RxPayload3, RegistersDefault::RxPayload3);
            WriteRegister(Registers::RxPayload4, RegistersDefault::RxPayload4);
            WriteRegister(Registers::RxPayload5, RegistersDefault::RxPayload5);
            WriteRegister(Registers::FifoStatus, RegistersDefault::FifoStatus);
            WriteRegister(Registers::DynamicPayload, RegistersDefault::DynamicPayload);
            WriteRegister(Registers::Feature, RegistersDefault::Feature);

        }

        static void FlushTx()
        {
            SSPin::Clear();
            SpiBus::Send(0xe1);
            SSPin::Set();
        }

        static void FlushRx()
        {
            SSPin::Clear();
            SpiBus::Send(0xe2);
            SSPin::Set();
        }
        static uint8_t ReadRegister(Registers registerAddress)
        {
            SSPin::Clear();
            SpiBus::Send((static_cast <uint8_t>(registerAddress) & 0x1F));
            uint8_t value = SpiBus::Send(0xff);
            SSPin::Set();
            return value;
        }

        static void ReadRegister(Registers registerAddress, uint8_t* data, uint8_t size)
        {
            SSPin::Clear();
            SpiBus::Send((static_cast <uint8_t>(registerAddress) & 0x1F));
            for (int i = 0; i < size; ++i)
            {
                data[i] = SpiBus::Send(0xff);
            }
            SSPin::Set();
        }

        /**
         * @brief Writes value to register.
         * 
         * @param [in] registerAddress Register address
         * @param [in] value Value for write
         * 
         * @par Returns
         *	Nothing
         */
        static void WriteRegister(Registers registerAddress, uint8_t value)
        {
            SSPin::Clear();
            SpiBus::Send((static_cast <uint8_t>(registerAddress) & 0x1F) | 0x20);
            SpiBus::Send(value);
            SSPin::Set();
        }

        /**
         * @brief Writes multi data to register.
         * 
         * @param [in] registerAddress Register address
         * @param [in] data Data to write
         * @param [in] size Data size
         * 
         * @par Returns
         *	Nothing
         */
        static void WriteRegister(Registers registerAddress, uint8_t* data, uint8_t size)
        {
            SSPin::Clear();
            SpiBus::Send((static_cast <uint8_t>(registerAddress) & 0x1F) | 0x20);
            for (int i = 0; i < size; ++i)
            {
                SpiBus::Send(data[i]);
            }
            SSPin::Set();
        }

        static void WriteBit(Registers registerAddress, uint8_t bitNumber, bool value)
        {
            uint8_t registerValue = ReadRegister(registerAddress);

            if (value)
            {
                registerValue |= 1 << bitNumber;
            }
            else
            {
                registerValue &= ~(1 << bitNumber);
            }
            WriteRegister(registerAddress, registerValue);
        }

        /**
         * @brief Returns status register
         * 
         * @returns Status register
         */
        static uint8_t ReadInterrupts()
        {
            return GetStatus();
        }
        /**
         * @brief Clear interrupts
         * 
         * @par Returns
         *	Nothing
         */
        static void ClearInterrupts()
        {
            WriteRegister(Registers::Status, 0x70);
        }

        static uint8_t _payloadSize;
    };

    template<typename SpiBus, typename SSPin, typename CEPin, typename EXTIPin>
    uint8_t Nrf24l<SpiBus, SSPin, CEPin, EXTIPin>::_payloadSize;
}
#endif //! ZHELE_DRIVERS_NRF24L_H