/**
 * @file
 * Driver for MFRC522 RFID card reader
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */
#ifndef ZHELE_DRIVERS_RC522_H
#define  ZHELE_DRIVERS_RC522_H

#include <spi.h>
#include <delay.h>

namespace Zhele
{
    namespace Drivers
    {
        /**
         * @brief Implements MFRC522 RFID reader
         * 
         * @tparam _SpiBus Target SPI
         * @tparam _SSPin Slave select pin for transaction control
         * 
         * @todo
         * Class exports basic functional: Init and Check method.
         * It is enough for most of tasks.
         * However, it will be good to implements extended functional
         * (such as small methods (GetVersion and more) and more difficult (Self test, for example).
         * The best source for Arduino that i know is MFRC522 class
         * https://github.com/miguelbalboa/rfid/
         */
        template<typename _SpiBus, typename _SSPin = IO::NullPin>
        class Rc522
        { 
            /**
             * @brief MFRC522 commands. Described in chapter 10 of datasheet.

             */
            enum class Commands : uint8_t
            {
                Idle = 0x00, ///< No action
                CalculateCRC = 0x03, ///< CRC calculate
                Transmit = 0x04, ///< Transmit data
                Receive = 0x08, ///< Receive data
                Transceive = 0x0c, ///< Transmit and receive data
                Auth = 0x0e, ///< Authentication
                Reset = 0x0f, ///< Reset

                // Mifare_One card commands
                RequestIdl = 0x26, ///< Find the antenna does not enter hibernation
                RequestAll = 0x52,  ///< Find all the cards antenna area
                AntiCollision = 0x93, ///< Anti-collision
                SelectTag = 0x93, ///< Selection card
                AuthA = 0x60, ///< Authentication key A
                AuthB = 0x61, ///< Authentication key B
                Read = 0x30, ///< Read block
                Write = 0xa0, ///< Send block
                Decrement = 0xc0, ///< Debit
                Increment = 0xc1, ///< Recharge
                Restore = 0xc2, ///< Transfer block data to buffer
                Transfer = 0xb0, ///< Save data in buffer
                Halt = 0x50 ///< Sleep
            };

            /**
             * @brief MFRC522 registers. Described in chapter 9 of datasheet.
             */
            enum class Registers : uint8_t
            {
                //Page 0 : Command and status
                Command = 0x01, ///< Start and stop command execution
                ComInterruptEnable = 0x02, ///< Enable and disable interrupt request control bits
                DivInterruptEnable = 0x03, ///< Enable and disable interrupt request control bits
                ComIrq = 0x04, ///< Interrupt request bits
                DivIrq = 0x05, ///< Interrupt request bits
                Error = 0x06, ///< Error bits showing the error status of the last command executed
                Status1 = 0x07, ///< Communication status bits
                Status2 = 0x08, ///< Receiver and transmitter status bits
                FifoData = 0x09, ///< Input and output of 64 byte FIFO buffer
                FifoLevel = 0x0a, ///< Number of bytes stored in the FIFO buffer
                WaterLevel = 0x0b, ///< Level for FIFO underflow and overflow warning
                Control = 0x0c, ///< Miscellaneous control registers
                BitFraming = 0x0d, ///< Adjustments for bit-oriented frames
                Collision = 0x0e, ///< Bit position of the first bit-collision detected on the RF interface

                // Page 1: Command
                Mode = 0x11, ///< Defines general modes for transmitting and receiving
                TxMode = 0x12, ///< Defines transmission data rate and framing
                RxMode = 0x13, ///< Defines reception data rate and framing
                TxControl = 0x14, ///< Controls the logical behavior of the antenna driver pins TX1 and TX2
                TxASK = 0x15, ///< Controls the setting of the transmission modulation
                TxSelect = 0x16, ///< Selects the internal sources for the antenna driver
                RxSelect = 0x17, ///< Selects internal receiver settings
                RxThreshold = 0x18, ///< Selects thresholds for the bit decoder
                Demodulator = 0x19, ///< Defines demodulator settings
                MfTx = 0x1c, ///< Controls some MIFARE communication transmit parameters
                MfRx = 0x1c, ///< Controls some MIFARE communication receive parameters
                SerialSpeed = 0x1f, ///< Selects the speed of the serial UART interface

