/**
 * @file
 * Implement USB MSC class
 * 
 * @author Aleksei Zhelonkin
 * @date 2022
 * @license FreeBSD
 */

#ifndef ZHELE_USB_MSC_H
#define ZHELE_USB_MSC_H

#include "interface.h"

#include "../template_utils/type_list.h"

namespace Zhele::Usb
{
    /// Mass storage class subclass
    enum class MscSubclass : uint8_t
    {
        Rbc = 0x01, ///< RBC
        Atapi = 0x02, ///< ATAPI
        Ufi = 0x04, ///< UFI
        Scsi = 0x06, ///< SCSI
        Lockable = 0x07, ///< Lockable (LSD FS)
        Ieee1667 = 0x08, ///< IEEE 1667
        VendorSpecified = 0xff, ///< Specific to device vendor
    };

    /// Mass storage class transport protocol
    enum class MscProtocol : uint8_t
    {
        Cbi = 0x00, ///< CBI (with command completion interrupt)
        CbiNoCompletionInterrut = 0x01, ///< CBI (with no command completion interrupt)
        Bbb = 0x50, ///< Bulk-only
        Uas = 0x62, ///< UAS
        VendorSpecified = 0xff, ///< Specific to device vendor
    };

    /// Mass storage class request code
    enum class MscRequest : uint8_t
    {
        Adsc = 0x00, ///< Accept device-specific command
        Get = 0xfc, ///< Get request
        Put = 0xfd, ///< Put request
        GetMaxLun = 0xfe, ///< Get max LUN request
        Bomsr = 0xff, ///< Bulk-Only Mass Storage Reset (BOMSR)
    };

    /// Command Block Wrapper structure
    struct BulkOnlyCBW
    {
        uint32_t Signature; ///< Signature (should be equal 43425355h ('USBC'))
        uint32_t Tag; ///<  Command identifier
        uint32_t DataLength; ///< Number of bytes to transfer
        uint8_t Flags; ///< Flags (bit 7: direction (0 - Out, 1 - In))
        uint8_t Lun; ///< Lun number
        uint8_t CommandBlockLength; ///< Length of next field (Command block)
        uint8_t CommandBlock[16]; ///< Command block to be executed
    } __packed;

    /// Command Status Wrapper structure
    struct BulkOnlyCSW
    {
        uint32_t Signature = 0x53425355; ///< Signature (should be equal 53425355h ('USBS'))
        uint32_t Tag; ///< Command identifier (should be equal BulkOnlyCBW.Tag)
        uint32_t DataResidue = 0; ///< Difference between the amount of data expected and the actual amount of data processed

        /// BulkOnlyCSW status
        enum CswStatus : uint8_t
        {
            Passed = 0x00, ///< Command passed
            Failed = 0x01, ///< Command failed
            PhaseError = 0x02 ///< Phase error
        } Status; ///< Command status
    } __packed;

    /// SCSI commands
    enum class ScsiCommand : uint8_t
    {
        TestUnitReady = 0x00, ///< Test unit ready
        RequestSense = 0x03, ///< Request sense
        FormatUnit = 0x04, ///< Format unit
        Read6 = 0x08, ///< Read 6 bytes
        Write6 = 0x0a, ///< Write 6 bytes
        Inquiry = 0x12, ///< Inquiry
        ModeSense6 = 0x1a, ///< Mode sense 6
        SendDiagnistic = 0x1d, ///< Send diagnostic
        ReadCapacity = 0x25, ///< Read capacity
        Read10 = 0x28, ///< Read 10 bytes
        Write10 = 0x2a, ///< Write 10 bytes

        MmcStartStopUnit = 0x1b,
        MmcPreventAllowRemoval = 0x1e,
        MmcReadFormatCapacity = 0x23
    };

