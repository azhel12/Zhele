#include <zhele/i2c.h>
#include <zhele/drivers/ds1307.h>

using namespace Zhele;
using namespace Zhele::Drivers;

#if defined (STM32G0)
using Interface = I2c1<>;
#else
using Interface = I2c1;
#endif

using Rtc = Ds1307<Interface>;

int main()
{
    Interface::Init();
    Interface::SelectPins<IO::Pb6, IO::Pb7>();

    // Write date
    Rtc::SetDay(23);
    Rtc::SetMonth(1);
    Rtc::SetYear(21);
    Rtc::SetWeekday(6);
    Rtc::SetHours(12);
    Rtc::SetMinutes(31);
    Rtc::SetSeconds(00);

    // Read all data
    auto dt = Rtc::GetDateTime();

    for (;;)
    {
    }
}