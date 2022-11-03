/**
 * @file
 * Driver for RS485 (wrapper on usart)
 * 
 * @author Aleksei Zhelonkin
 * @date 2022
 * @license FreeBSD
 */

#include <iopins.h>
#include <usart.h>

namespace Zhele::Drivers
{
    template<typename _Usart, typename _DirectPin = IO::NullPin>
    class Adm485 : public _Usart
    {
        using Base = _Usart;
    public:
        using UsartMode = Base::UsartMode;
        /**
         * @brief Initialize USART and direct pin
         * 
         * @tparam baud Baud rate
         * @param [in] mode Mode
         * 
         * @par Returns
         *	Nothing
            */
        template<unsigned long baud>
        static inline void Init(UsartMode mode = DefaultUsartMode)
        {
            Base::Init<baud>(mode);
            InitPin();
        }
        

        /**
         * @brief Initialize USART
         * 
         * @param [in] baud Baud rate
         * @param[in] mode Mode
         * 
         * @par Returns
         *	Nothing
            */
        static void Init(unsigned baud, UsartMode mode = DefaultUsartMode)
        {
            Base::Init(baud, mode);
            InitPin();
        }

        /**
         * @brief Write data to line
         * 
         * @param [in] data Data to write
         * @param [in] size Data size
         * 
         * @par Returns
         * 	Nothing
         */
        static void Write(const void* data, size_t size)
        {
            _DirectPin::Set();
            Base::Write(data, size + 1); // +1, because usart ready when TX buffer empty (not last byte has been transmitted)
            while(!Base::WriteReady()) continue;
            _DirectPin::Clear();
        }

        /**
         * @brief Write data to line async
         * 
         * @param [in] data Data to write
         * @param [in] size Data size
         * @param [in] async Write async or no
         * 
         * @par Returns
         * 	Nothing
         */
        static void WriteAsync(const void* data, size_t size, std::add_pointer_t<void()> callback = nullptr)
        {
            _DirectPin::Set();
            Base::WriteAsync(data, size + 1, [callback](auto... unused){
                while(!Base::WriteReady()) continue;
                _DirectPin::Clear();
                if(callback)
                    callback();
            });
        }

        /**
         * @brief Sync write byte
         * 
         * @param [in] data Byte to write
         * 
         * @par Returns
         *	Nothing
        */
        static void Write(uint8_t data)
        {
            _DirectPin::Set();
            Base::Write(data);
            _DirectPin::Clear();
        }
    private:
        static void InitPin()
        {
            if constexpr (std::is_same_v<_DirectPin, IO::NullPin>)
                return;

            _DirectPin::Port::Enable();
            _DirectPin::template SetConfiguration<_DirectPin::Configuration::Out>();
            _DirectPin::template SetDriverType<_DirectPin::DriverType::PushPull>();
            _DirectPin::Clear();
        }
    };
}