                // Page 2: Configuration
                CRCResultMSB = 0x21, ///< MSB value of CRC calculation
                CRCResultLSB = 0x22, ///< LSB value of CRC calculation
                ModWidth = 0x24, ///< Controls the ModWidth setting
                RfConfig = 0x26, ///< Configures the receiver gain
                GsConfig = 0x27, ///< Selects the conductance of the antenna driver pins TX1 and TX2 for modulation
                CWGsP = 0x28, ///< Defines the conductance of the p-driver output during periods of no modulation
                ModGsP = 0x29, ///< Defines the conductance of the p-driver output during periods of modulation
                TMode = 0x2a, ///< Defines settings for the internal timer
                TPrescaler = 0x2b, ///< Defines settings for the internal timer
                TReloadHigh = 0x2c, ///< Defines the high part of 16-bit timer reload value
                TReloadLow = 0x2d, ///< Defines the low part of 16-bit timer reload value
                TCounterHigh = 0x2e, ///< Shows the high part 16-bit timer value
                TCounterLow = 0x2f, ///< Shows the low part 16-bit timer value

                // Page 3: Test register
                TestSel1 = 0x31, ///< General test signal configuration
                TestSel2 = 0x32, ///< General test signal configuration
                TestPinEnable = 0x33, ///< Enables pin output driver on pins D1 to D7
                TestPinValue = 0x34, ///< Defines the values for D1 to D7 when it is used as an I/O bus
                TestBus = 0x25, ///< Shows the status of the internal test bus
                AutoTest = 0x26, ///< Controls the digital self test
                Version = 0x37, // Software version
            };

            static const uint8_t MaxDataSize = 16;

        public:
            enum Status
            {
                Success    = 0,
                NoTagError,
                Error
            }; 

            /**
             * @brief Initialize MCU and MFRC522 unit
             * 
             * @details
             * Init _SSPin and MFRC522 module for work.
             * 
             * @note
             * Initialize SPI manually!
             * 
             * @par Returns
             *	Nothing
             */
            static void Init()
            {
                _SSPin::Port::Enable();
                _SSPin::SetConfiguration(_SSPin::Configuration::Out);
                _SSPin::SetDriverType(_SSPin::DriverType::PushPull);
                _SSPin::SetSpeed(_SSPin::Speed::Fast);
                _SSPin::Set();
                _SSPin::WaitForSet();

                Reset();
                WriteRegister(Registers::TxMode, 0x00);
                WriteRegister(Registers::RxMode, 0x00);
                WriteRegister(Registers::ModWidth, 0x26);

                WriteRegister(Registers::TMode, 0x80);
                WriteRegister(Registers::TPrescaler, 0xa9);
                WriteRegister(Registers::TReloadHigh, 0x03);
                WriteRegister(Registers::TReloadLow, 0xe8);
                WriteRegister(Registers::TxASK, 0x40);
                WriteRegister(Registers::Mode, 0x3d);

                AntennaOn();
            }

            /**
			 * @brief Initialize MCU and MFRC522 unit
			 * 
			 * @details
			 * Init SPI, _SSPin and MFRC522 module for work.
			 * 
			 * @par Returns
			 *	Nothing
			 */
			static void InitWithSpi()
			{
                _SpiBus::Init(_SpiBus::ClockDivider::Medium);
                Init();
            }

            /**
             * @brief Try to read card and fix collision.
             * 
             * @param [out] cardId Buffer for card ID
             * 
             * @retval I2cStatus::Success If card was presented and successful readed.
             * @retval I2cStatus::NoTagError If card was presented but fail readed.
             * @retval I2cStatus::error If card was not presented and another fails.
             */
            static Status Check(uint8_t* cardId)
            {
                Status status = Request(static_cast <uint8_t>(Commands::RequestIdl), cardId);
                if (status == Status::Success)
                {
                    status = AntiCollision(cardId);
                }

                Halt();
                return status;
            }

            /**
             * @brief Compare two card ID.
             * 
             * @details
             * Compare two 5-bytes arrays.
             * 
             * @param[in] first First ID
             * @param[in] second Second ID
             * 
             * @retval true If IDs are equal
             * @retval false If IDs don't equal
             */
            static bool Compare(const uint8_t* first, const uint8_t* second)
            {
                for (int i = 0; i < 5; ++i)
                {
                    if (first[i] != second[i])
                        return false;
                }
                return true;
            }

        private:
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
                _SSPin::Clear();
                _SpiBus::Send((static_cast <uint8_t>(registerAddress) << 1) & 0x7e);
                _SpiBus::Send(value);
                _SSPin::Set();
            }

            /**
             * @brief Reads register value.
             * 
             * @param [in] registerAddress Register address
             * 
             * @returns Readed value
             */
            static uint8_t ReadRegister(Registers registerAddress)
            {
                _SSPin::Clear();
                _SpiBus::Write(((static_cast <uint8_t >(registerAddress) << 1) & 0x7e) | 0x80);
                uint8_t readed = _SpiBus::Read();
                _SSPin::Set();
                return readed;
            }