    /// READ/WRITE (10) request structure
    struct ScsiReadWrite10Request
    {
        uint8_t Opcode;
        uint8_t CdbInfo1;
        uint32_t BlockAddress;
        uint8_t CdbInfo2;
        uint16_t Length;
        uint8_t Control;
    } __packed;
    

    static const uint8_t  inquiry_page00_data[] = {					
        0x00,		
        0x00, 
        0x00, 
        (7 - 4),
        0x00, 
        0x80, 
        0x83 
    };
    static const uint8_t sense_response[18] = {
        0x70,	// Byte 0: VALID = 0, Response Code = 112
        0x00,	// Byte 1: Obsolete = 0
        0x00,	// Byte 2: Filemark = 0, EOM = 0, ILI = 0, Reserved = 0, Sense Key = 0
            // Byte 3 - Byte 6: Information = 0
        0, 0, 0, 0,
        0x0a,	// Byte 7: Additional Sense Length = 10
            // Byte 8 - Byte 11: Command Specific Info = 0
        0, 0, 0, 0,
        0x00,	// Byte 12: Additional Sense Code (ASC) = 0
        0x00,	// Byte 13: Additional Sense Code Qualifier (ASCQ) = 0
        0x00,	// Byte 14: Field Replaceable Unit Code (FRUC) = 0
        0x00,	// Byte 15: SKSV = 0, SenseKeySpecific[0] = 0
        0x00,	// Byte 16: SenseKeySpecific[0] = 0
        0x00	// Byte 17: SenseKeySpecific[0] = 0
    };

    static const uint8_t inquiry_response[] = {
        0x00,	// Byte 0: Peripheral Qualifier = 0, Peripheral Device Type = 0
        0x80,	// Byte 1: RMB = 1, Reserved = 0
        0x04,	// Byte 2: Version = 0
        0x02,	// Byte 3: Obsolete = 0, NormACA = 0, HiSup = 0, Response Data Format = 2
        0x1F,	// Byte 4: Additional Length (n-4) = 31 + 4
        0x00,	// Byte 5: SCCS = 0, ACC = 0, TPGS = 0, 3PC = 0, Reserved = 0, Protect = 0
        0x00,	// Byte 6: BQue = 0, EncServ = 0, VS = 0, MultiP = 0, MChngr = 0, Obsolete = 0, Addr16 = 0
        0x00,	// Byte 7: Obsolete = 0, Wbus16 = 0, Sync = 0, Linked = 0, CmdQue = 0, VS = 0
        'V', 'E', 'N', 'D', 'O', 'R', ' ', ' ', //Vendor (8 bytes)
        'P', 'R', 'O', 'D', 'U', 'C', 'T', ' ', //Product (16 bytes)
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        '0', '.', '0', '1' //Version (4 bytes)
    };

    class ScsiLunBase
    {
    public:
        /**
         * @brief Converter between Little-endian and Big-endian
         * 
         * @tparam T Type
         * 
         * @param value Value
         * 
         * @returns Converted value
         */
        template<typename T>
        inline constexpr static T ConvertLeBe(T value)
        {
            if constexpr (sizeof(value) == 1)
                return value;
            
            if constexpr (sizeof(value) == 2)
                return ((value & 0xff) << 8) | ((value >> 8) & 0xff);

            if constexpr (sizeof(value) == 4)
                return ((value & 0xff) << 24) | ((value & 0xff00) << 8) | ((value & 0xff0000) >> 8) | ((value >> 24) & 0xff);
        }
    };

    /**
     * @brief Class for SCSI LUN
     * 
     * @tparam _LunNumber Number
     * @tparam _LunSpecialization LUN specialization (with LBA size/count method, Read/Write handlers)
     */
    template<typename _LunSpecialization>
    class ScsiLun : public ScsiLunBase
    {
    public:
        static const uint8_t Number = _LunSpecialization::LunNumber;

