#define F_CPU 8000000

#include <clock.h>
#include <iopins.h>
#include <spi.h>

#include <drivers/sdcard.h>
#include <drivers/filesystem/fatfs/ff.h>


using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;

using SpiInterface = Spi1;
using SdCardReader = Drivers::SdCard<Spi1, IO::Pa4>;

void ConfigureClock();

int main()
{	
    ConfigureClock();

    SpiInterface::Init(SpiInterface::Fast, SpiInterface::Master);
    SpiInterface::SelectPins<Pa7, Pa6, Pa5, Pa4>();

    auto sdCardType = SdCardReader::Detect();
    // Check if sdCardType != SdCardNone

    FATFS fs;
    FRESULT res;

    res = f_mount(&fs, "", 0);
    // Check res == FR_OK

    DIR dir;
    res = f_opendir(&dir, "/");
    // Check res == FR_OK

    FILINFO fileInfo;
    for(;;)
    {
        res = f_readdir(&dir, &fileInfo);
        if((res != FR_OK) || (fileInfo.fname[0] == '\0'))
            break;
    }

    for (;;)
    {
    }
}

void ConfigureClock()
{
    PllClock::SelectClockSource(PllClock::ClockSource::External);
    PllClock::SetMultiplier(9);
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    SysClock::SelectClockSource(SysClock::Pll);
}