#define F_CPU 8000000

#include <one_wire.h>
#include <usart.h>

using namespace Zhele;

using Line = OneWire<Usart1, IO::Pa9>;

int main()
{
    // This example detect address (ROM) of one connected device.
    uint8_t rom[8] = {};
    Line::Init();
    Line::ReadRom(rom);
    // Now @rom array contains address of sensor. You can use it in future.
    

    volatile bool a = false;

    for (;;)
    {
    }
}