        /**
         * @brief LUN command handler
         * 
         * @tparam _InEp IN endpoint (for response)
         * 
         * @param cbw CBW
         * @param csw CSW
         * @param callback Response callback
         * 
         * @return true Waiting for receive
         * @return false Not waiting for receive
         */
        template<typename _InEp>
        static bool CommandHandler(const BulkOnlyCBW& cbw, BulkOnlyCSW& csw, InTransferCallback callback)
        {
            csw.Tag = cbw.Tag;
            csw.Status = BulkOnlyCSW::CswStatus::Passed;
            csw.DataResidue = 0;

            switch (static_cast<ScsiCommand>(cbw.CommandBlock[0]))
            {
            case ScsiCommand::Inquiry:
                if(cbw.CommandBlock[1] & 0x01) {
                    _InEp::SendData(inquiry_page00_data, cbw.DataLength < sizeof(inquiry_page00_data) ? cbw.DataLength : sizeof(inquiry_page00_data), callback);
                } else {
                    _InEp::SendData(inquiry_response, cbw.DataLength < sizeof(inquiry_response) ? cbw.DataLength : sizeof(inquiry_response), callback);
                }
                break;
            case ScsiCommand::MmcReadFormatCapacity: {
                constexpr uint8_t buffer[] = { 0, 0, 0, 8,
                    (_LunSpecialization::GetLbaCount() >> 24) & 0xff,
                    (_LunSpecialization::GetLbaCount() >> 16) & 0xff,
                    (_LunSpecialization::GetLbaCount() >> 8) & 0xff,
                    (_LunSpecialization::GetLbaCount() >> 0) & 0xff,

                    0b10, // formatted media
                    (_LunSpecialization::GetLbaSize() >> 16) & 0xff,
                    (_LunSpecialization::GetLbaSize() >> 8) & 0xff,
                    (_LunSpecialization::GetLbaSize() >> 0) & 0xff,
                };
                _InEp::SendData(buffer, sizeof(buffer), callback);
                break;
            }
            case ScsiCommand::ReadCapacity: {
                uint32_t buffer[] = { ConvertLeBe(_LunSpecialization::GetLbaCount() - 1), ConvertLeBe(_LunSpecialization::GetLbaSize()) };

                _InEp::SendData(buffer, sizeof(buffer), callback);
                break;
            }
            case ScsiCommand::ModeSense6: {
                // TODO:: Add template parameter for LUN type
                uint8_t buffer[] = {3, 0, 0, 0};
                csw.DataResidue = cbw.DataLength - sizeof(buffer);
                _InEp::SendData(buffer, sizeof(buffer), callback);
                break;
            }
            case ScsiCommand::TestUnitReady : {
                callback();
                break;
            }
            case ScsiCommand::Read10: {
                uint32_t startLba = ConvertLeBe(reinterpret_cast<const ScsiReadWrite10Request*>(&cbw.CommandBlock[0])->BlockAddress);
                uint32_t lbaCount = ConvertLeBe(reinterpret_cast<const ScsiReadWrite10Request*>(&cbw.CommandBlock[0])->Length);

                _LunSpecialization::template Read10Handler<_InEp>(startLba, lbaCount, callback);
                break;
            }
            case ScsiCommand::Write10: {
                uint32_t startLba = ConvertLeBe(reinterpret_cast<const ScsiReadWrite10Request*>(&cbw.CommandBlock[0])->BlockAddress);
                uint32_t lbaCount = ConvertLeBe(reinterpret_cast<const ScsiReadWrite10Request*>(&cbw.CommandBlock[0])->Length);

                return _LunSpecialization::Write10Handler(startLba, lbaCount);
            }
            case ScsiCommand::MmcStartStopUnit:
            case ScsiCommand::MmcPreventAllowRemoval:
                callback();
            default:
                break;
            }

            return false;
        }
    };

