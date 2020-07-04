// Define target cpu frequence.
#define F_CPU 8000000

#include <usart.h>

#include <cstring>

using namespace Zhele;
using namespace Zhele::IO;

using UsartConnection = Usart1;

// This program enables LED and USART. After initial setings sends "Hello\r\n" to USART.
// Then program waits input data. If "on" has been received LED will be turn on and controller transmits "LED is turn on\r\n".
// If "off" has been received LED will be turn off and controller transmits "LED is turn off\r\n".
int main()
{
	// Configure led (I have stm32f103c8t6-based board. It has led on C13)
	Pc13::Port::Enable();
	Pc13::SetConfiguration(Pc13::Configuration::Out);
	Pc13::SetDriverType(Pc13::DriverType::PushPull);
	Pc13::Clear();

    // Init usart1, baud = 9600
    UsartConnection::Init(9600);
    // Select pins
    UsartConnection::SelectTxRxPins<Pb6, Pb7>();
    // Enable interrupt for receive
    UsartConnection::EnableInterrupt(UsartConnection::InterruptFlags::RxNotEmptyInt);
    // Write sync "Hello\r\n"
    UsartConnection::Write("Hello\r\n", 7);

	for (;;)
	{
	}
}

char Buffer[8];
int Size = 0;


extern "C"
{
    void USART1_IRQHandler()
    {
        // Check that usart is ready to read
        if(UsartConnection::ReadReady())
        {
            // Read next byte
            Buffer[Size++] = UsartConnection::Read();
        }

        // Clear interrupt flag
        UsartConnection::ClearInterruptFlag(UsartConnection::InterruptFlags::RxNotEmptyInt);

        // Check that we receive 2 bytes and it is "on"
        if(Size == 2 && Buffer[0] == 'o' && Buffer[1] == 'n')
        {
            // Turn on led
            Pc13::Clear();
            // Send info to USART
            UsartConnection::Write("LED is turn on\r\n", 16);
            // Reset buffer
            Size = 0;
        }
        if(Size == 3 && Buffer[0] == 'o' && Buffer[1] == 'f' && Buffer[2] == 'f')
        {
            // Turn on led
            Pc13::Set();
            // Send info to USART
            UsartConnection::Write("LED is turn off\r\n", 17);
            // Reset buffer
            Size = 0;
        }
    }
}