            /**
             * @brief Set register`s bits with mask
             * 
             * @param [in] registerAddress Register address
             * @param [in] mask Mask
             * 
             * @par Returns
             *	Nothing
             */
            static void SetBitMask(Registers registerAddress, uint8_t mask)
            {
                WriteRegister(registerAddress, ReadRegister(registerAddress) | mask);
            }

            /**
             * @brief Clear register`s bits with mask
             * 
             * @param [in] registerAddress Register address
             * @param [in] mask Mask
             * 
             * @par Returns
             *	Nothing
             */
            static void ClearBitMask(Registers registerAddress, uint8_t mask)
            {
                WriteRegister(registerAddress, ReadRegister(registerAddress) & ~mask);
            }

            /**
             * @brief Turns antenna on
             *
             * @par Returns
             *	Nothing
             */
            static void AntennaOn()
            {
                uint8_t temp = ReadRegister(Registers::TxControl);
                if (!(temp & 0x03))
                {
                    SetBitMask(Registers::TxControl, 0x03);
                }
            }

            /**
             * @brief Turns antenna off
             *
             * @par Returns
             *	Nothing
             */
            static void AntennaOff()
            {
                ClearBitMask(Registers::TxControl, 0x03);
            }

            /**
             * @brief Reset MFRC522 module
             * 
             * @par Returns
             * Nothing
             */
            static void Reset()
            {
                WriteRegister(Registers::Command, static_cast<uint8_t>(Commands::Reset));
            }

            /**
             * @brief Makes a request to MFRC522
             * 
             * @param [in] requestMode Request type
             * @param [in, out] tagType Data for transmit and buffer for receive
             * 
             * @returns Operation result
             */
            static Status Request(uint8_t requestMode, uint8_t* tagType)
            {
                WriteRegister(Registers::BitFraming, 0x07);

                tagType[0] = requestMode;
                uint16_t tagTypeSize;
                Status status = ToCard(Commands::Transceive, tagType, 1, tagType, &tagTypeSize);
                if (tagTypeSize != 0x10)
                {
                    status = Status::Error;
                }
                return status;
            }

            /**
             * @brief Send command and data to module, read answer
             * 
             * @param [in] command Command
             * @param [in] transmitData Data to transmit
             * @param [in] transmitDataSize Transmit data size
             * @param [out] receiveData Output buffer
             * @param [out] receiveDataSize Received size
             * 
             * @returns Operation status
             */
            static Status ToCard(Commands command, uint8_t* transmitData, uint8_t transmitDataSize, uint8_t* receiveData, uint16_t* receiveDataSize)
            {
                uint8_t irqEnable = 0x00;
                uint8_t waitIrq = 0x00;

                switch (command)
                {
                case Commands::Auth:
                    irqEnable = 0x12;
                    waitIrq = 0x10;
                    break;

                case Commands::Transceive:
                    irqEnable = 0x77;
                    waitIrq = 0x30;
                    break;

                default:
                    break;
                }


                WriteRegister(Registers::ComInterruptEnable, irqEnable | 0x80);
                ClearBitMask(Registers::ComIrq, 0x80);
                SetBitMask(Registers::FifoLevel, 0x80);

                WriteRegister(Registers::Command, static_cast <uint8_t >(Commands::Idle));

                for (int i = 0; i < transmitDataSize; ++i)
                {
                    WriteRegister(Registers::FifoData, transmitData[i]);
                }

                WriteRegister(Registers::Command, static_cast <uint8_t >(command));

                if (command == Commands::Transceive)
                {
                    SetBitMask(Registers::BitFraming, 0x80);
                }

                uint16_t timeout = 10000;
                uint8_t comIrqRegisterValue;
                do
                {
                    comIrqRegisterValue = ReadRegister(Registers::ComIrq);
                    --timeout;
                } while ((timeout != 0) && !(comIrqRegisterValue & 0x01) && !(comIrqRegisterValue & waitIrq));

                ClearBitMask(Registers::BitFraming, 0x80);

                if (timeout == 0)
                {
                    return Status::Error;
                }

                if ((ReadRegister(Registers::Error) & 0x1b))
                    return Status::Error;

                if (comIrqRegisterValue & irqEnable & 0x01)
                    return Status::NoTagError;

                if (command == Commands::Transceive)
                {
                    uint8_t fifoSize = ReadRegister(Registers::FifoLevel);
                    uint8_t lastBits = ReadRegister(Registers::Control) & 0x07;
                    if (lastBits)
                    {
                        *receiveDataSize = (fifoSize - 1) * 8 + lastBits;
                    }
                    else
                    {
                        *receiveDataSize = fifoSize * 8;
                    }

                    if (fifoSize == 0)
                    {
                        fifoSize = 1;
                    }
                    if (fifoSize > MaxDataSize)
                    {
                        fifoSize = MaxDataSize;
                    }

                    for (int i = 0; i < fifoSize; ++i)
                    {
                        receiveData[i] = ReadRegister(Registers::FifoData);
                    }
                    
                }

                return Status::Success;
            }