    /**
     * @brief Class for SCSI LUN
     * 
     * @tparam _LunNumber Number
     * @tparam _LbaSize Lba size
     * @tparam _LbaCount Lba count
     */
    template<uint32_t _LbaSize, uint32_t _LbaCount>
    class ScsiLunWithConstSize : public ScsiLunBase
    {
    public:
        /**
         * @brief Returns LBA size
         * 
         * @returns LBA size (in bytes)
         */
        static inline constexpr uint32_t GetLbaSize()
        {
            return _LbaSize;
        }

        /**
         * @brief Returns LBA count
         * 
         * @returns LBA count
         */
        static inline constexpr uint32_t GetLbaCount()
        {
            return _LbaCount;
        }

        /**
         * @brief Read (10) command handler
         * 
         * @tparam _InEp IN endpoint
         * 
         * @param startLba Start LBA
         * @param lbaCount LBA count
         * @param callback Transfer complete callback for call
         * 
         * @par Returns
         *  Nothing
         */
        template<typename _InEp>
        static void Read10Handler(uint32_t startLba, uint32_t lbaCount, InTransferCallback callback);

        /**
         * @brief Write (10) command handler
         * 
         * @param startLba Start LBA
         * @param lbaCount LBA count
         * 
         * @retval true Wait for next packet
         * @retval false OUT transfer complete
         */
        static bool Write10Handler(uint32_t startLba, uint32_t lbaCount);

        /**
         * @brief LUN rx handler
         * 
         * @param data Data
         * @param size Data size
         * 
         * @return true Waiting for next packet (transfer does not complete)
         * @return false Transfer complete
         */
        static bool RxHandler(void* data, uint16_t size);
    };

    /**
     * @brief Default SCSI logical unit (static size + static )
     * 
     * @tparam _LunNumber Number
     * @tparam _LbaSize Lba size
     * @tparam _LbaCount Lba count
     */
    template<uint32_t _LbaSize, uint32_t _LbaCount>
    class DefaultScsiLun : public ScsiLunWithConstSize<_LbaSize, _LbaCount>
    {
    public:
        /**
         * @brief Read (10) command handler
         * 
         * @tparam _InEp IN endpoint
         * 
         * @param startLba Start LBA
         * @param lbaCount LBA count
         * @param callback Transfer complete callback for call
         * 
         * @par Returns
         *  Nothing
         */
        template<typename _InEp>
        static void Read10Handler(uint32_t startLba, uint32_t lbaCount, InTransferCallback callback)
        {
            _InEp::SendData(&_buffer[startLba * _LbaSize], lbaCount * _LbaSize, callback);
        }

        /**
         * @brief Write (10) command handler
         * 
         * @param startLba Start LBA
         * @param lbaCount LBA count
         * 
         * @retval true Wait for next packet
         * @retval false OUT transfer complete
         */
        static bool Write10Handler(uint32_t startLba, uint32_t lbaCount)
        {
            _rxAddress = startLba * _LbaSize;
            _rxBytesRemain = lbaCount * _LbaSize;
            
            return lbaCount > 0;
        }

        /**
         * @brief LUN rx handler
         * 
         * @param data Data
         * @param size Data size
         * 
         * @return true Waiting for next packet (transfer does not complete)
         * @return false Transfer complete
         */
        static bool RxHandler(void* data, uint16_t size)
        {
            CopyFromUsbPma(&_buffer[_rxAddress], data, size);

            _rxAddress += size;
            _rxBytesRemain -= size;

            return _rxBytesRemain > 0;
        }
    private:
        static uint32_t _rxAddress;
        static int32_t _rxBytesRemain;
        static uint8_t _buffer[_LbaCount * _LbaSize];
    };

    template<uint32_t _LbaSize, uint32_t _LbaCount>
    uint32_t DefaultScsiLun<_LbaSize, _LbaCount>::_rxAddress;
    template<uint32_t _LbaSize, uint32_t _LbaCount>
    int32_t DefaultScsiLun<_LbaSize, _LbaCount>::_rxBytesRemain;
    template<uint32_t _LbaSize, uint32_t _LbaCount>
    uint8_t DefaultScsiLun<_LbaSize, _LbaCount>::_buffer[_LbaCount * _LbaSize];;


    /**
     * @brief Implements SCSI BBB interface
     * 
     * @tparam _Number Interface number
     * @tparam _AlternateSetting Interface alternate setting
     * @tparam _Ep0 Zero endpoint instance
     * @tparam _OutEp OUT endpoint
     * @tparam _InEP IN endpoint
     * @tparam _Luns LUNs
     * 
     */
    template <uint8_t _Number, uint8_t _AlternateSetting, typename _Ep0, typename _OutEp, typename _InEp, typename... _Luns>
    class ScsiBulkInterface : public Interface<_Number, _AlternateSetting, InterfaceClass::Storage, static_cast<uint8_t>(MscSubclass::Scsi), static_cast<uint8_t>(MscProtocol::Bbb), _Ep0, _OutEp, _InEp>
    {     
        using Base = Interface<_Number, _AlternateSetting, InterfaceClass::Storage, static_cast<uint8_t>(MscSubclass::Scsi), static_cast<uint8_t>(MscProtocol::Bbb), _Ep0, _OutEp, _InEp>;

        static constexpr std::add_pointer_t<bool(void* buffer, uint16_t size)> _lunRxHandlers[] = {_Luns::RxHandler...};
        static constexpr std::add_pointer_t<bool(const BulkOnlyCBW& cbw, BulkOnlyCSW& csw, InTransferCallback callback)> _lunCommandHandlers[] = {(ScsiLun<_Luns>::template CommandHandler<_InEp>)...};
    public:
        using Endpoints = Base::Endpoints;

        /**
         * @brief Interface setup request handler
         * 
         * @par Returns
         *  Nothing
         */
        static void SetupHandler()
        {
            SetupPacket* setup = reinterpret_cast<SetupPacket*>(_Ep0::RxBuffer);

            switch (static_cast<MscRequest>(setup->Request))
            {
            case MscRequest::Bomsr:
                ResetScsi();
                break;

            case MscRequest::GetMaxLun: {
                uint8_t maxLun = (sizeof...(_Luns) - 1);
                _Ep0::SendData(&maxLun, 1);
                break;
            }

            default:
                break;
            };
        }

        /**
         * @brief Reset SCSI (handler BOMSR request)
         * 
         * @par Returns
         *  Nothing
         */
        static void ResetScsi()
        {
        }

        /**
         * @brief Handler for recive
         * 
         * @details Call this method from HandleRx OUT endpoint
         * Sorry, I don't know how define this method otherwise.
         * I can add new EP class with parent as template parameter to inherit HandleRx method
         * but its too crazy template magic.
         * 
         * @param data 
         * @param size 
         */
        static void HandleRx(void* data, uint16_t size)
        {
            static BulkOnlyCBW request;
            static BulkOnlyCSW response;
            static uint8_t cbwBytesReceived = 0;
            static bool needReceive = false;

            if(cbwBytesReceived < sizeof(BulkOnlyCBW)) {
                CopyFromUsbPma(reinterpret_cast<uint8_t*>(&request) + cbwBytesReceived, data, size);

                cbwBytesReceived += size;

                if(cbwBytesReceived == sizeof(BulkOnlyCBW)) {
                    needReceive = _lunCommandHandlers[request.Lun](request, response, [](){
                        cbwBytesReceived = 0;
                        _InEp::SendData(&response, sizeof(response));
                    });
                } else {
                    return;
                }
            } else if (needReceive) {
                needReceive = _lunRxHandlers[request.Lun](data, size);
                if(!needReceive) {
                    cbwBytesReceived = 0;
                    _InEp::SendData(&response, sizeof(response));
                }
            }
        }
    };
}

#endif // ZHELE_USB_MSC_H