            static Status AntiCollision(uint8_t* id)
            {
                WriteRegister(Registers::BitFraming, 0x00);

                id[0] = static_cast <uint8_t >(Commands::AntiCollision);
                id[1] = 0x20;

                uint16_t dummy;
                Status status = ToCard(Commands::Transceive, id, 2, id, &dummy);
            
                if (status != Status::Success)
                {
                    return status;
                }

                uint8_t idCheck = 0;
                for (int i = 0; i < 4; ++i)
                {
                    idCheck ^= id[i];
                }
                //todo:: Check this code! In TM lib it's strange place.
                return idCheck == id[4] ? Status::Success : Status::Error;
            }

            static void CalculateCRC(uint8_t* data, uint8_t size, uint8_t* result)
            {
                ClearBitMask(Registers::DivIrq, 0x04);
                SetBitMask(Registers::FifoLevel, 0x80);

                for (int i = 0; i < size; ++i)
                {
                    WriteRegister(Registers::FifoData, data[i]);
                }
                WriteRegister(Registers::Command, static_cast <uint8_t >(Commands::CalculateCRC));

                uint8_t timeout = 0xff;
                while (!(ReadRegister(Registers::DivIrq) & 0x04) && timeout--) ;

                result[0] = ReadRegister(Registers::CRCResultLSB);
                result[1] = ReadRegister(Registers::CRCResultMSB);
            }

            static uint8_t SelectTag(uint8_t* id)
            {
                uint8_t buffer[9];
                buffer[0] = Commands::SelectTag;
                buffer[1] = 0x70;

                for (int i = 0; i < 5; ++i)
                {
                    buffer[i + 2] = id[i];
                }

                CalculateCRC(buffer, 7, &buffer[7]);

                uint16_t receiveSize = 0;
                Status status = ToCard(Commands::Transeive, buffer, 9, buffer, &receiveSize);

                return status == Status::Success && receiveSize == 0x18
                    ? buffer[0]
                    : 0;
            }
            static Status Auth(uint8_t authMode, uint8_t blockAddress, uint8_t* sectorKey, uint8_t* id)
            {
                uint8_t buffer[12];

                buffer[0] = authMode;
                buffer[1] = blockAddress;

                for (int i = 0; i < 6; ++i)
                {
                    buffer[i + 2] = sectorKey[i];
                }

                for (int i = 0; i < 4; ++i)
                {
                    buffer[i + 8] = id[i];
                }

                uint16_t dummy;
                Status status = ToCard(Commands::Auth, buffer, 12, buffer, &dummy);
                return (status != Status::Success || (!(ReadRegister(Registers::Status2) & 0x08)))
                    ? Status::Error
                    : status;
            }

            static Status Read(uint8_t blockAddress, uint8_t* receiveData)
            {
                receiveData[0] = Commands::Read;
                receiveData[1] = blockAddress;

                CalculateCRC(receiveData, 2, &receiveData[2]);
                uint16_t receiveSize;
                Status status = ToCard(Commands::Transeive, receiveData, 4, receiveData, &receiveSize);

                return (status != Status::Success || receiveSize != 0x90)
                    ? Status::Error
                    : status;		
            }
            static Status Write(uint8_t blockAddress, uint8_t* data)
            {
                uint8_t buffer[18];

                buffer[0] = Commands::Send;
                buffer[1] = blockAddress;
                CalculateCRC(buffer, 2, &buffer[2]);
                uint16_t receiveSize;
                Status status = ToCard(Commands::Transeive, buffer, 4, buffer, &receiveSize);

                if (status != Status::Success || receiveSize != 4 || (buffer[0] & 0x0f) != 0x0a)
                    status == Status::Error;

                if (status != Status::Success)
                    return status;

                for (int i = 0; i < 16; ++i)
                {
                    buffer[i] = data[i];
                }
                CalculateCRC(buffer, 16, &buffer[16]);
                status = ToCard(Commands::Transeive, buffer, 18, buffer, &receiveSize);

                return (status != Status::Success || receiveSize != 4 || (buffer[0] & 0x0f) != 0x0a)
                    ? Status::Error
                    : status;

            }
            static void Halt()
            {
                uint8_t buffer[4];
                buffer[0] = static_cast <uint8_t >(Commands::Halt);
                CalculateCRC(buffer, 2, &buffer[2]);

                uint16_t resultSize;
                ToCard(Commands::Transceive, buffer, 4, buffer, &resultSize);
            }
        };
    }
}

#endif //! ZHELE_DRIVERS_RC522_H